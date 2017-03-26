/*
 * OneWire.c
 *
 *  Created on: 25-Mar-2017
 *      Author: rishabh
 */

#include "OneWire.h"

OW_Struct myOW_struct;
OW_Struct *myOW = &myOW_struct;

GPIO_InitTypeDef GPIO_InitStruct;
GPIO_InitTypeDef *ptr = &GPIO_InitStruct;
void OW_Delay(uint32_t delay)
{
	ticks = __HAL_TIM_GET_COUNTER(&htim3);
	HAL_TIM_Base_Start(&htim3);
	while(__HAL_TIM_GET_COUNTER(&htim3) - ticks <= delay)
	{

	}
	HAL_TIM_Base_Stop(&htim3);
}


void OW_Init()
{

	  /* GPIO Ports Clock Enable */
	  __HAL_RCC_GPIOC_CLK_ENABLE();

	  /*Configure GPIO pin Output Level */
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET);

	  /*Configure GPIO pin : PC1 */
	  GPIO_InitStruct.Pin = GPIO_PIN_1;
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
	  GPIO_InitStruct.Pull = GPIO_PULLUP;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}


void OW_Input()
{
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_TypeDef  *GPIOx = GPIOC;
	GPIO_InitTypeDef *GPIO_Init = ptr;
	uint32_t position;
	uint32_t ioposition = 0x00U;
	uint32_t iocurrent = 0x00U;
	uint32_t temp = 0x00U;

	/* Configure the port pins */
	for(position = 0U; position < GPIO_NUMBER; position++)
	{
		/* Get the IO position */
		ioposition = ((uint32_t)0x01U) << position;
		/* Get the current IO position */
		iocurrent = (uint32_t)(GPIO_Init->Pin) & ioposition;

		if(iocurrent == ioposition)
		{
		  /* Configure IO Direction mode (Input, Output, Alternate or Analog) */
		  temp = GPIOx->MODER;
		  temp &= ~(GPIO_MODER_MODER0 << (position * 2U));
		  temp |= ((GPIO_MODE_INPUT & GPIO_MODE) << (position * 2U));
		  GPIOx->MODER = temp;
		}
	}
}

void OW_Output()
{
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_TypeDef  *GPIOx = GPIOC;
	GPIO_InitTypeDef *GPIO_Init = ptr;
	uint32_t position;
	uint32_t ioposition = 0x00U;
	uint32_t iocurrent = 0x00U;
	uint32_t temp = 0x00U;

	/* Configure the port pins */
	for(position = 0U; position < GPIO_NUMBER; position++)
	{
		/* Get the IO position */
		ioposition = ((uint32_t)0x01U) << position;
		/* Get the current IO position */
		iocurrent = (uint32_t)(GPIO_Init->Pin) & ioposition;

		if(iocurrent == ioposition)
		{
		  /* Configure IO Direction mode (Input, Output, Alternate or Analog) */
		  temp = GPIOx->MODER;
		  temp &= ~(GPIO_MODER_MODER0 << (position * 2U));
		  temp |= ((GPIO_MODE_OUTPUT_OD & GPIO_MODE) << (position * 2U));
		  GPIOx->MODER = temp;
		}
	}
}

void OW_PinLow()
{
	GPIOC->BSRR = (uint32_t)GPIO_PIN_1 << 16U;
}

void OW_PinRelease()
{
	GPIOC->BSRR = GPIO_PIN_1;
}

uint8_t OW_Reset()
{
	uint8_t ret;
	OW_PinLow();
	OW_Output();
	OW_Delay(500);

	OW_PinRelease();
	OW_Input();
	OW_Delay(70);

	ret = (GPIOC->IDR & GPIO_PIN_1);
	if(ret == GPIO_PIN_SET)
	{
		ret = 1;
	}
	if(ret == GPIO_PIN_RESET)
	{
		ret = 0;
	}

return ret;
}

