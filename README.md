# downhill_time_leaderboard
A system to count time trial race with IoT and display it on leaderboard

## Simulator leaderboard ESP32-S3

ESP32-S3 membuat Wi-Fi Access Point sendiri dan menyajikan dashboard `web/index.html`
melalui HTTP. WebSocket di port 81 tetap digunakan untuk menerima event sensor.

1. Hubungkan komputer ke ESP32 lewat USB.
2. Build/upload firmware simulator (`pio run -e esp32-s3-devkitc-1 -t upload`).
3. Upload file dashboard ke LittleFS (`PlatformIO: Upload Filesystem Image`).
   Jalankan upload filesystem setelah upload firmware, dan ulangi setiap kali
   `web/index.html` diubah.
4. Dari ponsel atau laptop, sambungkan ke Wi-Fi `Downhill_Timing_AP` dengan
   password `balap12345`.
5. Buka alamat IP yang dicetak di Serial Monitor (umumnya `192.168.4.1`).
   Browser akan memuat dashboard dan menghubungkan WebSocket ke ESP32 secara otomatis.

Perangkat yang membuka dashboard harus tetap tersambung ke Wi-Fi ESP32. Jaringan
Access Point ini untuk koneksi lokal dan tidak menyediakan akses internet.

   Sensor tambahan dapat tersambung ke Access Point yang sama lalu mengirim pesan
   teks JSON ke WebSocket `ws://192.168.4.1:81/`. ESP32 meneruskan pesan itu ke
   dashboard. Format event yang dikenali:

   ```json
   {"event":"START","time":1700000000000}
   {"event":"FINISH","startTime":1700000000000,"endTime":1700000012345,"duration":12345}
   ```

   `time`, `startTime`, `endTime`, dan `duration` menggunakan milidetik; timestamp
   menggunakan epoch Unix. Ganti host dengan alamat IP yang dicetak ESP32 jika
   alamat Access Point berbeda.

## Firmware perangkat finish

Environment `finish_device` membangun sketch dari divisi perangkat elektronik
untuk WEMOS LOLIN S2 Mini. Firmware ini memakai GPS NEO-6M, LoRa E220, radar
RCWL-0516, serta dua panel P10 sesuai pin yang didefinisikan di
`finish_device.ino`. Periksa board, modul, catu daya, dan wiring fisik sebelum
upload.

1. Sambungkan perangkat finish ke komputer melalui USB.
2. Build firmware dengan `pio run -e finish_device`.
3. Upload firmware dengan `pio run -e finish_device -t upload`.
4. Buka Serial Monitor pada 115200 baud, sambungkan ponsel/laptop ke Wi-Fi
   `RaceTimer-Finish` dengan password `race2024`, lalu buka `http://192.168.4.1`.

Dashboard perangkat finish tertanam di `finish_web_page.h` dan disajikan langsung
oleh firmware. Tidak perlu upload filesystem untuk halaman web; SPIFFS dipakai
oleh firmware untuk menyimpan riwayat run dan nama event. `web/index.html`
terpisah dan tetap digunakan oleh simulator ESP32-S3 di atas, bukan oleh perangkat
finish. Firmware finish memakai API HTTP dan protokol LoRa perangkat start.

## Changelog perangkat finish

Ringkasan ini mencatat perubahan sejak dashboard perangkat finish mulai
diselaraskan dengan `finish_device.ino`. File `preview.html` mengikuti HTML yang
tertanam di `finish_web_page.h` dan digunakan untuk melihat dashboard di browser.

### 21 Oktober 2026

- Menyatukan dashboard web dengan operasi Finish Device: status perangkat,
  rider aktif, leaderboard, riwayat balapan, pengaturan nama event, dan aksi
  manual menggunakan API firmware.
- Mempertahankan tombol aksi manual untuk antisipasi masalah hardware: pemicu
  finish manual dan pembatalan balapan aktif. Tombol start race dari dashboard
  tidak digunakan; start balapan berasal dari Start Device melalui LoRa.
- Menambahkan roster rider di Finish Device, tersimpan di SPIFFS. Dashboard
  mendukung tambah, edit nama, hapus rider satu per satu, dan reset seluruh
  roster. Bib dibuat berurutan dan tidak digunakan ulang setelah rider dihapus.
- Menambahkan pencatatan DNF melalui dashboard dan firmware. DNF disimpan
  sebagai hasil balapan, muncul pada leaderboard/riwayat, dan tidak mendapat
  peringkat finisher. Aksi **Batalkan** tetap berbeda dari DNF dan tidak
  mencatat hasil balapan.
- Mempertahankan ekspor hasil ke CSV dari dashboard, termasuk status DNF dan
  sumber data Finish Device.
- Menghapus simulator rider lokal beserta tombol start, finish, DNF, dan clear
  simulasi dari dashboard Finish Device agar tampilan hanya menggunakan proses
  dan data perangkat.
- Memperbarui `preview.html` agar sama dengan halaman yang tertanam di firmware.

### Batasan yang masih diketahui

- Roster rider yang disimpan di Finish Device belum disinkronkan ke Start Device.
- Pemberitahuan DNF atau pembatalan dari Finish ke Start melalui LoRa belum
  diimplementasikan karena dukungan protokol pada firmware Start perlu
  disepakati dan diverifikasi terlebih dahulu.
- Simulator ESP32-S3 dan `web/index.html` merupakan bagian terpisah; perubahan
  dashboard Finish Device tidak menghapus atau menyinkronkan data simulator itu.
