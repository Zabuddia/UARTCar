#include "joy.h"
#include "esp_adc/adc_oneshot.h"

#define NUM_READS 3
#define X_CHANNEL ADC_CHANNEL_6
#define Y_CHANNEL ADC_CHANNEL_7

adc_oneshot_unit_handle_t adc_handle;

int average_x;
int average_y;

// Initialize the joystick driver. Must be called before use.
// May be called multiple times. Return if already initialized.
// Return zero if successful, or non-zero otherwise.
int32_t joy_init(void) {
    // Checks to see if the adc_handle hasn't already been initialized
    if (adc_handle == NULL) {
        // Configures the ADC
        adc_oneshot_unit_init_cfg_t init_config = {
            .unit_id = ADC_UNIT_1,
            .ulp_mode = ADC_ULP_MODE_DISABLE,
        };
        adc_oneshot_new_unit(&init_config, &adc_handle);
        
        // Configures the ADC channels
        adc_oneshot_chan_cfg_t config = {
            .bitwidth = ADC_BITWIDTH_DEFAULT,
            .atten = ADC_ATTEN_DB_12,
        };
        adc_oneshot_config_channel(adc_handle, X_CHANNEL, &config);
        adc_oneshot_config_channel(adc_handle, Y_CHANNEL, &config);

        // Gets the average of 3 readings to make the bias for x
        int read_x_1 = 0;
        int read_x_2 = 0;
        int read_x_3 = 0;
        adc_oneshot_read(adc_handle, X_CHANNEL, &read_x_1);
        adc_oneshot_read(adc_handle, X_CHANNEL, &read_x_2);
        adc_oneshot_read(adc_handle, X_CHANNEL, &read_x_3);
        average_x = (read_x_1 + read_x_2 + read_x_3) / NUM_READS;

        // Gets the average of 3 readings to make the bias for y
        int read_y_1 = 0;
        int read_y_2 = 0;
        int read_y_3 = 0;
        adc_oneshot_read(adc_handle, Y_CHANNEL, &read_y_1);
        adc_oneshot_read(adc_handle, Y_CHANNEL, &read_y_2);
        adc_oneshot_read(adc_handle, Y_CHANNEL, &read_y_3);
        average_y = (read_y_1 + read_y_2 + read_y_3) / NUM_READS;

        return 0;
    } else {
        return -1;
    }

}

// Free resources used by the joystick (ADC unit).
// Return zero if successful, or non-zero otherwise.
int32_t joy_deinit(void) {
    // Checks to see if the adc_handle has been initialized before deleting it
    if (adc_handle != NULL) {
        adc_oneshot_del_unit(adc_handle);
        return 0;
    } else {
        return -1;
    }
}

// Get the joystick displacement from center position.
// Displacement values range from 0 to +/- JOY_MAX_DISP.
// This function is not safe to call from an ISR context.
// Therefore, it must be called from a software task context.
// *dcx: pointer to displacement in x.
// *dcy: pointer to displacement in y.
void joy_get_displacement(int *dcx, int *dcy) {
    int read_x = 0;
    adc_oneshot_read(adc_handle, X_CHANNEL, &read_x);
    int read_y = 0;
    adc_oneshot_read(adc_handle, Y_CHANNEL, &read_y);
    *dcx = read_x - average_x;
    *dcy = read_y - average_y;
}