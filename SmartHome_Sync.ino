#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include <SoftwareSerial.h>

// --- SCREEN DEFINITIONS ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- PIN DEFINITIONS ---
#define PIR_PIN       2    // Hardware Interrupt INT0
#define DHT_PIN       3    // DHT11 Sensor Data
#define LED_LIGHT     7    // LED 1 (Room Light)
#define LED_FAN       8    // LED 2 (DC Fan Simulator)
#define BT_RX         10   // Software Serial RX (To Bluetooth TX)
#define BT_TX         11   // Software Serial TX (To Bluetooth RX)

#define DHTTYPE       DHT11
#define TEMP_THRESHOLD 30.0 // Turn Fan LED ON if temp exceeds 30°C

// --- OBJECT INITIALIZATION ---
DHT dht(DHT_PIN, DHTTYPE);
SoftwareSerial btSerial(BT_RX, BT_TX);

// --- OS SYNCHRONIZATION VARIABLES ---
volatile bool motionDetected = false; // Flag updated inside ISR
bool displayMutex = false;            // Mutex flag to protect OLED resource

// Timers for Non-Blocking Scheduler
unsigned long lastDHTRead = 0;
unsigned long lastOLEDUpdate = 0;
const unsigned long DHT_INTERVAL = 2000;  // Read temperature every 2 seconds
const unsigned long OLED_INTERVAL = 1000; // Refresh display every 1 second

// Peripheral States
bool lightState = false;
bool fanState = false;
float currentTemp = 0.0;
float currentHumidity = 0.0;

// --- HARDWARE INTERRUPT SERVICE ROUTINE (ISR) ---
void motionISR() {
  motionDetected = true;
}

// --- MUTEX-PROTECTED OLED DISPLAY FUNCTION ---
void safeUpdateOLED(String header, String statusText) {
  if (!displayMutex) {
    displayMutex = true; // LOCK DISPLAY
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    
    // Header
    display.setCursor(0, 0);
    display.println("Smart Home OS (Sync)");
    display.println("---------------------");
    
    // Content
    display.setCursor(0, 20);
    display.println(header);
    display.setCursor(0, 38);
    display.println(statusText);
    
    display.display();
    
    displayMutex = false; // UNLOCK DISPLAY
  }
}

void setup() {
  Serial.begin(9600);
  btSerial.begin(9600);

  pinMode(PIR_PIN, INPUT);
  pinMode(LED_LIGHT, OUTPUT);
  pinMode(LED_FAN, OUTPUT);

  // Initialize Outputs OFF
  digitalWrite(LED_LIGHT, LOW);
  digitalWrite(LED_FAN, LOW);

  dht.begin();

  // Initialize OLED (Address 0x3C is standard for 0.96" displays)
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    for(;;); // Loop forever if OLED initialization fails
  }

  // Attach Hardware Interrupt to Pin 2 on RISING edge
  attachInterrupt(digitalPinToInterrupt(PIR_PIN), motionISR, RISING);

  safeUpdateOLED("System Ready", "Waiting for events...");
  delay(1000);
}

void loop() {
  unsigned long currentMillis = millis();

  // --- TASK 1: PROCESS MOTION INTERRUPT (High Priority Event) ---
  if (motionDetected) {
    lightState = true;
    digitalWrite(LED_LIGHT, HIGH); // Turn Light LED ON
    motionDetected = false;        // Clear ISR flag
    
    safeUpdateOLED("EVENT: Motion Detected!", "Light LED: ON");
  }

  // --- TASK 2: PERIODIC DHT11 POLLING (Scheduled Task) ---
  if (currentMillis - lastDHTRead >= DHT_INTERVAL) {
    lastDHTRead = currentMillis;

    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (!isnan(t) && !isnan(h)) {
      currentTemp = t;
      currentHumidity = h;

      // Automated Control Logic
      if (currentTemp >= TEMP_THRESHOLD && !fanState) {
        fanState = true;
        digitalWrite(LED_FAN, HIGH); // Turn Fan LED ON
      } else if (currentTemp < TEMP_THRESHOLD && fanState) {
        fanState = false;
        digitalWrite(LED_FAN, LOW); // Turn Fan LED OFF
      }
    }
  }

  // --- TASK 3: BLUETOOTH OVERRIDE (Asynchronous Input) ---
  if (btSerial.available()) {
    char cmd = btSerial.read();
    if (cmd == '1') {
      lightState = true;
      digitalWrite(LED_LIGHT, HIGH);
    } else if (cmd == '0') {
      lightState = false;
      digitalWrite(LED_LIGHT, LOW);
    } else if (cmd == 'F') {
      fanState = true;
      digitalWrite(LED_FAN, HIGH);
    } else if (cmd == 'f') {
      fanState = false;
      digitalWrite(LED_FAN, LOW);
    }
  }

  // --- TASK 4: PERIODIC DISPLAY REFRESH (Shared Resource Update) ---
  if (currentMillis - lastOLEDUpdate >= OLED_INTERVAL) {
    lastOLEDUpdate = currentMillis;

    String line1 = "Temp:" + String((int)currentTemp) + "C  Hum:" + String((int)currentHumidity) + "%";
    String line2 = "Light:" + String(lightState ? "ON " : "OFF") + " Fan:" + String(fanState ? "ON" : "OFF");

    safeUpdateOLED(line1, line2);
  }
}