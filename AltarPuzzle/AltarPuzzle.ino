#include <SPI.h>
#include <MFRC522.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>
 
#define SS_PIN    7
#define SS_PIN2   8
#define RST_PIN   9
#define RST_PIN2  6
#define ARDUINO_RX 5  //should connect to TX of the Serial MP3 Player module
#define ARDUINO_TX 3  //connect to RX of the module
MFRC522 mfrc522_1(SS_PIN, RST_PIN);   // Create MFRC522 instance.
MFRC522 mfrc522_2(SS_PIN2, RST_PIN2);   // Create MFRC522 instance.
SoftwareSerial mp3(ARDUINO_RX, ARDUINO_TX); //Create an mp3 instance.


/*As I have the code formatted right now, the RGB LEDs turn yellow upon 
completing the color, no functionality has been added to mitigate which color should display or
when they should display, good luck m8.*/

int relayPin =        14;
int winningLED1 =     17;
int switch1Pin =      4;
int switch2Pin =      2;
int switch1LED =      15;
//int bluePin1 =       16;
//int greenPin2 =      18;
int switch2LED =       19;
String keyOne = "4D FD 50 D3";
String keyTwo = "99 5E C5 48";

bool puzzleFinished = false;

bool reader1Complete = false;
bool reader2Complete = false;

static int8_t Send_buf[8] = {0}; // Buffer for Send commands.  // BETTER LOCALLY
static uint8_t ansbuf[10] = {0}; // Buffer for the answers.    // BETTER LOCALLY

String mp3Answer;           // Answer from the MP3.

String sanswer(void);
String sbyte2hex(uint8_t b);

bool isProgramMode = false;
int programModePin = 18; //set Program mode here

const byte MAXTAGLEN = 4;
const int MEMBASE = 0;

byte RightCards[2][MAXTAGLEN];
byte read_rfid1[MAXTAGLEN];
byte read_rfid2[MAXTAGLEN];    

/************ Command byte **************************/
#define CMD_NEXT_SONG     0X01  // Play next song.
#define CMD_PREV_SONG     0X02  // Play previous song.
#define CMD_PLAY_W_INDEX  0X03
#define CMD_VOLUME_UP     0X04
#define CMD_VOLUME_DOWN   0X05
#define CMD_SET_VOLUME    0X06

#define CMD_SNG_CYCL_PLAY 0X08  // Single Cycle Play.
#define CMD_SEL_DEV       0X09
#define CMD_SLEEP_MODE    0X0A
#define CMD_WAKE_UP       0X0B
#define CMD_RESET         0X0C
#define CMD_PLAY          0X0D
#define CMD_PAUSE         0X0E
#define CMD_PLAY_FOLDER_FILE 0X0F

#define CMD_STOP_PLAY     0X16  // Stop playing continuously. 
#define CMD_FOLDER_CYCLE  0X17
#define CMD_SHUFFLE_PLAY  0x18 //
#define CMD_SET_SNGL_CYCL 0X19 // Set single cycle.

#define CMD_SET_DAC 0X1A
#define DAC_ON  0X00
#define DAC_OFF 0X01

#define CMD_PLAY_W_VOL    0X22
#define CMD_PLAYING_N     0x4C
#define CMD_QUERY_STATUS      0x42
#define CMD_QUERY_VOLUME      0x43
#define CMD_QUERY_FLDR_TRACKS 0x4e
#define CMD_QUERY_TOT_TRACKS  0x48
#define CMD_QUERY_FLDR_COUNT  0x4f

/************ Opitons **************************/
#define DEV_TF            0X02


/*********************************************************************/
  
void setup() {

  Serial.begin(9600);
  
  pinMode(relayPin, OUTPUT);
  pinMode(winningLED1, OUTPUT);
  pinMode(switch1Pin, INPUT_PULLUP);
  pinMode(switch2Pin, INPUT_PULLUP);
  pinMode(switch1LED, OUTPUT);
  pinMode(switch2LED, OUTPUT);
  digitalWrite(relayPin, HIGH);
  digitalWrite(winningLED1, LOW);
  digitalWrite(switch1LED, LOW);
  digitalWrite(switch2LED, LOW);
  
  // Code for the RFID reader
  SPI.begin();
  
  mfrc522_1.PCD_Init();
  mfrc522_2.PCD_Init();

  mp3.begin(9600);
  delay(500);

  sendCommand(CMD_SEL_DEV, 0, DEV_TF);
  delay(500);
  sendCommand(CMD_VOLUME_UP);
  sendCommand(CMD_VOLUME_UP);

  setCorrectCards();
  
  Serial.println("Approximate your card to the reader...");
  Serial.println();
}

