# Báo cáo TH5: Mở rộng lưu lịch sử và cảnh báo ngưỡng

**Hướng mở rộng đã chọn:** lưu lịch sử đo và cảnh báo vượt ngưỡng nhiệt độ.

Hệ thống TH5 truyền dữ liệu DHT22 từ ESP32 qua broker HiveMQ đến dashboard Node-RED. Khi mất mạng, thiết bị tự kết nối lại bằng backoff tăng dần, broker công bố trạng thái offline nhờ Last Will, và các mẫu đo trong thời gian gián đoạn được đệm trong bộ nhớ rồi gửi bù khi có mạng trở lại. Tuy nhiên dashboard chỉ hiển thị dữ liệu tức thời, nên tôi mở rộng thêm hai phần.

Thứ nhất, Node-RED ghi mỗi mẫu vào tệp CSV gồm thời gian, nhiệt độ, độ ẩm và cờ "gửi bù". Vì mỗi mẫu mang số thứ tự cùng mã khởi động, Node-RED loại được bản trùng khi thiết bị gửi lại, nên lịch sử không bị lặp sau sự cố mạng.

Thứ hai, dashboard hiển thị cảnh báo khi nhiệt độ vượt 35 °C. Chỉ các mẫu trực tiếp mới được xét, để dữ liệu cũ gửi bù không gây báo động giả. Cả hai phần chỉ cần cấu hình Node-RED, không đổi giao thức MQTT của thiết bị.

Hạn chế hiện tại là tệp CSV khó truy vấn và thời gian của thiết bị phụ thuộc NTP. Bước tiếp theo là chuyển sang InfluxDB, gửi cảnh báo qua Telegram và bổ sung xác thực TLS cho MQTT.
