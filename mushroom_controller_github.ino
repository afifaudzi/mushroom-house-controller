#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include "config.h"  // Contains sensitive credentials (NOT in GitHub)

// === Relay & LED Pin Mapping ===
#define RELAY_LED_R 15
#define RELAY_LED_G 16
#define RELAY_PUMP 17
#define RELAY_FAN 18

#define SWITCH_RED 9
#define SWITCH_GREEN 10

// Relay logic (active LOW modules for pump/fan)
#define RELAY_ACTIVE LOW    // LOW signal activates the component (Turns it ON)
#define RELAY_INACTIVE HIGH   // HIGH signal deactivates the component (Turns it OFF)

// --- Debounce ---
const unsigned long DEBOUNCE_DELAY = 50; 
unsigned long lastGreenButtonTime = 0;
unsigned long lastRedButtonTime = 0;
int lastGreenButtonState = HIGH;
int lastRedButtonState = LOW;

// --- Mode Toggle ---
const unsigned long LONG_PRESS_TIME = 3000; // 3 seconds for local mode toggle
unsigned long greenPressStartTime = 0;
bool modeToggleHandled = false;

// --- Non-blocking delay handling ---
bool fanPending = false;
unsigned long fanDelayStart = 0;
const unsigned long FAN_DELAY = 2000;  // 2 sec stagger

// --- Time Window for Auto Mode (configurable from dashboard) ---
int autoModeStartHour = DEFAULT_AUTO_START_HOUR;
int autoModeStartMinute = 0;
int autoModeEndHour = DEFAULT_AUTO_END_HOUR;
int autoModeEndMinute = 0;

// --- Temperature Threshold (configurable from dashboard) ---
float tempThresholdHigh = DEFAULT_TEMP_HIGH;
float tempThresholdLow = DEFAULT_TEMP_LOW;

// --- MQTT Topics ---
const char* MQTT_TELEMETRY_TOPIC = "v1/devices/me/telemetry";
const char* MQTT_RPC_REQUEST_TOPIC = "v1/devices/me/rpc/request/+";
const char* MQTT_ATTRIBUTES_TOPIC = "v1/devices/me/attributes";

WiFiClient espClient;
PubSubClient mqttClient(espClient);

// --- Telemetry ---
const unsigned long PUBLISH_INTERVAL = 30 * 1000;
unsigned long lastPublishTime = 0;

// --- System State ---
bool pumpState = false;
bool fanState = false;
bool autoMode = false;
float temperature = 0;
float humidity = 0;
float co2 = 0;

// === Forward Declarations ===
void reconnectMqtt();
void mqttCallback(char* topic, byte* payload, unsigned int length);
void setActuatorState(const char* actuator, bool state, bool fromLocal = false);
void publishState(bool force = false);
bool isWithinAutoModeTimeWindow();
void printLocalTime();

// === Setup ===
void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 Mushroom House Controller v2.0");

  // Pin setup
  pinMode(RELAY_PUMP, OUTPUT);
  pinMode(RELAY_FAN, OUTPUT);
  pinMode(RELAY_LED_G, OUTPUT);
  pinMode(RELAY_LED_R, OUTPUT);
  pinMode(SWITCH_GREEN, INPUT_PULLUP);
  pinMode(SWITCH_RED, INPUT_PULLUP);

  // --- INITIALIZATION ---
  Serial.println("Initializing: Pump ON, Fan ON, Green LED ON...");
  setActuatorState("pump", true, true);
  setActuatorState("fan", true, true);

  // --- WiFi Setup ---
  WiFiManager wifiManager;
  bool connected = false;

  WiFi.begin();
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 15000) {
    delay(500);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    connected = true;
    Serial.println("\nConnected with saved WiFiManager credentials.");
  }

  if (!connected) {
    Serial.println("No saved network, starting WiFiManager portal...");
    if (!wifiManager.autoConnect("MushroomNodeAP-H1", "mushroom_pw")) {
      Serial.println("Failed to connect. Restarting...");
      delay(3000);
      ESP.restart();
    }
  }

  Serial.println("WiFi connected, IP: " + WiFi.localIP().toString());

  // --- NTP Time Synchronization ---
  Serial.println("Synchronizing time with NTP server...");
  configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);
  
  int timeoutCounter = 0;
  while (time(nullptr) < 100000 && timeoutCounter < 20) {
    delay(500);
    Serial.print(".");
    timeoutCounter++;
  }
  Serial.println();
  
  if (time(nullptr) > 100000) {
    Serial.println("Time synchronized successfully!");
    printLocalTime();
  } else {
    Serial.println("Failed to sync time, continuing anyway...");
  }

  // MQTT setup
  mqttClient.setServer(THINGSBOARD_SERVER, THINGSBOARD_PORT);
  mqttClient.setCallback(mqttCallback);
}

