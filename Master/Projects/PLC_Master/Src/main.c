/**
  ******************************************************************************
  * @file    LwIP/LwIP_HTTP_Server_Netconn_RTOS/Src/main.c 
  * @author  MCD Application Team
  * @version V1.2.0
  * @date    30-December-2016
  * @brief   This sample code implements a http server application based on 
  *          Netconn API of LwIP stack and FreeRTOS. This application uses 
  *          STM32F7xx the ETH HAL API to transmit and receive data. 
  *          The communication is done with a web browser of a remote PC.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2016 STMicroelectronics International N.V. 
  * All rights reserved.</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "ethernetif.h"
#include "lwip/netif.h"
#include "lwip/tcpip.h"
#include "app_ethernet.h"
#include "lcd_log.h"
#include "socket_server.h"
#include "socket_client.h"
#include "modbus.h"
#include "GPIO.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
struct netif gnetif; /* network interface structure */

/* Private function prototypes -----------------------------------------------*/
void system_init(void);
static void SystemClock_Config(void);
static void StartThread(void const * argument);
static void BSP_Config(void);
static void Netif_Config(void);
static void MPU_Config(void);
static void Error_Handler(void);
static void CPU_CACHE_Enable(void);

/* Private functions ---------------------------------------------------------*/
void copy_array(uint16_t *target_array, uint16_t *source_array, uint8_t array_len);
void make_8b_msg(uint8_t *b8_data, uint8_t adr, uint16_t *b16_data, uint8_t b16_len);



/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
	system_init();

	/* Init thread */
	//	osThreadDef(Start, StartThread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE * 5);
	//	osThreadCreate (osThread(Start), NULL);

	/* Start scheduler */
	//  osKernelStart();

	/* Variables for Digital Slaves */
	uint8_t d_in_msg[2];		// This array holds the message for digital input slave (now it has only one element the address)
	uint8_t d_out_msg[2];		// First element of this array is the address, second is the state we want to see on digital output.
	uint8_t d_in_state;			// Pinstates of digital input will be stored in this variable.
	uint8_t d_out_state;		// Pinstates of digital output will be stored in this variable.

	d_in_msg[0] = 1;			// Digital input address.
	d_in_msg[1] = 0;
	d_out_msg[0] = 5;			// DIgital output address.
	d_out_msg[1] = 0;			// Set msg[2] to 0 for safety.
	d_in_state = 0;				// Set in state to 0 for safety.
	d_out_state = 0;			// Set out state to 0 for safety.

	/* Variables for Analog Slaves */
	uint8_t a_in_msg[2];		// This array holds the message for analog input slave (now it has only one element the address)
	uint8_t a_out_msg[13];		// First element of this array is the address, 2nd - 7th are the states we want to see on analog output.
	uint16_t a_in_state[6];		// Pinstates of analog input will be stored in this variable.
	uint16_t a_out_state[6];	// Pinstates of analog output will be stored in this variable.

	a_in_msg[0] = 9;			// Analog input address.
	a_out_msg[0] = 13;			// Analog output address.

	/* Set the a_out_msg[1] - [6]; a_in_state; a_out_state to 0 for safety */
	for (uint8_t i = 0; i < 6; i++) {
		a_in_state[i] = 0;
		a_out_state[i] = 0;
		a_out_msg[2*i + 1] = 0;
		a_out_msg[2*i + 2] = 0;
	}

	/*
    // Load table for test
	for (uint8_t i = 0; i < 6; i++) {
    	a_out_state[i] = (i + 1) * 100;
        LCD_UsrLog("data16_b[%u]: %u\n", i, a_out_state[i]);
    }

	make_8b_from_16_b(a_out_msg, 13, a_out_state, 6);

	for (uint8_t i = 0; i < 13; i++) {
		LCD_UsrLog("d8lh: %u \n", a_out_msg[i]);
	}
*/

	while (1) {

		while (!BSP_PB_GetState(BUTTON_KEY)) {
/*
		// Command to digital input
		LCD_UsrLog("DIG Input: ");
		modbus_send_command(d_in_msg, 2);					// Send the address to the digital input slave
		d_out_msg[1] = modbus_receive_data(1)[0];			// Receive pin states from digital input slave
*/
		// Command to analog input
		LCD_UsrLog("A_IN:  ");
		modbus_send_command(a_in_msg, 2);						// Send the address to the analog input slave
		copy_array(a_in_state, modbus_receive_u16_data(6), 6); // Receive adc datas from analog input slave

		// Logic
		copy_array(a_out_state, a_in_state, 6);					// Copy in to out table An = Bn logic

		// Command to analog output
		LCD_UsrLog("A_OUT: ");
		make_8b_msg(a_out_msg, 13, a_out_state, 6);			// Make 8bit message from 16bit data
		modbus_send_command(a_out_msg, 13);					// Send the address to the analog output slave
		modbus_receive_u16_data(6);							// Receive datas from analog input slave
/*
		LCD_UsrLog("DIG Output: ");
		modbus_send_command(d_out_msg, 2);					// Send message to digital output
		modbus_receive_data(1);								// Receive data from digital output
*/

		HAL_Delay(10);
		}

		HAL_Delay(10000);

	}
}

