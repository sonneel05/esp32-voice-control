// ==== Thông tin Template Blynk ====
#define BLYNK_TEMPLATE_ID "TMPL6uSeW6si_"
#define BLYNK_TEMPLATE_NAME "testdoan"
#define BLYNK_AUTH_TOKEN "xfGWSjU0KwX7gK8xcjSrO6GROYGqakom"

// ==== Thư viện cần thiết ====
#include <DHT.h>                    // Cảm biến nhiệt độ & độ ẩm DHT11
#include <WiFi.h>                   // Kết nối WiFi
#include <BlynkSimpleEsp32.h>       // Thư viện Blynk dành cho ESP32

// ==== Thông tin WiFi ====
char ssid[] = "Sonlee";            // Tên WiFi
char pass[] = "11223344";           // Mật khẩu WiFi

// ==== Định nghĩa chân kết nối ====
#define LDR_DO         34           // Chân tín hiệu digital của cảm biến ánh sáng LDR
#define DHT_PIN        13           // Chân kết nối cảm biến DHT11
#define DHT_TYPE       DHT11        // Loại cảm biến DHT sử dụng
#define FAN_PWM_PIN    14           // Chân điều khiển quạt bằng PWM
#define LED_PIN        27           // Chân điều khiển LED

// ==== Ngưỡng nhiệt độ tự động điều khiển quạt ====
#define TEMP_THRESHOLD        30.0   // Trên 30°C → bật quạt nhẹ
#define HIGH_TEMP_THRESHOLD   34.0   // Trên 34°C → bật quạt mạnh

// ==== Khởi tạo cảm biến DHT ====
DHT dht(DHT_PIN, DHT_TYPE);

// ==== Biến trạng thái hệ thống ====
bool ledManualMode = true;          // true = LED điều khiển tay
bool ledStateByVoice = false;       // Trạng thái LED do giọng nói quyết định

bool fanControlEnabled = false;     // true = điều khiển quạt tay
int fanManualLevel = 0;             // Mức PWM khi điều khiển tay

float lastTemp = -1;                // Nhiệt độ cuối cùng gửi lên Blynk
float lastHumi = -1;                // Độ ẩm cuối cùng gửi lên Blynk
int lastFanPower = -1;              // Trạng thái quạt cuối gửi lên Blynk
int lastLedState = -1;              // Trạng thái LED cuối gửi lên Blynk

// ==== Callback khi thay đổi V0 (nút LED) trên Blynk ====
BLYNK_WRITE(V0) {
  int val = param.asInt();

  if (!ledManualMode) {
    ledManualMode = true;
    Blynk.virtualWrite(V1, 0);  // Gửi về app: chuyển sang chế độ thủ công
    Serial.println("Chuyển về chế độ THỦ CÔNG vì người dùng bấm V0");
  }

  ledStateByVoice = val;
  Serial.print("LED thủ công (V0): ");
  Serial.println(val ? "BẬT" : "TẮT");
}

// ==== Callback khi thay đổi V1 (chế độ LED) trên Blynk ====
BLYNK_WRITE(V1) {
  int val = param.asInt();
  ledManualMode = (val == 0);  // 0 = thủ công, 1 = tự động (LDR)
  Serial.println(ledManualMode ? "LED chế độ THỦ CÔNG (V0)" : "LED chế độ TỰ ĐỘNG (LDR)");
}

// ==== Callback khi thay đổi V6 (slider quạt tay) ====
BLYNK_WRITE(V6) {
  int val = param.asInt();

  if (!fanControlEnabled) {
    fanControlEnabled = true;
    Blynk.virtualWrite(V7, 1);  // Cập nhật chế độ quạt trên App
    Serial.println("Chuyển về QUẠT THỦ CÔNG do người dùng chỉnh slider V6");
  }

  fanManualLevel = (val == 0) ? 0 : map(val, 0, 100, 127, 255);  // Đổi % sang PWM
  Serial.print("Blynk (V6) – Quạt tay: ");
  Serial.print(val);
  Serial.print("% => PWM: ");
  Serial.println(fanManualLevel);
}

// ==== Callback khi thay đổi V7 (chế độ quạt) ====
BLYNK_WRITE(V7) {
  int val = param.asInt();
  fanControlEnabled = (val == 1);
  Serial.println(fanControlEnabled ? "QUẠT: chế độ THỦ CÔNG" : "QUẠT: chế độ CẢM BIẾN NHIỆT");
}

