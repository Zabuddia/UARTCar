#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/gptimer.h"
#include "driver/rtc_io.h"
#include "driver/uart.h"

#include "btn.h"
#include "joy.h"
#include "pin.h"

#define UART_PORT UART_NUM_2
#define UART_BAUD_RATE 9600
#define UART_TX_PIN 16
#define UART_RX_PIN 17

#define RESOLUTION  1000000
#define ALARM_COUNT (RESOLUTION/20)

static const char* TAG = "main";

gptimer_handle_t gptimer;
bool interrupt_flag;

// Callback function that the timer calls when the alarm is triggered. The parameters are not used and it doesn't matter what it returns
static bool timer_on_alarm_cb(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx) {
	return (interrupt_flag = true);
}

void configure_and_start_gptimer() {
    // Configuring the timer to the default source and to count up and with 1Mhz resolution
	gptimer_config_t timer_config = {
		.clk_src = GPTIMER_CLK_SRC_DEFAULT,
		.direction = GPTIMER_COUNT_UP,
		.resolution_hz = RESOLUTION,
	};
    gptimer_new_timer(&timer_config, &gptimer);

	// Saying that the timer will do the callback function when the alarm is triggered
	gptimer_event_callbacks_t cbs = {
		.on_alarm = timer_on_alarm_cb,
	};
    gptimer_register_event_callbacks(gptimer, &cbs, NULL);

	// Configuring the alarm to trigger every 50ms
	gptimer_alarm_config_t alarm_config = {
	 	.reload_count = 0,
	 	.alarm_count = ALARM_COUNT,
		.flags.auto_reload_on_alarm = true,
	};
    gptimer_set_alarm_action(gptimer, &alarm_config);

    gptimer_enable(gptimer);

    gptimer_start(gptimer);
}

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

void uart_read_data() {
    uint8_t data[128];
    int length = 0;
    ESP_ERROR_CHECK(uart_get_buffered_data_len(UART_PORT, (size_t*)&length));
    if (length > 0) {
        int len = uart_read_bytes(UART_PORT, data, length, 100);
        if (len > 0) {
            data[len] = 0; // Null-terminate whatever we received and treat like a string
            printf("Received: %s\n", data);
        }
    }
}

void app_main() {
    interrupt_flag = false;

    // Configure I/O pins for buttons
	pin_reset(BTN_A);
	pin_input(BTN_A, true);
	pin_reset(BTN_B);
	pin_input(BTN_B, true);
	pin_reset(BTN_MENU);
	pin_input(BTN_MENU, true);
	pin_reset(BTN_OPTION);
	pin_input(BTN_OPTION, true);
	pin_reset(BTN_SELECT);
	pin_input(BTN_SELECT, true);
	pin_reset(BTN_START);
	pin_input(BTN_START, true);

    configure_and_start_gptimer();

	uart_init();

    while (1) {
        while (!interrupt_flag) ;
        interrupt_flag = false;
		if (!pin_get_level(BTN_A)) {
			uart_send_data("1");
		}
    }
}