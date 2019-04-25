#include <Arduino.h>
#include <IRLibAll.h>
#include <LiquidCrystal_I2C.h>


/* initialize constants */

const int pinTrigger = 7;
const int pinIRReceiver = 9;
const int pinIRSender = 11;
//NOTE: I2C uses A4 and A5
/* Declare Obects and variables! */

IRrecvPCI irReceiver(pinIRReceiver);
IRdecode decoder;

IRsendNEC sender;

LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display


int score = 0;

int maxHealth = 5;
int currentHealth = maxHealth;

uint8_t fullHeart[8] = { 
  0x0,
  0x0,
  0xa,
  0x1f,
  0x1f,
  0xe,
  0x4,
  0x0 
}; 

uint8_t emptyHeart[8] = { 
  0x0,
  0x0,
  0xa,
  0x15,
  0x11,
  0xa,
  0x4,
  0x0
}; 




void shootLaser()
{
  sender.send(0xfd807f);
  irReceiver.enableIRIn();
}


void showScore()
{
  unsigned int scoreSize = 0;
  unsigned int n = score;
do
{
  ++scoreSize; 
  n /= 10;
} while (n);

  lcd.setCursor(16 - scoreSize, 0);
  lcd.print(score);
}


//TODO: Health changes with RGB
void showHealth()
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
  pinMode(pinIRSender, OUTPUT);

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
