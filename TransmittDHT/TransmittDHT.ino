/*
  Lora Node2
  The IoT Projects

*/
#include <SPI.h>              // include libraries
#include <LoRa.h>
//#include <DHT.h>

#include <dht.h>        // Include library
#define outPin 8        // Defines pin number to which the sensor is connected
#define MQ2pin (0)

dht DHT;

//#define DHTPIN 5          //pin where the dht22 is connected
//DHT dht(DHTPIN, DHT11);
#define ss 10
#define rst 9
#define dio0 2

String outgoing;              // outgoing message

byte msgCount = 0;            // count of outgoing messages
byte MasterNode = 0xFF;
byte Node2 = 0xCC;
int buttonPin = 4;

float sensorValue;  //variable to store sensor value
float temperature;
float humidity;
int button;

String Mymessage = "";
String incoming = "";

void setup() {
  Serial.begin(9600);                   // initialize serial
  //dht.begin();
  pinMode(buttonPin, INPUT);
  Serial.println("Gas sensor warming up!");
  delay(20000); // allow the MQ-2 to warm up

  while (!Serial);

  Serial.println("LoRa Node2");

  LoRa.setPins(ss, rst, dio0);

  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
}

void loop() {
  int buttonState = digitalRead(buttonPin);
  int readData = DHT.read11(outPin);

	temperature = DHT.temperature;        // Read temperature
	humidity = DHT.humidity;           // Read humidity
  temperature = (temperature*9.0)/5.0+32.0;
  // temperature = dht.readTemperature();
  // humidity = dht.readHumidity();
  Serial.print((temperature*9.0)/5.0+32.0);
  Serial.println(humidity);

  sensorValue = analogRead(MQ2pin);

  // Button is pressed (HIGH state)
      if (buttonState == HIGH) {
      button=1;
    } 
    // Button is not pressed (LOW state)
    else {
      button=0;
    }
  // parse for a packet, and call onReceive with the result:
  onReceive(LoRa.parsePacket());
}

void onReceive(int packetSize) {
  if (packetSize == 0) return;          // if there's no packet, return

  // read packet header bytes:
  int recipient = LoRa.read();          // recipient address
  byte sender = LoRa.read();            // sender address
  byte incomingMsgId = LoRa.read();     // incoming msg ID
  byte incomingLength = LoRa.read();    // incoming msg length

  String incoming = "";

  while (LoRa.available()) {
    incoming += (char)LoRa.read();
  }

  if (incomingLength != incoming.length()) {   // check length for error
    // Serial.println("error: message length does not match length");
    ;
    return;                             // skip rest of function
  }

  // if the recipient isn't this device or broadcast,
  if (recipient != Node2 && recipient != MasterNode) {
    //Serial.println("This message is not for me.");
    ;
    return;                             // skip rest of function
  }
  Serial.println(incoming);
  int Val = incoming.toInt();
  if (Val == 20)
  {
    Mymessage = Mymessage + temperature + "," + humidity+","+sensorValue+","+button;
    sendMessage(Mymessage, MasterNode, Node2);
    delay(100);
    Mymessage = "";
  }

}

void sendMessage(String outgoing, byte MasterNode, byte Node2) {
  LoRa.beginPacket();                   // start packet
  LoRa.write(MasterNode);              // add destination address
  LoRa.write(Node2);             // add sender address
  LoRa.write(msgCount);                 // add message ID
  LoRa.write(outgoing.length());        // add payload length
  LoRa.print(outgoing);                 // add payload
  LoRa.endPacket();                     // finish packet and send it
  msgCount++;                           // increment message ID
}