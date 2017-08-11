
/*
********************************************************************************************************
MT25Q QSPI Flash Configuration File


STM32F7µÄquad-spi½Ó¿ÚÖ÷ÒªÌØµã£º
£¨1£©ÈýÖÖ¹¤×÷Ä£Ê½
£¨2£©Dual-FlashÄ£Ê½£¬¿ÉÒÔÍ¬Ê±½ÓÁ½Æ¬Flash£¬¹²ÓÃCLKºÍCSÆ¬Ñ¡Ïß¡£ÕâÑù¿ÉÒÔ×î¶àÍ¬Ê±´«Êä8Î»Êý¾Ý£¨4+4£©
£¨3£©Ö§³ÖSDRºÍDDR
£¨4£©¼ä½ÓÄ£Ê½µÄDMAÍ¨µÀ
£¨5£©ÄÚÇ¶½ÓÊÕºÍ·¢ËÍFIFO
£¨6£©Ö§³ÖFIFO threshold, timeout, operation complete, access errorËÄÖÖÖÐ¶Ï



---------------------------------------------------------------------------------------
     ±í2 ¿ØÖÆÆ÷×Ô¶¯Ê¶±ðº¬ÒåµÄÃüÁî
=======================================================================================
ÃüÁî                   º¬Òå                               ÃèÊö

READ             ¶ÁÊý¾Ý£¬CMD=0x03                  ÃüÁîÍ¨¹ýD0·¢³ö£»Êý¾ÝÍ¨¹ýD0½ÓÊÕ

FAST_READ        ¿ìËÙ¶ÁÊý¾Ý£¬CMD=0xB               ÃüÁîÍ¨¹ýD0·¢³ö£»Êý¾ÝÍ¨¹ýD0½ÓÊÕ

DOR              Ë«IO¶ÁÊý¾Ý£¬CMD=0x3B              ÃüÁîÍ¨¹ýD0·¢³ö£»Êý¾ÝÍ¨¹ýD[1:0]½ÓÊÕ

QOR              ËÄIOÏß¶ÁÊý¾Ý£¬CMD=0x6B            ÃüÁîÍ¨¹ýD0·¢³ö£»Êý¾ÝÍ¨¹ýD[3:0]½ÓÊÕ

DIOR             Ë«IOÃüÁî£¬ËÄIOÊý¾Ý£¬CMD=BB        ÃüÁîÍ¨¹ýD[1:0]·¢³ö£¬Êý¾ÝÍ¨¹ýD[3:0]½ÓÊÕ

QIOR             ËÄIOÃüÁî£¬ËÄIOÊý¾Ý£¬CMD=EB        ÃüÁîÍ¨¹ýD[3:0]·¢³ö£¬Êý¾ÝÍ¨¹ýD[3:0]½ÓÊÕ

PP               Ò³±à³ÌÃüÁî£¬CMD=02                ÃüÁîÍ¨¹ýD0·¢³ö£¬Êý¾ÝÍ¨¹ýD0·¢³ö

QPP              ËÄIOÒ³±à³ÌÃüÁî£¬CMD=32»ò38        ÃüÁîÍ¨¹ýD0·¢³ö£¬Êý¾ÝÍ¨¹ýD0[3:0]·¢³ö




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

static uint8_t QSPI_WorkMode = N25Q_SPI_MODE;		//QSPIÄ£Ê½±êÖ¾:0,SPIÄ£Ê½;1,QPIÄ£Ê½.

static QSPI_StaticTypeDef QSPI_WriteEnable(QSPI_HandleTypeDef *handle);
//static QSPI_StaticTypeDef QSPI_WriteDisable(QSPI_HandleTypeDef *handle);
static QSPI_StaticTypeDef QSPI_AutoPollingMemReady(QSPI_HandleTypeDef *handle);

static QSPI_StaticTypeDef QSPI_EnterFourBytesAddress(QSPI_HandleTypeDef *hqspi);
static QSPI_StaticTypeDef QSPI_Receive(uint8_t * _pBuf, uint32_t _NumByteToRead);
static QSPI_StaticTypeDef QSPI_Transmit(uint8_t * _pBuf, uint32_t _NumByteToRead);
static QSPI_StaticTypeDef QSPI_SendCmdData( uint8_t  __Instruction,       //  ·¢ËÍÖ¸Áî
                                             uint32_t __InstructionMode,   //  Ö¸ÁîÄ£Ê½
                                             uint32_t __AddressMode,       //  µØÖ·Ä£Ê½
                                             uint32_t __AddressSize,       //  µØÖ·³¤¶È  
                                             uint32_t __DataMode,          //  Êý¾ÝÄ£Ê½
                                             uint32_t __NbData,            //  Êý¾Ý¶ÁÐ´×Ö½ÚÊý
                                             uint32_t __DummyCycles,       //  ÉèÖÃ¿ÕÖ¸ÁîÖÜÆÚÊý
                                             uint32_t __Address,           //  ·¢ËÍµ½µÄÄ¿µÄµØÖ·
                                             uint8_t  *_pBuf,             //  ´ý·¢ËÍµÄÊý¾Ý
                                             __SEND_CMD_DATA_T  _SendCmdDat
                                           );

QSPI_StaticTypeDef QSPI_Read_SR(uint8_t ReadReg, uint8_t * RegValue, uint8_t ReadRegNum);


/*
**************************************************************************************
º¯ÊýÃû³Æ£ºQSPI_UserInit
º¯Êý¹¦ÄÜ£ºQSPI ³õÊ¼»¯£¬IO¿Ú³õÊ¼»¯£¬»òÕßQSPI Ð¾Æ¬µÄÏà¹ØÐÅÏ¢¼°ÈÝÁ¿µÈ
²ÎÊý£º   ÎÞ
·µ»ØÖµ£º QSPI_OK ³õÊ¼»¯³É¹¦£¬ÆäËûÖµÊ§°Ü
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
  	// QSPI memory reset  ÔÚµ÷ÊÔÊ±£¬Èç¹ûÊä³ö ¸ñÊ½²»Ö§³åÖØÐÂ¿ª¹Ø»ú²Ù×÷¼´¿É 
  __QspiStatus = QSPI_ResetMemory(&hqspi);
	if ( __QspiStatus != QSPI_OK)   // Õâ¸öº¯Êý²»ÄÜ¹»Ê¹ÓÃ£¬¾­³£»á¿¨ËÀ£
	{
    return QSPI_ERROR;
	}
	
	#endif
  
  // ×ÊÁÏ½éÉÜµÄ µÈ´ý tSHSL3 Ê±¼ä
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
      if(QSPI_Read_SR(QSPI_READ_FLAG_STATUS_REG_CMD, &_RegVal , 1) == QSPI_OK)   // ¶ÁÈ¡ 0x70¼Ä´æÆ÷£¬²é¿´ÊÇ·ñÎª4×Ö½ÚµØÖ·
      {
        if((_RegVal & 0x01) == 0x00)       //Èç¹û²»ÊÇ4×Ö½ÚµØÖ·Ä£Ê½,Ôò½øÈë4×Ö½ÚµØÖ·Ä£Ê½
        {
          __QspiStatus = QSPI_EnterFourBytesAddress(&hqspi);   // ÉèÖÃ4×Ö½ÚµØÖ·
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
  
        
  //QUAD,¿ìËÙ¶ÁÊý¾Ý,µØÖ·Îªaddress,4Ïß´«ÊäÊý¾Ý_32Î»µØÖ·_4Ïß´«ÊäµØÖ·_4Ïß´«ÊäÖ¸Áî,8¿ÕÖÜÆÚ,NumByteToRead¸öÊý¾Ý
  if(QSPI_SendCmdData(  QSPI_QUAD_INOUT_FAST_READ_4_BYTE_ADDR_CMD /*QSPI_FAST_READ_4_BYTE_ADDR_CMD*/ /*QSPI_FAST_READ_4_BYTE_ADDR_CMD*/ /*QSPI_QUAD_INOUT_FAST_READ_CMD*/  /*QSPI_QUAD_INOUT_FAST_READ_CMD*/ ,        // _Instruction,      ·¢ËÍÖ¸Áî
                        __InstructionMode,               // _InstructionMode,  Ö¸ÁîÄ£Ê½
                        __AddressMode,                   // _AddressMode,      µØÖ·Ä£Ê½
                        QSPI_ADDRESS_32_BITS,            // _AddressSize,      µØÖ·³¤¶È  
                        __DataMode,                      // _DataMode,         Êý¾ÝÄ£Ê½
                        size,                            // _NbData,           Êý¾Ý¶ÁÐ´×Ö½ÚÊý
                        10 ,                             // _DummyCycles,      ÉèÖÃ¿ÕÖ¸ÁîÖÜÆÚÊý Óë QSPI_SET_READ_PARAM Õâ¸öÖ¸ÁîÉèÖÃµÄÖµÒ»ÖÂ
                        address,                         // _Address,          ·¢ËÍµ½µÄÄ¿µÄµØÖ·
                        &_RegVal,                        //  *_pBuf,           ¶ÁÈ¡µÄÊý¾Ý£¬´Ë´¦Ã»ÓÐÊ¹ÓÃ
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
º¯ÊýÃû³Æ£ºQSPI_Read_SR
º¯Êý¹¦ÄÜ£º¶ÁÈ¡¼Ä´æÆ÷Öµ
²ÎÊý£º    ReadReg ÐèÒª¶ÁÈ¡µÄ¼Ä´æÆ÷
          @ QSPI_READ_ENHANCED_VOL_CFG_REG_CMD, QSPI_READ_EXT_ADDR_REG_CMD, QSPI_READ_NONVOL_CFG_REG_CMD
            QSPI_READ_FLAG_STATUS_REG_CMD, QSPI_READ_STATUS_REG_CMD  ... ...
          * RegValue  ¼Ä´æÆ÷Öµ
          ReadRegNum  ¶ÁÈ¡¼Ä´æÆ÷×Ö½ÚÊý
