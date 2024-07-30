#include <Arduino.h>
#include <IQS550.h>

// IQS550 Touch Controller configurations
static const int iqs_address = 0x74;
static const int ReadyPin = 3;
static const int ResetPin = 2;

IQS550 trackpad = IQS550(iqs_address, ReadyPin, ResetPin);  // Touch Controllers

void setup() 
{
  Serial.begin(115200);
  while(!Serial){;}
  Serial.println("Begin!");
  trackpad.begin();
  Serial.println("Setup Complete!");
}

void loop() 
{
  trackpad.Process_XY();
}