void OW_WriteBit(uint8_t bit)
{
	if(bit)
	{
		OW_PinLow();
		OW_Output();
		OW_Delay(10);

		OW_Input();

		OW_Delay(55);
		OW_Input();
	}
	else
	{
		OW_PinLow();
		OW_Output();
		OW_Delay(65);

		OW_Input();

		OW_Delay(5);
		OW_Input();

	}
}


uint8_t OW_ReadBit()
{
	uint8_t bit = 0;

	OW_PinLow();
	OW_Output();
	OW_Delay(3);

	OW_Input();
	OW_Delay(10);

	if((GPIOC->IDR & GPIO_PIN_1))
	{
		bit = 1;
	}

	OW_Delay(50);

	return bit;
}

void OW_WriteByte(uint8_t byte)
{
	uint8_t i = 8;
	while(i--)
	{
		OW_WriteBit(byte & 0x01);
		byte >>=1;
	}
}


uint8_t OW_ReadByte()
{
	uint8_t i = 8;
	uint8_t byte;
	while(i--)
	{
		byte >>=1;
		byte |= (OW_ReadBit() << 7);
	}

	return byte;
}

void OW_ResetSearch() {
	/* Reset the search state */
	myOW->LastDiscrepancy = 0;
	myOW->LastDeviceFlag = 0;
	myOW->LastFamilyDiscrepancy = 0;
}


/****************************************
 *
 *  __________________________________________________________
 * |MSB   64-bit "Registration" ROM Number                 LSB|
 * |----------------------------------------------------------|
 * | 8-Bit CRC   |   48-bit serial number  | 8-bit Family Code|
 * |MSB      LSB |MSB                  LSB |MSB            LSB|
 * |-------------|-------------------------|------------------|
 *
 * */



uint8_t OW_Search(uint8_t command)
{
	uint8_t id_bit_number;
	uint8_t last_zero, rom_byte_number, search_result;
	uint8_t id_bit, cmp_id_bit;
	uint8_t rom_byte_mask, search_direction;

	id_bit_number = 1;
	last_zero = 0;
	rom_byte_mask =1;
	rom_byte_number = 0;
	search_result = 0;

	if(!myOW->LastDeviceFlag) {
		//Perform a 1-Wire Reset
		if(OW_Reset()) {
			myOW->LastDiscrepancy =0;
			myOW->LastDeviceFlag = 0;
			myOW->LastFamilyDiscrepancy = 0;
			return 0;
		}

		//Send a search command
		OW_WriteByte(command);
		do
		{
			id_bit = OW_ReadBit();
			cmp_id_bit = OW_ReadBit();

			//Find the number of devices on the bus
			if((id_bit == 1) && (cmp_id_bit == 1)) {
				break;
			}
			else
			{
				//all devices coupled have 0 or 1
				if(id_bit != cmp_id_bit) {
					search_direction = id_bit; //bit write value for search
				}
				else
				{
					// if this discrepancy if before the Last Discrepancy
					// on a previous next then pick the same as last time
					if(id_bit_number < myOW->LastDiscrepancy)
					{
						search_direction = ((myOW->ROM_No[rom_byte_number] & rom_byte_mask) > 0);
					}
					else
					{
						search_direction = (id_bit_number == myOW->LastDiscrepancy);
					}

					if(search_direction == 0)
					{
						last_zero = id_bit_number;

						if(last_zero < 9) {
							myOW->LastFamilyDiscrepancy = last_zero;
						}
					}
				}

				//Set or clear the bit in the ROM byte rom_byte_number
				// with mask rom_byte_mask
				if(search_direction == 1) {
					myOW->ROM_No[rom_byte_number] |= rom_byte_mask;
				}else {
					myOW->ROM_No[rom_byte_number] &= ~rom_byte_mask;
				}

				OW_WriteBit(search_direction);

				id_bit_number++;
				rom_byte_mask <<= 1;

				if(rom_byte_mask == 0) {
					rom_byte_number++;
					rom_byte_mask = 1;
				}
			}
		}while(rom_byte_number < 8);


		if(!(id_bit_number < 65)) {
			myOW->LastDiscrepancy = last_zero;

			if(myOW->LastDiscrepancy == 0) {
				myOW->LastDeviceFlag = 1;
			}

			search_result = 1;
		}

	}

	if (!search_result || !myOW->ROM_No[0]) {
		myOW->LastDiscrepancy = 0;
		myOW->LastDeviceFlag = 0;
		myOW->LastFamilyDiscrepancy = 0;
		search_result = 0;
	}

	return search_result;

}


