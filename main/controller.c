#include "controller.h"

#define X_LOWER_THRESHOLD -200
#define X_UPPER_THRESHOLD 200
#define Y_LOWER_THRESHOLD -200
#define Y_UPPER_THRESHOLD 200

#define FIRST_THRESHOLD 200
#define SECOND_THRESHOLD 350
#define THIRD_THRESHOLD 700
#define FOURTH_THRESHOLD 1050
#define FIFTH_THRESHOLD 1400
#define SIXTH_THRESHOLD 1750

static int dcx, dcy;

static const char *TAG = "controller";

void controller_tick_1() {
    int speed = 0;
    int direction = 0;
    joy_get_displacement(&dcx, &dcy);

    if (abs(dcy) > abs(dcx)) {
        speed = abs(dcy);
    } else {
        speed = abs(dcx);
    }
    if (speed < FIRST_THRESHOLD) {
        speed = 0;
    } else if (speed < SECOND_THRESHOLD) {
        speed = 1;
    } else if (speed < THIRD_THRESHOLD) {
        speed = 2;
    } else if (speed < FOURTH_THRESHOLD) {
        speed = 3;
    } else if (speed < FIFTH_THRESHOLD) {
        speed = 4;
    } else if (speed < SIXTH_THRESHOLD){
        speed = 5;
    } else {
        speed = 6;
    }
    
    if (dcx < X_LOWER_THRESHOLD) {
        direction = 3;
    } else if (dcx > X_UPPER_THRESHOLD) {
        direction = 4;
    } else if (dcy < Y_LOWER_THRESHOLD) {
        direction = 1;
    } else if (dcy > Y_UPPER_THRESHOLD) {
        direction = 2;
    } else {
        direction = 0;
    }

    if (speed == 0 || direction == 0) {
        uart_send_data("0");
    } else if (speed == 1 && direction == 1) {
        uart_send_data("1");
    } else if (speed == 2 && direction == 1) {
        uart_send_data("2");
    } else if (speed == 3 && direction == 1) {
        uart_send_data("3");
    } else if (speed == 4 && direction == 1) {
        uart_send_data("4");
    } else if (speed == 5 && direction == 1) {
        uart_send_data("5");
    } else if (speed == 6 && direction == 1) {
        uart_send_data("6");
    } else if (speed == 1 && direction == 2) {
        uart_send_data("7");
    } else if (speed == 2 && direction == 2) {
        uart_send_data("8");
    } else if (speed == 3 && direction == 2) {
        uart_send_data("9");
    } else if (speed == 4 && direction == 2) {
        uart_send_data("A");
    } else if (speed == 5 && direction == 2) {
        uart_send_data("B");
    } else if (speed == 6 && direction == 2) {
        uart_send_data("C");
    } else if (speed == 1 && direction == 3) {
        uart_send_data("D");
    } else if (speed == 2 && direction == 3) {
        uart_send_data("E");
    } else if (speed == 3 && direction == 3) {
        uart_send_data("F");
    } else if (speed == 4 && direction == 3) {
        uart_send_data("G");
    } else if (speed == 5 && direction == 3) {
        uart_send_data("H");
    } else if (speed == 6 && direction == 3) {
        uart_send_data("I");
    } else if (speed == 1 && direction == 4) {
        uart_send_data("J");
    } else if (speed == 2 && direction == 4) {
        uart_send_data("K");
    } else if (speed == 3 && direction == 4) {
        uart_send_data("L");
    } else if (speed == 4 && direction == 4) {
        uart_send_data("M");
    } else if (speed == 5 && direction == 4) {
        uart_send_data("N");
    } else if (speed == 6 && direction == 4) {
        uart_send_data("O");
    }
    lcdDrawString(&dev, 0, 0, "Controller", WHITE);
    lcdDrawString(&dev, 0, 10, "Speed: ", WHITE);
    if (speed == 0) lcdDrawString(&dev, 50, 10, "0", WHITE);
    else if (speed == 1) lcdDrawString(&dev, 50, 10, "1", WHITE);
    else if (speed == 2) lcdDrawString(&dev, 50, 10, "2", WHITE);
    else if (speed == 3) lcdDrawString(&dev, 50, 10, "3", WHITE);
    else if (speed == 4) lcdDrawString(&dev, 50, 10, "4", WHITE);
    else if (speed == 5) lcdDrawString(&dev, 50, 10, "5", WHITE);
    else if (speed == 6) lcdDrawString(&dev, 50, 10, "6", WHITE);
    lcdDrawString(&dev, 0, 20, "Direction: ", WHITE);
    if (direction == 0) lcdDrawString(&dev, 60, 20, "NONE", WHITE);
    else if (direction == 1) lcdDrawString(&dev, 60, 20, "FORWARDS", WHITE);
    else if (direction == 2) lcdDrawString(&dev, 60, 20, "BACKWARDS", WHITE);
    else if (direction == 3) lcdDrawString(&dev, 60, 20, "LEFT", WHITE);
    else if (direction == 4) lcdDrawString(&dev, 60, 20, "RIGHT", WHITE);
}

void controller_tick_2() {
    static int total_data = 0;
    static int data_num = 0;
    static char data_output[128];
    char data[128];
    uart_read_data(data, 128);
    total_data += atoi(data);
    if (counter == 10) {
        counter = 0;
        data_num = total_data / 10;
        ESP_LOGI(TAG, "Data: %d", data_num);
        sprintf(data_output, "Data: %d", data_num);
        total_data = 0;
    }
    lcdDrawString(&dev, 0, 30, data_output, WHITE);
}