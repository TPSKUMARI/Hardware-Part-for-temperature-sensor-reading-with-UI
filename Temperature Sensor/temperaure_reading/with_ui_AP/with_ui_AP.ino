/**
 * ESP32 Temperature Monitor with Captive Portal
 * TMP117 Sensor + WiFi Access Point + Captive Portal
 * No password required - Auto-opens on connection
 */

#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Adafruit_TMP117.h>
#include <Adafruit_Sensor.h>

// === DNS Server for Captive Portal ===
DNSServer dns;
const byte DNS_PORT = 53;

// === WiFi AP Configuration ===
const char* ssid = "Temperature_Monitor";
const char* password = ""; // Open AP (no password)
// IP will be: 192.168.4.1

// === I2C Pins for TMP117 ===
#define I2C_SDA 21
#define I2C_SCL 22

// === Objects ===
Adafruit_TMP117 tmp117;
WebServer server(80);

// === Temperature Tracking ===
float currentTemp = 0;
float minTemp = 100;
float maxTemp = -100;

// === Function Declarations ===
void setupWiFiAP();
void setupWebServer();
void handleRoot();
void handleData();
void handleReset();

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n================================");
  Serial.println("Temperature Monitor - Captive Portal");
  Serial.println("================================\n");
  
  // Initialize I2C
  Wire.begin(I2C_SDA, I2C_SCL);
  Serial.println("I2C initialized");
  
  // Initialize TMP117
  Serial.print("Looking for TMP117 sensor...");
  if (!tmp117.begin()) {
    Serial.println(" FAILED!");
    Serial.println("Check wiring:");
    Serial.println("  TMP117 VCC -> ESP32 3.3V");
    Serial.println("  TMP117 GND -> ESP32 GND");
    Serial.println("  TMP117 SDA -> GPIO 21");
    Serial.println("  TMP117 SCL -> GPIO 22");
    while (1) {
      delay(1000);
      Serial.print(".");
    }
  }
  Serial.println(" Found!");
  
  // Setup WiFi Access Point with Captive Portal
  setupWiFiAP();
  
  // Setup Web Server
  setupWebServer();
  
  Serial.println("\n✓ System Ready!");
  Serial.println("================================");
  Serial.println("Connect to WiFi: " + String(ssid));
  Serial.println("Captive portal will auto-open");
  Serial.println("Or browse to: http://192.168.4.1");
  Serial.println("================================\n");
}

void loop() {
  // Handle DNS requests (captive portal)
  dns.processNextRequest();
  
  // Handle web requests
  server.handleClient();
  
  // Read temperature
  sensors_event_t temp;
  tmp117.getEvent(&temp);
  currentTemp = temp.temperature;
  
  // Update min/max
  if (currentTemp < minTemp) minTemp = currentTemp;
  if (currentTemp > maxTemp) maxTemp = currentTemp;
  
  delay(100);
}

// ========== WiFi Access Point Setup ==========
void setupWiFiAP() {
  // Set WiFi mode to Access Point only
  WiFi.mode(WIFI_AP);
  
  // Create open AP (no password)
  WiFi.softAP(ssid);
  
  // Force IP to 192.168.4.1
  WiFi.softAPConfig(
    IPAddress(192, 168, 4, 1),  // IP
    IPAddress(192, 168, 4, 1),  // Gateway
    IPAddress(255, 255, 255, 0) // Subnet
  );
  
  IPAddress apIP = WiFi.softAPIP();
  
  Serial.println("✓ WiFi AP started");
  Serial.print("  SSID: "); Serial.println(ssid);
  Serial.print("  IP:   "); Serial.println(apIP);
  
  // Start DNS server for captive portal
  // Resolve ALL domains to our IP
  dns.start(DNS_PORT, "*", apIP);
  Serial.println("✓ DNS captive portal started");
}

// ========== Web Server Setup ==========
void setupWebServer() {
  server.enableCORS(true);
  
  // Main page
  server.on("/", HTTP_GET, handleRoot);
  server.on("/data", HTTP_GET, handleData);
  server.on("/reset", HTTP_GET, handleReset);
  
  // CORS preflight
  server.on("/data", HTTP_OPTIONS, [](){
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "GET");
    server.send(200);
  });
  
  // ========== Captive Portal Probes ==========
  // Android
  server.on("/generate_204", HTTP_ANY, [](){
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    server.send(200, "text/html",
      "<!doctype html><html><head><meta http-equiv='refresh' content='0; url=/'></head><body></body></html>");
  });
  
  // iOS / macOS
  server.on("/hotspot-detect.html", HTTP_ANY, [](){
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    server.send(200, "text/html",
      "<!doctype html><html><head><meta http-equiv='refresh' content='0; url=/'></head><body></body></html>");
  });
  
  // Windows
  server.on("/ncsi.txt", HTTP_ANY, [](){
    server.send(200, "text/plain", "Microsoft NCSI");
  });
  
  server.on("/connecttest.txt", HTTP_ANY, [](){
    server.send(200, "text/plain", "OK");
  });
  
  server.on("/redirect", HTTP_ANY, [](){
    server.sendHeader("Location", "/", true);
    server.send(302, "text/plain", "");
  });
  
  server.on("/fwlink", HTTP_ANY, [](){
    server.sendHeader("Location", "/", true);
    server.send(302, "text/plain", "");
  });
  
  // ========== Catch-all: Redirect to Main Page ==========
  server.onNotFound([](){
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    server.sendHeader("Pragma", "no-cache");
    server.sendHeader("Expires", "-1");
    server.sendHeader("Location", "/", true);
    server.send(302, "text/plain", "");
  });
  
  server.begin();
  Serial.println("✓ Web server started");
}

