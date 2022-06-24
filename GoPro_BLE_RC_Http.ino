/*
  Board: ESP32 Dev Module
  Flash Size: 4MB
  Partition Scheme: Huge App
  ESP Version 1.0.6 works / 2.0.3 not
*/
// WiFi
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>

// BLE
#include "BLEDevice.h"

/* Make your personal settings here */
#define DEBUG                     // a lot of serial output
const char *DomainName = "gopro"; // set domain name "domain.local" -> why not working?
const char *ssid = "GoPro_RC";
const char *password = "00000000";
/* Don't change the rest if you don't know what you're doing */

WebServer server(80);

// HTML minified by "https://www.willpeavy.com/tools/minifier/"
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head> <title>GoPro BLE RC</title> <meta name="viewport" content="width=device-width, initial-scale=1"> <style>/* colors from dark to light #1c1c1c dark gray #4d4d4d gray #005eab blue #009fe0 light blue #fff wihte */ html{font-family: Segoe UI, Frutiger, Frutiger Linotype, Dejavu Sans, Helvetica Neue, Arial, sans-serif; display: inline-block; text-align: center; background-color: #1c1c1c; color: #fff;}input{background-color: #009fe0; color: #fff; border: none; border-radius: 5px; padding: 10px; margin: 10px 0; font-weight: bold;}input:hover{background-color: #005eab;}legend{margin: 0;}fieldset{display: flex; border-radius: 5px; justify-content: space-around; margin: 0 0 10px 0;}.h2{font-size: 3.0rem;}p{font-size: 1.0rem;}body{max-width: 600px; margin: 0px auto;}fieldset.foldable{height: 0px; overflow: hidden;}fieldset.foldable.expanded{height: auto;}fieldset.foldable>legend::after{content: "+"; margin: 0px 5px 0px 15px; padding: 0px 8px 5px 8px; background-color: #009fe0; border-radius: 5px; cursor: default;}fieldset.foldable.expanded>legend::after{content: "-"; margin: 0px 5px 0px 15px; padding: 0px 8px 5px 8px; background-color: #009fe0; border-radius: 5px; cursor: default;}.styled-table{border-collapse: collapse; margin: 10px 0; font-size: 0.9em; min-width: 569px;}.styled-table td{padding: 5px 0;}.styled-table thead tr{background-color: #005eab; color: #ffffff;}.styled-table tbody tr{border-bottom: 1px solid #dddddd; background-color: #4d4d4d;}.styled-table tbody tr:nth-of-type(even){background-color: #1c1c1c;}.styled-table tbody tr:last-of-type{border-bottom: 2px solid #005eab;}</style></head><body> <b class="h2">GoPro RC</b><input type="button" onclick="getReq('/pair')" value="Connect/Pair new" style="margin: 0px 0px 0px 52px;top: 15px;position: fixed;"> <span id="status" style="display: block;height: 22px;"></span> <fieldset> <legend><b>Control</b></legend> <div style="display:inline-block;"> <input type="button" onclick="getReq('/shutterOn')" value="Shutter: on"> <input type="button" onclick="getReq('/shutterOff')" value="Shutter: off"><br><input type="button" onclick="getReq('/camSleep')" value="Put camera to sleep"> <input type="button" onclick="getReq('/wiFiAPoff')" value="WiFi AP: off"> <input type="button" onclick="getReq('/wiFiAPon')" value="WiFi AP: on"> <input type="button" onclick="getReq('/hilight')" value="Hilight moment"> </div></fieldset> <fieldset class="foldable expanded"> <legend onclick="this.parentNode.classList.toggle('expanded');"><b>Load Presets</b></legend> <div style="display:inline-block;"> <input type="button" onclick="getReq('/videoGroup')" value="Video Group"> <input type="button" onclick="getReq('/photoGroup')" value="Photo Group"> <input type="button" onclick="getReq('/timelapseGroup')" value="Timelapse Group"><br><input type="button" onclick="getReq('/standard')" value="Standard"> <input type="button" onclick="getReq('/activity')" value="Activity"> <input type="button" onclick="getReq('/cinematic')" value="Cinematic"> <input type="button" onclick="getReq('/photo')" value="Photo"> <input type="button" onclick="getReq('/liveBurst')" value="Live Burst"> <input type="button" onclick="getReq('/burstPhoto')" value="Burst Photo"> <input type="button" onclick="getReq('/nightPhoto')" value="Night Photo"> <input type="button" onclick="getReq('/timeWarp')" value="Time Warp"> <input type="button" onclick="getReq('/timeLapse')" value="Time Lapse"> <input type="button" onclick="getReq('/nightLapse')" value="Night Lapse"> </div></fieldset> <fieldset class="foldable expanded"> <legend onclick="this.parentNode.classList.toggle('expanded');"><b>Connected devices</b></legend> <table id="dev_list" class="styled-table"> <thead> <tr> <th>No</th> <th>Name</th> <th>Model</th> <th>Mode</th> <th>Battery</th> <th>RSSI</th> <th>MAC</th> <th title="Last Command">LC</th> <th title="Last Error">LE</th> </tr></thead> <tbody></tbody> </table> </fieldset> <script>var conDevs=[]; var dev_list_body=document.getElementById("dev_list").tBodies[0]; window.addEventListener("DOMContentLoaded", function (){getList();}); function getReq(req){var xhr=new XMLHttpRequest(); xhr.onreadystatechange=function (){if (this.readyState==4 && this.status==200){document.getElementById("status").innerText="Response: '" + this.responseText + "'"; window.setTimeout(function (){document.getElementById("status").innerText="";}, 2000);}}; xhr.open("GET", req, true); xhr.send(); document.getElementById("status").innerText="Request was sent! Please wait...";}window.setInterval(function (){getList();}, 5000); function getList(){var xhttp=new XMLHttpRequest(); xhttp.onreadystatechange=function (){if (this.readyState==4 && this.status==200){conDevs=JSON.parse(this.responseText); var tBody=""; for (var i=0; i < conDevs.length; i++){tBody +="<tr><td>" + (i + 1) + "</td><td>" + conDevs[i].Name + "</td><td>" + conDevs[i].Model + "</td><td>" + conDevs[i].Mode + "</td><td>" + conDevs[i].Battery + "</td><td>" + conDevs[i].RSSI + "</td><td>" + conDevs[i].MAC + "</td><td>" + conDevs[i].LC + "</td><td>" + conDevs[i].LE + "</td></tr>\n";}dev_list_body.innerHTML=tBody;}}; xhttp.open("GET", "/list", true); xhttp.send();}</script></body></html>)rawliteral";

// The remote service we wish to connect to.
static PROGMEM BLEUUID controlServiceUUID("0000FEA6-0000-1000-8000-00805F9B34FB"); // Cam control service

// The characteristic of the remote service we are interested in.
static PROGMEM BLEUUID commandUUID("B5F90072-AA8D-11E3-9046-0002A5D5C51B");      // Command [WRITE]
static PROGMEM BLEUUID commandRespUUID("B5F90073-AA8D-11E3-9046-0002A5D5C51B");  // Command response [NOTIFY]
static PROGMEM BLEUUID settingsUUID("B5F90074-AA8D-11E3-9046-0002A5D5C51B");     // Settings [WRITE]
static PROGMEM BLEUUID settingsRespUUID("B5F90075-AA8D-11E3-9046-0002A5D5C51B"); // Settings response [NOTIFY]

struct GoProCam
{
  BLEAdvertisedDevice *pDevice = nullptr;
  BLEClient *pClient = nullptr;
  bool toConnect = false;
  bool isAuth = false;
  bool isConnected = false;
  uint32_t lastKeepAlive = 0;
  uint8_t lastCommand = 0;
  uint8_t lastError = 0;
  BLERemoteCharacteristic *pCommandCharacteristic = nullptr;
  BLERemoteCharacteristic *pCommandRespCharacteristic = nullptr;
  BLERemoteCharacteristic *pSettingsCharacteristic = nullptr;
  BLERemoteCharacteristic *pSettingsRespCharacteristic = nullptr;
};

// Try to provide 10 cams
const uint8_t maxCams = 10;
GoProCam goProCams[maxCams];
uint8_t scanSeconds = 5;

// The commands below are sent to pSettingsCharacteristic and responses/notifications are received on pSettingsRespCharacteristic
uint8_t keepAlive[] = {0x03, 0x5B, 0x01, 0x42}; // Only Hero 9 and 10 ? Hero 8 response is Error

// The commands below are sent to pCommandCharacteristic and responses/notifications are received on pCommandRespCharacteristic
uint8_t shutterOff[] = {0x03, 0x01, 0x01, 0x00};
uint8_t shutterOn[] = {0x03, 0x01, 0x01, 0x01};
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
class MySecurity : public BLESecurityCallbacks
{
  uint32_t onPassKeyRequest()
  {
#ifdef DEBUG
    Serial.println("[onPassKeyRequest]");
#endif
    return 123456;
  }
  void onPassKeyNotify(uint32_t pass_key)
  {
#ifdef DEBUG
    Serial.printf("[onPassKeyNotify] %d\n", pass_key);
#endif
  }
  bool onConfirmPIN(uint32_t pass_key)
  {
#ifdef DEBUG
    Serial.printf("[onConfirmPIN] %d\n", pass_key);
#endif
    vTaskDelay(5000);
    return true;
  }
  bool onSecurityRequest()
  {
#ifdef DEBUG
    Serial.println("[onSecurityRequest]");
#endif
    return true;
  }
  void onAuthenticationComplete(esp_ble_auth_cmpl_t auth_cmpl)
  {
#ifdef DEBUG
    Serial.printf("[onAuthenticationComplete] remote BD_ADDR: %02x:%02x:%02x:%02x:%02x:%02x, ", auth_cmpl.bd_addr[0], auth_cmpl.bd_addr[1], auth_cmpl.bd_addr[2], auth_cmpl.bd_addr[3], auth_cmpl.bd_addr[4], auth_cmpl.bd_addr[5]);
    Serial.printf("address type = %d - pairing %s\n", auth_cmpl.addr_type, auth_cmpl.success ? "success" : "fail");
#endif
    for (uint8_t i = 0; i < maxCams; i++)
    {
      if (goProCams[i].pDevice != nullptr)
      {
        if (goProCams[i].pDevice->getAddress().equals(auth_cmpl.bd_addr))
        {
          goProCams[i].isAuth = auth_cmpl.success;
        }
      }
    }
  }
};

static void notifyCallback(BLERemoteCharacteristic *pBLERemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify)
{
  // TODO This only works with response length of 3, see comments below
  uint8_t respCommand = pData[length - 2]; // position depends on header length
  uint8_t respError = pData[length - 1];   // position depends on msg length

  BLEClient *sender = pBLERemoteCharacteristic->getRemoteService()->getClient();
  for (uint8_t i = 0; i < maxCams; i++)
  {
    if (goProCams[i].pClient != nullptr)
    {
      if (goProCams[i].pClient->getPeerAddress().equals(sender->getPeerAddress()) && goProCams[i].lastCommand == respCommand)
        goProCams[i].lastError = respError;
    }
  }

#ifdef DEBUG
  Serial.printf("[notifyCallback] Command 0x%02X", respCommand);

  switch (respError)
  {
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

  Serial.print("data: {");
  for (uint8_t i = 0; i < length; i++)
  {
    Serial.printf("0x%02X", pData[i]);
    if (i < length - 1)
      Serial.print(", ");
  }
  Serial.println("}");
#endif
}

class MyClientCallback : public BLEClientCallbacks
{
  void onConnect(BLEClient *pClient)
  {
    // Serial.printf("[onConnect] %s\n", pClient->getPeerAddress().toString().c_str());
  }

  void onDisconnect(BLEClient *pClient)
  {
    for (uint8_t i = 0; i < maxCams; i++)
    {
      if (goProCams[i].pClient != nullptr)
      {
        if (goProCams[i].pClient->getPeerAddress().equals(pClient->getPeerAddress()))
        { // TODO Check if this works
          goProCams[i].isConnected = false;
          goProCams[i].pClient = nullptr;
          goProCams[i].pDevice = nullptr;
#ifdef DEBUG
          Serial.printf("[onDisconnect] %s\n", pClient->getPeerAddress().toString().c_str());
#endif
          return;
        }
      }
    }
  }
};

bool connectToCam(uint8_t camIndex)
{
#ifdef DEBUG
  Serial.printf("[connectToCam] connecting %s ...\n", goProCams[camIndex].pDevice->getAddress().toString().c_str());
#endif
  // Set security
  BLEDevice::setEncryptionLevel(ESP_BLE_SEC_ENCRYPT);
  BLEDevice::setSecurityCallbacks(new MySecurity());
  BLESecurity *pSecurity = new BLESecurity();
  pSecurity->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_BOND); // ESP_LE_AUTH_BOND, ESP_LE_AUTH_REQ_SC_ONLY
  pSecurity->setCapability(ESP_IO_CAP_NONE);                 // ESP_IO_CAP_OUT
  pSecurity->setRespEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);

#ifdef DEBUG
  Serial.printf("[connectToCam] createClient\n");
#endif

  // Create client
  goProCams[camIndex].pClient = BLEDevice::createClient();
  goProCams[camIndex].pClient->setClientCallbacks(new MyClientCallback());

#ifdef DEBUG
  Serial.printf("[connectToCam] Connect to the remote BLE Server\n");
#endif

  // Connect to the remote BLE Server. <- this requests authentication
  if (!goProCams[camIndex].pClient->connect(goProCams[camIndex].pDevice))
  {
#ifdef DEBUG
    Serial.printf("[connectToCam] pClient->connect failed\n");
#endif
    return false;
  }
#ifdef DEBUG
  Serial.printf("[connectToCam] wait for authentication\n");
#endif

  // wait for authentication -> timeout is 5 seconds (< 2 s measured)
  uint32_t startWait = millis();
  while (!goProCams[camIndex].isAuth)
  {
    if (millis() - startWait > 5000)
    {
#ifdef DEBUG
      Serial.printf("[connectToCam] no authentication response\n");
#endif
      return false;
    }
  }

#ifdef DEBUG
  Serial.printf("[connectToCam] get pRemoteService\n");
#endif

  // Obtain a reference to the service we are after in the remote BLE server.
  //[BUG] Stack canary watchpoint triggered (btController) with ESP32 Version 2.0.3
  BLERemoteService *pRemoteService = goProCams[camIndex].pClient->getService(controlServiceUUID);

#ifdef DEBUG
  Serial.printf("[connectToCam] check pRemoteService\n");
#endif

  if (pRemoteService == nullptr)
  {
#ifdef DEBUG
    Serial.print("[ERROR] Failed to find our service UUID: ");
    Serial.println(controlServiceUUID.toString().c_str());
#endif
    return false;
  }

#ifdef DEBUG
  Serial.printf("[connectToCam] get Characteristics\n");
#endif

  // Obtain a reference to the characteristic in the service of the remote BLE server.
  goProCams[camIndex].pCommandCharacteristic = goProCams[camIndex].pClient->getService(controlServiceUUID)->getCharacteristic(commandUUID);
  goProCams[camIndex].pCommandRespCharacteristic = goProCams[camIndex].pClient->getService(controlServiceUUID)->getCharacteristic(commandRespUUID);
  goProCams[camIndex].pSettingsCharacteristic = goProCams[camIndex].pClient->getService(controlServiceUUID)->getCharacteristic(settingsUUID);
  goProCams[camIndex].pSettingsRespCharacteristic = goProCams[camIndex].pClient->getService(controlServiceUUID)->getCharacteristic(settingsRespUUID);
#ifdef DEBUG
  Serial.printf("[connectToCam] check Characteristics\n");
#endif
  if (goProCams[camIndex].pCommandCharacteristic == nullptr ||
      goProCams[camIndex].pCommandRespCharacteristic == nullptr ||
      goProCams[camIndex].pSettingsCharacteristic == nullptr ||
      goProCams[camIndex].pSettingsRespCharacteristic == nullptr)
  {
#ifdef DEBUG
    Serial.print("[ERROR] Failed to find a characteristic UUID");
#endif
    return false;
  }
#ifdef DEBUG
  Serial.printf("[connectToCam] check Characteristics canNotify\n");
#endif
  if (!goProCams[camIndex].pCommandRespCharacteristic->canNotify() ||
      !goProCams[camIndex].pSettingsRespCharacteristic->canNotify())
  {
#ifdef DEBUG
    Serial.print("[ERROR] Failed to find a characteristic UUID");
#endif
    return false;
  }
#ifdef DEBUG
  Serial.printf("[connectToCam] Notify Characteristics\n");
#endif
  // notify characteristics
  goProCams[camIndex].pCommandRespCharacteristic->registerForNotify(notifyCallback);
  goProCams[camIndex].pSettingsRespCharacteristic->registerForNotify(notifyCallback);

  goProCams[camIndex].lastKeepAlive = millis();
#ifdef DEBUG
  Serial.printf("[connectToCam] successfuly connected %s\n", goProCams[camIndex].pDevice->getAddress().toString().c_str());
#endif
  return true;
}

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
  void onResult(BLEAdvertisedDevice advertisedDevice)
  {
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(controlServiceUUID))
    {
      // check if cam is already connected
      for (uint8_t i = 0; i < maxCams; i++)
      {
        if (goProCams[i].pDevice != nullptr)
        {
          if (goProCams[i].pDevice->getAddress().equals(advertisedDevice.getAddress()))
          {
            // already connected
            return;
          }
        }
      }
      bool spaceEnough = false;
      for (uint8_t i = 0; i < maxCams; i++)
      {
        if (goProCams[i].pDevice == nullptr)
        {
          goProCams[i].pDevice = new BLEAdvertisedDevice(advertisedDevice);
          goProCams[i].toConnect = true;
          spaceEnough = true;
#ifdef DEBUG
          Serial.printf("[onAdvertiseResult] %d toConnect-> ", i);
          Serial.println(advertisedDevice.toString().c_str());
#endif
          break;
        }
      }
#ifdef DEBUG
      if (!spaceEnough)
        Serial.println("[maxCams overflow] Last camera could not be connected!");
#endif
    }
