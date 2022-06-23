/*
  Board: ESP32 Dev Module
  Flash Size: 4MB
  Partition Scheme: Huge App
*/
//BLE
#include "BLEDevice.h"

//WiFi
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>

const char *DomainName = "gopro";  // set domain name domain.local
const char *ssid = "GoPro_RC";
const char *password = "00000000";

WebServer server(80);

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
    <title>GoPro RC</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        html { font-family: Arial; display: inline-block; text-align: center; }
        h2 { font-size: 3.0rem; }
        p { font-size: 1.0rem; }
        body { max-width: 600px; margin: 0px auto; padding-bottom: 25px; }
    </style>
</head>
<body>
    <h2>GoPro RC</h2>
    <input type="button" onclick="getReq('/pair')" value="Connect/Pair new">
    <input type="button" onclick="getReq('/shutterOn')" value="Shutter On">
    <input type="button" onclick="getReq('/shutterOff')" value="Shutter Off">
    <p id="status"></p>
    <fieldset>
        <legend><b>Connected devices</b></legend>
        <div id="list"></div>
    </fieldset>

    <script>
        function getReq(req) {
            var xhr = new XMLHttpRequest();
            xhr.onreadystatechange = function () {
                if (this.readyState == 4 && this.status == 200) {
                    document.getElementById("status").innerText = "Command was sent!";
                    
                    window.setTimeout(function(){
                        document.getElementById("status").innerText = "";                        
                    }, 2000);
                }
            };
            xhr.open("GET", req, true);
            xhr.send();
        }

        window.setInterval(function(){ 
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function () {
                if (this.readyState == 4 && this.status == 200) {
                    document.getElementById("list").innerText = this.responseText;
                }
            };
            xhttp.open("GET", "/list", true);
            xhttp.send(); 
        }, 5000);
    </script>
</body>
</html>
)rawliteral";

//void handleNotFound() {
//  server.send(404, "text/plain", "404 - File Not Found");
//}

// The remote service we wish to connect to.
static BLEUUID controlServiceUUID("0000FEA6-0000-1000-8000-00805F9B34FB"); //Cam control service

// The characteristic of the remote service we are interested in.
static BLEUUID    commandUUID("B5F90072-AA8D-11E3-9046-0002A5D5C51B"); // Command [WRITE]
static BLEUUID    commandRespUUID("B5F90073-AA8D-11E3-9046-0002A5D5C51B"); // Command response [NOTIFY]
static BLEUUID    settingsUUID("B5F90074-AA8D-11E3-9046-0002A5D5C51B"); // Settings [WRITE]
static BLEUUID    settingsRespUUID("B5F90075-AA8D-11E3-9046-0002A5D5C51B"); // Settings response [NOTIFY]

struct GoProCam {
  BLEAdvertisedDevice* pDevice = nullptr;
  BLEClient* pClient = nullptr;
  bool toConnect = false;
  bool isConnected = false;
  uint32_t lastKeepAlive = 0;
  BLERemoteCharacteristic* pCommandCharacteristic = nullptr;
  BLERemoteCharacteristic* pCommandRespCharacteristic = nullptr;
  BLERemoteCharacteristic* pSettingsCharacteristic = nullptr;
  BLERemoteCharacteristic* pSettingsRespCharacteristic = nullptr;
};

// Try to provide 10 cams
const uint8_t maxCams = 10;
GoProCam goProCams[maxCams];
uint8_t scanSeconds = 5;

//The commands below are sent to pSettingsCharacteristic and responses/notifications are received on pSettingsRespCharacteristic
uint8_t keepAlive[] = {0x03, 0x5B, 0x01 , 0x42}; //Only Hero 9 and 10 ? Hero 8 response is Error

