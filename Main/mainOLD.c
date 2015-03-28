/**
  ******************************************************************************
  * @file    DSCProject/Src/main.c 
  * @author  Inz. Bartlomiej Kozdras
  * @version V1.1.0
  * @date    02-February-2015
  * @brief   DSC Application - Main
  ******************************************************************************
*/
/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "stm32f4xx_nucleo.h"
#include "Utilities/Logger/Logger.h"
#include "FaultManagement/ErrorHandlers.h"

#include "cmsis_os.h"

static void SystemClock_Config(void);

static void exampleThread(void const* arg);
osTimerId exampleThreadId;

static void mainThread(void const* arg);
osThreadId threadId;

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
  /* STM32F4xx HAL library initialization:
       - Configure the Flash prefetch, instruction and Data caches
       - Configure the Systick to generate an interrupt each 1 msec
       - Set NVIC Group Priority to 4
       - Global MSP (MCU Support Package) initialization
     */
  HAL_Init();
  
  /* Configure the system clock to 84 MHz */
  SystemClock_Config();
  
  osThreadDef(Example, mainThread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
  threadId = osThreadCreate(osThread(Example), NULL);
    
  osKernelStart();
    
  /* Infinite loop */ 
  while (1)
  {
  }
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSI)
  *            SYSCLK(Hz)                     = 84000000
  *            HCLK(Hz)                       = 84000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 2
  *            APB2 Prescaler                 = 1
  *            HSI Frequency(Hz)              = 16000000
  *            PLL_M                          = 16
  *            PLL_N                          = 336
  *            PLL_P                          = 4
  *            PLL_Q                          = 7
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale2 mode
  *            Flash Latency(WS)              = 2
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;

  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();
  
  /* The voltage scaling allows optimizing the power consumption when the device is 
     clocked below the maximum system frequency, to update the voltage scaling value 
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);
  
  /* Enable HSI Oscillator and activate PLL with HSI as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 0x10;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    ErrorHandler_simpleHandler();
  }
  
  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
  //RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;  
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    ErrorHandler_simpleHandler();
  }
}

void exampleThread(void const* arg)
{
    static int counter = 0;
    
    Logger_debug("JEST: %d!", ++counter);
    osDelay(1000);
}

void mainThread(void const* arg)
{
    osTimerDef(Thread1, exampleThread);
    Logger_initialize(ELoggerType_EvalCOM1, ELoggerLevel_Debug);
    Logger_debug("MAIN: %d!", 50);
    exampleThreadId = osTimerCreate( osTimer(Thread1), osTimerPeriodic, NULL );

    osStatus status;
    
    status = osTimerStart(exampleThreadId, 100);
    
    if (osOK == status)
    {
        Logger_debug("Timer created.");
    }
    else
    {
        Logger_error("NOT CREATED");
    }
 
    Logger_debug("Systick: %u.", osKernelSysTick());
    uint32_t counter = osKernelSysTick();
    Logger_debug("Counter: %u.", osKernelSysTick());
    Logger_debug("Running: %u.", osKernelRunning());
    osDelay(osWaitForever);
}

/**
  * @}
  */ 

/**
  * @}
  */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
