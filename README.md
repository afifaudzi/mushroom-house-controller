# 🍄 Mushroom House IoT Controller

ESP32-S3 based evaporative cooling system controller for mushroom cultivation with ThingsBoard IoT integration.

![Version](https://img.shields.io/badge/version-2.0-blue)
![Platform](https://img.shields.io/badge/platform-ESP32--S3-green)
![License](https://img.shields.io/badge/license-MIT-yellow)

## 📋 Features

- **Automated Temperature Control**: Maintains optimal growing temperature (default: 27.5°C)
- **Time-Based Operation**: Configurable operating hours (default: 8 AM - 6 PM)
- **Evaporative Cooling**: Controls pump and fan with 2-second staggered startup
- **Remote Control**: Full control via ThingsBoard dashboard
- **Local Control**: Physical switches for manual override
- **Real-time Monitoring**: Telemetry data every 30 seconds
- **NTP Time Sync**: Automatic time synchronization
- **WiFi Manager**: Easy WiFi setup via captive portal

## 🔧 Hardware Requirements

### Components
- ESP32-S3 Development Board
- 4-Channel Relay Module (active LOW)
- Water Pump (for cooling pads)
- Fan
- 2x Push Buttons (Green: ON, Red: OFF)
- 2x LED indicators (Green: Active, Red: Standby)
- Evaporative cooling pads

### Pin Configuration

| Component | GPIO Pin |
|-----------|----------|
| Red LED | 15 |
| Green LED | 16 |
| Pump Relay | 17 |
| Fan Relay | 18 |
| Green Switch | 9 |
| Red Switch | 10 |

## 📦 Software Requirements

### Arduino Libraries
```cpp
WiFi.h              // Built-in
DNSServer.h         // Built-in
WebServer.h         // Built-in
time.h              // Built-in
WiFiManager.h       // tzapu/WiFiManager
PubSubClient.h      // knolleary/PubSubClient
ArduinoJson.h       // bblanchon/ArduinoJson
```

### Installation
1. Install [Arduino IDE](https://www.arduino.cc/en/software) or [PlatformIO](https://platformio.org/)
2. Add ESP32 board support
3. Install required libraries via Library Manager

## 🚀 Quick Start

### 1. Clone Repository
```bash
git clone https://github.com/YOUR_USERNAME/mushroom-house-controller.git
cd mushroom-house-controller
```

### 2. Configure Credentials
```bash
# Copy template to config.h
cp config_template.h config.h

# Edit config.h with your credentials
nano config.h
```

**config.h example:**
```cpp
const char* THINGSBOARD_SERVER = "your.thingsboard.server";
const int THINGSBOARD_PORT = 1883;
const char* TOKEN = "your_device_token_here";
```

### 3. Upload Code
1. Open `mushroom_controller_github.ino` in Arduino IDE
2. Select Board: **ESP32S3 Dev Module**
3. Select Port: Your ESP32 port
4. Click Upload

### 4. First Boot
1. Device creates WiFi AP: `MushroomNodeAP-H1`
2. Password: `mushroom_pw`
3. Connect and configure your WiFi
4. Device auto-connects on subsequent boots

## 🎮 Operation Modes

### Manual Mode (Default)
- **Green Button (short press)**: Turn ON pump and fan
- **Red Button**: Turn OFF pump and fan
- Full control via physical buttons or dashboard

### Auto Mode
- **Green Button (hold 3 sec)**: Toggle to Auto mode
- System automatically controls based on temperature
- Only operates during configured time window
- Falls back to Manual if internet/MQTT lost

## 📊 ThingsBoard Integration

### Telemetry Data Sent
```json
{
  "pump": true,
  "fan": true,
  "mode": "auto",
  "temperature": 28.5,
  "humidity": 75.0,
  "co2": 450,
  "tempThresholdHigh": 27.5,
  "tempThresholdLow": 26.0,
  "autoStartHour": 8,
  "autoEndHour": 18,
  "currentHour": 14,
  "currentMinute": 30,
  "inTimeWindow": true,
  "ledGreen": 1,
  "ledRed": 0
}
```

### RPC Commands

#### Enable Auto Mode
```json
{
  "method": "setMode",
  "params": true
}
```

#### Set Operating Hours
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

#### Set Temperature Thresholds
```json
{
  "method": "setTempThresholds",
  "params": {
    "high": 28.0,
    "low": 26.0
  }
}
```

#### Get Current Settings
```json
{
  "method": "getSettings",
  "params": {}
}
```

## 🛠️ Configuration

### Temperature Control
- **Default Turn ON**: > 27.5°C
- **Default Turn OFF**: < 26.0°C
- Hysteresis prevents rapid cycling
- Configurable via dashboard

### Time Window
- **Default Hours**: 8:00 AM - 6:00 PM
- Outside window: System turns OFF
- Saves energy and extends equipment life
- Fully configurable remotely

### Timezone
Default: GMT+8 (Malaysia)
```cpp
const long GMT_OFFSET_SEC = 8 * 3600;  // Change for your timezone
```

## 📱 Dashboard Setup

See [DASHBOARD_SETUP_GUIDE.md](DASHBOARD_SETUP_GUIDE.md) for detailed widget configuration.

**Quick Widgets:**
1. Auto Mode Switch
2. Temperature Thresholds Sliders
3. Time Window Input
4. Current Status Display
5. Pump/Fan State Indicators

## 🔒 Security Best Practices

### ⚠️ NEVER commit these files:
- `config.h` (contains credentials)
- Any file with API keys/tokens
- IP addresses or server details

### ✅ DO commit:
- `config_template.h` (template only)
- `.gitignore`
- Main code files
- Documentation

### If Credentials Exposed:
1. **Immediately** regenerate ThingsBoard device token
2. Update your local `config.h`
3. Remove exposed token from Git history:
```bash
git filter-branch --force --index-filter \
  "git rm --cached --ignore-unmatch config.h" \
  --prune-empty --tag-name-filter cat -- --all
```
4. Force push: `git push origin --force --all`

## 🐛 Troubleshooting

### WiFi Not Connecting
- Check saved credentials
- Reset WiFi: Hold green button for 10+ seconds (if implemented)
- Or flash with new credentials

### MQTT Not Connecting
- Verify ThingsBoard server IP is accessible
- Check device token is correct
- Check firewall allows port 1883

### Time Not Syncing
- Ensure internet connection
- Check NTP server: `pool.ntp.org`
- Adjust timezone in `config.h`

### Auto Mode Not Working
- Check telemetry: `inTimeWindow` should be `true`
- Verify current time is correct
- Confirm temperature from iRIV device is updating

## 📈 Future Enhancements

- [ ] EEPROM storage for settings persistence
- [ ] Multiple time windows (weekday/weekend)
- [ ] PWM fan speed control
- [ ] Humidity-based control
- [ ] SMS/Email alerts
- [ ] OTA updates
- [ ] Historical data logging
- [ ] Predictive maintenance

## 🤝 Contributing

Contributions welcome! Please:
1. Fork the repository
2. Create feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit changes (`git commit -m 'Add AmazingFeature'`)
4. Push to branch (`git push origin feature/AmazingFeature`)
5. Open Pull Request

## 📄 License

This project is licensed under the MIT License - see [LICENSE](LICENSE) file for details.

## 👨‍💻 Author

**Your Name**
- GitHub: [@afifaudzi](https://github.com/afifaudzi)
- ThingsBoard: Your TB profile

## 🙏 Acknowledgments

- [ThingsBoard](https://thingsboard.io/) - IoT platform
- [WiFiManager](https://github.com/tzapu/WiFiManager) - Easy WiFi setup
- Mushroom cultivation community

## 📞 Support

- Open an [Issue](https://github.com/yourusername/mushroom-house-controller/issues)
- Check [Documentation](FEATURE_DOCUMENTATION.md)
- ThingsBoard [Community](https://thingsboard.io/community/)

---

**⚠️ Safety Warning**: Ensure proper electrical isolation for water pump. Use GFCI protection. Follow local electrical codes.

**🍄 Happy Growing!**
