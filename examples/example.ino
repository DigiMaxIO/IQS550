#include <Arduino.h>
#include <IQS550.h>

// IQS550 Touch Controller configurations
static const int iqs_address = 0x74;
static const int ReadyPin = 3;
static const int ResetPin = 2;

IQS550 trackpad = IQS550(0x74, ReadyPin, ResetPin);  // Touch Controllers

int8_t x;
int8_t y;

uint8_t 	Data_Buff[44];
uint16_t	ui16SnapStatus[15], ui16PrevSnap[15];

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
  uint8_t		ui8TempData[30], i;
	trackpad.RDY_wait();
	
	trackpad.I2C_Read(GestureEvents0_adr, &Data_Buff[0], 44);

	if((Data_Buff[3] & SNAP_TOGGLE) != 0)
	{
		trackpad.I2C_Read(SnapStatus_adr, &ui8TempData[0], 30);
		for(i = 0; i < 15; i++)
		{
			ui16PrevSnap[i] = ui16SnapStatus[i];
			ui16SnapStatus[i] = ((uint16_t)(ui8TempData[2*i])<<8) + 
								 (uint16_t)ui8TempData[(2*i)+1];
		}
	}
	trackpad.Close_Comms();
  
  trackpad.Process_XY();
}
