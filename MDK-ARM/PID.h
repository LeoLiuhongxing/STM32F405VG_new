#ifndef __PID_H
#define __PID_H

#ifdef __cplusplus
 extern "C" {
#endif
     
typedef struct{
	float SetTarget;       //????
	float ActualTarget;    //???
	float err;            //k,?????
	float err_last;       //k-1,??????
	float err_last_next;  //k-2
	float Kp, Ki, Kd;     //p,i,d??
    float MIN_ERROR;
 
}_pid;

extern _pid pid;

void pid_value_init(void);
float PID_realize(float target);     
     
     

#ifdef __cplusplus
}
#endif

#endif /* __STM32F4xx_HAL_FLASH_H */
