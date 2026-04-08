/**
  * @file    ArduCAM.c
  * @author  Arducam 
  * @version V0.1
  * @date    2018.06.18
  * @brief   Arducam mainly driver
  */

#include "arducam_conf.h"
#include "ArduCAM.h"
#include "spi.h"
#include "sccb_bus.h"
#include "ov2640_regs.h"

byte sensor_model = 0;
byte sensor_addr = 0;
byte m_fmt = JPEG;
uint32_t length = 0;
uint8_t is_header= false ;

void ArduCAM_Init(byte model) 
{
	switch (model)
  {
    case OV2640:
    case OV9650:
    case OV9655:
		{		
		  	wrSensorReg8_8(0xff, 0x01);
			wrSensorReg8_8(0x12, 0x80);
			if(m_fmt == JPEG)
			{
					wrSensorRegs8_8(OV2640_JPEG_INIT);
					wrSensorRegs8_8(OV2640_YUV422);
					wrSensorRegs8_8(OV2640_JPEG);
					wrSensorReg8_8(0xff, 0x01);
					wrSensorReg8_8(0x15, 0x00);
					// wrSensorRegs8_8(OV2640_640x480_JPEG);
					// wrSensorRegs8_8(OV2640_320x240_JPEG);
					wrSensorRegs8_8(OV2640_160x120_JPEG);
			}
			else
			{
				wrSensorRegs8_8(OV2640_QVGA);
			}
			break;
		}
		default:
			break;
  }
}
//CS init
void ArduCAM_CS_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure = {0};
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitStructure.Pin = CS_PIN;
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(CS_PORT, &GPIO_InitStructure);
    CS_HIGH();	
}

//Control the CS pin
void CS_HIGH(void)
{
 	HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_SET);					
}

void CS_LOW(void)
{
 	HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_RESET);					    
}

void set_format(byte fmt)
{
  if (fmt == BMP)
    m_fmt = BMP;
  else
    m_fmt = JPEG;
}

uint8_t bus_read(int address)
{
	uint8_t value;
   CS_LOW();
	 SPI2_ReadWriteByte(address);
	 value = SPI2_ReadWriteByte(0x00);
	 CS_HIGH();
	 return value;
}

uint8_t bus_write(int address,int value)
{	
	CS_LOW();HAL_Delay(10);
	SPI2_ReadWriteByte(address);
	SPI2_ReadWriteByte(value);
	HAL_Delay(10);
	CS_HIGH();
	return 1;
}

uint8_t read_reg(uint8_t addr)
{
	uint8_t data;
	data = bus_read(addr & 0x7F);
	return data;
}
void write_reg(uint8_t addr, uint8_t data)
{
	 bus_write(addr | 0x80, data); 
}

uint8_t read_fifo(void)
{
	uint8_t data;
	data = bus_read(SINGLE_FIFO_READ);
	return data;
}
void set_fifo_burst()
{
	SPI2_ReadWriteByte(BURST_FIFO_READ);
}


void flush_fifo(void)
{
	write_reg(ARDUCHIP_FIFO, FIFO_CLEAR_MASK);
}

void start_capture(void)
{
	write_reg(ARDUCHIP_FIFO, FIFO_START_MASK);
}

void clear_fifo_flag(void )
{
	write_reg(ARDUCHIP_FIFO, FIFO_CLEAR_MASK);
}

uint32_t read_fifo_length(void)
{
	uint32_t len1,len2,len3,len=0;
	len1 = read_reg(FIFO_SIZE1);
	len2 = read_reg(FIFO_SIZE2);
	len3 = read_reg(FIFO_SIZE3) & 0x7f;
	len = ((len3 << 16) | (len2 << 8) | len1) & 0x07fffff;
	return len;	
}

//Set corresponding bit  
void set_bit(uint8_t addr, uint8_t bit)
{
	uint8_t temp;
	temp = read_reg(addr);
	write_reg(addr, temp | bit);
}
//Clear corresponding bit 
void clear_bit(uint8_t addr, uint8_t bit)
{
	uint8_t temp;
	temp = read_reg(addr);
	write_reg(addr, temp & (~bit));
}

//Get corresponding bit status
uint8_t get_bit(uint8_t addr, uint8_t bit)
{
  uint8_t temp;
  temp = read_reg(addr);
  temp = temp & bit;
  return temp;
}

//Set ArduCAM working mode
//MCU2LCD_MODE: MCU writes the LCD screen GRAM
//CAM2LCD_MODE: Camera takes control of the LCD screen
//LCD2MCU_MODE: MCU read the LCD screen GRAM
void set_mode(uint8_t mode)
{
  switch (mode)
  {
    case MCU2LCD_MODE:
      write_reg(ARDUCHIP_MODE, MCU2LCD_MODE);
      break;
    case CAM2LCD_MODE:
      write_reg(ARDUCHIP_MODE, CAM2LCD_MODE);
      break;
    case LCD2MCU_MODE:
      write_reg(ARDUCHIP_MODE, LCD2MCU_MODE);
      break;
    default:
      write_reg(ARDUCHIP_MODE, MCU2LCD_MODE);
      break;
  }
}


void OV2640_set_JPEG_size(uint8_t size)
{
	switch(size)
	{
		case OV2640_160x120:
			wrSensorRegs8_8(OV2640_160x120_JPEG);
			break;
		case OV2640_176x144:
			wrSensorRegs8_8(OV2640_176x144_JPEG);
			break;
		case OV2640_320x240:
			wrSensorRegs8_8(OV2640_320x240_JPEG);
			break;
		case OV2640_352x288:
	  	wrSensorRegs8_8(OV2640_352x288_JPEG);
			break;
		case OV2640_640x480:
			wrSensorRegs8_8(OV2640_640x480_JPEG);
			break;
		case OV2640_800x600:
			wrSensorRegs8_8(OV2640_800x600_JPEG);
			break;
		case OV2640_1024x768:
			wrSensorRegs8_8(OV2640_1024x768_JPEG);
			break;
		case OV2640_1280x1024:
			wrSensorRegs8_8(OV2640_1280x1024_JPEG);
			break;
		case OV2640_1600x1200:
			wrSensorRegs8_8(OV2640_1600x1200_JPEG);
			break;
		default:
			wrSensorRegs8_8(OV2640_320x240_JPEG);
			break;
	}
}


byte wrSensorReg8_8(int regID, int regDat)
{
    if (sccb_write_reg(sensor_addr, (uint8_t)regID, (uint8_t)regDat) != HAL_OK)
        return 1;
    HAL_Delay(1);
    return 0;
}

byte rdSensorReg8_8(uint8_t regID, uint8_t* regDat)
{
    if (sccb_read_reg(sensor_addr, regID, regDat) != HAL_OK)
        return 1;
    return 0;
}

//I2C Array Write 8bit address, 8bit data
int wrSensorRegs8_8(const struct sensor_reg reglist[])
{
  int err = 0;
  uint16_t reg_addr = 0;
  uint16_t reg_val = 0;
  const struct sensor_reg *next = reglist;
  while ((reg_addr != 0xff) | (reg_val != 0xff))
  {
    reg_addr = next->reg;
    reg_val = next->val;
    err = wrSensorReg8_8(reg_addr, reg_val);
 //   HAL_Delay(400);
    next++;
  }

  return err;
}



