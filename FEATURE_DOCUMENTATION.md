# Mushroom House Controller v2.0 - New Features Documentation

## 🆕 What's New

### 1. **Time-Based Auto Mode**
- Auto mode now only operates during a specific time window
- **Default: 8:00 AM to 6:00 PM**
- Outside this window, the system turns OFF automatically (even in auto mode)

### 2. **Configurable Temperature Threshold**
- **Default: Turn ON when temperature > 27.5°C**
- Turn OFF when temperature < 26.0°C
- Both thresholds can be adjusted from ThingsBoard

### 3. **NTP Time Synchronization**
- Automatically syncs with internet time servers
- Configured for GMT+8 (Malaysia timezone)

### 4. **Dashboard Control**
- All settings can be changed remotely via ThingsBoard RPC commands

---

## 📊 New Telemetry Data

Your device now sends these additional values every 30 seconds:

```json
{
  "autoStartHour": 8,
  "autoStartMinute": 0,
  "autoEndHour": 18,
  "autoEndMinute": 0,
  "tempThresholdHigh": 27.5,
  "tempThresholdLow": 26.0,
  "currentHour": 14,
  "currentMinute": 30,
  "inTimeWindow": true
}
```

---

## 🎮 ThingsBoard RPC Commands

### 1. **Set Auto Mode Time Window**

**Method:** `setAutoTimeWindow`

**Example Payload:**
```json
{
  "method": "setAutoTimeWindow",
  "params": {
    "startHour": 7,
    "startMinute": 30,
    "endHour": 19,
    "endMinute": 0
  }
}
```

This sets auto mode to operate from **7:30 AM to 7:00 PM**.

---

### 2. **Set Temperature Thresholds**

**Method:** `setTempThresholds`

**Example Payload:**
```json
{
  "method": "setTempThresholds",
  "params": {
    "high": 28.0,
    "low": 25.5
  }
}
```

- `high`: Turn ON when temperature exceeds this
- `low`: Turn OFF when temperature falls below this

---

### 3. **Get Current Settings**

**Method:** `getSettings`

**Example Payload:**
```json
{
  "method": "getSettings",
  "params": {}
}
```

**Response:**
```json
{
  "autoStartHour": 8,
  "autoStartMinute": 0,
  "autoEndHour": 18,
  "autoEndMinute": 0,
  "tempThresholdHigh": 27.5,
  "tempThresholdLow": 26.0,
  "autoMode": true,
  "inTimeWindow": true
}
```

---

### 4. **Existing Commands** (Still work as before)

**Set Mode (Auto/Manual):**
```json
{
  "method": "setMode",
  "params": true
}
```
- `true` = Auto mode
- `false` = Manual mode

**Get Mode:**
```json
{
  "method": "getMode",
  "params": {}
}
```

**Set Pump State (Manual mode only):**
```json
{
  "method": "setPumpState",
  "params": true
}
```

**Set Fan State (Manual mode only):**
```json
{
  "method": "setFanState",
  "params": true
}
```

---

## 🛠️ ThingsBoard Dashboard Setup

### Step 1: Create RPC Widgets

1. **Go to your device dashboard**
2. **Add RPC Command widgets** for each control

#### Widget 1: Set Auto Mode ON/OFF
- Widget Type: **Control widget → Switch**
- Target device: Your mushroom house device
- RPC method: `setMode`
- Add key mappings:
  - ON value: `true`
  - OFF value: `false`

#### Widget 2: Set Time Window
- Widget Type: **Control widget → RPC Terminal** or **Two-way RPC**
- Create input fields for:
  - Start Hour (0-23)
  - Start Minute (0-59)
  - End Hour (0-23)
  - End Minute (0-59)
- RPC method: `setAutoTimeWindow`

#### Widget 3: Set Temperature Thresholds
- Widget Type: **Control widget → Slider** or **Input widget**
- Create two sliders:
  - High threshold (25-35°C)
  - Low threshold (20-30°C)
