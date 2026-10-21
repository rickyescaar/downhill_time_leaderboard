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
    .btn:disabled {
      cursor: not-allowed;
      opacity: 0.55;
    }
    .sim-panel, .roster-panel {
      background: #101827;
      border: 1px dashed #334155;
      border-radius: 8px;
      padding: 12px;
      margin-top: 12px;
    }
    .sim-heading, .roster-heading {
      color: var(--text-muted);
      font-size: 0.7rem;
      font-weight: 800;
      letter-spacing: 0.5px;
      margin-bottom: 8px;
      text-transform: uppercase;
    }
    .sim-note {
      color: var(--text-dim);
      font-size: 0.75rem;
      line-height: 1.45;
      margin-bottom: 10px;
    }
    .sim-form, .sim-controls {
      display: flex;
      flex-wrap: wrap;
      gap: 8px;
    }
    .sim-form input {
      min-width: 140px;
      flex: 1;
      background: #090d16;
      border: 1px solid var(--card-border);
      border-radius: 6px;
      padding: 9px 10px;
      color: var(--text);
    }
    .sim-roster {
      display: grid;
      gap: 6px;
      list-style: none;
      max-height: 150px;
      overflow-y: auto;
    }
    .sim-roster li {
      display: flex;
      justify-content: space-between;
      gap: 8px;
      color: #cbd5e1;
      font-size: 0.8rem;
    }
    .sim-roster-status { color: var(--text-muted); }
    .confirm-dialog {
      width: min(440px, calc(100% - 32px));
      margin: auto;
      padding: 20px;
      color: var(--text);
      background: #111827;
      border: 1px solid var(--card-border);
      border-radius: 12px;
    }
    .confirm-dialog::backdrop { background: rgba(0, 0, 0, 0.75); }
    .confirm-dialog p {
      margin: 10px 0 14px;
      color: var(--text-muted);
      line-height: 1.5;
    }
    .confirm-error {
      min-height: 1.2em;
      margin-top: 6px;
      color: var(--red);
      font-size: 0.8rem;
    }
    .confirm-actions {
      display: flex;
      justify-content: flex-end;
      gap: 8px;
      margin-top: 14px;
    }

    /* NEXT RIDER & RUN HISTORY */
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
          EVENT: <span id="track-name">Bicycle Race Championship</span>
        </div>
        <div class="status-pill">
          <span class="status-dot" id="system-dot"></span>
          <span id="system-status-text">ESP32 Online (Live Telemetry)</span>
        </div>
        <button class="btn btn-secondary" id="simulation-mode-toggle" type="button" onclick="toggleSimulationMode()">
          Mode Simulasi Lokal
        </button>
        <div class="header-clock" id="live-clock" title="Waktu lokal browser, bukan waktu GPS perangkat">--:--:--</div>
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
            Antrean peserta belum tersedia di firmware Finish.
          </div>
        </div>

        <!-- CARD 3: RECENT FINISHES -->
        <div class="card">
          <div class="card-head">
            <span class="card-title">RIWAYAT FINISH TERBARU</span>
          </div>
          <div class="queue-list" id="queue-list-container">
            <div style="font-size:0.8rem; color:var(--text-dim);">Belum ada peserta yang finish</div>
          </div>
        </div>

        <div class="card">
          <div class="card-head">
            <span class="card-title">ANTREAN &amp; SIMULATOR MANUAL</span>
          </div>
          <div class="sim-note">
            Simulasi hanya tersimpan selama halaman ini terbuka; tidak mengirim start/DNF ke ESP32 atau menyimpan ke SPIFFS.
            Ekspor CSV sebelum menutup atau memuat ulang halaman. Aktifkan mode simulasi lokal terlebih dahulu.
          </div>
          <form class="sim-form" id="sim-rider-form">
            <input id="sim-rider-name" type="text" maxlength="80" placeholder="Nama rider" aria-label="Nama rider simulasi" required disabled>
            <button class="btn btn-secondary" type="submit" disabled>Tambah Rider</button>
          </form>
          <div class="roster-panel">
            <div class="roster-heading">Daftar Rider Simulasi</div>
            <ul class="sim-roster" id="sim-roster-list">
              <li><span>Belum ada rider simulasi.</span></li>
            </ul>
          </div>
          <div class="sim-panel">
            <div class="sim-heading" style="width:100%">Simulator Controls</div>
            <div class="sim-controls">
              <button class="btn btn-secondary" id="sim-start-button" type="button" style="display:none" disabled>Simulasi Start Rider</button>
              <button class="btn btn-secondary" id="sim-finish-button" type="button" disabled>Trigger Finish Radar</button>
              <button class="btn btn-secondary" id="sim-dnf-button" type="button" disabled>Not Finished (DNF)</button>
              <button class="btn btn-secondary" id="sim-clear-button" type="button">Clear Simulation</button>
            </div>
          </div>
        </div>
      </section>

      <!-- RIGHT COLUMN: OFFICIAL LEADERBOARD -->
      <section>
        <div class="card">
          <div class="card-head">
            <span class="card-title" id="leaderboard-title">OFFICIAL RACE LEADERBOARD (FASTEST TIME)</span>
            <span class="leaderboard-stats" id="leaderboard-stats">0 FINISHER</span>
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
            <button class="btn btn-secondary" id="export-csv-button" onclick="exportCSV()" disabled>
              📥 Export CSV
            </button>
            <button class="btn btn-secondary" onclick="openTrackModal()">
              ⚙️ Nama Event
            </button>
            <button class="btn btn-secondary" onclick="clearAllData()">
              🗑️ Reset Data
            </button>
          </div>
        </div>
      </section>
    </main>
  </div>

  <!-- MODAL EDIT EVENT NAME -->
  <div class="modal-backdrop" id="track-modal">
    <div class="modal-box">
      <div class="modal-head">
        <div class="modal-title">Konfigurasi Nama Event</div>
        <button class="btn btn-secondary" onclick="closeTrackModal()" style="padding:4px 8px;">✕</button>
      </div>
      <div class="form-group">
        <label>Nama Event</label>
        <input type="text" id="input-track-name" maxlength="63" placeholder="Misal: Kejuaraan Downhill">
      </div>
      <div style="display:flex; justify-content:flex-end; gap:8px;">
        <button class="btn btn-secondary" onclick="closeTrackModal()">Batal</button>
        <button class="btn btn-finish" onclick="saveTrackConfig()">Simpan</button>
      </div>
    </div>
  </div>

  <dialog class="confirm-dialog" id="sim-clear-dialog" aria-labelledby="sim-clear-title">
    <form id="sim-clear-form">
      <h2 id="sim-clear-title">Hapus data simulasi?</h2>
      <p>Daftar rider dan hasil simulasi lokal akan dihapus. Data perangkat tidak terpengaruh. Ketik HAPUS untuk melanjutkan.</p>
      <input id="sim-clear-confirmation" type="text" autocomplete="off" placeholder="Ketik HAPUS" aria-label="Ketik HAPUS untuk konfirmasi">
      <div class="confirm-error" id="sim-clear-error" aria-live="polite"></div>
      <div class="confirm-actions">
        <button class="btn btn-secondary" id="sim-clear-cancel" type="button">Batal</button>
        <button class="btn btn-cancel" type="submit">Hapus Simulasi</button>
      </div>
    </form>
  </dialog>

  <script>
    let raceActive = false;
    let raceStartLocalMs = 0;
    let raceElapsedServer = 0;
    let simulationMode = false;
    let simulatedParticipants = [];
    let simulatedResults = [];
    let currentSimulatedRider = null;
    let simulatedStartTime = 0;
    let simulatedTimer = null;
    let nextSimulatedBib = 101;
    let deviceLeaderboard = [];
    let deviceRuns = [];

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

    function formatLocalClockTime(ms) {
      if (!ms || ms === 0) return "--:--:--.---";
      const date = new Date(ms);
      return (
        String(date.getHours()).padStart(2, '0') + ":" +
        String(date.getMinutes()).padStart(2, '0') + ":" +
        String(date.getSeconds()).padStart(2, '0') + "." +
        String(date.getMilliseconds()).padStart(3, '0')
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

    // This clock reflects the browser's local time, not GPS time from the firmware.
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
      if (!raceActive || simulationMode) return;
      const now = performance.now();
      const currentElapsed = Math.floor(raceElapsedServer + (now - raceStartLocalMs));
      document.getElementById('active-timer').textContent = formatTime(currentElapsed);
      requestAnimationFrame(runLiveTimer);
    }

    // Fetch Active Race Status
    async function fetchStatus() {
      try {
        const res = await fetch('/api/state');
        if (!res.ok) throw new Error("HTTP error");
        const data = await res.json();

        document.getElementById('system-dot').className = "status-dot";
        const gpsStatus = data.gpsReady
          ? `GPS ${data.satellites} satelit`
          : "GPS belum siap";
        const loraStatus = data.loraReady ? "LoRa siap" : "LoRa tidak siap";
        const startStatus = data.startDeviceOnline ? "Start online" : "Start offline";
        document.getElementById('system-status-text').textContent = simulationMode
          ? `SIMULASI LOKAL · ESP32 Online · ${gpsStatus} · ${loraStatus} · ${startStatus}`
          : `ESP32 Online · ${gpsStatus} · ${loraStatus} · ${startStatus}`;

        if (data.eventName && data.eventName.length > 0) {
          document.getElementById('track-name').textContent = data.eventName;
        }

        if (simulationMode) return;

        // Active Rider handling
        const badge = document.getElementById('rider-status-badge');
        const actions = document.getElementById('action-buttons');
        document.getElementById('next-rider-text').textContent =
          "Antrean peserta belum tersedia dari firmware.";

        if (data.raceActive) {
          if (!raceActive) {
            raceActive = true;
            requestAnimationFrame(runLiveTimer);
          }
          raceStartLocalMs = performance.now();
          raceElapsedServer = data.elapsed || 0;

          badge.className = "status-badge badge-on-track";
          badge.textContent = "ON TRACK";

          document.getElementById('active-bib').textContent = "#" + (data.currentBib || "--");
          document.getElementById('active-name').textContent = data.currentName || "Peserta";
          document.getElementById('active-cat').textContent = data.currentTeam || "Men Elite Downhill";
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
        document.getElementById('system-status-text').textContent = simulationMode
          ? "SIMULASI LOKAL · ESP32 offline"
          : "ESP32 Offline (Reconnecting...)";
      }
    }

    // Fetch Leaderboard & Daftar Rider
    async function fetchLeaderboard() {
      try {
        const res = await fetch('/api/leaderboard');
        if (!res.ok) throw new Error("HTTP error");
        deviceLeaderboard = await res.json();
        renderLeaderboard(simulationMode
          ? [...deviceLeaderboard, ...simulatedResults]
          : deviceLeaderboard);
      } catch (err) {
        console.error(err);
      }
    }

    function renderLeaderboard(list) {
      const tbody = document.getElementById('leaderboard-tbody');

      const sorted = [...(list || [])].sort((a, b) => {
        if (!a.completed) return b.completed ? 1 : 0;
        if (!b.completed) return -1;
        return a.elapsed - b.elapsed;
      });

      if (!sorted.length) {
        tbody.innerHTML = '<tr><td colspan="8" style="text-align:center; padding: 30px; color: var(--text-dim);">Belum ada data finish peserta. Balapan belum dimulai.</td></tr>';
        document.getElementById('leaderboard-stats').textContent = "0 FINISHER, 0 DNF";
        updateExportButton();
        return;
      }

      const finishers = sorted.filter(item => item.completed);
      const fastestTime = finishers.length ? finishers[0].elapsed : 0;
      let finishRank = 0;
      const tableHtml = sorted.map(item => {
        const isDnf = !item.completed;
        const rank = isDnf ? null : ++finishRank;
        const posText = isDnf ? "DNF" : `#${rank}`;
        const posClass = isDnf ? "pos-nf" : rank === 1 ? "pos-gold" : rank === 2 ? "pos-silver" : rank === 3 ? "pos-bronze" : "pos-other";
        const startTimeStr = item.localTime
          ? formatLocalClockTime(item.start)
          : item.start ? formatClockTime(item.start) : (item.startTimeStr || "--:--:--");
        const endTimeStr = item.finish
          ? (item.localTime ? formatLocalClockTime(item.finish) : formatClockTime(item.finish))
          : "--";
        const gapText = isDnf ? "--" : rank === 1 ? "LEADER" : "+" + formatTime(item.elapsed - fastestTime);

        return `
            <tr>
              <td><span class="${posClass}">${posText}</span></td>
              <td class="bib-cell">#${escapeHtml(item.bib)}</td>
              <td class="name-cell">${escapeHtml(item.name)}</td>
              <td class="time-mono">${startTimeStr}</td>
              <td class="time-mono">${endTimeStr}</td>
              <td class="duration-cell ${isDnf ? 'nf' : ''}">${isDnf ? 'NOT FINISHED' : formatTime(item.elapsed)}</td>
              <td class="gap-cell">${gapText}</td>
              <td class="status-cell ${isDnf ? 'failed' : 'ok'}">${item.localTime
                ? (isDnf ? 'SIM DNF' : 'SIM FINISH')
                : (isDnf ? 'RACE FAILED' : 'FINISH')}</td>
            </tr>
          `;
        }).join("");

      tbody.innerHTML = tableHtml;
      document.getElementById('leaderboard-stats').textContent =
        `${finishers.length} FINISHER, ${sorted.length - finishers.length} DNF`;
      updateExportButton();
    }

    async function fetchRunHistory() {
      try {
        const res = await fetch('/api/runs');
        if (!res.ok) throw new Error("HTTP error");
        deviceRuns = await res.json();
        renderRunHistory();
      } catch (err) {
        console.error(err);
      }
    }

    function renderRunHistory() {
        const queueContainer = document.getElementById('queue-list-container');
        const runs = simulationMode
          ? [...deviceRuns, ...simulatedResults].sort((a, b) => (b.finish || 0) - (a.finish || 0))
          : deviceRuns;

        if (!runs || runs.length === 0) {
          queueContainer.innerHTML = '<div style="font-size:0.8rem; color:var(--text-dim);">Belum ada peserta yang finish</div>';
          return;
        }

        queueContainer.innerHTML = runs.slice(0, 6).map(item => `
          <div class="queue-item">
            <span class="queue-name">#${escapeHtml(item.bib)} - ${escapeHtml(item.name)}</span>
            <span class="queue-status-ok">${item.completed === false ? 'DNF' : formatTime(item.elapsed)}</span>
          </div>
        `).join("");
    }

    function updateExportButton() {
      const button = document.getElementById('export-csv-button');
      button.disabled = deviceLeaderboard.length === 0 && simulatedResults.length === 0;
    }

    function toggleSimulationMode() {
      simulationMode = !simulationMode;
      document.getElementById('simulation-mode-toggle').textContent =
        simulationMode ? "Kembali ke Mode Perangkat" : "Mode Simulasi Lokal";
      document.getElementById('system-status-text').textContent = simulationMode
        ? "SIMULASI LOKAL · data tidak dikirim ke perangkat"
        : "Menghubungkan ke Finish Device...";
      document.getElementById('sim-rider-name').disabled = !simulationMode;
      document.querySelector('#sim-rider-form button').disabled = !simulationMode;
      document.getElementById('sim-start-button').style.display =
        simulationMode ? "inline-flex" : "none";
      document.getElementById('leaderboard-title').textContent = simulationMode
        ? "LEADERBOARD · PERANGKAT + SIMULASI LOKAL"
        : "OFFICIAL RACE LEADERBOARD (FASTEST TIME)";

      if (simulationMode) {
        renderSimulationState();
        renderLeaderboard([...deviceLeaderboard, ...simulatedResults]);
        renderRunHistory();
      } else {
        clearInterval(simulatedTimer);
        simulatedTimer = null;
        raceActive = false;
        fetchStatus();
        renderLeaderboard(deviceLeaderboard);
        renderRunHistory();
      }
      updateSimulationControls();
    }

    function updateSimulationControls() {
      document.getElementById('sim-start-button').disabled =
        !simulationMode || Boolean(currentSimulatedRider) ||
        !simulatedParticipants.some(rider => rider.status === 'waiting');
      document.getElementById('sim-finish-button').disabled =
        !simulationMode || !currentSimulatedRider;
      document.getElementById('sim-dnf-button').disabled =
        !simulationMode || !currentSimulatedRider;
      document.getElementById('sim-clear-button').disabled =
        !simulationMode || (!simulatedParticipants.length && !simulatedResults.length);
    }

    function renderSimulationState() {
      const list = document.getElementById('sim-roster-list');
      list.replaceChildren();
      if (!simulatedParticipants.length) {
        const item = document.createElement('li');
        item.textContent = "Belum ada rider simulasi.";
        list.appendChild(item);
      } else {
        simulatedParticipants.forEach(rider => {
          const item = document.createElement('li');
          const identity = document.createElement('span');
          const status = document.createElement('span');
          identity.textContent = `${rider.bib} - ${rider.name}`;
          status.className = "sim-roster-status";
          status.textContent = rider.status === 'waiting' ? "Menunggu"
            : rider.status === 'racing' ? "Sedang race"
              : rider.status === 'dnf' ? "DNF" : "Selesai";
          item.append(identity, status);
          list.appendChild(item);
        });
      }

      const nextRider = simulatedParticipants.find(rider => rider.status === 'waiting');
      document.getElementById('next-rider-text').textContent = nextRider
        ? `${nextRider.bib} - ${nextRider.name}`
        : "Belum ada rider dalam antrean simulasi";

      const badge = document.getElementById('rider-status-badge');
      const actions = document.getElementById('action-buttons');
      actions.style.display = "none";
      if (currentSimulatedRider) {
        badge.className = "status-badge badge-on-track";
        badge.textContent = "SIMULASI";
        document.getElementById('active-bib').textContent = currentSimulatedRider.bib;
        document.getElementById('active-name').textContent = currentSimulatedRider.name;
        document.getElementById('active-cat').textContent = currentSimulatedRider.team;
        document.getElementById('active-timer').textContent =
          formatTime(Date.now() - simulatedStartTime);
        clearInterval(simulatedTimer);
        simulatedTimer = setInterval(() => {
          if (!simulationMode || !currentSimulatedRider) return;
          document.getElementById('active-timer').textContent =
            formatTime(Date.now() - simulatedStartTime);
        }, 25);
      } else {
        clearInterval(simulatedTimer);
        simulatedTimer = null;
        const lastResult = simulatedResults[simulatedResults.length - 1];
        if (lastResult) {
          badge.className = lastResult.completed
            ? "status-badge badge-finished"
            : "status-badge badge-standby";
          badge.textContent = lastResult.completed ? "SIM FINISH" : "DNF";
          document.getElementById('active-bib').textContent = lastResult.bib;
          document.getElementById('active-name').textContent = lastResult.name;
          document.getElementById('active-cat').textContent = lastResult.team;
          document.getElementById('active-timer').textContent = lastResult.completed
            ? formatTime(lastResult.elapsed)
            : "DNF";
        } else {
          badge.className = "status-badge badge-standby";
          badge.textContent = "WAITING";
          document.getElementById('active-bib').textContent = "#--";
          document.getElementById('active-name').textContent = "Menunggu Start...";
          document.getElementById('active-cat').textContent = "Men Elite Downhill";
          document.getElementById('active-timer').textContent = "00:00.000";
        }
      }
      renderLeaderboard([...deviceLeaderboard, ...simulatedResults]);
      renderRunHistory();
      updateSimulationControls();
    }

    document.getElementById('sim-rider-form').addEventListener('submit', event => {
      event.preventDefault();
      if (!simulationMode) return;
      const input = document.getElementById('sim-rider-name');
      const name = input.value.trim();
      if (!name) return;
      simulatedParticipants.push({
        id: `sim-${Date.now()}-${nextSimulatedBib}`,
        bib: `#${nextSimulatedBib++}`,
        name,
        team: "Men Elite Downhill",
        status: "waiting"
      });
      input.value = "";
      renderSimulationState();
      input.focus();
    });

    document.getElementById('sim-start-button').addEventListener('click', () => {
      if (!simulationMode || currentSimulatedRider) return;
      const rider = simulatedParticipants.find(item => item.status === 'waiting');
      if (!rider) return;
      rider.status = "racing";
      currentSimulatedRider = rider;
      simulatedStartTime = Date.now();
      renderSimulationState();
    });

    function completeSimulatedRider(completed) {
      if (!simulationMode || !currentSimulatedRider) return;
      const finishTime = Date.now();
      const elapsed = finishTime - simulatedStartTime;
      currentSimulatedRider.status = completed ? "finished" : "dnf";
      simulatedResults.push({
        bib: currentSimulatedRider.bib,
        name: currentSimulatedRider.name,
        team: currentSimulatedRider.team,
        start: simulatedStartTime,
        finish: completed ? finishTime : 0,
        elapsed: completed ? elapsed : 0,
        startTimeStr: formatLocalClockTime(simulatedStartTime),
        date: new Date(simulatedStartTime).toISOString().slice(0, 10),
        localTime: true,
        completed
      });
      currentSimulatedRider = null;
      renderSimulationState();
    }

    document.getElementById('sim-finish-button').addEventListener('click', () => {
      completeSimulatedRider(true);
    });
    document.getElementById('sim-dnf-button').addEventListener('click', () => {
      completeSimulatedRider(false);
    });

    document.getElementById('sim-clear-button').addEventListener('click', () => {
      const dialog = document.getElementById('sim-clear-dialog');
      document.getElementById('sim-clear-confirmation').value = "";
      document.getElementById('sim-clear-error').textContent = "";
      dialog.showModal();
      document.getElementById('sim-clear-confirmation').focus();
    });
    document.getElementById('sim-clear-cancel').addEventListener('click', () => {
      document.getElementById('sim-clear-dialog').close();
    });
    document.getElementById('sim-clear-form').addEventListener('submit', event => {
      event.preventDefault();
      const confirmation = document.getElementById('sim-clear-confirmation');
      if (confirmation.value.trim() !== "HAPUS") {
        document.getElementById('sim-clear-error').textContent =
          "Konfirmasi tidak cocok. Ketik HAPUS persis untuk melanjutkan.";
        confirmation.focus();
        return;
      }
      simulatedParticipants = [];
      simulatedResults = [];
      currentSimulatedRider = null;
      nextSimulatedBib = 101;
      clearInterval(simulatedTimer);
      simulatedTimer = null;
      document.getElementById('sim-clear-dialog').close();
      renderSimulationState();
    });

    function csvEscape(value) {
      return `"${String(value ?? '').replace(/"/g, '""')}"`;
    }

    function exportCSV() {
      const rowsToExport = [...deviceLeaderboard, ...simulatedResults]
        .sort((a, b) => {
          if (!a.completed) return b.completed ? 1 : 0;
          if (!b.completed) return -1;
          return a.elapsed - b.elapsed;
        });
      if (!rowsToExport.length) return;

      const finishers = rowsToExport.filter(item => item.completed);
      const fastestTime = finishers.length ? finishers[0].elapsed : 0;
      let rank = 0;
      const rows = [[
        'Position', 'Bib', 'Name', 'Team', 'Start Time', 'Finish Time',
        'Elapsed', 'Gap (ms)', 'Status', 'Source'
      ]];
      rowsToExport.forEach(item => {
        const isDnf = !item.completed;
        const itemRank = isDnf ? "DNF" : ++rank;
        rows.push([
          itemRank,
          item.bib,
          item.name,
          item.team,
          item.startTimeStr || (item.localTime ? formatLocalClockTime(item.start) : formatClockTime(item.start)),
          item.finish
            ? (item.localTime ? formatLocalClockTime(item.finish) : formatClockTime(item.finish))
            : "",
          isDnf ? "NOT FINISHED" : formatTime(item.elapsed),
          isDnf ? "" : item.elapsed - fastestTime,
          isDnf ? "RACE FAILED" : "OK / FINISH",
          simulatedResults.includes(item) ? "SIMULASI LOKAL" : "FINISH DEVICE"
        ]);
      });
      const csvContent = rows.map(row => row.map(csvEscape).join(',')).join('\r\n');
      const blob = new Blob(["\uFEFF", csvContent], { type: 'text/csv;charset=utf-8' });
      const url = URL.createObjectURL(blob);
      const link = document.createElement('a');
      link.href = url;
      link.download = `downhill_results_${Date.now()}.csv`;
      link.click();
      setTimeout(() => URL.revokeObjectURL(url), 1000);
    }

    // Actions
    async function manualFinishTrigger() {
      if (!confirm("Konfirmasi finish manual untuk peserta ini?")) return;
      try {
        const res = await fetch('/api/manual-finish', { method: 'POST' });
        if (!res.ok) throw new Error("HTTP error");
        fetchStatus();
        fetchLeaderboard();
        fetchRunHistory();
      } catch (e) { alert("Gagal memicu finish"); }
    }

    async function resetActiveRace() {
      if (!confirm("Batalkan balapan aktif peserta ini?")) return;
      try {
        const res = await fetch('/api/reset-race', { method: 'POST' });
        if (!res.ok) throw new Error("HTTP error");
        fetchStatus();
      } catch (e) { alert("Gagal mereset balapan"); }
    }

    async function clearAllData() {
      if (!confirm("PERINGATAN: Hapus seluruh data leaderboard dan riwayat balap?")) return;
      try {
        const res = await fetch('/api/clear-all', { method: 'POST' });
        if (!res.ok) throw new Error("HTTP error");
        fetchStatus();
        fetchLeaderboard();
        fetchRunHistory();
      } catch (e) { alert("Gagal menghapus data"); }
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
      if (new TextEncoder().encode(val).length > 63) {
        alert("Nama event maksimal 63 byte.");
        return;
      }
      try {
        const res = await fetch('/api/set-event', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ name: val })
        });
        if (!res.ok) throw new Error("HTTP error");
        document.getElementById('track-name').textContent = val;
        closeTrackModal();
      } catch (e) { alert("Gagal menyimpan nama event"); }
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
    setInterval(fetchRunHistory, 1000);
    updateSimulationControls();
    updateExportButton();
    fetchStatus();
    fetchLeaderboard();
    fetchRunHistory();
  </script>
</body>
</html>
)rawliteral";

#endif
