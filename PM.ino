#include <Arduino.h>
//#include <SPI.h> // needed for Arduino versions later than 0018
#include <Ethernet.h>
#include <EthernetUdp.h>

byte mac[] = {0xFE, 0xED, 0xDE, 0xAD, 0xBE, 0xE0};
unsigned int port = 8888;
EthernetUDP Udp;
IPAddress bc(255, 255, 255, 255);

#define LENG 31 // 0x42 + 31 bytes equal to 32 bytes
unsigned char buf[LENG];

int PM01Value = 0;
int PM2_5Value = 0;
int PM10Value = 0;

void setup() {
  Serial.begin(9600); // use serial0  start the Ethernet and UDP:
  if (Ethernet.begin(mac) == 0) {
    Serial.print("Static IP: ");
    IPAddress ip(192, 168, 0, 204);
    IPAddress gateway(192, 168, 0, 1);
    IPAddress subnet(255, 255, 255, 0);
    Ethernet.begin(mac, ip, gateway, subnet);
  } else {
    Serial.print("DHCP IP: ");
  }
  Serial.println(Ethernet.localIP());

  Udp.begin(port);

  IPAddress remote = Udp.remoteIP();
  for (int i = 0; i < 4; i++) {
    Serial.print(remote[i], DEC);
    if (i < 3) {
      Serial.print(".");
    }
  }
  // set the Timeout to 1500ms, longer than the data transmission periodic time
  // of the sensor
  Serial.setTimeout(1500);
}

void loop() {
  sniff();
  sneeze();
}

void sniff() {
  if (Serial.find(0x42)) { // start to read when detect 0x42
    Serial.readBytes(buf, LENG);
    if (buf[0] == 0x4d) {
      if (checkValue(buf, LENG)) {
        PM01Value = transmitPM01(buf);
        PM2_5Value = transmitPM2_5(buf);
        PM10Value = transmitPM10(buf);
      }
    }
  }

  static unsigned long OledTimer = millis();
  if (millis() - OledTimer >= 1000) {
    OledTimer = millis();
    Serial.print(PM01Value);
    Serial.print(",");
    Serial.print(PM2_5Value);
    Serial.print(",");
    Serial.println(PM10Value);
    Serial.println();
  }
}

void sneeze() {
  Udp.beginPacket(bc, port);
  Udp.print(PM01Value);
  Udp.print(",");
  Udp.print(PM2_5Value);
  Udp.print(",");
  Udp.print(PM10Value);
  Udp.endPacket();
  delay(10);
}

char checkValue(unsigned char *thebuf, char leng) {
  char receiveflag = 0;
  int receiveSum = 0;

  for (int i = 0; i < (leng - 2); i++) {
    receiveSum = receiveSum + thebuf[i];
  }
  receiveSum = receiveSum + 0x42;

  if (receiveSum == ((thebuf[leng - 2] << 8) + thebuf[leng - 1])) {
    receiveSum = 0; //?
    receiveflag = 1;
  }
  return receiveflag;
}

int transmitPM01(unsigned char *thebuf) {
  int PM01Val;
  PM01Val = ((thebuf[3] << 8) + thebuf[4]);
  return PM01Val;
}

// transmit PM Value to PC
int transmitPM2_5(unsigned char *thebuf) {
  int PM2_5Val;
  PM2_5Val = ((thebuf[5] << 8) + thebuf[6]);
  return PM2_5Val;
}

// transmit PM Value to PC
int transmitPM10(unsigned char *thebuf) {
  int PM10Val;
  PM10Val = ((thebuf[7] << 8) + thebuf[8]);
  return PM10Val;
}

int transmit(unsigned char *thebuf, int a, int b, int c) {
  return ((thebuf[a] << b) + thebuf[c]);
}
