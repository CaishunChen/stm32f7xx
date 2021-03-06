
/*
********************************************************************************************************
MT25Q QSPI Flash Configuration File


STM32F7的quad-spi接口主要特点：
（1）三种工作模式
（2）Dual-Flash模式，可以同时接两片Flash，共用CLK和CS片选线。这样可以最多同时传输8位数据（4+4）
（3）支持SDR和DDR
（4）间接模式的DMA通道
（5）内嵌接收和发送FIFO
（6）支持FIFO threshold, timeout, operation complete, access error四种中断



---------------------------------------------------------------------------------------
     表2 控制器自动识别含义的命令
=======================================================================================
命令                   含义                               描述

READ             读数据，CMD=0x03                  命令通过D0发出；数据通过D0接收

FAST_READ        快速读数据，CMD=0xB               命令通过D0发出；数据通过D0接收

DOR              双IO读数据，CMD=0x3B              命令通过D0发出；数据通过D[1:0]接收

QOR              四IO线读数据，CMD=0x6B            命令通过D0发出；数据通过D[3:0]接收

DIOR             双IO命令，四IO数据，CMD=BB        命令通过D[1:0]发出，数据通过D[3:0]接收

QIOR             四IO命令，四IO数据，CMD=EB        命令通过D[3:0]发出，数据通过D[3:0]接收

PP               页编程命令，CMD=02                命令通过D0发出，数据通过D0发出

QPP              四IO页编程命令，CMD=32或38        命令通过D0发出，数据通过D0[3:0]发出




********************************************************************************************************
*/

#ifdef DEBUG
#define DBG_LOG(x) printf x
#else
#define DBG_LOG(x) 
#endif

#include "bsp_qspi_n25q.h"
#include "quadspi.h"

QSPI_Information  _QspiFlashInf;

static uint8_t QSPI_WorkMode = N25Q_SPI_MODE;		//QSPI模式标志:0,SPI模式;1,QPI模式.

static QSPI_StaticTypeDef QSPI_WriteEnable(QSPI_HandleTypeDef *handle);
//static QSPI_StaticTypeDef QSPI_WriteDisable(QSPI_HandleTypeDef *handle);
static QSPI_StaticTypeDef QSPI_AutoPollingMemReady(QSPI_HandleTypeDef *handle);

static QSPI_StaticTypeDef QSPI_EnterFourBytesAddress(QSPI_HandleTypeDef *hqspi);
static QSPI_StaticTypeDef QSPI_Receive(uint8_t * _pBuf, uint32_t _NumByteToRead);
static QSPI_StaticTypeDef QSPI_Transmit(uint8_t * _pBuf, uint32_t _NumByteToRead);
static QSPI_StaticTypeDef QSPI_SendCmdData( uint8_t  __Instruction,       //  发送指令
                                             uint32_t __InstructionMode,   //  指令模式
                                             uint32_t __AddressMode,       //  地址模式
                                             uint32_t __AddressSize,       //  地址长度  
                                             uint32_t __DataMode,          //  数据模式
                                             uint32_t __NbData,            //  数据读写字节数
                                             uint32_t __DummyCycles,       //  设置空指令周期数
                                             uint32_t __Address,           //  发送到的目的地址
                                             uint8_t  *_pBuf,             //  待发送的数据
                                             __SEND_CMD_DATA_T  _SendCmdDat
                                           );

QSPI_StaticTypeDef QSPI_Read_SR(uint8_t ReadReg, uint8_t * RegValue, uint8_t ReadRegNum);


/*
**************************************************************************************
函数名称：QSPI_UserInit
函数功能：QSPI 初始化，IO口初始化，或者QSPI 芯片的相关信息及容量等
参数：   无
返回值： QSPI_OK 初始化成功，其他值失败
**************************************************************************************
*/

QSPI_StaticTypeDef QSPI_UserInit(void)
{
  uint32_t i;
  uint8_t  _RegVal;
  uint8_t  QspiID[3];
  __IO QSPI_StaticTypeDef  __QspiStatus = QSPI_OUT_TIME;
  
	__QspiStatus=QSPI_WriteEnable(&hqspi);
	if ( __QspiStatus != QSPI_OK)   
	{
    return QSPI_ERROR;
	}
	
  #if 0
  	// QSPI memory reset  在调试时，如果输出 格式不支冲重新开关机操作即可 
  __QspiStatus = QSPI_ResetMemory(&hqspi);
	if ( __QspiStatus != QSPI_OK)   // 这个函数不能够使用，经常会卡死�
	{
    return QSPI_ERROR;
	}
	
	#endif
  
  // 资料介绍的 等待 tSHSL3 时间
  i = 0x20000;
  while(i --)
  {
		;
  }
  
	
  __QspiStatus = QSPI_Quad_Enter();
  if(__QspiStatus != QSPI_OK)
  {
    return __QspiStatus;  
  }
  DBG_LOG(("quad enter...\r\n"));
  
  if(QSPI_WorkMode == N25Q_QUAD_MODE)
  {
    if(QSPI_Read_ID(QSPI_MULTIPLE_IO_READ_ID_CMD, (uint8_t *) & QspiID[0], 3) != QSPI_OK)
      return QSPI_ERROR;
  }
  else
  {
    if(QSPI_Read_ID(QSPI_READ_ID_CMD, (uint8_t *) & QspiID[0], 3) != QSPI_OK)
      return QSPI_ERROR;  
  }
    
  
  _QspiFlashInf.Id = (QspiID[0] << 16) | (QspiID[1] << 8) | QspiID[2];

  
  switch(_QspiFlashInf.Id)
  {
    case QSPI_N25Q512_JEDEC_ID:   // N25Q512A QSPI Chip  0x20BA20
    case QSPI_N25Q256_JEDEC_ID:   // N25Q256A QSPI Chip  0x20BA19
    case QSPI_MT25Q1GB_JEDEC_ID:  //MT25Q1GB  QSPI Chip  0x20BA21
    {
      if(QSPI_Read_SR(QSPI_READ_FLAG_STATUS_REG_CMD, &_RegVal , 1) == QSPI_OK)   // 读取 0x70寄存器，查看是否为4字节地址
      {
        if((_RegVal & 0x01) == 0x00)       //如果不是4字节地址模式,则进入4字节地址模式
        {
          __QspiStatus = QSPI_EnterFourBytesAddress(&hqspi);   // 设置4字节地址
          if(__QspiStatus != QSPI_OK)
          {
            return QSPI_ERROR;  
          }
        }
        
        QSPI_GetInformation( &_QspiFlashInf );    
      }
      break ;
    }

    default : 
      {
        return QSPI_ERROR;  
      }
  }
  
  
  if(_QspiFlashInf.Id == QSPI_MT25Q1GB_JEDEC_ID)
  {
    _QspiFlashInf.FlashTotalSize /= 1024;
    DBG_LOG(("QSPI MT25Q1GB Capacity %d MByte ... \r\n", _QspiFlashInf.FlashTotalSize / 1024));
    DBG_LOG(("QSPI MT25Q1GB ID  0x%X ...\r\n", _QspiFlashInf.Id));
		DBG_LOG(("QSPI MT25Q1GB FlashTotalSize 0x%X ...\r\n", _QspiFlashInf.FlashTotalSize));
		DBG_LOG(("QSPI MT25Q1GB SectorSize 0x%X ...\r\n", _QspiFlashInf.SectorSize));
		DBG_LOG(("QSPI MT25Q1GB EraseSectorSize 0x%X ...\r\n", _QspiFlashInf.EraseSectorSize));
		DBG_LOG(("QSPI MT25Q1GB EraseSectorsNumber 0x%X ...\r\n", _QspiFlashInf.EraseSectorsNumber));
		DBG_LOG(("QSPI MT25Q1GB ProgPageSize 0x%X ...\r\n", _QspiFlashInf.ProgPageSize));
		DBG_LOG(("QSPI MT25Q1GB ProgPagesNumber 0x%X ... \r\n", _QspiFlashInf.ProgPagesNumber));
  } 
    DBG_LOG(("QSPI_UserInit OK ...\r\n\r\n"));
  return QSPI_OK;
}




