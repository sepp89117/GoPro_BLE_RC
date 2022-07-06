# GoPro_BLE_RC
Control multiple GoPros

Due to the low memory, only a few cameras can be connected.
Better solution is here: https://github.com/sepp89117/GoPro_Web_RC

<img src="https://github.com/sepp89117/GoPro_BLE_RC/blob/main/webserver.png">

<b>Steps to use</b><br>
Set:
- Board: ESP32 Dev Module
- Flash Size: 4MB
- Partition Scheme: Huge App

Upload sketch

Connect PC or Mobile to ESP WiFi<br>
SSID: GoPro_RC<br>
Password: 00000000

Open http://192.168.4.1 in Browser

Put your Cam in pairing mode (remote) for first time connecting

Klick "Connect/Pair new" in browser

If your device is listed after several seconds, you can start and stop the recording

TIP: The serial monitor shows a lot of information at the same time

TODO:
- find out how many cameras it works with at the same time
  - Max tested is 3 (2022-06-24)
- find out with which models the code works
  - Hero 7, 8, 10 tested ok (2022-06-24)

