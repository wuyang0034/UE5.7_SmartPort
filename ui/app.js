const mockState = {
  portName: 'Smart Port Alpha',
  routes: [
    { name: '洋山 → 新加坡', vessel: 'MV COSCO NEBULA', eta: '13:20', status: '在途' },
    { name: '宁波 → 釜山', vessel: 'MV HANSEATIC STAR', eta: '14:05', status: '待靠泊' },
    { name: '青岛 → 洛杉矶', vessel: 'MV PACIFIC LANE', eta: '16:40', status: '装卸中' }
  ],
  typhoonTrack: [
    { x: 42, y: 136, timeLabel: '08:00', windSpeed: 14 },
    { x: 80, y: 120, timeLabel: '11:00', windSpeed: 16 },
    { x: 128, y: 97, timeLabel: '14:00', windSpeed: 19 },
    { x: 182, y: 84, timeLabel: '17:00', windSpeed: 22 },
    { x: 246, y: 66, timeLabel: '20:00', windSpeed: 24 }
  ],
  alarms: [
    { severity: 'warning', source: 'WeatherEngine', message: '东码头阵风增强，请关注桥吊防风状态。', timestamp: new Date().toISOString() }
  ],
  broadcasts: [
    { severity: 'info', title: 'AI 智能播报', content: '系统已启动，可通过连接后端接收实时天气、台风和设备联动播报。', timestamp: new Date().toISOString() }
  ],
  weather: { condition: '阵雨', windSpeed: 17.2, rainfall: 5.6, visibilityKm: 11.2 },
  kpi: { vesselsInPort: 12, dailyTeu: 18420, onlineDevices: 67, highPriorityAlarms: 1 },
  yard: [
    { area: 'A1', occupancy: 67.3, containerCount: 1480 },
    { area: 'A2', occupancy: 87.4, containerCount: 2055 },
    { area: 'B1', occupancy: 63.4, containerCount: 1268 },
    { area: 'B2', occupancy: 78.8, containerCount: 1640 }
  ]
};

const appConfig = {
  apiBaseUrl: 'http://localhost:5000/api',
  wsUrl: 'ws://localhost:5000/ws'
};

let socket = null;
let sessionId = crypto.randomUUID();

function byId(id) {
  return document.getElementById(id);
}

function escapeHtml(value) {
  return String(value)
    .replaceAll('&', '&amp;')
    .replaceAll('<', '&lt;')
    .replaceAll('>', '&gt;')
    .replaceAll('"', '&quot;')
    .replaceAll("'", '&#39;');
}

function applySnapshot(snapshot) {
  if (!snapshot) return;
  mockState.portName = snapshot.portName ?? mockState.portName;
  mockState.routes = snapshot.routes ?? mockState.routes;
  mockState.typhoonTrack = snapshot.typhoonTrack ?? mockState.typhoonTrack;
  mockState.alarms = snapshot.alarms ?? mockState.alarms;
  mockState.broadcasts = snapshot.broadcasts ?? mockState.broadcasts;
  mockState.weather = snapshot.weather ?? mockState.weather;
  mockState.kpi = snapshot.kpi ?? mockState.kpi;
  mockState.yard = (snapshot.yard ?? mockState.yard).map(item => ({
    area: item.area,
    occupancy: item.occupancy ?? Number(item.rate?.replace('%', '')) ?? 0,
    containerCount: item.containerCount ?? item.count ?? 0
  }));

  renderAll();
}

function renderRoutes() {
  byId('routeList').innerHTML = mockState.routes
    .map(r => `<li><b>${escapeHtml(r.name)}</b><br/>船舶: ${escapeHtml(r.vessel || '-')}<br/>ETA: ${escapeHtml(r.eta)} ｜ 状态: ${escapeHtml(r.status)}</li>`)
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
          <text x="${p.x + 6}" y="${p.y - 8}" fill="#cce7ff" font-size="10">${escapeHtml(p.timeLabel || p.t || '')}</text>
        </g>`
      )
      .join('')}
  `;
}

function renderAlarms() {
  byId('alarmList').innerHTML = mockState.alarms
    .slice(0, 6)
    .map(a => `<li class="alarm-${escapeHtml(a.severity || 'info')}"><b>[${escapeHtml(a.source || 'System')}]</b> ${escapeHtml(a.message)}</li>`)
    .join('');
}

function renderBroadcasts() {
  byId('broadcastList').innerHTML = mockState.broadcasts
    .slice()
    .reverse()
    .slice(0, 6)
    .map(item => `<li class="alarm-${escapeHtml(item.severity || 'info')}"><b>${escapeHtml(item.title || '播报')}</b><br/>${escapeHtml(item.content)}</li>`)
    .join('');

  const latest = mockState.broadcasts[mockState.broadcasts.length - 1];
  const banner = byId('broadcastBanner');
  if (latest) {
    banner.textContent = latest.content;
    banner.className = `broadcast-banner ${latest.severity || 'info'}`;
  }
}

