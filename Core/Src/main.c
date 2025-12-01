/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os2.h"
#include "FreeRTOS.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "lvgl_port_display.h"
#include "input_task.h"
#include "motor.h"
#include "sensor_task.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

COM_InitTypeDef BspCOMInit;
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim2;

/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 256 * 4
};
/* USER CODE BEGIN PV */
/* Definitions for LVGLTask */
osThreadId_t lvglTaskHandle;
const osThreadAttr_t lvglTask_attributes = {
  .name = "lvglTask",
  .priority = (osPriority_t) osPriorityHigh,
  .stack_size = 512 * 4
};

/* Definitions for InputTask */
osThreadId_t inputTaskHandle;
const osThreadAttr_t inputTask_attributes = {
  .name = "inputTask",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 256 * 4
};

/* Definitions for SensorTask */
osThreadId_t sensorTaskHandle;
const osThreadAttr_t sensorTask_attributes = {
  .name = "sensorTask",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 384 * 4
};

/* Flag to control LVGL rendering */
volatile bool rendering = true;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM2_Init(void);
static void MX_ADC1_Init(void);
void StartDefaultTask(void *argument);

/* USER CODE BEGIN PFP */
void StartLVGLTask(void *argument);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* Display functions moved to `Drivers/lvgl_port_display`.
 * Use `display_system_init()` to initialize the display and LVGL.
 */
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2C1_Init();
  MX_TIM2_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */
  /* Initializations moved to according tasks to avoid issues before scheduler starts */
  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* creation of lvglTask */
  lvglTaskHandle = osThreadNew(StartLVGLTask, NULL, &lvglTask_attributes);
  sensorTaskHandle = osThreadNew(StartSensorTask, NULL, &sensorTask_attributes);
  inputTaskHandle = osThreadNew(StartInputTask, NULL, &inputTask_attributes);
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

  /* Initialize leds */
  BSP_LED_Init(LED_BLUE);
  BSP_LED_Init(LED_GREEN);
  BSP_LED_Init(LED_RED);

  /* Initialize USER push-button, will be used to trigger an interrupt each time it's pressed.*/
  BSP_PB_Init(BUTTON_SW1, BUTTON_MODE_EXTI);
  BSP_PB_Init(BUTTON_SW2, BUTTON_MODE_EXTI);
  BSP_PB_Init(BUTTON_SW3, BUTTON_MODE_EXTI);

  /* Initialize COM1 port (115200, 8 bits (7-bit data + 1 stop bit), no parity */
  BspCOMInit.BaudRate   = 115200;
  BspCOMInit.WordLength = COM_WORDLENGTH_8B;
  BspCOMInit.StopBits   = COM_STOPBITS_1;
  BspCOMInit.Parity     = COM_PARITY_NONE;
  BspCOMInit.HwFlowCtl  = COM_HWCONTROL_NONE;
  if (BSP_COM_Init(COM1, &BspCOMInit) != BSP_ERROR_NONE)
  {
    Error_Handler();
  }

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (0)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_10;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV2;
  RCC_OscInitStruct.PLL.PLLN = 8;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the SYSCLKSource, HCLK, PCLK1 and PCLK2 clocks dividers
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK4|RCC_CLOCKTYPE_HCLK2
                              |RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.AHBCLK2Divider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLK4Divider = RCC_SYSCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SMPS;
  PeriphClkInitStruct.SmpsClockSelection = RCC_SMPSCLKSOURCE_HSI;
  PeriphClkInitStruct.SmpsDivSelection = RCC_SMPSCLKDIV_RANGE0;

  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN Smps */

  /* USER CODE END Smps */
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */
  /* ADC channels need the maximum sampling time.
   * Minimum measurement duration is 640.5/(64MHz/64) seconds with the current ADC clock.
   */
  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV32;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SEQ_CONV;
  hadc1.Init.LowPowerAutoWait = ENABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.NbrOfConversion = 4;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DMAContinuousRequests = ENABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.OversamplingMode = ENABLE;
  hadc1.Init.Oversampling.Ratio = ADC_OVERSAMPLING_RATIO_256;
  hadc1.Init.Oversampling.RightBitShift = ADC_RIGHTBITSHIFT_4;
  hadc1.Init.Oversampling.TriggeredMode = ADC_TRIGGEREDMODE_SINGLE_TRIGGER;
  hadc1.Init.Oversampling.OversamplingStopReset = ADC_REGOVERSAMPLING_CONTINUED_MODE;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_VREFINT;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_640CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
  sConfig.Rank = ADC_REGULAR_RANK_3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_VBAT;
  sConfig.Rank = ADC_REGULAR_RANK_4;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x0040040C;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }

  /** I2C Enable Fast Mode Plus
  */
  HAL_I2CEx_EnableFastModePlus(I2C_FASTMODEPLUS_I2C1);
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_Encoder_InitTypeDef sConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 65535;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  sConfig.EncoderMode = TIM_ENCODERMODE_TI12;
  sConfig.IC1Polarity = TIM_ICPOLARITY_FALLING;
  sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC1Filter = 15;
  sConfig.IC2Polarity = TIM_ICPOLARITY_FALLING;
  sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC2Filter = 15;
  if (HAL_TIM_Encoder_Init(&htim2, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMAMUX1_CLK_ENABLE();
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(MOTOR_IN2_GPIO_Port, MOTOR_IN2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(MOTOR_IN1_GPIO_Port, MOTOR_IN1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : MOTOR_IN2_Pin */
  GPIO_InitStruct.Pin = MOTOR_IN2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(MOTOR_IN2_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : BUTTON_MIDDLE_Pin */
  GPIO_InitStruct.Pin = BUTTON_MIDDLE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(BUTTON_MIDDLE_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : BUTTON_LEFT_Pin BUTTON_RIGHT_Pin */
  GPIO_InitStruct.Pin = BUTTON_LEFT_Pin|BUTTON_RIGHT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : MOTOR_IN1_Pin */
  GPIO_InitStruct.Pin = MOTOR_IN1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(MOTOR_IN1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : USB_DM_Pin USB_DP_Pin */
  GPIO_InitStruct.Pin = USB_DM_Pin|USB_DP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF10_USB;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI2_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI2_IRQn);

  HAL_NVIC_SetPriority(EXTI3_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);

  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

static int32_t scale_value_for_label(float value, uint8_t decimals)
{
  int32_t scale = 1;
  for (uint8_t i = 0; i < decimals; ++i)
  {
    scale *= 10;
  }
  const float scaled = value * (float)scale + (value >= 0.0f ? 0.5f : -0.5f);
  return (int32_t)scaled;
}

static void sensor_current_label_update(lv_obj_t *label, float current)
{
  if (label == NULL)
  {
    return;
  }

  const int32_t scaled = scale_value_for_label(current, 3);
  int32_t integral = scaled / 1000;
  int32_t fraction = scaled % 1000;
  if (fraction < 0)
  {
    fraction = -fraction;
  }

  char buf[32];
  snprintf(buf, sizeof(buf), "I_M:%ld.%03ldA", (long)integral, (long)fraction);
  lv_label_set_text(label, buf);
}

static void sensor_battery_label_update(lv_obj_t *label, float voltage)
{
  if (label == NULL)
  {
    return;
  }

  const int32_t scaled = scale_value_for_label(voltage, 2);
  int32_t integral = scaled / 100;
  int32_t fraction = scaled % 100;
  if (fraction < 0)
  {
    fraction = -fraction;
  }

  char buf[32];
  snprintf(buf, sizeof(buf), "Bat:%ld.%02ldV", (long)integral, (long)fraction);
  lv_label_set_text(label, buf);
}

static void sensor_temperature_label_update(lv_obj_t *label, float temperature)
{
  if (label == NULL)
  {
    return;
  }

  const int32_t scaled = scale_value_for_label(temperature, 1);
  int32_t integral = scaled / 10;
  int32_t fraction = scaled % 10;
  if (fraction < 0)
  {
    fraction = -fraction;
  }

  char buf[32];
  snprintf(buf, sizeof(buf), "T:%ld.%1ldC", (long)integral, (long)fraction);
  lv_label_set_text(label, buf);
}

static void sensor_display_update(lv_obj_t *current_label,
                                  lv_obj_t *battery_label,
                                  lv_obj_t *temp_label,
                                  const SensorValuesTypeDef *values)
{
  if (values == NULL)
  {
    return;
  }

  sensor_current_label_update(current_label, values->MotorCurrent);
  sensor_battery_label_update(battery_label, values->BatteryVoltage);
  sensor_temperature_label_update(temp_label, values->CurrentTemp);
}

static void update_go_button_label(lv_obj_t *label, bool forward)
{
  if (label == NULL)
  {
    return;
  }

  lv_label_set_text_fmt(label, "Go: %s", forward ? "F" : "R");
}

void StartLVGLTask(void *argument)
{
  /* Infinite loop - dedicated LVGL rendering task */
  for(;;)
  {
    /* Only render if the rendering flag is set */
    if (rendering)
    {
      /* Handle LVGL timers and rendering */
      lv_timer_handler();
    }

    /* Yield to other tasks - adjust delay based on display refresh rate */
    osDelay(10);
  }
}

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN 5 */
  /* Initialize display and LVGL after scheduler starts to avoid HardFault */
  display_system_init();
  Motor_Init();
  SensorTask_SetTemperatureCalibrationOffset(5.0f);

  lv_obj_t *scr = lv_scr_act();
  lv_obj_clean(scr);
  lv_obj_set_style_bg_color(scr, lv_color_black(), 0);

  rendering = false;
  osDelay(20);

  lv_obj_t *encoder_label = lv_label_create(scr);
  lv_obj_set_style_text_color(encoder_label, lv_color_white(), 0);
  lv_label_set_text(encoder_label, "RE: 0");
  lv_obj_align(encoder_label, LV_ALIGN_TOP_LEFT, 4, 4);

  lv_obj_t *current_label = lv_label_create(scr);
  lv_obj_set_style_text_color(current_label, lv_color_white(), 0);
  lv_label_set_text(current_label, "I: --mA");
  lv_obj_align_to(current_label, encoder_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 2);

  lv_obj_t *battery_label = lv_label_create(scr);
  lv_obj_set_style_text_color(battery_label, lv_color_white(), 0);
  lv_label_set_text(battery_label, "Bat: --.--V");
  lv_obj_align(battery_label, LV_ALIGN_TOP_RIGHT, -4, 4);

  lv_obj_t *temp_label = lv_label_create(scr);
  lv_obj_set_style_text_color(temp_label, lv_color_white(), 0);
  lv_label_set_text(temp_label, "T: --.-C");
  lv_obj_align_to(temp_label, battery_label, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 2);

  SensorValuesTypeDef initial_values = {0.0f, 0.0f, 0.0f};
  if (SensorValues_Copy(&initial_values))
  {
    sensor_display_update(current_label, battery_label, temp_label, &initial_values);
  }
  
  struct button_ui
  {
    lv_obj_t *btn;
    lv_obj_t *label;
    bool active;
  } buttons[3];

  const char *const button_texts[3] = {"Mode", "", "Menu"};
  const lv_coord_t button_width = 37;
  const lv_coord_t button_height = 18;
  const lv_coord_t gap = 3;
  const lv_coord_t margin = 3;

  for (size_t i = 0; i < 3; ++i)
  {
    buttons[i].btn = lv_btn_create(scr);
    lv_obj_set_size(buttons[i].btn, button_width, button_height);
    lv_obj_align(buttons[i].btn, LV_ALIGN_BOTTOM_LEFT, margin + i * (button_width + gap), -margin);
    lv_obj_set_style_bg_color(buttons[i].btn, lv_color_black(), 0);
    lv_obj_set_style_border_width(buttons[i].btn, 1, 0);
    lv_obj_set_style_border_color(buttons[i].btn, lv_color_white(), 0);

    buttons[i].label = lv_label_create(buttons[i].btn);
    lv_label_set_text(buttons[i].label, button_texts[i]);
    lv_obj_set_style_text_color(buttons[i].label, lv_color_white(), 0);
    lv_obj_set_style_text_font(buttons[i].label, &lv_font_montserrat_12, 0);
    lv_obj_center(buttons[i].label);
    buttons[i].active = false;
  }

  bool motor_running = false;
  bool motor_direction_forward = true;
  update_go_button_label(buttons[1].label, motor_direction_forward);

  rendering = true;

  static const Input2VPEventTypeDef button_event_types[] = {
    EVT_MODE_BTN,
    EVT_CENTRAL_BTN,
    EVT_MENU_BTN,
  };
  const size_t button_event_count = sizeof(button_event_types) / sizeof(button_event_types[0]);

  int32_t encoder_value = 0;
  const TickType_t sensor_display_interval = pdMS_TO_TICKS(500U);
  const TickType_t event_wait_ticks = pdMS_TO_TICKS(50U);
  TickType_t last_sensor_tick = osKernelGetTickCount();

  for (;;)
  {
    Input2VPEvent_t event;
    const bool event_ready = InputTask_TryGetVPEvent(&event, event_wait_ticks);

    if (event_ready)
    {
      if (event.type == EVT_CTRL_WHEEL_DELTA)
      {
        encoder_value += event.delta;
        lv_label_set_text_fmt(encoder_label, "RE:%d", (int)encoder_value);
      }
      else
      {
        switch (event.type)
        {
          case EVT_MODE_BTN:
            if (event.button_action == BUTTON_ACTION_PRESSED)
            {
              motor_direction_forward = !motor_direction_forward;
              if (motor_running)
              {
                Motor_SetState(motor_direction_forward ? MOTOR_FORWARD : MOTOR_BACKWARD);
              }
              update_go_button_label(buttons[1].label, motor_direction_forward);
            }
            break;
          case EVT_CENTRAL_BTN:
            if (event.button_action == BUTTON_ACTION_PRESSED)
            {
              motor_running = true;
              Motor_SetState(motor_direction_forward ? MOTOR_FORWARD : MOTOR_BACKWARD);
            }
            else
            {
              motor_running = false;
              Motor_SetState(MOTOR_COAST);
            }
            break;
          default:
            break;
        }

        size_t idx = button_event_count;
        for (size_t i = 0; i < button_event_count; ++i)
        {
          if (event.type == button_event_types[i])
          {
            idx = i;
            break;
          }
        }

        if (idx < button_event_count)
        {
          const bool pressed = (event.button_action == BUTTON_ACTION_PRESSED);
          if (pressed != buttons[idx].active)
          {
            buttons[idx].active = pressed;
            const lv_color_t bg = pressed ? lv_color_white() : lv_color_black();
            const lv_color_t txt = pressed ? lv_color_black() : lv_color_white();

            lv_obj_set_style_bg_color(buttons[idx].btn, bg, 0);
            lv_obj_set_style_text_color(buttons[idx].label, txt, 0);
          }
        }
      }
    }

    const TickType_t now = osKernelGetTickCount();
    if ((now - last_sensor_tick) >= sensor_display_interval)
    {
      SensorValuesTypeDef values = {0.0f, 0.0f, 0.0f};
      if (SensorValues_Copy(&values))
      {
        sensor_display_update(current_label, battery_label, temp_label, &values);
      }
      last_sensor_tick = now;
    }
  }
  /* USER CODE END 5 */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM17 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM17)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */
  lv_tick_inc(1);
  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