void loop() {

//  if(digitalRead(programModePin, HIGH))
//  {
//     programCards();
//  }

  while(mfrc522_1.PICC_IsNewCardPresent() && mfrc522_1.PICC_ReadCardSerial() || reader1Complete)
  { 
      Serial.println("While loop 1");
      // If it wasn't the correct RFID, break
      if(!(UID(mfrc522_1) == keyOne))
          break;

      Serial.println("Reader 1 correct");
      // If the switch is clicked, turn on the light and make a 'click' noise (reader 1 is complete)
      if(!digitalRead(switch1Pin) && !reader1Complete)
      {
        digitalWrite(switch1LED, HIGH); 
        Serial.println("switch 1 clicked");
        
        sendCommand(CMD_PLAY_FOLDER_FILE, 02, 02); // Play a 'click' sound here
        reader1Complete = true;
      }

      // Try to read reader 2
      if(mfrc522_2.PICC_IsNewCardPresent() && mfrc522_2.PICC_ReadCardSerial() || reader2Complete)
      {
            // If it wasn't the correct RFID, break
            if(!(UID(mfrc522_2) == keyTwo))
                break;

            Serial.println("Reader 2 correct");
            // If switch is clicked, turn on the light and make a 'click' noise (reader 2 is complete)
            if(!digitalRead(switch2Pin) && !reader2Complete)
            {
              digitalWrite(switch2LED, HIGH);  
              sendCommand(CMD_PLAY_FOLDER_FILE, 02, 02); // Play a 'click' sound here
              reader2Complete = true;
              Serial.println("switch 2 clicked");
            }
        }

        // If both readers are complete, the puzzle is finished
        if (reader1Complete && reader2Complete)
        {
          puzzleFinished = winningSequence();
        }
        while(puzzleFinished);
  }
  
  while(mfrc522_2.PICC_IsNewCardPresent() && mfrc522_2.PICC_ReadCardSerial() || reader2Complete)
  { 
      Serial.println("While loop 2");
      // If it wasn't the correct RFID, break
      if(!(UID(mfrc522_2) == keyTwo))
          break;

      Serial.println("Reader 2 correct");
      // If the switch is clicked, turn on the light and make a 'click' noise (reader 2 is complete)
      if(!digitalRead(switch2Pin) && !reader2Complete)
      {
        digitalWrite(switch2LED, HIGH); 
        Serial.println("switch 1 clicked");
        sendCommand(CMD_PLAY_FOLDER_FILE, 02, 02); // Play a 'click' sound here
        reader2Complete = true;
      }

      // Try to read reader 1
      if(mfrc522_1.PICC_IsNewCardPresent() && mfrc522_1.PICC_ReadCardSerial() || reader1Complete)
      {
            // If it wasn't the correct RFID, break
            if(!(UID(mfrc522_1) == keyOne))
                break;
            Serial.println("Reader 1 correct");
            // If switch is clicked, turn on the light and make a 'click' noise (reader 1 is complete)
            if(!digitalRead(switch1Pin) && !reader1Complete)
            {
              digitalWrite(switch1LED, HIGH);
              Serial.println("switch 1 clicked");  
              sendCommand(CMD_PLAY_FOLDER_FILE, 02, 02); // Play a 'click' sound here
              reader1Complete = true;
            }
        }

        // If both readers are complete, the puzzle is finished
        if (reader1Complete && reader2Complete)
        {
          puzzleFinished = winningSequence();
        }
        while(puzzleFinished);
    }
    if(digitalRead(switch1Pin) == LOW)
      Serial.println("not working");
}

/********************************************************************************/
/*Function: Update the data for correct cards on the arduino                    */

void programCards()
{
  Serial.println("Programming cards");

  if(mfrc522_2.PICC_IsNewCardPresent() && mfrc522_2.PICC_ReadCardSerial())
  {     

    Serial.println("Read card 2. Programming.");
    
    for (byte x = 0; x < MAXTAGLEN; x++) read_rfid2[x] = 0;

    for (byte i = 0; i < mfrc522_2.uid.size; i++) 
    {
      read_rfid2[i] = mfrc522_2.uid.uidByte[i];
    }
  }

  if(mfrc522_1.PICC_IsNewCardPresent() && mfrc522_1.PICC_ReadCardSerial())
  {  
    Serial.println("Read card 1. Programming.");
       
    for (byte x = 0; x < MAXTAGLEN; x++) read_rfid1[x] = 0;

    for (byte i = 0; i < mfrc522_1.uid.size; i++) 
    {
      read_rfid1[i] = mfrc522_1.uid.uidByte[i];
    }
  }

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
}

