# downhill_time_leaderboard
A system to count time trial race with IoT and display it on leaderboard

## Menjalankan leaderboard dari ESP32

ESP32-S3 membuat Wi-Fi Access Point sendiri dan menyajikan dashboard `web/index.html`
melalui HTTP. WebSocket di port 81 tetap digunakan untuk menerima event sensor.

1. Hubungkan komputer ke ESP32 lewat USB.
2. Upload firmware dari PlatformIO (`PlatformIO: Upload`).
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
