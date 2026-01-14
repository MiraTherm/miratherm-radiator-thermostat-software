#ifndef DRIVERS_MOTOR_MOTOR_H
#define DRIVERS_MOTOR_MOTOR_H

#include "main.h"

typedef enum {
  MOTOR_COAST = 0,
  MOTOR_FORWARD,
  MOTOR_BACKWARD,
  MOTOR_BRAKE
} MotorStateTypeDef;

void Motor_Init(void);
void Motor_SetState(MotorStateTypeDef state);
MotorStateTypeDef Motor_GetState(void);

#endif /* DRIVERS_MOTOR_MOTOR_H */
