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
#include "input_task.h"
#include "lvgl_port_display.h"
#include "maintenance_task.h"
#include "motor.h"
#include "sensor_task.h"
#include "storage_task.h"
#include "system_task.h"
#include "view_presenter_task.h"

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

RTC_HandleTypeDef hrtc;

TIM_HandleTypeDef htim2;

/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 1024 * 4
};
/* USER CODE BEGIN PV */
/* Definitions for LVGLTask */
osThreadId_t lvglTaskHandle;
const osThreadAttr_t lvglTask_attributes = {.name = "lvglTask",
                                            .priority =
                                                (osPriority_t)osPriorityHigh1,
                                            .stack_size = LVGL_TASK_STACK_SIZE};

/* Definitions for InputTask */
osThreadId_t inputTaskHandle;
const osThreadAttr_t inputTask_attributes = {
    .name = "inputTask",
    .priority = (osPriority_t)osPriorityHigh2,
    .stack_size = INPUT_TASK_STACK_SIZE};

/* Definitions for SensorTask */
osThreadId_t sensorTaskHandle;
const osThreadAttr_t sensorTask_attributes = {
    .name = "sensorTask",
    .priority = (osPriority_t)osPriorityHigh3,
    .stack_size = SENSOR_TASK_STACK_SIZE};

/* Definitions for StorageTask */
osThreadId_t storageTaskHandle;
const osThreadAttr_t storageTask_attributes = {
    .name = "storageTask",
    .priority = (osPriority_t)osPriorityLow,
    .stack_size = STORAGE_TASK_STACK_SIZE};

/* Storage event queue */
osMessageQueueId_t storage2SystemEventQueueHandle;

/* System -> Storage queue */
osMessageQueueId_t system2StorageEventQueueHandle;

/* Input to ViewPresenter event queue */
osMessageQueueId_t input2VPEventQueueHandle;

/* ViewPresenter -> System queue */
osMessageQueueId_t vp2SystemEventQueueHandle;

#if !TESTS
/*Definitions for ViewPresenterTask*/
osThreadId_t viewPresenterTaskHandle;
const osThreadAttr_t viewPresenterTask_attributes = {
    .name = "viewPresenterTask",
    .priority = (osPriority_t)osPriorityNormal3,
    .stack_size = VP_TASK_STACK_SIZE};

/* Definitions for SystemTask */
osThreadId_t systemTaskHandle;
const osThreadAttr_t systemTask_attributes = {
    .name = "systemTask",
    .priority = (osPriority_t)osPriorityNormal1,
    .stack_size = SYSTEM_TASK_STACK_SIZE};

/* Definitions for MaintTask */
osThreadId_t maintenanceTaskHandle;
const osThreadAttr_t maintenanceTask_attributes = {
    .name = "maintenanceTask",
    .priority = (osPriority_t)osPriorityNormal2,
    .stack_size = MAINT_TASK_STACK_SIZE};

/* System -> ViewPresenter queue */
osMessageQueueId_t system2VPEventQueueHandle;

/* System <-> Maint queues */
osMessageQueueId_t system2MaintEventQueueHandle;
osMessageQueueId_t maint2SystemEventQueueHandle;
#endif
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM2_Init(void);
static void MX_ADC1_Init(void);
static void MX_RTC_Init(void);
void StartDefaultTask(void *argument);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* Display functions moved to `Drivers/lvgl_port_display`.
 * Use `display_system_init()` to initialize the display and LVGL.
 */
#if OS_TASKS_DEBUG
static void DebugReportTaskCreation(const char *name, osThreadId_t handle) {
  printf("%s creation %s handle=%p\n", name, handle ? "succeeded" : "FAILED",
         (void *)handle);

#if ERROR_HANDLER_ON_FAILURE
  if (handle == NULL) {
    Error_Handler();
  }
#endif
}
#endif
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
  static DefaultTaskArgsTypeDef defaultTaskArgs = {
      .storage2system_event_queue = NULL,
      .input2vp_event_queue = NULL,
      .config_access = NULL,
      .sensor_values_access = NULL};
  static StorageTaskArgsTypeDef storageTaskArgs = {
      .storage2system_event_queue = NULL, .config_access = NULL};
  static SensorTaskArgsTypeDef sensorTaskArgs = {.config_access = NULL,
                                                 .sensor_values_access = NULL};
  static InputTaskArgsTypeDef inputTaskArgs = {.input2vp_event_queue = NULL};
#if !TESTS
  static ViewPresenterTaskArgsTypeDef viewPresenterTaskArgs = {
      .input2vp_event_queue = NULL,
      .vp2system_event_queue = NULL,
      .system2vp_event_queue = NULL,
      .system_context_access = NULL};
  static SystemTaskArgsTypeDef systemTaskArgs = {.vp2_system_queue = NULL,
                                                 .system2_vp_queue = NULL,
                                                 .system2_maint_queue = NULL,
                                                 .maint2_system_queue = NULL,
                                                 .system_context_access = NULL};
  static MaintenanceTaskArgsTypeDef maintenanceTaskArgs = {
      .system2_maint_queue = NULL, .maint2_system_queue = NULL};