//The commands below are sent to pCommandCharacteristic and responses/notifications are received on pCommandRespCharacteristic
uint8_t shutterOff[] = {0x03, 0x01, 0x01 , 0x00};
uint8_t shutterOn[] = {0x03, 0x01, 0x01 , 0x01};
uint8_t camSleep[] = {0x01, 0x05};
uint8_t wiFiAPoff[] = {0x03, 0x17, 0x01, 0x00};
uint8_t wiFiAPon[] = {0x03, 0x17, 0x01, 0x01};
uint8_t hilight[] = {0x01, 0x18};
uint8_t videoGroup[] = {0x04, 0x3E, 0x02, 0x03, 0xE8};
uint8_t photoGroup[] = {0x04, 0x3E, 0x02, 0x03, 0xE9};
uint8_t timelapseGroup[] = {0x04, 0x3E, 0x02, 0x03, 0xEA};
uint8_t standard[] = {0x06, 0x40, 0x04, 0x00, 0x00, 0x00, 0x00};
uint8_t activity[] = {0x06, 0x40, 0x04, 0x00, 0x00, 0x00, 0x01};
uint8_t cinematic[] = {0x06, 0x40, 0x04, 0x00, 0x00, 0x00, 0x02};
uint8_t photo[] = {0x06, 0x40, 0x04, 0x00, 0x01, 0x00, 0x00};
uint8_t liveBurst[] = {0x06, 0x40, 0x04, 0x00, 0x01, 0x00, 0x01};
uint8_t burstPhoto[] = {0x06, 0x40, 0x04, 0x00, 0x01, 0x00, 0x02};
uint8_t nightPhoto[] = {0x06, 0x40, 0x04, 0x00, 0x01, 0x00, 0x03};
uint8_t timeWarp[] = {0x06, 0x40, 0x04, 0x00, 0x02, 0x00, 0x00};
uint8_t timeLapse[] = {0x06, 0x40, 0x04, 0x00, 0x02, 0x00, 0x01};
uint8_t nightLapse[] = {0x06, 0x40, 0x04, 0x00, 0x02, 0x00, 0x02};

/*
   Security
*/
class MySecurity : public BLESecurityCallbacks {
    uint32_t onPassKeyRequest() {
      Serial.println("[onPassKeyRequest]");
      return 123456;
    }
    void onPassKeyNotify(uint32_t pass_key) {
      Serial.printf("[onPassKeyNotify] %d\n", pass_key);
    }
    bool onConfirmPIN(uint32_t pass_key) {
      Serial.printf("[onConfirmPIN] %d\n", pass_key);
      vTaskDelay(5000);
      return true;
    }
    bool onSecurityRequest() {
      Serial.println("[onSecurityRequest]");
      return true;
    }
    void onAuthenticationComplete(esp_ble_auth_cmpl_t auth_cmpl) {
      Serial.printf("[onAuthenticationComplete] remote BD_ADDR: %02x:%02x:%02x:%02x:%02x:%02x, ", auth_cmpl.bd_addr[0], auth_cmpl.bd_addr[1], auth_cmpl.bd_addr[2], auth_cmpl.bd_addr[3], auth_cmpl.bd_addr[4], auth_cmpl.bd_addr[5]);
      Serial.printf("address type = %d - pairing %s\n", auth_cmpl.addr_type, auth_cmpl.success ? "success" : "fail");
    }
};

static void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
  Serial.printf("[notifyCallback] Command 0x%02X", pData[length - 2]);

  switch (pData[length - 1]) {
    case 0:
      Serial.print(" 'Success', ");
      break;
    case 1:
      Serial.print(" 'Error', ");
      break;
    case 2:
      Serial.print(" 'Invalid parameter', ");
      break;
    default:
      Serial.print(" 'Unknown', ");
  }
  //Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());

  Serial.print("data: {");
  for (uint8_t i = 0; i < length; i++) {
    Serial.printf("0x%02X", pData[i]);
    if (i < length - 1) Serial.print(", ");
  }
  Serial.println("}");
}

class MyClientCallback : public BLEClientCallbacks {
    void onConnect(BLEClient* pClient) {
      //Serial.printf("[onConnect] %s\n", pClient->getPeerAddress().toString().c_str());
    }

    void onDisconnect(BLEClient* pClient) {
      for (uint8_t i = 0; i < maxCams; i++) {
        if (goProCams[i].pClient != nullptr) {
          if (goProCams[i].pClient->getPeerAddress().equals(pClient->getPeerAddress())) { // TODO Check if this works
            goProCams[i].isConnected = false;
            goProCams[i].pClient = nullptr;
            goProCams[i].pDevice = nullptr;
            Serial.printf("[onDisconnect] %s\n", pClient->getPeerAddress().toString().c_str());
            return;
          }
        }
      }
    }
};

