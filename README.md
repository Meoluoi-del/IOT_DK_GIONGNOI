# IOT_DK_GIONGNOI - Điều khiển thiết bị IoT bằng giọng nói

Dự án thực hành IoT: Mô phỏng ESP32 trên Wokwi, kết nối Blynk & ThingSpeak, điều khiển đèn/quạt bằng giọng nói tiếng Việt qua Web Server Python.

## Cấu trúc thư mục

- `giong-noi/`: Web server Python và giao diện nhận diện giọng nói (Web Speech API).
  - `server.py`: Bridge server nhận diện giọng nói và chuyển lệnh sang Blynk API.
  - `index.html`: Giao diện web thu âm và gửi giọng nói.
- `wokwi/`: Mã nguồn mô phỏng ESP32 trên Wokwi.
  - `sketch.ino`: Mã nguồn chính cho ESP32.
  - `config.h`: File cấu hình Blynk Template ID, Token, ThingSpeak Channel ID, Write API Key.
  - `libraries.txt`: Danh sách thư viện Wokwi cần cài đặt.
  - `thingspeak_ca.h`: Chứng chỉ SSL CA của ThingSpeak.
- `HUONG_DAN.md`: Tài liệu hướng dẫn chi tiết từng bước cài đặt, đấu nối và kiểm thử.