#endif
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
  MX_RTC_Init();
  /* USER CODE BEGIN 2 */
  display_system_init();
  Motor_Init();
  /* Initializations moved to according tasks to avoid issues before scheduler
   * starts */
  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* Create config access structure with mutex */
  static ConfigAccessTypeDef configAccess = {
      .mutex = NULL, .data = {.TemperatureOffsetC = 0.0f}};
  const osMutexAttr_t configMutexAttr = {
      .name = "ConfigMutex",
      .attr_bits = osMutexPrioInherit,
  };
  configAccess.mutex = osMutexNew(&configMutexAttr);
  if (configAccess.mutex == NULL) {
    Error_Handler();
  }
  defaultTaskArgs.config_access = &configAccess;
  storageTaskArgs.config_access = &configAccess;
  sensorTaskArgs.config_access = &configAccess;

  /* Create sensor values access structure with mutex */
  static SensorValuesAccessTypeDef sensorValuesAccess = {
      .mutex = NULL,
      .data = {.CurrentTemp = 0.0f,
               .SoC = 0,
#if DRIVER_TEST
               .BatteryVoltage = 0.0f,
#endif
               .MotorCurrent = 0.0f}};
  const osMutexAttr_t sensorValuesMutexAttr = {
      .name = "SensorValuesMutex",
      .attr_bits = osMutexPrioInherit,
  };
  sensorValuesAccess.mutex = osMutexNew(&sensorValuesMutexAttr);
  if (sensorValuesAccess.mutex == NULL) {
    Error_Handler();
  }
  defaultTaskArgs.sensor_values_access = &sensorValuesAccess;
  sensorTaskArgs.sensor_values_access = &sensorValuesAccess;

#if !TESTS
  static SystemContextAccessTypeDef systemContextAccess = {
      .mutex = NULL,
      .data = {.state = STATE_INIT,
               .mode = MODE_AUTO,
               .mode_before_boost = MODE_AUTO,
               .boost_begin_time = 0,
               .adapt_result = -1}};

  /* Create system context mutex */
  const osMutexAttr_t sysCtxMutexAttr = {
      .name = "SysCtxMutex",
      .attr_bits = osMutexPrioInherit,
  };
  systemContextAccess.mutex = osMutexNew(&sysCtxMutexAttr);
  if (systemContextAccess.mutex == NULL) {
    Error_Handler();
  }
  systemTaskArgs.system_context_access = &systemContextAccess;
  viewPresenterTaskArgs.system_context_access = &systemContextAccess;
#endif

  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* Create storage event queue */
  storage2SystemEventQueueHandle =
      osMessageQueueNew(4, sizeof(Storage2SystemEventTypeDef), NULL);
  if (storage2SystemEventQueueHandle == NULL) {
    Error_Handler();
  }
  defaultTaskArgs.storage2system_event_queue = storage2SystemEventQueueHandle;
  storageTaskArgs.storage2system_event_queue = storage2SystemEventQueueHandle;

  /* Create System -> Storage event queue */
  system2StorageEventQueueHandle =
      osMessageQueueNew(4U, sizeof(System2StorageEventTypeDef), NULL);
  if (system2StorageEventQueueHandle == NULL) {
    Error_Handler();
  }
  storageTaskArgs.system2storage_event_queue = system2StorageEventQueueHandle;

  /* Create input to ViewPresenter event queue */
  input2VPEventQueueHandle =
      osMessageQueueNew(8U, sizeof(Input2VPEvent_t), NULL);
  if (input2VPEventQueueHandle == NULL) {
    Error_Handler();
  }
  defaultTaskArgs.input2vp_event_queue = input2VPEventQueueHandle;
  inputTaskArgs.input2vp_event_queue = input2VPEventQueueHandle;
#if !TESTS
  viewPresenterTaskArgs.input2vp_event_queue = input2VPEventQueueHandle;
  /* Create ViewPresenter -> System event queue */
  vp2SystemEventQueueHandle =
      osMessageQueueNew(4U, sizeof(VP2SystemEventTypeDef), NULL);
  if (vp2SystemEventQueueHandle == NULL) {
    Error_Handler();
  }
  systemTaskArgs.vp2_system_queue = vp2SystemEventQueueHandle;
  viewPresenterTaskArgs.vp2system_event_queue = vp2SystemEventQueueHandle;

  /* Create System -> ViewPresenter event queue */
  system2VPEventQueueHandle =
      osMessageQueueNew(2U, sizeof(System2VPEventTypeDef), NULL);
  if (system2VPEventQueueHandle == NULL) {
    Error_Handler();
  }
  systemTaskArgs.system2_vp_queue = system2VPEventQueueHandle;
  viewPresenterTaskArgs.system2vp_event_queue = system2VPEventQueueHandle;
  viewPresenterTaskArgs.config_access = &configAccess;
  viewPresenterTaskArgs.sensor_values_access = &sensorValuesAccess;
  systemTaskArgs.config_access = &configAccess;

  /* Create System <-> Maint queues */
  system2MaintEventQueueHandle =
      osMessageQueueNew(4U, sizeof(System2MaintEventTypeDef), NULL);
  if (system2MaintEventQueueHandle == NULL) {
    Error_Handler();
  }
  systemTaskArgs.system2_maint_queue = system2MaintEventQueueHandle;
  maintenanceTaskArgs.system2_maint_queue = system2MaintEventQueueHandle;

  maint2SystemEventQueueHandle =
      osMessageQueueNew(4U, sizeof(Maint2SystemEventTypeDef), NULL);
  if (maint2SystemEventQueueHandle == NULL) {
    Error_Handler();
  }
  systemTaskArgs.maint2_system_queue = maint2SystemEventQueueHandle;
  maintenanceTaskArgs.maint2_system_queue = maint2SystemEventQueueHandle;

  systemTaskArgs.system2_storage_queue = system2StorageEventQueueHandle;
