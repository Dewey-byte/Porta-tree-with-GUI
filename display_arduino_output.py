import serial
import serial.tools.list_ports
import threading
import time
import tkinter as tk
from tkinter import Canvas
from PIL import Image, ImageTk
import sys
import os

# ===================== CONFIG =====================
BAUDRATE = 115200
BG_OPACITY = 0.35
PRE_STAGE_TIMEOUT = 1.0

# ===================== SERIAL AUTO-DETECT =====================
def find_serial_port():
    ports = list(serial.tools.list_ports.comports())
    if not ports:
        print("❌ No serial ports found")
        return None
    print("Available serial ports:")
    for p in ports:
        print(f" - {p.device}")
    return ports[0].device

PORT = find_serial_port()

# ===================== RESOURCE PATH =====================
def resource_path(relative_path):
    try:
        base_path = sys._MEIPASS
    except Exception:
        base_path = os.path.abspath(".")
    return os.path.join(base_path, relative_path)

# ===================== GLOBAL STATE =====================
current_kph = ""
current_end_time = ""
lane1_last_seen = 0
lane2_last_seen = 0
running_phase = False  # True when race is running

# ===================== GUI SETUP =====================
root = tk.Tk()
root.title("Porta Tree Race Display")
root.geometry("1200x650")
root.configure(bg="black")
root.minsize(900, 500)

canvas = Canvas(root, bg="black", highlightthickness=0)
canvas.pack(fill="both", expand=True)

# ===================== LOAD BACKGROUND =====================
bg_image_original = Image.open(resource_path("assets/bg.jpg")).convert("RGBA")
bg_item = canvas.create_image(0, 0, anchor="nw")

def apply_opacity(image, opacity):
    alpha = int(255 * opacity)
    r, g, b, a = image.split()
    a = a.point(lambda _: alpha)
    return Image.merge("RGBA", (r, g, b, a))

# ===================== LANES =====================
lane1_label = canvas.create_text(0, 0, text="LANE 1", fill="yellow", font=("Arial Black", 22))
lane2_label = canvas.create_text(0, 0, text="LANE 2", fill="yellow", font=("Arial Black", 22))

lane1_box = canvas.create_rectangle(0, 0, 0, 0, fill="black", outline="white", width=2)
lane2_box = canvas.create_rectangle(0, 0, 0, 0, fill="black", outline="white", width=2)

lane1_text = canvas.create_text(0, 0, text="PLS PRE-STAGE", fill="white", font=("Consolas", 26, "bold"))
lane2_text = canvas.create_text(0, 0, text="PLS PRE-STAGE", fill="white", font=("Consolas", 26, "bold"))

# === Reaction Time (RT outside the box) ===
lane1_rt_text = canvas.create_text(0, 0, text="", fill="cyan", font=("Consolas", 25, "bold"))
lane2_rt_text = canvas.create_text(0, 0, text="", fill="cyan", font=("Consolas", 25, "bold"))

# === KPH below lane box ===
lane1_kph_text = canvas.create_text(0, 0, text="", fill="white", font=("Consolas", 25, "bold"))
lane2_kph_text = canvas.create_text(0, 0, text="", fill="white", font=("Consolas", 25, "bold"))

# === Estimated Speed label above KPH ===
lane1_est_speed_label = canvas.create_text(0, 0, text="Estimated Speed", fill="white", font=("Consolas", 20, "bold"))
lane2_est_speed_label = canvas.create_text(0, 0, text="Estimated Speed", fill="white", font=("Consolas", 20, "bold"))

# Hide initially
canvas.itemconfig(lane1_est_speed_label, state="hidden")
canvas.itemconfig(lane2_est_speed_label, state="hidden")

# ===================== RESULTS =====================
winner_text = canvas.create_text(0, 0, text="", fill="white", font=("Consolas", 70, "bold"))