bool connectToCam(uint8_t camIndex) {
  Serial.printf("[connectToCam] connecting %s ...\n", goProCams[camIndex].pDevice->getAddress().toString().c_str());

  // Set security
  BLEDevice::setEncryptionLevel(ESP_BLE_SEC_ENCRYPT);
  BLEDevice::setSecurityCallbacks(new MySecurity());
  BLESecurity *pSecurity = new BLESecurity();
  pSecurity->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_BOND); //ESP_LE_AUTH_REQ_SC_ONLY
  pSecurity->setCapability(ESP_IO_CAP_OUT);
  pSecurity->setRespEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);

  // Create client
  goProCams[camIndex].pClient = BLEDevice::createClient();
  goProCams[camIndex].pClient->setClientCallbacks(new MyClientCallback());

  // Connect to the remove BLE Server.
  goProCams[camIndex].pClient->connect(goProCams[camIndex].pDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
  goProCams[camIndex].pClient->setMTU(23); //set client to request maximum MTU from server (default is 23 otherwise)

  // Obtain a reference to the service we are after in the remote BLE server.
  BLERemoteService* pRemoteService = goProCams[camIndex].pClient->getService(controlServiceUUID);
  if (pRemoteService == nullptr) {
    Serial.print("[ERROR] Failed to find our service UUID: ");
    Serial.println(controlServiceUUID.toString().c_str());
    goProCams[camIndex].pClient->disconnect();
    goProCams[camIndex].pClient = nullptr;
    return false;
  }

  // Obtain a reference to the characteristic in the service of the remote BLE server.
  goProCams[camIndex].pCommandCharacteristic = goProCams[camIndex].pClient->getService(controlServiceUUID)->getCharacteristic(commandUUID);
  goProCams[camIndex].pCommandRespCharacteristic = goProCams[camIndex].pClient->getService(controlServiceUUID)->getCharacteristic(commandRespUUID);
  goProCams[camIndex].pSettingsCharacteristic = goProCams[camIndex].pClient->getService(controlServiceUUID)->getCharacteristic(settingsUUID);
  goProCams[camIndex].pSettingsRespCharacteristic = goProCams[camIndex].pClient->getService(controlServiceUUID)->getCharacteristic(settingsRespUUID);

  if (goProCams[camIndex].pCommandCharacteristic == nullptr ||
      goProCams[camIndex].pCommandRespCharacteristic == nullptr ||
      goProCams[camIndex].pSettingsCharacteristic == nullptr ||
      goProCams[camIndex].pSettingsRespCharacteristic == nullptr ||
      !goProCams[camIndex].pCommandRespCharacteristic->canNotify() ||
      !goProCams[camIndex].pSettingsRespCharacteristic->canNotify()) {
    Serial.print("[ERROR] Failed to find a characteristic UUID");
    goProCams[camIndex].pClient->disconnect();
    goProCams[camIndex].pClient = nullptr;
    return false;
  }

  // Read the value of the characteristic.
  //  if (goProCams[camIndex].pCommandCharacteristic->canRead()) {
  //    std::string value = goProCams[camIndex].pCommandCharacteristic->readValue();
  //    Serial.print("The characteristic value was: ");
  //    Serial.println(value.c_str());
  //  }

  // notify characteristics
  goProCams[camIndex].pCommandRespCharacteristic->registerForNotify(notifyCallback);
  goProCams[camIndex].pSettingsRespCharacteristic->registerForNotify(notifyCallback);

  goProCams[camIndex].lastKeepAlive = millis();

  Serial.printf("[connectToCam] successfuly connected %s\n", goProCams[camIndex].pDevice->getAddress().toString().c_str());

  return true;
}

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(controlServiceUUID)) {
        //check if cam is already connected
        for (uint8_t i = 0; i < maxCams; i++) {
          if (goProCams[i].pDevice != nullptr) {
            if (goProCams[i].pDevice->getAddress().equals(advertisedDevice.getAddress()) && goProCams[i].isConnected) {  // TODO Check if this works
              //already connected
              // Serial.print("[onAdvertiseResult] already connected-> ");
              // Serial.println(advertisedDevice.toString().c_str());
              return;
            }
          }
        }

        for (uint8_t i = 0; i < maxCams; i++) {
          if (goProCams[i].pDevice == nullptr) {
            goProCams[i].pDevice = new BLEAdvertisedDevice(advertisedDevice);
            goProCams[i].toConnect = true;
            Serial.print("[onAdvertiseResult] toConnect-> ");
            Serial.println(advertisedDevice.toString().c_str());
            break;
          }
        }

      }
      //      else if (!advertisedDevice.isAdvertisingService(controlServiceUUID)) {
      //        Serial.print("[onAdvertiseResult] Advertised device ");
      //        Serial.print(advertisedDevice.getAddress().toString().c_str());
      //        Serial.println(" is not advertising service FEA6 and will not be connected!");
      //      }
    }
};

