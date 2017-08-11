#ifndef  __BSP_QSPI_N25Q_H
#define  __BSP_QSPI_N25Q_H



/*
***********************************************************************************************
QSPI N25Q256Axx  编程、擦除时间
-----------------------------------------------------------------------------------
   DESCRIPTION                         |          SPEC            |     UNIT
                                       |      MIN      MAX        |
-----------------------------------------------------------------------------------
/CS High to xxxx Time                  |               30         |      us
-----------------------------------------------------------------------------------
Write Status Reg Time                  |      10       8          |      ms
-----------------------------------------------------------------------------------                                            
Page Program Time                      |      0.7       5         |      ms
-----------------------------------------------------------------------------------   
Subsector ERASE cycle time             |      0.25      0.8       |      s   
----------------------------------------------------------------------------------- 
Sector ERASE cycle time                |      0.7       3         |      s    
-----------------------------------------------------------------------------------                       
Bulk ERASE cycle time                  |     240        480       |      s


***********************************************************************************************
*/


//#include "bsp_qspi.h"

#include "stm32f7xx.h"

#define QSPI_N25Q256A_TOTAL_SIZE   ( (uint32_t) 0x2000000)  
#define QSPI_N25Q512A_TOTAL_SIZE   ( (uint32_t) 0x4000000)   
#define QSPI_MT25Q1GB_TOTAL_SIZE   ( (uint32_t) 0x8000000) 

/* mt25q1gb Micron memory   #define QSPI_FLASH_SIZE			25 */

 #define QSPI_FLASH_SIZE			26

#define QSPI_FLASH_SIZE_LINE		(POSITION_VAL(QSPI_MT25Q1GB_TOTAL_SIZE)-1)  /* 
                                                           * QSPI FLASH大小，	使用二进制位数的个数表示，N25Q512A大小为64M字节
                                                           * 总共位26位地址线。因为程序内部会自动增加一位，所以此处是25位地址线，
                                                           * 所以使用POSITION_VAL 这个函数计算得出后的结果-1得出是25
                                                           */
										
#define N25Q_SPI_MODE     0
#define N25Q_QUAD_MODE    1


typedef enum
{
  QSPI_SEND_CMD = 0xEA,
  QSPI_SEND_DAT

} __SEND_CMD_DATA_T;


typedef enum
{
  QSPI_N25Q256_JEDEC_ID = 0x20BA19,    // N25Q256A   ID
  QSPI_N25Q512_JEDEC_ID = 0x20BA20,    // N25Q512A   ID
  QSPI_MT25Q1GB_JEDEC_ID =0x20BA21		 //MT25Q1GB   ID

}  __QSPI_JEDEC_ID;


//QSPI Status Bits
typedef enum
{
  QSPI_OK				= ((uint8_t)0x00),
  QSPI_ERROR			= ((uint8_t)0x01),
  QSPI_BUSY				= ((uint8_t)0x02),
  QSPI_NOT_SUPPORTED	= ((uint8_t)0x03),
  QSPI_SUSPENDED		= ((uint8_t)0x04),
  QSPI_OUT_TIME			= ((uint8_t)0x05),
  QSPI_WRITE_ENABLE_ERROR	  = ((uint8_t)0x06),
  
} QSPI_StaticTypeDef;


//QSPI Information Structure
typedef struct {
  uint32_t FlashTotalSize;		//Size of the flash Total
  uint32_t SectorSize;			//Size of sectors for operation
  uint32_t EraseSectorSize;		//Size of sectors for the erase operation
  uint32_t EraseSectorsNumber;	//Number of sectors for the erase operation
  uint32_t ProgPageSize;		//Size of pages for the program operation
  uint32_t ProgPagesNumber;		//Number of pages for the program operation
  uint32_t Id;                  // 芯片 ID 号
} QSPI_Information;


extern QSPI_Information  _QspiFlashInf;

#define QSPI_DUMMY_CYCLES_READ				    			8
#define QSPI_DUMMY_CYCLES_READ_QUAD	            10

