/*
 * ============================================================
 *  BICYCLE RACING STOPWATCH - FINISH DEVICE FIRMWARE
 *  Hardware: ESP32-S2 Mini + RCWL-0516 + E220-400T30D + NEO-6M + 2x P10 Panels
 *  Mode: Standalone Access Point (AP) with Embedded Web Server
 * ============================================================
 *
 *  Libraries Required:
 *  - TinyGPSPlus
 *  - LoRa_E220 (by Renzo Mischianti)
 *  - ArduinoJson
 *  - SPIFFS (built-in ESP32)
 *  - WiFi & WebServer (built-in ESP32-S2)
 *
 *  Access Point:
 *  - SSID: RaceTimer-Finish
 *  - Password: race2024
 *  - URL: http://192.168.4.1
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <TinyGPSPlus.h>
#include <Ticker.h>
#include "LoRa_E220.h"
#include "finish_web_page.h"

// ===================== PIN DEFINITIONS =====================
// Radar Sensor (RCWL-0516)
#define RADAR_PIN         12

// GPS (NEO-6M via HardwareSerial 1)
#define GPS_RX_PIN        18  // ESP32 RX <- GPS TX
#define GPS_TX_PIN        17  // ESP32 TX -> GPS RX
#define GPS_BAUD          9600

// LoRa E220-400T30D (via HardwareSerial 0 / pins 37, 39)
#define LORA_RX_PIN       37  // ESP32 RX <- LoRa TX
#define LORA_TX_PIN       39  // ESP32 TX -> LoRa RX
#define LORA_AUX_PIN      16
#define LORA_M0_PIN       13
#define LORA_M1_PIN       14

// P10 Panel (HUB12) - 2 Panels Daisy-Chained (64x16 pixels total)
#define P10_DATA_PIN      11  // R1
#define P10_CLK_PIN       7   // CLK
#define P10_LAT_PIN       5   // LAT / STB
#define P10_OE_PIN        6   // OE
#define P10_A_PIN         1   // Row Address A
#define P10_B_PIN         4   // Row Address B

// ===================== CONFIGURATION =====================
#define WIFI_AP_SSID      "RaceTimer-Finish"
#define WIFI_AP_PASS      "race2024"
#define DEBOUNCE_MS       3000
#define HEARTBEAT_MS      20000
#define LORA_CHANNEL      23
#define P10_PANELS_X      2
#define P10_WIDTH         (32 * P10_PANELS_X)  // 64 pixels
#define P10_HEIGHT        16
#define MAX_RUNS          200
#define MAX_RIDERS        200
#define MAX_RIDER_BIB     4294967295ULL

// P10 Panel Configuration Flags
// Standard P10 HUB12: OE is active HIGH (HIGH = light up, LOW = blank)
#define P10_OE_ACTIVE_HIGH    true   // Standard P10 HUB12: OE active HIGH (HIGH = lit)
#define P10_REVERSE_PANELS   true   // Invert byte order for left-to-right daisy chain
#define P10_SHIFT_LSB_FIRST  true   // Fix horizontally mirrored characters (E -> E)
#define P10_REVERSE_ROWS     false  // Set to true if top & bottom rows are swapped

// ===================== P10 DOUBLE FRAMEBUFFER =====================
#define FB_WIDTH          P10_WIDTH
#define FB_HEIGHT         P10_HEIGHT
#define FB_STRIDE         (FB_WIDTH / 8)       // 8 bytes per row = 64 pixels

// Double Buffering: drawBuffer is manipulated by UI rendering, displayBuffer is scanned by Timer ISR
uint8_t drawBuffer[FB_STRIDE * FB_HEIGHT];
volatile uint8_t displayBuffer[FB_STRIDE * FB_HEIGHT];
Ticker p10ScanTicker;
volatile uint8_t p10ScanRow = 0;

// 5x7 ASCII Font (32-90)
const uint8_t font5x7[][5] PROGMEM = {
  {0x00,0x00,0x00,0x00,0x00}, // Space (32)
  {0x00,0x00,0x5F,0x00,0x00}, // ! (33)
  {0x00,0x07,0x00,0x07,0x00}, // " (34)
  {0x14,0x7F,0x14,0x7F,0x14}, // # (35)
  {0x24,0x2A,0x7F,0x2A,0x12}, // $ (36)
  {0x23,0x13,0x08,0x64,0x62}, // % (37)
  {0x36,0x49,0x55,0x22,0x50}, // & (38)
  {0x00,0x05,0x03,0x00,0x00}, // ' (39)
  {0x00,0x1C,0x22,0x41,0x00}, // ( (40)
  {0x00,0x41,0x22,0x1C,0x00}, // ) (41)
  {0x08,0x2A,0x1C,0x2A,0x08}, // * (42)
  {0x08,0x08,0x3E,0x08,0x08}, // + (43)
  {0x00,0x50,0x30,0x00,0x00}, // , (44)
  {0x08,0x08,0x08,0x08,0x08}, // - (45)
  {0x00,0x60,0x60,0x00,0x00}, // . (46)
  {0x20,0x10,0x08,0x04,0x02}, // / (47)
  {0x3E,0x51,0x49,0x45,0x3E}, // 0 (48)
  {0x00,0x42,0x7F,0x40,0x00}, // 1 (49)
  {0x42,0x61,0x51,0x49,0x46}, // 2 (50)
  {0x21,0x41,0x45,0x4B,0x31}, // 3 (51)
  {0x18,0x14,0x12,0x7F,0x10}, // 4 (52)
  {0x27,0x45,0x45,0x45,0x39}, // 5 (53)
  {0x3C,0x4A,0x49,0x49,0x30}, // 6 (54)
  {0x01,0x71,0x09,0x05,0x03}, // 7 (55)
  {0x36,0x49,0x49,0x49,0x36}, // 8 (56)
  {0x06,0x49,0x49,0x29,0x1E}, // 9 (57)
  {0x00,0x36,0x36,0x00,0x00}, // : (58)
  {0x00,0x56,0x36,0x00,0x00}, // ; (59)
  {0x08,0x14,0x22,0x41,0x00}, // < (60)
  {0x14,0x14,0x14,0x14,0x14}, // = (61)
  {0x00,0x41,0x22,0x14,0x08}, // > (62)
  {0x02,0x01,0x51,0x09,0x06}, // ? (63)
  {0x32,0x49,0x79,0x41,0x3E}, // @ (64)
  {0x7E,0x11,0x11,0x11,0x7E}, // A (65)
  {0x7F,0x49,0x49,0x49,0x36}, // B
  {0x3E,0x41,0x41,0x41,0x22}, // C
  {0x7F,0x41,0x41,0x22,0x1C}, // D
  {0x7F,0x49,0x49,0x49,0x41}, // E
  {0x7F,0x09,0x09,0x09,0x01}, // F
  {0x3E,0x41,0x49,0x49,0x7A}, // G
  {0x7F,0x08,0x08,0x08,0x7F}, // H
  {0x00,0x41,0x7F,0x41,0x00}, // I
  {0x20,0x40,0x41,0x3F,0x01}, // J
  {0x7F,0x08,0x14,0x22,0x41}, // K
  {0x7F,0x40,0x40,0x40,0x40}, // L
  {0x7F,0x02,0x04,0x02,0x7F}, // M
  {0x7F,0x04,0x08,0x10,0x7F}, // N
  {0x3E,0x41,0x41,0x41,0x3E}, // O
  {0x7F,0x09,0x09,0x09,0x06}, // P
  {0x3E,0x41,0x51,0x21,0x5E}, // Q
  {0x7F,0x09,0x19,0x29,0x46}, // R
  {0x46,0x49,0x49,0x49,0x31}, // S
  {0x01,0x01,0x7F,0x01,0x01}, // T
  {0x3F,0x40,0x40,0x40,0x3F}, // U
  {0x1F,0x20,0x40,0x20,0x1F}, // V
  {0x3F,0x40,0x38,0x40,0x3F}, // W
  {0x63,0x14,0x08,0x14,0x63}, // X
  {0x07,0x08,0x70,0x08,0x07}, // Y
  {0x61,0x51,0x49,0x45,0x43}, // Z (90)
};

// ===================== DATA STRUCTURES =====================
struct RunRecord {
  char bib[12];
  char name[36];
  char team[36];
  unsigned long startTimeMs;    // GPS time ms
  unsigned long finishTimeMs;   // GPS time ms
  unsigned long elapsedMs;      // Net duration
  char startTimeStr[16];
  char date[12];
  bool completed;
};

// -- start by rizki
struct RiderRecord {
  char bib[12];
  char name[36];
  char team[36];
};
// -- end by rizki

// ===================== STATE VARIABLES =====================
RunRecord runs[MAX_RUNS];
int runCount = 0;

// -- start by rizki
RiderRecord riders[MAX_RIDERS];
int riderCount = 0;
unsigned long long nextRiderBib = 101;
// -- end by rizki

// GPS
TinyGPSPlus gps;
HardwareSerial gpsSerial(1);
bool gpsReady = false;
uint8_t satelliteCount = 0;
unsigned long gpsMidnightMs = 0;
unsigned long gpsMillisCapture = 0;

// LoRa
HardwareSerial loraSerial(0);
LoRa_E220 lora(&loraSerial, LORA_AUX_PIN, LORA_M0_PIN, LORA_M1_PIN);
bool loraReady = false;
bool startDeviceOnline = false;
unsigned long lastStartHB = 0;
int startDeviceSat = 0;

// Active Stopwatch
WebServer server(80);
bool raceActive = false;
unsigned long raceStartTimeMs = 0;
unsigned long raceStartLocalMs = 0;
unsigned long raceElapsedMs = 0;
char currentBib[12] = "";
char currentName[36] = "";
char currentTeam[36] = "";
char currentStartTimeStr[16] = "";
char currentDate[12] = "";

// Last Completed Run
bool hasLastRun = false;
RunRecord lastRunData;

// Event Metadata
char eventName[64] = "Bicycle Race Championship";

// P10 Display State Machine
enum P10Mode {
  P10_STANDBY,        // Running text marquee (Event info, standby status, best time)
  P10_START_SEQUENCE, // Staged start intro (absorbs LoRa latency seamlessly)
  P10_RACING,         // Live running stopwatch with animated pacing chevrons
  P10_FINISH_FREEZE   // Freezes official finish time with blinking celebration
};

P10Mode p10Mode = P10_STANDBY;
unsigned long startAnimStartTime = 0;
unsigned long lastP10Update = 0;
unsigned long finishFreezeUntil = 0;
unsigned long radarBlankUntil = 0; // Mute finish radar during 1W LoRa TX bursts
unsigned long bootTestUntil = 0;   // Static diagnostic test screen duration
bool bootDone = false;             // Point 1: Boot screen strictly once per power cycle

// ===================== FUNCTION DECLARATIONS =====================
void initP10();
void initGPS();
void initLoRa();
void initWiFiAP();
void initWebServer();
void initSPIFFS();
void loadData();
void saveData();
// -- start by rizki
bool handleDnf();
bool saveRuns();
// -- end by rizki
// -- start by rizki
bool saveRiders();
bool saveNextRiderBib();
void loadRiders();
unsigned long long parseBibNumber(const char* bib);
// -- end by rizki
void updateGPS();
void handleLoRa();
void handleFinishTrigger(bool manual = false);
void updateP10Display();
void renderBootScreen();
void renderStandbyMode();
void renderStartSequenceMode();
void renderRacingMode();
void renderFinishFreezeMode();
void p10Clear();
void p10SetPixel(int x, int y, bool on);
void p10DrawHLine(int x0, int x1, int y, bool on);
void p10InvertArea(int x0, int y0, int x1, int y1);
void p10DrawChar(int x, int y, char c);
void p10DrawString(int x, int y, const char* str);
inline void p10ShiftByte(uint8_t data);
void p10RowScanISR();
void p10Commit();
unsigned long getBestRunMs(char* bestBibOut = nullptr, size_t bibLen = 0);
unsigned long getInterpolatedGPSMs();
String formatTimeShort(unsigned long ms);

// ===================== SETUP =====================
void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println(F("\n======================================"));
  Serial.println(F(" RACE STOPWATCH - FINISH DEVICE GATE"));
  Serial.println(F("======================================"));

  initSPIFFS();
  initP10();
  initGPS();
  initLoRa();
  initWiFiAP();
  initWebServer();
  loadData();
  // -- start by rizki
  loadRiders();
  // -- end by rizki

  // Point 1: Tampilan boot hanya sekali saja saat booting (2.5 detik)
  bootTestUntil = millis() + 2500;
  bootDone = false;
  Serial.println(F("[BOOT] Finish Device Ready! Connect to WiFi 'RaceTimer-Finish' -> http://192.168.4.1"));
}

// ===================== MAIN LOOP =====================
void loop() {
  server.handleClient();
  updateGPS();
  handleLoRa();

  // Check Finish Radar Sensor with 80ms noise filtering and TX blanking
  static unsigned long finishHighStart = 0;
  bool isBlanked = (millis() < radarBlankUntil);
  bool rawFinishRadar = !isBlanked && (digitalRead(RADAR_PIN) == HIGH);
  bool finishConfirmed = false;

  if (rawFinishRadar) {
    if (finishHighStart == 0) {
      finishHighStart = millis();
    } else if (millis() - finishHighStart >= 80) { // Must stay HIGH for at least 80ms
      finishConfirmed = true;
    }
  } else {
    finishHighStart = 0;
  }

  if (raceActive && finishConfirmed) {
    static unsigned long lastFinishTrigger = 0;
    if (millis() - lastFinishTrigger > DEBOUNCE_MS) {
      handleFinishTrigger(false);
      lastFinishTrigger = millis();
    }
  }

  // Update real-time elapsed duration
  if (raceActive) {
    raceElapsedMs = millis() - raceStartLocalMs;
  }

  // Refresh P10 display animation frame (~30 FPS)
  // (Multiplex scanning runs in background via Timer ISR at 125 FPS with zero flicker)
  if (millis() - lastP10Update >= 30) {
    updateP10Display();
    lastP10Update = millis();
  }

  // Heartbeat timeout for Start Gate (12s)
  if (startDeviceOnline && millis() - lastStartHB > 12000) {
    startDeviceOnline = false;
    Serial.println(F("[WARN] Start Gate heartbeat lost!"));
  }

  yield();
}

// ===================== INITIALIZATION =====================

void initSPIFFS() {
  if (!SPIFFS.begin(true)) {
    Serial.println(F("[ERROR] SPIFFS mount failed!"));
    return;
  }
  Serial.println(F("[SPIFFS] Storage mounted"));
}

void initP10() {
  pinMode(P10_DATA_PIN, OUTPUT);
  pinMode(P10_CLK_PIN, OUTPUT);
  pinMode(P10_LAT_PIN, OUTPUT);
  pinMode(P10_OE_PIN, OUTPUT);
  pinMode(P10_A_PIN, OUTPUT);
  pinMode(P10_B_PIN, OUTPUT);

  // Blank display during init
  digitalWrite(P10_OE_PIN, P10_OE_ACTIVE_HIGH ? LOW : HIGH);
  digitalWrite(P10_LAT_PIN, LOW);
  digitalWrite(P10_CLK_PIN, LOW);

  pinMode(RADAR_PIN, INPUT_PULLDOWN);

  p10Clear();
  p10Commit();

  // Attach background scan timer at 2ms (500Hz per row = 125Hz full frame)
  // Completely eliminates visible flicker and isolates display from WiFi/LoRa delays
  p10ScanTicker.attach_ms(2, p10RowScanISR);

  Serial.println(F("[P10] Dual Panels Initialized (64x16, Timer ISR Refresh @ 125 FPS)"));
}

void initGPS() {
  gpsSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
  Serial.println(F("[GPS] NEO-6M Serial initialized on GPIO 18/17"));
}

void initLoRa() {
  pinMode(LORA_AUX_PIN, INPUT_PULLUP);
  pinMode(LORA_M0_PIN, OUTPUT);
  pinMode(LORA_M1_PIN, OUTPUT);
  digitalWrite(LORA_M0_PIN, LOW);
  digitalWrite(LORA_M1_PIN, LOW);

  loraSerial.begin(9600, SERIAL_8N1, LORA_RX_PIN, LORA_TX_PIN);
  delay(150);
  lora.begin();

  Serial.println(F("[LORA] Memeriksa modul E220..."));
  ResponseStructContainer c = lora.getConfiguration();
  if (c.status.code == 1) {
    Configuration configuration = *(Configuration*)c.data;
    c.close();

    configuration.ADDL = 0x00;
    configuration.ADDH = 0x00;
    configuration.CHAN = LORA_CHANNEL;
    configuration.SPED.uartBaudRate = UART_BPS_9600;
    configuration.SPED.airDataRate = AIR_DATA_RATE_010_24;
    configuration.SPED.uartParity = MODE_00_8N1;
    configuration.OPTION.subPacketSetting = SPS_200_00;
    configuration.OPTION.transmissionPower = POWER_22; // 0b00 = Max Power (30dBm/1W on T30D hardware)
    configuration.OPTION.RSSIAmbientNoise = RSSI_AMBIENT_NOISE_DISABLED;
    configuration.TRANSMISSION_MODE.enableRSSI = RSSI_DISABLED;
    configuration.TRANSMISSION_MODE.fixedTransmission = FT_TRANSPARENT_TRANSMISSION;
    configuration.TRANSMISSION_MODE.enableLBT = LBT_DISABLED;
    configuration.TRANSMISSION_MODE.WORPeriod = WOR_2000_011;

    lora.setConfiguration(configuration, WRITE_CFG_PWR_DWN_SAVE);
    loraReady = true;
    Serial.println(F("[LORA] E220-400T30D Berhasil Dikonfigurasi (CH 23)"));
  } else {
    Serial.print(F("[LORA WARN] Modul E220 belum merespons (belum dapat daya 5V atau belum dipasang): "));
    Serial.println(c.status.getResponseDescription());
    c.close();
    loraReady = false;
  }
}

void initWiFiAP() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASS);
  IPAddress IP = WiFi.softAPIP();
  Serial.print(F("[WIFI AP] Started SSID: "));
  Serial.println(WIFI_AP_SSID);
  Serial.print(F("[WIFI AP] IP Address: "));
  Serial.println(IP);
}

void initWebServer() {
  // Serve Embedded HTML Web UI directly from Flash
  server.on("/", HTTP_GET, []() {
    server.send_P(200, "text/html", FINISH_HTML);
  });

  // API State for Dashboard Polling
  server.on("/api/state", HTTP_GET, []() {
    JsonDocument doc;
    doc["raceActive"] = raceActive;
    doc["elapsed"] = raceElapsedMs;
    doc["currentBib"] = currentBib;
    doc["currentName"] = currentName;
    doc["currentTeam"] = currentTeam;
    doc["currentStartTime"] = currentStartTimeStr;
    doc["gpsReady"] = gpsReady;
    doc["satellites"] = satelliteCount;
    doc["loraReady"] = loraReady;
    doc["startDeviceOnline"] = startDeviceOnline;
    doc["startDeviceSat"] = startDeviceSat;
    doc["runCount"] = runCount;
    doc["eventName"] = eventName;

    // Calculate Best Time
    unsigned long bestTime = 0xFFFFFFFF;
    int bestIdx = -1;
    for (int i = 0; i < runCount; i++) {
      if (runs[i].completed && runs[i].elapsedMs < bestTime) {
        bestTime = runs[i].elapsedMs;
        bestIdx = i;
      }
    }
    if (bestIdx >= 0) {
      doc["bestTime"] = bestTime;
      doc["bestRunnerBib"] = runs[bestIdx].bib;
      doc["bestRunnerName"] = runs[bestIdx].name;
    } else {
      doc["bestTime"] = 0;
      doc["bestRunnerBib"] = "";
      doc["bestRunnerName"] = "";
    }

    if (hasLastRun) {
      JsonObject lr = doc["lastRun"].to<JsonObject>();
      lr["bib"] = lastRunData.bib;
      lr["name"] = lastRunData.name;
      lr["team"] = lastRunData.team;
      lr["elapsed"] = lastRunData.elapsedMs;
      // -- start by rizki
      lr["completed"] = lastRunData.completed;
      // -- end by rizki
    }

    String res;
    serializeJson(doc, res);
    server.send(200, "application/json", res);
  });

  // API Leaderboard (Ranked by fastest elapsedMs)
  server.on("/api/leaderboard", HTTP_GET, []() {
    JsonDocument doc;
    JsonArray arr = doc.to<JsonArray>();

    // Copy pointers / indices to sort
    int sortedIndices[MAX_RUNS];
    int validCount = 0;
    for (int i = 0; i < runCount; i++) {
      sortedIndices[validCount++] = i;
    }

    // -- start by rizki
    // Keep finishers ranked by elapsed time and place DNF records after them.
    for (int i = 0; i < validCount - 1; i++) {
      for (int j = i + 1; j < validCount; j++) {
        const RunRecord& current = runs[sortedIndices[i]];
        const RunRecord& candidate = runs[sortedIndices[j]];
        bool candidateComesFirst =
          (candidate.completed && !current.completed) ||
          (candidate.completed == current.completed &&
           candidate.completed && candidate.elapsedMs < current.elapsedMs);
        if (candidateComesFirst) {
          int temp = sortedIndices[i];
          sortedIndices[i] = sortedIndices[j];
          sortedIndices[j] = temp;
        }
      }
    }
    // -- end by rizki

    for (int i = 0; i < validCount; i++) {
      int idx = sortedIndices[i];
      JsonObject obj = arr.add<JsonObject>();
      obj["bib"] = runs[idx].bib;
      obj["name"] = runs[idx].name;
      obj["team"] = runs[idx].team;
      obj["start"] = runs[idx].startTimeMs;
      obj["finish"] = runs[idx].finishTimeMs;
      obj["elapsed"] = runs[idx].elapsedMs;
      obj["startTimeStr"] = runs[idx].startTimeStr;
      obj["date"] = runs[idx].date;
      obj["completed"] = runs[idx].completed;
    }

    String res;
    serializeJson(doc, res);
    server.send(200, "application/json", res);
  });

  // API All Runs
  server.on("/api/runs", HTTP_GET, []() {
    JsonDocument doc;
    JsonArray arr = doc.to<JsonArray>();

    for (int i = 0; i < runCount; i++) {
      JsonObject obj = arr.add<JsonObject>();
      obj["bib"] = runs[i].bib;
      obj["name"] = runs[i].name;
      obj["team"] = runs[i].team;
      obj["elapsed"] = runs[i].elapsedMs;
      obj["startTime"] = runs[i].startTimeStr;
      // -- start by rizki
      obj["start"] = runs[i].startTimeMs;
      obj["finish"] = runs[i].finishTimeMs;
      obj["date"] = runs[i].date;
      obj["completed"] = runs[i].completed;
      // -- end by rizki
    }

    String res;
    serializeJson(doc, res);
    server.send(200, "application/json", res);
  });

  // -- start by rizki
  // Registered riders are persisted separately from race results.
  server.on("/api/riders", HTTP_GET, []() {
    JsonDocument doc;
    JsonArray arr = doc.to<JsonArray>();
    for (int i = 0; i < riderCount; i++) {
      JsonObject obj = arr.add<JsonObject>();
      obj["bib"] = riders[i].bib;
      obj["name"] = riders[i].name;
      obj["team"] = riders[i].team;
    }

    String res;
    serializeJson(doc, res);
    server.send(200, "application/json", res);
  });

  server.on("/api/riders", HTTP_POST, []() {
    if (!server.hasArg("plain")) {
      server.send(400, "application/json", "{\"error\":\"Missing payload\"}");
      return;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, server.arg("plain"));
    if (error) {
      server.send(400, "application/json", "{\"error\":\"Invalid JSON payload\"}");
      return;
    }

    const char* name = doc["name"] | "";
    size_t nameLength = strlen(name);
    if (nameLength == 0 || nameLength >= sizeof(riders[0].name)) {
      server.send(400, "application/json", "{\"error\":\"Name must be 1 to 35 UTF-8 bytes\"}");
      return;
    }
    if (riderCount >= MAX_RIDERS) {
      server.send(409, "application/json", "{\"error\":\"Rider roster is full\"}");
      return;
    }

    if (nextRiderBib > MAX_RIDER_BIB) {
      server.send(409, "application/json", "{\"error\":\"No more rider bib numbers available\"}");
      return;
    }

    unsigned long long allocatedBib = nextRiderBib;
    nextRiderBib++;
    if (!saveNextRiderBib()) {
      nextRiderBib--;
      server.send(500, "application/json", "{\"error\":\"Failed to reserve the next rider bib\"}");
      return;
    }

    RiderRecord& rider = riders[riderCount];
    snprintf(rider.bib, sizeof(rider.bib), "%llu", allocatedBib);
    strlcpy(rider.name, name, sizeof(rider.name));
    strlcpy(rider.team, "Men Elite Downhill", sizeof(rider.team));
    riderCount++;

    if (!saveRiders()) {
      riderCount--;
      server.send(500, "application/json", "{\"error\":\"Failed to save rider roster\"}");
      return;
    }

    JsonDocument response;
    response["success"] = true;
    response["bib"] = rider.bib;
    response["name"] = rider.name;
    response["team"] = rider.team;
    String res;
    serializeJson(response, res);
    server.send(201, "application/json", res);
  });

  server.on("/api/riders/update", HTTP_POST, []() {
    if (!server.hasArg("plain")) {
      server.send(400, "application/json", "{\"error\":\"Missing payload\"}");
      return;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, server.arg("plain"));
    if (error) {
      server.send(400, "application/json", "{\"error\":\"Invalid JSON payload\"}");
      return;
    }

    const char* bib = doc["bib"] | "";
    const char* name = doc["name"] | "";
    size_t nameLength = strlen(name);
    if (strlen(bib) == 0 || nameLength == 0 || nameLength >= sizeof(riders[0].name)) {
      server.send(400, "application/json", "{\"error\":\"Bib and a 1 to 35 byte UTF-8 name are required\"}");
      return;
    }
    if (raceActive && strcmp(currentBib, bib) == 0) {
      server.send(409, "application/json", "{\"error\":\"Cannot edit the rider currently racing\"}");
      return;
    }

    int riderIndex = -1;
    for (int i = 0; i < riderCount; i++) {
      if (strcmp(riders[i].bib, bib) == 0) {
        riderIndex = i;
        break;
      }
    }
    if (riderIndex < 0) {
      server.send(404, "application/json", "{\"error\":\"Rider not found\"}");
      return;
    }

    RiderRecord original = riders[riderIndex];
    strlcpy(riders[riderIndex].name, name, sizeof(riders[riderIndex].name));
    const char* team = doc["team"] | riders[riderIndex].team;
    strlcpy(riders[riderIndex].team, team, sizeof(riders[riderIndex].team));
    if (!saveRiders()) {
      riders[riderIndex] = original;
      server.send(500, "application/json", "{\"error\":\"Failed to save rider changes\"}");
      return;
    }

    JsonDocument response;
    response["success"] = true;
    response["bib"] = riders[riderIndex].bib;
    response["name"] = riders[riderIndex].name;
    response["team"] = riders[riderIndex].team;
    String res;
    serializeJson(response, res);
    server.send(200, "application/json", res);
  });

  server.on("/api/riders/delete", HTTP_POST, []() {
    if (!server.hasArg("plain")) {
      server.send(400, "application/json", "{\"error\":\"Missing payload\"}");
      return;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, server.arg("plain"));
    if (error) {
      server.send(400, "application/json", "{\"error\":\"Invalid JSON payload\"}");
      return;
    }

    const char* bib = doc["bib"] | "";
    if (strlen(bib) == 0) {
      server.send(400, "application/json", "{\"error\":\"Bib is required\"}");
      return;
    }
    if (raceActive && strcmp(currentBib, bib) == 0) {
      server.send(409, "application/json", "{\"error\":\"Cannot delete the rider currently racing\"}");
      return;
    }

    int riderIndex = -1;
    for (int i = 0; i < riderCount; i++) {
      if (strcmp(riders[i].bib, bib) == 0) {
        riderIndex = i;
        break;
      }
    }
    if (riderIndex < 0) {
      server.send(404, "application/json", "{\"error\":\"Rider not found\"}");
      return;
    }

    RiderRecord removed = riders[riderIndex];
    for (int i = riderIndex; i < riderCount - 1; i++) {
      riders[i] = riders[i + 1];
    }
    riderCount--;
    if (!saveRiders()) {
      for (int i = riderCount; i > riderIndex; i--) {
        riders[i] = riders[i - 1];
      }
      riders[riderIndex] = removed;
      riderCount++;
      server.send(500, "application/json", "{\"error\":\"Failed to delete rider\"}");
      return;
    }

    server.send(200, "application/json", "{\"success\":true}");
  });

  server.on("/api/riders/clear", HTTP_POST, []() {
    if (raceActive) {
      server.send(409, "application/json", "{\"error\":\"Cannot clear rider roster while a race is active\"}");
      return;
    }
    if (SPIFFS.exists("/riders.json") && !SPIFFS.remove("/riders.json")) {
      server.send(500, "application/json", "{\"error\":\"Failed to delete rider roster\"}");
      return;
    }

    riderCount = 0;
    server.send(200, "application/json", "{\"success\":true}");
  });
  // -- end by rizki

  // API Manual Finish
  server.on("/api/manual-finish", HTTP_POST, []() {
    if (!raceActive) {
      server.send(400, "application/json", "{\"error\":\"No active race\"}");
      return;
    }
    handleFinishTrigger(true);
    server.send(200, "application/json", "{\"success\":true}");
  });

  // -- start by rizki
  server.on("/api/dnf", HTTP_POST, []() {
    if (!raceActive) {
      server.send(400, "application/json", "{\"error\":\"No active race\"}");
      return;
    }
    if (runCount >= MAX_RUNS) {
      server.send(409, "application/json", "{\"error\":\"Run history is full\"}");
      return;
    }
    if (!handleDnf()) {
      server.send(500, "application/json", "{\"error\":\"Failed to save DNF record\"}");
      return;
    }
    server.send(200, "application/json", "{\"success\":true}");
  });
  // -- end by rizki

  // API Reset Race
  server.on("/api/reset-race", HTTP_POST, []() {
    raceActive = false;
    currentBib[0] = '\0';
    currentName[0] = '\0';
    currentTeam[0] = '\0';
    p10Mode = P10_STANDBY;
    server.send(200, "application/json", "{\"success\":true}");
  });

  // API Set Event Name
  server.on("/api/set-event", HTTP_POST, []() {
    if (!server.hasArg("plain")) {
      server.send(400, "application/json", "{\"error\":\"Missing payload\"}");
      return;
    }
    JsonDocument doc;
    deserializeJson(doc, server.arg("plain"));
    const char* name = doc["name"] | "";
    if (strlen(name) > 0) {
      strlcpy(eventName, name, sizeof(eventName));
      saveData();
    }
    server.send(200, "application/json", "{\"success\":true}");
  });

  // API Clear All Data
  server.on("/api/clear-all", HTTP_POST, []() {
    runCount = 0;
    raceActive = false;
    hasLastRun = false;
    currentBib[0] = '\0';
    saveData();
    p10Mode = P10_STANDBY;
    server.send(200, "application/json", "{\"success\":true}");
  });

  server.begin();
  Serial.println(F("[HTTP] Web Server started"));
}

// ===================== GPS & TIME =====================

void updateGPS() {
  while (gpsSerial.available() > 0) {
    char c = gpsSerial.read();
    if (gps.encode(c)) {
      if (gps.satellites.isValid()) {
        satelliteCount = gps.satellites.value();
      }
      if (gps.time.isValid() && gps.date.isValid()) {
        unsigned long ms = 0;
        ms += (unsigned long)gps.time.hour() * 3600000UL;
        ms += (unsigned long)gps.time.minute() * 60000UL;
        ms += (unsigned long)gps.time.second() * 1000UL;
        ms += (unsigned long)gps.time.centisecond() * 10UL;

        gpsMidnightMs = ms;
        gpsMillisCapture = millis();

        if (!gpsReady) {
          gpsReady = true;
          Serial.println(F("[GPS] Fix acquired!"));
        }
      }
    }
  }
}

unsigned long getInterpolatedGPSMs() {
  if (!gpsReady) return millis();
  unsigned long elapsedSinceCapture = millis() - gpsMillisCapture;
  return gpsMidnightMs + elapsedSinceCapture;
}

// ===================== LORA RECEIVER =====================

void handleLoRa() {
  if (!loraReady || lora.available() <= 0) return;

  ResponseContainer rc = lora.receiveMessage();
  if (rc.status.code != 1) return;

  String msg = rc.data;
  Serial.println("[LORA RX] " + msg);

  // START Signal: CMD:START|BIB:<bib>|NAME:<name>|TEAM:<team>|TIME:<ms>|DATE:<date>|SAT:<sats>
  if (msg.startsWith("CMD:START")) {
    int bibIdx = msg.indexOf("BIB:");
    int nameIdx = msg.indexOf("NAME:");
    int teamIdx = msg.indexOf("TEAM:");
    int timeIdx = msg.indexOf("TIME:");
    int dateIdx = msg.indexOf("DATE:");
    int satIdx  = msg.indexOf("SAT:");

    if (timeIdx > 0) {
      String bibStr = (bibIdx > 0) ? msg.substring(bibIdx + 4, msg.indexOf('|', bibIdx + 4)) : "00";
      String nameStr = (nameIdx > 0) ? msg.substring(nameIdx + 5, msg.indexOf('|', nameIdx + 5)) : "Racer";
      String teamStr = (teamIdx > 0) ? msg.substring(teamIdx + 5, msg.indexOf('|', teamIdx + 5)) : "";
      unsigned long startTime = msg.substring(timeIdx + 5, msg.indexOf('|', timeIdx + 5)).toInt();

      strlcpy(currentBib, bibStr.c_str(), sizeof(currentBib));
      strlcpy(currentName, nameStr.c_str(), sizeof(currentName));
      strlcpy(currentTeam, teamStr.c_str(), sizeof(currentTeam));
      raceStartTimeMs = startTime;

      if (dateIdx > 0) {
        String dateStr = msg.substring(dateIdx + 5, msg.indexOf('|', dateIdx + 5));
        strlcpy(currentDate, dateStr.c_str(), sizeof(currentDate));
      }

      // Format start time string
      unsigned long sec = (startTime / 1000) % 60;
      unsigned long min = (startTime / 60000) % 60;
      unsigned long hr  = (startTime / 3600000) % 24;
      snprintf(currentStartTimeStr, sizeof(currentStartTimeStr), "%02lu:%02lu:%02lu", hr, min, sec);

      // Compensate for LoRa latency using GPS time
      unsigned long nowGPS = getInterpolatedGPSMs();
      unsigned long alreadyElapsed = (nowGPS > startTime) ? (nowGPS - startTime) : 0;

      raceStartLocalMs = millis() - alreadyElapsed;
      raceElapsedMs = alreadyElapsed;
      raceActive = true;

      Serial.println("[RACE START] BIB: " + bibStr + " | " + nameStr + " | Latency: " + String(alreadyElapsed) + "ms");

      // Send ACK back to Start Device
      char ack[48];
      snprintf(ack, sizeof(ack), "CMD:ACK|BIB:%s|TIME:%lu", currentBib, nowGPS);
      radarBlankUntil = millis() + 2000;
      lora.sendMessage(String(ack));

      // Trigger staged Start Animation sequence on P10 (non-blocking)
      p10Mode = P10_START_SEQUENCE;
      startAnimStartTime = millis();
    }
  }
  // Heartbeat from Start Gate: CMD:HB|DEV:START|SAT:<sats>|GPS:<0|1>
  else if (msg.startsWith("CMD:HB")) {
    startDeviceOnline = true;
    lastStartHB = millis();
    int satIdx = msg.indexOf("SAT:");
    if (satIdx > 0) {
      startDeviceSat = msg.substring(satIdx + 4, msg.indexOf('|', satIdx + 4)).toInt();
    }
    // Reply with heartbeat ack
    char hback[48];
    snprintf(hback, sizeof(hback), "CMD:HBACK|DEV:FINISH|SAT:%d|GPS:%d",
             satelliteCount, gpsReady ? 1 : 0);
    radarBlankUntil = millis() + 2000;
    lora.sendMessage(String(hback));
  }
}

// ===================== FINISH TRIGGER =====================

void handleFinishTrigger(bool manual) {
  if (!raceActive) return;

  unsigned long finishGPSMs = getInterpolatedGPSMs();
  unsigned long elapsed = raceElapsedMs;

  if (gpsReady && finishGPSMs > raceStartTimeMs) {
    elapsed = finishGPSMs - raceStartTimeMs;
  }

  raceActive = false;
  raceElapsedMs = elapsed;

  Serial.println("[FINISH] BIB: " + String(currentBib) + " | " + String(currentName) +
                 " | Time: " + formatTimeShort(elapsed));

  // Record Run
  if (runCount < MAX_RUNS) {
    strlcpy(runs[runCount].bib, currentBib, sizeof(runs[runCount].bib));
    strlcpy(runs[runCount].name, currentName, sizeof(runs[runCount].name));
    strlcpy(runs[runCount].team, currentTeam, sizeof(runs[runCount].team));
    runs[runCount].startTimeMs = raceStartTimeMs;
    runs[runCount].finishTimeMs = finishGPSMs;
    runs[runCount].elapsedMs = elapsed;
    strlcpy(runs[runCount].startTimeStr, currentStartTimeStr, sizeof(runs[runCount].startTimeStr));
    strlcpy(runs[runCount].date, currentDate, sizeof(runs[runCount].date));
    runs[runCount].completed = true;

    // Cache last run
    lastRunData = runs[runCount];
    hasLastRun = true;
    runCount++;
  }

  saveData();

  // Transmit Finish notification to Start Device via LoRa
  char finishPacket[80];
  snprintf(finishPacket, sizeof(finishPacket),
           "CMD:FINISH|BIB:%s|TIME:%lu|ELAPSED:%lu",
           currentBib, finishGPSMs, elapsed);
  radarBlankUntil = millis() + 2000;
  lora.sendMessage(String(finishPacket));

  // Trigger official finish celebration mode on P10 (non-blocking)
  p10Mode = P10_FINISH_FREEZE;
  finishFreezeUntil = millis() + 10000; // Freeze & celebrate for 10 seconds
}

// -- start by rizki
bool handleDnf() {
  if (!raceActive || runCount >= MAX_RUNS) return false;

  const unsigned long dnfTimeMs = getInterpolatedGPSMs();
  const unsigned long elapsed = millis() - raceStartLocalMs;
  RunRecord& record = runs[runCount];
  memset(&record, 0, sizeof(record));
  strlcpy(record.bib, currentBib, sizeof(record.bib));
  strlcpy(record.name, currentName, sizeof(record.name));
  strlcpy(record.team, currentTeam, sizeof(record.team));
  record.startTimeMs = raceStartTimeMs;
  record.finishTimeMs = dnfTimeMs;
  record.elapsedMs = elapsed;
  strlcpy(record.startTimeStr, currentStartTimeStr, sizeof(record.startTimeStr));
  strlcpy(record.date, currentDate, sizeof(record.date));
  record.completed = false;

  runCount++;
  if (!saveRuns()) {
    runCount--;
    Serial.println(F("[DNF] Could not persist DNF record; active race remains unchanged"));
    return false;
  }

  raceElapsedMs = elapsed;
  raceActive = false;
  lastRunData = record;
  hasLastRun = true;
  p10Mode = P10_FINISH_FREEZE;
  finishFreezeUntil = millis() + 10000;

  Serial.println("[DNF] BIB: " + String(record.bib) + " | " + String(record.name) +
                 " | Elapsed: " + formatTimeShort(elapsed));
  return true;
}
// -- end by rizki

// ===================== P10 DISPLAY DRIVER =====================

void p10Clear() {
  // On standard HUB12: 0 bit = LED ON, 1 bit = LED OFF.
  // 0xFF sets all bits to 1, turning all LEDs OFF.
  memset(drawBuffer, 0xFF, sizeof(drawBuffer));
}

void p10SetPixel(int x, int y, bool on) {
  if (x < 0 || x >= FB_WIDTH || y < 0 || y >= FB_HEIGHT) return;

  // Horizontal mirror correction: maps x from 0..63 to 63..0
  // Ensures characters and words read naturally from left to right
  x = (FB_WIDTH - 1) - x;

  int addr = (x / 8) + y * (P10_PANELS_X * 4);
  byte mask = 0x80 >> (x & 0x07);

  if (on) {
    drawBuffer[addr] &= ~mask; // 0 bit is LED ON
  } else {
    drawBuffer[addr] |= mask;  // 1 bit is LED OFF
  }
}

void p10DrawChar(int x, int y, char c) {
  c = toupper(c);
  if (c < 32 || c > 90) c = ' ';
  int idx = c - 32;
  if (idx >= (int)(sizeof(font5x7) / 5)) return;

  for (int col = 0; col < 5; col++) {
    uint8_t colData = pgm_read_byte(&font5x7[idx][col]);
    for (int row = 0; row < 7; row++) {
      if (colData & (1 << row)) {
        p10SetPixel(x + col, y + row, true);
      }
    }
  }
}

void p10DrawString(int x, int y, const char* str) {
  int cx = x;
  while (*str) {
    p10DrawChar(cx, y, *str);
    cx += 6;
    str++;
  }
}

inline void p10ShiftByte(uint8_t data) {
  for (int b = 0; b < 8; b++) {
    int bit = P10_SHIFT_LSB_FIRST ? b : (7 - b);
    digitalWrite(P10_DATA_PIN, (data >> bit) & 1);
    digitalWrite(P10_CLK_PIN, HIGH);
    delayMicroseconds(1);
    digitalWrite(P10_CLK_PIN, LOW);
  }
}

// Background Timer ISR: refreshes 1 multiplex phase every 2ms (125 FPS full frame, zero flicker)
void p10RowScanISR() {
  int rowsize = P10_PANELS_X * 4; // 8 bytes for 2 panels
  int offset = rowsize * p10ScanRow;

  // 1. Shift out 4 interleaved rows (32 bytes total)
  for (int i = 0; i < rowsize; i++) {
    int col = P10_REVERSE_PANELS ? (rowsize - 1 - i) : i;

    if (!P10_REVERSE_ROWS) {
      p10ShiftByte(displayBuffer[offset + col + 96]); // row 12 + scanRow
      p10ShiftByte(displayBuffer[offset + col + 64]); // row 8 + scanRow
      p10ShiftByte(displayBuffer[offset + col + 32]); // row 4 + scanRow
      p10ShiftByte(displayBuffer[offset + col]);      // row 0 + scanRow
    } else {
      p10ShiftByte(displayBuffer[offset + col]);      // row 0 + scanRow
      p10ShiftByte(displayBuffer[offset + col + 32]); // row 4 + scanRow
      p10ShiftByte(displayBuffer[offset + col + 64]); // row 8 + scanRow
      p10ShiftByte(displayBuffer[offset + col + 96]); // row 12 + scanRow
    }
  }

  // 2. Blank display (OE OFF) momentarily before switching row lines and latching
  digitalWrite(P10_OE_PIN, P10_OE_ACTIVE_HIGH ? LOW : HIGH);

  // 3. Latch shifted data to output storage registers
  digitalWrite(P10_LAT_PIN, HIGH);
  delayMicroseconds(1);
  digitalWrite(P10_LAT_PIN, LOW);

  // 4. Select row address (A, B)
  digitalWrite(P10_A_PIN, p10ScanRow & 0x01);
  digitalWrite(P10_B_PIN, (p10ScanRow >> 1) & 0x01);

  // 5. Unblank display (OE ON) to illuminate active rows continuously
  digitalWrite(P10_OE_PIN, P10_OE_ACTIVE_HIGH ? HIGH : LOW);

  // Advance to next row phase (0, 1, 2, 3)
  p10ScanRow = (p10ScanRow + 1) & 0x03;
}

// Atomically commit drawBuffer to displayBuffer (double buffering)
void p10Commit() {
  memcpy((void*)displayBuffer, drawBuffer, sizeof(drawBuffer));
}

void p10Refresh() {
  p10Commit();
}

void p10DrawHLine(int x0, int x1, int y, bool on) {
  if (y < 0 || y >= FB_HEIGHT) return;
  if (x0 > x1) { int t = x0; x0 = x1; x1 = t; }
  for (int x = x0; x <= x1; x++) {
    p10SetPixel(x, y, on);
  }
}

void p10InvertArea(int x0, int y0, int x1, int y1) {
  for (int y = y0; y <= y1; y++) {
    for (int x = x0; x <= x1; x++) {
      if (x < 0 || x >= FB_WIDTH || y < 0 || y >= FB_HEIGHT) continue;
      int px = (FB_WIDTH - 1) - x;
      int addr = (px / 8) + y * (P10_PANELS_X * 4);
      byte mask = 0x80 >> (px & 0x07);
      drawBuffer[addr] ^= mask;
    }
  }
}

unsigned long getBestRunMs(char* bestBibOut, size_t bibLen) {
  unsigned long best = 0xFFFFFFFF;
  int bestIdx = -1;
  for (int i = 0; i < runCount; i++) {
    if (runs[i].completed && runs[i].elapsedMs > 0) {
      if (runs[i].elapsedMs < best) {
        best = runs[i].elapsedMs;
        bestIdx = i;
      }
    }
  }
  if (bestIdx >= 0) {
    if (bestBibOut && bibLen > 0) {
      strlcpy(bestBibOut, runs[bestIdx].bib, bibLen);
    }
    return best;
  }
  return 0;
}

// Point 1: Boot Screen (Hanya muncul sekali saja saat awal dinyalakan)
void renderBootScreen() {
  p10DrawHLine(0, FB_WIDTH - 1, 0, true);
  p10DrawHLine(0, FB_WIDTH - 1, FB_HEIGHT - 1, true);
  for (int y = 0; y < FB_HEIGHT; y++) {
    p10SetPixel(0, y, true);
    p10SetPixel(FB_WIDTH - 1, y, true);
  }
  // "READY" = 5 karakter = 29 piksel. Centered tepat di tengah (64 - 29)/2 = 17, y = 4
  p10DrawString(17, 4, "READY");
}

// Point 2: Scroll Nama Acara di Baris Atas dan Standby di Baris Bawah
void renderStandbyMode() {
  // Baris Atas (y = 1): Smooth Scrolling Nama Acara (eventName)
  static String lastEventName = "";
  static String scrollEventText = "";
  static int eventX = 64;
  static unsigned long lastEventScrollTick = 0;

  // Siapkan teks marquee jika eventName berubah atau pertama kali
  if (scrollEventText.length() == 0 || String(eventName) != lastEventName) {
    lastEventName = String(eventName);
    if (lastEventName.length() == 0) lastEventName = "BICYCLE RACE";
    scrollEventText = lastEventName + "       ";
    eventX = 64;
  }

  // Geser 1 piksel setiap 32 ms untuk scrolling yang mulus
  if (millis() - lastEventScrollTick >= 32) {
    eventX--;
    int totalPx = (int)scrollEventText.length() * 6;
    if (eventX <= -totalPx) {
      eventX = 64;
    }
    lastEventScrollTick = millis();
  }

  for (int i = 0; i < (int)scrollEventText.length(); i++) {
    int cx = eventX + (i * 6);
    if (cx >= -5 && cx < 64) {
      p10DrawChar(cx, 1, scrollEventText[i]);
    }
  }

  // Pembatas Halus (y = 8): Titik-titik pembatas 1 piksel
  for (int x = 0; x < 64; x += 4) {
    p10SetPixel(x, 8, true);
  }

  // Baris Bawah (y = 9): Teks "STANDBY" terpusat tepat di tengah
  // "STANDBY" = 7 karakter = 41 piksel. Centered di (64 - 41) / 2 = 11!
  p10DrawString(11, 9, "STANDBY");
}

// Point 3: Animasi Nama Peserta di Baris Atas dan Live Waktu di Baris Bawah
void renderStartSequenceMode() {
  unsigned long elapsed = millis() - startAnimStartTime;

  // Setelah animasi intro nama 1.5 detik selesai, beralih mulus ke mode balap reguler
  if (elapsed >= 1500) {
    p10Mode = P10_RACING;
    renderRacingMode();
    return;
  }

  // Baris Atas (y = 1): Animasi Nama Peserta
  String riderLabel = "#" + String(currentBib);
  if (strlen(currentName) > 0) {
    riderLabel += " " + String(currentName);
  }
  int riderPx = (int)riderLabel.length() * 6 - 1;
  int targetX = (riderPx <= 60) ? ((FB_WIDTH - riderPx) / 2) : 2;

  if (elapsed < 600) {
    // Fase 1 (0..600ms): Teks meluncur (slide-in) dari sisi kanan (x=64) ke target posisi
    int curX = targetX + ((64 - targetX) * (600 - (int)elapsed)) / 600;
    for (int i = 0; i < (int)riderLabel.length(); i++) {
      int cx = curX + (i * 6);
      if (cx >= -5 && cx < 64) {
        p10DrawChar(cx, 1, riderLabel[i]);
      }
    }
  } else {
    // Fase 2 (600..1500ms): Teks sudah berada di posisi target
    for (int i = 0; i < (int)riderLabel.length(); i++) {
      int cx = targetX + (i * 6);
      if (cx >= -5 && cx < 64) {
        p10DrawChar(cx, 1, riderLabel[i]);
      }
    }
    // Efek highlight sejenak di baris atas pada start
    if ((elapsed >= 700 && elapsed < 900) || (elapsed >= 1100 && elapsed < 1300)) {
      p10InvertArea(0, 0, 63, 7);
    }
  }

  // Pembatas Halus (y = 8)
  for (int x = 0; x < 64; x += 4) {
    p10SetPixel(x, 8, true);
  }

  // Baris Bawah (y = 9): WAKTU (LIVE STOPWATCH MM:SS.cs)
  String timeStr = formatTimeShort(raceElapsedMs);
  int len = timeStr.length();
  int sx = (FB_WIDTH - (len * 6 - 1)) / 2;
  if (sx < 0) sx = 0;
  p10DrawString(sx, 9, timeStr.c_str());
}

// Point 3: Live Balap Reguler (Nama Peserta di Atas, Live Waktu di Bawah)
void renderRacingMode() {
  // Baris Atas (y = 1): Nama Peserta
  String riderLabel = "#" + String(currentBib);
  if (strlen(currentName) > 0) {
    riderLabel += " " + String(currentName);
  }
  int riderPx = (int)riderLabel.length() * 6 - 1;

  static int riderScrollX = 64;
  static unsigned long lastRiderScrollTick = 0;
  static String lastRiderLabel = "";

  if (riderLabel != lastRiderLabel) {
    lastRiderLabel = riderLabel;
    riderScrollX = 64;
  }

  // Jika muat di layar (<= 10 karakter): tampilkan terpusat
  if (riderPx <= 60) {
    int rx = (FB_WIDTH - riderPx) / 2;
    p10DrawString(rx, 1, riderLabel.c_str());
  } else {
    // Jika nama panjang (> 10 karakter): auto-scroll mulus di baris atas
    if (millis() - lastRiderScrollTick >= 35) {
      riderScrollX--;
      String fullScroll = riderLabel + "       ";
      int fullPx = (int)fullScroll.length() * 6;
      if (riderScrollX <= -fullPx) {
        riderScrollX = 64;
      }
      lastRiderScrollTick = millis();
    }

    String fullScroll = riderLabel + "       ";
    for (int i = 0; i < (int)fullScroll.length(); i++) {
      int cx = riderScrollX + (i * 6);
      if (cx >= -5 && cx < 64) {
        p10DrawChar(cx, 1, fullScroll[i]);
      }
    }
  }

  // Pembatas Halus (y = 8)
  for (int x = 0; x < 64; x += 4) {
    p10SetPixel(x, 8, true);
  }

  // Baris Bawah (y = 9): WAKTU (LIVE STOPWATCH MM:SS.cs)
  String timeStr = formatTimeShort(raceElapsedMs);
  int len = timeStr.length();
  int sx = (FB_WIDTH - (len * 6 - 1)) / 2;
  if (sx < 0) sx = 0;
  p10DrawString(sx, 9, timeStr.c_str());
}

// Point 4: Tampil Waktu Finish, Timeout 10 Detik Kembali ke Point 2 (atau ke Point 3 jika ada data start)
void renderFinishFreezeMode() {
  // Jika waktu 10 detik habis dan belum ada data start baru: kembali ke Point 2 (Standby)
  if (millis() >= finishFreezeUntil) {
    p10Mode = P10_STANDBY;
    return;
  }

  unsigned long freezeElapsed = 10000 - (finishFreezeUntil - millis());

  // Baris Atas (y = 1): Identitas Pembalap yang Finish
  // -- start by rizki
  String finishLabel = "#" + String(hasLastRun ? lastRunData.bib : currentBib);
  if (hasLastRun && !lastRunData.completed) finishLabel += " DNF";
  if (hasLastRun && strlen(lastRunData.name) > 0) {
    finishLabel += " " + String(lastRunData.name);
  } else {
    finishLabel += hasLastRun && !lastRunData.completed ? "" : " FINISH";
  }
  // -- end by rizki

  int fPx = (int)finishLabel.length() * 6 - 1;
  if (fPx <= 60) {
    int fx = (FB_WIDTH - fPx) / 2;
    p10DrawString(fx, 1, finishLabel.c_str());
  } else {
    // Jika nama panjang, auto-scroll di baris atas
    static int fScrollX = 64;
    static unsigned long lastFScroll = 0;
    static String lastFLabel = "";

    if (finishLabel != lastFLabel) {
      lastFLabel = finishLabel;
      fScrollX = 64;
    }

    if (millis() - lastFScroll >= 35) {
      fScrollX--;
      String fFull = finishLabel + "       ";
      if (fScrollX <= -(int)(fFull.length() * 6)) fScrollX = 64;
      lastFScroll = millis();
    }
    String fFull = finishLabel + "       ";
    for (int i = 0; i < (int)fFull.length(); i++) {
      int cx = fScrollX + (i * 6);
      if (cx >= -5 && cx < 64) {
        p10DrawChar(cx, 1, fFull[i]);
      }
    }
  }

  // Pembatas Solid di y = 8
  p10DrawHLine(0, 63, 8, true);

  // Baris Bawah (y = 9): WAKTU FINISH RESMI (MM:SS.cs)
  // -- start by rizki
  String timeStr = hasLastRun && !lastRunData.completed
    ? String("DNF")
    : formatTimeShort(hasLastRun ? lastRunData.elapsedMs : raceElapsedMs);
  // -- end by rizki
  int len = timeStr.length();
  int sx = (FB_WIDTH - (len * 6 - 1)) / 2;
  if (sx < 0) sx = 0;
  p10DrawString(sx, 9, timeStr.c_str());

  // Selebrasi kedip 3 detik pertama
  if (freezeElapsed < 3000) {
    if ((freezeElapsed / 250) % 2 == 0) {
      p10InvertArea(0, 9, 63, 15);
    }
  }
}

// Controller Display Update
void updateP10Display() {
  p10Clear();

  // Point 1: Tampilan boot hanya sekali saja saat dinyalakan (2.5 detik)
  if (!bootDone) {
    if (millis() < bootTestUntil) {
      renderBootScreen();
      p10Commit();
      return;
    } else {
      bootDone = true;
      p10Mode = P10_STANDBY;
    }
  }

  switch (p10Mode) {
    case P10_STANDBY:
      renderStandbyMode();
      break;
    case P10_START_SEQUENCE:
      renderStartSequenceMode();
      break;
    case P10_RACING:
      renderRacingMode();
      break;
    case P10_FINISH_FREEZE:
      renderFinishFreezeMode();
      break;
  }

  p10Commit();
}

String formatTimeShort(unsigned long ms) {
  unsigned long totalSec = ms / 1000;
  unsigned long minutes = totalSec / 60;
  unsigned long seconds = totalSec % 60;
  unsigned long centis = (ms % 1000) / 10;

  char buf[16];
  snprintf(buf, sizeof(buf), "%02lu:%02lu.%02lu", minutes, seconds, centis);
  return String(buf);
}

// ===================== PERSISTENCE (SPIFFS) =====================

// -- start by rizki
unsigned long long parseBibNumber(const char* bib) {
  if (!bib || !*bib) return 0;
  if (*bib == '#') bib++;
  if (!*bib) return 0;

  char* end = nullptr;
  unsigned long long value = strtoull(bib, &end, 10);
  return end != bib && *end == '\0' ? value : 0;
}

bool saveNextRiderBib() {
  JsonDocument doc;
  doc["nextBib"] = nextRiderBib;

  File f = SPIFFS.open("/rider-sequence.tmp", "w");
  if (!f) {
    Serial.println(F("[ERROR] Could not open temporary rider sequence file"));
    return false;
  }

  size_t expectedBytes = measureJson(doc);
  size_t writtenBytes = serializeJson(doc, f);
  f.flush();
  f.close();
  if (writtenBytes != expectedBytes) {
    SPIFFS.remove("/rider-sequence.tmp");
    Serial.println(F("[ERROR] Incomplete write while saving rider sequence"));
    return false;
  }

  if (!SPIFFS.rename("/rider-sequence.tmp", "/rider-sequence.json")) {
    SPIFFS.remove("/rider-sequence.tmp");
    Serial.println(F("[ERROR] Could not replace persisted rider sequence"));
    return false;
  }
  return true;
}

bool saveRiders() {
  JsonDocument doc;
  JsonArray arr = doc.to<JsonArray>();
  for (int i = 0; i < riderCount; i++) {
    JsonObject obj = arr.add<JsonObject>();
    obj["bib"] = riders[i].bib;
    obj["name"] = riders[i].name;
    obj["team"] = riders[i].team;
  }

  File f = SPIFFS.open("/riders.tmp", "w");
  if (!f) {
    Serial.println(F("[ERROR] Could not open temporary rider roster file"));
    return false;
  }

  size_t expectedBytes = measureJson(doc);
  size_t writtenBytes = serializeJson(doc, f);
  f.flush();
  f.close();
  if (writtenBytes != expectedBytes) {
    SPIFFS.remove("/riders.tmp");
    Serial.println(F("[ERROR] Incomplete write while saving rider roster"));
    return false;
  }

  if (!SPIFFS.rename("/riders.tmp", "/riders.json")) {
    SPIFFS.remove("/riders.tmp");
    Serial.println(F("[ERROR] Could not replace persisted rider roster"));
    return false;
  }
  Serial.println("[DATA] Saved " + String(riderCount) + " registered riders to SPIFFS");
  return true;
}

void loadRiders() {
  riderCount = 0;
  nextRiderBib = 101;
  bool sequenceLoaded = false;
  bool sequenceNeedsSave = false;

  if (SPIFFS.exists("/rider-sequence.json")) {
    File sequenceFile = SPIFFS.open("/rider-sequence.json", "r");
    if (sequenceFile) {
      JsonDocument sequenceDoc;
      DeserializationError sequenceError = deserializeJson(sequenceDoc, sequenceFile);
      sequenceFile.close();
      if (!sequenceError && sequenceDoc["nextBib"].is<unsigned long long>()) {
        nextRiderBib = sequenceDoc["nextBib"].as<unsigned long long>();
        sequenceLoaded = true;
      } else {
        Serial.println(F("[ERROR] Invalid persisted rider sequence; deriving from known bibs"));
        sequenceNeedsSave = true;
      }
    } else {
      Serial.println(F("[ERROR] Could not open persisted rider sequence; deriving from known bibs"));
      sequenceNeedsSave = true;
    }
  } else {
    sequenceNeedsSave = true;
  }

  if (SPIFFS.exists("/riders.json")) {
    File f = SPIFFS.open("/riders.json", "r");
    if (!f) {
      Serial.println(F("[ERROR] Could not open persisted rider roster"));
    } else {
      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, f);
      f.close();
      if (error || !doc.is<JsonArray>()) {
        Serial.println(F("[ERROR] Invalid persisted rider roster JSON"));
      } else {
        JsonArray arr = doc.as<JsonArray>();
        for (JsonObject obj : arr) {
          if (riderCount >= MAX_RIDERS) break;
          const char* bib = obj["bib"] | "";
          const char* name = obj["name"] | "";
          if (strlen(name) == 0) continue;

          strlcpy(riders[riderCount].bib, bib, sizeof(riders[0].bib));
          strlcpy(riders[riderCount].name, name, sizeof(riders[0].name));
          strlcpy(riders[riderCount].team, obj["team"] | "Men Elite Downhill",
                  sizeof(riders[0].team));
          riderCount++;
        }
      }
    }
  } else {
    Serial.println(F("[DATA] No registered rider roster found"));
  }

  unsigned long long highestBib = 100;
  for (int i = 0; i < riderCount; i++) {
    unsigned long long bibNumber = parseBibNumber(riders[i].bib);
    if (bibNumber > highestBib) highestBib = bibNumber;
  }
  for (int i = 0; i < runCount; i++) {
    unsigned long long bibNumber = parseBibNumber(runs[i].bib);
    if (bibNumber > highestBib) highestBib = bibNumber;
  }

  unsigned long long minimumNextBib =
    highestBib >= MAX_RIDER_BIB ? MAX_RIDER_BIB + 1 : highestBib + 1;
  if (!sequenceLoaded || nextRiderBib < minimumNextBib) {
    nextRiderBib = minimumNextBib;
    sequenceNeedsSave = true;
  }
  if (sequenceNeedsSave && !saveNextRiderBib()) {
    Serial.println(F("[ERROR] Could not persist recovered next rider bib"));
  }

  Serial.println("[DATA] Loaded " + String(riderCount) + " registered riders from SPIFFS");
  Serial.println("[DATA] Next unused rider bib: " + String((unsigned long long)nextRiderBib));
}
// -- end by rizki

// -- start by rizki
bool saveRuns() {
  JsonDocument doc;
  JsonArray arr = doc.to<JsonArray>();
  for (int i = 0; i < runCount; i++) {
    JsonObject obj = arr.add<JsonObject>();
    obj["bib"] = runs[i].bib;
    obj["name"] = runs[i].name;
    obj["team"] = runs[i].team;
    obj["start"] = runs[i].startTimeMs;
    obj["finish"] = runs[i].finishTimeMs;
    obj["elapsed"] = runs[i].elapsedMs;
    obj["startTimeStr"] = runs[i].startTimeStr;
    obj["date"] = runs[i].date;
    obj["completed"] = runs[i].completed;
  }

  File f = SPIFFS.open("/runs.tmp", "w");
  if (!f) {
    Serial.println(F("[ERROR] Could not open temporary run history file"));
    return false;
  }
  const size_t expectedBytes = measureJson(doc);
  const size_t writtenBytes = serializeJson(doc, f);
  f.flush();
  f.close();
  if (writtenBytes != expectedBytes) {
    SPIFFS.remove("/runs.tmp");
    Serial.println(F("[ERROR] Incomplete write while saving run history"));
    return false;
  }
  bool hadPreviousRuns = SPIFFS.exists("/runs.json");
  if (SPIFFS.exists("/runs.bak") && !SPIFFS.remove("/runs.bak")) {
    SPIFFS.remove("/runs.tmp");
    Serial.println(F("[ERROR] Could not clear old run history backup"));
    return false;
  }
  if (hadPreviousRuns && !SPIFFS.rename("/runs.json", "/runs.bak")) {
    SPIFFS.remove("/runs.tmp");
    Serial.println(F("[ERROR] Could not back up persisted run history"));
    return false;
  }
  if (!SPIFFS.rename("/runs.tmp", "/runs.json")) {
    SPIFFS.remove("/runs.tmp");
    if (hadPreviousRuns) SPIFFS.rename("/runs.bak", "/runs.json");
    Serial.println(F("[ERROR] Could not replace persisted run history"));
    return false;
  }
  if (hadPreviousRuns) SPIFFS.remove("/runs.bak");
  return true;
}

void saveData() {
  if (!saveRuns()) return;
  JsonDocument eventDoc;
  eventDoc["name"] = eventName;
  File f = SPIFFS.open("/event.json", "w");
  if (!f) {
    Serial.println(F("[ERROR] Could not open event metadata file"));
    return;
  }
  const size_t expectedEventBytes = measureJson(eventDoc);
  const size_t writtenEventBytes = serializeJson(eventDoc, f);
  f.flush();
  f.close();
  if (writtenEventBytes != expectedEventBytes) {
    Serial.println(F("[ERROR] Incomplete write while saving event metadata"));
    return;
  }

  Serial.println(F("[DATA] Saved runs and event to SPIFFS"));
}
// -- end by rizki

void loadData() {
  if (SPIFFS.exists("/event.json")) {
    File f = SPIFFS.open("/event.json", "r");
    if (f) {
      JsonDocument doc;
      deserializeJson(doc, f);
      strlcpy(eventName, doc["name"] | "Bicycle Race Championship", sizeof(eventName));
      f.close();
    }
  }

  // -- start by rizki
  if (!SPIFFS.exists("/runs.json") && SPIFFS.exists("/runs.bak")) {
    if (!SPIFFS.rename("/runs.bak", "/runs.json")) {
      Serial.println(F("[ERROR] Could not recover run history backup"));
    }
  } else if (SPIFFS.exists("/runs.json") && SPIFFS.exists("/runs.bak")) {
    SPIFFS.remove("/runs.bak");
  }
  // -- end by rizki
  if (SPIFFS.exists("/runs.json")) {
    File f = SPIFFS.open("/runs.json", "r");
    if (f) {
      JsonDocument doc;
      deserializeJson(doc, f);
      JsonArray arr = doc.as<JsonArray>();
      runCount = 0;
      for (JsonObject obj : arr) {
        if (runCount >= MAX_RUNS) break;
        strlcpy(runs[runCount].bib, obj["bib"] | "", sizeof(runs[0].bib));
        strlcpy(runs[runCount].name, obj["name"] | "", sizeof(runs[0].name));
        strlcpy(runs[runCount].team, obj["team"] | "", sizeof(runs[0].team));
        runs[runCount].startTimeMs = obj["start"] | 0;
        runs[runCount].finishTimeMs = obj["finish"] | 0;
        runs[runCount].elapsedMs = obj["elapsed"] | 0;
        strlcpy(runs[runCount].startTimeStr, obj["startTimeStr"] | "", sizeof(runs[0].startTimeStr));
        strlcpy(runs[runCount].date, obj["date"] | "", sizeof(runs[0].date));
        // -- start by rizki
        runs[runCount].completed = obj["completed"] | true;
        // -- end by rizki
        runCount++;
      }
      f.close();
      // -- start by rizki
      if (runCount > 0) {
        lastRunData = runs[runCount - 1];
        hasLastRun = true;
      }
      // -- end by rizki
      Serial.println("[DATA] Loaded " + String(runCount) + " runs from SPIFFS");
    }
  }
}
