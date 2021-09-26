#include <AP6408L.h>
#include "octospi.h"
#include "LOG.h"

void DLYB_OCTOSPI1_Calibration(uint8_t phase)
{
	int flag = 0;
	uint32_t LNGG = 0;
	uint32_t LNG[3];
	uint32_t UnitDelayNum = 0;
	uint32_t TimeOut = 0;

	/*校准前置 —— 复位OSPI延迟模块旁路位，设置自由运行时钟*/
	CLEAR_BIT(OCTOSPI1->DCR1, OCTOSPI_DCR1_DLYBYP);
	SET_BIT(OCTOSPI1->DCR1, OCTOSPI_DCR1_FRCK);

	/*配置延迟线长度为1个完整输入时钟周期*/
	DLYB_OCTOSPI1->CR |= 0x03;				//使能delay block和length sampling
	DLYB_OCTOSPI1->CFGR &= (~(0xf));			//设置SEL为12，即使能所有的Delay Unit
	DLYB_OCTOSPI1->CFGR |= 12;

	for (uint8_t i = 0; i < 128; i++)
	{
		DLYB_OCTOSPI1->CFGR &= (~(0x7f << 8)); 	//清零UNIT
		DLYB_OCTOSPI1->CFGR |= (i << 8);			//设置UNIT
		while (!((DLYB_OCTOSPI1->CFGR >> 31) & 0x01))		//LNGF被置为1
		{
			TimeOut++;
			if (TimeOut > 0xffff)
			{
				break;
			}
		}
		if (((DLYB_OCTOSPI1->CFGR >> 31) & 0x01))			//LNGF被置为1
		{
			flag = 1;
		}

		if (flag)
		{
			LNGG = (DLYB_OCTOSPI1->CFGR >> 16) & 0xfff;
			LNG[0] = LNGG & 0x7ff;
			LNG[1] = (LNGG >> 10) & 0x01;
			LNG[2] = (LNGG >> 11) & 0x01;
			if ((LNG[0] > 0) && ((LNG[1] == 0) || (LNG[2] == 0)))//判断Delay Line Length是否合理
			{
				//LOG("The Delay Line is set one input clock period\r\n");
				break;
			}
			else
			{
				flag = 0;
			}
		}
//		else
//			LOG("The Delay Line is set err\r\n");
	}

	/*确定有多少个Unit Delay，跨越一个输入时钟周期*/
	for (int8_t i = 10; i >= 0; i--)
	{
		if ((LNGG >> i) & 0x01)
		{
			UnitDelayNum = i;
			break;
		}
	}

	/*选择输出时钟相位*/
	DLYB_OCTOSPI1->CFGR &= (~(0xf));
	DLYB_OCTOSPI1->CFGR |= phase;

	//失能Sampler length enable bit
	DLYB_OCTOSPI1->CR &= (~(1 << 1));

	/*失能自由运行时钟*/
	SET_BIT(OCTOSPI1->CR, OCTOSPI_CR_ABORT);
	CLEAR_BIT(OCTOSPI1->DCR1, OCTOSPI_DCR1_FRCK);
}

/**
 * @brief  Writes an amount of data from the Psram_ device in polling mode.
 * @param  addr: Write start address
 * @param  buffer: Pointer to data to be written
 * @param  size: Size of written data from the memory
 */
void Psram_PageWrite(uint8_t *buffer, uint32_t addr, uint32_t size)
{
	OSPI_RegularCmdTypeDef sCommand;

	sCommand.OperationType = HAL_OSPI_OPTYPE_COMMON_CFG;
	sCommand.FlashId = HAL_OSPI_FLASH_ID_1;
	sCommand.InstructionMode = HAL_OSPI_INSTRUCTION_8_LINES;
	sCommand.InstructionSize = HAL_OSPI_INSTRUCTION_8_BITS;
	sCommand.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
	sCommand.AddressMode = HAL_OSPI_ADDRESS_8_LINES;
	sCommand.AddressSize = HAL_OSPI_ADDRESS_32_BITS;
	sCommand.AddressDtrMode = HAL_OSPI_ADDRESS_DTR_ENABLE;
	sCommand.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
	sCommand.DataMode = HAL_OSPI_DATA_8_LINES;
	sCommand.DataDtrMode = HAL_OSPI_DATA_DTR_ENABLE;
	sCommand.DQSMode = HAL_OSPI_DQS_ENABLE;
	sCommand.SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD;
	/* Writing Sequence ------------------------------------------------ */
	sCommand.Instruction = WRITE_CMD;
	sCommand.Address = addr;
	sCommand.NbData = size;
	sCommand.DummyCycles = DUMMY_CLOCK_CYCLES_WRITE;

	if (HAL_OSPI_Command(&hospi1, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE)
			!= HAL_OK)
	{
		Error_Handler();
	}

	if (HAL_OSPI_Transmit(&hospi1, buffer, HAL_OSPI_TIMEOUT_DEFAULT_VALUE)
			!= HAL_OK)
	{
		Error_Handler();
	}
}

