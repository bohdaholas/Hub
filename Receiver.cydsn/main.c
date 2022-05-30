#include "project.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define UART_BUFFLEN        (100u)
#define PAYLOAD_SIZE        32
#define DELAY_MS 500

const uint8_t APN[]  = "internet";
const uint8_t FIREBASE_HOST[]  = "https://hubgprs-e6a18-default-rtdb.firebaseio.com/";
const uint8_t FIREBASE_SECRET[]  = "Q5dK5otO6zeg3vYLMwf3iwpnaCTjBwOJrMboqkby";
char buff[100];

void init_gprs();
void http_post(char *dir, char *data, int length);
void waitResponse(const char *expected_answer);

volatile bool irq_flag = false;

uint8_t data[PAYLOAD_SIZE];
char UART_BUFFER[UART_BUFFLEN];

CY_ISR_PROTO(IRQ_Handler);

enum Pipes {
    TRANSMITTER1 = 0,
    TRANSMITTER2 = 1
};

int main(void)
{ 
    CyGlobalIntEnable;
    isr_IRQ_StartEx(IRQ_Handler);
    
    SIM800L_Start();
    init_gprs();

    nRF24_start();
    const uint8_t RX_ADDR0[5]= {0xBA, 0xAD, 0xC0, 0xFF, 0xEE};
    const uint8_t RX_ADDR1[5]= {0x78, 0x78, 0x78, 0x78, 0x78};
    nRF24_set_rx_pipe_address(NRF_ADDR_PIPE0, RX_ADDR0, 5);
    nRF24_set_rx_pipe_address(NRF_ADDR_PIPE1, RX_ADDR1, 5);
    nRF24_start_listening();
    
    while (1) {
        CySysPmSleep();
            
        // Get and clear the flag that caused the IRQ interrupt,
        nrf_irq flag = nRF24_get_irq_flag();
        nRF24_clear_irq_flag(flag);
        
        uint8_t pipe = nRF24_get_data_pipe_with_payload();
        nRF24_get_rx_payload(data, PAYLOAD_SIZE);
        char thisData[32];
        strcpy(thisData, (char *) data);
        
        if (pipe == TRANSMITTER1) {
            http_post("transmitor1", thisData, strlen(thisData));
        }
        if (pipe == TRANSMITTER2) {
            http_post("transmitor2", thisData, strlen(thisData));
        }    
    }
}

CY_ISR(IRQ_Handler)
{
    IRQ_ClearInterrupt();
}

void init_gprs() {
    SIM800L_UartPutString("AT\n");
    waitResponse("OK");
    CyDelay(1000);
    
    SIM800L_UartPutString("AT+SAPBR=3,1,\"Contype\",\"GPRS\"\n");
    waitResponse("OK");
    CyDelay(DELAY_MS);
    
    sprintf(buff, "AT+SAPBR=3,1,\"APN\",%s\n", APN);
    SIM800L_UartPutString(buff);
    waitResponse("OK");
    CyDelay(DELAY_MS);
}

void http_post(char *dir, char *data, int length) {
    SIM800L_UartPutString("AT+SAPBR=1,1\n");
    waitResponse("OK");
    CyDelay(DELAY_MS);
    
    SIM800L_UartPutString("AT+HTTPINIT\n");
    waitResponse("OK");
    CyDelay(DELAY_MS);
    
    SIM800L_UartPutString("AT+HTTPSSL=1\n");
    waitResponse("OK");
    CyDelay(DELAY_MS);
    
    SIM800L_UartPutString("AT+HTTPPARA=\"CID\",1\n");
    waitResponse("OK");
    CyDelay(DELAY_MS);
    
    sprintf(buff, "AT+HTTPPARA=URL,%s%s/.json?auth=%s\n", FIREBASE_HOST, dir, FIREBASE_SECRET);
    SIM800L_UartPutString(buff);
    waitResponse("OK");
    CyDelay(1000);
    
    SIM800L_UartPutString("AT+HTTPPARA=\"REDIR\",1\n");
    waitResponse("OK");
    CyDelay(DELAY_MS);
    
    SIM800L_UartPutString("AT+HTTPPARA=\"CONTENT\",\"application/json\"\n");
    waitResponse("OK");
    CyDelay(DELAY_MS);
   
    sprintf(buff, "AT+HTTPDATA=%d,10000\n", length);
    SIM800L_UartPutString(buff);
    waitResponse("DOWNLOAD");
    CyDelay(1000);
    
    SIM800L_UartPutString(data);
    waitResponse("OK");
    CyDelay(1000);
    
    SIM800L_UartPutString("AT+HTTPACTION=1\n");
    waitResponse("OK");
    CyDelay(10000);
   
    SIM800L_UartPutString("AT+HTTPTERM\n");
    waitResponse("OK");
    CyDelay(DELAY_MS);
    
    SIM800L_UartPutString("AT+SAPBR=1,0\n");
    waitResponse("OK");
    CyDelay(DELAY_MS);
    
}

void waitResponse(const char *expected_answer) {
    char response[500] = "";
    char rxData;
    bool answer = 0;
    bool error = 0;
    
    do {
        rxData = SIM800L_UartGetChar();
    } while (rxData);
    
    do {
        rxData = SIM800L_UartGetChar();
        if (rxData) {
            strncat(response, &rxData, 1);
            char *found = strstr(response, expected_answer);
            if(found != NULL){
                answer = 1;
            }
            found = strstr(response, "ERROR");
            if(found != NULL){
                error = 1;
            }
        }
            
    } while((answer == 0) && (error == 0));
    
}

/* [] END OF FILE */