/*
int QSPI_Read(uint8_t* data, uint32_t address, uint32_t size)	--Read data from QSPI flash
A function to read data from QSPI flash
Return an integer value (default QSPI_OK), a parameter for data buffer, 
a parameter for QSPI memory address, a parameter for data size.
*/

QSPI_StaticTypeDef QSPI_ReadBuff(uint8_t* data, uint32_t address, uint32_t size)
{
  uint32_t  __InstructionMode, __AddressMode, __DataMode;
  uint8_t _RegVal = 0;
  
  if(QSPI_WorkMode)   // Work In QUAD Model
  {
    __InstructionMode = QSPI_INSTRUCTION_4_LINES;
    __AddressMode     = QSPI_ADDRESS_4_LINES;
    __DataMode        = QSPI_DATA_4_LINES;
  }
  else
  {
    __InstructionMode = QSPI_INSTRUCTION_1_LINE;
    __AddressMode     = QSPI_ADDRESS_1_LINE;
    __DataMode        = QSPI_DATA_1_LINE;  
  } 
  
        
  //QUAD,快速读数据,地址为address,4线传输数据_32位地址_4线传输地址_4线传输指令,8空周期,NumByteToRead个数据
  if(QSPI_SendCmdData(  QSPI_QUAD_INOUT_FAST_READ_4_BYTE_ADDR_CMD /*QSPI_FAST_READ_4_BYTE_ADDR_CMD*/ /*QSPI_FAST_READ_4_BYTE_ADDR_CMD*/ /*QSPI_QUAD_INOUT_FAST_READ_CMD*/  /*QSPI_QUAD_INOUT_FAST_READ_CMD*/ ,        // _Instruction,      发送指令
                        __InstructionMode,               // _InstructionMode,  指令模式
                        __AddressMode,                   // _AddressMode,      地址模式
                        QSPI_ADDRESS_32_BITS,            // _AddressSize,      地址长度  
                        __DataMode,                      // _DataMode,         数据模式
                        size,                            // _NbData,           数据读写字节数
                        10 ,                             // _DummyCycles,      设置空指令周期数 与 QSPI_SET_READ_PARAM 这个指令设置的值一致
                        address,                         // _Address,          发送到的目的地址
                        &_RegVal,                        //  *_pBuf,           读取的数据，此处没有使用
                        QSPI_SEND_CMD                   // __SEND_CMD_DATA_T  _SendCmdDat
                     ) != QSPI_OK )
  {
    return QSPI_ERROR;
  }    
  
  if(QSPI_Receive(  data,  size) != QSPI_OK)
    return  QSPI_ERROR;  
  return  QSPI_OK;

}



/*
**************************************************************************************
函数名称：QSPI_Read_SR
函数功能：读取寄存器值
参数：    ReadReg 需要读取的寄存器
          @ QSPI_READ_ENHANCED_VOL_CFG_REG_CMD, QSPI_READ_EXT_ADDR_REG_CMD, QSPI_READ_NONVOL_CFG_REG_CMD
            QSPI_READ_FLAG_STATUS_REG_CMD, QSPI_READ_STATUS_REG_CMD  ... ...
          * RegValue  寄存器值
          ReadRegNum  读取寄存器字节数
返回值：QSPI_OK读取成功，否则失败
**************************************************************************************
*/

QSPI_StaticTypeDef QSPI_Read_SR(uint8_t ReadReg, uint8_t * RegValue, uint8_t ReadRegNum)
{
  uint32_t  __InstructionMode, __DataMode;
  
  if(QSPI_WorkMode)   // Work In QUAD Model
  {
    __InstructionMode = QSPI_INSTRUCTION_4_LINES;
    __DataMode        = QSPI_DATA_4_LINES;
  }
  else
  {
    __InstructionMode = QSPI_INSTRUCTION_1_LINE;
    __DataMode        = QSPI_DATA_1_LINE;  
  }   
  
  
  if(QSPI_SendCmdData(  ReadReg,                // _Instruction,      发送指令
                        __InstructionMode,      // _InstructionMode,  指令模式
                        QSPI_ADDRESS_NONE,      // _AddressMode,      地址模式
                        QSPI_ADDRESS_8_BITS,    // _AddressSize,      地址长度  
                        __DataMode,             // _DataMode,         数据模式
                        ReadRegNum,             // _NbData,           数据读写字节数
                        0,                      // _DummyCycles,      设置空指令周期数
                        0,                      // _Address,          发送到的目的地址
                        RegValue,               //  *_pBuf,           待发送的数据
                        QSPI_SEND_CMD           // __SEND_CMD_DATA_T  _SendCmdDat
                     ) != QSPI_OK )
  {
    return QSPI_ERROR;
  }
  
  if(QSPI_Receive( RegValue , ReadRegNum) != QSPI_OK)
    return QSPI_ERROR;

	return QSPI_OK;
}


