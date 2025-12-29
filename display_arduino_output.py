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
    return ports[0].device  # Use first available port

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

# ===================== TITLE =====================
title_text = canvas.create_text(
    0, 0,
    text="PORTA TREE READY",
    fill="#00aa00",
    font=("Arial Black", 36),
    anchor="center"
)

# ===================== BLINKING TITLE =====================
blink_job = None
blink_visible = True
blink_color = "#00aa00"

def update_title(text, color, interval=700):
    global blink_job, blink_visible, blink_color
    blink_color = color
    blink_visible = True
    canvas.itemconfig(title_text, text=text, fill=color)
    if blink_job:
        canvas.after_cancel(blink_job)

    def blink():
        global blink_visible, blink_job
        blink_visible = not blink_visible
        canvas.itemconfig(title_text, fill=blink_color if blink_visible else "")
        blink_job = canvas.after(interval, blink)

    blink()

# ===================== LANES =====================
lane1_label = canvas.create_text(0, 0, text="LANE 1", fill="yellow", font=("Arial Black", 22))
lane2_label = canvas.create_text(0, 0, text="LANE 2", fill="yellow", font=("Arial Black", 22))

lane1_box = canvas.create_rectangle(0, 0, 0, 0, fill="black", outline="white", width=2)
lane2_box = canvas.create_rectangle(0, 0, 0, 0, fill="black", outline="white", width=2)

lane1_text = canvas.create_text(0, 0, text="PLS PRE-STAGE", fill="white", font=("Consolas", 26, "bold"))
lane2_text = canvas.create_text(0, 0, text="PLS PRE-STAGE", fill="white", font=("Consolas", 26, "bold"))

# ===================== RESULTS =====================
end_time_text = canvas.create_text(0, 0, text="", fill="green", font=("Consolas", 40, "bold"))
speed_text = canvas.create_text(0, 0, text="", fill="cyan", font=("Consolas", 40, "bold"))

