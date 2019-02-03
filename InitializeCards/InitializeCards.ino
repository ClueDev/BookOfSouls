#include <SPI.h>
#include <MFRC522.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>

#define SS_PIN    7
#define SS_PIN2   8
#define RST_PIN   9
#define RST_PIN2  6
MFRC522 mfrc522_1(SS_PIN, RST_PIN);   // Create MFRC522 instance.
MFRC522 mfrc522_2(SS_PIN2, RST_PIN2);   // Create MFRC522 instance.

const byte MAXTAGLEN = 4;
const int MEMBASE = 0;


byte RightCards[2][MAXTAGLEN];
byte read_rfid1[MAXTAGLEN];
byte read_rfid2[MAXTAGLEN]; 

bool card1Read = false;
bool card2Read = false;
int startInt = 0;

void setup() {

  Serial.begin(9600);
  SPI.begin();
  // put your setup code here, to run once:
  mfrc522_1.PCD_Init();
  mfrc522_2.PCD_Init();

  Serial.println("Setup");
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("Programming cards");
  Serial.println(String(card1Read));
  Serial.println(String(card2Read));
  
  if(mfrc522_2.PICC_IsNewCardPresent() && mfrc522_2.PICC_ReadCardSerial() && !card2Read)
  {     
    Serial.println("Read card 2. Programming.");
    if(card1Read)
    {
        startInt = MAXTAGLEN;
    }
    else
    {
        startInt = 0;  
    }
      
    for (byte i = startInt; i < mfrc522_2.uid.size + startInt; i++) 
    {
      read_rfid2[i] = mfrc522_2.uid.uidByte[i];
    }
    card2Read = true;
  }

  if(mfrc522_1.PICC_IsNewCardPresent() && mfrc522_1.PICC_ReadCardSerial() && !card1Read)
  {  
    Serial.println("Read card 1. Programming.");
    if(card2Read)
    {
        startInt = MAXTAGLEN;
    }
    else
    {
        startInt = 0;  
    }

    for (byte i = startInt; i < mfrc522_1.uid.size + startInt; i++) 
    {
      read_rfid1[i] = mfrc522_1.uid.uidByte[i];
    }
    card1Read = true;
  }

  if(card1Read && card2Read)
  {
    Serial.println("Writing correct card 1.");
      
    for (byte x = 0; x < MAXTAGLEN; x++) RightCards[1][x] = read_rfid1[x];
  
    Serial.println("Writing correct card 2.");
      
    for (byte x = 0; x < MAXTAGLEN; x++) RightCards[2][x] = read_rfid2[x];
    
    for (byte x = 0; x < 2; x++)
    {
      for (byte pos = 0; pos < MAXTAGLEN; pos++)  //loop through each position in the right card string
      {
        Serial.println("Updating EEPROM.");
        
        EEPROM.update((x * MAXTAGLEN) + pos + MEMBASE, RightCards[x][pos]);   
      }
    }
    for(int i = 0; i < EEPROM.length(); i++)
    {
      Serial.println(EEPROM.read(i));  
    }
  }
  
}
