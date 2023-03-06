#include "../../inc/MarlinConfig.h"
#include "sys.h"

void MY_NVIC_SetVectorTable(u32 NVIC_VectTab,u32 Offset)	 
{ 	   	  
	SCB->VTOR=NVIC_VectTab|(Offset&(u32)0xFFFFFE00);
}

void MY_NVIC_PriorityGroupConfig(u8 NVIC_Group)	 
{ 
	u32 temp,temp1;	  
	temp1=(~NVIC_Group)&0x07;
	temp1<<=8;
	temp=SCB->AIRCR;
	temp&=0X0000F8FF;
	temp|=0X05FA0000;
	temp|=temp1;	   
	SCB->AIRCR=temp;	    	  				   
}

void MY_NVIC_Init(u8 NVIC_PreemptionPriority,u8 NVIC_SubPriority,u8 NVIC_Channel,u8 NVIC_Group)	 
{ 
	u32 temp;	  
	MY_NVIC_PriorityGroupConfig(NVIC_Group);
	temp=NVIC_PreemptionPriority<<(4-NVIC_Group);	  
	temp|=NVIC_SubPriority&(0x0f>>NVIC_Group);
	temp&=0xf;
	NVIC->ISER[NVIC_Channel/32]|=1<<NVIC_Channel%32;
	NVIC->IP[NVIC_Channel]|=temp<<4;    	  				   
} 

void Ex_NVIC_Config(u8 GPIOx,u8 BITx,u8 TRIM) 
{ 
	u8 EXTOFFSET=(BITx%4)*4;  
	RCC->APB2ENR|=1<<14;
	SYSCFG->EXTICR[BITx/4]&=~(0x000F<<EXTOFFSET);
	SYSCFG->EXTICR[BITx/4]|=GPIOx<<EXTOFFSET;

	EXTI->IMR|=1<<BITx;
	if(TRIM&0x01)EXTI->FTSR|=1<<BITx;
	if(TRIM&0x02)EXTI->RTSR|=1<<BITx;
} 	

void GPIO_AF_Set(GPIO_TypeDef* GPIOx,u8 BITx,u8 AFx)
{  
	GPIOx->AFR[BITx>>3]&=~(0X0F<<((BITx&0X07)*4));
	GPIOx->AFR[BITx>>3]|=(u32)AFx<<((BITx&0X07)*4);
}   

void GPIO_Set(GPIO_TypeDef* GPIOx,u32 BITx,u32 MODE,u32 OTYPE,u32 OSPEED,u32 PUPD)
{  
	u32 pinpos=0,pos=0,curpin=0;
	for(pinpos=0;pinpos<16;pinpos++)
	{
		pos=1<<pinpos;
		curpin=BITx&pos;
		if(curpin==pos)
		{
			GPIOx->MODER&=~(3<<(pinpos*2));
			GPIOx->MODER|=MODE<<(pinpos*2);
			if((MODE==0X01)||(MODE==0X02))
			{  
				GPIOx->OSPEEDR&=~(3<<(pinpos*2));
				GPIOx->OSPEEDR|=(OSPEED<<(pinpos*2)); 
				GPIOx->OTYPER&=~(1<<pinpos);
				GPIOx->OTYPER|=OTYPE<<pinpos;
			}  
			GPIOx->PUPDR&=~(3<<(pinpos*2));
			GPIOx->PUPDR|=PUPD<<(pinpos*2);
		}
	}
} 

void WFI_SET(void)
{
	__ASM volatile("wfi");		  
}

void INTX_DISABLE(void)
{
	__ASM volatile("cpsid i");
}

void INTX_ENABLE(void)
{
	__ASM volatile("cpsie i");		  
}

void Sys_Standby(void)
{ 
	SCB->SCR|=1<<2;  
	RCC->APB1ENR|=1<<28;
	PWR->CSR|=1<<8;
	PWR->CR|=1<<2;
	PWR->CR|=1<<1;
	WFI_SET();	 
}	     

void Sys_Soft_Reset(void)
{   
	SCB->AIRCR =0X05FA0000|(u32)0x04;	  
} 		 

u8 Sys_Clock_Set(u32 plln,u32 pllm,u32 pllp,u32 pllq)
{ 
	u16 retry=0;
	u8 status=0;
	RCC->CR|=1<<16;
	while(((RCC->CR&(1<<17))==0)&&(retry<0X1FFF))retry++;
	if(retry==0X1FFF)status=1;
	else   
	{
		RCC->APB1ENR|=1<<28;
		PWR->CR|=3<<14;
		RCC->CFGR|=(0<<4)|(5<<10)|(4<<13); 
		RCC->CR&=~(1<<24);
		RCC->PLLCFGR=pllm|(plln<<6)|(((pllp>>1)-1)<<16)|(pllq<<24)|(1<<22);
		RCC->CR|=1<<24;
		while((RCC->CR&(1<<25))==0);
		FLASH->ACR|=1<<8;
		FLASH->ACR|=1<<9;
		FLASH->ACR|=1<<10;
		FLASH->ACR|=5<<0;
		RCC->CFGR&=~(3<<0);
		RCC->CFGR|=2<<0; 
		while((RCC->CFGR&(3<<2))!=(2<<2));
	}
	return status;
}  

void Stm32_Clock_Init(u32 plln,u32 pllm,u32 pllp,u32 pllq)
{  
	RCC->CR|=0x00000001;
	RCC->CFGR=0x00000000;
	RCC->CR&=0xFEF6FFFF;
	RCC->PLLCFGR=0x24003010;
	RCC->CR&=~(1<<18);
	RCC->CIR=0x00000000;
	Sys_Clock_Set(plln,pllm,pllp,pllq);		  
#ifdef  VECT_TAB_RAM
	MY_NVIC_SetVectorTable(1<<29,0x0);
#else   
	MY_NVIC_SetVectorTable(0,0x0);
#endif 
}
