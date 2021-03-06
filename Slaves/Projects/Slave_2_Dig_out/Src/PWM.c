#include "PWM.h"

typedef struct {
	TIM_TypeDef *tim;
	HAL_TIM_ActiveChannel channel;
	uint32_t tim_ch;
}pwm_tim_ch;

typedef struct {
	GPIO_TypeDef *port;
	uint16_t pin;
	uint8_t Alternate;
} gpio_pins_t_pwm;


TIM_HandleTypeDef pwm_handle[3];
TIM_OC_InitTypeDef pwm_oc_init[3];

/* Private variables ---------------------------------------------------------*/

const pwm_tim_ch stm32l476rg_pwm_set[] = {
		{TIM3, HAL_TIM_ACTIVE_CHANNEL_2, TIM_CHANNEL_2},
		{TIM4, HAL_TIM_ACTIVE_CHANNEL_1, TIM_CHANNEL_1},
		{TIM17, HAL_TIM_ACTIVE_CHANNEL_1, TIM_CHANNEL_1},
};

const gpio_pins_t_pwm stm32l476rg_digital_pins_pwm[] = {
	{GPIOA, GPIO_PIN_3, 0},  				   	//PIN: D0
	{GPIOA, GPIO_PIN_2, 0},						//PIN: D1
	{GPIOA, GPIO_PIN_10, 0},					//PIN: D2
	{GPIOB, GPIO_PIN_3, 0},   					//PIN: D3
	{GPIOB, GPIO_PIN_5, 0},     				//PIN: D4
	{GPIOB, GPIO_PIN_4, 0},     				//PIN: D5
	{GPIOB, GPIO_PIN_10, 0},    				//PIN: D6
	{GPIOA, GPIO_PIN_8, 0},     				//PIN: D7
	{GPIOA, GPIO_PIN_9, 0},     				//PIN: D8
	{GPIOC, GPIO_PIN_7, GPIO_AF2_TIM3},     	//PIN: D9		PWM 	TIM3_CH2
	{GPIOB, GPIO_PIN_6, GPIO_AF2_TIM4},     	//PIN: D10		PWM		TIM4_CH1
	{GPIOA, GPIO_PIN_7, GPIO_AF14_TIM17},     	//PIN: D11		PWM		TIM17_CH1
	{GPIOA, GPIO_PIN_6, 0},     				//PIN: D12
	{GPIOA, GPIO_PIN_5, 0},     				//PIN: D13
	{GPIOB, GPIO_PIN_9, 0},     				//PIN: D14
	{GPIOB, GPIO_PIN_8, 0},     				//PIN: D15
	{GPIOA, GPIO_PIN_0, 0},     				//PIN: A0 - As a Digital PIN
	{GPIOA, GPIO_PIN_1, 0},     				//PIN: A1 - As a Digital PIN
	{GPIOA, GPIO_PIN_4, 0},     				//PIN: A2 - As a Digital PIN
	{GPIOB, GPIO_PIN_0, 0},     				//PIN: A3 - As a Digital PIN
	{GPIOC, GPIO_PIN_1, 0},     				//PIN: A4 - As a Digital PIN
	{GPIOC, GPIO_PIN_0, 0},     				//PIN: A5 - As a Digital PIN
};

/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

void gpio_clk_enable_pwm(GPIO_TypeDef *port)
{
	if (port == GPIOA)
		__HAL_RCC_GPIOA_CLK_ENABLE();
	else if (port == GPIOB)
		__HAL_RCC_GPIOB_CLK_ENABLE();
	else if (port == GPIOC)
		__HAL_RCC_TIM3_CLK_ENABLE();
	else if (port == GPIOD)
		__HAL_RCC_GPIOD_CLK_ENABLE();
	else if (port == GPIOE)
		__HAL_RCC_GPIOE_CLK_ENABLE();
	else if (port == GPIOF)
		__HAL_RCC_GPIOF_CLK_ENABLE();
	else if (port == GPIOG)
		__HAL_RCC_GPIOG_CLK_ENABLE();
}