function renderWeather() {
  byId('weatherCondition').textContent = mockState.weather.condition;
  byId('weatherWind').textContent = `${mockState.weather.windSpeed} m/s`;
  byId('weatherRain').textContent = `${mockState.weather.rainfall} mm`;
  byId('weatherVisibility').textContent = `${mockState.weather.visibilityKm} km`;
}

function renderKpis() {
  byId('kpiVessel').textContent = mockState.kpi.vesselsInPort;
  byId('kpiTeu').textContent = Number(mockState.kpi.dailyTeu).toLocaleString();
  byId('kpiDevice').textContent = mockState.kpi.onlineDevices;
  byId('kpiAlarm').textContent = mockState.kpi.highPriorityAlarms;
}

function renderYard() {
  byId('yardTable').innerHTML = mockState.yard
    .map(y => `<tr><td>${escapeHtml(y.area)}</td><td>${Number(y.occupancy).toFixed(1)}%</td><td>${Number(y.containerCount).toLocaleString()}</td></tr>`)
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

function addChat(role, text) {
  const chatLog = byId('chatLog');
  const time = new Date().toLocaleTimeString();
  const item = document.createElement('div');
  item.className = `chat-item ${role}`;
  item.innerHTML = `<div class="chat-meta">${role === 'user' ? '你' : 'AI 值班员'} · ${time}</div><div>${escapeHtml(text).replaceAll('\n', '<br/>')}</div>`;
  chatLog.appendChild(item);
  chatLog.scrollTop = chatLog.scrollHeight;
}

function renderSuggestions(suggestions = []) {
  const row = byId('suggestionRow');
  row.innerHTML = suggestions
    .map(text => `<button class="suggestion-chip" type="button">${escapeHtml(text)}</button>`)
    .join('');

  row.querySelectorAll('.suggestion-chip').forEach(button => {
    button.addEventListener('click', () => {
      byId('chatInput').value = button.textContent;
      byId('chatForm').dispatchEvent(new Event('submit', { cancelable: true, bubbles: true }));
    });
  });
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

async function fetchSnapshot() {
  try {
    const response = await fetch(`${appConfig.apiBaseUrl}/twin/snapshot`);
    if (!response.ok) throw new Error(`HTTP ${response.status}`);
    applySnapshot(await response.json());
    addLog('已同步最新数字孪生快照');
  } catch (error) {
    addLog('获取快照失败，使用本地模拟数据');
  }
}

async function askAgent(message) {
  addChat('user', message);

  try {
    const response = await fetch(`${appConfig.apiBaseUrl}/chat`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ message, sessionId, includeSnapshot: true })
    });

    if (!response.ok) {
      throw new Error(`HTTP ${response.status}`);
    }

    const data = await response.json();
    sessionId = data.sessionId || sessionId;
    byId('agentProvider').textContent = `${data.provider} · ${data.model}`;
    addChat('agent', data.reply);
    renderSuggestions(data.suggestedActions || []);
    applySnapshot(data.snapshot);
  } catch (error) {
    console.warn('askAgent fallback:', error);
    addChat('agent', '后端智能体暂不可用，当前保留本地演示模式。建议先启动 .NET 后端，再接入 Ollama 免费模型。');
    renderSuggestions(['查看台风处置建议', '汇总当前告警', '查询堆场占用率']);
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

  byId('chatForm').addEventListener('submit', event => {
    event.preventDefault();
    const input = byId('chatInput');
    const message = input.value.trim();
    if (!message) return;
    input.value = '';
    askAgent(message);
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
    try {
      const message = JSON.parse(event.data);
      if (message.type === 'snapshot') {
        applySnapshot(message.payload);
        if (message.broadcast) {
          mockState.broadcasts.push(message.broadcast);
          renderBroadcasts();
          addChat('agent', message.broadcast.content);
        }
      } else if (message.type === 'commandAck') {
        addLog(`命令确认：${message.payload.command} / ${message.payload.status}`);
      } else {
        addLog(`收到实时消息：${event.data}`);
      }
    } catch {
      addLog(`收到实时消息：${event.data}`);
    }
  };

  socket.onclose = () => {
    setConnectionStatus(false);
    addLog('实时通道已断开');
  };

  socket.onerror = () => {
    addLog('实时通道连接失败（保留离线演示）');
  };
}

function renderAll() {
  renderRoutes();
  renderTyphoonMap();
  renderAlarms();
  renderBroadcasts();
  renderWeather();
  renderKpis();
  renderYard();
}

function init() {
  renderAll();
  bindButtons();
  renderSuggestions(['查看台风处置建议', '汇总当前告警', '查询堆场占用率']);
  addChat('agent', '我是港口数字孪生智能体。当前已加载模拟天气、台风、堆场和设备数据。连接后端后，我可以基于实时快照自动播报突发事件。');

  byId('connectBtn').addEventListener('click', () => {
    fetchSnapshot();
    connectWebSocket();
  });

  addLog('大屏初始化完成，可先离线演示，后续连接 .NET 后端与 Ollama 即可升级为实时系统');
}

init();
