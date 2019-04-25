#include <Arduino.h>
#include <IRLibAll.h>
#include <LiquidCrystal_I2C.h>


/* initialize constants */

const int pinTrigger = 11;
const int pinIRReceiver = 6;
const int pinIRSender = 9;
//NOTE: I2C uses A4 and A5
/* Declare Obects and variables! */

IRrecvPCI irReceiver(pinIRReceiver);
IRdecode decoder;

IRsendNEC sender;

LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display


int score = 0;
int health = 3;

//FIXME: byte to hex string pls
byte fullHeart[8] = { 
  0B00000,
  0B00000,
  0B01010,
  0B11111,
  0B11111,
  0B01110,
  0B00100,
  0B00000 
}; 

byte emptyHeart[8] = { 
  0B00000,
  0B00000,
  0B01010,
  0B10101,
  0B10001,
  0B01010,
  0B00100,
  0B00000
}; 




void shootLaser()
{
  sender.send(0xfd807f);
  irReceiver.enableIRIn();
}

//TODO: Health changes with RGB
void showHealth()
{
  switch (health)
  {
    case 3:
    {
      lcd.setCursor(15,1);

      //lcd.print(char(0) + char(0) + char(0));
  
      lcd.print(char(0));
      lcd.print(char(0));
      lcd.print(char(0));      
      break;
    }
    case 2:
    {
      lcd.setCursor(15,1);
      
      lcd.print(char(0) + char(0) + char(1));
  
      //lcd.write(char(0));
      //lcd.write(char(0));
      //lcd.write(char(1));  
      break;
    }
    case 1:
    {
      lcd.setCursor(15,1);

      lcd.print(char(0) + char(1) + char(1));
      //lcd.write(char(0));
      //lcd.write(char(1));
      //lcd.write(char(1));  
      break;
    }
    case 0:
    {
      lcd.setCursor(15,1);

      lcd.print("Game Over! " + char(1) + char(1) + char(1));
      //lcd.write(char(1));
      //lcd.write(char(1));
      //lcd.write(char(1));  
      break;
    }
    default:
    {
      break;
    }
  }
}


void setup()
{  
  Serial.begin(9600);

  delay(2000); while(!Serial);

  pinMode(pinTrigger, INPUT);
  attachInterrupt(digitalPinToInterrupt(pinTrigger), shootLaser, RISING);
  
  lcd.createChar(0, fullHeart);
  lcd.createChar(1, emptyHeart);

  lcd.init();                      // initialize the lcd 
  Serial.println("LCD initialized");
  lcd.backlight();

  lcd.setCursor(15,0);
  lcd.rightToLeft();
  lcd.print(score);
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

          health -= 1;
          showHealth();
        }
        case 0xfd40bf:  //Volume Up
        {
          //Hit something!
          score += 100;
          lcd.setCursor(15, 0);
          lcd.print(score);

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
