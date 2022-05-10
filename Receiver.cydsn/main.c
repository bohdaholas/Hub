#include "project.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define UART_BUFFLEN        (100u)
#define PAYLOAD_SIZE        32

volatile bool irq_flag = false;

uint8_t data[PAYLOAD_SIZE];
char UART_BUFFER[UART_BUFFLEN];

CY_ISR_PROTO(IRQ_Handler);

int main(void)
{ 
    CyGlobalIntEnable;
    isr_IRQ_StartEx(IRQ_Handler);
    
    UART_Start();
    UART_UartPutChar(0x0C);
    
    nRF24_start();
    const uint8_t RX_ADDR[5]= {0xBA, 0xAD, 0xC0, 0xFF, 0xEE};
    nRF24_set_rx_pipe_address(NRF_ADDR_PIPE0, RX_ADDR, 5);
    nRF24_start_listening();
    
    while (1) {
        
        UART_UartPutString("Waiting for data...\r\n");
        
        while(false == irq_flag);
            
        // Get and clear the flag that caused the IRQ interrupt,
        nrf_irq flag = nRF24_get_irq_flag();
        nRF24_clear_irq_flag(flag);

        nRF24_get_rx_payload(data, PAYLOAD_SIZE);

        // send data via UART
        UART_UartPutString("Received: ");
        sprintf(UART_BUFFER, "\r\n%s\r\n", data);
        UART_UartPutString(UART_BUFFER);
        irq_flag = false;
    }
}

CY_ISR(IRQ_Handler)
{
    irq_flag = true;
    IRQ_ClearInterrupt();
}

/* [] END OF FILE */