#endif
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, (void *)&defaultTaskArgs, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  lvglTaskHandle = osThreadNew(StartLVGLTask, NULL, &lvglTask_attributes);
  sensorTaskHandle = osThreadNew(StartSensorTask, (void *)&sensorTaskArgs,
                                 &sensorTask_attributes);
  storageTaskHandle = osThreadNew(StartStorageTask, (void *)&storageTaskArgs,
                                  &storageTask_attributes);
  inputTaskHandle = osThreadNew(StartInputTask, (void *)&inputTaskArgs,
                                &inputTask_attributes);
#if !TESTS
  viewPresenterTaskHandle =
      osThreadNew(StartViewPresenterTask, (void *)&viewPresenterTaskArgs,
                  &viewPresenterTask_attributes);
  systemTaskHandle = osThreadNew(StartSystemTask, (void *)&systemTaskArgs,
                                 &systemTask_attributes);
  maintenanceTaskHandle =
      osThreadNew(StartMaintenanceTask, (void *)&maintenanceTaskArgs,
                  &maintenanceTask_attributes);
#endif
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
  BSP_COM_SelectLogPort(COM1);

#if OS_TASKS_DEBUG
  DebugReportTaskCreation("lvglTask", lvglTaskHandle);
  DebugReportTaskCreation("lvglTask", lvglTaskHandle);
  DebugReportTaskCreation("sensorTask", sensorTaskHandle);
  DebugReportTaskCreation("inputTask", inputTaskHandle);
#if !TESTS
  DebugReportTaskCreation("viewPresenterTask", viewPresenterTaskHandle);
  DebugReportTaskCreation("systemTask", systemTaskHandle);
  DebugReportTaskCreation("maintenanceTask", maintenanceTaskHandle);
#endif
#endif

  printf("Maininit completed. Starting scheduler...\n");

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (0) {
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI1
                              |RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_10;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
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
   * Minimum measurement duration is 640.5/(64MHz/64) seconds with the current
   * ADC clock.
   */
  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
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
  hadc1.Init.Oversampling.RightBitShift = ADC_RIGHTBITSHIFT_8;
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
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

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
// Override the HAL_Delay to use it in ssd1306_Init() before the scheduler
// starts: The issue is that HAL_Delay relies on the HAL tick interrupt (TIM17)
// to increment uwTick. During initialization, before the scheduler starts or if
// interrupts  are not yet fully active/prioritized correctly, uwTick may not
// increment, causing HAL_Delay to hang in an infinite loop.
/**
 * @brief  This function provides minimum delay (in milliseconds) based
 *         on variable incremented.
 * @param  Delay  specifies the delay time length, in milliseconds.
 * @retval None
 */
void HAL_Delay(uint32_t Delay) {
  if (osKernelGetState() == osKernelRunning) {
    osDelay(Delay);
  } else {
    /* Busy wait */
    volatile uint32_t count = (SystemCoreClock / 4000) * Delay;
    while (count--) {
      __NOP();
    }
  }
}
/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Pointer to DefaultTaskArgsTypeDef containing task arguments
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN 5 */
#if TESTS
  DefaultTaskArgsTypeDef *args = (DefaultTaskArgsTypeDef *)argument;
#else
  (void)argument; /* Unused */
#endif

#if OS_TASKS_DEBUG
  printf("DefaultTask running (heap=%lu)\n",
         (unsigned long)xPortGetFreeHeapSize());
#endif

#if TESTS
#if DRIVER_TEST
  Driver_Test(args->storage2system_event_queue, args->input2vp_event_queue,
              args->config_access, args->sensor_values_access);
#elif ADAPTATION_TEST
  Adaptation_Test();
#endif
#else
  for (;;) {
    osDelay(pdMS_TO_TICKS(60000U));
  }
#endif
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
  while (1) {
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
  /* User can add his own implementation to report the file name and line
     number, ex: printf("Wrong parameters value: file %s on line %d\r\n", file,
     line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