/*
**************************************************************************************
函数名称：QSPI_Write_SR
函数功能：写寄存器值
参数：    WriteReg 需要写入的寄存器
          @ QSPI_WRITE_ENHANCED_VOL_CFG_REG_CMD, QSPI_WRITE_EXT_ADDR_REG_CMD, QSPI_WRITE_NONVOL_CFG_REG_CMD
            QSPI_WRITE_FLAG_STATUS_REG_CMD, QSPI_WRITE_STATUS_REG_CMD ... ...
          RegValue    待写入的寄存器值
          WriteRegNum  写入的寄存器的字节数
返回值：QSPI_OK读取成功，否则失败
**************************************************************************************
*/
QSPI_StaticTypeDef QSPI_Write_SR(uint8_t WriteReg, uint8_t  RegValue, uint8_t WriteRegNum)
{
  
  uint32_t  __InstructionMode, __DataMode;
  
  if(QSPI_WriteEnable(&hqspi) != QSPI_OK)
    return QSPI_ERROR;  
  
  if(QSPI_WorkMode)
  {
    __InstructionMode = QSPI_INSTRUCTION_4_LINES;
    __DataMode        = QSPI_DATA_4_LINES;
  }
  else
  {
    __InstructionMode = QSPI_INSTRUCTION_1_LINE;
    __DataMode        = QSPI_DATA_1_LINE;  
  } 

  if(QSPI_SendCmdData(  WriteReg,               // _Instruction,      发送指令
                        __InstructionMode,      // _InstructionMode,  指令模式
                        QSPI_ADDRESS_NONE,      // _AddressMode,      地址模式
                        QSPI_ADDRESS_8_BITS,    // _AddressSize,      地址长度  
                        __DataMode,             // _DataMode,         数据模式
                        WriteRegNum,            // _NbData,           数据读写字节数
                        0,                      // _DummyCycles,      设置空指令周期数
                        0,                      // _Address,          发送到的目的地址
                        &RegValue,              //  *_pBuf,           待发送的数据
                        QSPI_SEND_DAT           // __SEND_CMD_DATA_T  _SendCmdDat
                     ) != QSPI_OK )
  {
    return QSPI_ERROR;
  } 

  if(QSPI_AutoPollingMemReady(&hqspi) != QSPI_OK)
    return QSPI_ERROR;
  
  return QSPI_OK;
}



/*
**************************************************************************************
函数名称：QSPI_Read_ID
函数功能：读取 ID 值
参数：    ReadID 需要读取的ID寄存器指令
          * _pIdBuf  读取ID信息的数据缓冲区
         ReadIdNum  需要读取id 的数据字节数量
返回值：QSPI_OK读取成功，否则失败
**************************************************************************************
*/

QSPI_StaticTypeDef QSPI_Read_ID(uint8_t ReadID, uint8_t * _pIdBuf, uint8_t ReadIdNum)
{
  
  uint32_t  __InstructionMode, __DataMode;
  
  if(QSPI_WorkMode)   // Work In QUAD Model
  {
    __InstructionMode = QSPI_INSTRUCTION_4_LINES;
    __DataMode        = QSPI_DATA_4_LINES;
		DBG_LOG(("worked in 4 lines mode...\r\n"));
  }
  else
  {
    __InstructionMode = QSPI_INSTRUCTION_1_LINE;
    __DataMode        = QSPI_DATA_1_LINE;  
		DBG_LOG(("worked in 1 lines mode...\r\n"));
  }   
  

  if(QSPI_SendCmdData(  ReadID,                   // _Instruction,      发送指令
                        __InstructionMode,        // _InstructionMode,  指令模式
                        QSPI_ADDRESS_NONE,        // _AddressMode,      地址模式
                        QSPI_ADDRESS_8_BITS,      // _AddressSize,      地址长度  
                        __DataMode,               // _DataMode,         数据模式
                        ReadIdNum,                // _NbData,           数据读写字节数
                        0,                        // _DummyCycles,      设置空指令周期数
                        0,                        // _Address,          发送到的目的地址
                        &_pIdBuf[0],              //  *_pBuf,           待发送的数据
                        QSPI_SEND_CMD             // __SEND_CMD_DATA_T  _SendCmdDat
                     ) != QSPI_OK )
  {
    return QSPI_ERROR;
  }

  
  if(QSPI_Receive( (uint8_t * ) _pIdBuf , ReadIdNum) != QSPI_OK)
    return QSPI_ERROR;  
  
	return QSPI_OK;
}

/*
**************************************************************************************
函数名称：QSPI_Quad_Enter
函数功能：进入 QUAD 模式
返回值：QSPI_OK读取成功，其他值失败
**************************************************************************************
*/
QSPI_StaticTypeDef QSPI_Quad_Enter(void)
{
  uint8_t _RegVal[2] = {0xaa, 0xaa};


  QSPI_WorkMode = N25Q_SPI_MODE;
  
  if(QSPI_Read_SR(QSPI_READ_ENHANCED_VOL_CFG_REG_CMD, &_RegVal[0], 1) != QSPI_OK )   // read reg 0x65
  {
    return QSPI_ERROR;
  }  
  
  if(_RegVal[0] == 0xff)   // 芯片初始值 = 0xDF
  {
    _RegVal[0] = 0x2F;   // bit7=0,bit6=1 QUAD 模式, bit5 系统保留且必须设置为0///guangqiang 此处需调试
    if(QSPI_Write_SR(QSPI_WRITE_ENHANCED_VOL_CFG_REG_CMD, _RegVal[0], 1) == QSPI_OK)  // 设置为 QUAD 模式 reg 0x61
    {
      QSPI_WorkMode = N25Q_QUAD_MODE;
      
      
      _RegVal [0]= 0xaa;
      QSPI_Read_SR(QSPI_READ_ENHANCED_VOL_CFG_REG_CMD, &_RegVal[0], 1);  // 测试读出来的值与写入的值是否一致，如果是 0x1F  说明ok
      DBG_LOG(("QSPI_READ_VOL_CFG_REG_CMD 2 =  0x%X\r\n", _RegVal[0]));    
      
      QSPI_Read_SR(QSPI_READ_VOL_CFG_REG_CMD, &_RegVal[0],1);  // 0xFB 说明ok
      
    }
  }
  else
  {
    DBG_LOG(("QSPI N25Qxx Chip Error  ... ... \r\n"));
    return QSPI_ERROR;
  }

  return QSPI_OK;  
}


