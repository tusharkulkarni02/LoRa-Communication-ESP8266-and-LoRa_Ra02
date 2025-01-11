#include <SPI.h>
#include <LoRa.h>
#include <DHT.h>

#define DHTPIN 5
DHT dht(DHTPIN, DHT22);
#define ss 10
#define rst 9
#define dio0 2

#define SMOKE_SENSOR A0
#define EMERGENCY_BUTTON 3

String outgoing;
byte msgCount = 0;
byte MasterNode = 0xFF;
byte Node2 = 0xCC;

float temperature;
float humidity;
int smokeLevel;

String Mymessage = "";
String incoming = "";

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(SMOKE_SENSOR, INPUT);
  pinMode(EMERGENCY_BUTTON, INPUT_PULLUP);

  while (!Serial);

  Serial.println("LoRa Node2");
  LoRa.setPins(ss, rst, dio0);

  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
}

void loop() {
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  smokeLevel = analogRead(SMOKE_SENSOR);
  bool emergencyAlert = digitalRead(EMERGENCY_BUTTON) == LOW;

  if (emergencyAlert) {
    sendMessage("Emergency Alert: " + String(temperature) + "C, " + String(humidity) + "%, Smoke Level: " + String(smokeLevel), MasterNode, Node2);
    delay(5000); // Wait for 5 seconds before sending another alert
  }

  onReceive(LoRa.parsePacket());
}

void onReceive(int packetSize) {
  if (packetSize == 0) return;

  int recipient = LoRa.read();
  byte sender = LoRa.read();
  byte incomingMsgId = LoRa.read();
  byte incomingLength = LoRa.read();

  String incoming = "";

  while (LoRa.available()) {
    incoming += (char)LoRa.read();
  }

  if (incomingLength != incoming.length()) {
    return;
  }

  if (recipient != Node2 && recipient != MasterNode) {
    return;
  }
  Serial.println(incoming);
  int Val = incoming.toInt();
  if (Val == 20) {
    Mymessage = Mymessage + temperature + "," + humidity + "," + smokeLevel;
    sendMessage(Mymessage, MasterNode, Node2);
    delay(100);
    Mymessage = "";
  }
}

void sendMessage(String outgoing, byte MasterNode, byte Node2) {
  LoRa.beginPacket();
  LoRa.write(MasterNode);
  LoRa.write(Node2);
  LoRa.write(msgCount);
  LoRa.write(outgoing.length());
  LoRa.print(outgoing);
  LoRa.endPacket();
  msgCount++;
}
