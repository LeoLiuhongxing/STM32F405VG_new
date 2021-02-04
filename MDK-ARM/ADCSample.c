#include "ADCSample.h"
#include "types.h"
#include "delay.h"
#include "cmsis_os2.h"
//#include "stm32f4xx_hal_adc.h"

//���ADCֵ
//ch: ͨ��ֵ 0~16��ȡֵ��ΧΪ��ADC_CHANNEL_0~ADC_CHANNEL_16
//����ֵ:ת�����
u16 Get_Adc(u32 ch)   
{
    ADC_ChannelConfTypeDef ADC1_ChanConf;
    
    ADC1_ChanConf.Channel=ch;                                   //ͨ��
    ADC1_ChanConf.Rank=1;                                       //��1�����У�����1
    ADC1_ChanConf.SamplingTime=ADC_SAMPLETIME_480CYCLES;        //����ʱ��
    ADC1_ChanConf.Offset=0;                 
    HAL_ADC_ConfigChannel(&hadc1,&ADC1_ChanConf);        //ͨ������
	
    HAL_ADC_Start(&hadc1);                               //����ADC
	
    //HAL_ADC_PollForConversion(&hadc1,10);                //��ѯת��
    HAL_ADC_PollForConversion(&hadc1,2);                //��ѯת��
 
	return (u16)HAL_ADC_GetValue(&hadc1);	        //�������һ��ADC1�������ת�����
}
//��ȡָ��ͨ����ת��ֵ��ȡtimes��,Ȼ��ƽ�� 
//times:��ȡ����
//����ֵ:ͨ��ch��times��ת�����ƽ��ֵ
u16 Get_Adc_Average(u32 ch,u8 times)
{
	u32 temp_val=0;
	u8 t;
	for(t=0;t<times;t++)
	{
		temp_val+=Get_Adc(ch);
		osDelay(1);
	}
	return temp_val/times;
}
