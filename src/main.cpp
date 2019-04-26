/**
 * Group 69
 * Laser-Tag
 * CS-362 w/ proffessor Troy
 * A collaborated project by:
 *  Victor Fong: vfong3 - 665878537
 *  Hayley Christianson: hchris6 - 667242226
 *  Tom Le: ble23 - 668579150
 * 
 * Program uses a rolling micro switch, IR receiver, IR LED implementing a Transistor, and a LCD using a I2C backpack with an Arduino,
 * housed in a gun-like housing made of artists foam.
 * 
 * 3D models were created, but the UIC makerspace would not allow us to create them due to fears of the weapon-like prints causing an alarm.
 * 
 * When the micro switch is triggered, an IR code is sent out to be picked up by another one of itself, if the signal is received, the receiver 
 * will lose 1 "health", tracked by the LCD display, and fire back a seperate code denoting a successful hit to the sender.
 * On a successful hit, the sender will earn 100 points. Killing another grants 500 additional points, and dying removes 500 points and removes player from play for 10 seconds.
 * 
 * Upon receiving 1500 points, player will secure the Epic Victory Royale. 
 * After 10 seconds. Player can pull trigger again to restart game on their end and resume play.
 * 
 **/

#include <Arduino.h>  
#include <IRLibAll.h>
#include <LiquidCrystal_I2C.h>


/* initialize constants */

const int pinTrigger = 8;
const int pinIRReceiver = 2;
//NOTE: IRSender uses pin 3
//NOTE: I2C uses A4 and A5
/* Declare Obects and variables! */

IRrecvPCI irReceiver(pinIRReceiver);
IRdecode decoder;

IRsendNECx sender;

LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display


long score = 0;

const int maxHealth = 5;
int currentHealth = maxHealth;
bool gameWon = false;
int val;                        // variable for reading the pin status
int val2;                       // variable for reading the delayed/debounced status
int buttonState = LOW;                // variable to hold the button state

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


void (* resetFunc) (void) = 0; //declare reset function @ address 0


void shootLaser()   //connected to interrupt of pinTrigger. shoots irLaser
{
  if (gameWon == true)
  {
    resetFunc();
  }
  Serial.println("pew pew!");
  sender.send(0xfd00ff);
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
  }
while (n);

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


void checkWin()
{
  if (score >= 2000)
  {
    gameWon = true;
    lcd.clear();
    lcd.print(" VICTORY ROYALE ");
    lcd.setCursor(0,1);
    lcd.print("Trigger in... ");

    for (int i = 9; i >= 0; i--)
    {
      lcd.setCursor(15,1);
      lcd.print(i);
      delay(1000);
    }
    while (digitalRead(pinTrigger) != HIGH) {}
    resetFunc();
  }
}


void checkDeath()
{
  if (currentHealth <= 0)
  {
    (score < 500) ? score = 0 : score -= 500;
    lcd.clear();
    lcd.print("You Died!");
    lcd.setCursor(0,1);
    lcd.print("Respawn in... ");
    for (int i = 9; i >= 0; i--)
    {
      lcd.setCursor(15,1);
      lcd.print(i);
      delay(1000);
    }
    currentHealth = maxHealth;
    lcd.clear();
    showScore();
    showHealth();
  }
}


void setup()
{  
  Serial.begin(9600);

  delay(2000); while(!Serial);

  pinMode(pinTrigger, INPUT);


  lcd.init();                      // initialize the lcd 
  lcd.backlight();

  lcd.createChar(0, fullHeart);
  lcd.createChar(1, emptyHeart);

  Serial.println("LCD initialized");

  showScore();
  showHealth();

  irReceiver.enableIRIn();
  Serial.println("IR enabled");
}


void loop()
{
  checkWin();
  checkDeath();

  val = digitalRead(pinTrigger);      // read input value and store it in val
  delay(10);                         // 10 milliseconds is a good amount of time
  val2 = digitalRead(pinTrigger);     // read the input again to check for bounces
  if (val == val2)  // make sure we got 2 consistant readings!
  {  
    if (val != buttonState) 
    {
      if (val == HIGH)
      {
        shootLaser();
        delay(200);
      }
    }
  }


  if (irReceiver.getResults())
  {
    Serial.println("IR RECEIVED");
    decoder.decode();
    Serial.print("protocol is: ");
    Serial.println(decoder.protocolNum);
    if (decoder.protocolNum == NECX)
    {
      switch (decoder.value)
      {
        case 0xfd00ff:  //Volume Down
        {
          Serial.println("Got Hit!");
          currentHealth--;

          if (currentHealth == 0)
          {
            sender.send(0xfd807f);  //died
            irReceiver.enableIRIn();
          }
          else
          {
            sender.send(0xfd40bf);  //hit
            irReceiver.enableIRIn();
          }
          showHealth();

          break;
        }
        case 0xfd40bf:  //Volume Up
        {
          Serial.println("Hit something!");
          score += 100;
          showScore();

          break;
        }
        case 0xfd807f:  //Play/Pause
        {
          Serial.println("Killed something!");
          score += 600;
          showScore();

          break;
        }
        default:  //Other code
        {
          Serial.print("unknown code sent: ");
          Serial.println(decoder.value);

          break;
        }
      }
    }
    else
    {
      Serial.print("Incorrect protocol:");
      Serial.println(decoder.protocolNum);
    }
    irReceiver.enableIRIn();
  }
  buttonState = val; 
}