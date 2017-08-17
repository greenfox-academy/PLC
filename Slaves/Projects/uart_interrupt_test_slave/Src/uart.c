/* Includes ------------------------------------------------------------------*/
#include "uart.h"
#include "lcd_log.h"
#include "main.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

void rx_tx_GPIO_init();
void uart_init();
void buffer_init();;

/* Private functions ---------------------------------------------------------*/

void UART_send(uint8_t *buffer)
{
	// Send buffer content
	HAL_UART_Transmit(&uart_handle, (uint8_t*) buffer, 16, 4);

}


void modbus_init()
{
	buffer_init();
	rx_tx_GPIO_init();
	uart_init();
}

void buffer_init()
{
	for (int i = 0; i < RXBUFFERSIZE; i++) {
		RX_buffer[i] = 0;
	}
}

void uart_init()
{
	uart_handle.Instance 	   	= USARTx;
	uart_handle.Init.BaudRate   = BAUDRATE;
	uart_handle.Init.WordLength	= UART_WORDLENGTH_8B;
	uart_handle.Init.StopBits  	= UART_STOPBITS_1;
	uart_handle.Init.Parity     	= UART_PARITY_NONE;
	uart_handle.Init.HwFlowCtl  	= UART_HWCONTROL_NONE;
	uart_handle.Init.Mode       	= UART_MODE_TX_RX;
	uart_handle.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

	if(HAL_UART_DeInit(&uart_handle) != HAL_OK)
		LCD_UsrLog("Uart deinit error.\n");
	 // Error_Handler();

	if(HAL_UART_Init(&uart_handle) != HAL_OK)
	LCD_UsrLog("Uart init error.\n");
		// Error_Handler();

	//Setup interrupts for UART
	HAL_NVIC_SetPriority(USART6_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(USART6_IRQn);

	// Start UART receiver in interrupt mode
	HAL_UART_Receive_IT(&uart_handle, RX_buffer, 16);

	LCD_UsrLog("UART - Initialized.\n");

	address = 10;
	interrupt_flag = 0;
}

void rx_tx_GPIO_init()
{
	GPIO_InitTypeDef  GPIO_InitStruct;

	/*##-1- Enable peripherals and GPIO Clocks ##*/
	/* Enable GPIO TX/RX clock */
	USARTx_TX_GPIO_CLK_ENABLE();
	USARTx_RX_GPIO_CLK_ENABLE();


	/* Enable USARTx clock */
	USARTx_CLK_ENABLE();

	/*##-2- Configure peripheral GPIO ##*/
	/* UART TX GPIO pin configuration  */
	GPIO_InitStruct.Pin       = USARTx_TX_PIN;
	GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull      = GPIO_PULLUP;
	GPIO_InitStruct.Speed     = GPIO_SPEED_HIGH;
	GPIO_InitStruct.Alternate = USARTx_TX_AF;

	HAL_GPIO_Init(USARTx_TX_GPIO_PORT, &GPIO_InitStruct);

	/* UART RX GPIO pin configuration  */
	GPIO_InitStruct.Pin = USARTx_RX_PIN;
	GPIO_InitStruct.Alternate = USARTx_RX_AF;

	HAL_GPIO_Init(USARTx_RX_GPIO_PORT, &GPIO_InitStruct);

	LCD_UsrLog("GPIO - Rx,Tx - Initialized.\n");
}


/*
void UART_rx_thread(void const * argument)
{

	uint8_t char_to_copy;

	uart_init();

	while (1) {

		if (command_in) {

			uint16_t i = 0;

			// Copy data from circular buffer to command buffer
			// between current position of the read pointer and the next LF
			do {
				// Copy one char and move the read pointer to the next one
				char_to_copy = *(RX_buffer.read_p++);

				// Loop read pointer at the end of the circular buffer
				if (RX_buffer.read_p > RX_buffer.tail_p) {
					RX_buffer.read_p = RX_buffer.head_p;
				}

				// Add char to command buffer and move pointer by one
				command_buffer[i++]= char_to_copy;

			} while (char_to_copy != '\n');

			// Remove LF char from the last written position
			command_buffer[--i] = 0;

			if ((i > 0) && (command_buffer[--i] == '\r')) {
				command_buffer[i] = 0;
			}

			// Uncomment for debug
			//
			// char tmp[RXBUFFERSIZE + 20] = "command_buffer: ";
			// strcat(tmp, (char*) command_buffer);
			// strcat(tmp, "\n");
			// log_msg(DEBUG, tmp);

			// Process command
			process_command();
			execute_command();

			// Decrease command counter
			command_in--;
		}

		osDelay(10);
	}

	while (1) {
		log_msg(USER, "UART RX thread terminating\n");
		osThreadTerminate(NULL);
	}
}

void process_command(void)
{

	// Clear command structure
	c_params.attrib = NO_ATTRIB;
	c_params.command = NO_COMMAND;
	c_params.device_id = 255;
	c_params.value = 0;
	c_params.value_x = 65535;
	c_params.value_y = 65535;
	c_params.value_z = 65535;
	c_params.error = 0;

	// Copy command from command buffer
	char received[RXBUFFERSIZE];
	strcpy(received, (char*) command_buffer);

	// Command
	char* s = strtok(received, " ");

	if ((strcmp(s, "set") == 0) || (strcmp(s, "s") == 0)) {
		c_params.command = SET_VALUE;
	} else if ((strcmp(s, "get") == 0) || (strcmp(s, "g") == 0)) {
		c_params.command = GET_VALUE;
	} else if ((strcmp(s, "help") == 0) || (strcmp(s, "h") == 0)) {
		c_params.command = HELP;
		return;
	} else {
		c_params.error = 4;
		return;
	}

	// Attribute
	s = strtok(NULL, " ");

	if ((strcmp(s, "pulse") == 0) || (strcmp(s, "pul") == 0)) {
		c_params.attrib = PULSE;
	} else if ((strcmp(s, "position") == 0) || (strcmp(s, "pos") == 0)) {
		c_params.attrib = POSITION;
	} else if ((strcmp(s, "angle") == 0) || (strcmp(s, "ang") == 0)) {
		c_params.attrib = ANGLE;
	} else if ((strcmp(s, "manual") == 0) || (strcmp(s, "man") == 0)) {
		c_params.attrib = MANUAL_CONTROL;
	} else if ((strcmp(s, "display") == 0) || (strcmp(s, "dis") == 0)) {
		c_params.attrib = DATA_DISP;
	} else if ((strcmp(s, "demo") == 0) || (strcmp(s, "dem") == 0)) {
		c_params.attrib = DEMO;
	} else {
		c_params.error = 2;
		return;
	}

	// Device id
	if ((c_params.command == SET_VALUE) &&
		((c_params.attrib == PULSE) || (c_params.attrib == ANGLE))) {
		char* s = strtok(NULL, " ");

		// Convert ASCII to integer
		c_params.device_id = atoi(s);

		// Check if we are in the accepted range
		if (c_params.device_id >= SERVOS) {
			c_params.error = 3;
		}
	}

	// Value
	if ((c_params.command != HELP) && (c_params.command != GET_VALUE)
		&& (c_params.attrib != POSITION)) {
		char* s = strtok(NULL, " ");

		// Convert ASCII to integer
		c_params.value = atoi(s);

		if ((c_params.command == SET_VALUE) && (c_params.attrib == PULSE)) {
			c_params.error = verify_pulse(c_params.device_id, c_params.value);
		}

		if ((c_params.command == SET_VALUE) && (c_params.attrib == ANGLE)) {
			c_params.error = verify_angle(c_params.device_id, c_params.value);
		}
	}

	// XYZ value
	if ((c_params.command == SET_VALUE) && (c_params.attrib == POSITION)) {
		char* s = strtok(NULL, " ");

		char* coord = strtok(s, ",");
		c_params.value_x = atoi(coord);

		coord = strtok(NULL, ",");
		c_params.value_y = atoi(coord);

		coord = strtok(NULL, ",");
		c_params.value_z = atoi(coord);

		c_params.error = verify_coordinates(c_params.value_x, c_params.value_y, c_params.value_z);
	}

	// Uncomment for debug
	//
	//	char tmp[100];
	//	sprintf(tmp, "command: %d, attrib: %d, dev: %d, value: %d, x: %d, y: %d, z: %d, err: %d\n",
	//				c_params.command, c_params.attrib, c_params.device_id, c_params.value,
	//				c_params.value_x, c_params.value_y, c_params.value_z, c_params.error);
	//	log_msg(DEBUG, tmp);

	return;
}

void execute_command(void)
{
	// Send error message
	if (c_params.error) {
		sprintf((char*) TX_buffer, "Unrecognized command or value: %s", (char*) command_buffer);
		UART_send((char*) TX_buffer);
		return;
	}

	// Execute command
	switch (c_params.command) {

	// Help
	case HELP:
		UART_send_help();
		break;

	// Get value
	case GET_VALUE:
		UART_send_settings();
		break;

	// Set value
	case SET_VALUE:
		set_value();
		break;

	// Error
	default:
		UART_send("Unrecognized command or value");
		return;
	}
	return;
}

void UART_send_settings(void)
{
	switch (c_params.attrib) {

	case PULSE:
		for (int i = 0; i < SERVOS; i++) {
			// Read value from global storage
			osMutexWait(servo_pulse_mutex, osWaitForever);
			uint32_t pulse = servo_pulse[i];
			osMutexRelease(servo_pulse_mutex);
			// Send value to user
			sprintf((char*) TX_buffer, "Servo%d pulse: %lu", i, pulse);
			UART_send((char*) TX_buffer);
		}
		break;

	case ANGLE:
		for (int i = 0; i < SERVOS; i++) {
			// Get pulse value from global storage
			osMutexWait(servo_pulse_mutex, osWaitForever);
			uint32_t pulse = servo_pulse[i];
			osMutexRelease(servo_pulse_mutex);

			// TODO check angle calculation

			// Calculate angle
			uint8_t angle = (uint8_t) map((double) pulse, (double) servo_conf[i].min_pulse,
					(double) servo_conf[i].max_pulse, (double) servo_conf[i].min_angle_deg,
					(double) servo_conf[i].max_angle_deg);

			// Send value
			sprintf((char*) TX_buffer, "Servo%d angle: %4d degrees", i, angle);
			UART_send((char*)TX_buffer);
		}
		break;

	case POSITION:

		// A block statement is needed for the declaration
		{
			coord_cart_t xyz;

			// Get xyz values
			pulse_to_xyz(&xyz);

			// Send value
			sprintf((char*) TX_buffer, "arm position: x:%d y:%d z:%d", (int16_t) xyz.x, (int16_t) xyz.y, (int16_t) xyz.z);
			UART_send((char*) TX_buffer);
		}
		break;

	case MANUAL_CONTROL:
		sprintf((char*) TX_buffer, "Manual control is %s", adc_on ? "on" : "off");
		UART_send((char*) TX_buffer);
		break;

	case DATA_DISP:
		sprintf((char*) TX_buffer, "LCD data display is %s", lcd_data_display_on ? "on" : "off");
		UART_send((char*) TX_buffer);
		break;

	case DEMO:
		sprintf((char*) TX_buffer, "Demo is %s", demo_on ? "running" : "off");
		UART_send((char*) TX_buffer);
		break;

	case NO_ATTRIB:
		break;
	}
	return;
}

void set_value(void)
{
	switch (c_params.attrib) {

	case PULSE:

		osMutexWait(servo_pulse_mutex, osWaitForever);
		servo_pulse[c_params.device_id] = c_params.value;
		osMutexRelease(servo_pulse_mutex);
		UART_send("Set pulse done.");
		break;

	case ANGLE:

		// TODO correct 2nd joint angle

		// A block statement is needed for the declaration
		{
			// Convert degree to radians
			double ang_rad = deg_to_rad(c_params.value);

			// Calculate pulse
			uint32_t pulse = (uint32_t) map(ang_rad, servo_conf[c_params.device_id].min_angle_rad,
										  servo_conf[c_params.device_id].max_angle_rad,
										  (double) servo_conf[c_params.device_id].min_pulse,
										  (double) servo_conf[c_params.device_id].max_pulse);

			// Set pulse
			osMutexWait(servo_pulse_mutex, osWaitForever);
			servo_pulse[c_params.device_id] = pulse;
			osMutexRelease(servo_pulse_mutex);
			UART_send("Set angle done.");
		}
		break;

	case POSITION:

		// A block statement is needed for the declaration
		{
			// Read in xyz values
			coord_cart_t coord;
			coord.x = (double) c_params.value_x;
			coord.y = (double) c_params.value_y;
			coord.z = (double) c_params.value_z;

			// Set pwm pulse
			xyz_to_pulse(&coord);

			UART_send("Set position done.");
		}
		break;

	case MANUAL_CONTROL:
		if (c_params.value > 0) {
			start_adc_thread();
			UART_send("Manual control started, ADC running.");
		} else {
			stop_adc_thread();
			UART_send("Manual control ended, ADC terminated.");
		}
		break;

	case DATA_DISP:
		if (c_params.value > 0) {
			start_lcd_data_display();
			UART_send("LCD data display turned on.");
		} else {
			stop_lcd_data_display();
			UART_send("LCD data display turned off.");
		}
		break;

	case DEMO:
		if (c_params.value > 0) {
			start_demo();
			UART_send("Demo is on.");
		} else {
			stop_demo();
			UART_send("Demo is turned off.");
		}
		break;

	case NO_ATTRIB:
		break;
	}
	return;
}

uint8_t verify_coordinates(int16_t x, int16_t y, int16_t z) {

	if ((x > WORK_AREA_MAX_X) || (x < WORK_AREA_MIN_X)) {
		return 1;	// Flag error
	}

	if ((y > WORK_AREA_MAX_Y) || (y < WORK_AREA_MIN_Y)) {
		return 1;	// Flag error
	}

	if ((z > WORK_AREA_MAX_Z) || (z < WORK_AREA_MIN_Z)) {
		return 1;	// Flag error
	}

	return 0;
}

uint8_t verify_pulse(uint8_t servo, uint32_t pulse) {

	if ((pulse > servo_conf[servo].max_pulse) || (pulse < servo_conf[servo].min_pulse)) {
		return 1; // Flag error
	}

	return 0;
}

uint8_t verify_angle(uint8_t servo, int16_t angle) {

	if ((angle > servo_conf[servo].max_angle_deg) || (angle < servo_conf[servo].min_angle_deg)) {
		return 1;  // Flag error
	}

	return 0;
}
*/

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	// This interrupt is fired when we received 20byte data via USART6

	interrupt_flag = 1;
	// Re-enable the interrupt
	HAL_UART_Receive_IT(huart, RX_buffer, 16);
}