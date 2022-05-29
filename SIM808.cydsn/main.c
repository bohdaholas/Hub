#include "project.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdio.h>

#define DELAY_MS 500

const uint8_t APN[]  = "internet";
const uint8_t FIREBASE_HOST[]  = "https://hubgprs-e6a18-default-rtdb.firebaseio.com/";
const uint8_t FIREBASE_SECRET[]  = "Q5dK5otO6zeg3vYLMwf3iwpnaCTjBwOJrMboqkby";
char buff[100];

void init_gprs();
void http_post(char *dir, char *data, int length);
void waitResponse(const char *expected_answer);

int main(void)
{
    CyGlobalIntEnable;

    UART_Start();
    SIM800L_Start();
    UART_UartPutChar(0x0C);
    UART_UartPutString("Initializing SIM800...\n\r");

    init_gprs();
    
    while(1) {
        char data[] = "56666";
        http_post("temperature", data, sizeof(data));
        CyDelay(2000);   
    }
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
   
    sprintf(buff, "AT+HTTPDATA=%d,10000\n", length );
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
    
    UART_UartPutString(response);
    UART_UartPutString("\n\r");
}

/* [] END OF FILE */
