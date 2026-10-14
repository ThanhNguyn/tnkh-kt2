"""
BAI 2 - PLOT REALTIME TIN HIEU NUT NHAN TU ESP32 (matplotlib)
-------------------------------------------------------------
Doc du lieu Serial gui tu board (dang ">ten:gia_tri") va ve 2 do thi
song song, cap nhat realtime:
  (1) pinState: dang song HIGH/LOW tai chan GPIO4 (giong dao dong ky)
  (2) isrCount / debouncedCount: so lan ISR da kich hoat theo thoi gian

Muc dich: cho du nut vat ly it/khong nay ro ret, van xem duoc CON SO
THAT: moi lan nhan isrCount tang bao nhieu don vi. Neu luon tang dung
1 -> ket luan nut nay tiep diem tot/khong nay dang ke (ket qua hop le,
khong phai loi do). Neu doi khi tang >1 -> da bat duoc hien tuong nay.

Cach dung:
    python plot_serial.py COM7
    python plot_serial.py COM7 --baud 115200 --window 4

Yeu cau: pip install pyserial matplotlib
"""

import argparse
import re
import time
from collections import deque

import serial
import matplotlib.pyplot as plt
import matplotlib.animation as animation

LINE_RE = re.compile(r">(\w+):(-?\d+)")


def parse_args():
    p = argparse.ArgumentParser(description="Plot realtime tin hieu nut nhan + bo dem ISR tu ESP32")
    p.add_argument("port", help="Cong COM cua ESP32, vi du COM7")
    p.add_argument("--baud", type=int, default=115200, help="Baud rate (mac dinh 115200)")
    p.add_argument("--window", type=float, default=4.0, help="So giay hien thi tren truc thoi gian (mac dinh 4s)")
    return p.parse_args()


def main():
    args = parse_args()

    ser = serial.Serial(args.port, args.baud, timeout=0.05)
    time.sleep(2)  # cho ESP32 reset xong sau khi mo cong serial (DTR toggling)
    ser.reset_input_buffer()

    start_time = time.time()
    MAXLEN = 50000

    t_state, v_state = deque(maxlen=MAXLEN), deque(maxlen=MAXLEN)
    count_series = {}   # ten bien dem (isrCount / debouncedCount / rawEdgeCount) -> (deque t, deque v)
    last_count_value = {}
    last_press_delta = {}   # ten bien -> so don vi tang trong lan nhan gan nhat

    fig, (ax_state, ax_count) = plt.subplots(2, 1, figsize=(10, 6.5), sharex=True,
                                              gridspec_kw={"height_ratios": [1, 1.4]})
    fig.suptitle(f"ESP32 - {args.port} @ {args.baud} baud — tin hieu nut & bo dem ngat (ISR)", fontsize=11)

    ax_state.set_ylim(-0.3, 1.3)
    ax_state.set_yticks([0, 1])
    ax_state.set_yticklabels(["LOW\n(dang nhan)", "HIGH\n(nha nut)"])
    ax_state.grid(True, alpha=0.3)
    (state_line,) = ax_state.plot([], [], drawstyle="steps-post", color="#c15b3a", linewidth=2)

    ax_count.set_xlabel("thoi gian (s)")
    ax_count.set_ylabel("so lan dem (count)")
    ax_count.grid(True, alpha=0.3)

    count_lines = {}
    count_colors = {"isrCount": "#c15b3a", "rawEdgeCount": "#c15b3a", "debouncedCount": "#3a7d5c"}

    delta_text = ax_count.text(0.01, 0.95, "", transform=ax_count.transAxes,
                                va="top", ha="left", fontsize=10, family="monospace")

    def get_count_line(name):
        if name not in count_lines:
            color = count_colors.get(name)
            (ln,) = ax_count.plot([], [], drawstyle="steps-post", linewidth=2, label=name, color=color)
            ax_count.legend(loc="upper right")
            count_lines[name] = ln
        return count_lines[name]

    def read_serial():
        while ser.in_waiting:
            raw = ser.readline().decode(errors="ignore").strip()
            m = LINE_RE.match(raw)
            if not m:
                continue
            name, value = m.group(1), int(m.group(2))
            t = time.time() - start_time

            if name.lower() == "pinstate":
                t_state.append(t)
                v_state.append(value)
                continue

            if name not in count_series:
                count_series[name] = (deque(maxlen=MAXLEN), deque(maxlen=MAXLEN))
                last_count_value[name] = value
                last_press_delta[name] = 0

            prev = last_count_value[name]
            if value != prev:
                last_press_delta[name] = value - prev
                last_count_value[name] = value

            ts, vs = count_series[name]
            ts.append(t)
            vs.append(value)

    def update(_frame):
        read_serial()

        now = max(
            t_state[-1] if t_state else 0,
            max((ts[-1] for ts, _ in count_series.values() if ts), default=0),
        )

        if t_state:
            state_line.set_data(t_state, v_state)
        ax_state.set_xlim(max(0, now - args.window), now + 0.1)

        all_vals = []
        for name, (ts, vs) in count_series.items():
            ln = get_count_line(name)
            ln.set_data(ts, vs)
            all_vals.extend(vs)
        if all_vals:
            ax_count.set_ylim(min(0, min(all_vals) - 1), max(all_vals) + 2)
        ax_count.set_xlim(max(0, now - args.window), now + 0.1)

        if last_press_delta:
            lines_txt = [f"{n}: +{d} / lan nhan gan nhat" for n, d in last_press_delta.items()]
            delta_text.set_text("\n".join(lines_txt))

        artists = [state_line, delta_text] + list(count_lines.values())
        return artists

    ani = animation.FuncAnimation(fig, update, interval=20, blit=False, cache_frame_data=False)
    plt.tight_layout()
    try:
        plt.show()
    finally:
        ser.close()


if __name__ == "__main__":
    main()