/**
 * @brief  Reset Psram_
 */
void Psram_Reset(void)
{
	OSPI_RegularCmdTypeDef s_command =
	{ 0 };

	/* Initialize the reset enable command */
	s_command.OperationType = HAL_OSPI_OPTYPE_COMMON_CFG;
	s_command.FlashId = HAL_OSPI_FLASH_ID_1;
	s_command.InstructionMode = HAL_OSPI_INSTRUCTION_8_LINES;
	s_command.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
	s_command.InstructionSize = HAL_OSPI_INSTRUCTION_8_BITS;
	s_command.AddressMode = HAL_OSPI_ADDRESS_8_LINES;
	s_command.AddressSize = HAL_OSPI_ADDRESS_32_BITS;
	s_command.AddressDtrMode = HAL_OSPI_ADDRESS_DTR_ENABLE;
	s_command.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
	s_command.DataMode = HAL_OSPI_DATA_NONE;
	s_command.Instruction = 0xFF;
	s_command.Address = 0x00;
	s_command.DummyCycles = 0U;
	s_command.DQSMode = HAL_OSPI_DQS_ENABLE;
	s_command.SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD;

	/* Send the command */
	if (HAL_OSPI_Command(&hospi1, &s_command, 100) != HAL_OK)
	{

	}
}

/**
 * @brief  Reads an amount of data from the Psram_ device in polling mode.
 * @param  addr: Read start address
 * @param  buffer: Pointer to data to be read
 * @param  size: Size of read data from the memory
 */
void Psram_PageRead(uint8_t *buffer, uint32_t addr, uint32_t size)
{
	OSPI_RegularCmdTypeDef sCommand;

	sCommand.OperationType = HAL_OSPI_OPTYPE_COMMON_CFG;
	sCommand.FlashId = HAL_OSPI_FLASH_ID_1;
	sCommand.InstructionMode = HAL_OSPI_INSTRUCTION_8_LINES;
	sCommand.InstructionSize = HAL_OSPI_INSTRUCTION_8_BITS;
	sCommand.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
	sCommand.AddressMode = HAL_OSPI_ADDRESS_8_LINES;
	sCommand.AddressSize = HAL_OSPI_ADDRESS_32_BITS;
	sCommand.AddressDtrMode = HAL_OSPI_ADDRESS_DTR_ENABLE;
	sCommand.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
	sCommand.DataMode = HAL_OSPI_DATA_8_LINES;
	sCommand.DataDtrMode = HAL_OSPI_DATA_DTR_ENABLE;
	sCommand.DQSMode = HAL_OSPI_DQS_ENABLE;
	sCommand.SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD;
	/* reading Sequence ------------------------------------------------ */
	sCommand.Instruction = READ_CMD;
	sCommand.Address = addr;
	sCommand.NbData = size;
	sCommand.DummyCycles = DUMMY_CLOCK_CYCLES_READ;

	if (HAL_OSPI_Command(&hospi1, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE)
			!= HAL_OK)
	{
		Error_Handler();
	}

	if (HAL_OSPI_Receive(&hospi1, buffer, HAL_OSPI_TIMEOUT_DEFAULT_VALUE)
			!= HAL_OK)
	{
		Error_Handler();
	}
}

