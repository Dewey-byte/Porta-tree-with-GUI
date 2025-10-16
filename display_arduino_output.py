import tkinter as tk
from tkinter import Canvas
import serial
import serial.tools.list_ports
import threading
import time
from PIL import Image, ImageTk

# -------------------------------
# CONFIGURATION
# -------------------------------
BAUD = 9600
RESET_TIME = 20  # seconds
BOX_COLOR = "#202020"
TEXT_COLOR = "white"
BOX_TRANSPARENCY = 0.5  # simulated using darker colors

# -------------------------------
# AUTO-DETECT SERIAL PORT
# -------------------------------
def find_arduino_port():
    ports = serial.tools.list_ports.comports()
    for port in ports:
        if "Arduino" in port.description or "USB Serial" in port.description:
            print(f"✅ Arduino detected on {port.device}")
            return port.device
    if ports:
        print(f"⚠️ Arduino not found, using first available: {ports[0].device}")
        return ports[0].device
    else:
        print("❌ No serial ports detected.")
        return None

PORT = find_arduino_port()

# -------------------------------
# GUI SETUP
# -------------------------------
root = tk.Tk()
root.title("Porta Tree Race Display")
root.geometry("1200x700")
root.configure(bg="black")

canvas = Canvas(root, bg="black", highlightthickness=0)
canvas.pack(fill="both", expand=True)

# -------------------------------
# BACKGROUND IMAGE
# -------------------------------
try:
    original_bg = Image.open("bg.jpg")
    bg_photo = ImageTk.PhotoImage(original_bg)
    bg_item = canvas.create_image(0, 0, anchor="nw", image=bg_photo)
except Exception as e:
    print("⚠️ Background image not found:", e)
    bg_item = None
    original_bg = None

# -------------------------------
# BLINKING CONTROL
# -------------------------------
blinking_states = {}

def blink_text(item, color1, color2, delay=500):
    current = blinking_states.get(item, False)
    new_color = color1 if not current else color2
    canvas.itemconfig(item, fill=new_color)
    blinking_states[item] = not current
    canvas.after(delay, blink_text, item, color1, color2, delay)

# -------------------------------
# CREATE ELEMENTS
# -------------------------------
title_text = canvas.create_text(0, 0, text="PORTA TREE READY", fill="lime", font=("Consolas", 32, "bold"))

laneA_label = canvas.create_text(0, 0, text="LANE 1", fill="white", font=("Consolas", 20, "bold"))
laneB_label = canvas.create_text(0, 0, text="LANE 2", fill="white", font=("Consolas", 20, "bold"))

laneA_box = canvas.create_rectangle(0, 0, 0, 0, fill="#111111", outline="white", width=2)
laneB_box = canvas.create_rectangle(0, 0, 0, 0, fill="#111111", outline="white", width=2)

laneA_text = canvas.create_text(0, 0, text="WAITING...", fill="white", font=("Consolas", 22, "bold"))
laneB_text = canvas.create_text(0, 0, text="WAITING...", fill="white", font=("Consolas", 22, "bold"))

stats_text = canvas.create_text(0, 0, text="", fill="white", font=("Consolas", 24, "bold"))