// === Loop ===
void loop() {
  unsigned long currentMillis = millis();

  if (WiFi.status() == WL_CONNECTED) {
    reconnectMqtt();
    mqttClient.loop();
  }

  int greenState = digitalRead(SWITCH_GREEN);
  int redState   = digitalRead(SWITCH_RED);

  // --- 1. Local Mode Toggle ---
  if (greenState == LOW) {
    if (greenPressStartTime == 0) {
        greenPressStartTime = currentMillis;
        modeToggleHandled = false;
    } else if (!modeToggleHandled && (currentMillis - greenPressStartTime) >= LONG_PRESS_TIME) {
        autoMode = !autoMode;
        Serial.println(String("Mode toggled locally to ") + (autoMode ? "AUTO" : "MANUAL"));
        publishState(true);
        
        if (!autoMode) {
            setActuatorState("pump", false, true);
            setActuatorState("fan", false, true);
        }
        modeToggleHandled = true;
    }
  } else {
    greenPressStartTime = 0;
    modeToggleHandled = false;
  }
  
  int previousGreenButtonState = lastGreenButtonState;
  lastGreenButtonState = greenState;
  
  // --- 2. AUTO MODE with Time Window ---
  if (autoMode) {
      if (WiFi.status() == WL_CONNECTED && mqttClient.connected()) {
          if (isWithinAutoModeTimeWindow()) {
              if (temperature > tempThresholdHigh) {
                setActuatorState("pump", true);
                fanPending = true;
                fanDelayStart = millis();
              } else if (temperature < tempThresholdLow) {
                setActuatorState("pump", false);
                setActuatorState("fan", false);
              }
          } else {
              if (pumpState || fanState) {
                Serial.println("Outside auto mode time window - turning OFF");
                setActuatorState("pump", false, true);
                setActuatorState("fan", false, true);
              }
          }
      } else {
          if (pumpState || fanState) {
            // Keep current state
          } else {
            setActuatorState("pump", false, true);
            setActuatorState("fan", false, true);
          }
          autoMode = false;
          Serial.println("⚠ Internet/MQTT lost → switching to MANUAL fallback mode");
          publishState(true);
      }
  } else {
    // --- MANUAL MODE ---
    if (greenState == LOW && previousGreenButtonState == HIGH && 
        (currentMillis - lastGreenButtonTime) > DEBOUNCE_DELAY && !modeToggleHandled) {
      lastGreenButtonTime = currentMillis;
      Serial.println("Green switch pressed → Local ON sequence");
      setActuatorState("pump", true, true);
      fanPending = true;
      fanDelayStart = millis();
      publishState(true);
    }

    if (redState != lastRedButtonState && (currentMillis - lastRedButtonTime) > DEBOUNCE_DELAY) {
      lastRedButtonTime = currentMillis;
      if (redState == LOW) {
        Serial.println("Red switch pressed → Local OFF");
        setActuatorState("pump", false, true);
        setActuatorState("fan", false, true);
        publishState(true);
      }
    }
    lastRedButtonState = redState;
  }

  // --- Non-blocking Fan Stagger ---
  if (fanPending && millis() - fanDelayStart >= FAN_DELAY) {
      setActuatorState("fan", true, true);
      fanPending = false;
  }

  // Regular telemetry
  if (WiFi.status() == WL_CONNECTED && mqttClient.connected() &&
      currentMillis - lastPublishTime >= PUBLISH_INTERVAL) {
    lastPublishTime = currentMillis;
    publishState();
  }
}

