# Catatan review: perangkat finish dan simulasi Wokwi

Dokumen ini merangkum diskusi integrasi perangkat finish agar bisa ditinjau
kembali dari komputer lain yang memiliki board ESP32. File firmware dan halaman
dari divisi elektronik harus diperlakukan sebagai file referensi yang tidak
diubah tanpa persetujuan pemiliknya.

## Ringkasan keputusan

- `finish_device.ino` adalah firmware perangkat finish fisik, bukan sekadar
  library.
- `finish_web_page.h` adalah header C++ yang berisi halaman HTML, CSS, dan
  JavaScript sebagai string `FINISH_HTML` di flash ESP32.
- Firmware menyajikan halaman tertanam itu pada alamat `/`. Browser tidak
  membaca header `.h` secara langsung.
- `web/index.html` adalah dashboard project lama untuk simulator ESP32-S3.
  Firmware finish tidak menyajikan file ini, sehingga memindahkannya ke root
  project saja tidak akan mengubah halaman yang dikirim firmware.
- Firmware dan dashboard finish dari divisi elektronik jangan diubah selama
  integrasi kecuali pemiliknya menyetujui. UI tertanam perlu dicek kecocokan API
  sebelum dianggap berfungsi penuh.

## Apa yang dilakukan kedua firmware

### Simulator ESP32-S3 yang sudah ada

`src/main.cpp` ditujukan untuk simulasi sederhana:

- Membuat Wi-Fi AP `Downhill_Timing_AP`.
- Menyajikan `web/index.html` dari LittleFS.
- Mengirim event START/FINISH ke dashboard melalui WebSocket port 81.
- START disimulasikan lewat Serial Monitor (`LORA_START`); FINISH lewat tombol
  BOOT pada GPIO 0.
- Tidak mengoperasikan GPS, LoRa E220, sensor radar, atau panel P10 fisik.

### Firmware perangkat finish

`finish_device.ino` ditulis untuk WEMOS LOLIN S2 Mini dengan perangkat tambahan
yang disebut di sketch:

- NEO-6M GPS untuk waktu.
- LoRa E220 untuk menerima START, heartbeat, dan mengirim ACK/FINISH ke perangkat
  start.
- RCWL-0516 sebagai sensor finish.
- Dua panel P10 untuk informasi start, timer berjalan, dan hasil finish.
- SPIFFS untuk menyimpan riwayat run dan nama event.
- Wi-Fi AP `RaceTimer-Finish`; halaman web dilayani pada `http://192.168.4.1`.
- API HTTP digunakan dashboard; firmware ini tidak memakai WebSocket untuk
  dashboard.

Ketepatan integrasi fisik tetap bergantung pada model board, pin, modul, catu
daya, antena, dan wiring yang benar. Build yang berhasil bukan pengganti uji
perangkat.

## Catatan penting tentang API dashboard tertanam

Pemeriksaan file yang ada menemukan kemungkinan ketidakcocokan:

- `finish_web_page.h` meminta `GET /api/status`, tetapi firmware menyediakan
  `GET /api/state`.
- Halaman membaca field seperti `active`, `bib`, `name`, dan `team`; firmware
  mengirim `raceActive`, `currentBib`, `currentName`, dan `currentTeam`.
- Tombol ekspor meminta `/api/export-csv`, tetapi route itu tidak ada pada
  firmware saat ini.
- Route leaderboard, manual finish, reset race, clear data, dan set event ada
  pada firmware.

Karena file dari divisi lain tidak boleh diedit, minta pemilik firmware/dashboard
menyelaraskan kontrak API sebelum mengandalkan status peserta dan ekspor CSV.
Kemungkinan perbaikan perlu disepakati dan dilakukan oleh pemilik file tersebut.

## Cara melihat halaman tertanam

### Preview HTML di browser tanpa board

File `.h` tidak dapat dibuka langsung sebagai halaman. Ekstrak isi raw string ke
file sementara tanpa mengubah file asli:

```bash
python3 - <<'PY'
from pathlib import Path

header = Path("finish_web_page.h").read_text(encoding="utf-8")
start_marker = 'R"rawliteral('
end_marker = ')rawliteral"'
start = header.index(start_marker) + len(start_marker)
end = header.rindex(end_marker)

preview = Path("/tmp/finish-web-preview.html")
preview.write_text(header[start:end], encoding="utf-8")
print(f"Preview dibuat: {preview}")
PY
```

