#include "../../inc/MarlinConfig.h"
#if ENABLED(BOARD_CONFIGURE)

#include "board_configure.h"

Board_Configure board_configure;

u16 Board_Configure:: Get_Adc(u8 ch)   
{
	u16 time=5000;
	 
	ADC1->SQR3&=0XFFFFFFE0;
	ADC1->SQR3|=ch;		  			    
	ADC1->CR2|=1<<30;      
	while((!(ADC1->SR&1<<1))&&(time--));	   
	return ADC1->DR;		
}

u16 Board_Configure:: Get_Adc_Average(u8 ch,u8 times)
{
   	u32 temp_val=0;
	u8 t;
	for(t=0;t<times;t++)
	{
		temp_val+=Get_Adc(ch);
		_delay_ms(5);
	}
	return temp_val/times;
}

void Board_Configure:: init(void)
{
   RCC->APB2ENR|=1<<8;    
   RCC->AHB1ENR|=1<<0;    	
    GPIO_Set(GPIOA,PIN0,GPIO_MODE_AIN,0,0,GPIO_PUPD_PU);	
	GPIO_Set(GPIOA,PIN1,GPIO_MODE_AIN,0,0,GPIO_PUPD_PU);	

	RCC->APB2RSTR|=1<<8;   	
	RCC->APB2RSTR&=~(1<<8);	
	ADC->CCR=3<<16;			
 	
	ADC1->CR1=0;   			
	ADC1->CR2=0;   			
	ADC1->CR1|=0<<24;      
	ADC1->CR1|=0<<8;    	
	
	ADC1->CR2&=~(1<<1);    	
 	ADC1->CR2&=~(1<<11);   	
	ADC1->CR2|=0<<28;    	
	
	ADC1->SQR1&=~(0XF<<20);
	ADC1->SQR1|=0<<20;     				   

	ADC1->SMPR2&=~(7<<(3*0));  
 	ADC1->SMPR2|=7<<(3*0); 	
	ADC1->SMPR2&=~(7<<(3*1));  
 	ADC1->SMPR2|=7<<(3*1); 	
 	ADC1->CR2|=1<<0;	   	 
    
	board_configure.adc1=board_configure.Get_Adc_Average(0,10);
    board_configure.adc2=board_configure.Get_Adc_Average(1,10);
    
    if((board_configure.adc1>496)&&(board_configure.adc1<869)) //100K/20K 0.55V => 0.4~0.7V
	{
		strcpy(board_configure.board_version,MAIN_BOARD_V8111_V0_5); 
	}
	else if((board_configure.adc1>=0)&&(board_configure.adc1<=100))
	{
		strcpy(board_configure.board_version,MAIN_BOARD_V8111_V0_2); 
	}
	else if((board_configure.adc1>1900)&&(board_configure.adc1<2200))
	{
		strcpy(board_configure.board_version,MAIN_BOARD_V8111_V0_3); 
	}

    if((board_configure.adc2>=0)&&(board_configure.adc2<=100))
	{
		strcpy(board_configure.board_chip,GD32F407VGT6_CHIP); 
	}
	else if((board_configure.adc2>=1900)&&(board_configure.adc2<=2200))
	{
		strcpy(board_configure.board_chip,STM32F407VGT6_CHIP); 
	}
}
#endif
