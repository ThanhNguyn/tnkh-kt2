import unittest

from stream_monitor import StreamMonitor


def sample(boot, seq, replayed=False):
    return {"boot": boot, "seq": seq, "ts": 0, "t": 25.0, "h": 50.0, "replayed": replayed}


class StreamMonitorTest(unittest.TestCase):
    def test_live_and_replayed_are_counted(self):
        m = StreamMonitor()
        self.assertEqual(m.on_sensor(sample(1, 1), 0), "live")
        self.assertEqual(m.on_sensor(sample(1, 2, True), 1), "replayed")
        s = m.summary()
        self.assertEqual((s["live"], s["replayed"]), (1, 1))

    def test_duplicates_are_detected(self):
        m = StreamMonitor()
        m.on_sensor(sample(1, 1), 0)
        self.assertEqual(m.on_sensor(sample(1, 1), 1), "duplicate")
        self.assertEqual(m.summary()["duplicates"], 1)

    def test_same_seq_after_reboot_is_not_a_duplicate(self):
        m = StreamMonitor()
        m.on_sensor(sample(1, 1), 0)
        self.assertEqual(m.on_sensor(sample(2, 1), 1), "live")

    def test_missing_sequence_numbers(self):
        m = StreamMonitor()
        for seq in (1, 2, 5):
            m.on_sensor(sample(7, seq), 0)
        boot = m.summary()["boots"][7]
        self.assertEqual((boot["first_seq"], boot["last_seq"], boot["missing"]), (1, 5, 2))

    def test_replay_fills_the_gap(self):
        m = StreamMonitor()
        for seq in (1, 2, 5, 6):
            m.on_sensor(sample(7, seq), 0)
        for seq in (3, 4):
            m.on_sensor(sample(7, seq, True), 1)
        self.assertEqual(m.summary()["boots"][7]["missing"], 0)

    def test_offline_duration(self):
        m = StreamMonitor()
        m.on_status("online", 10)
        m.on_status("offline", 20)
        m.on_status("offline", 25)  # repeated state is ignored
        m.on_status("online", 50)
        s = m.summary()
        self.assertEqual(s["offline_events"], 1)
        self.assertEqual(s["offline_durations_s"], [30.0])

    def test_invalid_payloads(self):
        m = StreamMonitor()
        self.assertEqual(m.on_sensor({"seq": 1}, 0), "invalid")
        self.assertEqual(m.on_sensor({"boot": 1, "seq": "x", "t": 1, "h": 1}, 0), "invalid")
        self.assertEqual(m.summary()["invalid"], 2)


if __name__ == "__main__":
    unittest.main()