// ========== Web Page Handler ==========
void handleRoot() {
  String html = R"=====(
<!DOCTYPE html>
<html>
<head>
  <title>Temperature Monitor</title>
  <meta charset='UTF-8'>
  <meta name='viewport' content='width=device-width, initial-scale=1.0'>
  <meta http-equiv='Cache-Control' content='no-cache'>
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }
    body {
      font-family: Arial, sans-serif;
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      color: white;
      min-height: 100vh;
      display: flex;
      align-items: center;
      justify-content: center;
      padding: 20px;
    }
    .container {
      background: rgba(255, 255, 255, 0.15);
      backdrop-filter: blur(10px);
      border-radius: 20px;
      padding: 40px;
      max-width: 600px;
      width: 100%;
      box-shadow: 0 8px 32px rgba(0,0,0,0.3);
      border: 2px solid rgba(255, 255, 255, 0.2);
    }
    h1 {
      text-align: center;
      font-size: 2.5em;
      margin-bottom: 30px;
      text-shadow: 2px 2px 4px rgba(0,0,0,0.3);
    }
    .temp-display {
      text-align: center;
      font-size: 5em;
      font-weight: bold;
      margin: 30px 0;
      text-shadow: 3px 3px 6px rgba(0,0,0,0.4);
      color: #00ffff;
      animation: pulse 2s infinite;
    }
    .stats {
      display: grid;
      grid-template-columns: 1fr 1fr;
      gap: 20px;
      margin: 30px 0;
    }
    .stat-box {
      background: rgba(0, 0, 0, 0.2);
      padding: 20px;
      border-radius: 15px;
      text-align: center;
      border: 1px solid rgba(255, 255, 255, 0.1);
    }
    .stat-label {
      font-size: 1.2em;
      opacity: 0.8;
      margin-bottom: 10px;
    }
    .stat-value {
      font-size: 2em;
      font-weight: bold;
    }
    .min-temp { color: #4fc3f7; }
    .max-temp { color: #ff6b6b; }
    .status {
      font-size: 1.8em;
      text-align: center;
      padding: 20px;
      background: rgba(255, 255, 255, 0.2);
      border-radius: 15px;
      margin: 20px 0;
      font-weight: bold;
    }
    .reset-btn {
      width: 100%;
      background: #4CAF50;
      color: white;
      border: none;
      padding: 15px;
      font-size: 1.2em;
      border-radius: 10px;
      cursor: pointer;
      margin-top: 20px;
      transition: background 0.3s;
    }
    .reset-btn:hover {
      background: #45a049;
    }
    .connection {
      text-align: center;
      margin-top: 20px;
      font-size: 0.9em;
      opacity: 0.7;
    }
    @keyframes pulse {
      0%, 100% { transform: scale(1); }
      50% { transform: scale(1.05); }
    }
  </style>
</head>
<body>
  <div class='container'>
    <h1>🌡️ Temperature</h1>
    
    <div class='temp-display' id='currentTemp'>--°C</div>
    
    <div class='status' id='status'>Loading...</div>
    
    <div class='stats'>
      <div class='stat-box'>
        <div class='stat-label'>Minimum</div>
        <div class='stat-value min-temp' id='minTemp'>--°C</div>
      </div>
      <div class='stat-box'>
        <div class='stat-label'>Maximum</div>
        <div class='stat-value max-temp' id='maxTemp'>--°C</div>
      </div>
    </div>
    
    <button class='reset-btn' onclick='resetMinMax()'>Reset Min/Max</button>
    
    <div class='connection' id='connection'>Connected</div>
  </div>
  
  <script>
    let errorCount = 0;
    
    function updateData() {
      fetch('/data', {method: 'GET', cache: 'no-cache'})
        .then(response => {
          if (!response.ok) throw new Error('Network error');
          return response.json();
        })
        .then(data => {
          errorCount = 0;
          
          // Update temperature
          document.getElementById('currentTemp').textContent = 
            data.current.toFixed(1) + '°C';
          
          // Update min/max
          document.getElementById('minTemp').textContent = 
            data.min.toFixed(1) + '°C';
          document.getElementById('maxTemp').textContent = 
            data.max.toFixed(1) + '°C';
          
          // Update status
          let status = '';
          if (data.current < 16) status = '🥶 COLD';
          else if (data.current < 20) status = '❄️ COOL';
          else if (data.current < 24) status = '😊 COMFORTABLE';
          else if (data.current < 28) status = '☀️ WARM';
          else status = '🔥 HOT';
          document.getElementById('status').textContent = status;
          
          document.getElementById('connection').textContent = '● Connected';
        })
        .catch(error => {
          errorCount++;
          document.getElementById('connection').textContent = 
            '● Error (' + errorCount + ')';
          console.error('Error:', error);
        });
    }
    
    function resetMinMax() {
      fetch('/reset')
        .then(r => r.text())
        .then(_ => {
          document.getElementById('status').textContent = '✓ Reset Complete';
          setTimeout(updateData, 500);
        });
    }
    
    // Update every second
    updateData();
    setInterval(updateData, 1000);
  </script>
</body>
</html>
)=====";

  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.send(200, "text/html", html);
}

// ========== API Endpoints ==========
void handleData() {
  String json = "{";
  json += "\"current\":" + String(currentTemp, 2) + ",";
  json += "\"min\":" + String(minTemp, 2) + ",";
  json += "\"max\":" + String(maxTemp, 2);
  json += "}";
  
  server.sendHeader("Cache-Control", "no-cache");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", json);
}

void handleReset() {
  minTemp = currentTemp;
  maxTemp = currentTemp;
  
  server.sendHeader("Cache-Control", "no-cache");
  server.send(200, "text/plain", "OK");
  
  Serial.println("Min/Max temperature reset");
}