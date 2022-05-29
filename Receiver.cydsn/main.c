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

enum Pipes {
    TEMPERATURE_SENSOR = 0,
    LIGHT_SENSOR = 1
};

int main(void)
{ 
    CyGlobalIntEnable;
    isr_IRQ_StartEx(IRQ_Handler);
    
    UART_Start();
    UART_UartPutChar(0x0C);

    nRF24_start();
    const uint8_t RX_ADDR0[5]= {0xBA, 0xAD, 0xC0, 0xFF, 0xEE};
    const uint8_t RX_ADDR1[5]= {0x78, 0x78, 0x78, 0x78, 0x78};
    nRF24_set_rx_pipe_address(NRF_ADDR_PIPE0, RX_ADDR0, 5);
    nRF24_set_rx_pipe_address(NRF_ADDR_PIPE1, RX_ADDR1, 5);
    nRF24_start_listening();
    
    while (1) {
        
        UART_UartPutString("\r\nWaiting for data...\r\n");
        
        CySysPmSleep();
            
        // Get and clear the flag that caused the IRQ interrupt,
        nrf_irq flag = nRF24_get_irq_flag();
        nRF24_clear_irq_flag(flag);
        
        uint8_t pipe = nRF24_get_data_pipe_with_payload();
        if (pipe == TEMPERATURE_SENSOR) {
            sprintf(UART_BUFFER, "\r\nGot data from temperature pipe!\r\n");
        }
        if (pipe == LIGHT_SENSOR) {
            sprintf(UART_BUFFER, "\r\nGot data from light sensor pipe!\r\n");
        }
        UART_UartPutString(UART_BUFFER);
        
        nRF24_get_rx_payload(data, PAYLOAD_SIZE);
        sprintf(UART_BUFFER, "Received: %s\r\n", data);
        UART_UartPutString(UART_BUFFER);
        
        irq_flag = false;
    }
}

CY_ISR(IRQ_Handler)
{
    IRQ_ClearInterrupt();
}

/* [] END OF FILE */
