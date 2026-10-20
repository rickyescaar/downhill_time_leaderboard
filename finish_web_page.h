#ifndef FINISH_WEB_PAGE_H
#define FINISH_WEB_PAGE_H

#include <Arduino.h>

const char FINISH_HTML[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html lang="id">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Downhill Time Trial System</title>
  <style>
    :root {
      --bg: #0d111b;
      --card-bg: #151c2a;
      --card-border: #202b3d;
      --cyan: #00f0ff;
      --cyan-dim: rgba(0, 240, 255, 0.15);
      --amber: #f59e0b;
      --amber-dim: rgba(245, 158, 11, 0.12);
      --green: #10b981;
      --green-dim: rgba(16, 185, 129, 0.15);
      --red: #ef4444;
      --red-dim: rgba(239, 68, 68, 0.15);
      --gold: #fbbf24;
      --silver: #cbd5e1;
      --bronze: #d97706;
      --text: #f8fafc;
      --text-muted: #94a3b8;
      --text-dim: #64748b;
    }

    * { box-sizing: border-box; margin: 0; padding: 0; font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, "Helvetica Neue", Arial, sans-serif; }
    body {
      background-color: var(--bg);
      color: var(--text);
      padding: 16px 20px;
      min-height: 100vh;
      -webkit-font-smoothing: antialiased;
    }
    .container { max-width: 1400px; margin: 0 auto; }

    /* HEADER */
    .header {
      background: #131926;
      border: 1px solid #1e293b;
      border-radius: 12px;
      padding: 14px 20px;
      margin-bottom: 20px;
      display: flex;
      flex-wrap: wrap;
      justify-content: space-between;
      align-items: center;
      gap: 16px;
    }
    .header-left {
      display: flex;
      align-items: center;
      gap: 14px;
    }
    .badge-esp32 {
      background: #0891b2;
      color: #050e18;
      font-weight: 900;
      font-size: 0.8rem;
      padding: 4px 10px;
      border-radius: 6px;
      letter-spacing: 0.5px;
    }
    .header-title-box h1 {
      font-size: 1.25rem;
      font-weight: 800;
      color: #fff;
      letter-spacing: 0.5px;
      text-transform: uppercase;
      line-height: 1.2;
    }
    .header-title-box p {
      font-size: 0.75rem;
      color: var(--text-muted);
      margin-top: 2px;
    }
    .header-right {
      display: flex;
      align-items: center;
      gap: 18px;
      flex-wrap: wrap;
    }
    .track-indicator {
      font-size: 0.82rem;
      font-weight: 700;
      color: #e2e8f0;
      letter-spacing: 0.5px;
    }
    .track-indicator span {
      color: var(--cyan);
    }
    .status-pill {
      background: rgba(15, 23, 42, 0.7);
      border: 1px solid #334155;
      border-radius: 20px;
      padding: 6px 14px;
      font-size: 0.75rem;
      font-weight: 600;
      display: flex;
      align-items: center;
      gap: 8px;
    }
    .status-dot {
      width: 8px;
      height: 8px;
      border-radius: 50%;
      background: var(--green);
      box-shadow: 0 0 10px var(--green);
    }
    .status-dot.offline {
      background: var(--red);
      box-shadow: 0 0 10px var(--red);
    }
    .header-clock {
      font-family: "Courier New", Courier, monospace;
      font-size: 0.95rem;
      font-weight: 700;
      color: var(--text-muted);
    }

    /* MAIN TWO-COLUMN GRID */
    .main-grid {
      display: grid;
      grid-template-columns: 360px 1fr;
      gap: 20px;
    }
    @media (max-width: 1024px) {
      .main-grid { grid-template-columns: 1fr; }
    }

    /* CARD STYLING */
    .card {
      background: var(--card-bg);
      border: 1px solid var(--card-border);
      border-radius: 12px;
      padding: 18px;
      margin-bottom: 16px;
    }
    .card-head {
      display: flex;
      justify-content: space-between;
      align-items: center;
      margin-bottom: 14px;
    }
    .card-title {
      font-size: 0.75rem;
      font-weight: 700;
      color: var(--text-muted);
      text-transform: uppercase;
      letter-spacing: 0.5px;
    }

    /* CURRENT RIDER CARD */
    .status-badge {
      font-size: 0.7rem;
      font-weight: 800;
      padding: 3px 9px;
      border-radius: 6px;
      text-transform: uppercase;
      letter-spacing: 0.5px;
    }
    .badge-finished {
      background: var(--cyan-dim);
      color: var(--cyan);
      border: 1px solid rgba(0, 240, 255, 0.4);
    }
    .badge-on-track {
      background: var(--green-dim);
      color: var(--green);
      border: 1px solid rgba(16, 185, 129, 0.4);
      animation: pulse-glow 1.5s infinite;
    }
    .badge-standby {
      background: rgba(255, 255, 255, 0.06);
      color: var(--text-dim);
      border: 1px solid rgba(255, 255, 255, 0.1);
    }

    @keyframes pulse-glow {
      0%, 100% { box-shadow: 0 0 5px rgba(16, 185, 129, 0.4); }
      50% { box-shadow: 0 0 15px rgba(16, 185, 129, 0.8); }
    }

    .rider-profile {
      display: flex;
      align-items: center;
      gap: 14px;
      margin-bottom: 16px;
    }
    .bib-box {
      width: 58px;
      height: 58px;
      border: 2px solid var(--amber);
      background: var(--amber-dim);
      border-radius: 8px;
      display: flex;
      align-items: center;
      justify-content: center;
      font-family: "Courier New", Courier, monospace;
      font-size: 1.35rem;
      font-weight: 900;
      color: var(--amber);
      flex-shrink: 0;
    }
    .rider-details {
      overflow: hidden;
    }
    .rider-name {
      font-size: 1.25rem;
      font-weight: 800;
      color: #fff;
      white-space: nowrap;
      overflow: hidden;
      text-overflow: ellipsis;
      line-height: 1.2;
    }
    .rider-cat {
      font-size: 0.85rem;
      font-weight: 600;
      color: var(--cyan);
      margin-top: 2px;
      white-space: nowrap;
      overflow: hidden;
      text-overflow: ellipsis;
    }

    /* DIGITAL TIMER BOX */
    .timer-box {
      background: #080c14;
      border: 1px solid #1a2334;
      border-radius: 10px;
      padding: 22px 14px;
      text-align: center;
      margin-bottom: 14px;
      position: relative;
    }
    .timer-digits {
      font-family: "Courier New", Courier, monospace;
      font-size: 3.3rem;
      font-weight: 800;
      color: var(--cyan);
      letter-spacing: 2px;
      text-shadow: 0 0 24px rgba(0, 240, 255, 0.4);
      line-height: 1;
    }
    .timer-label {
      font-size: 0.68rem;
      font-weight: 700;
      color: var(--text-dim);
      letter-spacing: 1.5px;
      margin-top: 8px;
      text-transform: uppercase;
    }

    .rider-actions {
      display: flex;
      gap: 8px;
      margin-top: 10px;
    }
    .btn {
      padding: 8px 14px;
      border-radius: 6px;
      border: none;
      font-size: 0.75rem;
      font-weight: 700;
      cursor: pointer;
      display: inline-flex;
      align-items: center;
      justify-content: center;
      gap: 6px;
      text-transform: uppercase;
      transition: all 0.15s;
    }
    .btn:active { transform: scale(0.97); }
    .btn-finish {
      background: var(--green);
      color: #050b14;
      flex: 1;
    }
    .btn-finish:hover { filter: brightness(1.1); }
    .btn-cancel {
      background: rgba(239, 68, 68, 0.15);
      border: 1px solid var(--red);
      color: var(--red);
    }
    .btn-secondary {
      background: #1e293b;
      color: #e2e8f0;
      border: 1px solid #334155;
    }
    .btn-secondary:hover { background: #334155; }

    /* NEXT RIDER & QUEUE */
    .next-rider-text {
      font-size: 0.95rem;
      font-weight: 600;
      color: #fff;
    }
    .queue-list {
      display: flex;
      flex-direction: column;
      gap: 8px;
    }
    .queue-item {
      display: flex;
      justify-content: space-between;
      align-items: center;
      font-size: 0.85rem;
      padding: 4px 0;
      border-bottom: 1px solid rgba(255, 255, 255, 0.04);
    }
    .queue-item:last-child { border-bottom: none; }
    .queue-name {
      color: #cbd5e1;
      font-weight: 500;
    }
    .queue-status-ok {
      color: var(--green);
      font-size: 0.78rem;
      font-weight: 600;
    }
    .queue-status-dnf {
      color: var(--text-dim);
      font-size: 0.78rem;
    }

    /* RIGHT COLUMN LEADERBOARD */
    .leaderboard-stats {
      font-size: 0.78rem;
      font-weight: 700;
      color: var(--text-muted);
      letter-spacing: 0.5px;
    }

    .table-container {
      overflow-x: auto;
      margin-top: 10px;
    }
    table {
      width: 100%;
      border-collapse: collapse;
      font-size: 0.82rem;
    }
    th {
      text-align: left;
      padding: 10px 12px;
      color: var(--text-dim);
      font-size: 0.7rem;
      font-weight: 700;
      text-transform: uppercase;
      letter-spacing: 0.5px;
      border-bottom: 1px solid var(--card-border);
    }
    td {
      padding: 12px;
      border-bottom: 1px solid rgba(255, 255, 255, 0.04);
      vertical-align: middle;
      color: #e2e8f0;
    }
    tr:hover {
      background: rgba(255, 255, 255, 0.02);
    }

    .pos-gold { color: var(--gold); font-weight: 900; font-size: 0.95rem; }
    .pos-silver { color: var(--silver); font-weight: 800; font-size: 0.95rem; }
    .pos-bronze { color: var(--bronze); font-weight: 800; font-size: 0.95rem; }
    .pos-other { color: var(--text-dim); font-weight: 700; }
    .pos-nf { color: var(--text-dim); font-weight: 700; }

    .bib-cell {
      font-family: "Courier New", Courier, monospace;
      font-weight: 700;
      color: #fff;
    }
    .name-cell {
      font-weight: 600;
      color: #fff;
    }
    .time-mono {
      font-family: "Courier New", Courier, monospace;
      color: var(--text-muted);
      font-size: 0.8rem;
    }
    .duration-cell {
      font-family: "Courier New", Courier, monospace;
      font-size: 0.95rem;
      font-weight: 800;
      color: var(--cyan);
    }
    .duration-cell.nf {
      color: var(--cyan);
      font-size: 0.8rem;
      font-weight: 700;
    }
    .gap-cell {
      font-family: "Courier New", Courier, monospace;
      font-size: 0.8rem;
      color: var(--text-muted);
    }
    .status-cell {
      font-size: 0.75rem;
      font-weight: 700;
    }
    .status-cell.ok { color: #cbd5e1; }
    .status-cell.failed { color: var(--text-dim); }

    /* ACTION TOOLBAR */
    .leaderboard-footer {
      display: flex;
      justify-content: flex-end;
      align-items: center;
      gap: 10px;
      margin-top: 18px;
      flex-wrap: wrap;
    }

    /* MODAL */
    .modal-backdrop {
      display: none;
      position: fixed;
      top: 0; left: 0; width: 100%; height: 100%;
      background: rgba(0, 0, 0, 0.75);
      backdrop-filter: blur(5px);
      z-index: 999;
      justify-content: center;
      align-items: center;
      padding: 16px;
    }
    .modal-box {
      background: #111827;
      border: 1px solid var(--card-border);
      border-radius: 12px;
      padding: 20px;
      width: 100%;
      max-width: 440px;
    }
    .modal-head {
      display: flex;
      justify-content: space-between;
      align-items: center;
      margin-bottom: 14px;
    }
    .modal-title {
      font-size: 1rem;
      font-weight: 800;
      color: #fff;
    }
    .form-group {
      margin-bottom: 14px;
    }
    .form-group label {
      display: block;
      font-size: 0.72rem;
      font-weight: 700;
      color: var(--text-muted);
      margin-bottom: 6px;
      text-transform: uppercase;
    }
    input[type="text"] {
      width: 100%;
      background: #090d16;
      border: 1px solid var(--card-border);
      border-radius: 6px;
      padding: 9px 12px;
      color: #fff;
      font-size: 0.9rem;
      outline: none;
    }
    input[type="text"]:focus {
      border-color: var(--cyan);
    }
  </style>
</head>
<body>
  <div class="container">
    <!-- HEADER -->
    <header class="header">
      <div class="header-left">
        <span class="badge-esp32">ESP32</span>
        <div class="header-title-box">
          <h1 id="event-title">DOWNHILL TIME TRIAL SYSTEM</h1>
          <p>Radar + GPS + LoRa Local Precision Telemetry</p>
        </div>
      </div>
      <div class="header-right">
        <div class="track-indicator">
          TRACK: <span id="track-name">SECTOR 1</span>
        </div>
        <div class="status-pill">
          <span class="status-dot" id="system-dot"></span>
          <span id="system-status-text">ESP32 Online (Live Telemetry)</span>
        </div>
        <div class="header-clock" id="live-clock">--:--:--</div>
      </div>
    </header>

    <!-- MAIN TWO COLUMN GRID -->
    <main class="main-grid">
      <!-- LEFT COLUMN -->
      <section>
        <!-- CARD 1: CURRENT ON-TRACK RIDER -->
        <div class="card">
          <div class="card-head">
            <span class="card-title">CURRENT ON-TRACK RIDER</span>
            <span class="status-badge badge-standby" id="rider-status-badge">WAITING</span>
          </div>

          <div class="rider-profile">
            <div class="bib-box" id="active-bib">#--</div>
            <div class="rider-details">
              <div class="rider-name" id="active-name">Menunggu Start...</div>
              <div class="rider-cat" id="active-cat">Men Elite Downhill</div>
            </div>
          </div>

          <div class="timer-box">
            <div class="timer-digits" id="active-timer">00:00.000</div>
            <div class="timer-label">RACE TIMER</div>
          </div>

          <div class="rider-actions" id="action-buttons" style="display:none;">
            <button class="btn btn-finish" onclick="manualFinishTrigger()">
              🏁 Memicu Finish
            </button>
            <button class="btn btn-cancel" onclick="resetActiveRace()">
              ✕ Batalkan
            </button>
          </div>
        </div>

        <!-- CARD 2: NEXT RIDER -->
        <div class="card">
          <div class="card-head">
            <span class="card-title">NEXT RIDER</span>
          </div>
          <div class="next-rider-text" id="next-rider-text">
            Belum ada rider dalam antrean
          </div>
        </div>

        <!-- CARD 3: DAFTAR RIDER (QUEUE / HISTORY) -->
        <div class="card">
          <div class="card-head">
            <span class="card-title">DAFTAR RIDER</span>
          </div>
          <div class="queue-list" id="queue-list-container">
            <div style="font-size:0.8rem; color:var(--text-dim);">Belum ada riwayat rider</div>
          </div>
        </div>
      </section>

      <!-- RIGHT COLUMN: OFFICIAL LEADERBOARD -->
      <section>
        <div class="card">
          <div class="card-head">
            <span class="card-title">OFFICIAL RACE LEADERBOARD (FASTEST TIME)</span>
            <span class="leaderboard-stats" id="leaderboard-stats">0 FINISHER, 0 DNF</span>
          </div>

          <div class="table-container">
            <table>
              <thead>
                <tr>
                  <th style="width: 50px;">POS</th>
                  <th style="width: 65px;">BIB #</th>
                  <th>RIDER NAME</th>
                  <th>START TIME</th>
                  <th>END TIME</th>
                  <th>TIME</th>
                  <th>GAP</th>
                  <th>STATUS</th>
                </tr>
              </thead>
              <tbody id="leaderboard-tbody">
                <tr>
                  <td colspan="8" style="text-align:center; padding: 30px; color: var(--text-dim);">
                    Belum ada data finish peserta. Balapan belum dimulai.
                  </td>
                </tr>
              </tbody>
            </table>
          </div>

          <div class="leaderboard-footer">
            <button class="btn btn-secondary" onclick="exportCSV()">
              📥 Export CSV
            </button>
            <button class="btn btn-secondary" onclick="openTrackModal()">
              ⚙️ Track Config
            </button>
            <button class="btn btn-secondary" onclick="clearAllData()">
              🗑️ Reset Data
            </button>
          </div>
        </div>
      </section>
    </main>
  </div>

  <!-- MODAL EDIT TRACK / EVENT -->
  <div class="modal-backdrop" id="track-modal">
    <div class="modal-box">
      <div class="modal-head">
        <div class="modal-title">Konfigurasi Track &amp; Event</div>
        <button class="btn btn-secondary" onclick="closeTrackModal()" style="padding:4px 8px;">✕</button>
      </div>
      <div class="form-group">
        <label>Nama Track / Sektor</label>
        <input type="text" id="input-track-name" placeholder="Misal: SECTOR 1 / DOWNHILL TRACK">
      </div>
      <div style="display:flex; justify-content:flex-end; gap:8px;">
        <button class="btn btn-secondary" onclick="closeTrackModal()">Batal</button>
        <button class="btn btn-finish" onclick="saveTrackConfig()">Simpan</button>
      </div>
    </div>
  </div>

  <script>
    let raceActive = false;
    let raceStartLocalMs = 0;
    let raceElapsedServer = 0;
    let timerInterval = null;
    let bestLeaderMs = 0;

    // Format millisecond duration: mm:ss.mmm
    function formatTime(ms) {
      if (!ms || isNaN(ms)) return "00:00.000";
      const totalSec = Math.floor(ms / 1000);
      const minutes = Math.floor(totalSec / 60);
      const seconds = totalSec % 60;
      const millis = ms % 1000;
      return (
        String(minutes).padStart(2, '0') + ":" +
        String(seconds).padStart(2, '0') + "." +
        String(millis).padStart(3, '0')
      );
    }

    // Format GPS time timestamp ms into HH:MM:SS.mmm
    function formatClockTime(ms) {
      if (!ms || ms === 0) return "--:--:--.---";
      const totalSec = Math.floor(ms / 1000);
      const hr = Math.floor(totalSec / 3600) % 24;
      const min = Math.floor(totalSec / 60) % 60;
      const sec = totalSec % 60;
      const mil = ms % 1000;
      return (
        String(hr).padStart(2, '0') + ":" +
        String(min).padStart(2, '0') + ":" +
        String(sec).padStart(2, '0') + "." +
        String(mil).padStart(3, '0')
      );
    }

    // Live Clock (System Clock)
    function updateClock() {
      const now = new Date();
      const h = String(now.getHours()).padStart(2, '0');
      const m = String(now.getMinutes()).padStart(2, '0');
      const s = String(now.getSeconds()).padStart(2, '0');
      document.getElementById('live-clock').textContent = h + ":" + m + ":" + s;
    }
    setInterval(updateClock, 1000);
    updateClock();

    // High frequency live timer runner (60fps)
    function runLiveTimer() {
      if (!raceActive) return;
      const now = performance.now();
      const currentElapsed = Math.floor(raceElapsedServer + (now - raceStartLocalMs));
      document.getElementById('active-timer').textContent = formatTime(currentElapsed);
      requestAnimationFrame(runLiveTimer);
    }

    // Fetch Active Race Status
    async function fetchStatus() {
      try {
        const res = await fetch('/api/status');
        if (!res.ok) throw new Error("HTTP error");
        const data = await res.json();

        // Update Online Indicator
        document.getElementById('system-dot').className = "status-dot";
        document.getElementById('system-status-text').textContent = "ESP32 Online (Live Telemetry)";

        if (data.eventName && data.eventName.length > 0) {
          document.getElementById('track-name').textContent = data.eventName;
        }

        // Active Rider handling
        const badge = document.getElementById('rider-status-badge');
        const actions = document.getElementById('action-buttons');

        if (data.active) {
          if (!raceActive) {
            raceActive = true;
            raceStartLocalMs = performance.now();
            raceElapsedServer = data.elapsed || 0;
            requestAnimationFrame(runLiveTimer);
          } else {
            raceElapsedServer = data.elapsed || 0;
          }

          badge.className = "status-badge badge-on-track";
          badge.textContent = "ON TRACK";

          document.getElementById('active-bib').textContent = "#" + (data.bib || "--");
          document.getElementById('active-name').textContent = data.name || "Peserta";
          document.getElementById('active-cat').textContent = data.team || "Men Elite Downhill";
          actions.style.display = "flex";

        } else {
          raceActive = false;
          actions.style.display = "none";

          if (data.lastRun && data.lastRun.bib) {
            badge.className = "status-badge badge-finished";
            badge.textContent = "FINISHED";
            document.getElementById('active-bib').textContent = "#" + data.lastRun.bib;
            document.getElementById('active-name').textContent = data.lastRun.name || "Peserta";
            document.getElementById('active-cat').textContent = data.lastRun.team || "Men Elite Downhill";
            document.getElementById('active-timer').textContent = formatTime(data.lastRun.elapsed);
          } else {
            badge.className = "status-badge badge-standby";
            badge.textContent = "WAITING";
            document.getElementById('active-bib').textContent = "#--";
            document.getElementById('active-name').textContent = "Menunggu Start...";
            document.getElementById('active-cat').textContent = "Men Elite Downhill";
            document.getElementById('active-timer').textContent = "00:00.000";
          }
        }

      } catch (err) {
        document.getElementById('system-dot').className = "status-dot offline";
        document.getElementById('system-status-text').textContent = "ESP32 Offline (Reconnecting...)";
      }
    }

    // Fetch Leaderboard & Daftar Rider
    async function fetchLeaderboard() {
      try {
        const res = await fetch('/api/leaderboard');
        if (!res.ok) return;
        const list = await res.json();

        const tbody = document.getElementById('leaderboard-tbody');
        const queueContainer = document.getElementById('queue-list-container');

        if (!list || list.length === 0) {
          tbody.innerHTML = '<tr><td colspan="8" style="text-align:center; padding: 30px; color: var(--text-dim);">Belum ada data finish peserta. Balapan belum dimulai.</td></tr>';
          document.getElementById('leaderboard-stats').textContent = "0 FINISHER, 0 DNF";
          queueContainer.innerHTML = '<div style="font-size:0.8rem; color:var(--text-dim);">Belum ada riwayat rider</div>';
          return;
        }

        let finishers = 0;
        let dnf = 0;
        let fastestTime = 0;

        let tableHtml = "";
        let queueHtml = "";

        list.forEach((item, index) => {
          if (item.completed && item.elapsed > 0) {
            finishers++;
            if (finishers === 1) fastestTime = item.elapsed;
          } else {
            dnf++;
          }

          // Rank Badge
          let posText = "";
          let posClass = "pos-other";
          if (index === 0) { posText = "#1"; posClass = "pos-gold"; }
          else if (index === 1) { posText = "#2"; posClass = "pos-silver"; }
          else if (index === 2) { posText = "#3"; posClass = "pos-bronze"; }
          else { posText = "#" + (index + 1); posClass = "pos-other"; }

          // Format timestamps
          const startTimeStr = item.start ? formatClockTime(item.start) : (item.startTimeStr || "--:--:--");
          const endTimeStr = item.finish ? formatClockTime(item.finish) : "--";

          // Calculate Gap
          let gapText = "--";
          if (index === 0) {
            gapText = "LEADER";
          } else if (item.elapsed > 0 && fastestTime > 0) {
            const diff = item.elapsed - fastestTime;
            gapText = "+" + formatTime(diff);
          }

          const durationText = item.completed ? formatTime(item.elapsed) : "NOT FINISHED";
          const statusText = item.completed ? "OK / FINISH" : "RACE FAILED";
          const statusClass = item.completed ? "status-cell ok" : "status-cell failed";
          const durClass = item.completed ? "duration-cell" : "duration-cell nf";

          tableHtml += `
            <tr>
              <td><span class="${posClass}">${posText}</span></td>
              <td class="bib-cell">#${escapeHtml(item.bib)}</td>
              <td class="name-cell">${escapeHtml(item.name)}</td>
              <td class="time-mono">${startTimeStr}</td>
              <td class="time-mono">${endTimeStr}</td>
              <td class="${durClass}">${durationText}</td>
              <td class="gap-cell">${gapText}</td>
              <td class="${statusClass}">${statusText}</td>
            </tr>
          `;

          // Add to Queue List (max 6 latest)
          if (index < 6) {
            queueHtml += `
              <div class="queue-item">
                <span class="queue-name">#${escapeHtml(item.bib)} - ${escapeHtml(item.name)}</span>
                <span class="${item.completed ? 'queue-status-ok' : 'queue-status-dnf'}">${item.completed ? 'Selesai' : 'Tidak finish'}</span>
              </div>
            `;
          }
        });

        tbody.innerHTML = tableHtml;
        queueContainer.innerHTML = queueHtml;
        document.getElementById('leaderboard-stats').textContent = `${finishers} FINISHER, ${dnf} DNF`;

      } catch (err) {
        console.error(err);
      }
    }

    // Actions
    async function manualFinishTrigger() {
      if (!confirm("Konfirmasi finish manual untuk peserta ini?")) return;
      try {
        await fetch('/api/manual-finish', { method: 'POST' });
        fetchStatus();
        fetchLeaderboard();
      } catch (e) { alert("Gagal memicu finish"); }
    }

    async function resetActiveRace() {
      if (!confirm("Batalkan balapan aktif peserta ini?")) return;
      try {
        await fetch('/api/reset-race', { method: 'POST' });
        fetchStatus();
      } catch (e) { alert("Gagal mereset balapan"); }
    }

    async function clearAllData() {
      if (!confirm("PERINGATAN: Hapus seluruh data leaderboard dan riwayat balap?")) return;
      try {
        await fetch('/api/clear-all', { method: 'POST' });
        fetchStatus();
        fetchLeaderboard();
      } catch (e) { alert("Gagal menghapus data"); }
    }

    function exportCSV() {
      window.location.href = '/api/export-csv';
    }

    // Modal Track Config
    function openTrackModal() {
      document.getElementById('input-track-name').value = document.getElementById('track-name').textContent;
      document.getElementById('track-modal').style.display = 'flex';
    }
    function closeTrackModal() {
      document.getElementById('track-modal').style.display = 'none';
    }
    async function saveTrackConfig() {
      const val = document.getElementById('input-track-name').value.trim();
      if (!val) return;
      try {
        await fetch('/api/set-event', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ name: val })
        });
        document.getElementById('track-name').textContent = val;
        closeTrackModal();
      } catch (e) { alert("Gagal menyimpan nama track"); }
    }

    function escapeHtml(str) {
      if (!str) return "";
      return String(str)
        .replace(/&/g, "&amp;")
        .replace(/</g, "&lt;")
        .replace(/>/g, "&gt;");
    }

    // Initial Poll Loops
    setInterval(fetchStatus, 300);
    setInterval(fetchLeaderboard, 1000);
    fetchStatus();
    fetchLeaderboard();
  </script>
</body>
</html>
)rawliteral";

#endif
