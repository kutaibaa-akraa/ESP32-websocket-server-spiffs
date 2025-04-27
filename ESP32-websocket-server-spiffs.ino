//كودًا كاملًا بلغة Arduino (C++) لشريحة مثل ESP32 ذات 30 بن، يقوم بما يلي:
//
//1. التحكم بأربعة مخارج (مثل LEDs أو ريليهات).
//2. استخدام WebSocket للتفاعل اللحظي مع الواجهة.
//3. إمكانية رفع ملفات إلى SPIFFS (نظام الملفات).
//4. صفحة إعدادات تمكن المستخدم من ضبط إعدادات الاتصال مثل SSID وكلمة السر وIP ثابت.
//
//
//### ✅ تشمل:
//
//- واجهة متجاوبة مع تصميم عصري بكروت.
//- تشغيل/إيقاف فردي + مؤقت زمني.
//- تشغيل/إيقاف جماعي.
//- حفظ واسترجاع أسماء المخارج.
//- الوضع الليلي.
//- إظهار/إخفاء الإعدادات.
//- زر "Reset to Factory" مع تأكيد.
//
//### ✅ المزايا:
//
//- تصميم **Responsive** مناسب للهواتف.
//- تحكم بـ 4 مخارج من خلال **Cards**.
//- WebSocket مباشر.
//- **مؤقت لكل مخرج**.
//- **زر الوضع الليلي**.
//- **زر إظهار/إخفاء الإعدادات**.
//- **تشغيل/إيقاف الكل**.
//- جلب أسماء المخارج ديناميكيًا من `/labels.json`.

// =================== 📚 المكتبات ===================
#include <WiFi.h>          // مكتبة الاتصال بالواي فاي
#include <WebServer.h>     // لإنشاء خادم ويب
#include <SPIFFS.h>        // نظام ملفات داخل الشريحة لتخزين الصفحات 
// مكتبة رفع الملفات -SPIFFS.h- قديمة و تعمل على اردوينو 1.8.19
#include <EEPROM.h>        // مكتبة EEPROM لحفظ الحالة بين الإقلاعات
#include <WebSocketsServer.h>

#define EEPROM_SIZE 148

char ssid[32] = "your-ssid";        // اسم الشبكة المحلية
char password[32] = "your-password"; // كلمة الدخول للشبكة المحلية
char static_ip[16] = "192.168.1.15";

char outputNames[4][16] = {"Output 1", "Output 2", "Output 3", "Output 4"};
bool outputStates[4] = {false, false, false, false};
unsigned long offTimers[4] = {0, 0, 0, 0};

WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

const int outputs[] = {25, 26, 27, 14};

void loadNetworkSettings() {
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.get(0, ssid);
  EEPROM.get(32, password);
  EEPROM.get(64, static_ip);
  for (int i = 0; i < 4; i++) {
    outputStates[i] = EEPROM.read(80 + i);
    EEPROM.get(84 + (i * 16), outputNames[i]);
    outputNames[i][15] = '\0'; // تأكيد نهاية السلسلة
  }
  EEPROM.end();
}

void saveNetworkSettings() {
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.put(0, ssid);
  EEPROM.put(32, password);
  EEPROM.put(64, static_ip);
  for (int i = 0; i < 4; i++) {
    outputNames[i][15] = '\0'; // تأكيد نهاية السلسلة
    EEPROM.write(80 + i, outputStates[i]);
    EEPROM.put(84 + (i * 16), outputNames[i]);
  }
  EEPROM.commit();
  EEPROM.end();
}

void setupOutputs() {
  for (int i = 0; i < 4; i++) {
    pinMode(outputs[i], OUTPUT);
    digitalWrite(outputs[i], outputStates[i] ? HIGH : LOW);
  }
}

void handleRoot() {
  File file = SPIFFS.open("/index.html", "r");
  if (file) {
    server.streamFile(file, "text/html");
    file.close();
  } else {
    server.send(404, "text/plain", "File not found");
  }
}

void handleStyle() { // --------تحميل ملف الستايل-------
  File file = SPIFFS.open("/style.css", "r");
  if (file) {
    server.streamFile(file, "text/css");
    file.close();
  } else {
    server.send(404, "text/plain", "File not found");
  }
}

void handleScript() { // -----------ملف السكريبت--------
  File file = SPIFFS.open("/script.js", "r");
  if (file) {
    server.streamFile(file, "application/javascript");
    file.close();
  } else {
    server.send(404, "text/plain", "File not found");
  }
}


void handleSaveSettings() {
  if (server.hasArg("ssid") && server.hasArg("pass") && server.hasArg("ip")) {
    strncpy(ssid, server.arg("ssid").c_str(), sizeof(ssid));
    strncpy(password, server.arg("pass").c_str(), sizeof(password));
    strncpy(static_ip, server.arg("ip").c_str(), sizeof(static_ip));
    for (int i = 0; i < 4; i++) {
      String argName = "name" + String(i);
      if (server.hasArg(argName)) {
        strncpy(outputNames[i], server.arg(argName).c_str(), sizeof(outputNames[i]));
      }
    }
    saveNetworkSettings();
    server.send(200, "text/plain", "Saved. Rebooting...");
    delay(1000);
    ESP.restart();
  } else {
    server.send(400, "text/plain", "Missing parameters");
  }
}

void handleLabelsJson() {
  String json = "{\"labels\":[";
  for (int i = 0; i < 4; i++) {
    json += "\"" + String(outputNames[i]) + "\"";
    if (i < 3) json += ",";
  }
  json += "]}";
  server.send(200, "application/json", json);
}