void system_init(void)
{
	/* Configure the MPU attributes as Device memory for ETH DMA descriptors */
	MPU_Config();

	/* Enable the CPU Cache */
	CPU_CACHE_Enable();

	/* STM32F7xx HAL library initialization:
	- Configure the Flash ART accelerator on ITCM interface
	- Configure the Systick to generate an interrupt each 1 msec
	- Set NVIC Group Priority to 4
	- Global MSP (MCU Support Package) initialization
	*/
	HAL_Init();

	/* Configure the system clock to 200 MHz */
	SystemClock_Config();

	/* Initialize LCD */
	BSP_Config();

	/* Initialize Button */
	BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_GPIO);

	/* Initialize Led */
	BSP_LED_Init(LED_GREEN);

	/* Initialize digital inputs and outputs*/
	for (int i = 8; i < 16; i++) {
		gpio_init_digital_pin(i, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL);
	}

	for (int i = 2; i < 8; i++) {
		gpio_init_digital_pin(i, GPIO_MODE_INPUT, GPIO_PULLDOWN);
	}

	/* Initialize Modbus */
	modbus_init();
}

/**
  * @brief  Start Thread
  * @param  argument not used
  * @retval None
  */
static void StartThread(void const * argument)
{

	/* Create tcp_ip stack thread */
	tcpip_init(NULL, NULL);

	/* Initialize the LwIP stack */
	Netif_Config();

	/* Notify user about the network interface config */
	User_notification(&gnetif);

	/* Start  control_slaves_thread*/
//	osThreadDef(CONTROL_SLAVES, control_slaves_thread, osPriorityBelowNormal, 0, configMINIMAL_STACK_SIZE * 2);
//	osThreadCreate (osThread(CONTROL_SLAVES), &gnetif);

	/* Start DHCPClient */
/*
	osThreadDef(DHCP, DHCP_thread, osPriorityBelowNormal, 0, configMINIMAL_STACK_SIZE * 2);
	osThreadCreate (osThread(DHCP), &gnetif);
	osDelay(2000);

	// Define and start the server thread
	osThreadDef(Servi, socket_server_thread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE * 2);
	osThreadCreate (osThread(Servi), &gnetif);
	osDelay(2000);

	// Define and start the client thread
	osThreadDef(send_message, socket_client_thread, osPriorityLow, 0, configMINIMAL_STACK_SIZE * 2);
	osThreadCreate (osThread(send_message), &gnetif);
*/

	osDelay(2000);

	while (1) {
		/* Delete the Init Thread */
		osThreadTerminate(NULL);
	}
}