void setup() {
  Serial.begin(115200);
  
  Serial.println("Starting WiFI");
  // init WIFI
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  if (MDNS.begin(DomainName)) {
    Serial.println("MDNS responder started");
    MDNS.addService("http", "tcp", 80);
  }
  
  Serial.println("Starting HTTP server");
  // init webServer
  // Route for root / web page
  server.on("/", HTTP_GET, [](){
    server.send(200, "text/html", index_html);
  });
  // Route for pair
  server.on("/pair", HTTP_GET, [](){
    server.send(200, "text/plain", "OK");
    Serial.println("Satrting BLEDevice scan");
    BLEDevice::getScan()->start(scanSeconds);
    Serial.println("BLEDevice scan started");
  });
  // Route for shutterOn
  server.on("/shutterOn", HTTP_GET, [](){
    server.send(200, "text/plain", "OK");
    for (uint8_t i = 0; i < maxCams; i++) {
      if (goProCams[i].pDevice != nullptr) {
        if (goProCams[i].isConnected) {
          goProCams[i].pCommandCharacteristic->writeValue(shutterOn, sizeof(shutterOn), false);
        }
      }
    }
  });
  // Route for shutterOff
  server.on("/shutterOff", HTTP_GET, [](){
    server.send(200, "text/plain", "OK");
    for (uint8_t i = 0; i < maxCams; i++) {
      if (goProCams[i].pDevice != nullptr) {
        if (goProCams[i].isConnected) {
          goProCams[i].pCommandCharacteristic->writeValue(shutterOff, sizeof(shutterOff), false);
        }
      }
    }
  });
  // Route for list
  server.on("/list", HTTP_GET, [](){
    String camList = "";
    for (uint8_t i = 0; i < maxCams; i++) {
      if (goProCams[i].pDevice != nullptr) {
        if (goProCams[i].isConnected) {
          camList += goProCams[i].pDevice->getName().c_str();
          camList += "  [";
          camList += goProCams[i].pDevice->getAddress().toString().c_str();
          camList += "]\n";
        }
      }
    }
    server.send(200, "text/plain", camList.c_str());
  });  
  server.begin();

  Serial.println("Starting Arduino BLE Client application...");
  BLEDevice::init("GoPro RC");
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->start(scanSeconds);
}

// This is the Arduino main loop function.
void loop() {
  BLEDevice::getScan()->stop();
  server.handleClient();
  delay(2);//allow the cpu to switch to other tasks
  
  // If the flag "toConnect" is true then we have scanned for and found the desired
  // BLE Server with which we wish to connect.  Now we connect to it.  Once we are
  // connected we set the isConnected flag to be true.
  bool someConnected = false;

  for (uint8_t i = 0; i < maxCams; i++) {
    if (goProCams[i].pDevice != nullptr) {
      if (goProCams[i].toConnect) {
        BLEDevice::getScan()->stop();

        if (connectToCam(i)) {
          goProCams[i].isConnected = true;
          someConnected = true;
        } else {
          Serial.printf("[connectToCam] connecting failed %s\n", goProCams[i].pDevice->getAddress().toString().c_str());
        }
        goProCams[i].toConnect = false;

        //BLEDevice::getScan()->start(0);
      } else if (goProCams[i].isConnected) {
        someConnected = true;
      }

      // It is recommended to send a keep-alive at least once every 120 seconds. We choose 60 seconds
      if (goProCams[i].isConnected && millis() - goProCams[i].lastKeepAlive > 60000) {
        //goProCams[i].pSettingsCharacteristic->writeValue(keepAlive, sizeof(keepAlive), false); //writeValue(uint8_t *data, size_t length, bool response)
        goProCams[i].lastKeepAlive = millis();
        Serial.printf("[keepAlive] was sent to Cam %d\n", i);

        //FOR DEBUG ONLY
        //        goProCams[i].pCommandCharacteristic->writeValue(shutterOn, sizeof(shutterOn), false);
        //        delay(5000);
        //        goProCams[i].pCommandCharacteristic->writeValue(shutterOff, sizeof(shutterOn), false);
      }
    }
  }

//  if (!someConnected) {
//    //No cam is connected, start search again
//    BLEDevice::getScan()->start(scanSeconds);
//  }
}
