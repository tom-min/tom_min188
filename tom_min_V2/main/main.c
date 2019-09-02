#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "misc.h"

#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

#include "Tom_Common_Api.h"

#include "ff.h"

#define Beep_On  		GPIO_SetBits(GPIOD,GPIO_Pin_6)
#define Beep_Off  	GPIO_ResetBits(GPIOD,GPIO_Pin_6)

#define Led_On_1  	GPIO_ResetBits(GPIOB,GPIO_Pin_5)
#define Led_Off_1  	GPIO_SetBits(GPIOB,GPIO_Pin_5)

#define Led_On_2  	GPIO_ResetBits(GPIOD,GPIO_Pin_12)
#define Led_Off_2  	GPIO_SetBits(GPIOD,GPIO_Pin_12)

#define KEY_1				GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_13)
#define KEY_2				GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_3)
#define WAKEUP			GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)


GPIO_InitTypeDef GPIO_InitStructure;
USART_InitTypeDef USART_InitStructure;
static u16 fac_ms=0;//ms延时倍乘数
static u32 count=0;
static __IO uint32_t TimingDelay;



TaskHandle_t  data_pack_task;
TaskHandle_t  data_transfer_task;

TimerHandle_t  test1Timer_Handle;


FATFS fs;            // Work area (file system object) for logical drive
FIL fsrc, fdst;      // file objects
BYTE buffer[512]; // file copy buffer
UINT br, bw;         // File R/W count
BYTE r_buf[128]; // file copy buffer
FRESULT res;
FATFS tom_fs;//逻辑磁盘工作区.


#define RTOS_FS_Name "tom_min.txt"

uint32_t tom_size;

/*#pragma import(__use_no_semihosting) 

_sys_exit(int x) 
{ 
  x = x; 
}
 
struct __FILE 
{ 
  int handle; 
}; 

FILE __stdout;

int fputc(int ch,FILE *fp)
{
	while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)!=SET);
	//dataFromPC=USART_ReceiveData(USART1);//(uint8_t)
	USART_SendData(USART1,ch);
	return ch;	
}*/

int SendChar (int ch)  
{
#if 1
	USART_SendData(USART1,(unsigned char)ch);        
	while(USART_GetFlagStatus(USART1,USART_FLAG_TC) != SET);
#else
	
  while (!(USART1->SR & USART_FLAG_TXE)); // USART1
  USART1->DR = (ch & 0x1FF);
#endif
  return (ch);
}


int GetKey (void)  {
  while (!(USART1->SR & USART_FLAG_RXNE));
  return ((int)(USART1->DR & 0x1FF));
}/**/


void delay(uint32_t n)
{
	while(n--);
}

void GPIO_Configure(void)
{
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF_PP;//fatal mistake:Out
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);//PA9:TX
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA,&GPIO_InitStructure);//PA10:RX
}

void Beep_Init(void)
{
	GPIO_InitTypeDef GPIO_Init88;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
	
	GPIO_Init88.GPIO_Pin=GPIO_Pin_6;
	//配置成普通io时，直接选择推挽输出，而不是复用推挽输出，还不打开复用时钟，细节细节，我真的太菜了
	GPIO_Init88.GPIO_Mode=GPIO_Mode_Out_PP;//fatal mistake:Out
	GPIO_Init88.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOD,&GPIO_Init88);//PD6
}

void Led_Init(void)
{
	GPIO_InitTypeDef GPIO_Init88;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOD, ENABLE);
	
	GPIO_Init88.GPIO_Pin=GPIO_Pin_5;
	GPIO_Init88.GPIO_Mode=GPIO_Mode_Out_PP;//fatal mistake:Out
	GPIO_Init88.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_Init88);//PB5
	
	GPIO_Init88.GPIO_Pin=GPIO_Pin_12;
	GPIO_Init(GPIOD,&GPIO_Init88);//PD12
	
	Led_Off_1;
	Led_Off_2;
}