void pwm_clk_enable(uint8_t Alternate)
{
	if (Alternate == GPIO_AF14_TIM17)
		__HAL_RCC_TIM17_CLK_ENABLE();
	else if (Alternate == GPIO_AF2_TIM4)
		__HAL_RCC_TIM4_CLK_ENABLE();
	else if (Alternate == GPIO_AF2_TIM3)
		__HAL_RCC_TIM3_CLK_ENABLE();
	/*else if (port == GPIOD)
		__HAL_RCC_GPIOD_CLK_ENABLE();
	else if (port == GPIOE)
		__HAL_RCC_GPIOE_CLK_ENABLE();
	else if (port == GPIOF)
		__HAL_RCC_GPIOF_CLK_ENABLE();
	else if (port == GPIOG)
		__HAL_RCC_GPIOG_CLK_ENABLE();*/
}


/* ########## Functions for Digital pins PWM ########## */

void pwm_pin_init(uint8_t pin_index, uint32_t mode, uint32_t pull)
{
	GPIO_InitTypeDef gpio_init_structure_pwm;

	// Set the clock
	gpio_clk_enable_pwm(stm32l476rg_digital_pins_pwm[pin_index].port);

	// Set the init structure
	gpio_init_structure_pwm.Pin = stm32l476rg_digital_pins_pwm[pin_index].pin;
	gpio_init_structure_pwm.Mode = mode;
	gpio_init_structure_pwm.Pull = pull;
	gpio_init_structure_pwm.Speed = GPIO_SPEED_FREQ_HIGH;
	gpio_init_structure_pwm.Alternate = stm32l476rg_digital_pins_pwm[pin_index].Alternate;

	// Init the pin
	HAL_GPIO_Init(stm32l476rg_digital_pins_pwm[pin_index].port, &gpio_init_structure_pwm);
}

void pwm_init(uint8_t pin_index)
{
	//set the clock
	pwm_clk_enable(stm32l476rg_digital_pins_pwm[pin_index + 9].Alternate);

	//set the init structure
	pwm_handle[pin_index].Instance = stm32l476rg_pwm_set[pin_index].tim;
	pwm_handle[pin_index].State = HAL_TIM_STATE_RESET;
	pwm_handle[pin_index].Channel = stm32l476rg_pwm_set[pin_index].channel;
	pwm_handle[pin_index].Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	pwm_handle[pin_index].Init.CounterMode = TIM_COUNTERMODE_UP;
	pwm_handle[pin_index].Init.Period = 0xFFFF;
	pwm_handle[pin_index].Init.Prescaler = 0;
	HAL_TIM_PWM_Init(&pwm_handle[pin_index]);

	pwm_oc_init[pin_index].OCFastMode = TIM_OCFAST_DISABLE;
	pwm_oc_init[pin_index].OCIdleState = TIM_OCIDLESTATE_RESET;
	pwm_oc_init[pin_index].OCMode = TIM_OCMODE_PWM1;
	pwm_oc_init[pin_index].OCPolarity = TIM_OCPOLARITY_HIGH;
	pwm_oc_init[pin_index].Pulse = 0xFFFF;
	HAL_TIM_PWM_ConfigChannel(&pwm_handle[pin_index], &pwm_oc_init[pin_index], stm32l476rg_pwm_set[pin_index].tim_ch);
}

void pwm_set_duty(uint8_t duty, uint8_t pin_index)
{
	uint32_t pulse = pwm_handle[pin_index].Init.Period * (duty / 100.0);
	pwm_oc_init[pin_index].Pulse = pulse;
	HAL_TIM_PWM_ConfigChannel(&pwm_handle[pin_index], &pwm_oc_init[pin_index], stm32l476rg_pwm_set[pin_index].tim_ch);
	HAL_TIM_PWM_Start(&pwm_handle[pin_index], stm32l476rg_pwm_set[pin_index].tim_ch);
}

void init_pwms()
{
	/* Init PINs from DPIN9 to DPIN11 as a PWM */
	for (int i = 9; i < 12; i++) {
		pwm_pin_init(i, GPIO_MODE_AF_PP, GPIO_NOPULL);
	}

	/* Init pwm function from DPIN9 DPIN11 */
	for (int i = 0; i < 3; i++) {
		pwm_init(i);
	}

}


