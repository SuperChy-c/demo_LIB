#include "sys.h"
#include "delay.h"
#include "usart.h"
u8 Run_flag = 0;
void TIM3_Int_Init(u16 arr,u16 psc)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //时钟使能

	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	 计数到5000为500ms
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值  10Khz的计数频率  
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
 
	TIM_ITConfig(  //使能或者失能指定的TIM中断
		TIM3, //TIM2
		TIM_IT_Update ,
		ENABLE  //使能
		);
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //从优先级3级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器

	TIM_Cmd(TIM3, ENABLE);  //使能TIMx外设
							 
}

void TIM3_IRQHandler(void)   //TIM3中断
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) //检查指定的TIM中断发生与否:TIM 中断源 
		{
				TIM_ClearITPendingBit(TIM3, TIM_IT_Update  );  //清除TIMx的中断待处理位:TIM 中断源 
				Run_flag = 1;
		}
}

void dat_init(void)
{
 GPIO_InitTypeDef  GPIO_InitStructure;
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	 //使能PA端口时钟
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;				 //PA.8 端口配置
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
 GPIO_Init(GPIOA, &GPIO_InitStructure);					 //根据设定参数初始化GPIOA.8
 GPIO_SetBits(GPIOA,GPIO_Pin_8);						 //PA.8 输出高
}
void busy_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);//使能PORTC时钟
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);//关闭jtag，使能SWD，可以用SWD模式调试
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_5;//PC5
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
 	GPIO_Init(GPIOC, &GPIO_InitStructure);//初始化GPIOC5
}
void Oneline_send(unsigned int dat)
{
	unsigned char i;
	unsigned int Temp_high;
	unsigned int Temp_low;

	Temp_high = dat & 0xff00;
	Temp_high = Temp_high >> 8;
	Temp_low = dat & 0x00ff;

	TIM_Cmd(TIM3,DISABLE); 
	GPIO_ResetBits(GPIOA,GPIO_Pin_8);	//PA8置0
	delay_ms(5);
	for (i = 0; i < 8; i++)
	{
		if ((Temp_high & 0x0001) == 1)
		{
			GPIO_SetBits(GPIOA,GPIO_Pin_8);
			delay_us(600);
			GPIO_ResetBits(GPIOA,GPIO_Pin_8);	//PA8置0
			delay_us(200);
		}
		else
		{
			GPIO_SetBits(GPIOA,GPIO_Pin_8);	//PA8置1
			delay_us(200);
			GPIO_ResetBits(GPIOA,GPIO_Pin_8);	//PA8置0
			delay_us(600);
		}
		Temp_high = Temp_high >> 1;
	}
	GPIO_SetBits(GPIOA,GPIO_Pin_8);	//PA8置1
	delay_ms(2);


	GPIO_ResetBits(GPIOA,GPIO_Pin_8);	//PA8置0
	delay_ms(5);
	for (i = 0; i < 8; i++)
	{
		if ((Temp_low & 0x0001) == 1)
		{
			GPIO_SetBits(GPIOA,GPIO_Pin_8);	//PA8置1
			delay_us(600);
			GPIO_ResetBits(GPIOA,GPIO_Pin_8);	//PA8置0
			delay_us(200);
		}
		else
		{
			GPIO_SetBits(GPIOA,GPIO_Pin_8);	//PA8置1
			delay_us(200);
			GPIO_ResetBits(GPIOA,GPIO_Pin_8);	//PA8置0
			delay_us(600);
		}
		Temp_low = Temp_low >> 1;
	}
	GPIO_SetBits(GPIOA,GPIO_Pin_8);//PA8置1
	delay_ms(100);
	TIM_Cmd(TIM3, ENABLE); 
}
 int main(void)
 {		
	delay_init();	    	 //延时函数初始化	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	uart_init(9600);	 //串口初始化为9600
	TIM3_Int_Init(9999,7199); 
	dat_init();
	busy_init();
	 while(1){
				if(Run_flag)
				{
					Oneline_send(0x01);//send 0x01 address voice in per Second 
				}
	}
 }
