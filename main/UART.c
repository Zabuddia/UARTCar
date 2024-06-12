#include "UART.h"

#define UART_PORT UART_NUM_2
#define UART_BAUD_RATE 9600
#define UART_TX_PIN 16
#define UART_RX_PIN 17

#define BUF_SIZE 1024
#define RD_BUF_SIZE (BUF_SIZE)

static char uart_buffer[BUF_SIZE];
static int uart_buffer_index = 0;
int counter = 0;

static const char *TAG = "UART";

void uart_init() {
	uart_config_t uart_config = {
		.baud_rate = UART_BAUD_RATE,
		.data_bits = UART_DATA_8_BITS,
		.parity = UART_PARITY_DISABLE,
		.stop_bits = UART_STOP_BITS_1,
		.flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
	};
	ESP_ERROR_CHECK(uart_param_config(UART_PORT, &uart_config));
	ESP_ERROR_CHECK(uart_set_pin(UART_PORT, UART_TX_PIN, UART_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
	QueueHandle_t uart_queue;
	ESP_ERROR_CHECK(uart_driver_install(UART_PORT, 1024 * 2, 1024 * 2, 10, &uart_queue, 0));
	//ESP_ERROR_CHECK(uart_driver_install(UART_PORT, 1024 * 2, 1024 * 2, 0, NULL, 0));
}

void uart_send_data(const char* data) {
    const int len = strlen(data);
    uart_write_bytes(UART_PORT, data, len);
}



void uart_read_data(char* data, int max_len) {
    int length = 0;
    ESP_ERROR_CHECK(uart_get_buffered_data_len(UART_PORT, (size_t*)&length));

    if (length > 0) {
        int len = uart_read_bytes(UART_PORT, (uint8_t*)&uart_buffer[uart_buffer_index], length, 100);
        if (len > 0) {
            uart_buffer_index += len;
            uart_buffer[uart_buffer_index] = '\0'; // Null-terminate the buffer

            // Check if there is a newline character in the buffer
            char* newline_pos = strrchr(uart_buffer, '\n');
            if (newline_pos != NULL) {
                // Null-terminate at the newline position
                *newline_pos = '\0';

                // Copy the latest complete number to the data buffer
                char* latest_number = strrchr(uart_buffer, '\n');
                if (latest_number == NULL) {
                    latest_number = uart_buffer;
                } else {
                    latest_number += 1; // Move past the previous newline
                }

                strncpy(data, latest_number, max_len - 1);
                data[max_len - 1] = '\0'; // Ensure null-termination

                // Remove the processed data from the buffer
                int remaining_data_length = uart_buffer_index - (newline_pos - uart_buffer + 1);
                memmove(uart_buffer, newline_pos + 1, remaining_data_length);
                uart_buffer_index = remaining_data_length;

                ESP_LOGI(TAG, "Latest number: %s", data);
				counter++;
            }
        }
    }
}