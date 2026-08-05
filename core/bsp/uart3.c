

// 绕回缓冲区
static void idleProcess(void)
{
    if (__HAL_UART_GET_FLAG(&uart1, UART_FLAG_IDLE) == SET) {
        __HAL_UART_CLEAR_IDLEFLAG(&uart1);

        uint16_t dmaRemain = __HAL_DMA_GET_COUNTER(uart1.hdmarx);
        rx->rxSize = segmentSize - remaining;

    }
}