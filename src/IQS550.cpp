#include <IQS550.h>

extern uint8_t 	Data_Buff[44];
extern uint16_t	ui16SnapStatus[15], ui16PrevSnap[15];;

uint16_t 	ui16Timeout = 100;  // Timeout on i2c in ms
uint8_t 	ui8Success;
uint32_t 	ui32StartTime;

IQS550::IQS550(uint8_t address, uint8_t readyPin, uint8_t resetPin)
{
    x = 0;
    y = 0;
    this->i2cDeviceAddr = address;
    this->ready_pin = readyPin;
    this->reset_pin = resetPin;
}

void IQS550::begin()
{
    pinMode(this->ready_pin, INPUT);
    pinMode(this->reset_pin, OUTPUT);
    digitalWrite(this->reset_pin, HIGH);
    I2C_Setup();
    AcknowledgeReset();
    CheckVersion();
    Close_Comms();
    delay(20);
}

void IQS550::AcknowledgeReset(void) 
{
    static uint8_t System_ctrl_0 = ACK_RESET;  

	I2C_Write(SystemControl0_adr, &System_ctrl_0, 1);
}


void IQS550::CheckVersion(void) 
{
	uint8_t ui8DataBuffer[6];
	I2C_Read(ProductNumber_adr, &ui8DataBuffer[0] ,6);
  
	Serial.print("Product "); 
	Serial.print((ui8DataBuffer[0]<<8) + ui8DataBuffer[1]);
	Serial.print("  Project ");
	Serial.print((ui8DataBuffer[2]<<8) + ui8DataBuffer[3]);
	Serial.print("  Version "); 
	Serial.print(ui8DataBuffer[4]); Serial.print("."); 
	Serial.println(ui8DataBuffer[5]);
}


void IQS550::DisplaySnap(void)
{
	uint8_t		ui8Tx, ui8Rx;
	uint16_t	ui16ToggledBits;
	
	for(ui8Tx = 0; ui8Tx < 15; ui8Tx++)
	{
		ui16ToggledBits = ui16PrevSnap[ui8Tx] ^ ui16SnapStatus[ui8Tx];

		for(ui8Rx = 0; ui8Rx < 10; ui8Rx++)
		{
			if(BitIsSet(ui16ToggledBits, ui8Rx))
			{
				if(BitIsSet(ui16SnapStatus[ui8Tx], ui8Rx))
				{
					Serial.print("Snap set on Rx");
				}
				else
				{
					Serial.print("Snap released on Rx");
				}
				Serial.print(ui8Rx);
				Serial.print("/Tx");
				Serial.print(ui8Tx);
				Serial.println(" channel    ");
			}
		}
	}
}