#define QSPI_FAST_READ_MAX_TIME			    	((uint32_t)250000)
#define QSPI_REG_READ_MAX_TIME			    	((uint32_t)300)
#define QSPI_BULK_ERASE_MAX_TIME					((uint32_t)250000)
#define QSPI_SECTOR_ERASE_MAX_TIME		    ((uint32_t)3000)            // block
#define QSPI_SUBSECTOR_ERASE_MAX_TIME	    ((uint32_t)800)             // sector

#define QSPI_WAIT_MAX_TIME	              ((uint32_t)0x3FFFFFF)
#define QSPI_QUAD_ENTER_MAX_TIME	        QSPI_WAIT_MAX_TIME


/* End address of the QSPI memory */
#define QSPI_END_ADDR              				(1 << QSPI_FLASH_SIZE)														   
#define QSPI_PAGE_SIZE			  						(256)
#define QSPI_SUBSECTOR_SIZE								(32768)		  // 4096 subsectors of 32kBytes


/* Reset Operations */
#define QSPI_RESET_ENABLE_CMD                     0x66
#define QSPI_RESET_MEMORY_CMD                     0x99

/* Identification Operations */
#define QSPI_READ_ID_CMD                          0x9E  // 支持 Extended 模式
#define QSPI_READ_ID_CMD2                         0x9F  // 支持 Extended 模式

#define QSPI_MULTIPLE_IO_READ_ID_CMD              0xAF  // 支持 Dual、Quad 模式

#define QSPI_READ_SERIAL_FLASH_DISCO_PARAM_CMD    0x5A  // 支持 Extended、Dual、Quad 模式

/* Read Operations */
#define QSPI_READ_CMD                             0x03
#define QSPI_READ_4_BYTE_ADDR_CMD                 0x13

#define QSPI_FAST_READ_CMD                        0x0B  // ok
#define QSPI_FAST_READ_DTR_CMD                    0x0D
#define QSPI_FAST_READ_4_BYTE_ADDR_CMD            0x0C  // ok Address bytes = 4. Dummy clock cycles default = 8. Dummy clock cycles default = 10

#define QSPI_DUAL_OUT_FAST_READ_CMD               0x3B
#define QSPI_DUAL_OUT_FAST_READ_DTR_CMD           0x3D
#define QSPI_DUAL_OUT_FAST_READ_4_BYTE_ADDR_CMD   0x3C

#define QSPI_DUAL_INOUT_FAST_READ_CMD             0xBB
#define QSPI_DUAL_INOUT_FAST_READ_DTR_CMD         0xBD
#define QSPI_DUAL_INOUT_FAST_READ_4_BYTE_ADDR_CMD 0xBC

#define QSPI_QUAD_OUT_FAST_READ_CMD               0x6B  // ok
#define QSPI_QUAD_OUT_FAST_READ_DTR_CMD           0x6D
#define QSPI_QUAD_OUT_FAST_READ_4_BYTE_ADDR_CMD   0x6C  // ok Address bytes = 4. Dummy clock cycles default = 8. Dummy clock cycles default = 10

#define QSPI_QUAD_INOUT_FAST_READ_CMD             0xEB  // ok
#define QSPI_QUAD_INOUT_FAST_READ_DTR_CMD         0xED
#define QSPI_QUAD_INOUT_FAST_READ_4_BYTE_ADDR_CMD 0xEC  // ok Address bytes = 4. Dummy clock cycles default = 10. 

/* Write Operations */
#define QSPI_WRITE_ENABLE_CMD                     0x06
#define QSPI_WRITE_DISABLE_CMD                    0x04

/* Register Operations */
#define QSPI_READ_STATUS_REG_CMD                  0x05
#define QSPI_WRITE_STATUS_REG_CMD                 0x01

#define QSPI_READ_LOCK_REG_CMD                    0xE8
#define QSPI_WRITE_LOCK_REG_CMD                   0xE5

#define QSPI_READ_FLAG_STATUS_REG_CMD             0x70
#define QSPI_CLEAR_FLAG_STATUS_REG_CMD            0x50

#define QSPI_READ_NONVOL_CFG_REG_CMD              0xB5  //READ  NONVOLATILE CONFIGURATION REGISTER
#define QSPI_WRITE_NONVOL_CFG_REG_CMD             0xB1  //WRITE NONVOLATILE CONFIGURATION REGISTER

