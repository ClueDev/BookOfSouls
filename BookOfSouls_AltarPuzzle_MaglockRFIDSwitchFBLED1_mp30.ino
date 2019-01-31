#include <SPI.h>
#include <MFRC522.h>
#include <SoftwareSerial.h>

//Test comment
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
int greenPin1 =      15;
int bluePin1 =       16;
int greenPin2 =      18;
int bluePin2 =       19;
String keyOne = "4D FD 50 D3";
String keyTwo = "99 5E C5 48";

bool puzzleFinished = false;

int card1Counter;
int card2Counter;

static int8_t Send_buf[8] = {0}; // Buffer for Send commands.  // BETTER LOCALLY
static uint8_t ansbuf[10] = {0}; // Buffer for the answers.    // BETTER LOCALLY

String mp3Answer;           // Answer from the MP3.


String sanswer(void);
String sbyte2hex(uint8_t b);


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
  pinMode(switch1Pin, INPUT);
  pinMode(switch2Pin, INPUT);
  pinMode(greenPin1, OUTPUT);
  pinMode(bluePin1, OUTPUT);
  pinMode(greenPin2, OUTPUT);
  pinMode(bluePin2, OUTPUT);
  digitalWrite(relayPin, HIGH);
  digitalWrite(winningLED1, LOW);

  // initialize counters
  card1Counter = 250;
  card2Counter = 250;
  
  // Code for the RFID reader
  SPI.begin();
  
  mfrc522_1.PCD_Init();
  mfrc522_2.PCD_Init();

  mp3.begin(9600);
  delay(500);

  sendCommand(CMD_SEL_DEV, 0, DEV_TF);
  delay(500);
  
  Serial.println("Approximate your card to the reader...");
  Serial.println();

  
}

