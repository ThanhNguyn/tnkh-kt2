#!/usr/bin/env python3
"""Verify the TH5 MQTT stream and produce evidence for the offline test.

Subscribes to the device topics and reports:
  * every online/offline transition and how long the device was offline,
  * live vs. replayed (buffered) samples,
  * duplicates and missing sequence numbers per boot.

Usage:
    pip install -r tools/requirements.txt
    python tools/stream_monitor.py --duration 180
    python tools/stream_monitor.py --root tnkh/th5/esp32-25112107 --duration 300
"""
import argparse
import json
import sys
import time


class StreamMonitor:
    """Pure logic (no network) so it can be unit-tested."""

    def __init__(self):
        self.live = 0
        self.replayed = 0
        self.duplicates = 0
        self.invalid = 0
        self._seen = set()
        self._seqs = {}  # boot -> set of seq
        self.transitions = []  # (state, time)
        self.offline_durations = []
        self._offline_since = None

    def on_sensor(self, payload, now):
        try:
            boot, seq = int(payload["boot"]), int(payload["seq"])
            float(payload["t"]), float(payload["h"])
        except (KeyError, TypeError, ValueError):
            self.invalid += 1
            return "invalid"
        key = (boot, seq)
        if key in self._seen:
            self.duplicates += 1
            return "duplicate"
        self._seen.add(key)
        self._seqs.setdefault(boot, set()).add(seq)
        if payload.get("replayed"):
            self.replayed += 1
            return "replayed"
        self.live += 1
        return "live"

    def on_status(self, state, now):
        state = state.strip().lower()
        if state not in ("online", "offline"):
            return
        if self.transitions and self.transitions[-1][0] == state:
            return
        self.transitions.append((state, now))
        if state == "offline":
            self._offline_since = now
        elif self._offline_since is not None:
            self.offline_durations.append(now - self._offline_since)
            self._offline_since = None

    def summary(self):
        boots = {}
        for boot, seqs in self._seqs.items():
            lo, hi = min(seqs), max(seqs)
            boots[boot] = {
                "first_seq": lo,
                "last_seq": hi,
                "received": len(seqs),
                "missing": (hi - lo + 1) - len(seqs),
            }
        return {
            "live": self.live,
            "replayed": self.replayed,
            "duplicates": self.duplicates,
            "invalid": self.invalid,
            "offline_events": sum(1 for s, _ in self.transitions if s == "offline"),
            "offline_durations_s": [round(d, 1) for d in self.offline_durations],
            "boots": boots,
        }


def run(args):
    try:
        import paho.mqtt.client as mqtt
    except ImportError:
        sys.exit("paho-mqtt is missing: pip install -r tools/requirements.txt")

    mon = StreamMonitor()
    sensor_topic = f"{args.root}/sensor"
    status_topic = f"{args.root}/status"
    t0 = time.time()

    def log(msg):
        print(f"[{time.time() - t0:7.1f}s] {msg}", flush=True)

    def on_connect(client, userdata, flags, rc, *extra):
        log(f"connected to {args.host}:{args.port} (rc={rc})")
        client.subscribe([(sensor_topic, 0), (status_topic, 1)])

    def on_message(client, userdata, msg):
        now = time.time()
        text = msg.payload.decode("utf-8", "replace")
        if msg.topic == status_topic:
            mon.on_status(text, now)
            log(f"STATUS {text}")
            return
        try:
            data = json.loads(text)
        except json.JSONDecodeError:
            mon.invalid += 1
            log(f"invalid JSON: {text[:60]}")
            return
        kind = mon.on_sensor(data, now)
        if kind != "live" or args.verbose:
            log(f"{kind.upper():9} boot={data.get('boot')} seq={data.get('seq')} "
                f"t={data.get('t')} h={data.get('h')}")

    try:
        client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
    except AttributeError:  # paho-mqtt 1.x
        client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message
    client.connect(args.host, args.port, keepalive=30)
    client.loop_start()
    try:
        time.sleep(args.duration)
    except KeyboardInterrupt:
        pass
    client.loop_stop()
    client.disconnect()

    print("\n=== summary ===")
    print(json.dumps(mon.summary(), indent=2))


def main():
    p = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    p.add_argument("--host", default="broker.hivemq.com")
    p.add_argument("--port", type=int, default=1883)
    p.add_argument("--root", default="tnkh/th5/esp32", help="TOPIC_ROOT from config.h")
    p.add_argument("--duration", type=int, default=120, help="seconds to listen")
    p.add_argument("-v", "--verbose", action="store_true", help="log live samples too")
    run(p.parse_args())


if __name__ == "__main__":
    main()
