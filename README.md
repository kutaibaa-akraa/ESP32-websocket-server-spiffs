```markdown
# ESP32 WebSocket Server with SPIFFS Control Panel 🚀

مشروع متكامل للتحكم في المخارج (مثل LED أو الريليهات) عبر واجهة ويب تفاعلية باستخدام ESP32. يدعم الميزات المتقدمة مثل المؤقتات الزمنية، الوضع الليلي، والنسخ الاحتياطي التلقائي.

![واجهة التحكم](https://via.placeholder.com/800x400.png?text=Control+Panel+Preview)

## ✨ الميزات الرئيسية

### 🔌 التحكم الذكي
- **تحكم فردي/جماعي**: تشغيل/إيقاف 4 مخارج بشكل فردي أو جماعي.
- **مؤقت زمني**: ضبط مؤقت لكل مخرج (بالثواني).
- **حالات لحظية**: تحديث فوري للحالات عبر WebSocket.

### 🌗 واجهة متطورة
- **وضع ليلي**: تبديل بين المظهر الفاتح والداكن بنقرة زر.
- **تصميم متجاوب**: يعمل على جميع الشاشات (هواتف/أجهزة لوحية/كمبيوتر).
- **كروت تفاعلية**: عرض الحالات بألوان واضحة (أخضر للتشغيل، أحمر للإيقاف).

### ⚙️ إدارة متقدمة
- **حفظ الإعدادات**: تخزين SSID، كلمة السر، وأسماء المخارج في EEPROM.
- **النسخ الاحتياطي**: تنزيل/تحميل إعدادات النظام كملف JSON.
- **نقطة وصول (AP)**: وضع التهيئة عند فشل الاتصال بالشبكة.

### 🔄 الاستعادة والإصلاح
- **إعادة ضبط المصنع**: مسح كل الإعدادات باستخدام زر مؤكد.
- **استعادة ذكية**: استيراد الإعدادات من نسخة احتياطية مع التحقق من التنسيق.

## 📦 المتطلبات

### العتاد
- ESP32 (مع 30+ دبوس)
- مخارج (LED/ريليه/إلخ)

### البرمجيات
| المكتبة | الإصدار |
|---------|---------|
| `ArduinoJson` | 7.4.1 |
| `WebSocketsServer` | 2.3.6 |
| `SPIFFS` | 1.0 |

## 🛠️ التثبيت

1. **رفع الملفات إلى SPIFFS**:
   - استخدم أداة [ESP32 Sketch Data Upload](https://github.com/me-no-dev/arduino-esp32fs-plugin)
   - تأكد من وجود:
     - `index.html`
     - `monitor.html` 
     - `style.css`
     - `script.js`

2. **تضمين المكتبات**:
   ```cpp
   #include <WebServer.h>
   #include <SPIFFS.h>
   #include <ArduinoJson.h>
   ```

## 🖥️ الاستخدام

### الوصول للواجهة
1. اتصل بشبكة ESP32 (الاسم: `ESP32_Config`، كلمة المرور: `12345678`)
2. افتح المتصفح: `http://192.168.4.1`
3. أدخل إعدادات الشبكة وحفظها

### الأوامر الرئيسية
| الزر | الوظيفة |
|------|---------|
| 🌓 | تبديل الوضع الليلي |
| 🔄 | تحديث أسماء المخارج |
| 🧨 | إعادة ضبط المصنع |
| 📥 | تنزيل النسخة الاحتياطية |

```markdown
# ESP32 WebSocket Server with SPIFFS Control Panel 🚀

## ✨ الميزات الرئيسية (تم التحديث)

### 🌐 واجهة REST API للتحكم الخارجي
- **دمج مع أنظمة خارجية**: التحكم بالمخارج عبر طلبات HTTP مباشرة من:
  - تطبيقات الجوال 📱
  - أدوات مثل Postman/Insomnia 🛠️
  - أنظمة IoT أخرى (مثل Node-RED) 🤖

#### الـ Endpoints المضافة:
| النوع | المسار | الوصف | مثال |
|-------|---------|---------|-------|
| `GET` | `/api/output` | التحكم الفردي بالمخارج | `?pin=0&state=on` |
| `GET` | `/api/status` | جلب الحالة الكاملة للمخارج | - |

#### أمثلة استخدام API:
```http
# تشغيل المخرج 1
GET http://192.168.1.200/api/output?pin=0&state=on

# إيقاف المخرج 2 مع مؤقت 30 ثانية
GET http://192.168.1.200/api/output?pin=1&state=on&timer=30

# جلب الحالة الكاملة (JSON Response)
GET http://192.168.1.200/api/status
```

#### نموذج الاستجابة من `/api/status`:
```json
{
  "outputs": [
    {"pin": 0, "state": "on"},
    {"pin": 1, "state": "off"},
    {"pin": 2, "state": "on"},
    {"pin": 3, "state": "off"}
  ]
}
```

---

## 🛠️ التثبيت (تم التحديث)

### مكتبات مطلوبة جديدة:
| المكتبة | الوظيفة |
|---------|---------|
| `WebServer.h` | لإنشاء نقاط نهاية API |

---

## 📚 الهيكل البرمجي (تم التحديث)

```cpp
// مثال كود الـ API Handler
void handleApiOutput() {
  if (server.hasArg("pin") && server.hasArg("state")) {
    int pin = server.arg("pin").toInt();
    String state = server.arg("state");
    // ... معالجة الأمر
  }
}
```

---

## 🔄 أمثلة استخدام متقدمة

### التكامل مع Python:
```python
import requests

def toggle_output(pin, state):
    url = f"http://192.168.1.200/api/output?pin={pin}&state={state}"
    response = requests.get(url)
    return response.json()
```

### التكامل مع Node.js:
```javascript
const axios = require('axios');

async function getStatus() {
    const response = await axios.get('http://192.168.1.200/api/status');
    return response.data.outputs;
}
```

---

``` 

تم تحديث ملف README.md ليعكس هذه الإضافات الجديدة مع الحفاظ على التناسق البصري والمعلومات الأساسية.
# مثال: تشغيل المخرج 1 لمدة 30 ثانية
curl -X POST http://192.168.1.2/api/output -d "pin=0&state=on&timer=30"
```

## 📚 الهيكل البرمجي

```
/ESP32-websocket-server-spiffs
├── ESP32-websocket-server-spiffs.ino  # الكود الرئيسي
├── data/               # ملفات الواجهة
│   ├── index.html
│   ├── monitor.html
│   ├── style.css
│   └── script.js
└── README.md           # هذا الملف
```

## ⚠️ استكشاف الأخطاء

- **"SPIFFS mount failed"**:
  ```cpp
  if (!SPIFFS.begin(true)) {
    ESP.restart(); // إعادة التشغيل التلقائي
  }
  ```
- **"WebSocket disconnected"**:
  - تأكد من استخدام المنفذ 81 (`ws://IP:81`)

## 🤝 المساهمة
المساهمات مرحب بها! قم بفتح:
- [Issue](https://github.com/kutaibaa-akraa/ESP32-websocket-server-spiffs/issues) للإبلاغ عن الأخطاء
- [Pull Request](https://github.com/kutaibaa-akraa/ESP32-websocket-server-spiffs/pulls) للإضافات الجديدة

## 📜 الترخيص
[GPL-2.0 License](LICENSE) - مشروع مفتوح المصدر للاستخدام التجاري والشخصي.

---

**النسخة 0.0.1** - تم التحديث في 2025-04-30  
```