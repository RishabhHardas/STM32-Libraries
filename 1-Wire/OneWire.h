/*
 * OneWire.h
 *
 *  Created on: 25-Mar-2017
 *      Author: rishabh
 */

#ifndef ONEWIRE_H_
#define ONEWIRE_H_

#include "stm32f4xx_hal.h"
#include <tim.h>


typedef struct {
	GPIO_TypeDef GPIOx;
	uint16_t GPIO_Pin;
	uint8_t LastDiscrepancy;
	uint8_t LastFamilyDiscrepancy;
	uint8_t LastDeviceFlag;
	uint8_t ROM_No[8];
} OW_Struct;


#define DELAY_A	6
#define DELAY_B	64
#define DELAY_C	60
#define DELAY_D	10
#define DELAY_E	9
#define DELAY_F	55
#define DELAY_G	0
#define DELAY_H	480
#define DELAY_I	70
#define DELAY_J	410

#define GPIO_MODE             ((uint32_t)0x00000003U)
#define EXTI_MODE             ((uint32_t)0x10000000U)
#define GPIO_MODE_IT          ((uint32_t)0x00010000U)
#define GPIO_MODE_EVT         ((uint32_t)0x00020000U)
#define RISING_EDGE           ((uint32_t)0x00100000U)
#define FALLING_EDGE          ((uint32_t)0x00200000U)
#define GPIO_OUTPUT_TYPE      ((uint32_t)0x00000010U)

#define GPIO_NUMBER           ((uint32_t)16U)


/* OneWire commands */
#define ONEWIRE_CMD_RSCRATCHPAD			0xBE
#define ONEWIRE_CMD_WSCRATCHPAD			0x4E
#define ONEWIRE_CMD_CPYSCRATCHPAD		0x48
#define ONEWIRE_CMD_RECEEPROM			0xB8
#define ONEWIRE_CMD_RPWRSUPPLY			0xB4
#define ONEWIRE_CMD_SEARCHROM			0xF0
#define ONEWIRE_CMD_READROM				0x33
#define ONEWIRE_CMD_MATCHROM			0x55
#define ONEWIRE_CMD_SKIPROM	0xCC




extern uint32_t ticks;

void OW_Delay(uint32_t delay);
void OW_Init(void);
void OW_Input(void);
void OW_Output(void);
void OW_PinLow(void);
void OW_PinRelease(void);
uint8_t OW_Reset(void);
void OW_WriteBit(uint8_t bit);
uint8_t OW_ReadBit(void);
void OW_WriteByte(uint8_t byte);
uint8_t OW_ReadByte(void);
void OW_ResetSearch(void);
uint8_t OW_Search(uint8_t command);
uint8_t OW_Next(void);
uint8_t OW_First(void);
int OW_Verify(void);
void OW_TargetSetup(uint8_t family_code);
void OW_FamilySkipSetup(void);
uint8_t OW_GetROM(uint8_t index);
void OW_Select(uint8_t* addr);
void OW_SelectWithPointer(uint8_t *ROM);
void OW_GetFullROM(uint8_t *firstIndex);
uint8_t TM_OneWire_CRC8(uint8_t *addr, uint8_t len);


#endif /* ONEWIRE_H_ */