- RPC method: `setTempThresholds`

---

### Step 2: Add Telemetry Display Widgets

Display current settings on your dashboard:

1. **Add Time Label widgets:**
   - Show `autoStartHour`, `autoStartMinute`
   - Show `autoEndHour`, `autoEndMinute`

2. **Add Temperature Threshold indicators:**
   - Show `tempThresholdHigh`
   - Show `tempThresholdLow`

3. **Add Status indicator:**
   - Show `inTimeWindow` (boolean - shows if system is in active time window)

---

## 📱 Usage Examples

### Example 1: Change Operating Hours to 6 AM - 8 PM

Send RPC command from dashboard:
```json
{
  "method": "setAutoTimeWindow",
  "params": {
    "startHour": 6,
    "startMinute": 0,
    "endHour": 20,
    "endMinute": 0
  }
}
```

### Example 2: Make System More Sensitive (Earlier Turn-ON)

Send RPC command:
```json
{
  "method": "setTempThresholds",
  "params": {
    "high": 26.0,
    "low": 24.0
  }
}
```
Now the system will turn ON at 26°C instead of 27.5°C.

### Example 3: Lunch Break (Turn OFF from 12-2 PM)

**Option A:** Create a rule in ThingsBoard to send `setMode` command:
- At 12:00 PM → Send `{"method": "setMode", "params": false}`
- At 2:00 PM → Send `{"method": "setMode", "params": true}`

**Option B:** Use two time windows (requires code modification for multiple windows)

---

## ⚠️ Important Notes

1. **Time Synchronization Required:**
   - Device must be connected to internet to sync time via NTP
   - If time sync fails, auto mode will still work (assumes always in window)

2. **Auto Mode Behavior:**
   - When auto mode is ON but outside time window → System turns OFF
   - When switching to manual mode → System turns OFF (you can turn it back on manually)

3. **Settings Persistence:**
   - Settings are stored in RAM only
   - After power cycle/restart, defaults will be restored:
     - Time: 8 AM - 6 PM
     - Temp: 27.5°C high, 26.0°C low
   - **To persist settings:** You can add EEPROM storage (future enhancement)

4. **Local Control Still Works:**
   - Green button: Turn ON (manual mode)
   - Red button: Turn OFF (manual mode)
   - Green button (3-sec hold): Toggle auto/manual mode

---

## 🔧 Advanced: ThingsBoard Rule Chain Configuration

To make the system even smarter, you can create rules:

1. **Send time window changes based on weather forecast**
2. **Adjust temperature thresholds based on outside humidity**
3. **Create alerts when system operates outside time window**
4. **Log all setting changes for audit trail**

---

## 📝 Troubleshooting

**Q: Time is wrong on the device**
- A: Check that device has internet connection
- Check serial monitor for "Time synchronized successfully!"
- Malaysia is GMT+8, adjust `GMT_OFFSET_SEC` if in different timezone

**Q: System doesn't turn OFF outside time window**
- A: Check telemetry value `inTimeWindow` - should be `false`
- Check that auto mode is actually enabled (`mode: "auto"`)
- Verify current time with `currentHour` and `currentMinute` telemetry

**Q: Can't change settings from dashboard**
- A: Ensure device is connected to MQTT (check serial monitor)
- Check that RPC commands are being sent to correct device
- Verify JSON format of RPC command

**Q: Settings reset after power cycle**
- A: This is normal - settings are not saved to EEPROM
- To persist settings, we can add EEPROM storage (let me know if needed)

---

## 🚀 Next Steps / Future Enhancements

1. **EEPROM Storage** - Save settings permanently
2. **Multiple Time Windows** - Different hours for weekdays/weekends
3. **Gradual Control** - PWM for fan speed based on temperature
4. **Humidity-based Control** - Use humidity + temperature for better control
5. **Historical Data** - Store min/max temps, operating hours
6. **Maintenance Reminders** - Alert for pump filter cleaning

Let me know if you need any of these features! 🍄