/*
**************************************************************************************
函数名称：QSPI_WritePageByte
函数描述： page 写，在写前必须先擦除相应的扇区，不支持跨页写
参数：_pBuf          数据缓冲区
      _uiWriteAddr   写入的地址
      _size          写入数据大小,每次最大能够写入256字节
返回值：QSPI_OK 表示成功，其他失败
**************************************************************************************
*/
QSPI_StaticTypeDef QSPI_WritePageByte(uint8_t* _pBuf, uint32_t _uiWriteAddr, uint32_t _size)
{
  uint32_t  __InstructionMode, __AddressMode, __DataMode;
  
	if (QSPI_WriteEnable(&hqspi) != QSPI_OK)
	{
		return QSPI_ERROR;
	}  
  
  if(QSPI_WorkMode)   // Work In QUAD Model
  {
    /*
    
       QSPI_EXT_QUAD_IN_FAST_PROG_CMD  写入 8192*4 需要 2018ms
       QSPI_PAGE_PROG_CMD              写入 8192*4 需要 2035ms
       QSPI_PAGE_PROG_4_BYTE_ADDR_CMD  写入 8192*4 需要 2029ms
       QSPI_QUAD_IN_FAST_PROG_CMD      写入 8192*4 需要 2014ms - 2023ms
       QSPI_EXT_QUAD_IN_FAST_PROG_CMD  写入 8192*4 需要 2025ms

    */
    __InstructionMode = QSPI_INSTRUCTION_4_LINES;
    __AddressMode     = QSPI_ADDRESS_4_LINES;
    __DataMode        = QSPI_DATA_4_LINES;
  }
  else
  {
    __InstructionMode = QSPI_INSTRUCTION_1_LINE;
    __AddressMode     = QSPI_ADDRESS_1_LINE;
    __DataMode        = QSPI_DATA_1_LINE;  
  }   
  
  if(QSPI_SendCmdData(  QSPI_PAGE_PROG_4_BYTE_ADDR_CMD,   //QSPI_QUAD_IN_FAST_PROG_CMD, //QSPI_PAGE_PROG_4_BYTE_ADDR_CMD, QSPI_PAGE_PROG_CMD,         // _Instruction,      发送指令
                        __InstructionMode,                // _InstructionMode,  指令模式
                        __AddressMode,                    // _AddressMode,      地址模式
                        QSPI_ADDRESS_32_BITS,             // _AddressSize,      地址长度  
                        __DataMode,                       // _DataMode,         数据模式
                        _size,                            // _NbData,           数据读写字节数
                        0,                                // _DummyCycles,      设置空指令周期数
                        _uiWriteAddr,                     // _Address,          发送到的目的地址
                        (uint8_t *) _pBuf,               //  *_pBuf,           待发送的数据
                        QSPI_SEND_DAT                    // __SEND_CMD_DATA_T  _SendCmdDat
                     ) != QSPI_OK )
  {
    return QSPI_ERROR;
  }   
    
  // Configure automatic polling mode to wait for end of program ----- 
  if(QSPI_AutoPollingMemReady(&hqspi) != QSPI_OK)
    return QSPI_ERROR;
  
  return QSPI_OK;
}



/*
**************************************************************************************
函数名称：QSPI_WriteBuff
函数描述：支持夸 page 写，在写前必须先擦除相应的扇区
参数：_pBuf          数据缓冲区
      _uiWriteAddr   写入的地址
      _size          写入数据大小,
返回值：QSPI_OK 表示成功，其他失败
**************************************************************************************
*/
QSPI_StaticTypeDef QSPI_WriteBuff(uint8_t* _pBuf, uint32_t _uiWriteAddr, uint32_t _size)
{
  __IO uint32_t end_addr;
  __IO uint32_t current_size, current_addr=0;

	//Calculation of the size between the write address and the end of the page
	current_addr = 0;

	while (current_addr <= _uiWriteAddr)
	{
		current_addr += QSPI_PAGE_SIZE;
	}
	
	current_size = current_addr - _uiWriteAddr;

	//Check if the size of the data is less than the remaining place in the page
	if (current_size > _size)
	{
		current_size = _size;
	}
	
	//Initialize the adress variables
	current_addr = _uiWriteAddr;
	end_addr = _uiWriteAddr + _size;

	//Perform the write page by page
	do
	{

    if(QSPI_WritePageByte( (uint8_t *) _pBuf, current_addr, current_size) != QSPI_OK)
      return QSPI_ERROR;  
        
		//Update the address and size variables for next page programming
		current_addr += current_size;
		_pBuf += current_size;
		current_size = ((current_addr + QSPI_PAGE_SIZE) > end_addr) ? (end_addr - current_addr) : QSPI_PAGE_SIZE;
	} while (current_addr < end_addr);

	return QSPI_OK;
}


/*
**************************************************************************************
函数名称：QSPI_Write_NoCheck
函数描述： 无检验写 QSPI FLASH, 必须确保所写的地址范围内的数据全部为0XFF,否则在非0XFF处
           写入的数据将失败! 具有自动换页功能, 在指定地址开始写入指定长度的数据,但是要
           确保地址不越界!
参数：_pBuf          数据缓冲区
      _uiWriteAddr   写入的地址
      _NumByteToWrite 要写入的字节数(最大65535)
返回值：QSPI_OK 表示成功，其他失败
**************************************************************************************
*/
QSPI_StaticTypeDef QSPI_Write_NoCheck(uint8_t* _pBuf, uint32_t _uiWriteAddr, uint32_t  _NumByteToWrite)   
{ 			 		 
	uint32_t pageremain;	   
	pageremain = 256 - (_uiWriteAddr % QSPI_PAGE_SIZE); //单页剩余的字节数		 	    
	if(_NumByteToWrite <= pageremain)
    pageremain = _NumByteToWrite;           //不大于256个字节
	while(1)
	{   
		if(QSPI_WritePageByte(_pBuf, _uiWriteAddr, pageremain) != QSPI_OK)
      return QSPI_ERROR;
    
		if(_NumByteToWrite == pageremain)         //写入结束了
      break;
    
	 	else                                      //NumByteToWrite>pageremain
		{
			_pBuf        += pageremain;
			_uiWriteAddr += pageremain;	

			_NumByteToWrite -= pageremain;			    //减去已经写入了的字节数
			if(_NumByteToWrite > QSPI_PAGE_SIZE)
        pageremain = QSPI_PAGE_SIZE;          //一次可以写入256个字节
			else 
        pageremain = _NumByteToWrite; 	      //不够256个字节了
		}
	} 
  return QSPI_OK;
}