# ===================== RESPONSIVE LAYOUT =====================
def on_resize(event):
    w, h = event.width, event.height

    resized = bg_image_original.resize((w, h), Image.LANCZOS)
    faded = apply_opacity(resized, BG_OPACITY)
    canvas.bg_photo = ImageTk.PhotoImage(faded)
    canvas.itemconfig(bg_item, image=canvas.bg_photo)

    canvas.coords(lane1_label, w * 0.1, h * 0.1 - 30)
    canvas.coords(lane1_box, w * 0.05, h * 0.1, w * 0.25, h * 0.25)
    canvas.coords(lane1_text, w * 0.15, h * 0.165)
    canvas.coords(lane1_rt_text, w * 0.15, h * 0.29)
    canvas.coords(lane1_kph_text, w * 0.15, h * 0.35)

    canvas.coords(lane2_label, w * 0.9, h * 0.1 - 30)
    canvas.coords(lane2_box, w * 0.75, h * 0.1, w * 0.95, h * 0.25)
    canvas.coords(lane2_text, w * 0.85, h * 0.165)
    canvas.coords(lane2_rt_text, w * 0.85, h * 0.29)
    canvas.coords(lane2_kph_text, w * 0.85, h * 0.35)
    canvas.coords(lane1_est_speed_label, w * 0.15, h * 0.32)  # just above KPH
    canvas.coords(lane2_est_speed_label, w * 0.85, h * 0.32)  # just above KPH


    canvas.coords(winner_text, w/2, h * 0.50)  #  center

canvas.bind("<Configure>", on_resize)

# ===================== HELPERS =====================
def update_lane(lane, text, color):
    if lane == 1:
        canvas.itemconfig(lane1_text, text=text)
        canvas.itemconfig(lane1_box, fill=color)
    else:
        canvas.itemconfig(lane2_text, text=text)
        canvas.itemconfig(lane2_box, fill=color)

def update_lane_et(lane, et):
    txt = f" {et:.2f} s"
    if lane == 1:
        canvas.itemconfig(lane1_text, text=txt)
        canvas.itemconfig(lane1_box, fill="black")
    else:
        canvas.itemconfig(lane2_text, text=txt)
        canvas.itemconfig(lane2_box, fill="black")

def update_lane_kph(lane, kph):
    txt = f"KPH: {kph:.2f}"
    if lane == 1:
        canvas.itemconfig(lane1_kph_text, text=txt)
        # show label when KPH is available
        canvas.itemconfig(lane1_est_speed_label, state="normal")
    else:
        canvas.itemconfig(lane2_kph_text, text=txt)
        canvas.itemconfig(lane2_est_speed_label, state="normal")

def update_reaction_time(lane, rt):
    txt = f"Reaction Time: {rt:.2f} s"
    if lane == 1:
        canvas.itemconfig(lane1_rt_text, text=txt)
    else:
        canvas.itemconfig(lane2_rt_text, text=txt)

# ===================== WINNER & BLINKING =====================
def show_winner(text):
    canvas.itemconfig(winner_text, text=text)
    
    # Reset lane box colors first
    canvas.itemconfig(lane1_box, fill="black")
    canvas.itemconfig(lane2_box, fill="black")
    
    # Color lanes based on winner
    if "Lane 1" in text:
        canvas.itemconfig(lane1_box, fill="#00aaff")
        canvas.itemconfig(lane2_box, fill="red")
    elif "Lane 2" in text:
        canvas.itemconfig(lane1_box, fill="red")
        canvas.itemconfig(lane2_box, fill="#00aaff")
    elif "Draw" in text:
        canvas.itemconfig(lane1_box, fill="yellow")
        canvas.itemconfig(lane2_box, fill="yellow")

def blink_winner():
    # Toggle visibility for blinking effect
    current_color = canvas.itemcget(winner_text, "fill")
    if current_color:  # visible
        canvas.itemconfig(winner_text, fill="")
    else:  # hidden
        canvas.itemconfig(winner_text, fill="white")
    canvas.after(500, blink_winner)  # repeat every 500ms

# Start blinking
blink_winner()

# ===================== AUTO RESET =====================
def reset_gui(delay=3):
    def _reset():
        global running_phase
        time.sleep(delay)
        running_phase = False
        canvas.after(0, manual_reset)
    threading.Thread(target=_reset, daemon=True).start()
    
def manual_reset():
    update_lane(1, "PLS PRE-STAGE", "black")
    update_lane(2, "PLS PRE-STAGE", "black")
    canvas.itemconfig(lane1_rt_text, text="")
    canvas.itemconfig(lane2_rt_text, text="")
    canvas.itemconfig(lane1_kph_text, text="")
    canvas.itemconfig(lane2_kph_text, text="")
    canvas.itemconfig(lane1_est_speed_label, state="hidden")
    canvas.itemconfig(lane2_est_speed_label, state="hidden")
    show_winner("")

# ===================== PRE-STAGE WATCHDOG =====================
def prestage_watchdog():
    global lane1_last_seen, lane2_last_seen
    now = time.time()

    if lane1_last_seen and now - lane1_last_seen > PRE_STAGE_TIMEOUT:
        update_lane(1, "PLS PRE-STAGE", "black")
        lane1_last_seen = 0

    if lane2_last_seen and now - lane2_last_seen > PRE_STAGE_TIMEOUT:
        update_lane(2, "PLS PRE-STAGE", "black")
        lane2_last_seen = 0

    canvas.after(200, prestage_watchdog)