void Key_Init(void)
{
	GPIO_InitTypeDef GPIO_Init88;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD|RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_Init88.GPIO_Pin=GPIO_Pin_13;
	GPIO_Init88.GPIO_Mode=GPIO_Mode_IPU;//上拉输出，默认高电平
	GPIO_Init(GPIOC,&GPIO_Init88);//PC13
	
	GPIO_Init88.GPIO_Pin=GPIO_Pin_3;
	GPIO_Init(GPIOD,&GPIO_Init88);//PD3
	
	GPIO_Init88.GPIO_Pin=GPIO_Pin_0;
	GPIO_Init88.GPIO_Mode=GPIO_Mode_IPD;//下拉输出，默认低电平
	GPIO_Init(GPIOA,&GPIO_Init88);//PC13
}


void USART_Configure(void)
{
	USART_InitStructure.USART_BaudRate=115200;
	USART_InitStructure.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode=USART_Mode_Tx|USART_Mode_Rx;
	USART_InitStructure.USART_Parity=USART_Parity_No;
	USART_InitStructure.USART_StopBits=USART_StopBits_1;
	USART_InitStructure.USART_WordLength=USART_WordLength_8b;

	USART_Init(USART1,&USART_InitStructure);
	USART_Cmd(USART1, ENABLE);
	
	USART_Init(USART3,&USART_InitStructure);
	USART_Cmd(USART3, ENABLE);
	
	USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);
}


void NVIC_Config(void)
{
		NVIC_InitTypeDef NVIC_InitStructure;

		/* Enable the USART1 Interrupt */
		NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	
		NVIC_Init(&NVIC_InitStructure);
}


void TIMx_Configuration(void)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
		
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	
    TIM_TimeBaseStructure.TIM_Period = 1000;
    TIM_TimeBaseStructure.TIM_Prescaler= 71;
	
    TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;
//	  TIM_TimeBaseStructure.TIM_RepetitionCounter=0;
	
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
    TIM_ClearFlag(TIM2, TIM_FLAG_Update);
    TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);
    TIM_Cmd(TIM2, ENABLE);																		
}

void TIM2_IRQHandler(void)
{
//	printf("hello minyankun\r\n");
		count++;
}

void Delay_ms(__IO uint32_t nTime)
{ 
		TimingDelay = nTime;

		while(TimingDelay != 0);
}


void TimingDelay_Decrement(void)
{
	if(TimingDelay != 0)
	{ 
		TimingDelay--;
	}
}


void test1Timer_HandleCallback(TimerHandle_t xTimer)
{			
		printf("welcome to [%s]\r\n",__func__);
}


void data_write(void)
{
		FIL fsrc;     // file objects
		UINT br;         		// File R/W count
		FRESULT res;
//		char buf[] = "FreeRTOS";
		char buf[] = "Come on, Minyankun";
		char send_data[64] = {0};
		static int cnt = 1;
		
		res = f_open(&fsrc, RTOS_FS_Name, FA_OPEN_ALWAYS | FA_READ | FA_WRITE);
		if(res == FR_OK)
		{
				tom_size = fsrc.fsize;
				f_lseek(&fsrc,fsrc.fsize);
				sprintf(send_data,"%s_%d",buf,cnt);
				cnt++;
				res = f_write(&fsrc, send_data, strlen((char *)send_data), &br);
				if(res == FR_OK)
						printf("[%s] f_write ok, length: %d\r\n",__func__,br);
				f_sync(&fsrc);
		}
		else
		{
				printf("[%s] error number result: %d,open faild\r\n",__func__,res);
		}

		f_close(&fsrc);
}