/*
**************************************************************************************
函数名称：QSPI_WriteBuffAutoEraseSector
函数描述：在指定地址开始写入指定长度的数据, 该函数带擦除扇区操作功能 !
参数：_pBuf           数据缓冲区
      _uiWriteAddr    写入的地址
      _NumByteToWrite 写入数据大小,
返回值：QSPI_OK 表示成功，其他失败
**************************************************************************************
*/
#if 1
#pragma pack(4)
static uint8_t __packed g_tQSpiBuf[4*1024];
#pragma pack()
#endif

QSPI_StaticTypeDef QSPI_WriteBuffAutoEraseSector(uint8_t* _pBuf, uint32_t _uiWriteAddr, uint32_t _NumByteToWrite)
{
	uint32_t secpos;
	uint16_t secoff;
	uint16_t secremain;	   
 	uint16_t i;    
	uint8_t * _pStr = g_tQSpiBuf; 
     
 	secpos = _uiWriteAddr / 4096;           //扇区地址  
	secoff = _uiWriteAddr % 4096;           //在扇区内的偏移
	secremain = 4096 - secoff;              //扇区剩余空间大小   

 	if(_NumByteToWrite <= secremain)
    secremain = _NumByteToWrite;          //不大于4096个字节
	while(1) 
	{	
		if(QSPI_ReadBuff(_pStr, (secpos * 4096), 4096) !=  QSPI_OK)  //读出整个扇区的内容
      return QSPI_ERROR;
    
		for(i=0;i<secremain;i++)//校验数据
		{
			if(_pStr[secoff+i] != 0XFF)
        break;//需要擦除  	  
		}
		if(i<secremain)//需要擦除
		{
			if(QSPI_EraseSector_4K(secpos) !=  QSPI_OK)   //擦除这个扇区
        return QSPI_ERROR;
      
			for(i=0;i<secremain;i++)	   //复制
			{
				_pStr[i+secoff] = _pBuf[i];	  
			}
			if(QSPI_Write_NoCheck(_pStr, (secpos * 4096), 4096) !=  QSPI_OK)    //写入整个扇区 
        return QSPI_ERROR;
		}
    else 
    {
      if(QSPI_Write_NoCheck(_pBuf, _uiWriteAddr, secremain) !=  QSPI_OK)     //写已经擦除了的,直接写入扇区剩余区间. 	
        return QSPI_ERROR;
    }
    
		if(_NumByteToWrite == secremain)  break;  //写入结束了
    
		else                                    //写入未结束
		{
			secpos ++;                            //扇区地址增1
			secoff = 0;                           //偏移位置为0 	 

		  _pBuf           += secremain;         //指针偏移
			_uiWriteAddr    += secremain;         //写地址偏移	   
		  _NumByteToWrite -= secremain;				  //字节数递减
      
			if(_NumByteToWrite > 4096)          // 4096
        secremain = 4096;	                  //下一个扇区还是写不完
			else 
        secremain = _NumByteToWrite;			    //下一个扇区可以写完了
		}	 
	} ;
	return QSPI_OK;
}




/*
int QSPI_Erase_Block(uint32_t Sector_address)	--Earse a QSPI flash Sector
A function to earse a QSPI flash Sector
Return an integer value (default QSPI_OK), a parameter for block address.
*/
QSPI_StaticTypeDef QSPI_EraseSector_4K(uint32_t Sector_address)
{
  uint8_t _RegVal = 0;
  uint32_t  __InstructionMode, __AddressMode;
  
	if (QSPI_WriteEnable(&hqspi) != QSPI_OK)
	{
		return QSPI_ERROR;
	}    
  
  if(QSPI_WorkMode)   // Work In QUAD Model
  {
    __InstructionMode = QSPI_INSTRUCTION_4_LINES;
    __AddressMode     = QSPI_ADDRESS_4_LINES;
  }
  else
  {
    __InstructionMode = QSPI_INSTRUCTION_1_LINE;
    __AddressMode     = QSPI_ADDRESS_1_LINE;  
  }   

  Sector_address <<= 12;  

  if(QSPI_SendCmdData(  QSPI_SUBSECTOR_4K_ERASE_CMD,   // _Instruction,      发送指令
                        __InstructionMode,          // _InstructionMode,  指令模式
                        __AddressMode,              // _AddressMode,      地址模式
                        QSPI_ADDRESS_32_BITS,       // _AddressSize,      地址长度  
                        QSPI_DATA_NONE,             // _DataMode,         数据模式
                        0,                          // _NbData,           数据读写字节数
                        0,                          // _DummyCycles,      设置空指令周期数
                        Sector_address,             // _Address,          发送到的目的地址
                        &_RegVal,                   //  *_pBuf,           待发送的数据
                        QSPI_SEND_CMD               // __SEND_CMD_DATA_T  _SendCmdDat
                     ) != QSPI_OK )
  {
    return QSPI_ERROR;
  }   
 
  if(QSPI_AutoPollingMemReady(&hqspi) != QSPI_OK)
    return QSPI_ERROR;  
  
  return QSPI_OK;  
}




/*
int QSPI_Erase_Block(uint32_t Sector_address)	--Earse a QSPI flash Sector
A function to earse a QSPI flash Sector
Return an integer value (default QSPI_OK), a parameter for block address.

*/
QSPI_StaticTypeDef QSPI_EraseSector_32K(uint32_t Sector_address)
{
  uint8_t _RegVal = 0;
  uint32_t  __InstructionMode, __AddressMode;
  
	if (QSPI_WriteEnable(&hqspi) != QSPI_OK)
	{
		return QSPI_ERROR;
	}    
  
  if(QSPI_WorkMode)   // Work In QUAD Model
  {
    __InstructionMode = QSPI_INSTRUCTION_4_LINES;
    __AddressMode     = QSPI_ADDRESS_4_LINES;
  }
  else
  {
    __InstructionMode = QSPI_INSTRUCTION_1_LINE;
    __AddressMode     = QSPI_ADDRESS_1_LINE;  
  }   

  Sector_address <<= 15;  

  if(QSPI_SendCmdData( QSPI_SUBSECTOR_32K_ERASE_CMD,   // _Instruction,      发送指令
  
                        __InstructionMode,          // _InstructionMode,  指令模式
                        __AddressMode,              // _AddressMode,      地址模式
                        QSPI_ADDRESS_32_BITS,       // _AddressSize,      地址长度  
                        QSPI_DATA_NONE,             // _DataMode,         数据模式
                        0,                          // _NbData,           数据读写字节数
                        0,                          // _DummyCycles,      设置空指令周期数
                        Sector_address,             // _Address,          发送到的目的地址
                        &_RegVal,                   //  *_pBuf,           待发送的数据
                        QSPI_SEND_CMD               // __SEND_CMD_DATA_T  _SendCmdDat
                     ) != QSPI_OK )
  {
    return QSPI_ERROR;
  }   
  if(QSPI_AutoPollingMemReady(&hqspi) != QSPI_OK)
    return QSPI_ERROR;  
  
  return QSPI_OK;  
}



