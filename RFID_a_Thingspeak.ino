#include <SoftwareSerial.h>
#include <SPI.h>      // incluye libreria bus SPI
#include <MFRC522.h>      // incluye libreria especifica para MFRC522

#define RX 2
#define TX 3
#define RST_PIN  9      // constante para referenciar pin de reset
#define SS_PIN  10      // constante para referenciar pin de slave select
#define trigPin 8
#define echoPin 5
String AP = "AXTEL XTREMO-88F1";       // AP NAME
String PASS = "038788F1"; // AP PASSWORD
String API = "MODKV138K9INLWRW";   // Write API KEY
String HOST = "api.thingspeak.com";
String PORT = "80";
String field = "field1";
int countTrueCommand;
int countTimeCommand; 
boolean found = false; 
int valSensor = 1;
String tag = "";
int rfid = 0;
int nfcTag = 0;
int speakerPin = 7;
int numTones = 10;
int tones[ ] = {261, 277, 294, 311, 330, 349, 370, 392, 415, 440,466, 494};// mid C C# D D# E F F# G G# A
                       
SoftwareSerial esp8266(RX,TX); 

MFRC522 mfrc522(SS_PIN, RST_PIN); // crea objeto mfrc522 enviando pines de slave select y reset

void setup() {
  Serial.begin(9600);     // inicializa comunicacion por monitor serie a 9600 bps
  SPI.begin();        // inicializa bus SPI
  mfrc522.PCD_Init();     // inicializa modulo lector
  esp8266.begin(115200);
  sendCommand("AT",5,"OK");
  sendCommand("AT+CWMODE=1",5,"OK");
  sendCommand("AT+CWJAP=\""+ AP +"\",\""+ PASS +"\"",20,"OK");
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

void loop() {
  if ( ! mfrc522.PICC_IsNewCardPresent()) // si no hay una tarjeta presente
    return;         // retorna al loop esperando por una tarjeta
  
  if ( ! mfrc522.PICC_ReadCardSerial())   // si no puede obtener datos de la tarjeta
    return;         // retorna al loop esperando por otra tarjeta
    
  Serial.print("UID:");       // muestra texto UID:
  for (byte i = 0; i < mfrc522.uid.size; i++) { // bucle recorre de a un byte por vez el UID
    if (mfrc522.uid.uidByte[i] < 0x10){   // si el byte leido es menor a 0x10
      Serial.print(" 0");     // imprime espacio en blanco y numero cero
      }
      else{         // sino
      Serial.print(" ");      // imprime un espacio en blanco
      }  // imprime el byte del UID leido en hexadecimal
    Serial.println();
    Serial.print(mfrc522.uid.uidByte[i]);
    rfid = rfid + mfrc522.uid.uidByte[i];

  } 
  
  tag = String(rfid);
  if(tag == "364") {
    nfcTag = 1;
  } else if (tag == "401") {
    nfcTag = 2;
  } else {
    nfcTag = 0;
  }
  
  Serial.println();       // nueva linea
  mfrc522.PICC_HaltA();                   // detiene comunicacion con tarjeta
  
  makeRequest();
}

void makeRequest(){
  // ESP 8266 CONNECTION TO THINGSKEAK API 

  String getData = "GET /update?api_key="+ API +"&"+ field +"="+String(nfcTag);
  sendCommand("AT+CIPMUX=1",5,"OK");
  sendCommand("AT+CIPSTART=0,\"TCP\",\""+ HOST +"\","+ PORT,15,"OK");
  sendCommand("AT+CIPSEND=0," +String(getData.length()+4),4,">");
  esp8266.println(getData);delay(3000);countTrueCommand++;
  sendCommand("AT+CIPCLOSE=0",5,"OK");
  tag = "";
  rfid = 0;
  nfcTag = 0;
}

/* ESP 8266 FUNCTIONS */

void sendCommand(String command, int maxTime, char readReplay[]) {
  while(countTimeCommand < (maxTime*1))
  {
    esp8266.println(command);//at+cipsend
    if(esp8266.find(readReplay))//ok
    {
      found = true;
      break;
    }
  
    countTimeCommand++;
  }
  
  if(found == true)
  {
    Serial.println("   <---------------- SUCCESS");
    countTrueCommand++;
    countTimeCommand = 0;
  }
  
  if(found == false)
  {
    Serial.println("   <---------------- FAILURE");
    countTrueCommand = 0;
    countTimeCommand = 0;
    
    delay(3000);
    makeRequest();
  }
  
  found = false;
 }