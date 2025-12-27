import serial
import threading
import time
import tkinter as tk
from tkinter import Canvas
from PIL import Image, ImageTk

# ===================== SERIAL CONFIG =====================
PORT = "COM5"
BAUDRATE = 115200

# ===================== GUI SETUP =====================
root = tk.Tk()
root.title("Porta Tree Race Display")
root.geometry("1200x650")
root.configure(bg="black")
root.minsize(900, 500)

canvas = Canvas(root, bg="black", highlightthickness=0)
canvas.pack(fill="both", expand=True)

# ===================== LOAD & RESIZE BACKGROUND =====================
bg_image = Image.open("assets/bg.jpg")
bg_photo = None
bg_item = canvas.create_image(0, 0, anchor="nw")
def resize_bg(event):
    resized = bg_image.resize((event.width, event.height), Image.LANCZOS)
    canvas.bg_photo = ImageTk.PhotoImage(resized)
    canvas.itemconfig(bg_item, image=canvas.bg_photo)

# Force initial image set
canvas.bg_photo = ImageTk.PhotoImage(bg_image.resize((1200, 650), Image.LANCZOS))
canvas.itemconfig(bg_item, image=canvas.bg_photo)


# ===================== HEADER =====================
title_text = canvas.create_text(
    0, 0,
    text="PORTA TREE READY",
    fill="#00aa00",
    font=("Arial Black", 36),
    anchor="n"
)

# ===================== RESET BUTTON =====================
def manual_reset():
    update_title("PORTA TREE READY", "#00aa00")
    update_lane(1, "PLS PRE-STAGE", "black")
    update_lane(2, "PLS PRE-STAGE", "black")

reset_btn = tk.Button(
    root,
    text="RESET",
    bg="red",
    fg="white",
    font=("Arial Black", 14),
    command=manual_reset
)

reset_window = canvas.create_window(30, 30, anchor="nw", window=reset_btn)

# ===================== LANE LABELS =====================
lane1_label = canvas.create_text(0, 0, text="LANE 1", fill="yellow", font=("Arial Black", 22))
lane2_label = canvas.create_text(0, 0, text="LANE 2", fill="yellow", font=("Arial Black", 22))

# ===================== STATUS PANELS =====================
lane1_box = canvas.create_rectangle(0, 0, 0, 0, fill="black", outline="white", width=2)
lane2_box = canvas.create_rectangle(0, 0, 0, 0, fill="black", outline="white", width=2)

lane1_text = canvas.create_text(0, 0, text="PLS PRE-STAGE", fill="white", font=("Consolas", 26, "bold"))
lane2_text = canvas.create_text(0, 0, text="PLS PRE-STAGE", fill="white", font=("Consolas", 26, "bold"))

# ===================== RESPONSIVE LAYOUT =====================
def layout(event):
    w, h = event.width, event.height

    canvas.coords(title_text, w // 2, 40)

    canvas.coords(lane1_label, w * 0.3, h * 0.45 - 70)
    canvas.coords(lane2_label, w * 0.7, h * 0.45 - 70)

    canvas.coords(lane1_box, w * 0.15, h * 0.45, w * 0.45, h * 0.60)
    canvas.coords(lane2_box, w * 0.55, h * 0.45, w * 0.85, h * 0.60)

    canvas.coords(lane1_text, w * 0.3, h * 0.525)
    canvas.coords(lane2_text, w * 0.7, h * 0.525)

canvas.bind("<Configure>", layout)

# ===================== UPDATE FUNCTIONS =====================
def update_lane(lane, text, color):
    if lane == 1:
        canvas.itemconfig(lane1_text, text=text)
        canvas.itemconfig(lane1_box, fill=color)
    elif lane == 2:
        canvas.itemconfig(lane2_text, text=text)
        canvas.itemconfig(lane2_box, fill=color)

def update_title(text, color):
    canvas.itemconfig(title_text, text=text, fill=color)

# ===================== AUTO RESET =====================
def reset_gui(delay=3):
    def _reset():
        time.sleep(delay)
        canvas.after(0, manual_reset)
    threading.Thread(target=_reset, daemon=True).start()

# ===================== SERIAL LISTENER (UNCHANGED LOGIC) =====================
def serial_listener():
    try:
        ser = serial.Serial(PORT, BAUDRATE, timeout=1)
        print(f"Connected to {PORT}")

        while True:
            if ser.in_waiting:
                line = ser.readline().decode(errors="ignore").strip()
                if not line:
                    continue

                print("SERIAL:", line)
                u = line.upper()
                
                # ---------- PRE-STAGE / READY ----------
                if "PRE-STAGE" in u or "PRE STAGE" in u:
                    update_lane(1, "PLS PRE-STAGE", "black")
                    update_lane(2, "PLS PRE-STAGE", "black")
                    update_title("PORTA TREE READY", "#00aa00")

                elif "READY" in u or "VEHICLES READY" in u:
                    update_title("READY", "orange")
                    update_lane(1, "READY", "#ff4600")
                    update_lane(2, "READY", "#ff4600")

                elif "SET" in u or "VEHICLES SET" in u:
                    update_title("SET", "orange")
                    update_lane(1, "SET", "#bf3400")
                    update_lane(2, "SET", "#bf3400")

                # ---------- STAGED ----------
                elif "LANE1" in u and "STAGED" in u:
                    update_lane(1, "STAGED", "#00aaff")

                elif "LANE2" in u and "STAGED" in u:
                    update_lane(2, "STAGED", "#00aaff")

                # ---------- COUNTDOWN ----------
                elif u in ["3", "2", "1"]:
                    update_title(u, "orange")
                    update_lane(1, "READY", "#ffaa00")
                    update_lane(2, "READY", "#ffaa00")

                
                if "LANE 1" in u and "BAD START" in u:
                    update_title("BAD START", "red")
                    update_lane(1, "FALSE START", "#ff0000")
                    update_lane(2, "GOOD", "#00aa00")
                    reset_gui(3)

                elif "LANE 2" in u and "BAD START" in u:
                    update_title("BAD START", "red")
                    update_lane(2, "FALSE START", "#ff0000")
                    update_lane(1, "GOOD", "#00aa00")
                    reset_gui(3)

                elif "DOUBLE" in u and "BAD START" in u:
                    update_title("BOTH FALSE START", "red")
                    update_lane(1, "FALSE START", "#ff0000")
                    update_lane(2, "FALSE START", "#ff0000")
                    reset_gui(3)

                elif "LANE 1 WINS" in u:
                    update_title("FINISH", "white")
                    update_lane(1, "WINNER", "#00ff00")
                    update_lane(2, "LOSE", "#ff0000")
                    reset_gui(4)

                elif "LANE 2 WINS" in u:
                    update_title("FINISH", "white")
                    update_lane(2, "WINNER", "#00ff00")
                    update_lane(1, "LOSE", "#ff0000")
                    reset_gui(4)

                    # ---------- TIE ----------
                elif "DOUBLE WINNER" in u:
                    update_title("TIE!", "white")
                    update_lane(1, "TIE", "#00ffff")
                    update_lane(2, "TIE", "#00ffff")
                    reset_gui(4)

    except Exception as e:
        print("Serial Error:", e)

# ===================== START THREAD =====================
threading.Thread(target=serial_listener, daemon=True).start()

manual_reset()
root.mainloop()