/*
int QSPI_Erase_Chip(void)	--Earse QSPI flash full chip
A function to earse QSPI flash full chip
Return an integer value (default QSPI_OK), no parameters.

擦除 N25Q256A13xx 整个芯片时间 233879ms 左右, 接近4分钟
*/
QSPI_StaticTypeDef QSPI_EraseChip(void)
{
  uint32_t  __InstructionMode ;
  uint8_t _RegVal = 0;
	//Enable write operations
	if (QSPI_WriteEnable(&hqspi) != QSPI_OK)
	{
		return QSPI_ERROR;
	}

  if(QSPI_WorkMode)   // Work In QUAD Model
  {
    __InstructionMode = QSPI_INSTRUCTION_4_LINES;
  }
  else
  {
    __InstructionMode = QSPI_INSTRUCTION_1_LINE; 
  }   
  
  if(QSPI_SendCmdData(  QSPI_BULK_ERASE_CMD,        // _Instruction,      发送指令
                        __InstructionMode,          // _InstructionMode,  指令模式
                        QSPI_ADDRESS_NONE,          // _AddressMode,      地址模式
                        QSPI_ADDRESS_8_BITS,        // _AddressSize,      地址长度  
                        QSPI_DATA_NONE,             // _DataMode,         数据模式
                        0,                          // _NbData,           数据读写字节数
                        0,                          // _DummyCycles,      设置空指令周期数
                        0,                          // _Address,          发送到的目的地址
                        &_RegVal,                   //  *_pBuf,           待发送的数据
                        QSPI_SEND_CMD               // __SEND_CMD_DATA_T  _SendCmdDat
                     ) != QSPI_OK )
  {
    return QSPI_ERROR;
  }
  
  
  if(QSPI_AutoPollingMemReady(&hqspi) != QSPI_OK)
    return QSPI_ERROR;  
  
  return QSPI_OK;  
}



/*
int QSPI_GetStatus(void)	--Get QSPI flash's status
A function to get QSPI flash's status
Return an integer value (no default value), no parameters.
调试 ok
*/
/*
QSPI_StaticTypeDef QSPI_GetStatus(void)
{
	QSPI_CommandTypeDef qspi_cmd;
	uint8_t reg;
  __IO uint32_t i=0;

	//Initialize the read flag status register command
	qspi_cmd.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
	qspi_cmd.Instruction       = QSPI_READ_FLAG_STATUS_REG_CMD;
	qspi_cmd.AddressMode       = QSPI_ADDRESS_NONE;
	qspi_cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	qspi_cmd.DataMode          = QSPI_DATA_1_LINE;
	qspi_cmd.DummyCycles       = 0;
	qspi_cmd.NbData            = 1;
	qspi_cmd.DdrMode           = QSPI_DDR_MODE_DISABLE;
	qspi_cmd.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
	qspi_cmd.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

	//Configure the command
	if (HAL_QSPI_Command(&hqspi, &qspi_cmd, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		return QSPI_ERROR;
	}

  do{
  
    i ++;
    
    //Reception of the data
    if (HAL_QSPI_Receive(&hqspi, &reg, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
      return QSPI_ERROR;
    }    
    
    if ((reg & QSPI_FSR_READY) != 0)  //Check the value of the register
    {
      return QSPI_OK;
    }    
  } while(i < QSPI_WAIT_MAX_TIME );
  
  
	//Check the value of the register
	if ((reg & (QSPI_FSR_PRERR | QSPI_FSR_VPPERR | QSPI_FSR_PGERR | QSPI_FSR_ERERR)) != 0)
	{
		return QSPI_ERROR;
	}
	else if ((reg & (QSPI_FSR_PGSUS | QSPI_FSR_ERSUS)) != 0)
	{
		return QSPI_SUSPENDED;
	}
	else
	{
		return QSPI_BUSY;
	}  
}
*/

/*
int QSPI_GetInformation(QSPI_Information* info)		--Get QSPI flash's informations
A function to get QSPI flash's informations
Return an integer value (no default value), a parameter for QSPI informataion.
*/
QSPI_StaticTypeDef QSPI_GetInformation(QSPI_Information* info)
{
	//Configure the structure with the memory configuration
	info->FlashTotalSize     = QSPI_MT25Q1GB_TOTAL_SIZE;      
  info->SectorSize         = QSPI_SUBSECTOR_SIZE;   // 
	info->EraseSectorSize    = QSPI_SUBSECTOR_SIZE;
	info->EraseSectorsNumber = ((QSPI_MT25Q1GB_TOTAL_SIZE) / QSPI_SUBSECTOR_SIZE);
	info->ProgPageSize       = QSPI_PAGE_SIZE;
	info->ProgPagesNumber    = ((QSPI_MT25Q1GB_TOTAL_SIZE) / QSPI_PAGE_SIZE);

	return QSPI_OK;
}