·µ»ØÖµ£ºQSPI_OK¶ÁÈ¡³É¹¦£¬·ñÔòÊ§°Ü
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
  
  
  if(QSPI_SendCmdData(  ReadReg,                // _Instruction,      ·¢ËÍÖ¸Áî
                        __InstructionMode,      // _InstructionMode,  Ö¸ÁîÄ£Ê½
                        QSPI_ADDRESS_NONE,      // _AddressMode,      µØÖ·Ä£Ê½
                        QSPI_ADDRESS_8_BITS,    // _AddressSize,      µØÖ·³¤¶È  
                        __DataMode,             // _DataMode,         Êý¾ÝÄ£Ê½
                        ReadRegNum,             // _NbData,           Êý¾Ý¶ÁÐ´×Ö½ÚÊý
                        0,                      // _DummyCycles,      ÉèÖÃ¿ÕÖ¸ÁîÖÜÆÚÊý
                        0,                      // _Address,          ·¢ËÍµ½µÄÄ¿µÄµØÖ·
                        RegValue,               //  *_pBuf,           ´ý·¢ËÍµÄÊý¾Ý
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
º¯ÊýÃû³Æ£ºQSPI_Write_SR
º¯Êý¹¦ÄÜ£ºÐ´¼Ä´æÆ÷Öµ
²ÎÊý£º    WriteReg ÐèÒªÐ´ÈëµÄ¼Ä´æÆ÷
          @ QSPI_WRITE_ENHANCED_VOL_CFG_REG_CMD, QSPI_WRITE_EXT_ADDR_REG_CMD, QSPI_WRITE_NONVOL_CFG_REG_CMD
            QSPI_WRITE_FLAG_STATUS_REG_CMD, QSPI_WRITE_STATUS_REG_CMD ... ...
          RegValue    ´ýÐ´ÈëµÄ¼Ä´æÆ÷Öµ
          WriteRegNum  Ð´ÈëµÄ¼Ä´æÆ÷µÄ×Ö½ÚÊý
·µ»ØÖµ£ºQSPI_OK¶ÁÈ¡³É¹¦£¬·ñÔòÊ§°Ü
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

  if(QSPI_SendCmdData(  WriteReg,               // _Instruction,      ·¢ËÍÖ¸Áî
                        __InstructionMode,      // _InstructionMode,  Ö¸ÁîÄ£Ê½
                        QSPI_ADDRESS_NONE,      // _AddressMode,      µØÖ·Ä£Ê½
                        QSPI_ADDRESS_8_BITS,    // _AddressSize,      µØÖ·³¤¶È  
                        __DataMode,             // _DataMode,         Êý¾ÝÄ£Ê½
                        WriteRegNum,            // _NbData,           Êý¾Ý¶ÁÐ´×Ö½ÚÊý
                        0,                      // _DummyCycles,      ÉèÖÃ¿ÕÖ¸ÁîÖÜÆÚÊý
                        0,                      // _Address,          ·¢ËÍµ½µÄÄ¿µÄµØÖ·
                        &RegValue,              //  *_pBuf,           ´ý·¢ËÍµÄÊý¾Ý
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
º¯ÊýÃû³Æ£ºQSPI_Read_ID
º¯Êý¹¦ÄÜ£º¶ÁÈ¡ ID Öµ
²ÎÊý£º    ReadID ÐèÒª¶ÁÈ¡µÄID¼Ä´æÆ÷Ö¸Áî
          * _pIdBuf  ¶ÁÈ¡IDÐÅÏ¢µÄÊý¾Ý»º³åÇø
         ReadIdNum  ÐèÒª¶ÁÈ¡id µÄÊý¾Ý×Ö½ÚÊýÁ¿
·µ»ØÖµ£ºQSPI_OK¶ÁÈ¡³É¹¦£¬·ñÔòÊ§°Ü
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
  

  if(QSPI_SendCmdData(  ReadID,                   // _Instruction,      ·¢ËÍÖ¸Áî
                        __InstructionMode,        // _InstructionMode,  Ö¸ÁîÄ£Ê½
                        QSPI_ADDRESS_NONE,        // _AddressMode,      µØÖ·Ä£Ê½
                        QSPI_ADDRESS_8_BITS,      // _AddressSize,      µØÖ·³¤¶È  
                        __DataMode,               // _DataMode,         Êý¾ÝÄ£Ê½
                        ReadIdNum,                // _NbData,           Êý¾Ý¶ÁÐ´×Ö½ÚÊý
                        0,                        // _DummyCycles,      ÉèÖÃ¿ÕÖ¸ÁîÖÜÆÚÊý
                        0,                        // _Address,          ·¢ËÍµ½µÄÄ¿µÄµØÖ·
                        &_pIdBuf[0],              //  *_pBuf,           ´ý·¢ËÍµÄÊý¾Ý
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
º¯ÊýÃû³Æ£ºQSPI_Quad_Enter
º¯Êý¹¦ÄÜ£º½øÈë QUAD Ä£Ê½
·µ»ØÖµ£ºQSPI_OK¶ÁÈ¡³É¹¦£¬ÆäËûÖµÊ§°Ü
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
  
  if(_RegVal[0] == 0xff)   // Ð¾Æ¬³õÊ¼Öµ = 0xDF
  {
    _RegVal[0] = 0x2F;   // bit7=0,bit6=1 QUAD Ä£Ê½, bit5 ÏµÍ³±£ÁôÇÒ±ØÐëÉèÖÃÎª0///guangqiang ´Ë´¦Ðèµ÷ÊÔ
    if(QSPI_Write_SR(QSPI_WRITE_ENHANCED_VOL_CFG_REG_CMD, _RegVal[0], 1) == QSPI_OK)  // ÉèÖÃÎª QUAD Ä£Ê½ reg 0x61
    {
      QSPI_WorkMode = N25Q_QUAD_MODE;
      
      
      _RegVal [0]= 0xaa;
      QSPI_Read_SR(QSPI_READ_ENHANCED_VOL_CFG_REG_CMD, &_RegVal[0], 1);  // ²âÊÔ¶Á³öÀ´µÄÖµÓëÐ´ÈëµÄÖµÊÇ·ñÒ»ÖÂ£¬Èç¹ûÊÇ 0x1F  ËµÃ÷ok
      DBG_LOG(("QSPI_READ_VOL_CFG_REG_CMD 2 =  0x%X\r\n", _RegVal[0]));    
      
      QSPI_Read_SR(QSPI_READ_VOL_CFG_REG_CMD, &_RegVal[0],1);  // 0xFB ËµÃ÷ok
      
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
º¯ÊýÃû³Æ£ºQSPI_WritePageByte
º¯ÊýÃèÊö£º page Ð´£¬ÔÚÐ´Ç°±ØÐëÏÈ²Á³ýÏàÓ¦µÄÉÈÇø£¬²»Ö§³Ö¿çÒ³Ð´
²ÎÊý£º_pBuf          Êý¾Ý»º³åÇø
      _uiWriteAddr   Ð´ÈëµÄµØÖ·
      _size          Ð´ÈëÊý¾Ý´óÐ¡,Ã¿´Î×î´óÄÜ¹»Ð´Èë256×Ö½Ú
·µ»ØÖµ£ºQSPI_OK ±íÊ¾³É¹¦£¬ÆäËûÊ§°Ü
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
    
       QSPI_EXT_QUAD_IN_FAST_PROG_CMD  Ð´Èë 8192*4 ÐèÒª 2018ms
       QSPI_PAGE_PROG_CMD              Ð´Èë 8192*4 ÐèÒª 2035ms
       QSPI_PAGE_PROG_4_BYTE_ADDR_CMD  Ð´Èë 8192*4 ÐèÒª 2029ms
       QSPI_QUAD_IN_FAST_PROG_CMD      Ð´Èë 8192*4 ÐèÒª 2014ms - 2023ms
       QSPI_EXT_QUAD_IN_FAST_PROG_CMD  Ð´Èë 8192*4 ÐèÒª 2025ms

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
  
  if(QSPI_SendCmdData(  QSPI_PAGE_PROG_4_BYTE_ADDR_CMD,   //QSPI_QUAD_IN_FAST_PROG_CMD, //QSPI_PAGE_PROG_4_BYTE_ADDR_CMD, QSPI_PAGE_PROG_CMD,         // _Instruction,      ·¢ËÍÖ¸Áî
                        __InstructionMode,                // _InstructionMode,  Ö¸ÁîÄ£Ê½
                        __AddressMode,                    // _AddressMode,      µØÖ·Ä£Ê½
                        QSPI_ADDRESS_32_BITS,             // _AddressSize,      µØÖ·³¤¶È  
                        __DataMode,                       // _DataMode,         Êý¾ÝÄ£Ê½
                        _size,                            // _NbData,           Êý¾Ý¶ÁÐ´×Ö½ÚÊý
                        0,                                // _DummyCycles,      ÉèÖÃ¿ÕÖ¸ÁîÖÜÆÚÊý
                        _uiWriteAddr,                     // _Address,          ·¢ËÍµ½µÄÄ¿µÄµØÖ·
                        (uint8_t *) _pBuf,               //  *_pBuf,           ´ý·¢ËÍµÄÊý¾Ý
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
º¯ÊýÃû³Æ£ºQSPI_WriteBuff
º¯ÊýÃèÊö£ºÖ§³Ö¿ä page Ð´£¬ÔÚÐ´Ç°±ØÐëÏÈ²Á³ýÏàÓ¦µÄÉÈÇø
²ÎÊý£º_pBuf          Êý¾Ý»º³åÇø
      _uiWriteAddr   Ð´ÈëµÄµØÖ·
      _size          Ð´ÈëÊý¾Ý´óÐ¡,
·µ»ØÖµ£ºQSPI_OK ±íÊ¾³É¹¦£¬ÆäËûÊ§°Ü
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
º¯ÊýÃû³Æ£ºQSPI_Write_NoCheck
º¯ÊýÃèÊö£º ÎÞ¼ìÑéÐ´ QSPI FLASH, ±ØÐëÈ·±£ËùÐ´µÄµØÖ··¶Î§ÄÚµÄÊý¾ÝÈ«²¿Îª0XFF,·ñÔòÔÚ·Ç0XFF´¦
           Ð´ÈëµÄÊý¾Ý½«Ê§°Ü! ¾ßÓÐ×Ô¶¯»»Ò³¹¦ÄÜ, ÔÚÖ¸¶¨µØÖ·¿ªÊ¼Ð´ÈëÖ¸¶¨³¤¶ÈµÄÊý¾Ý,µ«ÊÇÒª
           È·±£µØÖ·²»Ô½½ç!
²ÎÊý£º_pBuf          Êý¾Ý»º³åÇø
      _uiWriteAddr   Ð´ÈëµÄµØÖ·
      _NumByteToWrite ÒªÐ´ÈëµÄ×Ö½ÚÊý(×î´ó65535)
·µ»ØÖµ£ºQSPI_OK ±íÊ¾³É¹¦£¬ÆäËûÊ§°Ü
**************************************************************************************
*/
QSPI_StaticTypeDef QSPI_Write_NoCheck(uint8_t* _pBuf, uint32_t _uiWriteAddr, uint32_t  _NumByteToWrite)   
{ 			 		 
	uint32_t pageremain;	   
	pageremain = 256 - (_uiWriteAddr % QSPI_PAGE_SIZE); //µ¥Ò³Ê£ÓàµÄ×Ö½ÚÊý		 	    
	if(_NumByteToWrite <= pageremain)
    pageremain = _NumByteToWrite;           //²»´óÓÚ256¸ö×Ö½Ú
	while(1)
	{   
		if(QSPI_WritePageByte(_pBuf, _uiWriteAddr, pageremain) != QSPI_OK)
      return QSPI_ERROR;
    
		if(_NumByteToWrite == pageremain)         //Ð´Èë½áÊøÁË
      break;
    
	 	else                                      //NumByteToWrite>pageremain
		{
			_pBuf        += pageremain;
			_uiWriteAddr += pageremain;	

			_NumByteToWrite -= pageremain;			    //¼õÈ¥ÒÑ¾­Ð´ÈëÁËµÄ×Ö½ÚÊý
			if(_NumByteToWrite > QSPI_PAGE_SIZE)
        pageremain = QSPI_PAGE_SIZE;          //Ò»´Î¿ÉÒÔÐ´Èë256¸ö×Ö½Ú
			else 
        pageremain = _NumByteToWrite; 	      //²»¹»256¸ö×Ö½ÚÁË
		}
	} 
  return QSPI_OK;
}


/*
**************************************************************************************
º¯ÊýÃû³Æ£ºQSPI_WriteBuffAutoEraseSector
º¯ÊýÃèÊö£ºÔÚÖ¸¶¨µØÖ·¿ªÊ¼Ð´ÈëÖ¸¶¨³¤¶ÈµÄÊý¾Ý, ¸Ãº¯Êý´ø²Á³ýÉÈÇø²Ù×÷¹¦ÄÜ !
²ÎÊý£º_pBuf           Êý¾Ý»º³åÇø
      _uiWriteAddr    Ð´ÈëµÄµØÖ·
      _NumByteToWrite Ð´ÈëÊý¾Ý´óÐ¡,
·µ»ØÖµ£ºQSPI_OK ±íÊ¾³É¹¦£¬ÆäËûÊ§°Ü
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
     
 	secpos = _uiWriteAddr / 4096;           //ÉÈÇøµØÖ·  
	secoff = _uiWriteAddr % 4096;           //ÔÚÉÈÇøÄÚµÄÆ«ÒÆ
	secremain = 4096 - secoff;              //ÉÈÇøÊ£Óà¿Õ¼ä´óÐ¡   

 	if(_NumByteToWrite <= secremain)
    secremain = _NumByteToWrite;          //²»´óÓÚ4096¸ö×Ö½Ú
	while(1) 
	{	
		if(QSPI_ReadBuff(_pStr, (secpos * 4096), 4096) !=  QSPI_OK)  //¶Á³öÕû¸öÉÈÇøµÄÄÚÈÝ
      return QSPI_ERROR;
    
		for(i=0;i<secremain;i++)//Ð£ÑéÊý¾Ý
		{
			if(_pStr[secoff+i] != 0XFF)
        break;//ÐèÒª²Á³ý  	  
		}
		if(i<secremain)//ÐèÒª²Á³ý
		{
			if(QSPI_EraseSector_4K(secpos) !=  QSPI_OK)   //²Á³ýÕâ¸öÉÈÇø
        return QSPI_ERROR;
      
			for(i=0;i<secremain;i++)	   //¸´ÖÆ
			{
				_pStr[i+secoff] = _pBuf[i];	  
			}
			if(QSPI_Write_NoCheck(_pStr, (secpos * 4096), 4096) !=  QSPI_OK)    //Ð´ÈëÕû¸öÉÈÇø 
        return QSPI_ERROR;
		}
    else 
    {
      if(QSPI_Write_NoCheck(_pBuf, _uiWriteAddr, secremain) !=  QSPI_OK)     //Ð´ÒÑ¾­²Á³ýÁËµÄ,Ö±½ÓÐ´ÈëÉÈÇøÊ£ÓàÇø¼ä. 	
        return QSPI_ERROR;
    }
    
		if(_NumByteToWrite == secremain)  break;  //Ð´Èë½áÊøÁË
    
		else                                    //Ð´ÈëÎ´½áÊø
		{
			secpos ++;                            //ÉÈÇøµØÖ·Ôö1
			secoff = 0;                           //Æ«ÒÆÎ»ÖÃÎª0 	 

		  _pBuf           += secremain;         //Ö¸ÕëÆ«ÒÆ
			_uiWriteAddr    += secremain;         //Ð´µØÖ·Æ«ÒÆ	   
		  _NumByteToWrite -= secremain;				  //×Ö½ÚÊýµÝ¼õ
      
			if(_NumByteToWrite > 4096)          // 4096
        secremain = 4096;	                  //ÏÂÒ»¸öÉÈÇø»¹ÊÇÐ´²»Íê
			else 
        secremain = _NumByteToWrite;			    //ÏÂÒ»¸öÉÈÇø¿ÉÒÔÐ´ÍêÁË
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

  if(QSPI_SendCmdData(  QSPI_SUBSECTOR_4K_ERASE_CMD,   // _Instruction,      ·¢ËÍÖ¸Áî
                        __InstructionMode,          // _InstructionMode,  Ö¸ÁîÄ£Ê½
                        __AddressMode,              // _AddressMode,      µØÖ·Ä£Ê½
                        QSPI_ADDRESS_32_BITS,       // _AddressSize,      µØÖ·³¤¶È  
                        QSPI_DATA_NONE,             // _DataMode,         Êý¾ÝÄ£Ê½
                        0,                          // _NbData,           Êý¾Ý¶ÁÐ´×Ö½ÚÊý
                        0,                          // _DummyCycles,      ÉèÖÃ¿ÕÖ¸ÁîÖÜÆÚÊý
                        Sector_address,             // _Address,          ·¢ËÍµ½µÄÄ¿µÄµØÖ·
                        &_RegVal,                   //  *_pBuf,           ´ý·¢ËÍµÄÊý¾Ý
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

  if(QSPI_SendCmdData( QSPI_SUBSECTOR_32K_ERASE_CMD,   // _Instruction,      ·¢ËÍÖ¸Áî
  
                        __InstructionMode,          // _InstructionMode,  Ö¸ÁîÄ£Ê½
                        __AddressMode,              // _AddressMode,      µØÖ·Ä£Ê½
                        QSPI_ADDRESS_32_BITS,       // _AddressSize,      µØÖ·³¤¶È  
                        QSPI_DATA_NONE,             // _DataMode,         Êý¾ÝÄ£Ê½
                        0,                          // _NbData,           Êý¾Ý¶ÁÐ´×Ö½ÚÊý
                        0,                          // _DummyCycles,      ÉèÖÃ¿ÕÖ¸ÁîÖÜÆÚÊý
                        Sector_address,             // _Address,          ·¢ËÍµ½µÄÄ¿µÄµØÖ·
                        &_RegVal,                   //  *_pBuf,           ´ý·¢ËÍµÄÊý¾Ý
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

²Á³ý N25Q256A13xx Õû¸öÐ¾Æ¬Ê±¼ä 233879ms ×óÓÒ, ½Ó½ü4·ÖÖÓ
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
  
  if(QSPI_SendCmdData(  QSPI_BULK_ERASE_CMD,        // _Instruction,      ·¢ËÍÖ¸Áî
                        __InstructionMode,          // _InstructionMode,  Ö¸ÁîÄ£Ê½
                        QSPI_ADDRESS_NONE,          // _AddressMode,      µØÖ·Ä£Ê½
                        QSPI_ADDRESS_8_BITS,        // _AddressSize,      µØÖ·³¤¶È  
                        QSPI_DATA_NONE,             // _DataMode,         Êý¾ÝÄ£Ê½
                        0,                          // _NbData,           Êý¾Ý¶ÁÐ´×Ö½ÚÊý
                        0,                          // _DummyCycles,      ÉèÖÃ¿ÕÖ¸ÁîÖÜÆÚÊý
                        0,                          // _Address,          ·¢ËÍµ½µÄÄ¿µÄµØÖ·
                        &_RegVal,                   //  *_pBuf,           ´ý·¢ËÍµÄÊý¾Ý
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
µ÷ÊÔ ok
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
N25Q256/N25Q512 ×ÊÁÏ Table 9: Status Register Bit DefinitionsÓÐ½éÉÜ
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
  
  if(QSPI_SendCmdData(  QSPI_WRITE_ENABLE_CMD,        // _Instruction,      ·¢ËÍÖ¸Áî
                        __InstructionMode,            // _InstructionMode,  Ö¸ÁîÄ£Ê½
                        QSPI_ADDRESS_NONE,            // _AddressMode,      µØÖ·Ä£Ê½
                        QSPI_ADDRESS_8_BITS,          // _AddressSize,      µØÖ·³¤¶È  
                        QSPI_DATA_NONE,               // _DataMode,         Êý¾ÝÄ£Ê½
                        0,                            // _NbData,           Êý¾Ý¶ÁÐ´×Ö½ÚÊý
                        0,                            // _DummyCycles,      ÉèÖÃ¿ÕÖ¸ÁîÖÜÆÚÊý
                        0,                            // _Address,          ·¢ËÍµ½µÄÄ¿µÄµØÖ·
                        &_RegVal,                     //  *_pBuf,           ´ý·¢ËÍµÄÊý¾Ý
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
  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;     //Ö¸ÁîÄ£Ê½
  sCommand.Instruction       = QSPI_WRITE_DISABLE_CMD;       //Ö¸Áî
  sCommand.AddressMode       = QSPI_ADDRESS_NONE;           //µØÖ·Ä£Ê½
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;   //ÎÞ½»Ìæ×Ö½Ú
  sCommand.DataMode          = QSPI_DATA_NONE;              //Êý¾ÝÄ£Ê½
  sCommand.DummyCycles       = 0;                           //ÉèÖÃ¿ÕÖ¸ÁîÖÜÆÚÊý
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;       //¹Ø±ÕDDRÄ£Ê½
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  sCommand.SIOOMode         = QSPI_SIOO_INST_EVERY_CMD;     //Ã¿´Î¶¼·¢ËÍÖ¸Áî

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
		QSPI_Read_SR(QSPI_READ_STATUS_REG_CMD, &_RegVal , 1);   // ¶Á×´Ì¬¼Ä´æÆ÷ 1 

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
º¯ÊýÃû³Æ£ºQSPI_Receive
º¯Êý¹¦ÄÜ£ºQSPI½ÓÊÕÖ¸¶¨³¤¶ÈµÄÊý¾Ý
²ÎÊý£º    _pBuf:Êý¾Ý´æ´¢Çø
          _NumByteToRead:Òª¶ÁÈ¡µÄ×Ö½ÚÊý(×î´ó0xFFFFFFFF)
·µ»ØÖµ£ºQSPI_OK¶ÁÈ¡³É¹¦£¬·ñÔòÊ§°Ü
**************************************************************************************
*/
static QSPI_StaticTypeDef QSPI_Receive(uint8_t * _pBuf, uint32_t _NumByteToRead)
{
    hqspi.Instance->DLR = _NumByteToRead-1;                           //ÅäÖÃÊý¾Ý³¤¶È
    if(HAL_QSPI_Receive(&hqspi, _pBuf, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) 
      return QSPI_ERROR; 

      return QSPI_OK;
}

/*
**************************************************************************************
º¯ÊýÃû³Æ£ºQSPI_Transmit
º¯Êý¹¦ÄÜ£ºQSPI½ÓÊÕÖ¸¶¨³¤¶ÈµÄÊý¾Ý
²ÎÊý£º    _pBuf:·¢ËÍÊý¾Ý»º³åÇøÊ×µØÖ·
          _NumByteToRead:Òª´«ÊäµÄÊý¾Ý³¤¶È
·µ»ØÖµ£ºQSPI_OK¶ÁÈ¡³É¹¦£¬·ñÔòÊ§°Ü
**************************************************************************************
*/
static QSPI_StaticTypeDef QSPI_Transmit(uint8_t * _pBuf, uint32_t _NumByteToRead)
{
    hqspi.Instance->DLR = _NumByteToRead-1;                            //ÅäÖÃÊý¾Ý³¤¶È
    if(HAL_QSPI_Transmit(&hqspi, (uint8_t *) _pBuf, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) 
      return QSPI_ERROR; 

      return QSPI_OK;
}



static QSPI_StaticTypeDef QSPI_SendCmdData( uint8_t  __Instruction,       //  ·¢ËÍÖ¸Áî
                                             uint32_t __InstructionMode,   //  Ö¸ÁîÄ£Ê½
                                             uint32_t __AddressMode,       //  µØÖ·Ä£Ê½
                                             uint32_t __AddressSize,       //  µØÖ·³¤¶È  
                                             uint32_t __DataMode,          //  Êý¾ÝÄ£Ê½
                                             uint32_t __NbData,            //  Êý¾Ý¶ÁÐ´×Ö½ÚÊý
                                             uint32_t __DummyCycles,       //  ÉèÖÃ¿ÕÖ¸ÁîÖÜÆÚÊý
                                             uint32_t __Address,           //  ·¢ËÍµ½µÄÄ¿µÄµØÖ·
                                             uint8_t  *_pBuf,              //  ´ý·¢ËÍµÄÊý¾Ý
                                             __SEND_CMD_DATA_T  _SendCmdDat
                                           )
{
  QSPI_CommandTypeDef     sCommand;
  
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;      //Ã¿´Î¶¼·¢ËÍÖ¸Áî
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;     //ÎÞ½»Ìæ×Ö½Ú
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;         //¹Ø±ÕDDRÄ£Ê½
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  
  sCommand.Instruction       = __Instruction;                  //Ö¸Áî
  sCommand.DummyCycles       = __DummyCycles;                  //ÉèÖÃ¿ÕÖ¸ÁîÖÜÆÚÊý
  sCommand.Address           = __Address;                      //·¢ËÍµ½µÄÄ¿µÄµØÖ·
//  sCommand.NbData            = __NbData;                     //Õâ¸öµØ·½²»Ê¹ÓÃ

  sCommand.InstructionMode   = __InstructionMode;              //Ö¸ÁîÄ£Ê½
  sCommand.AddressMode       = __AddressMode;                  //µØÖ·Ä£Ê½
  sCommand.AddressSize       = __AddressSize;                  //µØÖ·³¤¶È  
  sCommand.DataMode          = __DataMode;                     //Êý¾ÝÄ£Ê½  

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