// === Check Time Window ===
bool isWithinAutoModeTimeWindow() {
  time_t now = time(nullptr);
  struct tm timeinfo;
  if (!localtime_r(&now, &timeinfo)) {
    return true;
  }

  int currentHour = timeinfo.tm_hour;
  int currentMinute = timeinfo.tm_min;
  
  int currentTimeMinutes = currentHour * 60 + currentMinute;
  int startTimeMinutes = autoModeStartHour * 60 + autoModeStartMinute;
  int endTimeMinutes = autoModeEndHour * 60 + autoModeEndMinute;
  
  return (currentTimeMinutes >= startTimeMinutes && currentTimeMinutes < endTimeMinutes);
}

// === Print Local Time ===
void printLocalTime() {
  time_t now = time(nullptr);
  struct tm timeinfo;
  if (!localtime_r(&now, &timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.print("Current time: ");
  Serial.printf("%04d-%02d-%02d %02d:%02d:%02d\n",
                timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
                timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
}

// === Actuator Control ===
void setActuatorState(const char* actuator, bool state, bool fromLocal) {
  if (strcmp(actuator, "pump") == 0) {
    if (pumpState != state) {
      digitalWrite(RELAY_PUMP, state ? RELAY_ACTIVE : RELAY_INACTIVE);
      pumpState = state;
      Serial.println(String("Pump → ") + (state ? "ON" : "OFF") + (fromLocal ? " (local)" : " (remote)"));
    }
  } else if (strcmp(actuator, "fan") == 0) {
    if (fanState != state) {
      digitalWrite(RELAY_FAN, state ? RELAY_ACTIVE : RELAY_INACTIVE);
      fanState = state;
      Serial.println(String("Fan → ") + (state ? "ON" : "OFF") + (fromLocal ? " (local)" : " (remote)"));
    }
  }

  if (pumpState || fanState) {
    digitalWrite(RELAY_LED_G, HIGH);
    digitalWrite(RELAY_LED_R, LOW);
  } else {
    digitalWrite(RELAY_LED_G, LOW);
    digitalWrite(RELAY_LED_R, HIGH);
  }
}

// === Publish Telemetry ===
void publishState(bool force) {
  if (WiFi.status() != WL_CONNECTED || !mqttClient.connected()) {
    if (force) Serial.println("⚠ Skipped telemetry: no MQTT");
    return;
  }

  time_t now = time(nullptr);
  struct tm timeinfo;
  bool timeValid = localtime_r(&now, &timeinfo);

  StaticJsonDocument<512> doc;
  doc["pump"] = pumpState;
  doc["fan"]  = fanState;
  doc["mode"] = autoMode ? "auto" : "manual";
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;
  doc["co2"] = co2;
  
  doc["autoStartHour"] = autoModeStartHour;
  doc["autoStartMinute"] = autoModeStartMinute;
  doc["autoEndHour"] = autoModeEndHour;
  doc["autoEndMinute"] = autoModeEndMinute;
  
  doc["tempThresholdHigh"] = tempThresholdHigh;
  doc["tempThresholdLow"] = tempThresholdLow;
  
  if (timeValid) {
    doc["currentHour"] = timeinfo.tm_hour;
    doc["currentMinute"] = timeinfo.tm_min;
    doc["inTimeWindow"] = isWithinAutoModeTimeWindow();
  }

  doc["ledGreen"] = digitalRead(RELAY_LED_G);
  doc["ledRed"]   = digitalRead(RELAY_LED_R);
  doc["switchGreen"] = (digitalRead(SWITCH_GREEN) == LOW);
  doc["switchRed"]   = (digitalRead(SWITCH_RED) == LOW);

  char buffer[512];
  serializeJson(doc, buffer);
  mqttClient.publish(MQTT_TELEMETRY_TOPIC, buffer);
  Serial.println("Telemetry → " + String(buffer));
}

// === MQTT Reconnect ===
void reconnectMqtt() {
  if (WiFi.status() != WL_CONNECTED || mqttClient.connected()) return;

  Serial.print("Connecting to ThingsBoard...");
  if (mqttClient.connect("Esp32-RC1", TOKEN, NULL)) { 
    Serial.println("connected");
    mqttClient.subscribe(MQTT_RPC_REQUEST_TOPIC);
    mqttClient.subscribe(MQTT_ATTRIBUTES_TOPIC);
    publishState(true);
  } else {
    Serial.print("failed, rc=");
    Serial.println(mqttClient.state());
  }
}

// === MQTT Callback ===
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (int i = 0; i < length; i++) msg += (char)payload[i];
  Serial.println("Incoming: " + String(topic) + " - " + msg);

  DynamicJsonDocument doc(512);
  DeserializationError error = deserializeJson(doc, msg);
  if (error) {
    Serial.print("JSON parse failed: ");
    Serial.println(error.c_str());
    return;
  }

  String t = String(topic);

  if (t.startsWith("v1/devices/me/rpc/request/")) {
    String method = doc["method"];
    String requestId = t.substring(t.lastIndexOf('/') + 1);
    String responseTopic = "v1/devices/me/rpc/response/" + requestId;
    
    if (method == "getMode") {
      mqttClient.publish(responseTopic.c_str(), autoMode ? "true" : "false");
      Serial.println("Replied getMode: " + String(autoMode ? "true" : "false"));
      
    } else if (method == "setMode") {
      autoMode = doc["params"];
      Serial.println(String("Mode changed to ") + (autoMode ? "AUTO" : "MANUAL"));
      if (!autoMode) {
        setActuatorState("pump", false, false);
        setActuatorState("fan", false, false);
      }
      publishState(true);
      
    } else if (method == "setPumpState" && !autoMode) {
      setActuatorState("pump", doc["params"]);
      publishState(true);
      
    } else if (method == "setFanState" && !autoMode) {
      setActuatorState("fan", doc["params"]);
      publishState(true);
      
    } else if (method == "setAutoTimeWindow") {
      if (doc["params"].containsKey("startHour")) {
        autoModeStartHour = doc["params"]["startHour"];
      }
      if (doc["params"].containsKey("startMinute")) {
        autoModeStartMinute = doc["params"]["startMinute"];
      }
      if (doc["params"].containsKey("endHour")) {
        autoModeEndHour = doc["params"]["endHour"];
      }
      if (doc["params"].containsKey("endMinute")) {
        autoModeEndMinute = doc["params"]["endMinute"];
      }
      Serial.printf("Time window updated: %02d:%02d - %02d:%02d\n",
                    autoModeStartHour, autoModeStartMinute,
                    autoModeEndHour, autoModeEndMinute);
      publishState(true);
      mqttClient.publish(responseTopic.c_str(), "OK");
      
    } else if (method == "setTempThresholds") {
      if (doc["params"].containsKey("high")) {
        tempThresholdHigh = doc["params"]["high"];
      }
      if (doc["params"].containsKey("low")) {
        tempThresholdLow = doc["params"]["low"];
      }
      Serial.printf("Temperature thresholds updated: HIGH=%.1f°C, LOW=%.1f°C\n",
                    tempThresholdHigh, tempThresholdLow);
      publishState(true);
      mqttClient.publish(responseTopic.c_str(), "OK");
      
    } else if (method == "getSettings") {
      StaticJsonDocument<256> response;
      response["autoStartHour"] = autoModeStartHour;
      response["autoStartMinute"] = autoModeStartMinute;
      response["autoEndHour"] = autoModeEndHour;
      response["autoEndMinute"] = autoModeEndMinute;
      response["tempThresholdHigh"] = tempThresholdHigh;
      response["tempThresholdLow"] = tempThresholdLow;
      response["autoMode"] = autoMode;
      response["inTimeWindow"] = isWithinAutoModeTimeWindow();
      
      char responseBuffer[256];
      serializeJson(response, responseBuffer);
      mqttClient.publish(responseTopic.c_str(), responseBuffer);
      Serial.println("Replied getSettings: " + String(responseBuffer));
    }
  }

  if (t == MQTT_ATTRIBUTES_TOPIC) {
    if (doc.containsKey("temperature")) temperature = doc["temperature"];
    if (doc.containsKey("humidity")) humidity = doc["humidity"];
    if (doc.containsKey("co2")) co2 = doc["co2"];
    Serial.printf("Updated attributes → T=%.1f, H=%.1f, CO2=%.1f\n", temperature, humidity, co2);
  }
}