#define QSPI_READ_VOL_CFG_REG_CMD                 0x85  //READ  VOLATILE CONFIGURATION REGISTER
#define QSPI_WRITE_VOL_CFG_REG_CMD                0x81  //WRITE VOLATILE CONFIGURATION REGISTER

#define QSPI_READ_ENHANCED_VOL_CFG_REG_CMD        0x65  //READ  ENHANCED VOLATILE CONFIGURATION REGISTER
#define QSPI_WRITE_ENHANCED_VOL_CFG_REG_CMD       0x61  //WRITE ENHANCED VOLATILE CONFIGURATION REGISTER

#define QSPI_READ_EXT_ADDR_REG_CMD                0xC8  //READ  EXTENDED ADDRESS REGISTER
#define QSPI_WRITE_EXT_ADDR_REG_CMD               0xC5  //WRITE EXTENDED ADDRESS REGISTER

/* Program Operations */
#define QSPI_PAGE_PROG_CMD                        0x02
#define QSPI_PAGE_PROG_4_BYTE_ADDR_CMD            0x12   /*0x38 使能4字节模式*/

#define QSPI_DUAL_IN_FAST_PROG_CMD                0xA2
#define QSPI_EXT_DUAL_IN_FAST_PROG_CMD            0xD2

#define QSPI_QUAD_IN_FAST_PROG_CMD                0x32
#define QSPI_EXT_QUAD_IN_FAST_PROG_CMD            QSPI_PAGE_PROG_4_BYTE_ADDR_CMD
#define QSPI_QUAD_IN_FAST_PROG_4_BYTE_ADDR_CMD    0x34

/* Erase Operations */
#define QSPI_SUBSECTOR_4K_ERASE_CMD               0x20    // 4096Byte Erase
#define QSPI_SUBSECTOR_32K_ERASE_CMD              0x52    // 32768Byte Erase
#define QSPI_BLOCK_ERASE_CMD                      0xD8    // 64KByte Erasr    1 sector
#define QSPI_BULK_ERASE_CMD                       0xC4    // 芯片擦除

#define QSPI_SUBSECTOR_4K_ERASE_4_BYTE_ADDR_CMD   0x21    // Only available for part numbers N25Q512A83GSF40x and N25Q512A83G1240x.
#define QSPI_SECTOR_ERASE_4_BYTE_ADDR_CMD      	  0xDC  
#define QSPI_SUBSECTOR_32K_ERASE_4_BYTE_ADDR_CMD  0x5C  

//#define QSPI_SECTOR_ERASE_4_BYTE_ADDR_CMD         0xDC

#define QSPI_PROG_ERASE_RESUME_CMD                0x7A
#define QSPI_PROG_ERASE_SUSPEND_CMD               0x75

/* One-Time Programmable Operations */
#define QSPI_READ_OTP_ARRAY_CMD                   0x4B
#define QSPI_PROG_OTP_ARRAY_CMD                   0x42

/* 4-byte Address Mode Operations */
#define QSPI_ENTER_4_BYTE_ADDR_MODE_CMD           0xB7   // 不支持 N25Q512A83GSF40x and N25Q512A83G1240x
#define QSPI_EXIT_4_BYTE_ADDR_MODE_CMD            0xE9


/* Power Programmable Operations */
#define QSPI_POWER_DOWN_CMD                       0xB9
#define QSPI_POWER_DOWN_RELEASE_CMD               0xAB

/* 
The ENTER or EXIT QUAD (QPI) command is available only on the N25Q512A83G1241E
and N25Q512A83G1241F devices. 系列使用指令 
*/
#define QSPI_ENTER_QUAD_CMD                       0x35
#define QSPI_EXIT_QUAD_CMD                        0xF5

//to here guangqiang add 

//N25Q Series Registers  
//Status Register
#define QSPI_SR_WIP				  	((uint8_t)0x01) //Write in progress
#define QSPI_SR_WREN			    ((uint8_t)0x02) //Write enable latch
#define QSPI_SR_BLOCKPR	    	((uint8_t)0x5C) //Block protected against program and erase operations
#define QSPI_SR_PRBOTTOM	    ((uint8_t)0x20) //Protected memory area defined by BLOCKPR starts from top or bottom
#define QSPI_SR_SRWREN		    ((uint8_t)0x80) //Status register write enable/disable

