/**
  ******************************************************************************
  * @file    DSCProject/main.c 
  * @author  Inz. Bartlomiej Kozdras
  * @version V1.1.0
  * @date    02-February-2015
  * @brief   DSC Application - Main
  ******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "System/SystemManager.h"
#include "HardwareSettings/Processor.h"

void SystemClock_Config(void);

int main(void)
{
  HAL_Init();

  SystemClock_Config();  
    
  SystemManager_run();
}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct;
    RCC_ClkInitTypeDef RCC_ClkInitStruct;

    __PWR_CLK_ENABLE();

    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    #ifdef STM32F401RE
        RCC_OscInitStruct.HSICalibrationValue = 6;
    #else
        RCC_OscInitStruct.HSICalibrationValue = 0x10;
    #endif
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM = 16;
    #ifdef STM32F401RE
        RCC_OscInitStruct.PLL.PLLN = 336;
    #else
        RCC_OscInitStruct.PLL.PLLN = 400;
    #endif
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
    RCC_OscInitStruct.PLL.PLLQ = 7;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    #ifdef STM32F401RE
        RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1;
    #else
        RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
    #endif
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    #ifdef STM32F401RE
        HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
    #else
        HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3);
    #endif
}