uint8_t OW_Next() {
   /* Leave the search state alone */
   return OW_Search(ONEWIRE_CMD_SEARCHROM);
}

uint8_t OW_First() {
	/* Reset search values */
	OW_ResetSearch();

	/* Start with searching */
	return OW_Search(ONEWIRE_CMD_SEARCHROM);
}


int OW_Verify() {
	unsigned char rom_backup[8];
	int i,rslt,ld_backup,ldf_backup,lfd_backup;

	// keep a backup copy of the current state
	for (i = 0; i < 8; i++)
		rom_backup[i] = myOW->ROM_No[i];
	ld_backup = myOW->LastDiscrepancy;
	ldf_backup = myOW->LastDeviceFlag;
	lfd_backup = myOW->LastFamilyDiscrepancy;

	// set search to find the same device
	myOW->LastDiscrepancy = 64;
	myOW->LastDeviceFlag = 0;

	if (OW_Search(ONEWIRE_CMD_SEARCHROM)) {
		// check if same device found
		rslt = 1;
		for (i = 0; i < 8; i++) {
			if (rom_backup[i] != myOW->ROM_No[i]) {
				rslt = 1;
				break;
			}
		}
	} else {
		rslt = 0;
	}

	// restore the search state
	for (i = 0; i < 8; i++) {
		myOW->ROM_No[i] = rom_backup[i];
	}
	myOW->LastDiscrepancy = ld_backup;
	myOW->LastDeviceFlag = ldf_backup;
	myOW->LastFamilyDiscrepancy = lfd_backup;

	// return the result of the verify
	return rslt;
}


void OW_TargetSetup(uint8_t family_code) {
   uint8_t i;

	// set the search state to find SearchFamily type devices
	myOW->ROM_No[0] = family_code;
	for (i = 1; i < 8; i++) {
		myOW->ROM_No[i] = 0;
	}

	myOW->LastDiscrepancy = 64;
	myOW->LastFamilyDiscrepancy = 0;
	myOW->LastDeviceFlag = 0;
}

void OW_FamilySkipSetup() {

	// set the Last discrepancy to last family discrepancy
	myOW->LastDiscrepancy = myOW->LastFamilyDiscrepancy;
	myOW->LastFamilyDiscrepancy = 0;

	// check for end of list
	if (myOW->LastDiscrepancy == 0) {
		myOW->LastDeviceFlag = 1;
	}
}

uint8_t OW_GetROM(uint8_t index) {
	return myOW->ROM_No[index];
}

void OW_Select(uint8_t* addr) {
	uint8_t i;
	OW_WriteByte(ONEWIRE_CMD_MATCHROM);

	for (i = 0; i < 8; i++) {
		OW_WriteByte(*(addr + i));
	}
}


void OW_SelectWithPointer(uint8_t *ROM) {
	uint8_t i;
	OW_WriteByte(ONEWIRE_CMD_MATCHROM);

	for (i = 0; i < 8; i++) {
		OW_WriteByte(*(ROM + i));
	}
}

void OW_GetFullROM(uint8_t *firstIndex) {
	uint8_t i;
	for (i = 0; i < 8; i++) {
		*(firstIndex + i) = myOW->ROM_No[i];
	}
}

uint8_t TM_OneWire_CRC8(uint8_t *addr, uint8_t len) {
	uint8_t crc = 0, inbyte, i, mix;

	while (len--) {
		inbyte = *addr++;
		for (i = 8; i; i--) {
			mix = (crc ^ inbyte) & 0x01;
			crc >>= 1;
			if (mix) {
				crc ^= 0x8C;
			}
			inbyte >>= 1;
		}
	}

	/* Return calculated CRC */
	return crc;
}
