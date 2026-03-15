/*
 * Smart IoT-Based Waste Classification and Monitoring System
 * ThingSpeak Cloud Integration
 * Hardware: NodeMCU ESP8266, IR Sensor, Moisture Sensor, Servo Motor, LCD
 *
 * FINAL VERSION:
 *  1. Moisture CORRECTED (LOW = WET, HIGH = DRY)
 *  2. DRY  → LEFT  (160°) fast
 *  3. WET  → RIGHT  (20°) fast
 *  4. CENTER → 90° detach (no jitter)
 *  5. Instant servo — direct write (no loop)
 *  6. Continuous ThingSpeak upload every 20s
 */
 
#include <ESP8266WiFi.h>
#include <ThingSpeak.h>
#include <Servo.h>
#include <LiquidCrystal.h>
 
// ========== WIFI ==========
#define WIFI_SSID "TgP"
#define WIFI_PASSWORD "pooja2006"
 
// ========== THINGSPEAK ==========
#define THINGSPEAK_CHANNEL_ID    3282129
#define THINGSPEAK_WRITE_API_KEY "03MKMJZ3MCRJ5MX4"
 
// ========== PINS ==========
const int IR_PIN       = 4;    // D2 (GPIO4)
const int MOISTURE_PIN = A0;   // A0
const int SERVO_PIN    = 5;    // D1 (GPIO5)
const int BUZZER_PIN   = 16;   // D0 (GPIO16)
const int LED_STATUS   = 2;    // D4 (GPIO2)
 
// LCD: RS, E, D4, D5, D6, D7
LiquidCrystal lcd(13, 12, 14, 2, 0, 15);
 
Servo wasteServo;
 
// ========== SERVO ANGLES ==========
const int SERVO_WET    = 20;   // WET  bin — RIGHT side
const int SERVO_DRY    = 160;  // DRY  bin — LEFT  side
const int SERVO_CENTER = 90;   // NEUTRAL  — detaches here
 
// ========== MOISTURE CALIBRATION ==========
// DRY material → HIGH value (700-900)
// WET material → LOW  value (200-400)
const int DRY_VALUE          = 800;
const int WET_VALUE          = 300;
const int MOISTURE_THRESHOLD = (DRY_VALUE + WET_VALUE) / 2;  // = 550
 
// ========== SYSTEM VARIABLES ==========
int wetCount  = 0;
int dryCount  = 0;
unsigned long lastUpload = 0;
const unsigned long UPLOAD_INTERVAL = 20000;  // 20 seconds
 
WiFiClient client;
 
// ========== FUNCTION DECLARATIONS ==========
void connectWiFi();
void beep(int duration);
int  readMoistureAverage(int samples);
void updateThingSpeak(int moisture, int wasteType, int wet, int dry);
void showLCD(String line1, String line2);
void moveServo(int targetAngle);
 
// ============================================================
//  SETUP
// ============================================================
void setup() {
  Serial.begin(115200);
  delay(1000);
 
  Serial.println("\n=== Smart Waste Segregation System ===");
  Serial.println("WET  angle : " + String(SERVO_WET)    + "° (RIGHT)");
  Serial.println("DRY  angle : " + String(SERVO_DRY)    + "° (LEFT)");
  Serial.println("CENTER     : " + String(SERVO_CENTER) + "°");
  Serial.println("Threshold  : " + String(MOISTURE_THRESHOLD));
 
  pinMode(IR_PIN,     INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_STATUS, OUTPUT);
  digitalWrite(LED_STATUS, LOW);
 
  // Servo NOT attached at startup — prevents random rotation
  lcd.begin(16, 2);
  showLCD("Waste System", "Initializing...");
 
  connectWiFi();
 
  ThingSpeak.begin(client);
  Serial.println("ThingSpeak initialized");
 
  showLCD("System Ready", "Insert Waste");
  Serial.println("=== System Ready ===");
  beep(200);
  digitalWrite(LED_STATUS, HIGH);
}
 
