#include "main.h"
#include "cmsis_os.h"
#include "adc.h"
#include "can.h"
#include "crc.h"
#include "dma.h"
#include "i2c.h"
#include "usb_device.h"
#include "gpio.h"

#define BOOTLOADER_FLAG_ADDRESS 0x40024000   // Backup SRAM address to store bootloader flag
#define BOOTLOADER_MAGIC_CODE   0xDEADBEEF  // Magic code to indicate bootloader request

void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
void JumpToBootloader(void);
void CheckBootloaderFlag(void);

#if( configGENERATE_RUN_TIME_STATS == 1)
void IncrementRuntimeStats(void);
#endif

int main(void)
{
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  CheckBootloaderFlag();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_CRC_Init();
  MX_I2C1_Init();
  
  /* Init scheduler */
  osKernelInitialize();  /* Call init function for freertos objects (in freertos.c) */
  MX_FREERTOS_Init();

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  while (1)
  {
    
  }
 }

 void EnterStopMode(CanSpeed_t eSpeed)
 {
    //Set CAN TX/RX as GPIO event to enable wakeup from STOP mode
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_CAN1_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_8|GPIO_PIN_9);
    HAL_NVIC_DisableIRQ(CAN1_RX0_IRQn);

    GPIO_InitStruct.Pin = GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING_FALLING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING_FALLING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    //Enter Stop Mode
    HAL_SuspendTick();
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
    HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFE);

    //Execution will resume here after wakeup
    SystemClock_Config();
    HAL_ResumeTick();

    //Reconfigure CAN after wakeup from STOP mode  
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_8);
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_9);

    HAL_CAN_MspInit(&hcan1);
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
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 5;
  RCC_OscInitStruct.PLL.PLLN = 180;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 6;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler(FATAL_ERROR_RCC);
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV4;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler(FATAL_ERROR_RCC);
  }
}


/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }
#if( configGENERATE_RUN_TIME_STATS == 1)
    if (htim->Instance == TIM1) {
      IncrementRuntimeStats();
    }
#endif
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  eErrorCode: PdmFatalError_t error code
  * @retval None
  */
void Error_Handler(PdmFatalError_t eErrorCode)
{
  __disable_irq();
  FatalError(eErrorCode);
}

//=======================================================
// Jump to bootloader
//=======================================================

#define BOOT_ADDR  0x1FFF0000

struct boot_vectable_{
  uint32_t Initial_SP;
  void (*Reset_Handler)(void);
};

#define BOOTVTAB ((struct boot_vectable_*)BOOT_ADDR)
 void JumpToBootloader(void)
 {
    //Disable all interrupts
    __disable_irq();

    // Reset USB
    USB_OTG_FS->GOTGCTL |= USB_OTG_DCTL_SDIS;
    USB_OTG_FS->GRSTCTL |= USB_OTG_GRSTCTL_CSRST;

    //De-init all peripherals
    HAL_ADC_DeInit(&hadc1);
    HAL_CAN_DeInit(&hcan1);
    HAL_CRC_DeInit(&hcrc);
    HAL_I2C_DeInit(&hi2c1);
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_All);
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_All);
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_All);
    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_All);

    //Set the clock to the default state
    HAL_RCC_DeInit();

    //Disable systick timer
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;

    //Clear interrupt enable register and interrupt pending register
    for (uint8_t i = 0; i < sizeof(NVIC->ICER) / sizeof(NVIC->ICER[0]); i++)
    {
      NVIC->ICER[i] = 0xFFFFFFFF;
      NVIC->ICPR[i] = 0xFFFFFFFF;
    }

    //Remap system memory
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_SYSCFG_REMAPMEMORY_SYSTEMFLASH();

    //Re-enable all interrupts
    __enable_irq();

    //Set the MSP
    __set_PSP(BOOTVTAB->Initial_SP);
    __set_MSP(BOOTVTAB->Initial_SP);

    //Jump to bootloader
    BOOTVTAB->Reset_Handler();
 }

void RequestBootloader(void)
{
    __HAL_RCC_PWR_CLK_ENABLE();
    HAL_PWR_EnableBkUpAccess();
    __HAL_RCC_BKPSRAM_CLK_ENABLE();

    // Set the magic code in the reserved SRAM location
    *(volatile uint32_t*)BOOTLOADER_FLAG_ADDRESS = BOOTLOADER_MAGIC_CODE;

    // -Reset the microcontroller to start the bootloader on next boot
    // -Using this approach solves the issue of tracking down every possible 
    // peripheral that could be enabled and preventing entering the bootloader properly
    HAL_NVIC_SystemReset();
    
    // No further code will execute after this point
}

void CheckBootloaderFlag(void)
{
    // Enable access to Backup SRAM
    __HAL_RCC_PWR_CLK_ENABLE();
    HAL_PWR_EnableBkUpAccess();
    __HAL_RCC_BKPSRAM_CLK_ENABLE();
    
    if (*(volatile uint32_t*)BOOTLOADER_FLAG_ADDRESS == BOOTLOADER_MAGIC_CODE) {
        // Clear the flag
        *(volatile uint32_t*)BOOTLOADER_FLAG_ADDRESS = 0;
        
        // Enter bootloader
        JumpToBootloader();
    }
}


#ifdef  USE_FULL_ASSERT
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