Buka `/tmp/finish-web-preview.html` di browser. Cara ini menampilkan UI, tetapi
permintaan API tidak akan berfungsi tanpa firmware server yang berjalan.

### Preview dari perangkat fisik

Upload firmware finish, sambungkan ponsel/laptop ke AP `RaceTimer-Finish`
(password `race2024`), kemudian buka `http://192.168.4.1`. Halaman yang tampil
berasal dari `FINISH_HTML`, bukan `web/index.html`.

## Status konfigurasi PlatformIO project

Konfigurasi integrasi lokal saat catatan ini dibuat:

- Environment `finish_device` memakai board `lolin_s2_mini`, SPIFFS, ArduinoJson,
  TinyGPSPlus, dan EByte LoRa E220.
- `src/finish_device.cpp` adalah wrapper build yang menyertakan
  `finish_device.ino`; wrapper ini tidak mengubah isi sketch.
- Environment `esp32-s3-devkitc-1` dipisahkan agar `setup()`/`loop()` simulator
  tidak berbenturan dengan firmware finish.
- File dari divisi elektronik `finish_device.ino` dan `finish_web_page.h` tidak
  perlu dimodifikasi untuk konfigurasi build ini.
- Build kedua environment telah berhasil:

  ```bash
  pio run -e finish_device
  pio run -e esp32-s3-devkitc-1
  ```

Perintah upload finish (gunakan hanya setelah board dan wiring sudah dipastikan):

```bash
pio run -e finish_device -t upload
pio device monitor -b 115200
```

Untuk simulator lama, upload firmware dan LittleFS terpisah; `web/index.html`
tetap dipakai oleh target tersebut.

## Wokwi: kegunaan dan batasannya

Wokwi adalah simulator ESP32 dan rangkaian elektronik yang berjalan melalui
browser atau ekstensi VS Code. Wokwi dapat membantu memeriksa firmware dan
komponen yang tersedia di simulator, tetapi tidak menjamin keseluruhan perangkat
finish dapat direplikasi:

- Dukungan keluarga chip ESP32 mencakup ESP32-S2, tetapi model virtualnya belum
  tentu persis WEMOS LOLIN S2 Mini.
- Jaringan Wokwi berbeda dari AP fisik yang dibuat firmware. Wokwi menyediakan
  jaringan simulasi `Wokwi-GUEST`; akses browser masuk ke web server ESP32
  membutuhkan konfigurasi gateway yang sesuai.
- LoRa E220, GPS NEO-6M, radar RCWL-0516, dan panel P10 mungkin tidak memiliki
  model/behavior virtual yang cocok.
- Karena keterbatasan itu, gunakan Wokwi untuk eksperimen terbatas (misalnya
  logika firmware atau periferal yang didukung), bukan sebagai validasi akhir
  wiring dan fungsi perangkat.

Referensi resmi:

- [Wokwi ESP32 guide](https://docs.wokwi.com/guides/esp32)
- [Wokwi ESP32 Wi-Fi guide](https://docs.wokwi.com/guides/esp32-wifi)
- [Wokwi for VS Code getting started](https://docs.wokwi.com/vscode/getting-started)
- [Wokwi diagram editor](https://docs.wokwi.com/guides/diagram-editor)

Alur percobaan Wokwi di VS Code:

1. Pasang ekstensi Wokwi for VS Code dan aktifkan license sesuai petunjuk Wokwi.
2. Build target yang ingin diuji dengan PlatformIO.
3. Siapkan `wokwi.toml` yang menunjuk ke firmware ELF, serta `diagram.json` untuk
   board ESP32-S2 dan komponen virtual yang tersedia.
4. Jalankan `Wokwi: Start Simulator` dari Command Palette.
5. Periksa Serial Monitor; anggap komponen yang tidak tersedia sebagai di luar
   cakupan simulasi, lalu validasi di hardware nyata.

## Langkah review berikutnya

1. Dari perangkat dengan ESP32, cocokkan model board dan seluruh sambungan pin
   terhadap `finish_device.ino` serta rangkaian sebenarnya.
2. Putuskan apakah dashboard finish tertanam sudah cukup atau tim memerlukan
   dashboard `web/index.html` pada perangkat finish.
3. Minta pemilik firmware/dashboard menyelesaikan ketidakcocokan API yang
   disebutkan di atas, jika file divisi lain tetap harus tidak diubah.
4. Build, upload, dan uji Wi-Fi, GPS, komunikasi LoRa, radar finish, panel P10,
   penyimpanan SPIFFS, serta endpoint dashboard pada perangkat fisik.