void Psram_RegRead(uint8_t *reg, uint32_t addr)
{
	OSPI_RegularCmdTypeDef sCommand;

	sCommand.OperationType = HAL_OSPI_OPTYPE_COMMON_CFG;
	sCommand.FlashId = HAL_OSPI_FLASH_ID_1;
	sCommand.InstructionMode = HAL_OSPI_INSTRUCTION_8_LINES;
	sCommand.InstructionSize = HAL_OSPI_INSTRUCTION_8_BITS;
	sCommand.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
	sCommand.AddressMode = HAL_OSPI_ADDRESS_8_LINES;
	sCommand.AddressSize = HAL_OSPI_ADDRESS_32_BITS;
	sCommand.AddressDtrMode = HAL_OSPI_ADDRESS_DTR_ENABLE;
	sCommand.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
	sCommand.DataMode = HAL_OSPI_DATA_8_LINES;
	sCommand.DataDtrMode = HAL_OSPI_DATA_DTR_ENABLE;
	sCommand.DQSMode = HAL_OSPI_DQS_ENABLE;
	sCommand.SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD;
	sCommand.Instruction = READ_REG_CMD_SRAM;
	sCommand.Address = addr;
	sCommand.NbData = 2;
	sCommand.DummyCycles = 5;

	if (HAL_OSPI_Command(&hospi1, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE)
			!= HAL_OK)
	{
		NVIC_SystemReset();
	}

	if (HAL_OSPI_Receive(&hospi1, reg, HAL_OSPI_TIMEOUT_DEFAULT_VALUE)
			!= HAL_OK)
	{
		NVIC_SystemReset();
	}
}

void Psram_RegWrite(uint8_t *reg, uint32_t addr)
{
	OSPI_RegularCmdTypeDef sCommand;

	sCommand.OperationType = HAL_OSPI_OPTYPE_COMMON_CFG;
	sCommand.FlashId = HAL_OSPI_FLASH_ID_1;
	sCommand.InstructionMode = HAL_OSPI_INSTRUCTION_8_LINES;
	sCommand.InstructionSize = HAL_OSPI_INSTRUCTION_8_BITS;
	sCommand.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
	sCommand.AddressMode = HAL_OSPI_ADDRESS_8_LINES;
	sCommand.AddressSize = HAL_OSPI_ADDRESS_32_BITS;
	sCommand.AddressDtrMode = HAL_OSPI_ADDRESS_DTR_ENABLE;
	sCommand.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
	sCommand.DataMode = HAL_OSPI_DATA_8_LINES;
	sCommand.DataDtrMode = HAL_OSPI_DATA_DTR_ENABLE;
	sCommand.DQSMode = HAL_OSPI_DQS_ENABLE;
	sCommand.SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD;
	sCommand.Instruction = WRITE_REG_CMD_SRAM;
	sCommand.Address = addr;
	sCommand.NbData = 2;
	sCommand.DummyCycles = 0;

	if (HAL_OSPI_Command(&hospi1, &sCommand, 50)
			!= HAL_OK)
	{
		NVIC_SystemReset();
	}

	if (HAL_OSPI_Transmit(&hospi1, reg, 50)
			!= HAL_OK)
	{
		NVIC_SystemReset();
	}
}

/* This function enables memory-mapped mode for Read and Write operations */
void EnableMemMapped(void)
{
	OSPI_RegularCmdTypeDef sCommand;
	OSPI_MemoryMappedTypeDef sMemMappedCfg;
	sCommand.FlashId = HAL_OSPI_FLASH_ID_1;
	sCommand.InstructionMode = HAL_OSPI_INSTRUCTION_8_LINES;
	sCommand.InstructionSize = HAL_OSPI_INSTRUCTION_8_BITS;
	sCommand.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
	sCommand.AddressMode = HAL_OSPI_ADDRESS_8_LINES;
	sCommand.AddressSize = HAL_OSPI_ADDRESS_32_BITS;
	sCommand.AddressDtrMode = HAL_OSPI_ADDRESS_DTR_ENABLE;
	sCommand.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
	sCommand.DataMode = HAL_OSPI_DATA_8_LINES;
	sCommand.DataDtrMode = HAL_OSPI_DATA_DTR_ENABLE;
	sCommand.DQSMode = HAL_OSPI_DQS_ENABLE;
	sCommand.SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD;
	sCommand.Address = 0;
	sCommand.NbData = 1;

	/* Memory-mapped mode configuration for Linear burst write operations */
	sCommand.OperationType = HAL_OSPI_OPTYPE_WRITE_CFG;
	sCommand.Instruction = WRITE_LINEAR_BURST_CMD;
	sCommand.DummyCycles = DUMMY_CLOCK_CYCLES_WRITE;
	if (HAL_OSPI_Command(&hospi1, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE)
			!= HAL_OK)
	{
		Error_Handler();
	}

	/* Memory-mapped mode configuration for Linear burst read operations */
	sCommand.OperationType = HAL_OSPI_OPTYPE_READ_CFG;
	sCommand.Instruction = READ_LINEAR_BURST_CMD;
	sCommand.DummyCycles = DUMMY_CLOCK_CYCLES_READ;
	if (HAL_OSPI_Command(&hospi1, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE)
			!= HAL_OK)
	{
		Error_Handler();
	}

	/*Disable timeout counter for memory mapped mode*/
	sMemMappedCfg.TimeOutActivation = HAL_OSPI_TIMEOUT_COUNTER_DISABLE;
	/*Enable memory mapped mode*/
	if (HAL_OSPI_MemoryMapped(&hospi1, &sMemMappedCfg) != HAL_OK)
	{
		Error_Handler();
	}
}

