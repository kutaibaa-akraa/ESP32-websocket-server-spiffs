// فتح اتصال WebSocket مع ESP32
// const ws = new WebSocket(`ws://${location.hostname}:81`);
const ws = new WebSocket('ws://esp32-control.local:81'); // <-- استخدام اسم mDNS

// إعادة فتح WebSocket عند انقطاعه
function reconnectWebSocket() {
  setTimeout(() => {
    if (ws.readyState === WebSocket.CLOSED) {
      ws = new WebSocket('ws://esp32-control.local:81');
      ws.onmessage = (event) => { /* ... */ };
    }
  }, 3000); // حاول إعادة الاتصال كل 3 ثواني
}

// تحديث البيانات كل 5 ثواني
setInterval(() => {
  fetch('/api/status')
    .then(res => res.json())
    .then(data => {
      // تحديث كل مخرج بناءً على البيانات
      data.outputs.forEach((output, index) => {
        const stateEl = document.getElementById(`state${index}`);
        if (stateEl) {
          stateEl.innerText = output.state.toUpperCase();
          stateEl.className = output.state === "on" ? "mon" : "moff";
        }
      });
    })
    .catch(() => console.log("فشل تحديث البيانات!"));
}, 5000); // كل 5000 مللي ثانية (5 ثواني)

ws.onclose = () => {
  console.log("WebSocket مغلق! جاري إعادة الاتصال...");
  reconnectWebSocket();
};

// عند فتح الاتصال
ws.onopen = () => {
  console.log('✅ WebSocket connected');
  refreshLabels(); // <-- تحديث الأسماء والحالات
};

// عند استقبال رسالة من السيرفر
// إعادة تعيين معالج الأحداث
ws.onmessage = (event) => {
  const [index, state] = event.data.split(":");
  const button = document.getElementById(`btn${index}`);
  if (button) {
    button.className = "btn " + (state === "1" ? "on" : "off");
  }
   const el = document.getElementById(`state${index}`);
      if (el) {
        el.innerText = state === "1" ? "ON" : "OFF";
        el.className = state === "1" ? "mon" : "moff";
      }
};

// التعامل مع أخطاء الاتصال
ws.onerror = (e) => console.error('❌ WebSocket error', e);

// عند غلق الاتصال
ws.onclose = () => console.warn('⚠️ WebSocket disconnected');

// دالة إرسال أمر تشغيل/إيقاف لمخرج محدد
// function handleButton(index) {
//  const timerVal = document.getElementById(`timer${index}`).value;
 // if (ws.readyState === WebSocket.OPEN) {
  //  if (timerVal && Number(timerVal) > 0) {
   //   ws.send(`${index}:${timerVal}`);
    //} else {
     // ws.send(index.toString());
  //  }
 // } else {
   // console.warn('WebSocket not connected');
 // }
// }

let isProcessing = false; // متغير لمنع الطلبات المتداخلة

function handleButton(index) {
  if (isProcessing) return;
  isProcessing = true;
  
  const timerVal = document.getElementById(`timer${index}`).value;
  if (ws.readyState === WebSocket.OPEN) {
    if (timerVal && Number(timerVal) > 0) {
      ws.send(`${index}:${timerVal}`);
    } else {
      ws.send(index.toString());
    }
  }
  
  setTimeout(() => { isProcessing = false; }, 200); // تأخير 200 مللي ثانية
}


// دالة تشغيل/إيقاف الكل
function toggleAll(state) {
  if (ws.readyState === WebSocket.OPEN) {
    ws.send(`all:${state}`);
  }
}

// دالة تحديث أسماء المخارج من السيرفر
function refreshLabels() {
  fetch('/labels.json')
    .then(res => res.json())
    .then(data => {
      // تحديث الأسماء
      if (data.labels && Array.isArray(data.labels)) {
        data.labels.forEach((label, i) => {
          const labelElement = document.getElementById(`label${i}`);
          if (labelElement) labelElement.innerText = label;
        });
      }
      // إرسال طلب للحصول على الحالات الحالية
      fetch('/api/status')
        .then(res => res.json())
        .then(status => {
          status.outputs.forEach((output, i) => {
            const btn = document.getElementById(`btn${i}`);
            const stateEl = document.getElementById(`state${i}`);
            if (btn) {
              btn.className = "btn " + (output.state === "on" ? "on" : "off");
            }
            if (stateEl) {
              stateEl.innerText = output.state.toUpperCase();
              stateEl.className = output.state === "on" ? "mon" : "moff";
            }
          });
        });
    })
    .catch(() => console.warn('Could not fetch labels.'));
}


// الوضع الليلي صفحة المونيتور
    function toggleDark() {
      document.body.classList.toggle("dark");
      localStorage.setItem("darkMode", document.body.classList.contains("dark") ? "1" : "0");
    }

    if (localStorage.getItem("darkMode") === "1") {
      document.body.classList.add("dark");
    }

// إظهار/إخفاء الإعدادات
function toggleSettings() {
  const form = document.getElementById('settingsForm');
  form.style.display = (form.style.display === 'none' || form.style.display === '') ? 'block' : 'none';
}

// إعادة ضبط المصنع
function factoryReset() {
  if (confirm("هل أنت متأكد من إعادة الضبط إلى المصنع؟")) {
    fetch("/reset")
      .then(() => {
        alert("✅ تمت إعادة الضبط بنجاح. سيتم إعادة تحميل الصفحة.");
        setTimeout(() => {
          location.reload();
          refreshLabels(); // تحديث الأسماء بعد التحميل
        }, 2000);
      })
      .catch(() => alert("❌ فشل في إعادة الضبط!"));
  }
}

// عرض الوقت والتاريخ
function updateClock() {
  const now = new Date();
  const formatted = now.toLocaleString();
  const datetime = document.getElementById('datetime');
  if (datetime) {
    datetime.innerText = formatted;
  }
}
setInterval(updateClock, 1000);
updateClock();

function downloadBackup() {
  window.open("/backup", "_blank");
}

let uploadedContent = "";

function uploadBackup() {
  const fileInput = document.getElementById('uploadFile');
  const file = fileInput.files[0];
  if (!file) return;

  const reader = new FileReader();
  reader.onload = function(event) {
    uploadedContent = event.target.result;
    document.getElementById('warningBox').style.display = "flex";
  };
  reader.readAsText(file);
}

function confirmRestore(confirmed) {
  document.getElementById('warningBox').style.display = "none";
  if (!confirmed) {
    document.getElementById('uploadFile').value = "";
    uploadedContent = "";
    return;
  }

  const xhr = new XMLHttpRequest();
  xhr.open("POST", "/restore", true);
  xhr.setRequestHeader("Content-Type", "application/json");

  xhr.upload.onprogress = function(e) {
    if (e.lengthComputable) {
      const percent = (e.loaded / e.total) * 100;
      document.getElementById('progressContainer').style.display = "block";
      document.getElementById('uploadProgress').value = percent;
    }
  };

  xhr.onload = function() {
    document.getElementById('progressContainer').style.display = "none";
    if (xhr.status == 200) {
      alert("✅ تم استعادة الإعدادات بنجاح، سيتم إعادة التشغيل.");
      setTimeout(() => location.reload(), 5000);
    } else {
      alert("❌ حدث خطأ أثناء الاستعادة!");
    }
  };

  xhr.onerror = function() {
    document.getElementById('progressContainer').style.display = "none";
    alert("❌ فشل رفع النسخة الاحتياطية.");
  };

  xhr.send(uploadedContent);
}


function resetAP() {  // إعادة الضبط إلى وضع AP
  fetch("/resetAP").then(() => location.reload());
}