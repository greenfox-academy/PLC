#ifndef PWM_H_INCLUDED
#define PWM_H_INCLUDED

#include "stm32l4xx_hal.h"

TIM_HandleTypeDef pwm_handle;
TIM_OC_InitTypeDef pwm_oc_init;


void pwm_init();
void pwm_set_duty(float duty);


/* ########## Functions for Digital pins ########## */

/* Function name: GPIO_Set_PIN
 * Function purpose: Set PIN to output or input
 * Function input - GPIO_Pin_t *GPIO_Pin: This is the pin to be set
 * Function input - uint32_t pode: This is the mode it can be these:
 * 		GPIO_MODE_INPUT:		Input Floating Mode
 * 		GPIO_MODE_OUTPUT_PP: 	Output Push Pull Mode
 * 		GPIO_MODE_OUTPUT_OD: 	Output Open Drain Mode
 * 		GPIO_MODE_AF_PP:		Alternate Function Push Pull Mode
 *		GPIO_MODE_AF_OD:		Alternate Function Open Drain Mode
 * Function input - uint32_t pull can be:
 * 		GPIO_PULLDOWN
 * 		GPIO_PULLUP
 * 		GPIO_NOPULL
 *
 */

void pwm_gpio_init_digital_pin(uint8_t pin_index, uint32_t mode, uint32_t pull);


#endif // PWM_H_INCLUDED