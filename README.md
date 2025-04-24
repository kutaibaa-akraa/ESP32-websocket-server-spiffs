# ESP32-websocket-server-spiffs
 كود بلغة Arduino (C++) لشريحة مثل ESP32 ذات 30 بن، يقوم بما يلي:  1. التحكم بأربعة مخارج (مثل LEDs أو ريليهات).  2. استخدام WebSocket للتفاعل اللحظي مع الواجهة.  3. إمكانية رفع ملفات إلى SPIFFS (نظام الملفات).  4. صفحة إعدادات تمكن المستخدم من ضبط إعدادات الاتصال مثل SSID وكلمة السر وIP ثابت. 
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>ESP32 Control Panel</title>
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <style>
    body {
      font-family: Arial, sans-serif;
      margin: 0;
      padding: 20px;
      background: #f4f4f4;
      transition: background 0.3s, color 0.3s;
    }
    .dark-mode {
      background-color: #121212;
      color: #f4f4f4;
    }
    .dark-mode input, .dark-mode button {
      background-color: #1e1e1e;
      color: #eee;
      border: 1px solid #444;
    }

    h2 { text-align: center; }

    .controls {
      display: flex;
      flex-wrap: wrap;
      justify-content: center;
      gap: 16px;
      margin-bottom: 20px;
    }

    .card {
      background: white;
      padding: 16px;
      border-radius: 12px;
      box-shadow: 0 0 10px rgba(0,0,0,0.1);
      text-align: center;
      width: 240px;
    }

    .dark-mode .card {
      background-color: #1e1e1e;
    }

    .btn {
      padding: 10px 20px;
      font-size: 16px;
      margin-top: 10px;
      cursor: pointer;
      border: none;
      border-radius: 6px;
    }

    .on { background-color: green; color: white; }
    .off { background-color: red; color: white; }

    .timer-input {
      width: 60px;
      margin-top: 8px;
    }

    #settingsToggle {
      display: block;
      margin: 0 auto 20px;
      padding: 10px 16px;
    }

    #settingsForm {
      max-width: 400px;
      margin: auto;
      display: none;
    }

    form input[type="text"], form input[type="number"] {
      width: 100%;
      margin: 4px 0;
      padding: 6px;
      border-radius: 4px;
      border: 1px solid #ccc;
    }

    form hr { margin: 16px 0; }
  </style>
</head>
<body>
  <button onclick="toggleDarkMode()">🌓 Toggle Dark Mode</button>
  <button id="settingsToggle" onclick="toggleSettings()">⚙️ Show/Hide Settings</button>

  <h2>ESP32 Output Control</h2>

  <div style="text-align:center; margin-bottom: 20px;">
    <button class="btn on" onclick="toggleAll('on')">🔛 Turn All ON</button>
    <button class="btn off" onclick="toggleAll('off')">🔘 Turn All OFF</button>
  </div>

  <div class="controls">
    <div class="card">
      <h3 id="label0">Output 1</h3>
      <button class="btn off" onclick="handleButton(0)" id="btn0">Toggle</button><br>
      <input class="timer-input" type="number" id="timer0" placeholder="sec">
    </div>
    <div class="card">
      <h3 id="label1">Output 2</h3>
      <button class="btn off" onclick="handleButton(1)" id="btn1">Toggle</button><br>
      <input class="timer-input" type="number" id="timer1" placeholder="sec">
    </div>
    <div class="card">
      <h3 id="label2">Output 3</h3>
      <button class="btn off" onclick="handleButton(2)" id="btn2">Toggle</button><br>
      <input class="timer-input" type="number" id="timer2" placeholder="sec">
    </div>
    <div class="card">
      <h3 id="label3">Output 4</h3>
      <button class="btn off" onclick="handleButton(3)" id="btn3">Toggle</button><br>
      <input class="timer-input" type="number" id="timer3" placeholder="sec">
    </div>
  </div>

  <form method="POST" action="/save" id="settingsForm">
    <h3>WiFi & Output Settings</h3>
    SSID: <input type="text" name="ssid"><br>
    Password: <input type="text" name="pass"><br>
    Static IP: <input type="text" name="ip"><br>
    <hr>
    Output 1 Name: <input type="text" name="name0"><br>
    Output 2 Name: <input type="text" name="name1"><br>
    Output 3 Name: <input type="text" name="name2"><br>
    Output 4 Name: <input type="text" name="name3"><br>
    <br><input type="submit" value="Save & Restart">
	<hr>
<button type="button" onclick="factoryReset()" style="background:#d9534f; color:white; border:none; padding:10px 16px; border-radius:6px; cursor:pointer;">
  🧨 Reset to Factory
</button>
  </form>


  <script>
    const ws = new WebSocket(`ws://${location.hostname}:81`);
    const buttons = document.querySelectorAll('.btn');

    function toggleDarkMode() {
      document.body.classList.toggle('dark-mode');
      localStorage.setItem('darkMode', document.body.classList.contains('dark-mode') ? '1' : '0');
    }

    function toggleSettings() {
      const form = document.getElementById('settingsForm');
      form.style.display = (form.style.display === 'none' || form.style.display === '') ? 'block' : 'none';
    }

    if (localStorage.getItem('darkMode') === '1') {
      document.body.classList.add('dark-mode');
    }

    ws.onopen = () => {
      console.log('✅ WebSocket connected');
      fetch('/labels.json')
        .then(res => res.json())
        .then(data => {
          if (data.labels && Array.isArray(data.labels)) {
            data.labels.forEach((label, i) => {
              const el = document.getElementById(`label${i}`);
              if (el) el.innerText = label;
            });
          }
        });
    };

    ws.onerror = (e) => console.error('❌ WebSocket error', e);
    ws.onclose = () => console.warn('⚠️ WebSocket disconnected');

    ws.onmessage = (e) => {
      const [index, state] = e.data.split(':');
      const btn = document.getElementById(`btn${index}`);
      if (btn) {
        btn.className = 'btn ' + (state === '1' ? 'on' : 'off');
      }
    };

    function handleButton(index) {
      const timerVal = document.getElementById(`timer${index}`).value;
      if (ws.readyState === WebSocket.OPEN) {
        if (timerVal && Number(timerVal) > 0) {
          ws.send(`${index}:${timerVal}`);
        } else {
          ws.send(index.toString());
        }
      }
    }

    function toggleAll(state) {
      if (ws.readyState === WebSocket.OPEN) {
        ws.send(`all:${state}`);
      }
    }

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

  </script>
</body>
</html>