/*
QSPI_TurnOnMemoryMappedMode QSPI_TurnOnMemoryMappedMode(void)		--Turn on QSPI memory mapped mode
A function to turn on QSPI memory mapped mode
Return an integer value (no default value), No parameters.
*/
/*
QSPI_StaticTypeDef QSPI_TurnOnMemoryMappedMode(void)
{
	QSPI_CommandTypeDef   qspi_cmd;
	QSPI_MemoryMappedTypeDef s_mem_mapped_cfg;

	//Configure the command for the read instruction
	qspi_cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
	qspi_cmd.Instruction = QSPI_QUAD_INOUT_FAST_READ_CMD;
	qspi_cmd.AddressMode = QSPI_ADDRESS_4_LINES;
	qspi_cmd.AddressSize = QSPI_ADDRESS_24_BITS;
	qspi_cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	qspi_cmd.DataMode = QSPI_DATA_4_LINES;
	qspi_cmd.DummyCycles = QSPI_DUMMY_CYCLES_READ;
	qspi_cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
	qspi_cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	qspi_cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

	//Configure the memory mapped mode
	s_mem_mapped_cfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_ENABLE;
	s_mem_mapped_cfg.TimeOutPeriod = 1;

	if (HAL_QSPI_MemoryMapped(&hqspi, &qspi_cmd, &s_mem_mapped_cfg) != HAL_OK)
	{
		return QSPI_ERROR;
	}

	return QSPI_OK;
}
*/


