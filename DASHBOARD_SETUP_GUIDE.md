# ThingsBoard Dashboard Widget Examples

## Quick Setup Guide for Your Dashboard

### Widget 1: Auto Mode Switch
```json
{
  "type": "rpc",
  "settings": {
    "title": "Auto Mode",
    "requestTimeout": 500,
    "requestPersistent": false,
    "persistentPollingInterval": 5000,
    "showTitle": true,
    "dropShadow": true,
    "enableFullscreen": false,
    "widgetStyle": {},
    "titleStyle": {
      "fontSize": "16px",
      "fontWeight": 400
    },
    "controlWidgetStyle": {},
    "width": "100%",
    "oneWayElseTwoWay": false,
    "setValueMethod": "setMode",
    "getValueMethod": "getMode",
    "valueKey": "value",
    "valueExpression": "",
    "parseValueFunction": "return value === 'true';",
    "convertValueFunction": "return value;",
    "compareToValue": true,
    "requestValueOnInit": true,
    "initialState": false,
    "checkInterval": 5000,
    "showOnOffLabel": true,
    "onUpdateLabel": "Auto",
    "offUpdateLabel": "Manual"
  }
}
```

### Widget 2: Time Window Control (Simple Version)

Create 4 input widgets with these RPC settings:

**Start Hour Widget:**
```json
{
  "method": "setAutoTimeWindow",
  "params": {
    "startHour": "${inputValue}",
    "startMinute": 0
  }
}
```

**End Hour Widget:**
```json
{
  "method": "setAutoTimeWindow",
  "params": {
    "endHour": "${inputValue}",
    "endMinute": 0
  }
}
```

### Widget 3: Temperature Threshold Sliders

**High Threshold Slider:**
```json
{
  "type": "rpc",
  "settings": {
    "title": "Turn ON Temperature (°C)",
    "showTitle": true,
    "widgetStyle": {},
    "setValueMethod": "setTempThresholds",
    "getValueMethod": "getSettings",
    "valueKey": "tempThresholdHigh",
    "minValue": 25,
    "maxValue": 35,
    "step": 0.5,
    "showTickMarks": true
  }
}
```

**Low Threshold Slider:**
```json
{
  "type": "rpc",
  "settings": {
    "title": "Turn OFF Temperature (°C)",
    "showTitle": true,
    "widgetStyle": {},
    "setValueMethod": "setTempThresholds",
    "getValueMethod": "getSettings",
    "valueKey": "tempThresholdLow",
    "minValue": 20,
    "maxValue": 30,
    "step": 0.5,
    "showTickMarks": true
  }
}
```

---

## Dashboard Layout Suggestion

```
┌──────────────────────────────────────────────┐
│  Mushroom House H1 - Control Panel          │
├──────────────────────────────────────────────┤
│                                              │
│  ┌─────────────┐  ┌─────────────┐          │
│  │ Auto Mode   │  │ Current     │          │
│  │   [  ON  ]  │  │ Temp: 28.5°C│          │
│  └─────────────┘  └─────────────┘          │
│                                              │
│  ┌──────────────────────────────┐          │
│  │ Operating Hours              │          │
│  │ Start:  [08] : [00]          │          │
│  │ End:    [18] : [00]          │          │
│  │ Status: ✅ IN WINDOW         │          │
│  └──────────────────────────────┘          │
│                                              │
│  ┌──────────────────────────────┐          │
│  │ Temperature Control          │          │
│  │ Turn ON:  [━━━●━━━] 27.5°C   │          │
│  │ Turn OFF: [━━●━━━━] 26.0°C   │          │
│  └──────────────────────────────┘          │
│                                              │
│  ┌─────────────┐  ┌─────────────┐          │
│  │ Pump        │  │ Fan         │          │
│  │   ● ON      │  │   ● ON      │          │
│  └─────────────┘  └─────────────┘          │
│                                              │
│  ┌──────────────────────────────┐          │
│  │ Environment Data (from iRIV) │          │
│  │ Temp:     28.5°C             │          │
│  │ Humidity: 75%                │          │
│  │ CO2:      450 ppm            │          │
│  └──────────────────────────────┘          │
│                                              │
└──────────────────────────────────────────────┘
```

---

## Step-by-Step Widget Creation

### 1. Create Auto Mode Switch