// ============================================================
//  MAIN LOOP
// ============================================================
void loop() {
 
  // WiFi watchdog
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi lost! Reconnecting...");
    connectWiFi();
  }
 
  // Continuous upload every 20s — no chart gap
  unsigned long currentTime = millis();
  if (currentTime - lastUpload >= UPLOAD_INTERVAL) {
    int liveMoisture = readMoistureAverage(5);
    Serial.println("\n--- Periodic Upload ---");
    updateThingSpeak(liveMoisture, 0, wetCount, dryCount);
    lastUpload = currentTime;
  }
 
  // Stage 1: IR sensor — waste detection
  if (digitalRead(IR_PIN) == LOW) {
    delay(100);
    if (digitalRead(IR_PIN) == LOW) {
 
      Serial.println("\n--- Waste Detected ---");
      beep(150);
      showLCD("Waste Detected", "Analyzing...");
 
      // Stage 2: Read moisture
      int moistureValue = readMoistureAverage(15);
      Serial.println("Moisture  : " + String(moistureValue));
      Serial.println("Threshold : " + String(MOISTURE_THRESHOLD));
 
      // Stage 3: Classify
      int    wasteType;
      String wasteTypeStr;
 
      if (moistureValue < MOISTURE_THRESHOLD) {
 
        // WET WASTE → RIGHT (20°)
        wasteType    = 1;
        wasteTypeStr = "WET";
        wetCount++;
        Serial.println("Classification: WET WASTE → RIGHT");
 
        showLCD("WET Waste", "Detected!");
        delay(500);
        showLCD("Routing to", "Wet Bin...");
        moveServo(SERVO_WET);    // 20° → RIGHT instantly
        delay(2000);
 
      } else {
 
        // DRY WASTE → LEFT (160°)
        wasteType    = 0;
        wasteTypeStr = "DRY";
        dryCount++;
        Serial.println("Classification: DRY WASTE → LEFT");
 
        showLCD("DRY Waste", "Detected!");
        delay(500);
        showLCD("Routing to", "Dry Bin...");
        moveServo(SERVO_DRY);    // 160° → LEFT instantly
        delay(2000);
      }
 
      Serial.println("Wet   : " + String(wetCount));
      Serial.println("Dry   : " + String(dryCount));
      Serial.println("Total : " + String(wetCount + dryCount));
 
      // Stage 5: Return to center & detach
      showLCD("Returning...", "Please Wait");
      moveServo(SERVO_CENTER);   // 90° — detaches automatically
 
      // Stage 6: Upload
      Serial.println("\n--- Uploading after detection ---");
      updateThingSpeak(moistureValue, wasteType, wetCount, dryCount);
      lastUpload = millis();
 
      showLCD("System Ready",
              "W:" + String(wetCount) + " D:" + String(dryCount));
 
      delay(2000);  // cooldown
    }
  }
 
  // Blink LED = system alive
  static unsigned long lastBlink = 0;
  if (millis() - lastBlink > 1000) {
    digitalWrite(LED_STATUS, !digitalRead(LED_STATUS));
    lastBlink = millis();
  }
 
  delay(50);
}
 
// ============================================================
//  MOVE SERVO — Instant direct write
//  No loop, no sweep — jumps directly to angle
//  Fast both ways, same speed ✅
// ============================================================
void moveServo(int targetAngle) {
 
  wasteServo.attach(SERVO_PIN);
  delay(100);                      // settle after attach
 
  wasteServo.write(targetAngle);   // instant jump to position
  delay(700);                      // wait for servo to physically reach
 
  // Detach at center — no jitter, no random trigger when idle
  if (targetAngle == SERVO_CENTER) {
    delay(200);
    wasteServo.detach();
    Serial.println("Servo DETACHED — idle (no jitter)");
  }
 
  Serial.println("Servo at: " + String(targetAngle) + "°");
}
 
// ============================================================
//  WIFI
// ============================================================
void connectWiFi() {
  Serial.print("Connecting to: ");
  Serial.println(WIFI_SSID);
  showLCD("Connecting WiFi", "...");
 
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
 
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
 
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected! IP: " + WiFi.localIP().toString());
    Serial.println("RSSI: " + String(WiFi.RSSI()) + " dBm");
    showLCD("WiFi Connected", WiFi.localIP().toString());
    delay(2000);
    digitalWrite(LED_STATUS, HIGH);
  } else {
    Serial.println("\nWiFi FAILED!");
    showLCD("WiFi Failed!", "Check Settings");
    digitalWrite(LED_STATUS, LOW);
  }
}
 
// ============================================================
//  THINGSPEAK UPLOAD
// ============================================================
void updateThingSpeak(int moisture, int wasteType, int wet, int dry) {
  ThingSpeak.setField(1, moisture);
  ThingSpeak.setField(2, wasteType);
  ThingSpeak.setField(3, wet);
  ThingSpeak.setField(4, dry);
  ThingSpeak.setField(5, wet + dry);
 
  int statusCode = ThingSpeak.writeFields(
                     THINGSPEAK_CHANNEL_ID, THINGSPEAK_WRITE_API_KEY);
 
  if (statusCode == 200) {
    Serial.println("Upload SUCCESS");
    Serial.println("  Moisture : " + String(moisture));
    Serial.println("  Type     : " + String(wasteType == 1 ? "WET" : "DRY"));
    Serial.println("  Wet      : " + String(wet));
    Serial.println("  Dry      : " + String(dry));
    Serial.println("  Total    : " + String(wet + dry));
    beep(100);
  } else {
    Serial.println("Upload FAILED. Code: " + String(statusCode));
    beep(500);
  }
}
 
// ============================================================
//  MOISTURE AVERAGE
// ============================================================
int readMoistureAverage(int samples) {
  long total = 0;
  for (int i = 0; i < samples; i++) {
    total += analogRead(MOISTURE_PIN);
    delay(10);
  }
  int avg = total / samples;
  Serial.println("Moisture Avg: " + String(avg));
  return avg;
}
 
// ============================================================
//  BUZZER
// ============================================================
void beep(int duration) {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(duration);
  digitalWrite(BUZZER_PIN, LOW);
}
 
// ============================================================
//  LCD
// ============================================================
void showLCD(String line1, String line2) {
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(line1);
  lcd.setCursor(0, 1); lcd.print(line2);
}
 
// ========== END ==========