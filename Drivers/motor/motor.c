#include "motor/motor.h"

static MotorStateTypeDef s_current_state = MOTOR_COAST;

static void motor_apply_pins(GPIO_PinState a_state, GPIO_PinState b_state)
{
	HAL_GPIO_WritePin(M_A_GPIO_Port, M_A_Pin, a_state);
	HAL_GPIO_WritePin(M_B_GPIO_Port, M_B_Pin, b_state);
}

void Motor_Init(void)
{
	motor_apply_pins(GPIO_PIN_RESET, GPIO_PIN_RESET);
	s_current_state = MOTOR_COAST;
}

void Motor_SetState(MotorStateTypeDef state)
{
	switch (state)
	{
		case MOTOR_COAST:
			/* Both inputs low -> coast/fast decay (Table 1). */
			motor_apply_pins(GPIO_PIN_RESET, GPIO_PIN_RESET);
			break;
		case MOTOR_FORWARD:
			motor_apply_pins(GPIO_PIN_SET, GPIO_PIN_RESET);
			break;
		case MOTOR_BACKWARD:
			motor_apply_pins(GPIO_PIN_RESET, GPIO_PIN_SET);
			break;
		case MOTOR_BRAKE:
			/* Both inputs high -> brake/slow decay. */
			motor_apply_pins(GPIO_PIN_SET, GPIO_PIN_SET);
			break;
		default:
			motor_apply_pins(GPIO_PIN_RESET, GPIO_PIN_RESET);
			break;
	}

	s_current_state = (state < MOTOR_COAST || state > MOTOR_BRAKE) ? MOTOR_COAST : state;
}

MotorStateTypeDef Motor_GetState(void)
{
	return s_current_state;
}