void IQS550::Process_XY(void) 
{ 
	uint8_t 	i; 
	static uint8_t ui8FirstTouch = 0;
	uint8_t 	ui8NoOfFingers;
	uint8_t 	ui8SystemFlags[2];
	int16_t 	i16RelX[6];
	int16_t 	i16RelY[6];
	uint16_t 	ui16AbsX[6];
	uint16_t 	ui16AbsY[6];
	uint16_t 	ui16TouchStrength[6];
	uint8_t  	ui8Area[6];
 
	ui8SystemFlags[0] = Data_Buff[2];
	ui8SystemFlags[1] = Data_Buff[3];
	ui8NoOfFingers = Data_Buff[4];
	//
	// Re-initialize the device if unexpected RESET detected
	//
	if((ui8SystemFlags[0] & SHOW_RESET) != 0)
	{
		Serial.println("RESET DETECTED");
		AcknowledgeReset();
		return;
	}

	if((ui8SystemFlags[1] & SNAP_TOGGLE) != 0)
	{
		// A snap state has changed, thus indicate which channel
		//
		DisplaySnap();
		return;
	}

	if((Data_Buff[0]) == SINGLE_TAP) 
	{		
		Serial.println("Single Tap  "); 
	}
	else if((Data_Buff[1]) == TWO_FINGER_TAP)   
	{
		Serial.println("2 Finger Tap"); 
	}		

	if(ui8NoOfFingers != 0) 
	{
		if (!(ui8FirstTouch)) 
		{
			Serial.print("Gestures:    ");
			Serial.print(" RelX: ");
			Serial.print("RelY: ");
			Serial.print("Fig: ");
			Serial.print("X1:  "); Serial.print("Y1:  "); Serial.print("TS1: "); Serial.print("TA1: ");
			Serial.print("X2:  "); Serial.print("Y2:  "); Serial.print("TS2: "); Serial.print("TA2: ");
			Serial.print("X3:  "); Serial.print("Y3:  "); Serial.print("TS3: "); Serial.print("TA3: ");
			Serial.print("X4:  "); Serial.print("Y4:  "); Serial.print("TS4: "); Serial.print("TA4: ");
			Serial.print("X5:  "); Serial.print("Y5:  "); Serial.print("TS5: "); Serial.println("TA5: ");
			ui8FirstTouch = 1;
		}

		switch (Data_Buff[0])
		{
			case SINGLE_TAP		:  	Serial.print("Single Tap  "); 
									break;
			case TAP_AND_HOLD	:  	Serial.print("Tap & Hold  "); 
									break;
			case SWIPE_X_NEG	:  	Serial.print("Swipe X-    "); 
									break;
			case SWIPE_X_POS	:  	Serial.print("Swipe X+    "); 
									break;
			case SWIPE_Y_POS	:  	Serial.print("Swipe Y+    "); 
									break;
			case SWIPE_Y_NEG	:  	Serial.print("Swipe Y-    "); 
									break;
		}

		switch (Data_Buff[1])
		{
			case TWO_FINGER_TAP	:  	Serial.print("2 Finger Tap"); 
									break;
			case SCROLL			:  	Serial.print("Scroll      "); 
									break;
			case ZOOM			:  	Serial.print("Zoom        "); 
									break;
		}
		if((Data_Buff[0] | Data_Buff[1]) == 0) 
		{
			Serial.print("            ");
		}

		i16RelX[1] = ((Data_Buff[5] << 8) | (Data_Buff[6]));
		i16RelY[1] = ((Data_Buff[7] << 8) | (Data_Buff[8]));
		Print_signed(i16RelX[1]);
		Print_signed(i16RelY[1]);
		Print_unsigned(ui8NoOfFingers);    

		for (i = 0; i < 5; i++)
		{
			ui16AbsX[i+1] = ((Data_Buff[(7*i)+9] << 8) | (Data_Buff[(7*i)+10])); //9-16-23-30-37//10-17-24-31-38
			ui16AbsY[i+1] = ((Data_Buff[(7*i)+11] << 8) | (Data_Buff[(7*i)+12])); //11-18-25-32-39//12-19-26-33-40
			ui16TouchStrength[i+1] = ((Data_Buff[(7*i)+13] << 8) | (Data_Buff[(7*i)+14])); //13-20-27-34-11/14-21-28-35-42
			ui8Area[i+1] = (Data_Buff[7*i+15]); //15-22-29-36-43
			Print_unsigned(ui16AbsX[i+1]);
			Print_unsigned(ui16AbsY[i+1]);
			Print_unsigned(ui16TouchStrength[i+1]);
			Print_unsigned(ui8Area[i+1]);
		}
		Serial.println("");
	} 
	else 
	{
		ui8FirstTouch = 0;
	}
}

void IQS550::Print_signed(int16_t i16Num)
{
	if(i16Num < (-99))
	{
		Serial.print(" ");
	}
	else if(i16Num < (-9))
	{
		Serial.print("  ");
	}
	else if(i16Num < 0)
	{
		Serial.print("   ");
	}
	else if(i16Num < 10)
	{
		Serial.print("    ");
	}
	else if(i16Num < 100)
	{
		Serial.print("   ");
	}
	else if(i16Num < 1000)
	{
		Serial.print("  ");
	}
	Serial.print(i16Num);
}

