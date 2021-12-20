#include "eeprom_in_flash.h"

#if defined(STM32F103xE)
#define PAGE_SIZE     2048
#else
#define PAGE_SIZE     1024
#endif

//EEPROM_PART0 0x0800F000 - 0x0800F800 size 2048
//EEPROM_PART1 0x0800F800 - 0x08001000 size 2048

/* Erase from PART_BASE_ADDRESS, size EEPROM_PART_SIZE */
int EE_ErasePart(int part)
{
	uint32_t PageError = 0;
	uint32_t time_use = HAL_GetTick();
	int ret;
	uint32_t erase_address = part == 0 ? PART0_BASE_ADDRESS : PART1_BASE_ADDRESS;
	uint32_t erase_size = part == 0 ? EEPROM_PART0_SIZE : EEPROM_PART1_SIZE;

	if (erase_address < PART0_BASE_ADDRESS || erase_address >= PART1_END_ADDRESS)
		return HAL_ERROR;

	log_printf(LOG_Debug, "FLASHE Erase 0x%08x size %#x\r\n", erase_address, erase_size);

	HAL_FLASH_Unlock();

	FLASH_EraseInitTypeDef EraseInitStruct;
	EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
	EraseInitStruct.NbPages = erase_size / PAGE_SIZE;
	EraseInitStruct.PageAddress = erase_address;

	ret = HAL_FLASHEx_Erase(&EraseInitStruct, &PageError);
	if (ret != HAL_OK)
	{
		HAL_FLASH_Lock();
		log_printf(LOG_Error, "FLASHE Erase error %d at %#x\r\n", ret, PageError);
		return HAL_ERROR;
	}

	HAL_FLASH_Lock();
	log_printf(LOG_Verbose, "FLASHE Erase over, use %dms\r\n", HAL_GetTick() - time_use);
	return HAL_OK;
}

/* If the write operation is not a 32-bit atomic operation,
   Write the low 16-bit first, then write the high 16-bit */
int EE_ProgramWord(uint32_t Address, uint32_t Data)
{
	int ret;

	if (Address < PART0_BASE_ADDRESS || Address >= PART1_END_ADDRESS)
		return HAL_ERROR;

	log_printf(LOG_Debug, "FLASHE Program 0x%08x 0x%08x\r\n", Address, Data);

	HAL_FLASH_Unlock();

	ret = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, Address, Data);
	if (ret != HAL_OK)
	{
		HAL_FLASH_Lock();
		log_printf(LOG_Error, "FLASHE Program error %d\r\n", ret);
		return HAL_ERROR;
	}
	ret = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, Address + 2, Data >> 16);
	if (ret != HAL_OK)
	{
		HAL_FLASH_Lock();
		log_printf(LOG_Error, "FLASHE Program error %d\r\n", ret);
		return HAL_ERROR;
	}

	HAL_FLASH_Lock();
	return HAL_OK;
}
