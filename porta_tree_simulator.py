import tkinter as tk
from PIL import Image, ImageTk
import threading, time, random, os

# ================ CONFIG ================
BG_IMAGE_FILE = "bg.jpg"   # optional background; will be scaled to window
LANES = 2
FONT_NAME = "Arial"
RESET_DELAY = 3
WINDOW_WIDTH = 1200
WINDOW_HEIGHT = 700
# ========================================

root = tk.Tk()
root.title("Porta Tree Simulator")
root.geometry(f"{WINDOW_WIDTH}x{WINDOW_HEIGHT}")
root.minsize(800, 500)
root.configure(bg="black")

# Canvas - everything will be drawn here so layering is stable
canvas = tk.Canvas(root, bg="black", highlightthickness=0)
canvas.pack(fill="both", expand=True)

# Globals
bg_photo = None           # scaled PhotoImage (kept referenced)
lane_boxes = []
lane_labels = []
lane_timers = []
lane_times = [0.0] * LANES
lane_running = [False] * LANES
race_started = False
porta_label = None
blink_flags = {}
stats_text = None
start_button_window = None
bg_image_orig = None

# Try load original bg image (kept for resizing)
if os.path.exists(BG_IMAGE_FILE):
    try:
        bg_image_orig = Image.open(BG_IMAGE_FILE).convert("RGBA")
    except Exception as e:
        print("⚠️ Failed to open bg image:", e)
        bg_image_orig = None
else:
    bg_image_orig = None

# ---------- Utilities ----------
def safe_itemcget(item, option):
    try:
        return canvas.itemcget(item, option)
    except tk.TclError:
        return ""

def blink_label(item_id, color_on, color_off, interval=500):
    # toggles between color_on and color_off while blink_flags[item_id] is True
    if item_id not in blink_flags:
        blink_flags[item_id] = True
    if not blink_flags.get(item_id, False):
        return
    current = safe_itemcget(item_id, "fill")
    new = color_on if current == color_off else color_off
    try:
        canvas.itemconfig(item_id, fill=new)
    except tk.TclError:
        return
    root.after(interval, blink_label, item_id, color_on, color_off, interval)

def stop_blink(item_id):
    blink_flags[item_id] = False

# ---------- Drawing / Layout ----------
def draw_background(w, h):
    global bg_photo
    canvas.delete("bg_image")  # remove previous background image item (if any)
    if bg_image_orig is None:
        return
    # resize original image to current canvas size while preserving aspect cover
    try:
        img = bg_image_orig.resize((w, h), Image.LANCZOS)
        bg_photo = ImageTk.PhotoImage(img)
        canvas.create_image(0, 0, anchor="nw", image=bg_photo, tags="bg_image")
    except Exception as e:
        print("⚠️ Failed to draw background:", e)

def clear_lane_items():
    for item in lane_boxes + lane_labels + lane_timers:
        try:
            canvas.delete(item)
        except Exception:
            pass
    lane_boxes.clear()
    lane_labels.clear()
    lane_timers.clear()

def create_ui():
    global porta_label, stats_text, start_button_window
    # clear previous (useful on resize)
    canvas.delete("ui")
    clear_lane_items()

    w = canvas.winfo_width() or WINDOW_WIDTH
    h = canvas.winfo_height() or WINDOW_HEIGHT

    # draw background first
    draw_background(w, h)

    # Porta Tree label (top center)
    porta_label = canvas.create_text(w/2, h*0.09, text="PORTA TREE READY",
                                     font=(FONT_NAME, 28, "bold"), fill="lime", tags="ui")
    blink_label(porta_label, "lime", "black")

    # Lanes centered horizontally
    spacing = w // (LANES + 1)
    lane_y = int(h * 0.42)
    box_w = 350
    box_h = 180

    for i in range(LANES):
        x = (i + 1) * spacing
        box = canvas.create_rectangle(x - box_w//2, lane_y - box_h//2,
                                      x + box_w//2, lane_y + box_h//2,
                                      fill="#000000", outline="white", width=2, tags="ui")
        lane_boxes.append(box)

        label = canvas.create_text(x, lane_y - box_h//2 + 30,
                                   text=f"LANE {i+1}", font=(FONT_NAME, 22, "bold"),
                                   fill="white", tags="ui")
        lane_labels.append(label)

        timer = canvas.create_text(x, lane_y + 20, text="PLS STAGE",
                                   font=(FONT_NAME, 20, "bold"), fill="cyan", tags="ui")
        lane_timers.append(timer)

    # Stats panel (below lanes)
    stats_box_top = int(h * 0.7)
    stats_box_left = int(w*0.25)
    stats_box_right = int(w*0.75)
    stats_box_bottom = stats_box_top + 120
    canvas.create_rectangle(stats_box_left, stats_box_top, stats_box_right, stats_box_bottom,
                            fill="#111111", outline="white", width=2, tags="ui")
    stats_text = canvas.create_text(w/2, stats_box_top + 55, text="No stats yet",
                                    font=(FONT_NAME, 16, "bold"), fill="white", tags="ui")

    # Start button (placed via canvas window so it layers above bg and sits in a stable spot)
    # Create a real tk.Button and place it on canvas
    start_btn = tk.Button(root, text="🧪 START SIMULATION", command=start_simulation,
                          bg="lime", fg="black", font=(FONT_NAME, 14, "bold"))
    # remove any old button window
    if start_button_window:
        try:
            canvas.delete(start_button_window)
        except Exception:
            pass
    # place button centered below stats box
    start_button_window = canvas.create_window(w/2, stats_box_bottom + 28, window=start_btn, tags="ui")
    # Keep reference so the button doesn't get garbage-collected (not strictly necessary here)
    canvas.start_btn_ref = start_btn

