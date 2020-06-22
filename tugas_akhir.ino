
#include <Servo.h>
//RFID
#include <SPI.h>
#include <MFRC522.h>
#include "BluetoothSerial.h"
#include <MQTT.h>
#include <WiFi.h> 

const char* ssid = "smarthomeIOT";
const char* password = "joyjoyjoy";
unsigned long lastMillis = 0;
bool authorized = false;

const char* mqtt_server = "192.168.43.167";

#define SS_PIN 5
#define RST_PIN 13
#define LOCK 14

BluetoothSerial SerialBT;
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.
Servo myservo;
int pos = 0;

WiFiClient net;
MQTTClient client;

void setup() 
{
  Serial.begin(115200);   // Initiate a serial communication
  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522
  myservo.attach(16);
  Serial.println("Put your card to the reader...");
  Serial.println();
  pinMode(LOCK, OUTPUT);

  WiFi.begin(ssid, password);
  client.begin("192.168.43.167", net);
  client.onMessage(messageReceived);

  connect();

}
//connect
void connect() {
  Serial.print("checking wifi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  Serial.print("\nconnecting...");
  while (!client.connect("arduino", "try", "try")) {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("\nconnected!");

  client.subscribe("servo");
  // client.unsubscribe("/hello");
}

//message received
void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);
  
  if(authorized == true) {
    if(topic == "servo" && payload == "putar"){
    for (pos = 0; pos <= 180; pos += 1) { // goes from 0 degrees to 180 degrees
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
    }
  } else if(topic == "servo" && payload == "balik"){
    for (pos = 180; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
      myservo.write(pos);              // tell servo to go to position in variable 'pos'
      delay(15);                       // waits 15ms for the servo to reach the position
    }
  }
  }else{
    Serial.println("you're not authorized to do this, scan card first");
  }
  

  // Note: Do not use the client in the callback to publish, subscribe or
  // unsubscribe as it may cause deadlocks when other things arrive while
  // sending and receiving acknowledgments. Instead, change a global variable,
  // or push to a queue and handle it in the loop after calling `client.loop()`.
}

void loop() 
{
  client.loop();
  delay(10);

  if (!client.connected()) {
    connect();
  }
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) 
  {
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) 
  {
    return;
  }
  //Show UID on serial monitor
  Serial.print("UID tag :");
  String content= "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
     Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
     Serial.print(mfrc522.uid.uidByte[i], HEX);
     content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();
  Serial.print("Message : ");
  content.toUpperCase();
  if (content.substring(1) == "80 B3 8C BB") //change here the UID of the card/cards that you want to give access
  {
    Serial.println("Authorized access");
    Serial.println();
    delay(500);
    Serial.println("AKSES DITERIMA");
    client.publish("lock","1");
    
    Serial.println("ID KARTU ANDA : " + content.substring(1));
    delay(1000);

    authorized = true;

    digitalWrite(LOCK, HIGH);
  }
 
 else   {
    Serial.println(" Access denied");
    Serial.println("AKSES DITOLAK");
    Serial.println("ID KARTU ANDA : " + content.substring(1));
    digitalWrite(LOCK, LOW);
    client.publish("lock","0");
    authorized = false;
    
  }
}