void loop() {

  // if the loop has been called 50 times without a card being read, turn the lights off (Can we check if a light is on first? if(>=50 and light is on) - that would be ideal)
  if(card1Counter >= 25)
  {
      setColorOnLED1(0, 0, 0);
  }
  if(card2Counter >= 25)
  {
      setColorOnLED2(0, 0, 0);
  }

  while(mfrc522_1.PICC_IsNewCardPresent() && mfrc522_1.PICC_ReadCardSerial())
  { 
      if(!(UID(mfrc522_1) == keyOne))
          break;
          
      // we read card1 successfully, so turn on LED and reset the counter
      setColorOnLED1(0, 0, 255);
      card1Counter = 0;
      
      if(mfrc522_2.PICC_IsNewCardPresent() && mfrc522_2.PICC_ReadCardSerial() && digitalRead(switch1Pin) && digitalRead(switch2Pin)){

            if(!(UID(mfrc522_2) == keyTwo))
                break;
                
            // we read card2 successfully, so turn on LED and reset the counter
            setColorOnLED2(0, 0, 255);
            card2Counter = 0;
            delay(100);
            
            puzzleFinished = winningSequence();

        }
        while(puzzleFinished);
  }
  while(mfrc522_2.PICC_IsNewCardPresent() && mfrc522_2.PICC_ReadCardSerial())
  {
      if(!(UID(mfrc522_2) == keyTwo))
          break;
      
      // we read card2 successfully, so turn on LED and reset the counter
      setColorOnLED2(0, 0, 255);
      card2Counter = 0;
      
      if(mfrc522_1.PICC_IsNewCardPresent() && mfrc522_1.PICC_ReadCardSerial() && digitalRead(switch1Pin) && digitalRead(switch2Pin))
      {
            if(!(UID(mfrc522_1) == keyOne))
                break;
                
            setColorOnLED1(0, 0, 255);
            card1Counter = 0;
            delay(100);
            puzzleFinished = winningSequence();
        }
        //Serial.println("Card 2 on");
      while(puzzleFinished);
  }

  card1Counter = card1Counter + 1;
  card2Counter = card2Counter + 1;
  //delay(500);
}
boolean winningSequence()
{
  setColorOnLED1(0, 255, 0);
  setColorOnLED2(0, 255, 0);
  digitalWrite(winningLED1, HIGH);
  //sendCommand(CMD_PLAY_FOLDER_FILE, 01, 01);
  //Serial.println("first song playing");
  //delay(2347);
  sendCommand(CMD_PLAY_FOLDER_FILE, 01, 01);
  Serial.println("second song playing");
  delay(8344);
  //sendCommand(CMD_PLAY_FOLDER_FILE, 01, 01);
  //delay(301);
  Serial.println("third song playing");

  digitalWrite(relayPin, LOW);
  return true; 
}
void setColorOnLED1(int red, int green, int blue)
{
  analogWrite(greenPin1, green);
  analogWrite(bluePin1, blue); 
}
void setColorOnLED2(int red, int green, int blue)
{
  analogWrite(greenPin2, green);
  analogWrite(bluePin2, blue);  
}
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
boolean isCorrectTag(String currentTag, String Key)
{
    if(currentTag == Key)
        return true;

    return false;
}
void sendMP3Command(char c) {
  Serial.println(c);
  switch (c) {
    case '?':
    case 'h':
      Serial.println("HELP  ");
      Serial.println(" p = Play");
      Serial.println(" P = Pause");
      Serial.println(" > = Next");
      Serial.println(" < = Previous");
      Serial.println(" s = Stop Play"); 
      Serial.println(" + = Volume UP");
      Serial.println(" - = Volume DOWN");
      Serial.println(" c = Query current file");
      Serial.println(" q = Query status");
      Serial.println(" v = Query volume");
      Serial.println(" x = Query folder count");
      Serial.println(" t = Query total file count");
      Serial.println(" f = Play folder 1.");
      Serial.println(" S = Sleep");
      Serial.println(" W = Wake up");
      Serial.println(" r = Reset");
      break;


    case 'p':
      Serial.println("Play ");
      sendCommand(CMD_PLAY);
      break;

    case 'P':
      Serial.println("Pause");
      sendCommand(CMD_PAUSE);
      break;

    case '>':
      Serial.println("Next");
      sendCommand(CMD_NEXT_SONG);
      sendCommand(CMD_PLAYING_N); // ask for the number of file is playing
      break;

    case '<':
      Serial.println("Previous");
      sendCommand(CMD_PREV_SONG);
      sendCommand(CMD_PLAYING_N); // ask for the number of file is playing
      break;

    case 's':
      Serial.println("Stop Play");
      sendCommand(CMD_STOP_PLAY);
      break;


    case '+':
      Serial.println("Volume Up");
      sendCommand(CMD_VOLUME_UP);
      break;

    case '-':
      Serial.println("Volume Down");
      sendCommand(CMD_VOLUME_DOWN);
      break;

    case 'c':
      Serial.println("Query current file");
      sendCommand(CMD_PLAYING_N);
      break;

    case 'q':
      Serial.println("Query status");
      sendCommand(CMD_QUERY_STATUS);
      break;

    case 'v':
      Serial.println("Query volume");
      sendCommand(CMD_QUERY_VOLUME);
      break;

    case 'x':
      Serial.println("Query folder count");
      sendCommand(CMD_QUERY_FLDR_COUNT);
      break;

    case 't':
      Serial.println("Query total file count");
      sendCommand(CMD_QUERY_TOT_TRACKS);
      break;

    case 'f':
      Serial.println("Playing folder 1");
      sendCommand(CMD_FOLDER_CYCLE, 1, 0);
      break;

    case 'S':
      Serial.println("Sleep");
      sendCommand(CMD_SLEEP_MODE);
      break;

    case 'W':
      Serial.println("Wake up");
      sendCommand(CMD_WAKE_UP);
      break;

    case 'r':
      Serial.println("Reset");
      sendCommand(CMD_RESET);
      break;
  }
}



/********************************************************************************/
/*Function decodeMP3Answer: Decode MP3 answer.                                  */
/*Parameter:-void                                                               */
/*Return: The                                                  */

String decodeMP3Answer() {
  String decodedMP3Answer = "";
  Serial.println("made it to here!");
  decodedMP3Answer += sanswer();

  switch (ansbuf[3]) {
    case 0x3A:
      decodedMP3Answer += " -> Memory card inserted.";
      break;

    case 0x3D:
      decodedMP3Answer += " -> Completed play num " + String(ansbuf[6], DEC);
      break;

    case 0x40:
      decodedMP3Answer += " -> Error";
      break;

    case 0x41:
      decodedMP3Answer += " -> Data recived correctly. ";
      break;

    case 0x42:
      decodedMP3Answer += " -> Status playing: " + String(ansbuf[6], DEC);
      break;

    case 0x48:
      decodedMP3Answer += " -> File count: " + String(ansbuf[6], DEC);
      break;

    case 0x4C:
      decodedMP3Answer += " -> Playing: " + String(ansbuf[6], DEC);
      break;

    case 0x4E:
      decodedMP3Answer += " -> Folder file count: " + String(ansbuf[6], DEC);
      break;

    case 0x4F:
      decodedMP3Answer += " -> Folder count: " + String(ansbuf[6], DEC);
      break;
  }

  return decodedMP3Answer;
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
