// config.h - TEMPLATE FILE
// Copy this to config.h and fill in your actual credentials
// DO NOT commit the real config.h to GitHub!

#ifndef CONFIG_H
#define CONFIG_H

// === ThingsBoard Configuration ===
// Get your device token from ThingsBoard Device Credentials
const char* THINGSBOARD_SERVER = "YOUR_SERVER_IP_HERE";  // e.g., "162.91.62.126"
const int THINGSBOARD_PORT = 1883;
const char* TOKEN = "YOUR_DEVICE_TOKEN_HERE";  // From ThingsBoard device credentials

// === NTP Configuration ===
const char* NTP_SERVER = "pool.ntp.org";
const long GMT_OFFSET_SEC = 8 * 3600;  // GMT+8 for Malaysia (change if needed)
const int DAYLIGHT_OFFSET_SEC = 0;

// === Time Window Defaults ===
const int DEFAULT_AUTO_START_HOUR = 8;   // 8 AM
const int DEFAULT_AUTO_END_HOUR = 18;    // 6 PM

// === Temperature Threshold Defaults ===
const float DEFAULT_TEMP_HIGH = 27.5;  // Turn ON
const float DEFAULT_TEMP_LOW = 26.0;   // Turn OFF

#endif
