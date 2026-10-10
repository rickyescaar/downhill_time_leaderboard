#include <Arduino.h>
#include <WiFi.h>
#include <WebSocketsServer.h>

// Konfigurasi Access Point Wi-Fi ESP32
const char *ssid = "Downhill_Timing_AP";
const char *password = "balap12345";

// WebSocket Server di port 81
WebSocketsServer webSocket = WebSocketsServer(81);

// Pin tombol BOOT bawaan ESP32 untuk tes simulasi finish radar
const int BUTTON_PIN = 0; 
int lastButtonState = HIGH;

// Handler event WebSocket
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;

    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Client terhubung dari %d.%d.%d.%d\n", num, ip[0], ip[1], ip[2], ip[3]);
        // Beri tahu web bahwa ESP32 siap
        webSocket.sendTXT(num, "STATUS:ESP32_READY");
      }
      break;

    case WStype_TEXT:
      Serial.printf("[%u] Pesan dari Web: %s\n", num, payload);
      break;
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Nyalakan Wi-Fi Mode SoftAP
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();

  Serial.println("\n--- ESP32 DOWNHILL TIMING ENGINE ---");
  Serial.print("Wi-Fi Hotspot: ");
  Serial.println(ssid);
  Serial.print("Alamat IP ESP32: ");
  Serial.println(IP);

  // Jalankan WebSocket server
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void loop() {
  webSocket.loop();

  // Deteksi pencetan tombol BOOT di board ESP32
  int currentButtonState = digitalRead(BUTTON_PIN);
  if (lastButtonState == HIGH && currentButtonState == LOW) {
    delay(50); // Debounce sederhana

    // Mengirim string murni sesuai format trigger finish
    // Contoh format teks mentah: "RADAR_FINISH_TRIGGER"
    String payload = "RADAR_FINISH_TRIGGER";
    webSocket.broadcastTXT(payload);

    Serial.println(">> Tombol BOOT ditekan! Mengirim string: " + payload);
  }
  lastButtonState = currentButtonState;
}