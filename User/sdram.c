#include "sdram.h"

#ifdef DEBUG
#define DBG_LOG(x) printf x
#else
#define DBG_LOG(x) 
#endif

static volatile void delay(uint32_t t);

static volatile void delay(uint32_t t)
{
	uint32_t i,j,k;
	while(t--)
	{
		for(i=0;i<10;i++)
		{
			for(j=0;j<100;j++)
				{
					for(k=0;k<1000;k++)
					{
						k=k;
					}
				}
		}
	}
}

void SDRAM_Initialization_Sequence(SDRAM_HandleTypeDef *hsdram)
{
	uint32_t temp=0;
  //SDRAM控制器初始化完成以后还需要按照如下顺序初始化SDRAM
  SDRAM_Send_Cmd(0,FMC_SDRAM_CMD_CLK_ENABLE,1,0); //时钟配置使能
  //delay_us(500);                                  //至少延时200us
	delay(100);
  SDRAM_Send_Cmd(0,FMC_SDRAM_CMD_PALL,1,0);       //对所有存储区预充电
  SDRAM_Send_Cmd(0,FMC_SDRAM_CMD_AUTOREFRESH_MODE,8,0);//设置自刷新次数 
  //配置模式寄存器,SDRAM的bit0~bit2为指定突发访问的长度，
	//bit3为指定突发访问的类型，bit4~bit6为CAS值，bit7和bit8为运行模式
	//bit9为指定的写突发模式，bit10和bit11位保留位
	temp=(uint32_t)SDRAM_MODEREG_BURST_LENGTH_1       |	//设置突发长度:1(可以是1/2/4/8)
              SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL   |	//设置突发类型:连续(可以是连续/交错)
              SDRAM_MODEREG_CAS_LATENCY_3           |	//设置CAS值:3(可以是2/3)
              SDRAM_MODEREG_OPERATING_MODE_STANDARD |   //设置操作模式:0,标准模式
              SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;     //设置突发写模式:1,单点访问
  SDRAM_Send_Cmd(0,FMC_SDRAM_CMD_LOAD_MODE,1,temp);   //设置SDRAM的模式寄存器
    
  //刷新频率计数器(以SDCLK频率计数),计算方法:
	//COUNT=SDRAM刷新周期/行数-20=SDRAM刷新周期(us)*SDCLK频率(Mhz)/行数
  //我们使用的SDRAM刷新周期为64ms,SDCLK=216/2=108Mhz,行数为8192(2^13).
	//所以,COUNT=64*1000*108/8192-20=823
	HAL_SDRAM_ProgramRefreshRate(&hsdram1,823);	
}	

//向SDRAM发送命令
//bankx:0,向BANK5上面的SDRAM发送指令
//      1,向BANK6上面的SDRAM发送指令
//cmd:指令(0,正常模式/1,时钟配置使能/2,预充电所有存储区/3,自动刷新/4,加载模式寄存器/5,自刷新/6,掉电)
//refresh:自刷新次数
//regval:模式寄存器的定义
//返回值:0,正常;1,失败.
uint8_t SDRAM_Send_Cmd(uint8_t bankx,uint8_t cmd,uint8_t refresh,uint16_t regval)
{
    uint32_t target_bank=0;
    FMC_SDRAM_CommandTypeDef Command;
    
    if(bankx==0) target_bank=FMC_SDRAM_CMD_TARGET_BANK1;       
    else if(bankx==1) target_bank=FMC_SDRAM_CMD_TARGET_BANK2;   
    Command.CommandMode=cmd;                //命令
    Command.CommandTarget=target_bank;      //目标SDRAM存储区域
    Command.AutoRefreshNumber=refresh;      //自刷新次数
    Command.ModeRegisterDefinition=regval;  //要写入模式寄存器的值
    if(HAL_SDRAM_SendCommand(&hsdram1,&Command,0X1000)==HAL_OK) //向SDRAM发送命令
    {
        return 0;  
    }
    else return 1;    
}

//在指定地址(WriteAddr+Bank5_SDRAM_ADDR)开始,连续写入n个字节.
//pBuffer:字节指针
//WriteAddr:要写入的地址
//n:要写入的字节数
void FMC_SDRAM_WriteBuffer(uint8_t *pBuffer,uint32_t WriteAddr,uint32_t n)
{
	for(;n!=0;n--)
	{
		*(volatile uint8_t*)(Bank5_SDRAM_ADDR+WriteAddr)=*pBuffer;
		WriteAddr++;
		pBuffer++;
	}
}

//在指定地址((WriteAddr+Bank5_SDRAM_ADDR))开始,连续读出n个字节.
//pBuffer:字节指针
//ReadAddr:要读出的起始地址
//n:要写入的字节数
void FMC_SDRAM_ReadBuffer(uint8_t *pBuffer,uint32_t ReadAddr,uint32_t n)
{
	for(;n!=0;n--)
	{
		*pBuffer++=*(volatile uint8_t*)(Bank5_SDRAM_ADDR+ReadAddr);
		ReadAddr++;
	}
}

void fsmc_sdram_test(void)
{  
	uint32_t i=0;  	  
	uint32_t temp=0;	   
	uint32_t sval=0;	
	for(i=0;i<32*1024*1024;i+=16*1024)
	{
		*(volatile uint32_t*)(Bank5_SDRAM_ADDR+i)=temp; 
		temp++;
	}	  
 	for(i=0;i<32*1024*1024;i+=16*1024) 
	{	
  		temp=*(volatile uint32_t*)(Bank5_SDRAM_ADDR+i);
		if(i==0)sval=temp;
 		else if(temp<=sval)break;   		   
 	}		
	DBG_LOG(("SDRAM Capacity:%dKB...\r\n",(uint16_t)(temp-sval+1)*16));
}	

void read_write_test(uint32_t base)
{
	uint8_t tx[256]={"\0"};
	uint8_t rx[256]={"\0"};
	uint8_t *ptx=tx;
	uint8_t *prx=rx;
	uint16_t i=0;
	DBG_LOG(("-->BASE at 0x%X\r\n",base));
	//DBG_LOG(("Start Read/Write Test\r\n"));
	//DBG_LOG(("Set TX data:\r\n"));
	#if 1
	for(i=0;i<256;i++)
	{
		tx[i]=(uint8_t)i;
		//DBG_LOG(("%.2X ",tx[i]));
	}
	#endif
	FMC_SDRAM_WriteBuffer(ptx,base,256);
	FMC_SDRAM_ReadBuffer (prx,base,256);
	//DBG_LOG(("\r\nRX is :\r\n "));
	for(i=0;i<256;i++)
	{
		if(rx[i] != i)
		{
			DBG_LOG(("%.2X ",rx[i]));
		}
	}
	DBG_LOG(("\r\n"));
}

void sdram_rw_test(void)
{
	uint32_t base=0;
	while(base <= 0x20000)
	{
		read_write_test(base);
		base=base+256;
	}
}
