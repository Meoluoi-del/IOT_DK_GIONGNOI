# Thực hành IoT với Wokwi Blynk ThingSpeak và giọng nói

Bạn sẽ dựng một mạch ESP32 mô phỏng, đọc DHT22 và cảm biến ánh sáng, điều khiển đèn và relay đại diện cho quạt, lưu dữ liệu trên ThingSpeak rồi gửi lệnh tiếng Việt từ máy tính qua Blynk.

## Các project hiện có của bạn

- [Mạch Wokwi 477223713128462337](https://wokwi.com/projects/477223713128462337): đã có sơ đồ đúng các chân GPIO15, GPIO34, GPIO26, GPIO27 và các file mã mẫu.
- [Template Blynk TMPL6HbyluXzh](https://blynk.cloud/dashboard/1153470/templates/2567226/dashboard): dùng cho các kênh V0–V4 trong mã. Cần đăng nhập để kiểm tra Datastreams và widget thực tế.
- [ThingSpeak channel 3525538](https://thingspeak.mathworks.com/channels/3525538): tên `IoT Giong Noi`, có năm biểu đồ Field 1–5.

`config.h` cục bộ đã đặt `LAB_STAGE 3`, Template ID, Channel ID và máy chủ Blynk vùng `sgp1.blynk.cloud` theo các project này. Hai khóa truy cập vẫn là chỗ trống trong file cục bộ để tránh lưu khóa vào thư mục project. Trong Wokwi bạn đã điền hai khóa; giữ bí mật khi chia sẻ project. ThingSpeak xác định channel đích bằng **Write API Key** trong request; `THINGSPEAK_CHANNEL_ID` trong file chỉ là mã để đối chiếu và hiển thị, không tự xác nhận Write API Key thuộc channel 3525538.

Nếu bạn đang bắt đầu lại từ phần 1, đổi `LAB_STAGE` từ 3 về 1 theo bước bên dưới. Nếu đã kiểm tra mạch, có thể chuyển thẳng đến phần 2 hoặc 3.

### Kết quả kiểm tra kết nối trên project hiện có

Trong phiên mô phỏng, mã với `LAB_STAGE 3` đã biên dịch. ESP32 báo `Ready` với `sgp1.blynk.cloud`, thiết bị Blynk `Dieukhiengiongnoi` hiện Online, và số đo trên trang thiết bị khớp Wokwi (11,2 °C, 46% độ ẩm, 76% ánh sáng tương đối). Hai công tắc V3/V4 làm Serial lần lượt báo đèn và quạt ON/OFF; sau thử nghiệm cả hai được đưa về OFF. ThingSpeak trả `ThingSpeak OK` với Entry ID tăng. Trên chính channel 3525538, ba biểu đồ cảm biến hiển thị 11,2 °C, 46% và 76%; Field 4 và Field 5 là trạng thái đèn/quạt.

Sau khi đăng nhập tài khoản sở hữu Wokwi, bản sửa đã được lưu vào chính project 477223713128462337. Tải lại URL gốc vẫn thấy `LAB_STAGE 3`, máy chủ vùng `sgp1.blynk.cloud` và `Blynk.connect(15000)`; nút Save ở trạng thái không có thay đổi chờ lưu.

Project Wokwi gốc đã được lưu và đường kết nối Blynk/ThingSpeak đã được kiểm tra như trên. Mỗi lần thử điều khiển, bạn vẫn phải bấm chạy mô phỏng và chờ Device Blynk hiện **Online**; khi Wokwi dừng, ESP32 không thể nhận lệnh. Điều khiển giọng nói còn cần chạy `server.py` trên máy tính và mở trang `http://127.0.0.1:8000`.

| Phần | Công việc | Kết quả cần có |
|---|---|---|
| 1 | Dựng mạch, đọc cảm biến, thử đầu ra | Dữ liệu đổi trên Serial; bật/tắt riêng được đèn và relay |
| 2 | Tạo Blynk và nối ESP32 | Online, xem số đo, điều khiển hai công tắc |
| 3 | Tạo ThingSpeak và gửi dữ liệu | Có các bản ghi và biểu đồ mới theo thời gian |
| 4 | Nhận giọng nói tiếng Việt | Nói lệnh trên máy tính và thấy đầu ra thay đổi ở Wokwi |

Giữ lại cùng một project Wokwi xuyên suốt. Hoàn thành tiêu chí kiểm tra mỗi phần trước khi sang phần sau. Mẫu không có chế độ tự động bật đèn/quạt theo ngưỡng; việc đổi số đo không tự làm đầu ra bật lên.

## Phần 1 Dựng mạch và kiểm tra tại Wokwi

### 1.1 Tạo project

1. Đăng nhập [Wokwi](https://wokwi.com/).
2. Mở [project ESP32 mới](https://wokwi.com/projects/new/esp32), dùng Arduino C++.
3. Đặt tên `IoT Giong Noi` và lưu project theo nút Save của giao diện.
4. Nhấn nút `+` ở vùng sơ đồ để thêm linh kiện.

### 1.2 Thêm linh kiện

| Linh kiện | Số lượng | Tên để tìm |
|---|---:|---|
| ESP32 DevKit | 1 | Có sẵn trong project ESP32 |
| Cảm biến nhiệt độ và độ ẩm | 1 | DHT22 |
| Module cảm biến ánh sáng | 1 | Photoresistor Sensor |
| LED đỏ, đại diện đèn | 1 | LED |
| LED xanh, đại diện tải quạt được relay đóng điện | 1 | LED |
| Điện trở hạn dòng | 2 | Resistor, đặt mỗi cái 330 ohm |
| Điện trở kéo lên DATA DHT22 | 1 | Resistor, đặt 10000 ohm |
| Module relay một kênh | 1 | Relay Module |

DHT22 là một cảm biến đo hai đại lượng. LDR là cảm biến thứ hai. LED xanh chỉ biểu diễn quạt đang được cấp điện; mô hình này không mô phỏng tốc độ hay luồng gió của quạt thật.

### 1.3 Nối dây

Nhấn vào chân đầu tiên rồi chân đích để nối. Dựa vào nhãn chân khi rê chuột. Một số bo ghi `D15`, một số ghi `15`; đều là GPIO15 trong mẫu này.

| Chân linh kiện | Nối đến |
|---|---|
| DHT22 VCC | ESP32 3V3 |
| DHT22 GND | ESP32 GND |
| DHT22 SDA/DATA | ESP32 GPIO15 |
| Điện trở 10k | Giữa DATA của DHT22 và 3V3 |
| DHT22 NC | Để trống |
| LDR VCC | ESP32 3V3 |
| LDR GND | ESP32 GND |
| LDR AO | ESP32 GPIO34 |
| LDR DO | Để trống |
| ESP32 GPIO26 | Điện trở 330 ohm thứ nhất → chân A LED đỏ |
| LED đỏ K | GND |
| Relay VCC | ESP32 5V hoặc VIN của bo mô phỏng |
| Relay GND | GND |
| Relay IN | ESP32 GPIO27 |
| Relay COM | ESP32 3V3 |
| Relay NO | Điện trở 330 ohm thứ hai → chân A LED xanh |
| LED xanh K | GND |
| Relay NC | Để trống |

Tất cả GND nối chung. Không nối LDR AO vào GPIO26/27: GPIO34 được chọn để đọc analog khi Wi-Fi đang hoạt động. A là anode, K là cathode của LED. Đây là sơ đồ cho mô phỏng tải LED điện áp thấp; khi chuyển sang quạt thật cần chọn nguồn và mạch công suất phù hợp.

### 1.4 Thêm mã và thư viện

Các file mẫu nằm trong thư mục `D:\IOT\huong-dan-iot\wokwi`.

1. Mở `sketch.ino` trên máy, sao chép toàn bộ vào tab `sketch.ino` của Wokwi.
2. Trong menu file/tab của trình soạn thảo Wokwi, chọn tạo file mới và tạo `config.h`. Dán nội dung file `config.h` trên máy vào đó.
3. Tạo thêm `thingspeak_ca.h` và dán nguyên nội dung file cùng tên. Đây là chứng chỉ CA công khai để xác minh HTTPS, không phải API key.
4. Vào `Library Manager`, nhấn `+`, tìm và thêm `DHT sensor library for ESPx` và `Blynk`.
5. Kiểm tra `libraries.txt` có hai dòng giống file mẫu. Wokwi tự quản lý file này khi bạn thêm thư viện.
6. Trong `config.h`, đổi tạm thành `#define LAB_STAGE 1`. Ở phần này chưa cần kết nối Blynk hay ThingSpeak.
7. Nhấn nút chạy mô phỏng hình tam giác.

Mẫu dùng DHTesp vì [Wokwi khuyến nghị thư viện này cho DHT22 trên ESP32](https://docs.wokwi.com/parts/wokwi-dht22). Cách thêm thư viện xem tại [Wokwi Library Manager](https://docs.wokwi.com/guides/libraries).

### 1.5 Thử cảm biến và đầu ra

1. Xem Serial Monitor ở 115200 baud. Mỗi khoảng 2 giây có một dòng `T=... | H=... | Sang=... | ADC=...`.
2. Khi đang chạy, nhấn DHT22, thay đổi nhiệt độ và độ ẩm. Chờ số liệu Serial đổi tương ứng.
3. Nhấn LDR, thay đổi mức lux đầu vào của mô phỏng. Quan sát chỉ số `Sang` thay đổi.
4. Trong ô nhập Serial Monitor, gửi lần lượt từng ký tự rồi Enter:

| Ký tự | Kết quả cần thấy |
|---|---|
| `1` | LED đỏ bật |
| `0` | LED đỏ tắt |
| `3` | Relay đóng COM–NO, LED xanh bật |
| `2` | Relay ngắt COM–NO, LED xanh tắt |

`Sang` là chỉ số tương đối 0–100% quy đổi từ ADC, không phải phép đo lux đã hiệu chuẩn. Giá trị lux bạn kéo trên LDR không có cùng đơn vị với chỉ số này.

Nếu LED xanh sáng khi gửi `2` và tắt khi gửi `3`, dừng mô phỏng, đổi `#define RELAY_ON_LEVEL LOW` thành `HIGH` trong `config.h`, chạy lại và kiểm tra. Relay có cấu hình đảo logic, vì vậy phải xác nhận bằng LED tải, không chỉ nhìn LED tích hợp của module. Giữ giá trị đã kiểm tra cho các phần sau.

**Hoàn thành phần 1:** hai cảm biến cho số liệu thay đổi và cả hai đầu ra bật/tắt riêng đúng lệnh. Chụp lại sơ đồ và Serial làm minh chứng.

## Phần 2 Điều khiển qua Blynk

### 2.1 Tạo Template

1. Đăng nhập [Blynk Console](https://blynk.cloud/).
2. Nếu chưa thấy Developer Zone, bật Developer Mode trong hồ sơ.
3. Vào `Developer Zone → My Templates/Templates → New Template`.
4. Tên `IoT Giong Noi`, Hardware `ESP32`, Connection `WiFi`, rồi lưu.

Tên menu có thể khác nhẹ theo phiên bản giao diện. Luồng chuẩn là tạo Template → khai báo Datastreams → tạo dashboard → tạo Device từ Template. [Hướng dẫn Blynk](https://docs.blynk.io/en/getting-started/template-quick-setup)

### 2.2 Tạo năm Datastream

Trong Template, mở `Datastreams`, tạo kiểu `Virtual Pin` theo bảng:

| Tên | Virtual Pin | Kiểu dữ liệu | Min | Max | Đơn vị |
|---|---|---|---:|---:|---|
| Nhiet do | V0 | Double | -40 | 80 | °C |
| Do am | V1 | Double | 0 | 100 | % |
| Anh sang tuong doi | V2 | Integer | 0 | 100 | % |
| Den | V3 | Integer | 0 | 1 | Không |
| Quat | V4 | Integer | 0 | 1 | Không |

Đặt mặc định V3/V4 bằng 0 nếu có mục Default. Virtual Pin là kênh dữ liệu của Blynk, không phải chân vật lý: V3 được code ánh xạ tới GPIO26, V4 tới GPIO27.

### 2.3 Tạo Web Dashboard

1. Mở `Web Dashboard` trong Template và chọn Edit.
2. Thêm ba widget hiển thị giá trị, gán lần lượt V0, V1, V2.
3. Thêm hai widget Switch, gán lần lượt V3 và V4; giá trị OFF=0, ON=1.
4. Nếu widget tên Button, chọn chế độ Switch để giữ trạng thái, không chọn Push/momentary.
5. Đặt nhãn `Đèn` và `Quạt`, Save/Apply.

Bạn có thể làm dashboard điện thoại sau. Dashboard điện thoại cần bố trí widget riêng, không mặc định giống hệt web. Phần bài này có thể hoàn thành bằng dashboard web và biểu đồ ThingSpeak.

### 2.4 Tạo Device và điền cấu hình

1. Vào `Devices → New/Create Device → From Template`.
2. Chọn Template vừa tạo và đặt tên `ESP32 Wokwi`.
3. Trong Device, tìm `Developer Tools` hoặc thông tin thiết bị để lấy Device Auth Token.
4. Lấy Template ID từ Template. Điền vào `config.h` ở Wokwi:

```cpp
#define LAB_STAGE 2
#define BLYNK_TEMPLATE_ID "TMPL..."
#define BLYNK_TEMPLATE_NAME "IoT Giong Noi"
#define BLYNK_AUTH_TOKEN "TOKEN_CUA_DEVICE"
#define BLYNK_SERVER "blynk.cloud"
```

Nếu Blynk cung cấp hostname vùng như `sgp1.blynk.cloud`, dùng hostname đó thay `blynk.cloud`; không thêm `https://` hay đường dẫn. Không nhầm Auth Token của Device với Template ID hoặc mật khẩu đăng nhập.

Chỉ điền khóa trong bản làm việc của bạn; trước khi chia sẻ project Wokwi công khai hoặc ảnh code, xóa token và Write API Key. [Cách tạo Device và lấy token](https://docs.blynk.io/en/getting-started/activating-devices/manual-device-activation)

### 2.5 Chạy và kiểm tra

1. Dừng và chạy lại Wokwi sau khi sửa file.
2. Chờ ESP32 kết nối `Wokwi-GUEST`, đồng bộ đồng hồ, kết nối TLS tới Blynk. Có thể mất vài chục giây.
3. Mở Device vừa tạo, xác nhận `Online`.
4. Thử hai công tắc, quan sát LED đỏ và xanh; Serial in `OUTPUT: den=... quat=...`.
5. Thay đổi DHT22/LDR trong Wokwi, xem ba giá trị Blynk cập nhật khoảng mỗi 5 giây.

Trong phần 2 trở đi, điều khiển bằng Blynk; các ký tự Serial ở phần 1 đã được tắt để tránh nhầm nguồn lệnh. Khi mô phỏng khởi động lại, hai tải trở về OFF. Khi kết nối lại Blynk, ESP32 báo trạng thái hiện tại lên dashboard.

**Hoàn thành phần 2:** Device Online, nhận số đo và điều khiển được hai đầu ra. Giữ Wokwi đang chạy trong khi thử cloud.

Nếu Serial lặp `Connecting to blynk.cloud:443` và `Certificate OK` nhưng không có `Ready`, thiết bị vẫn chưa Online. Mẫu đã tăng thời gian chờ đăng nhập lên 15 giây vì bắt tay TLS trong Wokwi mất vài giây. Kiểm tra `BLYNK_SERVER` theo vùng hiển thị trong Console và Device Auth Token của đúng Device; đừng coi `Certificate OK` là đã đăng nhập Blynk thành công.

## Phần 3 Lưu dữ liệu và vẽ biểu đồ trên ThingSpeak

### 3.1 Tạo Channel

1. Đăng nhập [ThingSpeak](https://thingspeak.mathworks.com/).
2. Vào `Channels → My Channels → New Channel`.
3. Đặt tên `IoT Giong Noi`, bật năm Field:

| Field | Tên | Nội dung |
|---|---|---|
| Field 1 | Nhiet do | °C |
| Field 2 | Do am | % |
| Field 3 | Anh sang tuong doi | Chỉ số 0–100% |
| Field 4 | Trang thai den | 0 hoặc 1 |
| Field 5 | Trang thai quat | 0 hoặc 1 |

4. Save Channel. Có thể giữ channel private và xem trong `Private View` khi đăng nhập.
5. Mở tab `API Keys`, sao chép **Write API Key**.

### 3.2 Bật phần 3 trong mã

Trong `config.h` của Wokwi, giữ nguyên cấu hình Blynk và thay:

```cpp
#define LAB_STAGE 3
#define THINGSPEAK_WRITE_API_KEY "WRITE_API_KEY_CUA_CHANNEL"
```

Đảm bảo đã tạo tab `thingspeak_ca.h` với đúng nội dung mẫu. Không cần cài thêm thư viện ThingSpeak; mẫu dùng HTTPS API có sẵn qua HTTPClient của ESP32. API ghi chọn channel theo Write API Key, nên mẫu không cần Channel ID.

### 3.3 Chạy và xem kết quả

1. Chạy lại Wokwi, chờ Blynk Online.
2. Chờ khoảng 20–40 giây sau khi mạng và đồng hồ sẵn sàng.
3. Trong Serial, tìm `ThingSpeak OK, entry=...`. Entry ID phải lớn hơn 0.
4. Mở `Private View` của channel, làm mới trang hoặc bật cập nhật biểu đồ nếu giao diện có tùy chọn đó.
5. Đổi nhiệt độ DHT22, đợi ít nhất một lần ghi rồi xem điểm mới.
6. Bật đèn trên Blynk, giữ trạng thái hơn 20 giây, kiểm tra Field 4 có giá trị 1; thử quạt với Field 5.

Gói miễn phí cho phép cập nhật mỗi channel cách nhau tối thiểu 15 giây; mẫu chọn ít nhất 20 giây giữa các request. Giá trị trạng thái trên ThingSpeak là ảnh chụp định kỳ: nếu bật rồi tắt trong khoảng giữa hai lần ghi, biểu đồ có thể không lưu lần bật ngắn đó. Blynk vẫn là đường điều khiển trực tiếp. [Giới hạn ThingSpeak](https://www.mathworks.com/help/thingspeak/channel-control.html)

Code gửi dữ liệu bằng một task riêng để không phải dừng vòng xử lý Blynk trong lúc chờ HTTPS. HTTP 200 chưa đủ để kết luận ghi thành công: mẫu còn kiểm tra Entry ID vì API có thể trả `0` khi ghi thất bại. [API Write Data](https://www.mathworks.com/help/thingspeak/writedata.html)

**Hoàn thành phần 3:** có ít nhất ba bản ghi mới, các giá trị hợp lý, Blynk vẫn điều khiển được mạch. Chụp các biểu đồ làm minh chứng.

## Phần 4 Điều khiển bằng giọng nói tiếng Việt

### 4.1 Hiểu đường đi của lệnh

```text
Micro trên máy tính
  → trình duyệt chuyển giọng nói thành chữ tiếng Việt
  → Python kiểm tra và ánh xạ câu lệnh
  → Blynk HTTPS API cập nhật V3 hoặc V4
  → ESP32 mô phỏng nhận lệnh
  → LED/relay đổi trạng thái
```

Phần này chạy trên máy tính Windows, dùng micro của máy tính. Không đặt file Python hoặc HTML vào Wokwi. Không cần micro nối ESP32. Wokwi giữ `LAB_STAGE 3`.

### 4.2 Khởi chạy chương trình cục bộ

Hai file có sẵn tại `D:\IOT\huong-dan-iot\giong-noi`: `server.py` và `index.html`. Giữ chúng cùng thư mục. Python chỉ dùng thư viện chuẩn, không cần cài Flask hoặc gói pip.

Mở PowerShell và chạy:

```powershell
Set-Location 'D:\IOT\huong-dan-iot\giong-noi'
py -3 server.py
```

Nếu máy không có lệnh `py`, có thể dùng Python được đóng gói sẵn trên máy này:

```powershell
& 'C:\Users\ADMIN\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe' 'D:\IOT\huong-dan-iot\giong-noi\server.py'
```

1. Chỉ khi terminal hiện đúng lời nhắc `Device Auth Token (an khi nhap):`, dán cùng token đã dùng trong Wokwi rồi Enter. Không thấy ký tự hiện ra là bình thường. Nếu terminal còn hiện `PS ...>`, hãy chạy `server.py` trước; dán token tại dấu nhắc PowerShell sẽ làm lộ token trong lịch sử lệnh.
2. Khi hỏi hostname, nhập cùng `BLYNK_SERVER` trong `config.h`, hoặc nhấn Enter để dùng mặc định `sgp1.blynk.cloud` của project này.
3. Giữ cửa sổ PowerShell đang chạy. Dừng bằng Ctrl+C khi xong.
4. Mở `http://127.0.0.1:8000` bằng Chrome trên cùng máy. Không mở `index.html` bằng cách nhấp đôi, vì trang cần Python chuyển tiếp lệnh.

Token chỉ được giữ ở chương trình Python trong phiên chạy; trang web không cần nhận token. Chương trình chỉ phục vụ trên máy tính này.

### 4.3 Thử nhập chữ trước

1. Đảm bảo Wokwi đang chạy và Device Blynk Online.
2. Nhập `bật đèn`, nhấn `Gửi lệnh`.
3. Trang báo Blynk nhận lệnh; quan sát LED đỏ để xác nhận mạch thực sự bật.
4. Thử lần lượt `tắt đèn`, `bật quạt`, `tắt quạt`.

Nếu bước nhập chữ chưa hoạt động, sửa cấu hình token/hostname/Virtual Pin hoặc kết nối trước khi thử micro. Thông báo cloud chấp nhận lệnh không thay thế xác nhận từ đầu ra Wokwi.

### 4.4 Thử nói

1. Nhấn `Nhấn để nói`.
2. Cho phép trình duyệt dùng micro khi được hỏi.
3. Nói rõ một câu: `bật đèn`.
4. Chờ kết thúc nhận giọng nói; chữ nhận được sẽ xuất hiện trong ô nhập, rồi được gửi tự động.
5. Quan sát LED đỏ và công tắc tương ứng trên Blynk.
6. Thử đủ bốn câu lệnh.

Mẫu đặt ngôn ngữ `vi-VN`. Web Speech API có mức hỗ trợ khác nhau theo trình duyệt và có thể cần dịch vụ nhận dạng qua Internet. Nếu trình duyệt không hỗ trợ hoặc dịch vụ không nhận được tiếng Việt, phần nhập chữ vẫn giúp kiểm tra đường điều khiển; riêng tiêu chí giọng nói chưa hoàn thành và cần xử lý môi trường STT. [SpeechRecognition](https://developer.mozilla.org/en-US/docs/Web/API/SpeechRecognition), [ngôn ngữ nhận dạng](https://developer.mozilla.org/en-US/docs/Web/API/SpeechRecognition/lang)

### 4.5 Phạm vi câu lệnh

| Câu nói | Kênh Blynk | Giá trị |
|---|---|---|
| bật đèn | V3 | 1 |
| tắt đèn | V3 | 0 |
| bật quạt | V4 | 1 |
| tắt quạt | V4 | 0 |

Mẫu chấp nhận chữ không dấu, viết hoa và dấu câu ở cuối. Câu phủ định như `không bật đèn`, câu không rõ nghĩa hoặc nhiều lệnh ghép sẽ bị từ chối, không tự suy đoán. Mẫu chưa xử lý `đặt nhiệt độ 25 độ`: đề bài nêu đây là ví dụ; muốn thêm cần định nghĩa setpoint và chế độ điều khiển nhiệt độ.

**Hoàn thành phần 4:** nhận đúng và thực hiện được cả bốn câu lệnh tiếng Việt; nút điều khiển Blynk vẫn dùng được, ThingSpeak vẫn nhận dữ liệu.

## Xử lý các lỗi thường gặp

| Hiện tượng | Kiểm tra |
|---|---|
| Không tìm thấy DHTesp.h | Thêm đúng `DHT sensor library for ESPx` trong Library Manager |
| Không tìm thấy config.h hoặc thingspeak_ca.h | Tạo đúng file trong chính project Wokwi, không chỉ trên máy |
| DHT22 báo lỗi | DATA ở GPIO15, có nguồn và GND chung, đúng thư viện |
| Ánh sáng không đổi | Dùng AO đến GPIO34, chỉnh LDR khi mô phỏng đang chạy |
| LED không sáng | A/K, GND, điện trở và GPIO26 |
| Relay hoạt động ngược | Kiểm tra COM/NO và `RELAY_ON_LEVEL` theo phần 1 |
| Blynk Offline | Wokwi có chạy, Wi-Fi, token của đúng Device, hostname, đồng bộ thời gian và Serial TLS |
| Blynk có số đo nhưng không điều khiển được | Switch gán đúng V3/V4, Integer 0–1; mở dashboard của đúng Device |
| ThingSpeak trả 0 hoặc không có điểm mới | Write API Key, Field đã bật, đúng channel, không có chương trình khác ghi quá nhanh |
| HTTPS thất bại | Internet, đồng hồ ESP32, nội dung CA; không xóa xác minh chứng chỉ để che lỗi |
| Không mở được web cục bộ | Python đang chạy, cổng 8000 còn trống, mở đúng 127.0.0.1:8000 |
| Blynk đổi công tắc nhưng Wokwi không phản ứng | Bấm nút chạy Wokwi, chờ Serial báo `Ready` và Device Blynk Online rồi thử lại; tải lại trang Wokwi có thể dừng mô phỏng |
| Giọng nói không điều khiển được dù Blynk làm được | Chạy `server.py`, nhấn Enter ở lời nhắc hostname, mở `http://127.0.0.1:8000`; thử nhập chữ trước để tách lỗi micro/nhận dạng |
| API Blynk báo 400 | Token đúng Device, hostname đúng vùng, V3/V4 tồn tại và nhận Integer |
| Trang báo nhận lệnh nhưng mạch không đổi | Wokwi phải đang chạy và Blynk Online; kiểm tra Serial và đấu dây |
| Micro báo not-allowed | Cấp quyền micro cho trang và cho trình duyệt trong Windows |
| Micro báo network hoặc no-speech | Kiểm tra Internet, micro, dịch vụ nhận dạng; thử nhập chữ để tách lỗi |

## Các file trong bộ mẫu

```text
huong-dan-iot/
  HUONG_DAN.md
  wokwi/
    sketch.ino
    config.h
    libraries.txt
    thingspeak_ca.h
  giong-noi/
    server.py
    index.html
```

Sơ đồ được bạn dựng trực tiếp trong Wokwi theo bảng dây; bộ mẫu chưa có `diagram.json`. Hãy lưu project Wokwi sau khi dựng mạch để giữ sơ đồ.

Trong project Wokwi gốc hiện có Device Auth Token và ThingSpeak Write API Key ngay trong `config.h`; người mở được mã nguồn có thể thấy các khóa này. Nếu dùng URL để nộp hoặc chia sẻ, hãy tự tạo khóa mới trong Blynk/ThingSpeak và cập nhật lại cấu hình, hoặc dùng bản project dành riêng cho việc chia sẻ đã bỏ khóa. Sau khi thử xong, dừng mô phỏng để tránh tiếp tục dùng hạn mức cloud.
