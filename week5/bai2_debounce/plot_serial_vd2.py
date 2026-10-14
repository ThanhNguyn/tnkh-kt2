"""
BAI 2 - VI DU 2: PLOT REALTIME SO SANH rawEdgeCount vs debouncedCount
-------------------------------------------------------------------
Doc du lieu Serial tu board chay vd2_debounce.cpp (dang ">ten:gia_tri")
va hien thi 2 CON SO LON, DE DOC:

    rawEdgeCount    -> tong so suon xuong THO ma ISR nhan duoc (chua loc)
    debouncedCount  -> so lan nhan THUC SU sau khi da chong doi 50ms

Muc dich su pham: cho SV thay ro bang mat, du 1 lan nhan co the tao ra
nhieu suon xuong (rawEdgeCount tang nhieu), debouncedCount VAN CHI TANG
DUNG 1 -> chung minh thuat toan chong doi hoat dong dung.

Phia duoi la do thi 2 duong rawEdgeCount (do, duong tren) va
debouncedCount (xanh, duong duoi) theo thoi gian de thay khoang cach
giua 2 duong (= so suon da bi loc bo) ngay khi no xuat hien.

Cach dung:
    python plot_serial_vd2.py COM7
    python plot_serial_vd2.py COM7 --baud 115200 --window 5

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
    p = argparse.ArgumentParser(description="Plot realtime rawEdgeCount vs debouncedCount tu ESP32 (vd2_debounce)")
    p.add_argument("port", help="Cong COM cua ESP32, vi du COM7")
    p.add_argument("--baud", type=int, default=115200, help="Baud rate (mac dinh 115200)")
    p.add_argument("--window", type=float, default=5.0, help="So giay hien thi tren truc thoi gian (mac dinh 5s)")
    return p.parse_args()


def main():
    args = parse_args()

    ser = serial.Serial(args.port, args.baud, timeout=0.05)
    time.sleep(2)  # cho ESP32 reset xong sau khi mo cong serial (DTR toggling)
    ser.reset_input_buffer()

    start_time = time.time()
    MAXLEN = 50000

    raw_t, raw_v = deque(maxlen=MAXLEN), deque(maxlen=MAXLEN)
    deb_t, deb_v = deque(maxlen=MAXLEN), deque(maxlen=MAXLEN)

    state = {
        "raw": 0, "deb": 0,
        "raw_delta_last_press": 0,   # so suon tho trong lan nhan gan nhat
        "deb_delta_last_press": 0,   # luon la 0 hoac 1 neu debounce dung
        "filtered_last_press": 0,    # = raw_delta - deb_delta: so suon da bi loc
        "press_events": 0,           # tong so lan nhan (theo debouncedCount) tu luc chay script
    }
    prev_raw = None
    prev_deb = None

    # ----------- Layout: 2 so to ben tren, do thi nho ben duoi -----------
    fig = plt.figure(figsize=(11, 7.5))
    fig.suptitle(f"VD2 — Chong doi (debounce 50ms) — {args.port} @ {args.baud} baud", fontsize=13, fontweight="bold")

    gs = fig.add_gridspec(3, 12, height_ratios=[1.3, 1.3, 2], hspace=0.45, wspace=0.6)

    ax_raw_num = fig.add_subplot(gs[0, 0:6])
    ax_deb_num = fig.add_subplot(gs[0, 6:12])
    ax_info = fig.add_subplot(gs[1, :])
    ax_plot = fig.add_subplot(gs[2, 1:11])

    for ax in (ax_raw_num, ax_deb_num, ax_info):
        ax.axis("off")

    ax_raw_num.set_title("rawEdgeCount (suon xuong THO)", fontsize=12, color="#c15b3a", fontweight="bold")
    raw_num_text = ax_raw_num.text(0.5, 0.45, "0", ha="center", va="center",
                                    fontsize=64, fontweight="bold", color="#c15b3a",
                                    family="monospace", transform=ax_raw_num.transAxes)

    ax_deb_num.set_title("debouncedCount (lan nhan THUC)", fontsize=12, color="#3a7d5c", fontweight="bold")
    deb_num_text = ax_deb_num.text(0.5, 0.45, "0", ha="center", va="center",
                                    fontsize=64, fontweight="bold", color="#3a7d5c",
                                    family="monospace", transform=ax_deb_num.transAxes)

    info_text = ax_info.text(0.5, 0.5, "Nhan nut de bat dau quan sat...", ha="center", va="center",
                              fontsize=15, family="monospace", transform=ax_info.transAxes,
                              bbox=dict(boxstyle="round,pad=0.6", facecolor="#f0d9cd", edgecolor="#c15b3a"))

    ax_plot.set_xlabel("thoi gian (s)")
    ax_plot.set_ylabel("so lan dem (count)")
    ax_plot.grid(True, alpha=0.3)
    (raw_line,) = ax_plot.plot([], [], drawstyle="steps-post", color="#c15b3a", linewidth=2.2, label="rawEdgeCount (tho)")
    (deb_line,) = ax_plot.plot([], [], drawstyle="steps-post", color="#3a7d5c", linewidth=2.2, label="debouncedCount (da loc)")
    ax_plot.legend(loc="upper left")

    def read_serial():
        nonlocal prev_raw, prev_deb
        while ser.in_waiting:
            line = ser.readline().decode(errors="ignore").strip()
            m = LINE_RE.match(line)
            if not m:
                continue
            name, value = m.group(1), int(m.group(2))
            t = time.time() - start_time

            if name == "rawEdgeCount":
                raw_t.append(t)
                raw_v.append(value)
                if prev_raw is not None and value != prev_raw:
                    state["raw_delta_last_press"] = value - prev_raw
                prev_raw = value
                state["raw"] = value

            elif name == "debouncedCount":
                deb_t.append(t)
                deb_v.append(value)
                if prev_deb is not None and value != prev_deb:
                    state["deb_delta_last_press"] = value - prev_deb
                    state["filtered_last_press"] = state["raw_delta_last_press"] - state["deb_delta_last_press"]
                    state["press_events"] = value
                prev_deb = value
                state["deb"] = value

    def update(_frame):
        read_serial()

        raw_num_text.set_text(str(state["raw"]))
        deb_num_text.set_text(str(state["deb"]))

        if state["press_events"] > 0:
            filtered = max(0, state["filtered_last_press"])
            if filtered > 0:
                msg = (f"Lan nhan #{state['press_events']}:  "
                       f"{state['raw_delta_last_press']} suon xuong tho  ->  CHONG DOI loc bo {filtered} suon  ->  "
                       f"debouncedCount chi +1\n"
                       f"KET LUAN: ISR da dem CHUAN, khong con bi dem dup do nay phim.")
                info_text.set_text(msg)
                info_text.get_bbox_patch().set_facecolor("#d9ead9")
                info_text.get_bbox_patch().set_edgecolor("#3a7d5c")
            else:
                msg = (f"Lan nhan #{state['press_events']}:  "
                       f"{state['raw_delta_last_press']} suon xuong (khong co nay dang ke lan nay)  ->  debouncedCount +1")
                info_text.set_text(msg)
                info_text.get_bbox_patch().set_facecolor("#f0d9cd")
                info_text.get_bbox_patch().set_edgecolor("#c15b3a")

        now = max(raw_t[-1] if raw_t else 0, deb_t[-1] if deb_t else 0)
        if raw_t:
            raw_line.set_data(raw_t, raw_v)
        if deb_t:
            deb_line.set_data(deb_t, deb_v)
        ax_plot.set_xlim(max(0, now - args.window), now + 0.1)
        all_vals = list(raw_v) + list(deb_v)
        if all_vals:
            ax_plot.set_ylim(min(0, min(all_vals) - 1), max(all_vals) + 2)

        return [raw_num_text, deb_num_text, info_text, raw_line, deb_line]

    ani = animation.FuncAnimation(fig, update, interval=20, blit=False, cache_frame_data=False)
    plt.tight_layout()
    try:
        plt.show()
    finally:
        ser.close()


if __name__ == "__main__":
    main()