# ===================== SERIAL LISTENER =====================
def serial_listener():
    global current_kph, current_end_time
    global lane1_last_seen, lane2_last_seen, running_phase

    try:
        ser = serial.Serial(PORT, BAUDRATE, timeout=1)
        print(f"Connected to {PORT}")

        while True:
            line = ser.readline().decode(errors="ignore").strip()
            if not line:
                continue

            print("SERIAL:", line)
            u = line.upper()

            # ---- Reaction Time ----
            if "REACTION TIME 1:" in u:
                canvas.after(0, update_reaction_time, 1, float(line.split(":")[1]))
            elif "REACTION TIME 2:" in u:
                canvas.after(0, update_reaction_time, 2, float(line.split(":")[1]))

            # ---- Live ET (running) ----
            elif "L1 ET:" in u and "L2 ET:" in u:
                try:
                    parts = line.split("|")
                    l1_et = float(parts[0].split("L1 ET:")[1].strip().split()[0])
                    l2_et = float(parts[1].split("L2 ET:")[1].strip().split()[0])
                    running_phase = True
                    canvas.after(0, update_lane_et, 1, l1_et)
                    canvas.after(0, update_lane_et, 2, l2_et)
                except:
                    pass
                continue

            # ---- Final ET & KPH ----
            elif "LANE 1 ET:" in u and "KPH:" in u:
                try:
                    parts = line.split("|")
                    l1_et = float(parts[0].split("Lane 1 ET:")[1].strip().split()[0])
                    l1_kph = float(parts[1].split("KPH:")[1].strip())
                    canvas.after(0, update_lane_et, 1, l1_et)
                    canvas.after(0, update_lane_kph, 1, l1_kph)
                except:
                    pass
            elif "LANE 2 ET:" in u and "KPH:" in u:
                try:
                    parts = line.split("|")
                    l2_et = float(parts[0].split("Lane 2 ET:")[1].strip().split()[0])
                    l2_kph = float(parts[1].split("KPH:")[1].strip())
                    canvas.after(0, update_lane_et, 2, l2_et)
                    canvas.after(0, update_lane_kph, 2, l2_kph)
                except:
                    pass

            # ---- Winner / Draw ----
            elif "WINNER: LANE 1" in u:
                canvas.after(0, show_winner, "Winner: Lane 1")
                reset_gui(15)
            elif "WINNER: LANE 2" in u:
                canvas.after(0, show_winner, "Winner: Lane 2")
                reset_gui(15)
            elif "TIE RACE" in u or "DRAW" in u:
                canvas.after(0, show_winner, "Draw")
                reset_gui(15)

            # ---- Pre-stage / staged ----
            elif "PRE STAGED 1" in u:
                lane1_last_seen = time.time()
                canvas.after(0, update_lane, 1, "PRE-STAGED")
            elif "PRE STAGED 2" in u:
                lane2_last_seen = time.time()
                canvas.after(0, update_lane, 2, "PRE-STAGED")
            elif "SET" in u:
                if not running_phase:  # hide SET if race is running
                    canvas.after(0, update_lane, 1, "SET", "#bf3400")
                    canvas.after(0, update_lane, 2, "SET", "#bf3400")
            elif "LANE1" in u and "STAGED" in u:
                canvas.after(0, update_lane, 1, "STAGED", "#00aaff")
            elif "LANE2" in u and "STAGED" in u:
                canvas.after(0, update_lane, 2, "STAGED", "#00aaff")

            # ---- False starts / Good run ----
            elif "LANE 1" in u and "BAD START" in u:
                canvas.after(0, update_lane, 1, "FALSE START", "#ff0000")
                canvas.after(0, update_lane, 2, "GOOD", "#00aaff")
                reset_gui(3)
            elif "LANE 2" in u and "BAD START" in u:
                canvas.after(0, update_lane, 2, "FALSE START", "#ff0000")
                canvas.after(0, update_lane, 1, "GOOD", "#00aaff")
                reset_gui(3)
            elif "DOUBLE" in u and "BAD START" in u:
                canvas.after(0, update_lane, 1, "FALSE START", "#ff0000")
                canvas.after(0, update_lane, 2, "FALSE START", "#ff0000")
                reset_gui(3)

    except Exception as e:
        print("Serial Error:", e)

# ===================== START =====================
threading.Thread(target=serial_listener, daemon=True).start()
manual_reset()
prestage_watchdog()
root.mainloop()
