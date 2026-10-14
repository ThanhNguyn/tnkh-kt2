"""
BAI 3 - PLOT REALTIME SO SANH gia tri THO vs DA LOC (trung binh truot) tu ADC
-----------------------------------------------------------------------------
Doc du lieu Serial tu board chay vd1_adc1_loc_ledc.cpp (dang ">ten:gia_tri")
va hien thi 2 CON SO LON, DE DOC:

    tho   -> gia tri ADC tho moi lan analogRead() (0-4095), nhay loan vi nhieu
    loc   -> gia tri sau khi loc trung binh truot (N=8 mau), muot hon han

Muc dich su pham: cho SV thay ro bang mat, du khong dong vao bien tro, gia
tri THO van dao dong vai don vi do nhieu dien tu/ADC noi; gia tri DA LOC
bam sat xu huong that va khong giat.

Phia duoi la do thi 2 duong tho (do, dao dong manh) va loc (xanh, muot)
theo thoi gian de thay ro hieu qua loc ngay khi no xuat hien.

Cach dung:
    python plot_serial_vd3.py COM7
    python plot_serial_vd3.py COM7 --baud 115200 --window 8

Dung CHUNG script nay cho ca vd2_adc2_strapping.cpp (cung dinh dang
">tho:gia_tri") de so sanh do nhay loan/dung cua ADC2 so voi ADC1.

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
    p = argparse.ArgumentParser(description="Plot realtime tho vs loc (ADC) tu ESP32 (bai3)")
    p.add_argument("port", help="Cong COM cua ESP32, vi du COM7")
    p.add_argument("--baud", type=int, default=115200, help="Baud rate (mac dinh 115200)")
    p.add_argument("--window", type=float, default=8.0, help="So giay hien thi tren truc thoi gian (mac dinh 8s)")
    return p.parse_args()


def main():
    args = parse_args()

    ser = serial.Serial(args.port, args.baud, timeout=0.05)
    time.sleep(2)  # cho ESP32 reset xong sau khi mo cong serial (DTR toggling)
    ser.reset_input_buffer()

    start_time = time.time()
    MAXLEN = 50000

    tho_t, tho_v = deque(maxlen=MAXLEN), deque(maxlen=MAXLEN)
    loc_t, loc_v = deque(maxlen=MAXLEN), deque(maxlen=MAXLEN)

    state = {"tho": 0, "loc": 0, "wifiBat": None}

    # ----------- Layout: 2 so to ben tren, do thi nho ben duoi -----------
    fig = plt.figure(figsize=(11, 7.5))
    fig.suptitle(f"BAI 3 — ADC tho vs da loc (trung binh truot) — {args.port} @ {args.baud} baud",
                 fontsize=13, fontweight="bold")

    gs = fig.add_gridspec(3, 12, height_ratios=[1.3, 1.3, 2], hspace=0.45, wspace=0.6)

    ax_tho_num = fig.add_subplot(gs[0, 0:6])
    ax_loc_num = fig.add_subplot(gs[0, 6:12])
    ax_info = fig.add_subplot(gs[1, :])
    ax_plot = fig.add_subplot(gs[2, 1:11])

    for ax in (ax_tho_num, ax_loc_num, ax_info):
        ax.axis("off")

    ax_tho_num.set_title("tho (ADC THO, 0-4095)", fontsize=12, color="#c15b3a", fontweight="bold")
    tho_num_text = ax_tho_num.text(0.5, 0.45, "0", ha="center", va="center",
                                    fontsize=64, fontweight="bold", color="#c15b3a",
                                    family="monospace", transform=ax_tho_num.transAxes)

    ax_loc_num.set_title("loc (TRUNG BINH TRUOT, N=8)", fontsize=12, color="#3a7d5c", fontweight="bold")
    loc_num_text = ax_loc_num.text(0.5, 0.45, "0", ha="center", va="center",
                                    fontsize=64, fontweight="bold", color="#3a7d5c",
                                    family="monospace", transform=ax_loc_num.transAxes)

    info_text = ax_info.text(0.5, 0.5, "Dang doi du lieu tu board... quay bien tro de xem thay doi.",
                              ha="center", va="center", fontsize=14, family="monospace",
                              transform=ax_info.transAxes,
                              bbox=dict(boxstyle="round,pad=0.6", facecolor="#f0d9cd", edgecolor="#c15b3a"))

    ax_plot.set_xlabel("thoi gian (s)")
    ax_plot.set_ylabel("gia tri ADC (0-4095)")
    ax_plot.grid(True, alpha=0.3)
    (tho_line,) = ax_plot.plot([], [], color="#c15b3a", linewidth=1.3, alpha=0.85, label="tho (chua loc)")
    (loc_line,) = ax_plot.plot([], [], color="#3a7d5c", linewidth=2.4, label="loc (da loc)")
    ax_plot.legend(loc="upper left")

    def read_serial():
        while ser.in_waiting:
            line = ser.readline().decode(errors="ignore").strip()
            m = LINE_RE.match(line)
            if not m:
                continue
            name, value = m.group(1), int(m.group(2))
            t = time.time() - start_time

            if name == "tho":
                tho_t.append(t)
                tho_v.append(value)
                state["tho"] = value
            elif name == "loc":
                loc_t.append(t)
                loc_v.append(value)
                state["loc"] = value
            elif name == "wifiBat":
                state["wifiBat"] = value

    def update(_frame):
        read_serial()

        tho_num_text.set_text(str(state["tho"]))
        loc_num_text.set_text(str(state["loc"]) if state["loc"] else str(state["tho"]))

        if state["wifiBat"] == 1:
            info_text.set_text("Wi-Fi DA BAT — neu dang chay tren GPIO12 (ADC2), gia tri co the SAI/HANG.\n"
                                "So sanh voi ADC1 (GPIO34) van doc on dinh trong cung dieu kien.")
            info_text.get_bbox_patch().set_facecolor("#f4cccc")
            info_text.get_bbox_patch().set_edgecolor("#a33")
        elif tho_v:
            do_nhay = max(list(tho_v)[-50:]) - min(list(tho_v)[-50:]) if len(tho_v) >= 2 else 0
            info_text.set_text(f"Do nhay (max-min) cua TIN THO trong ~1s gan nhat: {do_nhay}  "
                                f"(cang nho cang chung to loc hieu qua)")
            info_text.get_bbox_patch().set_facecolor("#d9ead9")
            info_text.get_bbox_patch().set_edgecolor("#3a7d5c")

        now = max(tho_t[-1] if tho_t else 0, loc_t[-1] if loc_t else 0)
        if tho_t:
            tho_line.set_data(tho_t, tho_v)
        if loc_t:
            loc_line.set_data(loc_t, loc_v)
        ax_plot.set_xlim(max(0, now - args.window), now + 0.1)
        all_vals = list(tho_v) + list(loc_v)
        if all_vals:
            ax_plot.set_ylim(max(0, min(all_vals) - 50), min(4095, max(all_vals) + 50))

        return [tho_num_text, loc_num_text, info_text, tho_line, loc_line]

    ani = animation.FuncAnimation(fig, update, interval=20, blit=False, cache_frame_data=False)
    plt.tight_layout()
    try:
        plt.show()
    finally:
        ser.close()


if __name__ == "__main__":
    main()
