#include "config.h"
#include <DHTesp.h>

#if LAB_STAGE >= 2
#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <BlynkSimpleEsp32_SSL.h>
#include <time.h>
#endif

#if LAB_STAGE >= 3
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include "thingspeak_ca.h"
#endif

constexpr int DHT_PIN = 15;
constexpr int LDR_PIN = 34;
constexpr int LIGHT_PIN = 26;
constexpr int RELAY_PIN = 27;
DHTesp dht;
bool lightOn = false;
bool fanOn = false;
bool sensorValid = false;
float temperature = 0;
float humidity = 0;
int brightness = 0;
uint32_t lastSensor = 0;

void applyOutputs() {
  digitalWrite(LIGHT_PIN, lightOn ? HIGH : LOW);
  digitalWrite(RELAY_PIN, fanOn ? RELAY_ON_LEVEL : !RELAY_ON_LEVEL);
  Serial.printf("OUTPUT: den=%s, quat=%s\n", lightOn ? "ON" : "OFF",
                fanOn ? "ON" : "OFF");
}

#if LAB_STAGE >= 2
uint32_t lastBlynkSend = 0;
uint32_t lastConnectAttempt = 0;
uint32_t lastWiFiAttempt = 0;

BLYNK_WRITE(V3) {
  lightOn = param.asInt() == 1;
  applyOutputs();
}

BLYNK_WRITE(V4) {
  fanOn = param.asInt() == 1;
  applyOutputs();
}

BLYNK_CONNECTED() {
  // Bao trang thai hien tai; khong bat lai tai do lenh cu trong cloud.
  Blynk.virtualWrite(V3, lightOn ? 1 : 0);
  Blynk.virtualWrite(V4, fanOn ? 1 : 0);
}

void maintainCloud() {
  uint32_t now = millis();
  if (WiFi.status() != WL_CONNECTED) {
    if (now - lastWiFiAttempt >= 10000) {
      lastWiFiAttempt = now;
      Serial.println("Dang ket noi lai WiFi...");
      WiFi.reconnect();
    }
    return;
  }
  // TLS can dong ho dung de xac minh chung chi.
  if (time(nullptr) < 1700000000) return;
  if (!Blynk.connected()) {
    if (now - lastConnectAttempt >= 10000) {
      lastConnectAttempt = now;
      Blynk.connect(15000);
    }
    return;
  }
  Blynk.run();
  if (now - lastBlynkSend >= 5000 && sensorValid) {
    lastBlynkSend = now;
    Blynk.virtualWrite(V0, temperature);
    Blynk.virtualWrite(V1, humidity);
    Blynk.virtualWrite(V2, brightness);
  }
}
#endif

#if LAB_STAGE >= 3
struct Reading {
  float temperature;
  float humidity;
  int brightness;
  int light;
  int fan;
};
QueueHandle_t uploadQueue = nullptr;
uint32_t lastUpload = 0;

// Gui HTTPS o task rieng de Blynk van nhan lenh trong luc gui du lieu.
void thingSpeakTask(void*) {
  Reading data;
  for (;;) {
    if (xQueueReceive(uploadQueue, &data, portMAX_DELAY) != pdTRUE) continue;
    if (WiFi.status() != WL_CONNECTED || time(nullptr) < 1700000000) continue;
    WiFiClientSecure secure;
    secure.setCACert(THINGSPEAK_ROOT_CA);
    secure.setHandshakeTimeout(8);
    HTTPClient http;
    http.setConnectTimeout(8000);
    http.setTimeout(8000);
    if (!http.begin(secure, "https://api.thingspeak.com/update")) {
      Serial.println("ThingSpeak: khong khoi tao duoc HTTPS");
      continue;
    }
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    String body = String("api_key=") + THINGSPEAK_WRITE_API_KEY
      + "&field1=" + String(data.temperature, 1)
      + "&field2=" + String(data.humidity, 1)
      + "&field3=" + String(data.brightness)
      + "&field4=" + String(data.light)
      + "&field5=" + String(data.fan);
    int code = http.POST(body);
    String result = code > 0 ? http.getString() : "";
    result.trim();
    if (code == 200 && result.toInt() > 0) {
      Serial.printf("ThingSpeak OK, entry=%ld\n", result.toInt());
    } else {
      Serial.printf("ThingSpeak FAIL, HTTP=%d (kiem tra key, field, mang va chu ky)\n", code);
    }
    http.end();
    // Bao dam moi request cach nhau it nhat 20 giay, ke ca khi mang cham.
    vTaskDelay(pdMS_TO_TICKS(20000));
  }
}
#endif

