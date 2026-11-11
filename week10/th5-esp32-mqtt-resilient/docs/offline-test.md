# Offline / auto-reconnect test

Goal: show that the device detects a network loss, keeps sampling, reconnects
by itself, and that no data is lost or duplicated.

## Preparation

1. Firmware running (Wokwi or real board) and Node-RED dashboard open.
2. In a terminal, start the verifier (leave it running for the whole test):

   ```bash
   pip install -r tools/requirements.txt
   python tools/stream_monitor.py --duration 240
   ```

   Use `--root <your TOPIC_ROOT>` if you changed it in `src/config.h`.

## Procedure

| Step | Action | Expected behaviour |
|---|---|---|
| 1 | Let the system run for 30 s | Dashboard updates every 2 s, status **ONLINE** |
| 2 | Press the button on GPIO15 (or send `o` in the Serial Monitor) | Serial: `[sim] network outage START`; broker publishes Last Will, dashboard shows **OFFLINE** |
| 3 | Wait 30 s | Serial: `[buf] seq=... (queued=N)`; N grows by 1 every 2 s |
| 4 | Press the button again | Serial: `[sim] network outage END`, `[mqtt] reconnected after ... ms` |
| 5 | Watch for about 20 s | Status **ONLINE**; queued samples replayed (`replayed:true`); live data resumes |
| 6 | Repeat steps 2-5 two more times (3 cycles in total) | Same behaviour every time |
| 7 | Stop the verifier with Ctrl+C | Summary is printed |

For a real board you can replace steps 2 and 4 by switching the Wi-Fi hotspot
off and on.

## Pass criteria

- [ ] Device status goes `offline` then `online` in every cycle.
- [ ] `missing` is `0` for the current boot in the verifier summary.
- [ ] `duplicates` is `0` (or duplicates were dropped by Node-RED as designed).
- [ ] `replayed` is roughly (outage seconds / 2) per cycle.
- [ ] The device reconnects without a reset or manual action.

## Results (fill in after running)

| Cycle | Outage length (s) | Time to reconnect (s) | Samples buffered | Samples replayed | Missing |
|---|---|---|---|---|---|
| 1 | | | | | |
| 2 | | | | | |
| 3 | | | | | |

Paste the final verifier summary here:

```
(paste output of tools/stream_monitor.py)
```

## Notes

- The buffer holds 120 samples (about 4 minutes at a 2 s period). Longer
  outages drop the oldest samples; the count is exposed as `dropped` in
  `GET /api/status`.
- Samples taken before the NTP clock is synced carry `ts: 0`; Node-RED then
  uses the arrival time for them.