//Nonvolatile Configuration Register 0xB5  0xB1
#define QSPI_NVCR_LOCK				((uint16_t)0x0001) //Lock nonvolatile configuration register
#define QSPI_NVCR_DUAL				((uint16_t)0x0004) //Dual I/O protocol
#define QSPI_NVCR_QUAD				((uint16_t)0x0008) //Quad I/O protocol
#define QSPI_NVCR_RH					((uint16_t)0x0010) //Reset/hold
#define QSPI_NVCR_ODS					((uint16_t)0x01C0) //Output driver strength
#define QSPI_NVCR_XIP					((uint16_t)0x0E00) //XIP mode at power-on reset
#define QSPI_NVCR_NB_DUMMY	  ((uint16_t)0xF000) //Number of dummy clock cycles

//Volatile Configuration Register  0x85   0x81
#define QSPI_VCR_WRAP			  	((uint8_t)0x03) //Wrap
#define QSPI_VCR_XIP					((uint8_t)0x08) //XIP
#define QSPI_VCR_NB_DUMMY	  	((uint8_t)0xF0) //Number of dummy clock cycles

//Enhanced Volatile Configuration Register   0x65   0x61
#define QSPI_EVCR_ODS	      	((uint8_t)0x07) //Output driver strength
#define QSPI_EVCR_VPPA	      ((uint8_t)0x08) //Vpp accelerator
#define QSPI_EVCR_RH		      ((uint8_t)0x10) //Reset/hold
#define QSPI_EVCR_DUAL	      ((uint8_t)0x40) //Dual I/O protocol
#define QSPI_EVCR_QUAD	      ((uint8_t)0x80) //Quad I/O protocol

//Flag Status Register
#define QSPI_FSR_PRERR		    ((uint8_t)0x02) //Protection error
#define QSPI_FSR_PGSUS		    ((uint8_t)0x04) //Program operation suspended
#define QSPI_FSR_VPPERR		  	((uint8_t)0x08) //Invalid voltage during program or erase
#define QSPI_FSR_PGERR		    ((uint8_t)0x10) //Program error
#define QSPI_FSR_ERERR		    ((uint8_t)0x20) //Erase error
#define QSPI_FSR_ERSUS		    ((uint8_t)0x40) //Erase operation suspended
#define QSPI_FSR_READY		    ((uint8_t)0x80) //Ready or command in progress


QSPI_StaticTypeDef QSPI_UserInit(void);
QSPI_StaticTypeDef QSPI_ReadBuff(uint8_t* data, uint32_t address, uint32_t size);
QSPI_StaticTypeDef QSPI_Read_Reg(uint8_t ReadReg, uint8_t * RegValue);
QSPI_StaticTypeDef QSPI_Write_Reg(uint8_t WriteReg, uint8_t  RegValue);
QSPI_StaticTypeDef QSPI_Read_ID(uint8_t ReadID, uint8_t * _pIdBuf, uint8_t ReadIdNum);
QSPI_StaticTypeDef QSPI_Quad_Enter(void);
QSPI_StaticTypeDef QSPI_WritePageByte(uint8_t* _pBuf, uint32_t _uiWriteAddr, uint32_t _size);
QSPI_StaticTypeDef QSPI_WriteBuff(uint8_t* _pBuf, uint32_t _uiWriteAddr, uint32_t _size);
QSPI_StaticTypeDef QSPI_WriteBuffAutoEraseSector(uint8_t* _pBuf, uint32_t _uiWriteAddr, uint32_t _NumByteToWrite);
QSPI_StaticTypeDef QSPI_EraseSector_4K(uint32_t Sector_address);
QSPI_StaticTypeDef QSPI_EraseSector_32K(uint32_t Sector_address);
QSPI_StaticTypeDef QSPI_EraseChip(void);
QSPI_StaticTypeDef QSPI_GetStatus(void);
QSPI_StaticTypeDef QSPI_GetInformation(QSPI_Information* info);
QSPI_StaticTypeDef QSPI_TurnOnMemoryMappedMode(void);
QSPI_StaticTypeDef QSPI_ResetMemory(QSPI_HandleTypeDef *handle);
QSPI_StaticTypeDef QSPI_DummyCyclesCfg(QSPI_HandleTypeDef *hqspi);


#endif


