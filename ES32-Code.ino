#include <WiFi.h>
#include <HTTPClient.h>

#define LED_PIN 2
#define RXD2 27
#define TXD2 28

const char* wifiName = "";
const char* wifiPassword = "";
const char* serverAddress = ""; // Server IP Address
const int serverPort = 3000;
const char* authToken = "2434902c38d2bac903e913a53d78284994a765509cac728d06437e635f319236"; // Authentication Token

// DFPlayer Mini Commands
#define START_BYTE 0x7E
#define VERSION_BYTE 0xFF
#define LENGTH_BYTE 0x06
#define END_BYTE 0xEF
#define CMD_PLAY_W_INDEX 0x03
#define CMD_VOLUME 0x06
#define MAX_VOLUME 30

void setup() {
  Serial.begin(115200);
  connectToWiFi();
  pinMode(LED_PIN, OUTPUT);

  // Initialize DFPlayer Mini Mp3 module
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  mp3.begin(Serial2);
  
  // Set DFPlayer Mini volume
  setVolume(MAX_VOLUME);

  // Play audio upon ESP32 startup
  playAudio("0001-timeForBed.mp3");
}

void loop() {
  // Main loop code can be left empty as audio playback has been initiated in setup
}

void connectToWiFi() {
  Serial.print("Connecting to network ");
  Serial.println(wifiName);
  WiFi.begin(wifiName, wifiPassword);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 5) {
    delay(2000);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi connection established");
    Serial.println("IP Address: ");
    Serial.println(WiFi.localIP());
    sendRegistrationData(generateSerialNumber());
  } else {
    Serial.println("");
    Serial.println("Failed to connect to network");
  }
}

void sendRegistrationData(String serialNumber) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Error: WiFi not connected.");
    return;
  }

  HTTPClient http;

  String url = "http://" + String(serverAddress) + ":" + String(serverPort) + "/register";
  http.begin(url);

  // Add authentication token to request header
  http.addHeader("Authorization", "Bearer " + String(authToken));
  http.addHeader("Content-Type", "application/json");

  // Construct JSON request body
  String postData = "{\"name\":\"" + serialNumber + "\",\"ip\":\"" + WiFi.localIP().toString() + "\"}";

  int httpResponseCode = http.POST(postData);

  if (httpResponseCode > 0) {
    if (httpResponseCode == HTTP_CODE_OK) {
      Serial.println("Bear's Data:");
      Serial.println("Status: Online");
      Serial.print("Bear Name: ");
      Serial.println(serialNumber);
      Serial.print("Bear IP Address: ");
      Serial.println(WiFi.localIP());
    } else {
      Serial.print("Error registering bear. HTTP response code: ");
      Serial.println(httpResponseCode);
    }
  } else {
    Serial.println("Error connecting to server.");
  }

  http.end();
}

void playAudio(String fileName) {
  uint8_t cmd[] = {START_BYTE, VERSION_BYTE, LENGTH_BYTE, CMD_PLAY_W_INDEX, 0x00, 0x00, 0x00, 0x00, 0x00, END_BYTE};
  cmd[5] = 0x01; // Play the first file (value 0x01) on the SD card
  cmd[7] = 0x01; // Continuous playback (value 0x01) indefinite

  Serial2.write(cmd, sizeof(cmd));
}

void setVolume(uint8_t volume) {
  uint8_t cmd[] = {START_BYTE, VERSION_BYTE, LENGTH_BYTE, CMD_VOLUME, 0x00, 0x00, volume, END_BYTE};
  Serial2.write(cmd, sizeof(cmd));
}

String generateSerialNumber() {
  uint8_t mac[6];
  WiFi.macAddress(mac);
  String serialNumber = "BEAR-" + String(mac[5], HEX);
  return serialNumber;
}
