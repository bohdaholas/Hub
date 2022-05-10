#include "project.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define ADC_CH_TEMP         (0x02u)
#define ADC_INJ_PERIOD      (5u)
#define DIETEMP_VREF_MV_VALUE   (1024)
#define UART_BUFFLEN        (100u)

volatile bool irq_flag = false;
char UART_BUFFER[UART_BUFFLEN];

CY_ISR_PROTO(IRQ_Handler);
CY_ISR_PROTO(Isr_Timer_Handler);

int main(void)
{
    CyGlobalIntEnable;
    
    UART_Start();
    UART_UartPutChar(0x0C);
    
    int16 adcResult[ADC_SAR_Seq_TOTAL_CHANNELS_NUM] = {0};
    int16 voltage[ADC_SAR_Seq_SEQUENCED_CHANNELS_NUM] = {0};
    int32 temperature = 0;
    int16 ADCCountsCorrected = 0;
    uint16 i = 0;
    
    ADC_SAR_Seq_Start();
    Timer_Start();
    Isr_Timer_StartEx(Isr_Timer_Handler);
    
    nRF24_start();
    isr_IRQ_StartEx(IRQ_Handler);
    const uint8_t TX_ADDR[5]= {0xBA, 0xAD, 0xC0, 0xFF, 0xEE};
    nRF24_set_rx_pipe_address(NRF_ADDR_PIPE0, TX_ADDR, 5);
    // set tx pipe address to match the receiver address
    nRF24_set_tx_address(TX_ADDR, 5);
    
    while (1) {
        if(0u != ADC_SAR_Seq_IsEndConversion(ADC_SAR_Seq_RETURN_STATUS)) {
            for(i = 0; i < ADC_SAR_Seq_SEQUENCED_CHANNELS_NUM; i++) {
                adcResult[i] = ADC_SAR_Seq_GetResult16(i);
                voltage[i] = ADC_SAR_Seq_CountsTo_mVolts(i, adcResult[i]);
            }
        }
        
        /* When conversion of the injection channel has completed */
        if(0u != ADC_SAR_Seq_IsEndConversion(ADC_SAR_Seq_RETURN_STATUS_INJ)) {
            adcResult[ADC_CH_TEMP] = ADC_SAR_Seq_GetResult16(ADC_CH_TEMP);
            
            // Adjust data from ADC with respect to ADC Vref value
            ADCCountsCorrected = 
           (int16)(((int32)adcResult[ADC_CH_TEMP] * ADC_SAR_Seq_DEFAULT_VREF_MV_VALUE) / 
                    DIETEMP_VREF_MV_VALUE);
            
            temperature = DieTemp_CountsTo_Celsius(ADCCountsCorrected);
        }
          
        UART_UartPutString(UART_BUFFER);
        UART_UartPutString("\r\nSending data...\r\n");
        char strTemp[32];
        itoa(temperature, strTemp, 10);
        sprintf(UART_BUFFER, "\r\n%s", strTemp);  
        nRF24_transmit((const uint8_t *) strTemp, 32);
        
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
        
        CyDelay(250);
      
    }
}

CY_ISR(IRQ_Handler) {
    irq_flag = true;
    IRQ_ClearInterrupt();
}

CY_ISR(Isr_Timer_Handler) {
    static uint32 injTimer = 0u;
    
    injTimer++;
    
    if(injTimer >= ADC_INJ_PERIOD) {
        injTimer = 0u;
        // Enable the injection channel for the next scan */
        ADC_SAR_Seq_EnableInjection();
    }
    
    ADC_SAR_Seq_StartConvert();

    Timer_ClearInterrupt(Timer_INTR_MASK_TC);
}

/* [] END OF FILE */
