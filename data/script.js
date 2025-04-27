// فتح اتصال WebSocket مع ESP32
const ws = new WebSocket(`ws://${location.hostname}:81`);

// عند فتح الاتصال
ws.onopen = () => {
  console.log('✅ WebSocket connected');
  refreshLabels();
};

// عند استقبال رسالة من السيرفر
ws.onmessage = (event) => {
  const [index, state] = event.data.split(":");
  const button = document.getElementById(`btn${index}`);
  if (button) {
    button.className = "btn " + (state === "1" ? "on" : "off");
  }
};

// التعامل مع أخطاء الاتصال
ws.onerror = (e) => console.error('❌ WebSocket error', e);

// عند غلق الاتصال
ws.onclose = () => console.warn('⚠️ WebSocket disconnected');

// دالة إرسال أمر تشغيل/إيقاف لمخرج محدد
function handleButton(index) {
  const timerVal = document.getElementById(`timer${index}`).value;
  if (ws.readyState === WebSocket.OPEN) {
    if (timerVal && Number(timerVal) > 0) {
      ws.send(`${index}:${timerVal}`);
    } else {
      ws.send(index.toString());
    }
  } else {
    console.warn('WebSocket not connected');
  }
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
      if (data.labels && Array.isArray(data.labels)) {
        data.labels.forEach((label, i) => {
          const labelElement = document.getElementById(`label${i}`);
          if (labelElement) labelElement.innerText = label;
        });
      }
    })
    .catch(() => console.warn('Could not fetch labels.'));
}

// الوضع الليلي
function toggleDarkMode() {
  document.body.classList.toggle('dark-mode');
  localStorage.setItem('darkMode', document.body.classList.contains('dark-mode') ? '1' : '0');
}

if (localStorage.getItem('darkMode') === '1') {
  document.body.classList.add('dark-mode');
}

// إظهار/إخفاء الإعدادات
function toggleSettings() {
  const form = document.getElementById('settingsForm');
  form.style.display = (form.style.display === 'none' || form.style.display === '') ? 'block' : 'none';
}

// إعادة ضبط المصنع
function factoryReset() {
  if (confirm("هل أنت متأكد أنك تريد إعادة ضبط المصنع؟")) {
    fetch("/reset")
      .then(res => res.text())
      .then(msg => {
        alert(msg);
        setTimeout(() => location.reload(), 2000);
      })
      .catch(() => alert("حدث خطأ أثناء إعادة الضبط."));
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