// ==== Xử lý lệnh điều khiển từ máy tính (giọng nói) ====
void checkSerial() {
  if (Serial.available()) {
    int cmd = Serial.read();

    if (cmd == '5') {         // Bật LED
      ledManualMode = true;
      ledStateByVoice = true;
      Blynk.virtualWrite(V0, 1);
      Blynk.virtualWrite(V1, 0);
      Serial.println("Lệnh giọng nói: BẬT LED");
    } else if (cmd == '6') {  // Tắt LED
      ledManualMode = true;
      ledStateByVoice = false;
      Blynk.virtualWrite(V0, 0);
      Blynk.virtualWrite(V1, 0);
      Serial.println("Lệnh giọng nói: TẮT LED");
    } else if (cmd == '7') {  // Bật quạt
      fanControlEnabled = true;
      fanManualLevel = 255;
      Blynk.virtualWrite(V6, 100);
      Blynk.virtualWrite(V7, 1);
      Serial.println("Lệnh giọng nói: BẬT QUẠT");
    } else if (cmd == '8') {  // Tắt quạt
      fanControlEnabled = true;
      fanManualLevel = 0;
      Blynk.virtualWrite(V6, 0);
      Blynk.virtualWrite(V7, 1);
      Serial.println("Lệnh giọng nói: TẮT QUẠT");
    } else if (cmd == '4') {  // Tắt hết
      ledManualMode = true;
      ledStateByVoice = false;
      fanControlEnabled = true;
      fanManualLevel = 0;
      Blynk.virtualWrite(V0, 0);
      Blynk.virtualWrite(V1, 0);
      Blynk.virtualWrite(V6, 0);
      Blynk.virtualWrite(V7, 1);
      Serial.println("Lệnh giọng nói: TẮT HẾT");
    } else if (cmd == '9') {  // Bật hết
      ledManualMode = true;
      ledStateByVoice = true;
      fanControlEnabled = true;
      fanManualLevel = 255;
      Blynk.virtualWrite(V0, 1);
      Blynk.virtualWrite(V1, 0);
      Blynk.virtualWrite(V6, 100);
      Blynk.virtualWrite(V7, 1);
      Serial.println("Lệnh giọng nói: BẬT HẾT");
    }
  }
}

// ==== Hàm điều khiển LED theo chế độ ====
void controlLed() {
  int ledState = 0;
  if (ledManualMode) {
    ledState = ledStateByVoice;  // Điều khiển theo tay
  } else {
    ledState = (digitalRead(LDR_DO) == 0) ? 0 : 1;  // Theo cảm biến ánh sáng
  }

  digitalWrite(LED_PIN, ledState);

  // Cập nhật Blynk khi trạng thái thay đổi
  if (ledState != lastLedState) {
    Blynk.virtualWrite(V2, ledState);
    lastLedState = ledState;
  }

  Serial.print("LED: ");
  Serial.println(ledState ? "BẬT" : "TẮT");
}

// ==== Hàm điều khiển quạt theo nhiệt độ hoặc tay ====
void controlFan(float temperature) {
  int pwmVal = 0;

  if (fanControlEnabled) {
    pwmVal = fanManualLevel;
  } else {
    if (temperature > HIGH_TEMP_THRESHOLD)
      pwmVal = 255;
    else if (temperature > TEMP_THRESHOLD)
      pwmVal = 127;
  }

  analogWrite(FAN_PWM_PIN, pwmVal);  
  int percent = (pwmVal * 100) / 255;
  if (percent != lastFanPower) {
    Blynk.virtualWrite(V5, percent);
    lastFanPower = percent;
  }

  Serial.print("Quạt: ");
  Serial.print(percent);
  Serial.println("%");
}

// ==== Hàm đọc cảm biến DHT11 và gửi dữ liệu ====
void readDHTSensor() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (!isnan(t) && !isnan(h)) {
    if (abs(t - lastTemp) > 0.1) {
      Blynk.virtualWrite(V4, t);  // V4: nhiệt độ
      lastTemp = t;
    }
    if (abs(h - lastHumi) > 0.1) {
      Blynk.virtualWrite(V3, h);  // V3: độ ẩm
      lastHumi = h;
    }

    Serial.print("Nhiệt độ: ");
    Serial.print(t);
    Serial.print("°C - Độ ẩm: ");
    Serial.println(h);

    controlFan(t);  // Gửi nhiệt độ để điều khiển quạt
  } else {
    Serial.println("Lỗi đọc DHT11");
  }
}

// ==== Cấu hình ban đầu ====
void setup() {
  Serial.begin(115200);           // Bật Serial
  pinMode(LDR_DO, INPUT);         // LDR đọc digital
  pinMode(FAN_PWM_PIN, OUTPUT);   // Quạt
  pinMode(LED_PIN, OUTPUT);       // LED

  dht.begin();                    // Bắt đầu DHT11
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);  // Kết nối Blynk
  Serial.println("=== HỆ THỐNG BẮT ĐẦU ===");
}

// ==== Vòng lặp chính ====
void loop() {
  Blynk.run();        // Xử lý App
  checkSerial();      // Xử lý giọng nói
  controlLed();       // Cập nhật đèn
  readDHTSensor();    // Cập nhật nhiệt độ, quạt
  delay(1000);        // Đợi 1 giây
}