void IQS550::Print_unsigned(uint16_t ui16Num)
{
	if(ui16Num < 10)
	{
		Serial.print("    ");
	}
	else if(ui16Num < 100)
	{
		Serial.print("   ");
	}
	else if(ui16Num < 1000)
	{
		Serial.print("  ");
	}
	else if(ui16Num < 10000)
	{
		Serial.print(" ");
	}

	if(ui16Num > 10000)
	{
		Serial.print("  xxx");
	}
	else
	{
		Serial.print(ui16Num);
	}
}


void IQS550::I2C_Setup(void)
{
	Wire.begin();
  Wire.setClock(400000);
}
 

uint8_t IQS550::I2C_Write(uint16_t ui16RegisterAddress, uint8_t *pData, uint8_t ui8NoOfBytes)
{
	uint8_t ui8Retry = 4;
	
	ui8Success = I2C_Write2(ui16RegisterAddress, pData, ui8NoOfBytes);

	//
	// If comms was not successful, retry 4 more times
	//
	while((!ui8Success) && (ui8Retry != 0))
	{
		delay(5);
		ui8Success = I2C_Write2(ui16RegisterAddress, pData, ui8NoOfBytes);
		ui8Retry--;
	}
	
	if(ui8Success)
	{
		return(TRUE);
	}
	else
	{
		Serial.println("Comms write error");
		return(FALSE);
	}
}

uint8_t IQS550::I2C_Read(uint16_t ui16RegisterAddress, uint8_t *pData, uint8_t ui8NoOfBytes)
{
	uint8_t ui8Retry = 4;
	
	ui8Success = I2C_Read2(ui16RegisterAddress, pData, ui8NoOfBytes);
	//
	// If comms was not successful, retry 4 more times
	//
	while((!ui8Success) && (ui8Retry != 0))
	{
		delay(5);
		ui8Success = I2C_Read2(ui16RegisterAddress, pData, ui8NoOfBytes);
		ui8Retry--;
	}
	
	if(ui8Success)
	{
		return(TRUE);
	}
	else
	{
		Serial.println("Comms read error");
		return(FALSE);
	}
}

uint8_t IQS550::I2C_Write2(uint16_t ui16RegisterAddress, uint8_t *pData, uint8_t ui8NoOfBytes)
{
  Wire.beginTransmission(this->i2cDeviceAddr);
  Wire.write((uint8_t)(ui16RegisterAddress>>8));
  Wire.write((uint8_t)ui16RegisterAddress);
  Wire.write(pData, ui8NoOfBytes);
  Wire.endTransmission();
  return(TRUE);
}

uint8_t IQS550::I2C_Read2(uint16_t ui16RegisterAddress, uint8_t *pData, uint8_t ui8NoOfBytes)
{
  uint8_t i = 0;
  Wire.beginTransmission(this->i2cDeviceAddr);
  Wire.write((uint8_t)(ui16RegisterAddress>>8));
  Wire.write((uint8_t)ui16RegisterAddress);
  Wire.endTransmission(RESTART);
  
  do{
      Wire.requestFrom((uint8_t)this->i2cDeviceAddr, ui8NoOfBytes);
    }while(Wire.available() == 0);
    //Load bytes into array
    while(Wire.available())
    {
        pData[i] = Wire.read();
        i++;
    }


  return(TRUE);
}

void IQS550::RDY_wait() 
{
	while(digitalRead(this->ready_pin) == 0)
	{
        ;
	}
}

void IQS550::Close_Comms() 
{
  uint8_t ui8DataBuffer[1];
  
  I2C_Write(END_WINDOW, &ui8DataBuffer[0], 1);
}
