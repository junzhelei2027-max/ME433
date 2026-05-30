/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    main.c
  * @brief   STM32 UART bridge: PC VCP (USART2) <-> Pico (USART1)
  *
  * Wiring:
  *   STM32 PA0 (USART1 TX) ----> Pico GP1 (UART0 RX)
  *   STM32 PA1 (USART1 RX) <---- Pico GP0 (UART0 TX)
  *   GND ---------------------- GND
  ******************************************************************************
  */
/* USER CODE END Header */

#include "main.h"
#include <string.h>

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart1;   // USART1: link to Pico
UART_HandleTypeDef huart2;   // USART2: PC virtual COM port

/* USER CODE BEGIN PV */

#define QUEUE_LEN 64

typedef struct {
    uint8_t data[QUEUE_LEN];
    volatile uint8_t write;
    volatile uint8_t read;
} ByteQueue;

static uint8_t from_pc_byte;
static uint8_t from_pico_byte;

static ByteQueue pc_to_pico = {0};
static ByteQueue pico_to_pc = {0};

static volatile uint8_t usart1_sending = 0;
static volatile uint8_t usart2_sending = 0;

static uint8_t usart1_out_byte;
static uint8_t usart2_out_byte;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
static void service_usart1_tx(void);
static void service_usart2_tx(void);

/* USER CODE BEGIN 0 */

static inline uint8_t queue_put(ByteQueue *q, uint8_t value)
{
    uint8_t next = (uint8_t)((q->write + 1u) % QUEUE_LEN);

    if (next == q->read) {
        return 0;       // queue full, drop this byte
    }

    q->data[q->write] = value;
    q->write = next;
    return 1;
}

static inline uint8_t queue_get(ByteQueue *q, uint8_t *value)
{
    if (q->write == q->read) {
        return 0;       // queue empty
    }

    *value = q->data[q->read];
    q->read = (uint8_t)((q->read + 1u) % QUEUE_LEN);
    return 1;
}

static void service_usart1_tx(void)
{
    if (usart1_sending) {
        return;
    }

    if (queue_get(&pc_to_pico, &usart1_out_byte)) {
        usart1_sending = 1;
        HAL_UART_Transmit_IT(&huart1, &usart1_out_byte, 1);
    }
}

static void service_usart2_tx(void)
{
    if (usart2_sending) {
        return;
    }

    if (queue_get(&pico_to_pc, &usart2_out_byte)) {
        usart2_sending = 1;
        HAL_UART_Transmit_IT(&huart2, &usart2_out_byte, 1);
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2) {
        queue_put(&pc_to_pico, from_pc_byte);
        HAL_UART_Receive_IT(&huart2, &from_pc_byte, 1);
        service_usart1_tx();
    }

    if (huart->Instance == USART1) {
        queue_put(&pico_to_pc, from_pico_byte);
        HAL_UART_Receive_IT(&huart1, &from_pico_byte, 1);
        service_usart2_tx();
    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) {
        usart1_sending = 0;
        service_usart1_tx();
    }

    if (huart->Instance == USART2) {
        usart2_sending = 0;
        service_usart2_tx();
    }
}

/* USER CODE END 0 */

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART1_UART_Init();
    MX_USART2_UART_Init();

    /* USER CODE BEGIN 2 */
    BSP_LED_Init(LED_GREEN);
    BSP_LED_Init(LED_BLUE);
    BSP_PB_Init(BUTTON_USER, BUTTON_MODE_EXTI);
    BSP_LED_On(LED_GREEN);

    HAL_UART_Receive_IT(&huart2, &from_pc_byte, 1);
    HAL_UART_Receive_IT(&huart1, &from_pico_byte, 1);
    /* USER CODE END 2 */

    while (1)
    {
        service_usart1_tx();
        service_usart2_tx();

        BSP_LED_Toggle(LED_BLUE);
        HAL_Delay(500);
    }
}

/* System Clock --------------------------------------------------------------*/
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    __HAL_FLASH_SET_LATENCY(FLASH_LATENCY_0);
    RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState            = RCC_HSI_ON;
    RCC_OscInitStruct.HSIDiv              = RCC_HSI_DIV4;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) Error_Handler();

    RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                     | RCC_CLOCKTYPE_PCLK1;
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_HSI;
    RCC_ClkInitStruct.SYSCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV1;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) Error_Handler();
}

/* USART1: Pico link, PA0=TX PA1=RX AF4 -------------------------------------*/
static void MX_USART1_UART_Init(void)
{
    huart1.Instance                    = USART1;
    huart1.Init.BaudRate               = 115200;
    huart1.Init.WordLength             = UART_WORDLENGTH_8B;
    huart1.Init.StopBits               = UART_STOPBITS_1;
    huart1.Init.Parity                 = UART_PARITY_NONE;
    huart1.Init.Mode                   = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl              = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling           = UART_OVERSAMPLING_16;
    huart1.Init.OneBitSampling         = UART_ONE_BIT_SAMPLE_DISABLE;
    huart1.Init.ClockPrescaler         = UART_PRESCALER_DIV1;
    huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    if (HAL_UART_Init(&huart1) != HAL_OK) Error_Handler();
    if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK) Error_Handler();
    if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK) Error_Handler();
    if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK) Error_Handler();
}

/* USART2: PC VCP, PA2=TX PA3=RX AF1 ----------------------------------------*/
static void MX_USART2_UART_Init(void)
{
    huart2.Instance                    = USART2;
    huart2.Init.BaudRate               = 115200;
    huart2.Init.WordLength             = UART_WORDLENGTH_8B;
    huart2.Init.StopBits               = UART_STOPBITS_1;
    huart2.Init.Parity                 = UART_PARITY_NONE;
    huart2.Init.Mode                   = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl              = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling           = UART_OVERSAMPLING_16;
    huart2.Init.OneBitSampling         = UART_ONE_BIT_SAMPLE_DISABLE;
    huart2.Init.ClockPrescaler         = UART_PRESCALER_DIV1;
    huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    if (HAL_UART_Init(&huart2) != HAL_OK) Error_Handler();
    if (HAL_UARTEx_SetTxFifoThreshold(&huart2, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK) Error_Handler();
    if (HAL_UARTEx_SetRxFifoThreshold(&huart2, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK) Error_Handler();
    if (HAL_UARTEx_DisableFifoMode(&huart2) != HAL_OK) Error_Handler();
}

/* GPIO ----------------------------------------------------------------------*/
static void MX_GPIO_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
}

/* Error Handler -------------------------------------------------------------*/
void Error_Handler(void)
{
    __disable_irq();

    while (1)
    {
        BSP_LED_Toggle(LED_GREEN);
        HAL_Delay(100);
    }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line) {}
#endif
