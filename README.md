# 🎙️ Voice-Controlled-ESP32  

Điều khiển **LED / Quạt** bằng giọng nói Tiếng Việt thông qua máy tính và ESP32.  
Máy tính (Python) sẽ nhận diện giọng nói, gửi lệnh qua Serial cho ESP32 để bật/tắt thiết bị.  

---

## 📌 Yêu cầu hệ thống
- **Python 3.9+**
- **Arduino IDE** (hoặc PlatformIO)
- **ESP32 Board** (đã cài driver USB)
- **Microphone** để nhập lệnh giọng nói
- **LED / Quạt mini** kết nối qua ESP32

---

## 📦 Cài thư viện Python

Chạy lần lượt trong terminal / CMD:

```bash
pip install pyserial
pip install SpeechRecognition
pip install pyaudio
