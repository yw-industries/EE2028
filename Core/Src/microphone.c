
#include "microphone.h"

static DFSDM_Channel_HandleTypeDef hchannel;
static DFSDM_Filter_HandleTypeDef hfilter;
static DMA_HandleTypeDef hdma_dfsdm;

volatile uint8_t microphone_done = 0;
volatile uint8_t microphone_error = 0;

HAL_StatusTypeDef Microphone_Init(void)
{
    GPIO_InitTypeDef gpio = {0};

    /* Enable peripheral clocks */
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_DFSDM1_CLK_ENABLE();
    __HAL_RCC_DMA1_CLK_ENABLE();
    __HAL_RCC_DMAMUX1_CLK_ENABLE();

    /* PE7: PDM data, PE9: microphone clock */
    gpio.Pin = GPIO_PIN_7 | GPIO_PIN_9;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio.Alternate = GPIO_AF6_DFSDM1;

    HAL_GPIO_Init(GPIOE, &gpio);

    /* Configure DFSDM Channel 2 */
    hchannel.Instance = DFSDM1_Channel2;

    hchannel.Init.OutputClock.Activation = ENABLE;
    hchannel.Init.OutputClock.Selection =
        DFSDM_CHANNEL_OUTPUT_CLOCK_SYSTEM;
    hchannel.Init.OutputClock.Divider = 2;

    hchannel.Init.Input.Multiplexer =
        DFSDM_CHANNEL_EXTERNAL_INPUTS;
    hchannel.Init.Input.DataPacking =
        DFSDM_CHANNEL_STANDARD_MODE;
    hchannel.Init.Input.Pins =
        DFSDM_CHANNEL_SAME_CHANNEL_PINS;

    hchannel.Init.SerialInterface.Type =
        DFSDM_CHANNEL_SPI_RISING;
    hchannel.Init.SerialInterface.SpiClock =
        DFSDM_CHANNEL_SPI_CLOCK_INTERNAL;

    hchannel.Init.Awd.FilterOrder =
        DFSDM_CHANNEL_FASTSINC_ORDER;
    hchannel.Init.Awd.Oversampling = 1;

    hchannel.Init.Offset = 0;
    hchannel.Init.RightBitShift = 0;

    if (HAL_DFSDM_ChannelInit(&hchannel) != HAL_OK) { 
        return HAL_ERROR;
    }

    /* Enable the DFSDM peripheral globally */
    SET_BIT(
        DFSDM1_Channel0->CHCFGR1,
        DFSDM_CHCFGR1_DFSDMEN
    );

    /* Configure DMA1 Channel 4 */
    hdma_dfsdm.Instance = DMA1_Channel4;
    hdma_dfsdm.Init.Request =
        DMA_REQUEST_DFSDM1_FLT0;
    
    hdma_dfsdm.Init.Direction =
        DMA_PERIPH_TO_MEMORY;
    hdma_dfsdm.Init.PeriphInc =
        DMA_PINC_DISABLE;
    hdma_dfsdm.Init.MemInc =
        DMA_MINC_ENABLE;
    hdma_dfsdm.Init.PeriphDataAlignment =
        DMA_PDATAALIGN_WORD;
    hdma_dfsdm.Init.MemDataAlignment =
        DMA_MDATAALIGN_WORD;
    hdma_dfsdm.Init.Mode = DMA_NORMAL;
    hdma_dfsdm.Init.Priority = DMA_PRIORITY_HIGH;

    HAL_StatusTypeDef dma_status = HAL_DMA_Init(&hdma_dfsdm);


    if (dma_status != HAL_OK)
    {
        return HAL_ERROR;
    }

    /* Configure DFSDM Filter 0 */
    hfilter.Instance = DFSDM1_Filter0;

    hfilter.Init.RegularParam.Trigger =
        DFSDM_FILTER_SW_TRIGGER;
    hfilter.Init.RegularParam.FastMode = DISABLE;
    hfilter.Init.RegularParam.DmaMode = ENABLE;

    hfilter.Init.InjectedParam.Trigger =
        DFSDM_FILTER_SW_TRIGGER;
    hfilter.Init.InjectedParam.ScanMode = DISABLE;
    hfilter.Init.InjectedParam.DmaMode = DISABLE;
    hfilter.Init.InjectedParam.ExtTrigger =
        DFSDM_FILTER_EXT_TRIG_TIM1_TRGO;
    hfilter.Init.InjectedParam.ExtTriggerEdge =
        DFSDM_FILTER_EXT_TRIG_RISING_EDGE;

    hfilter.Init.FilterParam.SincOrder =
        DFSDM_FILTER_SINC3_ORDER;
    hfilter.Init.FilterParam.Oversampling = 64;
    hfilter.Init.FilterParam.IntOversampling = 1;

    if (HAL_DFSDM_FilterInit(&hfilter) != HAL_OK)
        return HAL_ERROR;

    /* Connect Channel 2 to Filter 0 */
    if (HAL_DFSDM_FilterConfigRegChannel(
        &hfilter,
        DFSDM_CHANNEL_2,
        DFSDM_CONTINUOUS_CONV_ON
    ) != HAL_OK)
        return HAL_ERROR;

    /* Connect DMA handle to filter */
    __HAL_LINKDMA(&hfilter, hdmaReg, hdma_dfsdm);

    HAL_NVIC_SetPriority(DMA1_Channel4_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel4_IRQn);

    return HAL_OK;
}

HAL_StatusTypeDef Microphone_Start(
    int32_t *buffer,
    uint32_t length)
{
    microphone_done = 0;
    microphone_error = 0;

    return HAL_DFSDM_FilterRegularStart_DMA(
        &hfilter, buffer, length
    );
}

HAL_StatusTypeDef Microphone_Stop(void)
{
    return HAL_DFSDM_FilterRegularStop_DMA(&hfilter);
}

/* DMA interrupt handler */
void DMA1_Channel4_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&hdma_dfsdm);
}

/* Called when the recording buffer is full */
void HAL_DFSDM_FilterRegConvCpltCallback(
    DFSDM_Filter_HandleTypeDef *filter)
{
    if (filter == &hfilter)
        microphone_done = 1;
}

/* Called on a DFSDM */
void HAL_DFSDM_FilterErrorCallback(
    DFSDM_Filter_HandleTypeDef *filter)
{
    if (filter == &hfilter)
        microphone_error = 1;
}
