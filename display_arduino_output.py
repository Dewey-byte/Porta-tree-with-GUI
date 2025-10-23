import tkinter as tk
from tkinter import Canvas
import serial
import threading
import time
from PIL import Image, ImageTk

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
try:
    bg_img = Image.open("bg.jpg")
    bg_photo = ImageTk.PhotoImage(bg_img)
    bg_item = canvas.create_image(0,0,anchor="nw", image=bg_photo)
except:
    bg_img = None
    bg_item = None

# -------------------------------
# GUI Elements
title_text = canvas.create_text(0,0,text="PORTA TREE READY", fill="lime", font=("Consolas",32,"bold"))
lane_boxes = []
lane_texts = []

for i in range(NUM_LANES):
    box = canvas.create_rectangle(0,0,0,0,fill="#111111", outline="white", width=2)
    lane_boxes.append(box)
    txt = canvas.create_text(0,0,text="PLS PRE-STAGE", fill="white", font=("Consolas",22,"bold"))
    lane_texts.append(txt)

# -------------------------------
# Track state
lane_running = [False]*NUM_LANES
lane_start_time = [0]*NUM_LANES
lane_elapsed = [0]*NUM_LANES
lane_prestaged = [False]*NUM_LANES
lane_false = [False]*NUM_LANES
lane_reaction = [0]*NUM_LANES
race_started = False
countdown_running = False
result_shown = False

# -------------------------------
# BLINKING TITLE
blinking_state = False
def blink_title():
    global blinking_state
    color = "lime" if blinking_state else "#004400"
    canvas.itemconfig(title_text, fill=color)
    blinking_state = not blinking_state
    canvas.after(600, blink_title)
blink_title()

# -------------------------------
def resize(event=None):
    w, h = canvas.winfo_width(), canvas.winfo_height()
    if bg_img:
        resized_bg = bg_img.resize((w,h))
        bg_photo_resized = ImageTk.PhotoImage(resized_bg)
        canvas.itemconfig(bg_item, image=bg_photo_resized)
        canvas.bg_photo_resized = bg_photo_resized
    canvas.itemconfig(title_text, font=("Consolas", int(h*0.045),"bold"))
    canvas.coords(title_text, w/2, h*0.1)

    box_width = w*0.35
    box_height = h*0.15
    spacing = w*0.05
    y1 = h*0.4
    y2 = y1 + box_height
    for i in range(NUM_LANES):
        x1 = (w/2 - box_width - spacing/2) if i==0 else (w/2 + spacing/2)
        x2 = (w/2 - spacing/2) if i==0 else (w/2 + box_width + spacing/2)
        canvas.coords(lane_boxes[i], x1, y1, x2, y2)
        canvas.coords(lane_texts[i], (x1+x2)/2, (y1+y2)/2)
canvas.bind("<Configure>", resize)
resize()

# -------------------------------
def update_lane_running(lane):
    """Update lane box with running time in seconds"""
    elapsed_sec = int(lane_elapsed[lane-1]/1000)
    canvas.itemconfig(lane_texts[lane-1], text=f"RUNNING\n{elapsed_sec}s")
    canvas.itemconfig(lane_boxes[lane-1], fill="green")

def update_lane_result(lane, state, rt, et, kph):
    """Update lane box after race finished, display stats"""
    txt_id = lane_texts[lane-1]
    box_id = lane_boxes[lane-1]
    canvas.itemconfig(txt_id, text=f"{state}\nRT:{rt}ms ET:{et}ms KPH:{kph}")
    color = "red" if state=="FALSE START" else "green"
    canvas.itemconfig(box_id, fill=color)

def reset_gui():
    global race_started, result_shown, countdown_running
    race_started = False
    countdown_running = False
    result_shown = False
    for i in range(NUM_LANES):
        lane_running[i] = False
        lane_start_time[i] = 0
        lane_elapsed[i] = 0
        lane_prestaged[i] = False
        lane_false[i] = False
        lane_reaction[i] = 0
        canvas.itemconfig(lane_texts[i], text="PLS PRE-STAGE")
        canvas.itemconfig(lane_boxes[i], fill="#111111")
    canvas.itemconfig(title_text,text="PORTA TREE READY", fill="lime")

# -------------------------------
def handle_serial():
    global race_started, countdown_running, result_shown
    try:
        ser = serial.Serial(PORT, BAUD, timeout=1)
        print(f"Connected to {PORT}")
        while True:
            line = ser.readline().decode(errors='ignore').strip()
            if not line:
                # update running timer
                if race_started and not result_shown:
                    for i in range(NUM_LANES):
                        if lane_running[i]:
                            lane_elapsed[i] = int((time.time() - lane_start_time[i])*1000)
                            update_lane_running(i+1)
                continue
            print("Serial:", line)

            # --- PRESTAGE / STAGE ---
            if "PRESTAGE" in line:
                lane = int(line[1])
                lane_prestaged[lane-1] = True
                canvas.itemconfig(lane_texts[lane-1], text="PRE-STAGE")
                canvas.itemconfig(lane_boxes[lane-1], fill="orange")
            elif "STAGE" in line:
                lane = int(line[1])
                canvas.itemconfig(lane_texts[lane-1], text="STAGE")
                canvas.itemconfig(lane_boxes[lane-1], fill="cyan")

            # --- GO ---
            elif "GO" in line:
                if all(lane_prestaged):
                    race_started = True
                    countdown_running = False
                    for i in range(NUM_LANES):
                        lane_running[i] = True
                        lane_start_time[i] = time.time()
                        lane_elapsed[i] = 0
                        canvas.itemconfig(lane_texts[i], text="GO\n0s")
                        canvas.itemconfig(lane_boxes[i], fill="green")
                else:
                    print("⚠️ Countdown blocked: not all lanes prestaged")

            # --- FALSE START (during countdown) ---
            elif "FALSESTART" in line:
                lane = int(line[1])
                lane_false[lane-1] = True
                canvas.itemconfig(lane_texts[lane-1], text="FALSE START")
                canvas.itemconfig(lane_boxes[lane-1], fill="red")

            # --- FINISH ---
            elif "FINISH" in line:
                lane = int(line[1])
                lane_running[lane-1] = False

            # --- FINAL STATS ---
            elif "_RT:" in line:
                if not result_shown:
                    result_shown = True
                    for i in range(NUM_LANES):
                        parts = line.split("_RT:")[1].split("|")
                        rt = int(parts[0])
                        et = int(parts[1].split(":")[1])
                        kph = int(parts[2].split(":")[1])
                        state = "FALSE START" if lane_false[i] else "WINNER" if i==0 else "LOSER"
                        update_lane_result(i+1, state, rt, et, kph)

            # --- RESET ---
            elif "RESET" in line:
                reset_gui()

    except Exception as e:
        print("Serial error:", e)

# -------------------------------
threading.Thread(target=handle_serial, daemon=True).start()
root.mainloop()