void setup() {
  Serial.begin(115200);
  pinMode(LIGHT_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  applyOutputs();
  analogReadResolution(12);
  dht.setup(DHT_PIN, DHTesp::DHT22);
  Serial.printf("BAT DAU PHAN %d\n", LAB_STAGE);

#if LAB_STAGE >= 3
  Serial.printf("ThingSpeak channel du kien: %d\n", THINGSPEAK_CHANNEL_ID);
#endif

#if LAB_STAGE == 1
  Serial.println("Serial: 1=bat den, 0=tat den, 3=bat quat, 2=tat quat");
#endif

#if LAB_STAGE >= 2
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin("Wokwi-GUEST", "", 6);
  configTime(0, 0, "pool.ntp.org", "time.google.com");
  Blynk.config(BLYNK_AUTH_TOKEN, BLYNK_SERVER, 443);
  Serial.println("Dang cho WiFi va dong bo thoi gian truoc khi ket noi Blynk...");
#endif

#if LAB_STAGE >= 3
  uploadQueue = xQueueCreate(1, sizeof(Reading));
  if (uploadQueue == nullptr) {
    Serial.println("Khong du bo nho tao hang doi ThingSpeak");
  } else if (xTaskCreate(thingSpeakTask, "ThingSpeak", 12288, nullptr, 1, nullptr) != pdPASS) {
    vQueueDelete(uploadQueue);
    uploadQueue = nullptr;
    Serial.println("Khong du bo nho tao task ThingSpeak");
  }
#endif
}

void loop() {
  uint32_t now = millis();
  if (now - lastSensor >= 2000) {
    lastSensor = now;
    TempAndHumidity reading = dht.getTempAndHumidity();
    sensorValid = !isnan(reading.temperature) && !isnan(reading.humidity);
    int raw = analogRead(LDR_PIN);
    // Chi so sang tuong doi 0-100%, KHONG phai lux da hieu chuan.
    brightness = constrain(map(raw, 0, 4095, 100, 0), 0L, 100L);
    if (sensorValid) {
      temperature = reading.temperature;
      humidity = reading.humidity;
      Serial.printf("T=%.1f C | H=%.1f %% | Sang=%d %% | ADC=%d\n",
                    temperature, humidity, brightness, raw);
    } else {
      Serial.println("Loi DHT22: kiem tra VCC/GND/SDA va thu vien DHTesp");
    }
  }

#if LAB_STAGE == 1
  while (Serial.available()) {
    char command = Serial.read();
    switch (command) {
      case '1': lightOn = true; break;
      case '0': lightOn = false; break;
      case '3': fanOn = true; break;
      case '2': fanOn = false; break;
      default: continue;
    }
    applyOutputs();
  }
#endif

#if LAB_STAGE >= 2
  maintainCloud();
#endif

#if LAB_STAGE >= 3
  if (uploadQueue && sensorValid && WiFi.status() == WL_CONNECTED
      && time(nullptr) >= 1700000000 && now - lastUpload >= 20000) {
    lastUpload = now;
    Reading data = {temperature, humidity, brightness, lightOn ? 1 : 0, fanOn ? 1 : 0};
    xQueueOverwrite(uploadQueue, &data);
  }
#endif
  delay(2);
}