void copy_array(uint16_t *target_array, uint16_t *source_array, uint8_t array_len)
{
	for (uint8_t i = 0; i < array_len; i++) {
		target_array[i] = source_array[i];
	}
}



void make_8b_msg(uint8_t *b8_data, uint8_t adr, uint16_t *b16_data, uint8_t b16_len)
{
    b8_data[0] = adr;

    for (int i = 1; i <= b16_len; i++) {
        b8_data[2 * i - 1] = b16_data[i - 1];
        b8_data[2 * i] = b16_data[i -1] >> 8;
    }
}

/**
  * @brief  Initializes the lwIP stack
  * @param  None
  * @retval None
  */
static void Netif_Config(void)
{ 
  ip_addr_t ipaddr;
  ip_addr_t netmask;
  ip_addr_t gw;
 
  ip_addr_set_zero_ip4(&ipaddr);
  ip_addr_set_zero_ip4(&netmask);
  ip_addr_set_zero_ip4(&gw);
  
  netif_add(&gnetif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &tcpip_input);
  
  /*  Registers the default network interface. */
  netif_set_default(&gnetif);
  
  if (netif_is_link_up(&gnetif))
  {
    /* When the netif is fully configured this function must be called.*/
    netif_set_up(&gnetif);
  }
  else
  {
    /* When the netif link is down this function must be called */
    netif_set_down(&gnetif);
  }
}

/**
  * @brief  Initializes the STM327546G-Discovery's LCD  resources.
  * @param  None
  * @retval None
  */
static void BSP_Config(void)
{
  /* Initialize the LCD */
  BSP_LCD_Init();
  
  /* Initialize the LCD Layers */
  BSP_LCD_LayerDefaultInit(1, LCD_FB_START_ADDRESS);
  
  /* Set LCD Foreground Layer  */
  BSP_LCD_SelectLayer(1);
  
  BSP_LCD_SetFont(&LCD_DEFAULT_FONT);
  
  /* Initialize TS */
  BSP_TS_Init(BSP_LCD_GetXSize(), BSP_LCD_GetYSize());

  /* Initialize LCD Log module */
  LCD_LOG_Init();
  
  /* Show Header and Footer texts */
  LCD_LOG_SetHeader((uint8_t *)"PLC - MASTER");
  LCD_LOG_SetFooter((uint8_t *)"STM32746G-DISCO - GreenFoxAcademy");
  
//  LCD_UsrLog ((char *)"Notification - Ethernet Initialization ...\n");
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 200000000
  *            HCLK(Hz)                       = 200000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 4
  *            APB2 Prescaler                 = 2
  *            HSE Frequency(Hz)              = 25000000
  *            PLL_M                          = 25
  *            PLL_N                          = 432
  *            PLL_P                          = 2
  *            PLL_Q                          = 9
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 7
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;

  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_OFF;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 400;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 9;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /* activate the OverDrive */
  if(HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }
  
  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;  
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void Error_Handler(void)
{
  /* User may add here some code to deal with this error */
  while(1)
  {
  }
}

/**
  * @brief  Configure the MPU attributes as Device for  Ethernet Descriptors in the SRAM1.
  * @note   The Base Address is 0x20010000 since this memory interface is the AXI.
  *         The Configured Region Size is 256B (size of Rx and Tx ETH descriptors) 
  *       
  * @param  None
  * @retval None
  */
static void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct;
  
  /* Disable the MPU */
  HAL_MPU_Disable();
  
  /* Configure the MPU attributes as Device for Ethernet Descriptors in the SRAM */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress = 0x20010000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_256B;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /* Enable the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

/**
  * @brief  CPU L1-Cache enable.
  * @param  None
  * @retval None
  */
static void CPU_CACHE_Enable(void)
{
  /* Enable I-Cache */
  SCB_EnableICache();

  /* Enable D-Cache */
  SCB_EnableDCache();
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