/*
static QSPI_StaticTypeDef QSPI_ResetMemory(QSPI_HandleTypeDef *handle)	--Reset QSPI flash memory
A function to reset QSPI flash memory
Return a integer value(default QSPI_OK), a parameter for QSPI handle
*/
QSPI_StaticTypeDef QSPI_ResetMemory(QSPI_HandleTypeDef *handle)
{
	QSPI_CommandTypeDef qspi_cmd;

	//Initialize the reset enable command
	qspi_cmd.InstructionMode    = QSPI_INSTRUCTION_1_LINE;
	qspi_cmd.Instruction        = QSPI_RESET_ENABLE_CMD;
	qspi_cmd.AddressMode        = QSPI_ADDRESS_NONE;
	qspi_cmd.AlternateByteMode  = QSPI_ALTERNATE_BYTES_NONE;
	qspi_cmd.DataMode           = QSPI_DATA_NONE;
	qspi_cmd.DummyCycles        = 0;
	qspi_cmd.DdrMode            = QSPI_DDR_MODE_DISABLE;
	qspi_cmd.DdrHoldHalfCycle   = QSPI_DDR_HHC_ANALOG_DELAY;
	qspi_cmd.SIOOMode           = QSPI_SIOO_INST_EVERY_CMD;

	//Send the command
	if (HAL_QSPI_Command(handle, &qspi_cmd, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		return QSPI_ERROR;
	}
  
	//Send the reset memory command
	qspi_cmd.Instruction = QSPI_RESET_MEMORY_CMD;
	if (HAL_QSPI_Command(handle, &qspi_cmd, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		return QSPI_ERROR;
	}

	//Configure automatic polling mode to wait the memory is ready
	if (QSPI_AutoPollingMemReady(handle) != QSPI_OK)
	{
		return QSPI_ERROR;
	}

	return QSPI_OK;	
}


/**
  * @brief  This function set the QSPI memory in 4-byte address mode
  * @param  hqspi: QSPI handle
  * @retval None
  */
static QSPI_StaticTypeDef QSPI_EnterFourBytesAddress(QSPI_HandleTypeDef *hqspi)
{
  QSPI_CommandTypeDef s_command;

/* Initialize the command */
  
  if(QSPI_WorkMode)   // QUAD Model 
    s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
  else
    s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  
  s_command.Instruction       = QSPI_ENTER_4_BYTE_ADDR_MODE_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_NONE;
  s_command.DummyCycles       = 0;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Enable write operations */
  if (QSPI_WriteEnable(hqspi) != QSPI_OK)
  {
    return QSPI_WRITE_ENABLE_ERROR;
  } 

  /* Send the command */
  if (HAL_QSPI_Command(hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }  

  
  /* Configure automatic polling mode to wait the memory is ready */
  if (QSPI_AutoPollingMemReady(hqspi) != QSPI_OK)
  {
    return QSPI_ERROR;
  }
  return QSPI_OK;
}




/*
static QSPI_StaticTypeDef QSPI_WriteEnable(QSPI_HandleTypeDef *handle)	--Enable QSPI memory write mode
A function to enable QSPI memory write mode
Return a integer value(default QSPI_OK), a parameter for QSPI handle
N25Q256/N25Q512 资料 Table 9: Status Register Bit Definitions有介绍
*/
static QSPI_StaticTypeDef QSPI_WriteEnable(QSPI_HandleTypeDef *handle)
{
  uint32_t  __InstructionMode;
  uint8_t _RegVal = 0;
  
  if(QSPI_WorkMode)   // Work In QUAD Model
  {
    __InstructionMode = QSPI_INSTRUCTION_4_LINES;
  }
  else
  {
    __InstructionMode = QSPI_INSTRUCTION_1_LINE; 
  }   
  
  if(QSPI_SendCmdData(  QSPI_WRITE_ENABLE_CMD,        // _Instruction,      发送指令
                        __InstructionMode,            // _InstructionMode,  指令模式
                        QSPI_ADDRESS_NONE,            // _AddressMode,      地址模式
                        QSPI_ADDRESS_8_BITS,          // _AddressSize,      地址长度  
                        QSPI_DATA_NONE,               // _DataMode,         数据模式
                        0,                            // _NbData,           数据读写字节数
                        0,                            // _DummyCycles,      设置空指令周期数
                        0,                            // _Address,          发送到的目的地址
                        &_RegVal,                     //  *_pBuf,           待发送的数据
                        QSPI_SEND_CMD                 // __SEND_CMD_DATA_T  _SendCmdDat
                     ) != QSPI_OK )
  {
    return QSPI_ERROR;
  } 
  
	return QSPI_OK;
}



#if   0
/*
static QSPI_StaticTypeDef QSPI_WriteDisable(QSPI_HandleTypeDef *handle)	--Enable QSPI memory write mode
A function to enable QSPI memory write mode
Return a integer value(default QSPI_OK), a parameter for QSPI handle
*/
static QSPI_StaticTypeDef QSPI_WriteDisable(QSPI_HandleTypeDef *handle)
{
  QSPI_CommandTypeDef     sCommand;
  QSPI_AutoPollingTypeDef sConfig;

  /* Enable write operations ------------------------------------------ */
  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;     //指令模式
  sCommand.Instruction       = QSPI_WRITE_DISABLE_CMD;       //指令
  sCommand.AddressMode       = QSPI_ADDRESS_NONE;           //地址模式
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;   //无交替字节
  sCommand.DataMode          = QSPI_DATA_NONE;              //数据模式
  sCommand.DummyCycles       = 0;                           //设置空指令周期数
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;       //关闭DDR模式
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  sCommand.SIOOMode         = QSPI_SIOO_INST_EVERY_CMD;     //每次都发送指令

  if (HAL_QSPI_Command( handle, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }
  
  /* Configure automatic polling mode to wait for write enabling ---- */  
  sConfig.Match           = 0x02;
  sConfig.Mask            = 0x02;
  sConfig.MatchMode       = QSPI_MATCH_MODE_AND;
  sConfig.StatusBytesSize = 1;
  sConfig.Interval        = 0x10;
  sConfig.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;

  sCommand.Instruction    = QSPI_READ_STATUS_REG_CMD;
  sCommand.DataMode       = QSPI_DATA_1_LINE;

  if (HAL_QSPI_AutoPolling( handle, &sCommand, &sConfig, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

	return QSPI_OK;
}

#endif




/*
static QSPI_StaticTypeDef QSPI_AutoPollingMemReady(QSPI_HandleTypeDef *handle, uint32_t timeout)	--Ready auto polling memory
A function to ready auto polling memory
Return a integer value(default QSPI_OK), a parameter for QSPI handle, 
a parameter for timeout(uint32_t).
*/
static QSPI_StaticTypeDef QSPI_AutoPollingMemReady(QSPI_HandleTypeDef *handle)
{
  uint32_t cnt = 0;
  uint8_t _RegVal;
	
	do
	{
		QSPI_Read_SR(QSPI_READ_STATUS_REG_CMD, &_RegVal , 1);   // 读状态寄存器 1 

		if((_RegVal & 0x01) == 0x00)
			return QSPI_OK;
		
		cnt ++;
		
	}while( cnt < QSPI_WAIT_MAX_TIME );  

	return QSPI_OUT_TIME;
}


/**
  * @brief  This function configure the dummy cycles on memory side.
  * @param  hqspi: QSPI handle
  * @retval None
  */
QSPI_StaticTypeDef QSPI_DummyCyclesCfg(QSPI_HandleTypeDef *hqspi)
{
  QSPI_CommandTypeDef sCommand;
  uint8_t reg;
  
  if(QSPI_WorkMode)   // qpi 
  {
    sCommand.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
    sCommand.DataMode          = QSPI_DATA_4_LINES;
  }
  else
  {
    sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    sCommand.DataMode          = QSPI_DATA_1_LINE;  
  }


  /* Read Volatile Configuration register --------------------------- */
//  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  sCommand.Instruction       = QSPI_READ_VOL_CFG_REG_CMD;
  sCommand.AddressMode       = QSPI_ADDRESS_NONE;
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
//  sCommand.DataMode          = QSPI_DATA_1_LINE;
  sCommand.DummyCycles       = 0;
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  sCommand.SIOOMode         = QSPI_SIOO_INST_EVERY_CMD;
  sCommand.NbData            = 1;

  if (HAL_QSPI_Command( hqspi , &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  if (HAL_QSPI_Receive( hqspi , &reg, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Enable write operations ---------------------------------------- */
  QSPI_WriteEnable( hqspi );

  /* Write Volatile Configuration register (with new dummy cycles) -- */  
  sCommand.Instruction = QSPI_WRITE_VOL_CFG_REG_CMD;
  MODIFY_REG(reg, 0xF0, (QSPI_DUMMY_CYCLES_READ_QUAD << POSITION_VAL(0xF0)));
      
  if (HAL_QSPI_Command(hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  if (HAL_QSPI_Transmit(hqspi, &reg, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  return QSPI_OK;
}



/*
**************************************************************************************
函数名称：QSPI_Receive
函数功能：QSPI接收指定长度的数据
参数：    _pBuf:数据存储区
          _NumByteToRead:要读取的字节数(最大0xFFFFFFFF)
返回值：QSPI_OK读取成功，否则失败
**************************************************************************************
*/
static QSPI_StaticTypeDef QSPI_Receive(uint8_t * _pBuf, uint32_t _NumByteToRead)
{
    hqspi.Instance->DLR = _NumByteToRead-1;                           //配置数据长度
    if(HAL_QSPI_Receive(&hqspi, _pBuf, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) 
      return QSPI_ERROR; 

      return QSPI_OK;
}

/*
**************************************************************************************
函数名称：QSPI_Transmit
函数功能：QSPI接收指定长度的数据
参数：    _pBuf:发送数据缓冲区首地址
          _NumByteToRead:要传输的数据长度
返回值：QSPI_OK读取成功，否则失败
**************************************************************************************
*/
static QSPI_StaticTypeDef QSPI_Transmit(uint8_t * _pBuf, uint32_t _NumByteToRead)
{
    hqspi.Instance->DLR = _NumByteToRead-1;                            //配置数据长度
    if(HAL_QSPI_Transmit(&hqspi, (uint8_t *) _pBuf, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) 
      return QSPI_ERROR; 

      return QSPI_OK;
}



static QSPI_StaticTypeDef QSPI_SendCmdData( uint8_t  __Instruction,       //  发送指令
                                             uint32_t __InstructionMode,   //  指令模式
                                             uint32_t __AddressMode,       //  地址模式
                                             uint32_t __AddressSize,       //  地址长度  
                                             uint32_t __DataMode,          //  数据模式
                                             uint32_t __NbData,            //  数据读写字节数
                                             uint32_t __DummyCycles,       //  设置空指令周期数
                                             uint32_t __Address,           //  发送到的目的地址
                                             uint8_t  *_pBuf,              //  待发送的数据
                                             __SEND_CMD_DATA_T  _SendCmdDat
                                           )
{
  QSPI_CommandTypeDef     sCommand;
  
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;      //每次都发送指令
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;     //无交替字节
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;         //关闭DDR模式
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  
  sCommand.Instruction       = __Instruction;                  //指令
  sCommand.DummyCycles       = __DummyCycles;                  //设置空指令周期数
  sCommand.Address           = __Address;                      //发送到的目的地址
//  sCommand.NbData            = __NbData;                     //这个地方不使用

  sCommand.InstructionMode   = __InstructionMode;              //指令模式
  sCommand.AddressMode       = __AddressMode;                  //地址模式
  sCommand.AddressSize       = __AddressSize;                  //地址长度  
  sCommand.DataMode          = __DataMode;                     //数据模式  

  if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }
  if( _SendCmdDat == QSPI_SEND_DAT)
  {
    if(QSPI_Transmit( ( uint8_t * )_pBuf, __NbData) != QSPI_OK)
    {
      return QSPI_ERROR;
    }
  }
  
  return QSPI_OK; 
}