void handleApiOutput() {
  if (!server.hasArg("pin") || !server.hasArg("state")) {
    server.send(400, "application/json", "{\"error\":\"Missing pin or state\"}");
    return;
  }
  int pin = server.arg("pin").toInt();
  String state = server.arg("state");
  if (pin < 0 || pin > 3 || (state != "on" && state != "off")) {
    server.send(400, "application/json", "{\"error\":\"Invalid parameters\"}");
    return;
  }
  outputStates[pin] = (state == "on");
  digitalWrite(outputs[pin], outputStates[pin]);
  offTimers[pin] = 0;
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.write(80 + pin, outputStates[pin]);
  EEPROM.commit();
  EEPROM.end();
  String msg = "{\"pin\":" + String(pin) + ",\"state\":\"" + state + "\"}";
  server.send(200, "application/json", msg);
  webSocket.broadcastTXT(String(pin) + ":" + (outputStates[pin] ? "1" : "0"));
}

void handleApiStatus() {
  String response = "{\"outputs\":[";
  for (int i = 0; i < 4; i++) {
    response += "{\"pin\":" + String(i) + ",\"state\":\"" + (outputStates[i] ? "on" : "off") + "\"}";
    if (i < 3) response += ",";
  }
  response += "]}";
  server.send(200, "application/json", response);
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  if (type == WStype_TEXT) {
    Serial.printf("WebSocket received: %s\n", payload);
    String data = String((char*)payload);

    if (data.startsWith("all:")) {
      String command = data.substring(4);
      bool state = command == "on";
      for (int i = 0; i < 4; i++) {
        outputStates[i] = state;
        digitalWrite(outputs[i], state ? HIGH : LOW);
        offTimers[i] = 0;
        EEPROM.begin(EEPROM_SIZE);
        EEPROM.write(80 + i, outputStates[i]);
        EEPROM.commit();
        EEPROM.end();
        webSocket.broadcastTXT(String(i) + ":" + (state ? "1" : "0"));
      }
      return;
    }

    if (data.indexOf(":") > 0) {
      int sep = data.indexOf(":");
      int pinIndex = data.substring(0, sep).toInt();
      int seconds = data.substring(sep + 1).toInt();
      if (pinIndex >= 0 && pinIndex < 4) {
        outputStates[pinIndex] = true;
        digitalWrite(outputs[pinIndex], HIGH);
        offTimers[pinIndex] = millis() + (seconds * 1000);
        EEPROM.begin(EEPROM_SIZE);
        EEPROM.write(80 + pinIndex, outputStates[pinIndex]);
        EEPROM.commit();
        EEPROM.end();
        String message = String(pinIndex) + ":1";
        webSocket.broadcastTXT(message);
      }
    } else {
      int pinIndex = data.toInt();
      if (pinIndex >= 0 && pinIndex < 4) {
        outputStates[pinIndex] = !outputStates[pinIndex];
        digitalWrite(outputs[pinIndex], outputStates[pinIndex]);
        offTimers[pinIndex] = 0;
        EEPROM.begin(EEPROM_SIZE);
        EEPROM.write(80 + pinIndex, outputStates[pinIndex]);
        EEPROM.commit();
        EEPROM.end();
        String message = String(pinIndex) + ":" + (outputStates[pinIndex] ? "1" : "0");
        webSocket.broadcastTXT(message);
      }
    }
  }
}

void handleReset() {
  EEPROM.begin(EEPROM_SIZE);
  for (int i = 0; i < EEPROM_SIZE; i++) {
    EEPROM.write(i, 0);
  }
  EEPROM.commit();
  EEPROM.end();
  server.send(200, "text/plain", "EEPROM cleared. Restarting...");
  delay(1000);
  ESP.restart();
}

void setup() {
  Serial.begin(115200);
  loadNetworkSettings();

  IPAddress localIP;
  localIP.fromString(static_ip);
  IPAddress gateway(192,168,1,1);
  IPAddress subnet(255,255,255,0);
  WiFi.config(localIP, gateway, subnet);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi");
  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nFailed to connect, starting Access Point...");
    WiFi.softAP("ESP32_SETUP", "12345678");
    Serial.println("AP IP address: " + WiFi.softAPIP().toString());
  } else {
    Serial.println("\nConnected!");
    Serial.println(WiFi.localIP());
  }

  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS mount failed");
    return;
  }

  setupOutputs();
  server.on("/", handleRoot);
  server.on("/style.css", handleStyle);
  server.on("/script.js", handleScript);
  server.on("/save", HTTP_POST, handleSaveSettings);
  server.on("/labels.json", handleLabelsJson);
  server.on("/api/output", handleApiOutput);
  server.on("/api/status", handleApiStatus);
  server.on("/reset", handleReset); // <-- نقطة إعادة الضبط

  server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void loop() {
  server.handleClient();
  webSocket.loop();
  for (int i = 0; i < 4; i++) {
    if (offTimers[i] > 0 && millis() > offTimers[i]) {
      outputStates[i] = false;
      digitalWrite(outputs[i], LOW);
      offTimers[i] = 0;
      EEPROM.begin(EEPROM_SIZE);
      EEPROM.write(80 + i, 0);
      EEPROM.commit();
      EEPROM.end();
      String message = String(i) + ":0";
      webSocket.broadcastTXT(message);
    }
  }
}
