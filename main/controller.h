#ifndef CONTROL_H
#define CONTROL_H

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "esp_log.h"
#include "btn.h"
#include "joy.h"
#include "pin.h"
#include "lcd.h"
#include "UART.h"

void controller_tick_1();
void controller_tick_2();

#endif // CONTROL_H