void data_read(void)
{
		FIL fsrc;     		// file objects
		UINT br;         	// File R/W count
		FRESULT res;
		char r_buf[64] = {0};
		
		res = f_open(&fsrc, RTOS_FS_Name, FA_OPEN_ALWAYS | FA_READ | FA_WRITE);
		if(res == FR_OK)
		{
//				f_lseek(&fsrc,tom_size-13);
				f_lseek(&fsrc,tom_size);
				res = f_read(&fsrc, r_buf, sizeof(r_buf), &br);
				if(res == FR_OK)
						printf("[%s] f_read ok, r_buf: %s, length: %d\r\n",__func__,r_buf,br);
		}
		else
		{
				printf("[%s] error number result: %d,open faild\r\n",__func__,res);
		}

		f_close(&fsrc);
}

void Data_transfer(void *pvParameters)
{
		printf("111111\r\n");
		while(1)
		{
				printf("welcome to [%s]\r\n",__func__);
//				get_task_info(data_transfer_task);
//				get_task_info(data_pack_task);
				data_read();
				vTaskDelay(5000);
		}
}	


void Data_pack(void *pvParameters)
{
		printf("222222\r\n");
		while(1)
		{
				printf("welcome to [%s]\r\n",__func__);
//				get_task_info(data_transfer_task);
//				get_task_info(data_pack_task);
				data_write();
				vTaskDelay(5000);
		}
}


int main_FreeRTos(void)
{  
		BaseType_t tmp;
	
		printf("welcome to [%s]\r\n",__func__);
	
		tmp = xTaskCreate(Data_pack, "Data_pack", 500, NULL ,3, &data_pack_task);
		if(tmp != pdPASS)
		{
				printf("Data_pack task create failed\r\n");
		}
		else
		{
				printf("Data_pack task create successfully\r\n");
		}
		
		tmp = xTaskCreate(Data_transfer, "Data_transfer", 500, NULL ,2, &data_transfer_task);
		if(tmp != pdPASS)
		{
				printf("Data_transfer task create failed\r\n");
		}
		else
		{
				printf("Data_transfer task create successfully\r\n");
		}
		
    test1Timer_Handle = xTimerCreate((const char*)"test1Timer_Handle",
    							(TickType_t	)2000,
    							(UBaseType_t)pdTRUE,
    							(void*)1,
    							(TimerCallbackFunction_t)test1Timer_HandleCallback);  
    if (test1Timer_Handle == NULL)
    {
				printf("Failed to test1Timer_Handle\r\n");
    }
    else
    {
//				xTimerStart(test1Timer_Handle,mainDONT_BLOCK);
				printf("success to test1Timer_Handle\r\n");
    }
		
    vTaskStartScheduler();
		printf("22222222\r\n");
    return 0;
}


int main(void)
{   
//	uint8_t dataFromMCU[]="HP Computer惠普电脑\n";
	uint8_t dataFromMCU[]="hello tom\r\n";
	//uint8_t chineseChar[]="惠普电脑\n";
	uint8_t i,charSize=0;
	static uint8_t flag=1;
	RCC_ClocksTypeDef RCC_Clocks;
//	SD_Error Status = SD_OK;
  FILINFO finfo;
  DIR dirs;
	char path[50]={""}; 
	unsigned int a;
	

	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO|RCC_APB2Periph_USART1, ENABLE);
	GPIO_Configure();
	USART_Configure();
	printf("welcome to tom188888\r\n");
//	Beep_Init();
//	Led_Init();
//	Key_Init();
	RCC_GetClocksFreq(&RCC_Clocks);
	charSize=sizeof(dataFromMCU);	
//	TIMx_Configuration();
	
	//导致任务切换不了，卡死，坑我半天
//	if(SysTick_Config(SystemCoreClock / 1000))
//	{ 
//		/* 出错 */ 
//		while (1);
//	}
	
	printf("\r\nSYSCLK_Frequency = %d MHz\n",RCC_Clocks.SYSCLK_Frequency/1000000);
	printf("\r\nHCLK_Frequency = %d MHz\n",RCC_Clocks.HCLK_Frequency/1000000);
	printf("\r\nPCLK1_Frequency = %d MHz\n",RCC_Clocks.PCLK1_Frequency/1000000);
	printf("\r\nPCLK2_Frequency = %d MHz\n",RCC_Clocks.PCLK2_Frequency/1000000);
	
	printf("welcome to my computer1314\r\n");//实践证明这个RESET低电平真的会复位
	
