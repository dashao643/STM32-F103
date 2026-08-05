



typedef struct {
    volatile bool rxFlag;                       // 接收到数据标志
    uint8_t rxBuf[UART1_RX_BUFF_MAX_SIZE];      // 接收缓冲区
    volatile uint16_t rxTail;                   // 缓冲区尾指针
} UART1_RxTypeDef;