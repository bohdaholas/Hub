/*
 * 01_Basic_Tx
 * 
 * The nrf24 radio is configured to transmit 1 byte payload.
 */

#include "project.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdio.h>

volatile bool irq_flag = false;
char TEXT_BUFFER[32];

CY_ISR_PROTO(nrf_IRQ_Handler);
CY_ISR_PROTO(timer_IRQ_Handler);

int main(void)
{
    Timer_Start();
    CyGlobalIntEnable;
    isr_nrf_IRQ_StartEx(nrf_IRQ_Handler);
    isr_Timer_StartEx(timer_IRQ_Handler);
    
    UART_Start();
    UART_UartPutChar(0x0C);
    
    nRF24_start();
    const uint8_t TX_ADDR[5]= {0x78, 0x78, 0x78, 0x78, 0x78};
    nRF24_set_rx_pipe_address(NRF_ADDR_PIPE0, TX_ADDR, 5);
    // set tx pipe address to match the receiver address
    nRF24_set_tx_address(TX_ADDR, 5);
    
    unsigned char data[] = "RAW data";
    
    while (1) {
        UART_UartPutString("\r\nSending data...\r\n");
        sprintf(TEXT_BUFFER, "\r\n%s", data);  
        UART_UartPutString(TEXT_BUFFER);
        nRF24_transmit(data, 32);
        
        while(false == irq_flag);

        nrf_irq flag = nRF24_get_irq_flag();
        switch (flag) {
        case NRF_TX_DS_IRQ:
            // Acknowledge packet is received
            LED_Write(0);
            break;
        case NRF_MAX_RT_IRQ:
            // Maximum number of retransmits exceeded
            LED_Write(1);
            break;
        default:
            break;
        }
        nRF24_clear_irq_flag(flag);

        irq_flag = false;
        
        CySysPmSleep();
    }
}

CY_ISR(nrf_IRQ_Handler)
{
    irq_flag = true;
    IRQ_ClearInterrupt();
}

CY_ISR(timer_IRQ_Handler) {
    Timer_ClearInterrupt(Timer_INTR_MASK_TC);
}

/* [] END OF FILE */
