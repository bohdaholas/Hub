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

// the IRQ pin triggered an interrupt
CY_ISR_PROTO(IRQ_Handler);

int main(void)
{
    const uint8_t TX_ADDR[5]= {0xBA, 0xAD, 0xC0, 0xFF, 0xEE};
    unsigned char data[] = "This is my data";
    
    // Set the interrupts handlers
    isr_IRQ_StartEx(IRQ_Handler);
    
    CyGlobalIntEnable;
    
    UART_Start();
    UART_UartPutChar(0x0C);
    UART_UartPutString("Basic project: Tx\r\n");
    
    nRF24_start();
    nRF24_set_rx_pipe_address(NRF_ADDR_PIPE0, TX_ADDR, 5);
    // set tx pipe address to match the receiver address
    nRF24_set_tx_address(TX_ADDR, 5);
    
    int counter = 0;
    
    while (1) {
        UART_UartPutString("\r\nSending letter ");
        sprintf(TEXT_BUFFER, "\r\n%s", data);  
        UART_UartPutString(TEXT_BUFFER);
        nRF24_transmit(data, sizeof(data));
        
        while(false == irq_flag);
            
        // Get and clear the flag that caused the IRQ interrupt,
        // in this project the only IRQ cause is the caused by
        // transmitting data (NRF_STATUS_TX_DS_MASK) or timeout
        nrf_irq flag = nRF24_get_irq_flag();
        
        switch (flag) {
        case NRF_TX_DS_IRQ:
            // turn on the Green LED if the transmit was successfull
            LED_Write(0);
            break;
        case NRF_MAX_RT_IRQ:
            // turn off the Green LED if the transmit was not successfull
            LED_Write(1);
            break;
        default:
            break;
        }
        
        nRF24_clear_irq_flag(flag);
        
        irq_flag = false;
        
        CyDelay(250);
      
    }
}

CY_ISR(IRQ_Handler)
{
    irq_flag = true;
    IRQ_ClearInterrupt();
}

/* [] END OF FILE */