#ifdef DEBUG
    else if (!advertisedDevice.isAdvertisingService(controlServiceUUID))
    {
      Serial.print("[onAdvertiseResult] Advertised device ");
      Serial.print(advertisedDevice.getAddress().toString().c_str());
      Serial.println(" is not advertising service FEA6 and will not be connected! Is it a GoPro? Tell the author!");
    }
#endif
  }
};

void sendCommand(uint8_t *command)
{
  for (uint8_t i = 0; i < maxCams; i++)
  {
    if (goProCams[i].pDevice != nullptr)
    {
      if (goProCams[i].isConnected)
      {
        goProCams[i].pCommandCharacteristic->writeValue(command, sizeof(shutterOn), false);
        goProCams[i].lastCommand = command[1];
        goProCams[i].lastError = 3; // 3 = outstanding
      }
    }
  }
}

void initServer()
{
  server.on("/", HTTP_GET, []()
            { server.send(200, "text/html", index_html); });

  server.on("/pair", HTTP_GET, []()
            {
    Serial.println("Start BLEDevice scan");
    BLEDevice::getScan()->start(scanSeconds);
    server.send(200, "text/plain", "OK"); });

  server.on("/shutterOn", HTTP_GET, []()
            {
    server.send(200, "text/plain", "OK");
    sendCommand(shutterOn); });

  server.on("/shutterOff", HTTP_GET, []()
            {
    server.send(200, "text/plain", "OK");
    sendCommand(shutterOff); });

  server.on("/camSleep", HTTP_GET, []()
            {
    server.send(200, "text/plain", "OK");
    sendCommand(camSleep); });

  server.on("/wiFiAPoff", HTTP_GET, []()
            {
    server.send(200, "text/plain", "OK");
    sendCommand(wiFiAPoff); });

  server.on("/wiFiAPon", HTTP_GET, []()
            {
    server.send(200, "text/plain", "OK");
    sendCommand(wiFiAPon); });

  server.on("/hilight", HTTP_GET, []()
            {
    server.send(200, "text/plain", "OK");
    sendCommand(hilight); });

  server.on("/videoGroup", HTTP_GET, []()
            {
    server.send(200, "text/plain", "OK");
    sendCommand(videoGroup); });

  server.on("/photoGroup", HTTP_GET, []()
            {
    server.send(200, "text/plain", "OK");
    sendCommand(photoGroup); });

  server.on("/timelapseGroup", HTTP_GET, []()
            {
    server.send(200, "text/plain", "OK");
    sendCommand(timelapseGroup); });

  server.on("/standard", HTTP_GET, []()
            {
    server.send(200, "text/plain", "OK");
    sendCommand(standard); });

  server.on("/activity", HTTP_GET, []()
            {
    server.send(200, "text/plain", "OK");
    sendCommand(activity); });

  server.on("/cinematic", HTTP_GET, []()
            {
    server.send(200, "text/plain", "OK");
    sendCommand(cinematic); });

  server.on("/photo", HTTP_GET, []()
            {
    server.send(200, "text/plain", "OK");
    sendCommand(photo); });

  server.on("/liveBurst", HTTP_GET, []()
            {
    server.send(200, "text/plain", "OK");
    sendCommand(liveBurst); });

  server.on("/burstPhoto", HTTP_GET, []()
            {
    server.send(200, "text/plain", "OK");
    sendCommand(burstPhoto); });

  server.on("/nightPhoto", HTTP_GET, []()
            {
    server.send(200, "text/plain", "OK");
    sendCommand(nightPhoto); });

  server.on("/timeWarp", HTTP_GET, []()
            {
    server.send(200, "text/plain", "OK");
    sendCommand(timeWarp); });

  server.on("/timeLapse", HTTP_GET, []()
            {
    server.send(200, "text/plain", "OK");
    sendCommand(timeLapse); });

  server.on("/nightLapse", HTTP_GET, []()
            {
    server.send(200, "text/plain", "OK");
    sendCommand(nightLapse); });

  server.onNotFound([]()
                    { server.send(200, "text/plain", "NA"); });

  // TODO
  server.on("/list", HTTP_GET, []()
            {
    String camList = "[";
    for (uint8_t i = 0; i < maxCams; i++) {
      if (goProCams[i].pDevice != nullptr) {
        if (goProCams[i].isConnected) {
          if(i>0) camList += ",{";
          else camList += "{";         
           
          camList += "\"Name\":\"";
          camList += goProCams[i].pDevice->getName().c_str();
          camList += "\",";
          
          camList += "\"Model\":\"";
          camList += "?"; // TODO
          camList += "\",";
          
          camList += "\"Mode\":\"";
          camList += "?"; // TODO
          camList += "\",";
          
          camList += "\"Battery\":\"";
          camList += "?"; // TODO
          camList += " %\",";
          
          camList += "\"RSSI\":\"";
          camList += goProCams[i].pDevice->getRSSI(); // TODO test
          camList += " dB\",";
          
          camList += "\"MAC\":\"";
          camList += goProCams[i].pDevice->getAddress().toString().c_str();
          camList += "\",";
          
          camList += "\"LC\":\"";
          camList += String(goProCams[i].lastCommand, HEX);
          camList += "\",";
          
          camList += "\"LE\":\"";
          camList += String(goProCams[i].lastError, HEX);  // TODO test
          camList += "\"";

          camList += "}";
        }
      }
    }
    camList += "]";
    
    server.send(200, "application/json; charset=UTF-8", camList.c_str()); });
  server.begin();
}

