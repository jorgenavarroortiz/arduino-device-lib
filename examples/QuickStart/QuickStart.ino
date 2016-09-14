#include <TheThingsNetwork.h>

// Set your AppEUI and AppKey
const byte appEui[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
const byte appKey[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

#define debugSerial Serial
#define loraSerial Serial1

#define debugPrintLn(...) { if (debugSerial) debugSerial.println(__VA_ARGS__); }
#define debugPrint(...) { if (debugSerial) debugSerial.print(__VA_ARGS__); }

TheThingsNetwork ttn;

void setup() {
  debugSerial.begin(9600);
  loraSerial.begin(57600);

  // Wait a maximum of 10s for Serial Monitor
  uint32_t timeout = millis() + 10000;
  while (!debugSerial && millis() < timeout);

  ttn.init(loraSerial, debugSerial);

  debugPrintLn("-- RESET");
  ttn.reset();

  // Wait 10s between each attempt to get a confirmed activation
  debugPrintLn("-- JOIN");
  while(!ttn.join(appEui, appKey)){
    delay(10000);
  }

  debugPrintLn("-- STATUS");
  ttn.showStatus();
}

void loop() {
  debugPrintLn("-- LOOP");

  // Get uptime in ms (resets every 50 days)
  uint32_t uptime = millis();
  debugPrintLn("Uptime (ms): " + String(uptime));

  // Encode uptime into 4 bytes
  byte bytes[4];
  longToBytes(uptime, bytes);
  debugPrintLn("Uptime (bytes in hex): " + bytesToHexString(bytes, 4));

  // Send and check for incoming message size
  int downlinkSize = ttn.sendBytes(bytes, 4);

  if (downlinkSize > 0) {
    debugPrintLn("Downlink (bytes in hex): " + bytesToHexString(ttn.downlink, downlinkSize));
    debugPrintLn("Port: " + String(ttn.downlinkPort));

    // Only handle messages to port 1 with a size of 1 byte
    if (ttn.downlinkPort == 1 && downlinkSize == 1) {
  
      // Check the byte
      switch (ttn.downlink[0]) {
        case 0x00:
          debugPrintLn("LED: off");
          digitalWrite(LED_BUILTIN, LOW);
          break;
        case 0x01:
          debugPrintLn("LED: on");
          digitalWrite(LED_BUILTIN, HIGH);
          break;
      }
    }
  }
  
  delay(10000);
}

void longToBytes(long lng, byte bytes[]) {          
  bytes[0] = (int) ((lng & 0xFF000000) >> 24 );
  bytes[1] = (int) ((lng & 0x00FF0000) >> 16 );
  bytes[2] = (int) ((lng & 0x0000FF00) >> 8  );
  bytes[3] = (int) ((lng & 0X000000FF)       );
}

String bytesToHexString(const byte* bytes, int length) {
  String str = "";
  for (int i = 0; i < length; i++) {
    if (i > 0) {
      str += " ";
    }
    String hex = String(bytes[i], HEX);
    if (hex.length() != 2) {
      str += "0";
    }
    str += hex;
  }
  str.toUpperCase();
  return str;
}