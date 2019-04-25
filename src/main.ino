#include <Arduino.h>
#include <IRLibAll.h>


/* initialize constants */

const int pinTrigger;
const int pinIRReceiver;
const int pinIRSender;

/* Declare Obects and variables! */

IRrecvPCI irReceiver(pinIRReceiver);
IRdecode decoder;

IRsendNEC sender;

void setup()
{  
  Serial.begin(9600);

  delay(2000); while(!Serial);

  pinMode(pinTrigger, INPUT);
  attachInterrupt(digitalPinToInterrupt(pinTrigger), shootLaser, RISING);
  
  pinMode(pinIRReceiver, INPUT);
  pinMode(pinIRSender, OUTPUT);

  irReceiver.enableIRIn();
  Serial.println("IR enabled");
}


void shootLaser()
{
  sender.send(0xfd807f);
  irReceiver.enableIRIn();
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
      case 0xfd807f:  //Play/Pause:
      {
        //change something!
        break;
      }
      
      default:
        break;
      }
    }
  }
}