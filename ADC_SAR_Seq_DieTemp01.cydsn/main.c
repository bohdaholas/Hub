#include "project.h"
#include <stdio.h>

#define ADC_CH_TEMP         (0x02u)
#define ADC_INJ_PERIOD      (5u)
#define DIETEMP_VREF_MV_VALUE   (1024)
#define UART_BUFFLEN        (100u)

//  Handles the Interrupt Service Routine for the Timer Component.
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

int main()
{
    int16 adcResult[ADC_SAR_Seq_TOTAL_CHANNELS_NUM] = {0};
    int16 voltage[ADC_SAR_Seq_SEQUENCED_CHANNELS_NUM] = {0};
    int32 temperature = 0;
    int16 ADCCountsCorrected = 0;
    uint16 i = 0;
    char  uartBuff[UART_BUFFLEN];

    CyGlobalIntEnable; 

    UART_Start();
    ADC_SAR_Seq_Start();
    Timer_Start();
    Isr_Timer_StartEx(Isr_Timer_Handler);
    
    UART_PutString("Starting measurements...\r\n\r\n");
    
    for(;;)
    {
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
            
            sprintf(uartBuff, "Temperature value: %liC \r\n\r\n", temperature);
            UART_PutString(uartBuff);
        }
    }
}

