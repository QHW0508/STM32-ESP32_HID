#ifndef __SERVO_H
#define __SERVO_H

void Servo_Init(void);
void Servo_SetAnglePA6(float Angle);//直接输出的是角度
void Servo_SetAnglePA7(float Angle);
void Servo_SetAnglePB0(float Angle);
void Servo_SetAnglePB1(float Angle);
void Servo_SetAnglePB6(float Angle);
void Servo_SetAngle(uint8_t a);

#endif
