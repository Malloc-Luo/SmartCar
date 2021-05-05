#include "timer.h"

const uint16_t ENCODER_TIM_PERIOD = 65535;
SpeedStruct_t MotorSpeed = {0, 0, 0, 0};
SpeedStruct_t SetMotorSpeed = {0, 0, 0, 0};
/**********************FUNCTION***********************
  * @brief: TM2������ģʽ
  * @note: �ο�����
  *****************************************************/
static void TM2_encoder_init() {
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;  
    TIM_ICInitTypeDef TIM_ICInitStructure;  
    GPIO_InitTypeDef GPIO_InitStructure;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    
    GPIO_PinRemapConfig(GPIO_PartialRemap1_TIM2, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;	
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
    TIM_TimeBaseStructure.TIM_Prescaler = 0x00;
    TIM_TimeBaseStructure.TIM_Period = 65535;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
    TIM_EncoderInterfaceConfig(TIM2, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
    TIM_ICStructInit(&TIM_ICInitStructure);
    TIM_ICInitStructure.TIM_ICFilter = 10;
    TIM_ICInit(TIM2, &TIM_ICInitStructure);
    TIM_ClearFlag(TIM2, TIM_FLAG_Update);
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    
    TIM_SetCounter(TIM2,0);
    TIM_Cmd(TIM2, ENABLE); 
}

/**********************FUNCTION***********************
  * @brief: TM3������ģʽ
  * @note: �ο�����
  *****************************************************/
static void TM3_encoder_init() {
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;  
    TIM_ICInitTypeDef TIM_ICInitStructure;  
    GPIO_InitTypeDef GPIO_InitStructure;
    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
    TIM_TimeBaseStructure.TIM_Prescaler = 0x0;
    TIM_TimeBaseStructure.TIM_Period = 65535;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
    TIM_EncoderInterfaceConfig(TIM3, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
    TIM_ICStructInit(&TIM_ICInitStructure);
    TIM_ICInitStructure.TIM_ICFilter = 10;
    TIM_ICInit(TIM3, &TIM_ICInitStructure);
    TIM_ClearFlag(TIM3, TIM_FLAG_Update);
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
    
    TIM_SetCounter(TIM3,0);
    TIM_Cmd(TIM3, ENABLE); 
}

/**********************FUNCTION***********************
  * @brief: TM4������ģʽ
  * @note: �ο�����
  *****************************************************/
static void TM4_encoder_init() {
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;  
    TIM_ICInitTypeDef TIM_ICInitStructure;  
    GPIO_InitTypeDef GPIO_InitStructure;
    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
    TIM_TimeBaseStructure.TIM_Prescaler = 0x0;
    TIM_TimeBaseStructure.TIM_Period = ENCODER_TIM_PERIOD;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
    TIM_EncoderInterfaceConfig(TIM4, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
    TIM_ICStructInit(&TIM_ICInitStructure);
    TIM_ICInitStructure.TIM_ICFilter = 10;
    TIM_ICInit(TIM4, &TIM_ICInitStructure);
    TIM_ClearFlag(TIM4, TIM_FLAG_Update);
    TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
    
    TIM_SetCounter(TIM4,0);
    TIM_Cmd(TIM4, ENABLE); 
}

/**********************FUNCTION***********************
  * @brief: TM4������ģʽ
  * @note: �ο�����
  *****************************************************/
static void TM5_encoder_init() {
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;  
    TIM_ICInitTypeDef TIM_ICInitStructure;  
    GPIO_InitTypeDef GPIO_InitStructure;
    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);	

    TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
    TIM_TimeBaseStructure.TIM_Prescaler = 0x0;
    TIM_TimeBaseStructure.TIM_Period = ENCODER_TIM_PERIOD;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);
    TIM_EncoderInterfaceConfig(TIM5, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
    TIM_ICStructInit(&TIM_ICInitStructure);
    TIM_ICInitStructure.TIM_ICFilter = 10;
    TIM_ICInit(TIM5, &TIM_ICInitStructure);
    TIM_ClearFlag(TIM5, TIM_FLAG_Update);
    TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE);
    
    TIM_SetCounter(TIM5,0);
    TIM_Cmd(TIM5, ENABLE); 
}

void encoder_init() {
    TM2_encoder_init();
    TM3_encoder_init();
    TM4_encoder_init();
    TM5_encoder_init();
}

/**********************FUNCTION***********************
  * @brief: ��ȡ�ĸ���������ֵ��������������������
  * @param: Motor_t ö�٣������ĸ����
  * @return: int32_t ����������ֵ������������
  * @note: ���Բο������ϣ���STM32���Ĳο��ֲ�_V10.pdf��p273-275
  *****************************************************/
int32_t get_encoder(Motor_t motor) {
    int32_t cntVal = 0;
    switch (motor) {
    case MotorA: {
        cntVal = (int32_t)TIM2->CNT;
        TIM2->CNT = 0;
        break;
    }
    case MotorB: {
        cntVal = (int32_t)TIM3->CNT;
        TIM3->CNT = 0;
        break;
    }
    case MotorC: {
        cntVal = (int32_t)TIM4->CNT;
        TIM4->CNT = 0;
        break;
    }
    case MotorD: {
        cntVal = (int32_t)TIM5->CNT;
        TIM5->CNT = 0;
        break;
    }
    default:
        break;
    }
    return cntVal;
}

/* �жϷ����� */
void TIM2_IRQHandler(void) {
	if (TIM2->SR & 0x0001) {}
	TIM2->SR &= ~(1 << 0);
}

void TIM3_IRQHandler(void) {
	if (TIM3->SR & 0X0001) {}
	TIM3->SR &= ~(1 << 0);
}

void TIM4_IRQHandler(void) {
	if (TIM4->SR & 0X0001) {}
	TIM4->SR &= ~(1 << 0);
}

void TIM5_IRQHandler(void) {
	if (TIM5->SR & 0X0001) {}
	TIM5->SR &= ~(1 << 0);
}
