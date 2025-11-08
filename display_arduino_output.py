import tkinter as tk
from tkinter import Canvas
from PIL import Image, ImageTk
import threading
import time
import serial

# -------------------------------
PORT = "COM5"
BAUD = 115200
NUM_LANES = 2
# -------------------------------

root = tk.Tk()
root.title("Porta Tree Race Display")
root.geometry("1200x700")
root.configure(bg="black")

canvas = Canvas(root, bg="black", highlightthickness=0)
canvas.pack(fill="both", expand=True)

# -------------------------------
# Load background image
bg_image = Image.open("bg.jpg")
bg_photo = ImageTk.PhotoImage(bg_image)
bg_item = canvas.create_image(0, 0, anchor="nw", image=bg_photo)

# -------------------------------
# GUI Elements
title_text = canvas.create_text(0, 0, text="PORTA TREE READY", fill="green", font=("Consolas", 32, "bold"))
lane_boxes, lane_texts, lane_labels = [], [], []

def resize(event=None):
    w, h = canvas.winfo_width(), canvas.winfo_height()

    # Resize and update background
    resized = bg_image.resize((w, h), Image.Resampling.LANCZOS)
    new_bg = ImageTk.PhotoImage(resized)
    canvas.itemconfig(bg_item, image=new_bg)
    canvas.image = new_bg  # prevent garbage collection

    # Title
    canvas.coords(title_text, w / 2, h * 0.1)
    canvas.itemconfig(title_text, font=("Consolas", int(h * 0.045), "bold"))

    box_width = w * 0.35
    box_height = h * 0.15
    spacing = w * 0.05
    y1 = h * 0.45
    y2 = y1 + box_height

    # Lane boxes and labels
    for i in range(NUM_LANES):
        x1 = (w / 2 - box_width - spacing / 2) if i == 0 else (w / 2 + spacing / 2)
        x2 = (w / 2 - spacing / 2) if i == 0 else (w / 2 + box_width + spacing / 2)
        canvas.coords(lane_boxes[i], x1, y1, x2, y2)
        canvas.coords(lane_texts[i], (x1 + x2) / 2, (y1 + y2) / 2)
        canvas.coords(lane_labels[i], (x1 + x2) / 2, y1 - 35)
        canvas.itemconfig(lane_texts[i], font=("Consolas", int(h * 0.04), "bold"))
        canvas.itemconfig(lane_labels[i], font=("Consolas", int(h * 0.035), "bold"))

# Create lane boxes
for i in range(NUM_LANES):
    label = canvas.create_text(0, 0, text=f"LANE {i+1}", fill="yellow", font=("Consolas", 22, "bold"))
    lane_labels.append(label)

    box = canvas.create_rectangle(0, 0, 0, 0, fill="black", outline="white", width=2)
    lane_boxes.append(box)

    txt = canvas.create_text(0, 0, text="PLS PRE-STAGE", fill="white", font=("Consolas", 24, "bold"))
    lane_texts.append(txt)

canvas.bind("<Configure>", resize)

# -------------------------------
# Reset button
def send_reset():
    try:
        ser.write(b"RESET\n")
    except:
        pass
    reset_gui()

reset_btn = tk.Button(root, text="RESET", font=("Consolas", 18, "bold"), bg="red", fg="white", width=10, command=send_reset)
reset_window = canvas.create_window(100, 60, window=reset_btn)

# -------------------------------
# Race states
race_started = False

def update_lane(lane, text, color):
    canvas.itemconfig(lane_texts[lane - 1], text=text, fill="white")
    canvas.itemconfig(lane_boxes[lane - 1], fill=color)

def blink_winner(lane):
    def blink():
        for _ in range(6):
            canvas.itemconfig(lane_boxes[lane - 1], fill="lime")
            time.sleep(0.3)
            canvas.itemconfig(lane_boxes[lane - 1], fill="black")
            time.sleep(0.3)
        canvas.itemconfig(lane_boxes[lane - 1], fill="lime")
    threading.Thread(target=blink, daemon=True).start()

def reset_gui():
    global race_started
    race_started = False
    canvas.itemconfig(title_text, text="PORTA TREE READY", fill="green")
    for i in range(NUM_LANES):
        canvas.itemconfig(lane_boxes[i], fill="black")
        canvas.itemconfig(lane_texts[i], text="PLS PRE-STAGE", fill="white")

# -------------------------------
# Serial listener
def serial_listener():
    global race_started
    try:
        ser = serial.Serial(PORT, BAUD, timeout=1)
        print(f"✅ Connected to {PORT}")
        while True:
            line = ser.readline().decode(errors='ignore').strip()
            if not line:
                continue
            print("Serial:", line)
            line = line.upper()

            lane = None
            if "L" in line:
                try:
                    lane = int(line.split("L")[-1][0])
                except:
                    lane = None

            if "PRESTAGE" in line and lane:
                update_lane(lane, "PRE-STAGE", "#222222")

            elif "STAGE" in line and lane:
                update_lane(lane, "STAGE", "#0088ff")

            elif "GO" in line:
                race_started = True
                canvas.itemconfig(title_text, text="GO!", fill="lime")

            elif "FALSESTART" in line and lane:
                update_lane(lane, "FALSE START", "red")

            elif "WINNER" in line and lane:
                update_lane(lane, "WINNER", "lime")
                canvas.itemconfig(title_text, text=f"🏁 WINNER: LANE {lane}", fill="yellow")
                blink_winner(lane)

            elif "LOSER" in line and lane:
                update_lane(lane, "LOSER", "gray")

            elif "TIE" in line:
                canvas.itemconfig(title_text, text="🤝 TIE!", fill="yellow")
                for i in range(NUM_LANES):
                    update_lane(i + 1, "TIE", "yellow")

            elif "RESET" in line:
                reset_gui()

    except Exception as e:
        print("❌ Serial error:", e)

# -------------------------------
threading.Thread(target=serial_listener, daemon=True).start()
root.mainloop()