void Psram_MapWrite_U8(uint8_t *buffer, uint32_t addr, uint32_t size)
{
#if (CHECK_LEGAL == 1)
	if( (addr + size) > 8388608/* 8*1024*1024 */ || buffer == NULL)	return;
#endif
	volatile uint8_t *p = (volatile uint8_t*) (MAP_BASE_ADDR + addr);
	for (unsigned int i = 0; i < size; i++)
	{
		*p = buffer[i];
		p++;
	}
}

void Psram_MapRead_U8(uint8_t *buffer, uint32_t addr, uint32_t size)
{
#if (CHECK_LEGAL == 1)
	if( (addr + size) > 8388608/* 8*1024*1024 */ || buffer == NULL)	return;
#endif
	volatile uint8_t *p = (volatile uint8_t*) (MAP_BASE_ADDR + addr);
	for (uint32_t i = 0; i < size; i++)
	{
		buffer[i] = *p;
		p++;
	}
}

void Psram_MapWrite_U16(uint16_t *buffer, uint32_t addr, uint32_t size)
{
#if (CHECK_LEGAL == 1)
	if( (addr + size*2) > 8388608/* 8*1024*1024 */ || buffer == NULL)	return;
#endif
	volatile uint16_t *p = (volatile uint16_t*) (MAP_BASE_ADDR + addr);
	for (uint32_t i = 0; i < size; i++)
	{
		*p = buffer[i];
		p++;
	}
}

void Psram_MapRead_U16(uint16_t *buffer, uint32_t addr, uint32_t size)
{
#if (CHECK_LEGAL == 1)
	if( (addr + size*2) > 8388608/* 8*1024*1024 */ || buffer == NULL)	return;
#endif
	volatile uint16_t *p = (volatile uint16_t*) (MAP_BASE_ADDR + addr);
	for (uint32_t i = 0; i < size; i++)
	{
		buffer[i] = *p;
		p++;
	}
}

void Psram_MapWrite_U32(uint32_t *buffer, uint32_t addr, uint32_t size)
{
#if (CHECK_LEGAL == 1)
	if( (addr + size*4) > 8388608/* 8*1024*1024 */ || buffer == NULL)	return;
#endif
	volatile uint32_t *p = (volatile uint32_t*) (MAP_BASE_ADDR + addr);
	for (uint32_t i = 0; i < size; i++)
	{
		*p = buffer[i];
		p++;
	}
}

void Psram_MapRead_U32(uint32_t *buffer, uint32_t addr, uint32_t size)
{
#if (CHECK_LEGAL == 1)
	if( (addr + size*4) > 8388608/* 8*1024*1024 */ || buffer == NULL)	return;
#endif
	volatile uint32_t *p = (volatile uint32_t*) (MAP_BASE_ADDR + addr);
	for (uint32_t i = 0; i < size; i++)
	{
		buffer[i] = *p;
		p++;
	}
}

void Psram_MapFastWrite_U8(uint8_t *buffer, uint32_t addr, uint32_t size)
{
	if (size <= FAST_WD_U8_NUM)
	{
		Psram_MapWrite_U8(buffer, addr, size);
		return;
	}

	volatile uint64_t *p_fast_addr = (volatile uint64_t*) (MAP_BASE_ADDR + addr);
	uint32_t remains = size & (FAST_WD_U8_NUM - 1);
	uint32_t count = (size - remains) / FAST_WD_U8_NUM;
	uint64_t *p_fast_buff = (uint64_t*) buffer;

	while (count)
	{
		*p_fast_addr = *p_fast_buff;
		p_fast_addr++;
		p_fast_buff++;
		count--;
	}

	if (remains)
	{
		volatile uint8_t *p_addr = (volatile uint8_t*) p_fast_addr;
		uint8_t *p_buff = (volatile uint8_t*) p_fast_buff;
		Psram_MapWrite_U8(p_buff, p_addr, remains);
	}
}

