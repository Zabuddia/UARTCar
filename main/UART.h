#ifndef __UART_H__
#define __UART_H__

#include <string.h>
#include "driver/uart.h"
#include "esp_log.h"

extern int counter;

void uart_init();
void uart_send_data(const char* data);
void uart_read_data(char* data, int max_len);

#endif