void setup()
{
  Serial.begin(115200);

  Serial.println("Starting WiFI");
  // init WIFI
  WiFi.softAP(ssid, password);
  Serial.print("GoPro RC IP-Address: ");
  Serial.println(WiFi.softAPIP());

  Serial.println("Starting HTTP server");
  initServer();

  Serial.println("Starting Arduino BLE Client application...");
  BLEDevice::init(ssid);
  BLEScan *pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->start(scanSeconds);
}

void loop()
{
  BLEDevice::getScan()->stop();
  server.handleClient();
  delay(2); // allow the cpu to switch to other tasks

  for (uint8_t i = 0; i < maxCams; i++)
  {
    if (goProCams[i].pDevice != nullptr)
    {
      if (goProCams[i].toConnect)
      {
        BLEDevice::getScan()->stop();

        if (connectToCam(i))
          goProCams[i].isConnected = true;
        else
        {
#ifdef DEBUG
          Serial.printf("[connectToCam] connecting failed %s\n", goProCams[i].pDevice->getAddress().toString().c_str());
#endif
          if (goProCams[i].pClient != nullptr)
          {
            if (goProCams[i].pClient->isConnected())
              goProCams[i].pClient->disconnect();
            goProCams[i].pClient = nullptr;
          }
        }
        goProCams[i].toConnect = false;
      }

      // It is recommended to send a keep-alive at least once every 120 seconds. We choose 60 seconds
      if (goProCams[i].isConnected && millis() - goProCams[i].lastKeepAlive > 60000)
      {
        goProCams[i].pSettingsCharacteristic->writeValue(keepAlive, sizeof(keepAlive), false); // writeValue(uint8_t *data, size_t length, bool response)
        goProCams[i].lastKeepAlive = millis();
#ifdef DEBUG
        Serial.printf("[keepAlive] was sent to Cam %d\n", i);
#endif
      }
    }
  }
}
