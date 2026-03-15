const mockState = {
  routes: [
    { name: '洋山 → 新加坡', eta: '13:20', status: '在途' },
    { name: '宁波 → 釜山', eta: '14:05', status: '待靠泊' },
    { name: '青岛 → 洛杉矶', eta: '16:40', status: '装卸中' }
  ],
  typhoonTrack: [
    { x: 42, y: 136, t: '08:00' },
    { x: 80, y: 120, t: '11:00' },
    { x: 128, y: 97, t: '14:00' },
    { x: 182, y: 84, t: '17:00' },
    { x: 246, y: 66, t: '20:00' }
  ],
  alarms: [
    '风速偏高：东码头 18.2m/s',
    '堆场温度异常：B3 区域 42°C'
  ],
  kpi: { vessels: 12, teu: 18420, devices: 67, highAlarm: 2 },
  yard: [
    { area: 'A1', rate: '72%', count: 1480 },
    { area: 'A2', rate: '88%', count: 2055 },
    { area: 'B1', rate: '63%', count: 1268 },
    { area: 'B2', rate: '79%', count: 1640 }
  ]
};

const appConfig = {
  apiBaseUrl: 'http://localhost:5000/api',
  wsUrl: 'ws://localhost:5000/ws'
};

let socket = null;

function byId(id) {
  return document.getElementById(id);
}

function renderRoutes() {
  byId('routeList').innerHTML = mockState.routes
    .map(r => `<li><b>${r.name}</b><br/>ETA: ${r.eta} ｜ 状态: ${r.status}</li>`)
    .join('');
}

function renderTyphoonMap() {
  const map = byId('typhoonMap');
  const points = mockState.typhoonTrack;
  const polylinePoints = points.map(p => `${p.x},${p.y}`).join(' ');
  map.innerHTML = `
    <polyline points="${polylinePoints}" fill="none" stroke="#ffb347" stroke-width="3" stroke-dasharray="6 4" />
    ${points
      .map(
        p => `<g>
          <circle cx="${p.x}" cy="${p.y}" r="5" fill="#ff6e6e" />
          <text x="${p.x + 6}" y="${p.y - 8}" fill="#cce7ff" font-size="10">${p.t}</text>
        </g>`
      )
      .join('')}
  `;
}

function renderAlarms() {
  byId('alarmList').innerHTML = mockState.alarms.map(a => `<li>${a}</li>`).join('');
}

function renderKpis() {
  byId('kpiVessel').textContent = mockState.kpi.vessels;
  byId('kpiTeu').textContent = mockState.kpi.teu.toLocaleString();
  byId('kpiDevice').textContent = mockState.kpi.devices;
  byId('kpiAlarm').textContent = mockState.kpi.highAlarm;
}

function renderYard() {
  byId('yardTable').innerHTML = mockState.yard
    .map(y => `<tr><td>${y.area}</td><td>${y.rate}</td><td>${y.count}</td></tr>`)
    .join('');
}

function addLog(line) {
  const log = byId('cmdLog');
  const time = new Date().toLocaleTimeString();
  const li = document.createElement('li');
  li.textContent = `[${time}] ${line}`;
  log.prepend(li);
  if (log.children.length > 12) {
    log.removeChild(log.lastChild);
  }
}

function setConnectionStatus(connected) {
  const el = byId('connStatus');
  el.textContent = connected ? '已连接' : '未连接';
  el.classList.toggle('connected', connected);
  el.classList.toggle('disconnected', !connected);
}

async function postCommand(kind, command) {
  const payload = {
    commandId: crypto.randomUUID(),
    kind,
    command,
    timestamp: new Date().toISOString(),
    source: 'port-ui'
  };

  try {
    const response = await fetch(`${appConfig.apiBaseUrl}/commands/execute`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(payload)
    });

    if (!response.ok) {
      throw new Error(`HTTP ${response.status}`);
    }

    addLog(`${kind} 命令已发送：${command}`);
  } catch (error) {
    addLog(`命令发送失败（已走本地演示）：${command}`);
    console.warn('postCommand fallback:', error);
  }
}

function bindButtons() {
  document.querySelectorAll('[data-scene-cmd]').forEach(btn => {
    btn.addEventListener('click', () => {
      const cmd = btn.getAttribute('data-scene-cmd');
      postCommand('scene', cmd);
      addLog(`触发 UE 场景命令：${cmd}`);
    });
  });

  document.querySelectorAll('[data-device-cmd]').forEach(btn => {
    btn.addEventListener('click', () => {
      const cmd = btn.getAttribute('data-device-cmd');
      postCommand('device', cmd);
      addLog(`触发硬件命令：${cmd}`);
    });
  });
}

function connectWebSocket() {
  if (socket && socket.readyState === WebSocket.OPEN) {
    addLog('连接已存在，无需重复连接');
    return;
  }

  socket = new WebSocket(appConfig.wsUrl);
  addLog(`正在连接：${appConfig.wsUrl}`);

  socket.onopen = () => {
    setConnectionStatus(true);
    addLog('实时通道连接成功');
  };

  socket.onmessage = event => {
    addLog(`收到实时消息：${event.data}`);
  };

  socket.onclose = () => {
    setConnectionStatus(false);
    addLog('实时通道已断开');
  };

  socket.onerror = () => {
    addLog('实时通道连接失败（保留离线演示）');
  };
}

function init() {
  renderRoutes();
  renderTyphoonMap();
  renderAlarms();
  renderKpis();
  renderYard();
  bindButtons();

  byId('connectBtn').addEventListener('click', connectWebSocket);
  addLog('大屏初始化完成，可先离线演示，后续替换 API/WS 地址即可对接');
}

init();
