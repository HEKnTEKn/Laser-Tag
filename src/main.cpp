/**
 * Laser-Tag
 * CS-362 w/ proffessor Troy
 * A collaborated project by:
 *  Victor Fong: vfong3 - 665878537
 *  Hayley Christianson: hchris6 - 
 *  Tom Le: ble23 - 668579150
 * 
 * Program uses a rolling micro switch, IR receiver, IR LED implementing a Transistor, and a LCD using a I2C backpack with an Arduino,
 * housed in a gun-like housing made of artists foam.
 * 
 * 3D models were created, but the UIC makerspace would not allow us to create them due to fears of the weapon-like prints causing an alarm.
 * 
 * When the micro switch is triggered, an IR code is sent out to be picked up by another one of itself, if the signal is received, the receiver 
 * will lose 1 "health", tracked by the LCD display
 * 
 */

#include <Arduino.h>
#include <IRLibAll.h>
#include <LiquidCrystal_I2C.h>


/* initialize constants */

const int pinTrigger = 7;
const int pinIRReceiver = 9;
//NOTE: IRSender uses pin 3
//NOTE: I2C uses A4 and A5
/* Declare Obects and variables! */

IRrecvPCI irReceiver(pinIRReceiver);
IRdecode decoder;

IRsendNEC sender;

LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display


int score = 0;

int maxHealth = 5;
int currentHealth = maxHealth;

uint8_t fullHeart[8] = 
{ 
  0x0,
  0x0,
  0xa,
  0x1f,
  0x1f,
  0xe,
  0x4,
  0x0 
}; 

uint8_t emptyHeart[8] =
{ 
  0x0,
  0x0,
  0xa,
  0x15,
  0x11,
  0xa,
  0x4,
  0x0
}; 




void shootLaser()   //connected to interrupt of pinTrigger. shoots irLaser
{
  sender.send(0xfd807f);
  irReceiver.enableIRIn();
}


void showScore()  //Shows score right-justified on LCD row 1
{
  unsigned int scoreSize = 0;
  unsigned int n = score;
do
{
  scoreSize++; 
  n /= 10;
} while (n);

  lcd.setCursor(16 - scoreSize, 0);
  lcd.print(score);
}



void showHealth()   //Shows health via heart-shaped custom characters right justified on LCD row 2
{
  int i = 15;
  lcd.setCursor(i,1);
  for (int j = 0; j < currentHealth; j++)
  {
    lcd.write(char(0));
    i--;
    lcd.setCursor(i,1);
  }
  for (int j = 0; j  < (maxHealth - currentHealth); j++)
  {
    lcd.write(char(1));
    i--;
    lcd.setCursor(i,1);
  }
}


void setup()
{  
  Serial.begin(9600);

  delay(2000); while(!Serial);

  pinMode(pinTrigger, INPUT);
  attachInterrupt(digitalPinToInterrupt(pinTrigger), shootLaser, RISING);
  


  lcd.init();                      // initialize the lcd 

  lcd.createChar(0, fullHeart);
  lcd.createChar(1, emptyHeart);
  lcd.backlight();

  Serial.println("LCD initialized");

  lcd.setCursor(15,0);
  showScore();
  showHealth();

  pinMode(pinIRReceiver, INPUT);

  irReceiver.enableIRIn();
  Serial.println("IR enabled");

}


void loop()
{
  if (irReceiver.getResults())
  {
    decoder.decode();
    
    if (decoder.protocolNum == NEC)
    {
      switch (decoder.value)
      {
        case 0xfd00ff:  //Volume Down
        {
          //Got Hit!
          sender.send(0xfd40bf);
          irReceiver.enableIRIn();

          currentHealth--;
          showHealth();
        }
        case 0xfd40bf:  //Volume Up
        {
          //Hit something!
          score += 100;
          showScore();

          break;
        }
        default:  //Other code
        {
          break;
        }
      }
    }
  }
}