void Psram_MapFastWrite_U16(uint16_t *buffer, uint32_t addr, uint32_t size)
{
	if (size <= FAST_WD_U16_NUM)
	{
		Psram_MapWrite_U16(buffer, addr, size);
		return;
	}

	volatile uint64_t *p_fast_addr = (volatile uint64_t*) (MAP_BASE_ADDR + addr);
	uint32_t remains = size & (FAST_WD_U16_NUM - 1);
	uint32_t count = (size - remains) / FAST_WD_U16_NUM;
	uint64_t *p_fast_buff = (uint64_t*) buffer;

	while (count)
	{
		*p_fast_addr = *p_fast_buff;
		p_fast_addr++;
		p_fast_buff++;
		count--;
	}

	if (remains)
	{
		volatile uint16_t *p_addr = (volatile uint16_t*) p_fast_addr;
		uint16_t *p_buff = (volatile uint16_t*) p_fast_buff;
		Psram_MapWrite_U16(p_buff, p_addr, remains);
	}
}

void Psram_MapFastWrite_U32(uint32_t *buffer, uint32_t addr, uint32_t size)
{
	if (size <= FAST_WD_U32_NUM)
	{
		Psram_MapWrite_U32(buffer, addr, size);
		return;
	}

	volatile uint64_t *p_fast_addr = (volatile uint64_t*) (MAP_BASE_ADDR + addr);
	uint32_t remains = size & (FAST_WD_U32_NUM - 1);
	uint32_t count = (size - remains) / FAST_WD_U32_NUM;
	uint64_t *p_fast_buff = (uint64_t*) buffer;

	while (count)
	{
		*p_fast_addr = *p_fast_buff;
		p_fast_addr++;
		p_fast_buff++;
		count--;
	}

	if (remains)
	{
		volatile uint32_t *p_addr = (volatile uint32_t*) p_fast_addr;
		uint32_t *p_buff = (volatile uint32_t*) p_fast_buff;
		Psram_MapWrite_U32(p_buff, p_addr, remains);
	}
}

void Psram_MapFastRead_U8(uint8_t *buffer, uint32_t addr, uint32_t size)
{
	if (size <= FAST_WD_U8_NUM)
	{
		Psram_MapRead_U8(buffer, addr, size);
		return;
	}

	volatile uint64_t *p_fast_addr = (volatile uint64_t*) (MAP_BASE_ADDR + addr);
	uint32_t remains = size & (FAST_WD_U8_NUM - 1);
	uint32_t count = (size - remains) / FAST_WD_U8_NUM;
	uint64_t *p_fast_buff = (uint64_t*) buffer;

	while (count)
	{
		*p_fast_buff = *p_fast_addr;
		p_fast_addr++;
		p_fast_buff++;
		count--;
	}

	if (remains)
	{
		volatile uint8_t *p_addr = (volatile uint8_t*) p_fast_addr;
		uint8_t *p_buff = (volatile uint8_t*) p_fast_buff;
		Psram_MapRead_U8(p_buff, p_addr, remains);
	}
}

void Psram_Set_DQS(enum DQS_Strength_e DQS_Strength)
{
	uint8_t reg[2] =
	{ 0x00, 0x00 };
	uint8_t regs;
	switch (DQS_Strength)
	{
	case DQS_Half:
		regs = 0x08;
		break;
	case DQS_One_Fourth:
		regs = 0x09;
		break;
	case DQS_One_Eighth:
		regs = 0x0A;
		break;
	case DQS_One_Sixteenth:
		regs = 0x0B;
		break;
	default:
		regs = 0x09;
		break;
	}
	while (reg[0] != regs)
	{
		Psram_RegWrite(&regs, 0);
		HAL_Delay(1);
		Psram_RegRead(reg, 0);
	}
}


void Psram_Init(void)
{
	DLYB_OCTOSPI1_Calibration(1);
	HAL_Delay(5);
	Psram_Reset();
	HAL_Delay(5);
	Psram_Set_DQS(DQS_Half);
	HAL_Delay(5);
	EnableMemMapped();
}

