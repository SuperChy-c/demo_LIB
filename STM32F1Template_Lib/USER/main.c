#include "sys.h"
#include "delay.h"
#include "usart.h"
u8 Run_flag = 0;
void TIM3_Int_Init(u16 arr,u16 psc)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //ʱ��ʹ��

	TIM_TimeBaseStructure.TIM_Period = arr; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	 ������5000Ϊ500ms
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ  10Khz�ļ���Ƶ��  
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
 
	TIM_ITConfig(  //ʹ�ܻ���ʧ��ָ����TIM�ж�
		TIM3, //TIM2
		TIM_IT_Update ,
		ENABLE  //ʹ��
		);
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //��ռ���ȼ�0��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //�����ȼ�3��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQͨ����ʹ��
	NVIC_Init(&NVIC_InitStructure);  //����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ���

	TIM_Cmd(TIM3, ENABLE);  //ʹ��TIMx����
							 
}

void TIM3_IRQHandler(void)   //TIM3�ж�
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) //���ָ����TIM�жϷ������:TIM �ж�Դ 
		{
				TIM_ClearITPendingBit(TIM3, TIM_IT_Update  );  //���TIMx���жϴ�����λ:TIM �ж�Դ 
				Run_flag = 1;
		}
}

void dat_init(void)
{
 GPIO_InitTypeDef  GPIO_InitStructure;
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	 //ʹ��PA�˿�ʱ��
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;				 //PA.8 �˿�����
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
 GPIO_Init(GPIOA, &GPIO_InitStructure);					 //�����趨������ʼ��GPIOA.8
 GPIO_SetBits(GPIOA,GPIO_Pin_8);						 //PA.8 �����
}
void busy_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);//ʹ��PORTCʱ��
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);//�ر�jtag��ʹ��SWD��������SWDģʽ����
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_5;//PC5
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //���ó���������
 	GPIO_Init(GPIOC, &GPIO_InitStructure);//��ʼ��GPIOC5
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
	GPIO_ResetBits(GPIOA,GPIO_Pin_8);	//PA8��0
	delay_ms(5);
	for (i = 0; i < 8; i++)
	{
		if ((Temp_high & 0x0001) == 1)
		{
			GPIO_SetBits(GPIOA,GPIO_Pin_8);
			delay_us(600);
			GPIO_ResetBits(GPIOA,GPIO_Pin_8);	//PA8��0
			delay_us(200);
		}
		else
		{
			GPIO_SetBits(GPIOA,GPIO_Pin_8);	//PA8��1
			delay_us(200);
			GPIO_ResetBits(GPIOA,GPIO_Pin_8);	//PA8��0
			delay_us(600);
		}
		Temp_high = Temp_high >> 1;
	}
	GPIO_SetBits(GPIOA,GPIO_Pin_8);	//PA8��1
	delay_ms(2);


	GPIO_ResetBits(GPIOA,GPIO_Pin_8);	//PA8��0
	delay_ms(5);
	for (i = 0; i < 8; i++)
	{
		if ((Temp_low & 0x0001) == 1)
		{
			GPIO_SetBits(GPIOA,GPIO_Pin_8);	//PA8��1
			delay_us(600);
			GPIO_ResetBits(GPIOA,GPIO_Pin_8);	//PA8��0
			delay_us(200);
		}
		else
		{
			GPIO_SetBits(GPIOA,GPIO_Pin_8);	//PA8��1
			delay_us(200);
			GPIO_ResetBits(GPIOA,GPIO_Pin_8);	//PA8��0
			delay_us(600);
		}
		Temp_low = Temp_low >> 1;
	}
	GPIO_SetBits(GPIOA,GPIO_Pin_8);//PA8��1
	delay_ms(100);
	TIM_Cmd(TIM3, ENABLE); 
}
 int main(void)
 {		
	delay_init();	    	 //��ʱ������ʼ��	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	uart_init(9600);	 //���ڳ�ʼ��Ϊ9600
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