# -------------------------------
# RESPONSIVE LAYOUT
# -------------------------------
def resize(event=None):
    w, h = canvas.winfo_width(), canvas.winfo_height()

    # Resize background
    if original_bg:
        resized_bg = original_bg.resize((w, h))
        bg_photo = ImageTk.PhotoImage(resized_bg)
        canvas.itemconfig(bg_item, image=bg_photo)
        canvas.bg_photo = bg_photo  # prevent garbage collection

    # Scale fonts dynamically
    title_font_size = int(h * 0.045)
    lane_font_size = int(h * 0.03)
    stats_font_size = int(h * 0.035)

    canvas.itemconfig(title_text, font=("Consolas", title_font_size, "bold"))
    canvas.itemconfig(laneA_label, font=("Consolas", lane_font_size, "bold"))
    canvas.itemconfig(laneB_label, font=("Consolas", lane_font_size, "bold"))
    canvas.itemconfig(laneA_text, font=("Consolas", lane_font_size, "bold"))
    canvas.itemconfig(laneB_text, font=("Consolas", lane_font_size, "bold"))
    canvas.itemconfig(stats_text, font=("Consolas", stats_font_size, "bold"))

    # Positions
    center_x = w / 2

    canvas.coords(title_text, center_x, h * 0.1)

    box_width = w * 0.35
    box_height = h * 0.15
    spacing = w * 0.05

    # Lane A
    x1a = center_x - box_width - spacing / 2
    x2a = center_x - spacing / 2
    y1 = h * 0.4
    y2 = y1 + box_height

    # Lane B
    x1b = center_x + spacing / 2
    x2b = center_x + box_width + spacing / 2

    canvas.coords(laneA_box, x1a, y1, x2a, y2)
    canvas.coords(laneB_box, x1b, y1, x2b, y2)

    canvas.coords(laneA_label, (x1a + x2a) / 2, y1 - 40)
    canvas.coords(laneB_label, (x1b + x2b) / 2, y1 - 40)

    canvas.coords(laneA_text, (x1a + x2a) / 2, (y1 + y2) / 2)
    canvas.coords(laneB_text, (x1b + x2b) / 2, (y1 + y2) / 2)

    canvas.coords(stats_text, center_x, h * 0.75)

canvas.bind("<Configure>", resize)

# -------------------------------
# UI LOGIC
# -------------------------------
def update_lane(lane, state, color=None, blink=False):
    text_id = laneA_text if lane == 1 else laneB_text
    box_id = laneA_box if lane == 1 else laneB_box

    canvas.itemconfig(text_id, text=state)
    if color:
        canvas.itemconfig(box_id, fill=color)
    else:
        canvas.itemconfig(box_id, fill="#111111")

    if blink:
        blink_text(text_id, color, "white", 500)
    else:
        canvas.itemconfig(text_id, fill="white")

def show_stats(rt, et, kph):
    canvas.itemconfig(stats_text, text=f"RT: {rt} | ET: {et} | KPH: {kph}")

def reset_race():
    canvas.itemconfig(title_text, text="RESETTING...", fill="yellow")
    canvas.itemconfig(stats_text, text="")
    update_lane(1, "WAITING...")
    update_lane(2, "WAITING...")
    root.after(RESET_TIME * 1000, lambda: canvas.itemconfig(title_text, text="PORTA TREE READY", fill="lime"))

def handle_serial():
    if not PORT:
        print("❌ No serial port found.")
        return

    try:
        ser = serial.Serial(PORT, BAUD)
        while True:
            data = ser.readline().decode().strip()
            if not data:
                continue
            print("📡 Serial:", data)

            if "L1_PRESTAGE" in data:
                update_lane(1, "PRE-STAGE", "orange", blink=True)
            elif "L2_PRESTAGE" in data:
                update_lane(2, "PRE-STAGE", "orange", blink=True)
            elif "L1_STAGE" in data:
                update_lane(1, "PLS STAGE", "cyan", blink=True)
            elif "L2_STAGE" in data:
                update_lane(2, "PLS STAGE", "cyan", blink=True)
            elif "L1_FALSESTART" in data:
                update_lane(1, "FALSE START", "red", blink=True)
            elif "L2_FALSESTART" in data:
                update_lane(2, "FALSE START", "red", blink=True)
            elif "WINNER:1" in data:
                update_lane(1, "WINNER", "lime", blink=True)
            elif "WINNER:2" in data:
                update_lane(2, "WINNER", "lime", blink=True)
            elif "RESULT:TIE" in data:
                update_lane(1, "DRAW", "lime", blink=True)
                update_lane(2, "DRAW", "lime", blink=True)
            elif "L1_RT" in data or "L2_RT" in data:
                parts = data.replace("L1_", "").replace("L2_", "").split("|")
                rt = parts[0].split(":")[1]
                et = parts[1].split(":")[1]
                kph = parts[2].split(":")[1]
                show_stats(rt, et, kph)
                reset_race()
    except Exception as e:
        print("Serial error:", e)

# -------------------------------
# MAIN LOOP
# -------------------------------
blink_text(title_text, "lime", "#004400", 600)  # title blinking
threading.Thread(target=handle_serial, daemon=True).start()
resize()
root.mainloop()