1. Click **"+"** to add widget
2. Select **"Control widgets"**
3. Choose **"Switch control"**
4. Configure:
   - **Target device:** Select your mushroom house device
   - **RPC get value method:** `getMode`
   - **RPC set value method:** `setMode`
   - **Parse function:**
     ```javascript
     return value === 'true' || value === true;
     ```
   - **On value:** `true`
   - **Off value:** `false`
   - **On label:** "Auto"
   - **Off label:** "Manual"

### 2. Create Time Display (Current Time Window)

1. Add **"Latest values"** widget
2. Configure data keys:
   - `autoStartHour` - Label: "Start Hour"
   - `autoStartMinute` - Label: "Start Min"
   - `autoEndHour` - Label: "End Hour"
   - `autoEndMinute` - Label: "End Min"
   - `inTimeWindow` - Label: "Active Now"

### 3. Create Temperature Threshold Display

1. Add **"Latest values"** widget
2. Configure:
   - `tempThresholdHigh` - Label: "Turn ON at (°C)"
   - `tempThresholdLow` - Label: "Turn OFF at (°C)"
   - Add units: "°C"

### 4. Create Time Window Control (Advanced - Using RPC Terminal)

1. Add **"RPC Terminal"** widget
2. Add these buttons:

**Button: Set 7 AM - 7 PM**
```json
{
  "method": "setAutoTimeWindow",
  "params": {
    "startHour": 7,
    "startMinute": 0,
    "endHour": 19,
    "endMinute": 0
  }
}
```

**Button: Set 8 AM - 6 PM (Default)**
```json
{
  "method": "setAutoTimeWindow",
  "params": {
    "startHour": 8,
    "startMinute": 0,
    "endHour": 18,
    "endMinute": 0
  }
}
```

**Button: Set 6 AM - 8 PM (Extended)**
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

### 5. Create Quick Temperature Presets

Add RPC buttons for common settings:

**Button: Hot Day (26°C ON)**
```json
{
  "method": "setTempThresholds",
  "params": {
    "high": 26.0,
    "low": 24.0
  }
}
```

**Button: Normal (27.5°C ON - Default)**
```json
{
  "method": "setTempThresholds",
  "params": {
    "high": 27.5,
    "low": 26.0
  }
}
```

**Button: Very Hot (29°C ON)**
```json
{
  "method": "setTempThresholds",
  "params": {
    "high": 29.0,
    "low": 27.0
  }
}
```

---

## Testing Your Dashboard

### Test Sequence:

1. **Verify telemetry is coming in:**
   - Check that temperature, humidity, CO2 are updating
   - Check that current time (`currentHour`, `currentMinute`) is correct

2. **Test Auto Mode toggle:**
   - Toggle switch to Auto
   - Check device serial monitor shows "Mode changed to AUTO"
   - Toggle back to Manual

3. **Test Time Window change:**
   - Send RPC command to change time window
   - Check telemetry shows new values
   - Verify `inTimeWindow` updates correctly

4. **Test Temperature Threshold:**
   - Change high threshold to current temp - 1°C
   - Wait 30 seconds
   - System should turn ON
   - Change back to default

5. **Test Outside Time Window:**
   - Set end time to current hour - 1
   - System should turn OFF
   - Check `inTimeWindow` = false

---

## Mobile App Optimization

If using ThingsBoard mobile app:

1. Use **"Control widgets"** bundle for better mobile experience
2. Arrange widgets in single column for vertical scrolling
3. Use larger buttons and sliders (min height: 100px)
4. Add confirmation dialogs for critical actions

---

## Common Issues & Solutions

**Issue:** RPC commands not working
- Solution: Check that device is online and MQTT connected
- Check Serial Monitor for "Incoming: v1/devices/me/rpc/request/..."

**Issue:** Time window not updating
- Solution: Verify JSON format is correct
- Check that all 4 parameters are sent (startHour, startMinute, endHour, endMinute)

**Issue:** Sliders not showing current value
- Solution: Use `getSettings` RPC to fetch current values on dashboard load
- Set "Request value on init" to true

---

## Example: Complete RPC Call from Dashboard

When you press a button/slider, ThingsBoard sends:

```
Topic: v1/devices/me/rpc/request/1234
Payload: {
  "method": "setAutoTimeWindow",
  "params": {
    "startHour": 7,
    "startMinute": 30,
    "endHour": 19,
    "endMinute": 45
  }
}
```

Device receives, processes, and publishes updated telemetry with new values.

---

Need help creating specific widgets? Let me know! 🚀
