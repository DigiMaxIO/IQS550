#ifndef __IQS550_H__
#define __IQS550_H__

#include <Wire.h>
#include <Arduino.h>
#include <registers.h>
#include <stdint.h>

#define IQS550_DEFAULT_ADDR          	0x74
#define RDY_PIN_DEFAULT             	3
#define	END_WINDOW				(uint16_t)0xEEEE

#define BitIsSet(VAR,Index)		(VAR & (1<<Index)) != 0
#define BitIsNotSet(VAR,Index)	(VAR & (1<<Index)) == 0

#define SetBit(VAR,Index)		VAR |= (1<<Index)
#define ClearBit(VAR,Index)		VAR &= (uint8_t)(~(1<<Index))

#define FALSE 					0
#define	TRUE 					!FALSE

#define START           0x08
#define REPEATED_START  0x10
#define MT_SLA_ACK		0x18
#define MT_SLA_NACK		0x20
#define MT_DATA_ACK     0x28
#define MT_DATA_NACK    0x30
#define MR_SLA_ACK		  0x40
#define MR_SLA_NACK		  0x48
#define MR_DATA_ACK     0x50
#define MR_DATA_NACK    0x58
#define LOST_ARBTRTN    0x38

#define STOP                              true    
#define RESTART                           false



typedef struct {
   uint16_t    product;
   uint16_t    project;
	 uint8_t 		 major_version;
	 uint8_t 		 minor_version;
} IQS_Version;


class IQS550
{
public:


	IQS550(uint8_t address, uint8_t readyPin, uint8_t resetPin);
	void begin();
	void AcknowledgeReset(); 
	void CheckVersion();
	void DisplaySnap();
	void Process_XY();
	void Print_signed(int16_t num);
	void Print_unsigned(uint16_t num);
	void I2C_Setup();
	uint8_t I2C_Write(uint16_t registerAddress, uint8_t *data, uint8_t numberBytes);
	uint8_t I2C_Read(uint16_t registerAddress, uint8_t *data, uint8_t numberBytes);
	uint8_t I2C_Write2(uint16_t ui16RegisterAddress, uint8_t *pData, uint8_t ui8NoOfBytes);
	uint8_t I2C_Read2(uint16_t ui16RegisterAddress, uint8_t *pData, uint8_t ui8NoOfBytes);
	void RDY_wait();
	void Close_Comms();
	
	
private:
	uint8_t i2cDeviceAddr;
	uint8_t ready_pin;
	uint8_t reset_pin;
	uint8_t x;
	uint8_t y;
};


#endif