//	Status = SD_Init();
//	if(Status != SD_OK)
//			printf("SD_Init() failed!\r\n");
//	else
//			printf("SD_Init() ok!\r\n");
	

	f_mount(0,&tom_fs); 		//挂载SD卡
	f_unlink(RTOS_FS_Name);
//	f_mkfs(0,0,512);
	printf("1111\r\n");
#if 1
	if (f_opendir(&dirs, path) == FR_OK) 
  {
  	printf("open dir success\n");
    while (f_readdir(&dirs, &finfo) == FR_OK)  
    {
				printf("read dir OK\n");
				if (finfo.fattrib & AM_ARC) 
				{
						if(!finfo.fname[0])	
								break;         
						printf("\r\n file name is:\n   %s\n",finfo.fname);
						res = f_open(&fsrc, finfo.fname, FA_OPEN_EXISTING | FA_READ);
						br=1;
						a=0;
						for (;;) 
						{
								for(a=0; a<512; a++) 
								buffer[a]=0; 
								res = f_read(&fsrc, buffer, sizeof(buffer), &br);
								printf("%s\n",buffer);	
								if (res || br == 0) 
										break;   // error or eof
					
						}
						f_close(&fsrc);                      
				}
    } 
    
  }
#else	
	
	
	memset(r_buf,0,sizeof(r_buf));
	memset(buffer,0,sizeof(buffer));
	memcpy((char *)buffer,"hello world",20);
	
	res = f_open(&fsrc, "tom188.txt", FA_OPEN_ALWAYS | FA_READ | FA_WRITE);
	if(res == FR_OK)
	{
			res = f_write(&fsrc, buffer, strlen((char *)buffer), &br);
			if(res == FR_OK)
				printf("f_write ok, length: %d\r\n",br);
			f_sync(&fsrc);
			f_lseek(&fsrc,0);
			res = f_read(&fsrc, r_buf, 28, &br);
			if(res == FR_OK)
				printf("f_read ok, r_buf: %s, length: %d\r\n",r_buf,br);
			f_close(&fsrc);
	}
	else
	{
			printf("error number result: %d,open faild\r\n",res);
			f_close(&fsrc);
	}
	
#endif
	
//	while(1);
	while (1)
	{
#if 0
//		Delay_ms(500);

		for(i=0;i<charSize-1;i++)
		{
				printf("%c",dataFromMCU[i]);
		}
//		Beep_On;
		Led_On_1;
		Led_Off_2;

		Delay_ms(500);
//		Beep_Off;
		Led_Off_1;
		Led_On_2;
		Delay_ms(500);
#elif 0
		//简单的按键消抖，哈哈，明天弄个按键中断玩玩
		if(KEY_1 == 0 && flag)
		{
			Delay_ms(10);
			flag = 0;
			Led_On_1;
			printf("KEY1 hahaha\r\n");
		}
		else if(KEY_2 == 0 && flag)
		{
			Delay_ms(100);
			flag = 0;
			Led_On_2;
			printf("KEY2 hahaha\r\n");
		}
		else if(WAKEUP == 1 && flag)
		{
			Delay_ms(100);
			flag = 0;
			Led_On_1;
			Led_On_2;
			printf("WAKEUP hahaha\r\n");
		}
		else if(!flag && KEY_1 == 1 && KEY_2 == 1 && WAKEUP == 0)
		{
			Led_Off_1;
			Led_Off_2;
			Delay_ms(50);//这里加个延时，防止松开按键时的干扰
			flag = 1;
		}
		
#else		
		
		main_FreeRTos();
		printf("33333333333\r\n");
#endif			
	}
}


#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}

#endif



