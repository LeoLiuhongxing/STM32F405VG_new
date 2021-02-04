#include "PID.h"

_pid pid;

void pid_value_init(void)
{

	pid.SetTarget = 0;
	pid.ActualTarget = 0;
	pid.err = 0;
	pid.err_last = 0;
	pid.err_last_next = 0;
 
	pid.Kp = 0.1;
	pid.Ki = 0.15;
	pid.Kd = 0.1;
 
}

float PID_realize(float target)
{
	float incrementSpeed;
	pid.SetTarget = target;
	pid.err = pid.SetTarget - pid.ActualTarget;
 
	incrementSpeed = pid.Kp * (pid.err -pid.err_last ) + pid.Ki*pid.err + pid.Kd*(pid.err -2* pid.err_last + pid.err_last_next);
 
	pid.ActualTarget += incrementSpeed;
	pid.err_last = pid.err;
	pid.err_last_next = pid.err_last;
	return pid.ActualTarget;
}
