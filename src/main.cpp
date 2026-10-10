#include <Arduino.h>
#include <WiFi.h>
#include <WebSocketsServer.h>
#include <esp_timer.h>

const char *ssid = "Downhill_Timing_AP";
const char *password = "balap12345";

WebSocketsServer webSocket(81);

constexpr uint8_t BUTTON_PIN = 0;
constexpr uint32_t BUTTON_DEBOUNCE_MS = 50;
constexpr uint64_t SIMULATED_GPS_EPOCH_BASE_MS = 1700000000000ULL;

int lastButtonReading = HIGH;
int stableButtonState = HIGH;
uint32_t lastButtonChangeMs = 0;
uint64_t activeStartTimeMs = 0;
bool isRiderOnTrack = false;
String serialCommand;

uint64_t simulatedGpsEpochMs() {
  return SIMULATED_GPS_EPOCH_BASE_MS +
         static_cast<uint64_t>(esp_timer_get_time() / 1000ULL);
}

void simulateLoRaStartReception() {
  if (isRiderOnTrack) {
    Serial.println("START diabaikan: rider sebelumnya belum FINISH.");
    return;
  }

  activeStartTimeMs = simulatedGpsEpochMs();
  isRiderOnTrack = true;

  char payload[80];
  snprintf(payload, sizeof(payload),
           "{\"event\":\"START\",\"time\":%llu}",
           static_cast<unsigned long long>(activeStartTimeMs));
  webSocket.broadcastTXT(payload);

  Serial.printf("LoRa START tersimulasikan | epoch=%llu ms | payload=%s\n",
                static_cast<unsigned long long>(activeStartTimeMs), payload);
}

void handleFinishTrigger() {
  if (!isRiderOnTrack) {
    Serial.println("FINISH diabaikan: belum ada event START aktif.");
    return;
  }

  const uint64_t endTimeMs = simulatedGpsEpochMs();
  const uint64_t durationMs = endTimeMs - activeStartTimeMs;
  char payload[128];
  snprintf(payload, sizeof(payload),
           "{\"event\":\"FINISH\",\"startTime\":%llu,\"endTime\":%llu,\"duration\":%llu}",
           static_cast<unsigned long long>(activeStartTimeMs),
           static_cast<unsigned long long>(endTimeMs),
           static_cast<unsigned long long>(durationMs));
  webSocket.broadcastTXT(payload);

  Serial.printf("BOOT FINISH | start=%llu ms | end=%llu ms | duration=%llu ms | payload=%s\n",
                static_cast<unsigned long long>(activeStartTimeMs),
                static_cast<unsigned long long>(endTimeMs),
                static_cast<unsigned long long>(durationMs), payload);
  isRiderOnTrack = false;
}

void webSocketEvent(uint8_t client, WStype_t type, uint8_t *payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] WebSocket client disconnected\n", client);
      break;

    case WStype_CONNECTED: {
      const IPAddress ip = webSocket.remoteIP(client);
      Serial.printf("[%u] WebSocket connected: %d.%d.%d.%d\n",
                    client, ip[0], ip[1], ip[2], ip[3]);
      break;
    }

    case WStype_TEXT:
      Serial.printf("[%u] WebSocket message received (%u bytes)\n",
                    client, static_cast<unsigned int>(length));
      break;

    default:
      break;
  }
}

void handleSerialSimulation() {
  while (Serial.available() > 0) {
    const char character = static_cast<char>(Serial.read());
    if (character == '\n' || character == '\r') {
      serialCommand.trim();
      if (serialCommand.equalsIgnoreCase("LORA_START")) {
        simulateLoRaStartReception();
      } else if (!serialCommand.isEmpty()) {
        Serial.println("Perintah tidak dikenal. Gunakan LORA_START untuk simulasi LoRa.");
      }
      serialCommand = "";
    } else if (serialCommand.length() < 32) {
      serialCommand += character;
    }
  }
}

void handleBootButton() {
  const uint32_t now = millis();
  const int reading = digitalRead(BUTTON_PIN);

  if (reading != lastButtonReading) {
    lastButtonReading = reading;
    lastButtonChangeMs = now;
  }

  if (reading != stableButtonState &&
      static_cast<uint32_t>(now - lastButtonChangeMs) >= BUTTON_DEBOUNCE_MS) {
    stableButtonState = reading;
    if (stableButtonState == LOW) {
      handleFinishTrigger();
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  lastButtonReading = digitalRead(BUTTON_PIN);
  stableButtonState = lastButtonReading;

  WiFi.mode(WIFI_AP);
  if (!WiFi.softAP(ssid, password)) {
    Serial.println("Gagal mengaktifkan Access Point Wi-Fi.");
    return;
  }

  Serial.println("\n--- ESP32-S3 DOWNHILL TIMING ENGINE ---");
  Serial.printf("Wi-Fi AP: %s\n", ssid);
  Serial.printf("Password: %s\n", password);
  Serial.print("Alamat IP: ");
  Serial.println(WiFi.softAPIP());
  Serial.println("WebSocket server: port 81");
  Serial.println("Simulasi START LoRa: kirim LORA_START melalui Serial Monitor.");
  Serial.println("Simulasi FINISH: tekan tombol BOOT (GPIO 0).");
  Serial.println("Epoch GPS berjalan dari basis epoch simulasi + waktu monotonic perangkat.");

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void loop() {
  webSocket.loop();
  handleSerialSimulation();
  handleBootButton();
}