# ===================== RESPONSIVE LAYOUT + BG =====================
def on_resize(event):
    w, h = event.width, event.height

    resized = bg_image_original.resize((w, h), Image.LANCZOS)
    faded = apply_opacity(resized, BG_OPACITY)
    canvas.bg_photo = ImageTk.PhotoImage(faded)
    canvas.itemconfig(bg_item, image=canvas.bg_photo)

    canvas.coords(title_text, w // 2, 40)

    canvas.coords(lane1_label, w * 0.3, h * 0.45 - 70)
    canvas.coords(lane2_label, w * 0.7, h * 0.45 - 70)

    canvas.coords(lane1_box, w * 0.15, h * 0.45, w * 0.45, h * 0.60)
    canvas.coords(lane2_box, w * 0.55, h * 0.45, w * 0.85, h * 0.60)

    canvas.coords(lane1_text, w * 0.3, h * 0.525)
    canvas.coords(lane2_text, w * 0.7, h * 0.525)

    canvas.coords(end_time_text, w // 2, h * 0.70)
    canvas.coords(speed_text, w // 2, h * 0.80)

canvas.bind("<Configure>", on_resize)

# ===================== HELPERS =====================
def update_lane(lane, text, color):
    if lane == 1:
        canvas.itemconfig(lane1_text, text=text)
        canvas.itemconfig(lane1_box, fill=color)
    else:
        canvas.itemconfig(lane2_text, text=text)
        canvas.itemconfig(lane2_box, fill=color)

def show_end_time(t):
    try:
        t = float(t)
        canvas.itemconfig(end_time_text, text=f"END TIME: {t:.2f} s")
    except:
        canvas.itemconfig(end_time_text, text="END TIME: --")

def show_speed(kph):
    canvas.itemconfig(speed_text, text=f"SPEED: {kph} KPH")

# ===================== AUTO RESET =====================
def reset_gui(delay=3):
    def _reset():
        time.sleep(delay)
        canvas.after(0, manual_reset)
    threading.Thread(target=_reset, daemon=True).start()

def manual_reset():
    update_title("PORTA TREE READY", "#00aa00")
    update_lane(1, "PLS PRE-STAGE", "black")
    update_lane(2, "PLS PRE-STAGE", "black")
    canvas.itemconfig(end_time_text, text="")
    canvas.itemconfig(speed_text, text="")

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
    global lane1_last_seen, lane2_last_seen

    try:
        ser = serial.Serial(PORT, BAUDRATE, timeout=1)
        print(f"Connected to {PORT}")

        while True:
            line = ser.readline().decode(errors="ignore").strip()
            if not line:
                continue

            print("SERIAL:", line)
            u = line.upper()

            if u.startswith("END TIME:"):
                current_end_time = line.split(":")[1].strip()

            elif u.startswith("KPH:"):
                current_kph = line.split(":")[1].strip()

            elif "PRE STAGED 1" in u:
                lane1_last_seen = time.time()
                canvas.after(0, update_lane, 1, "PRE-STAGED", "#00aa00")

            elif "PRE STAGED 2" in u:
                lane2_last_seen = time.time()
                canvas.after(0, update_lane, 2, "PRE-STAGED", "#00aa00")

            elif "READY" in u:
                lane1_last_seen = 0
                lane2_last_seen = 0
                canvas.after(0, update_title, "READY", "orange")
                canvas.after(0, update_lane, 1, "READY", "#ff4600")
                canvas.after(0, update_lane, 2, "READY", "#ff4600")

            elif "SET" in u:
                canvas.after(0, update_title, "SET", "orange")
                canvas.after(0, update_lane, 1, "SET", "#bf3400")
                canvas.after(0, update_lane, 2, "SET", "#bf3400")
                reset_gui(5)

            elif "LANE1" in u and "STAGED" in u:
                canvas.after(0, update_lane, 1, "STAGED", "#00aaff")

            elif "LANE2" in u and "STAGED" in u:
                canvas.after(0, update_lane, 2, "STAGED", "#00aaff")

            elif u.strip().endswith(("3", "2", "1")):
                num = u.strip()[-1]
                canvas.after(0, update_title, num, "orange")

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

            elif "GOOD RUN" in u:
                canvas.after(0, update_title, "GOOD RUN", "#00ff00")
                canvas.after(0, update_lane, 1, "GOOD RUN", "#00aa00")
                canvas.after(0, update_lane, 2, "GOOD RUN", "#00aa00")

            elif "LANE 1 WINS" in u:
                canvas.after(0, update_title, "FINISH", "white")
                canvas.after(0, update_lane, 1, "WINNER", "#00ff00")
                canvas.after(0, update_lane, 2, "LOSE", "#ff0000")
                canvas.after(0, show_end_time, current_end_time)
                canvas.after(0, show_speed, current_kph)
                reset_gui(10)

            elif "LANE 2 WINS" in u:
                canvas.after(0, update_title, "FINISH", "white")
                canvas.after(0, update_lane, 2, "WINNER", "#00ff00")
                canvas.after(0, update_lane, 1, "LOSE", "#ff0000")
                canvas.after(0, show_end_time, current_end_time)
                canvas.after(0, show_speed, current_kph)
                reset_gui(10)

            elif "DOUBLE WINNER" in u:
                canvas.after(0, update_title, "TIE", "white")
                canvas.after(0, update_lane, 1, "TIE", "#00aaff")
                canvas.after(0, update_lane, 2, "TIE", "#00aaff")
                canvas.after(0, show_end_time, current_end_time)
                canvas.after(0, show_speed, current_kph)
                reset_gui(10)

    except Exception as e:
        print("Serial Error:", e)

# ===================== START =====================
threading.Thread(target=serial_listener, daemon=True).start()
manual_reset()
prestage_watchdog()
root.mainloop()
