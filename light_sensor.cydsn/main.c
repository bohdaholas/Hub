/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include "project.h"
#include <stdio.h>

#define STATUS_COMPLETE 0x01
#define ADDR (0x23)
#define COMMAND (0x10)

char TEXT_BUFFER[32];

uint16 lightmeter_read();

int main(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */
   
    MyI2C_Start();
    UART_Start();
    UART_UartPutChar(0x0C);
    UART_UartPutString("Start measurement:\r\n");
    
    for(;;)
    {
        sprintf(TEXT_BUFFER, "Light is %u lux\r\n", lightmeter_read());    
        UART_UartPutString(TEXT_BUFFER);
        CyDelay(500);
    }
}

uint16 lightmeter_read() {
    // This code doesn't work!
    uint8 Write_Buf[1] = {0};
    Write_Buf[0] = COMMAND;

    uint8 Read_Buf[2] = {0, 0};

    MyI2C_I2CMasterWriteBuf(ADDR, (uint8 *) Write_Buf, 1, MyI2C_I2C_MODE_NO_STOP);
    while((MyI2C_I2CMasterStatus() & MyI2C_I2C_MSTAT_WR_CMPLT) == 0);
    CyDelay(180);
    MyI2C_I2CMasterReadBuf(ADDR, (uint8 *) Read_Buf, 2, MyI2C_I2C_MODE_REPEAT_START);
    while((MyI2C_I2CMasterStatus() & MyI2C_I2C_MSTAT_RD_CMPLT) == 0);

    uint8 high = Read_Buf[0];
    uint8 low = Read_Buf[1];
    uint16 result = ((high << 8) | low) / 1.2;

    return result;
}

/* [] END OF FILE */
