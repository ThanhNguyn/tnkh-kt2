# Demo media checklist

Record these after running the project, then put the files in this folder
(or link them from the README if they are too large for the repository).

| File | What it must show |
|---|---|
| `dashboard.png` | Node-RED dashboard with gauges, chart, device status **ONLINE** |
| `serial-offline.png` | Serial Monitor: `[sim] network outage START`, `[buf] ...` lines, retry/backoff lines |
| `serial-recovered.png` | `[mqtt] reconnected after ... ms, N samples buffered` and the replay |
| `dashboard-offline.png` | Dashboard showing device status **OFFLINE** during the outage |
| `demo.mp4` | 30-60 s clip: normal run -> outage -> OFFLINE -> recovery -> data back |

Tip: Wokwi has a built-in screen recorder, and Node-RED's dashboard can be
captured with the OS screenshot tool. Keep the Serial Monitor and dashboard
visible side by side for the video.