void setCorrectCards()
{
  byte NewChar;
  
  for(byte reader; reader < 2; reader++)
  {
    for (byte x = reader * MAXTAGLEN; x < (reader * MAXTAGLEN) + MAXTAGLEN; x++)
    {
      NewChar = EEPROM.read(x + MEMBASE);   //read one byte from EEPROM
      Serial.print(NewChar, HEX);
      Serial.print(" ");
    
      RightCards[reader][x - (reader * MAXTAGLEN)] = NewChar;
    }
  } 
}

/********************************************************************************/
/*Function: Call operations associated with puzzle completion                   */
/* 1. Light LED to illuminate book                                              */
/* 2. Play sound                                                                */
/* 3. Release the MagLock                                                       */

boolean winningSequence()
{
  delay(2000);
  digitalWrite(winningLED1, HIGH);
  sendCommand(CMD_VOLUME_DOWN);
  sendCommand(CMD_PLAY_FOLDER_FILE, 01, 01);
  delay(330);
  digitalWrite(relayPin, LOW);
  return true; 
}

/********************************************************************************/
/*Function: Set the LED color on reader 1                                       */
void setColorOnLED1(int red, int green, int blue)
{
//  analogWrite(greenPin1, green);
  //analogWrite(bluePin1, blue); 
}

/********************************************************************************/
/*Function: Set the LED color on reader 2                                       */
void setColorOnLED2(int red, int green, int blue)
{
  //analogWrite(greenPin2, green);
  //analogWrite(bluePin2, blue);  
}

/********************************************************************************/
/*Function: Get the UID of an RFID chip                                         */
String UID(MFRC522 mfrc522)
{
  String UIDString = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
     UIDString.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     UIDString.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  UIDString.toUpperCase();
  return UIDString.substring(1);  
}

/********************************************************************************/
/*Function: See if the UID tag is correct                                       */
boolean isCorrectTag(String currentTag, String Key)
{
    if(currentTag == Key)
        return true;

    return false;
}

/********************************************************************************/
/*Function: Send command to the MP3                                             */
/*Parameter: byte command                                                       */
/*Parameter: byte dat1 parameter for the command                                */
/*Parameter: byte dat2 parameter for the command                                */

void sendCommand(byte command){
  sendCommand(command, 0, 0);
}

void sendCommand(byte command, byte dat1, byte dat2){
  delay(20);
  Send_buf[0] = 0x7E;    //
  Send_buf[1] = 0xFF;    //
  Send_buf[2] = 0x06;    // Len
  Send_buf[3] = command; //
  Send_buf[4] = 0x01;    // 0x00 NO, 0x01 feedback
  Send_buf[5] = dat1;    // datah
  Send_buf[6] = dat2;    // datal
  Send_buf[7] = 0xEF;    //
  Serial.print("Sending: ");
  for (uint8_t i = 0; i < 8; i++)
  {
    mp3.write(Send_buf[i]) ;
    Serial.print(sbyte2hex(Send_buf[i]));
  }
  Serial.println();
}

/********************************************************************************/
/*Function: sbyte2hex. Returns a byte data in HEX format.                       */
/*Parameter:- uint8_t b. Byte to convert to HEX.                                */
/*Return: String                                                                */


String sbyte2hex(uint8_t b)
{
  String shex;

  shex = "0X";

  if (b < 16) shex += "0";
  shex += String(b, HEX);
  shex += " ";
  return shex;
}

/********************************************************************************/
/*Function: shex2int. Returns a int from an HEX string.                         */
/*Parameter: s. char *s to convert to HEX.                                      */
/*Parameter: n. char *s' length.                                                */
/*Return: int                                                                   */

int shex2int(char *s, int n){
  int r = 0;
  for (int i=0; i<n; i++){
     if(s[i]>='0' && s[i]<='9'){
      r *= 16; 
      r +=s[i]-'0';
     }else if(s[i]>='A' && s[i]<='F'){
      r *= 16;
      r += (s[i] - 'A') + 10;
     }
  }
  return r;
}

/********************************************************************************/
/*Function: sanswer. Returns a String answer from mp3 UART module.          */
/*Parameter:- uint8_t b. void.                                                  */
/*Return: String. If the answer is well formated answer.                        */

String sanswer(void)
{
  uint8_t i = 0;
  String mp3answer = "";

  // Get only 10 Bytes
  while (mp3.available() && (i < 10))
  {
    uint8_t b = mp3.read();
    ansbuf[i] = b;
    i++;

    mp3answer += sbyte2hex(b);
  }

  // if the answer format is correct.
  if ((ansbuf[0] == 0x7E) && (ansbuf[9] == 0xEF))
  {
    return mp3answer;
  }

  return "???: " + mp3answer;
}