# ---------- Resize handler ----------
def on_resize(event):
    # Recreate UI when canvas size changes to keep things centered.
    # Use a short debounce by scheduling create_ui via after_cancel/after if needed.
    # Simple immediate redraw is fine for modest resizing.
    create_ui()

# ---------- Logic ----------
def update_lane(lane, text, color="white", blink=False):
    try:
        canvas.itemconfig(lane_timers[lane], text=text, fill=color)
    except Exception:
        return
    stop_blink(lane_timers[lane])
    if blink:
        blink_label(lane_timers[lane], color, "black")

def reset_ui():
    for i in range(LANES):
        update_lane(i, "PLS STAGE", "cyan", blink=True)
        try:
            canvas.itemconfig(lane_boxes[i], fill="#000000")
        except Exception:
            pass
        lane_times[i] = 0.0
        lane_running[i] = False
    try:
        canvas.itemconfig(porta_label, text="PORTA TREE READY", fill="lime")
        blink_label(porta_label, "lime", "black")
        canvas.itemconfig(stats_text, text="No stats yet", fill="white")
    except Exception:
        pass

def simulate_race_thread():
    global race_started
    race_started = True
    # update porta label
    try:
        canvas.itemconfig(porta_label, text="RACE STARTING...", fill="yellow")
        stop_blink(porta_label)
    except Exception:
        pass

    # PLS STAGE
    for i in range(LANES):
        update_lane(i, "PLS STAGE", "cyan", blink=True)
    time.sleep(1.5)

    # PRE STAGE
    for i in range(LANES):
        update_lane(i, "PRE STAGE", "orange", blink=True)
    time.sleep(1.5)

    # GO
    try:
        canvas.itemconfig(porta_label, text="GO!", fill="lime")
        blink_label(porta_label, "lime", "black")
    except Exception:
        pass

    for i in range(LANES):
        lane_running[i] = True
        lane_times[i] = time.time()

    # Running timer loop
    while any(lane_running):
        now = time.time()
        for i in range(LANES):
            if lane_running[i]:
                elapsed = now - lane_times[i]
                try:
                    canvas.itemconfig(lane_timers[i], text=f"{elapsed:.2f}s", fill="white")
                except Exception:
                    pass
                if elapsed >= random.uniform(2.5, 5.0):
                    lane_running[i] = False
                    lane_times[i] = elapsed
        try:
            root.update()
        except tk.TclError:
            race_started = False
            return
        time.sleep(0.04)

    # Determine winner
    valid_times = [t if t > 0 else float('inf') for t in lane_times]
    winner_time = min(valid_times)
    if winner_time == float('inf'):
        winner_index = 0
    else:
        winner_index = valid_times.index(winner_time)

    for i in range(LANES):
        t = lane_times[i]
        if i == winner_index:
            update_lane(i, f"WINNER {t:.2f}s", "green", blink=True)
            canvas.itemconfig(lane_boxes[i], fill="#008000")
        else:
            update_lane(i, f"LOSS {t:.2f}s", "red", blink=True)
            canvas.itemconfig(lane_boxes[i], fill="#800000")

    # Update stats area
    stats_lines = [
        f"🏁 Winner: LANE {winner_index + 1}   |   ⏱ {lane_times[winner_index]:.2f}s",
        f"Lane 1: {lane_times[0]:.2f}s    Lane 2: {lane_times[1]:.2f}s"
    ]
    try:
        canvas.itemconfig(stats_text, text="\n".join(stats_lines), fill="lime")
    except Exception:
        pass

    # Pause and reset
    try:
        canvas.itemconfig(porta_label, text="RESETTING...", fill="red")
    except Exception:
        pass
    time.sleep(RESET_DELAY)
    reset_ui()
    race_started = False

def start_simulation():
    global race_started
    if race_started:
        return
    threading.Thread(target=simulate_race_thread, daemon=True).start()

# ---------- Init ----------
canvas.bind("<Configure>", on_resize)
# ensure UI is created after mainloop calculates sizes
root.after(100, create_ui)
root.after(120, reset_ui)

root.mainloop()
