#include <stdio.h>
#include "soc/reg_base.h" // DR_REG_GPIO_BASE, DR_REG_IO_MUX_BASE
#include "driver/rtc_io.h" // rtc_gpio_*
#include "pin.h"

// GPIO Matrix Registers
#define GPIO_OUT_REG                 (DR_REG_GPIO_BASE+0x04)
#define GPIO_OUT_W1TS_REG            (DR_REG_GPIO_BASE+0x08)
#define GPIO_OUT_W1TC_REG            (DR_REG_GPIO_BASE+0x0C)
#define GPIO_OUT1_REG                (DR_REG_GPIO_BASE+0x10)
#define GPIO_OUT1_W1TS_REG           (DR_REG_GPIO_BASE+0x14)
#define GPIO_OUT1_W1TC_REG           (DR_REG_GPIO_BASE+0x18)
#define GPIO_ENABLE_REG              (DR_REG_GPIO_BASE+0x20)
#define GPIO_ENABLE_W1TS_REG         (DR_REG_GPIO_BASE+0x24)
#define GPIO_ENABLE_W1TC_REG         (DR_REG_GPIO_BASE+0x28)
#define GPIO_ENABLE1_REG             (DR_REG_GPIO_BASE+0x2C)
#define GPIO_ENABLE1_W1TS_REG        (DR_REG_GPIO_BASE+0x30)
#define GPIO_ENABLE1_W1TC_REG        (DR_REG_GPIO_BASE+0x34)
#define GPIO_STRAP_REG               (DR_REG_GPIO_BASE+0x38)
#define GPIO_IN_REG                  (DR_REG_GPIO_BASE+0x3C)
#define GPIO_IN1_REG                 (DR_REG_GPIO_BASE+0x40)
#define GPIO_STATUS_REG              (DR_REG_GPIO_BASE+0x44)
#define GPIO_STATUS_W1TS_REG         (DR_REG_GPIO_BASE+0x48)
#define GPIO_STATUS_W1TC_REG         (DR_REG_GPIO_BASE+0x4C)
#define GPIO_STATUS1_REG             (DR_REG_GPIO_BASE+0x50)
#define GPIO_STATUS1_W1TS_REG        (DR_REG_GPIO_BASE+0x54)
#define GPIO_STATUS1_W1TC_REG        (DR_REG_GPIO_BASE+0x58)
#define GPIO_ACPU_INT_REG            (DR_REG_GPIO_BASE+0x60)
#define GPIO_ACPU_NMI_INT_REG        (DR_REG_GPIO_BASE+0x64)
#define GPIO_PCPU_INT_REG            (DR_REG_GPIO_BASE+0x68)
#define GPIO_PCPU_NMI_INT_REG        (DR_REG_GPIO_BASE+0x6C)
#define GPIO_ACPU1_INT_REG           (DR_REG_GPIO_BASE+0x74)
#define GPIO_ACPU1_NMI_INT_REG       (DR_REG_GPIO_BASE+0x78)
#define GPIO_PCPU1_INT_REG           (DR_REG_GPIO_BASE+0x7C)
#define GPIO_PCPU1_NMI_INT_REG       (DR_REG_GPIO_BASE+0x80)
#define GPIO_PIN_REG(n)              (DR_REG_GPIO_BASE+0x88+(0x04*n))
#define GPIO_FUNC_IN_SEL_CFG_REG(n)  (DR_REG_GPIO_BASE+0x0130+(0x04*n))
#define GPIO_FUNC_OUT_SEL_CFG_REG(n) (DR_REG_GPIO_BASE+0x0530+(0x04*n))

// GPIO Register Fields
#define PAD_DRIVER 2

// IO MUX Registers
#define IO_MUX_REG(n) (DR_REG_IO_MUX_BASE+PIN_MUX_REG_OFFSET[n])

// IO MUX Register Fields
#define MCU_OE   0
#define SLP_SEL  1
#define MCU_WPD  2
#define MCU_WPU  3
#define MCU_IE   4
#define MCU_DRV  5
#define FUN_WPD  7
#define FUN_WPU  8
#define FUN_IE   9
#define FUN_DRV  10
#define MCU_SEL  12

// Bit masks
#define RESET_GPIO_PIN_REG_MASK              0x00
#define RESET_GPIO_FUNC_OUT_SEL_CFG_REG_MASK 0x100
#define RESET_IO_MUX_REG_MASK                0x2900

// Register macros
#define REG(r) (*(volatile uint32_t *)(r))
#define REG_SET_BIT(r,b) (REG(r) |= (0x01 << b))
#define REG_CLR_BIT(r,b) (REG(r) &= ~(0x01 << b))
#define REG_GET_BIT(r,b) ((REG(r) >> b) & 0x01)

#define REGISTER_SIZE 32

// Gives byte offset of IO_MUX Configuration Register
// from base address DR_REG_IO_MUX_BASE
static const uint8_t PIN_MUX_REG_OFFSET[] = {
    0x44, 0x88, 0x40, 0x84, 0x48, 0x6c, 0x60, 0x64, // pin  0- 7
    0x68, 0x54, 0x58, 0x5c, 0x34, 0x38, 0x30, 0x3c, // pin  8-15
    0x4c, 0x50, 0x70, 0x74, 0x78, 0x7c, 0x80, 0x8c, // pin 16-23
    0x90, 0x24, 0x28, 0x2c, 0xFF, 0xFF, 0xFF, 0xFF, // pin 24-31
    0x1c, 0x20, 0x14, 0x18, 0x04, 0x08, 0x0c, 0x10, // pin 32-39
};


// Reset the configuration of a pin to not be an input or an output.
// Pull-up is enabled so the pin does not float.
int32_t pin_reset(pin_num_t pin)
{
	if (rtc_gpio_is_valid_gpio(pin)) { // hand-off work to RTC subsystem
		rtc_gpio_deinit(pin);
		rtc_gpio_pullup_en(pin);
		rtc_gpio_pulldown_dis(pin);
	}

	// Reset GPIO_PINn_REG: All fields zero
	REG(GPIO_PIN_REG(pin)) = RESET_GPIO_PIN_REG_MASK;

	// Reset GPIO_FUNCn_OUT_SEL_CFG_REG: GPIO_FUNCn_OUT_SEL=0x100
	REG(GPIO_FUNC_OUT_SEL_CFG_REG(pin)) = RESET_GPIO_FUNC_OUT_SEL_CFG_REG_MASK;

	// Reset IO_MUX_x_REG: MCU_SEL=2, FUN_DRV=2, FUN_WPU=1
	REG(IO_MUX_REG(pin)) = RESET_IO_MUX_REG_MASK;

	// NOTE: By default, pin should not float, save power with FUN_WPU=1
	REG_SET_BIT(IO_MUX_REG(pin), FUN_WPU);
	// Now that the pin is reset, set the output level to zero
	return pin_set_level(pin, 0);
}

// Enable or disable a pull-up on the pin.
int32_t pin_pullup(pin_num_t pin, bool enable)
{
	if (rtc_gpio_is_valid_gpio(pin)) { // hand-off work to RTC subsystem
		if (enable) return rtc_gpio_pullup_en(pin);
		else return rtc_gpio_pullup_dis(pin);
	}
	// Set or clear the FUN_WPU bit in an IO_MUX register
	enable ? REG_SET_BIT(IO_MUX_REG(pin), FUN_WPU) : REG_CLR_BIT(IO_MUX_REG(pin), FUN_WPU);

	return 0;
}

// Enable or disable a pull-down on the pin.
int32_t pin_pulldown(pin_num_t pin, bool enable)
{
	if (rtc_gpio_is_valid_gpio(pin)) { // hand-off work to RTC subsystem
		if (enable) return rtc_gpio_pulldown_en(pin);
		else return rtc_gpio_pulldown_dis(pin);
	}
	// Set or clear the FUN_WPD bit in an IO_MUX register
	enable ? REG_SET_BIT(IO_MUX_REG(pin), FUN_WPD) : REG_CLR_BIT(IO_MUX_REG(pin), FUN_WPD);

	return 0;
}

// Enable or disable the pin as an input signal.
int32_t pin_input(pin_num_t pin, bool enable)
{
	// Set or clear the FUN_IE bit in an IO_MUX register
	enable ? REG_SET_BIT(IO_MUX_REG(pin), FUN_IE) : REG_CLR_BIT(IO_MUX_REG(pin), FUN_IE);

	return 0;
}

// Enable or disable the pin as an output signal.
int32_t pin_output(pin_num_t pin, bool enable)
{
	// Set or clear the I/O pin bit in the ENABLE or ENABLE1 register
	if (pin < REGISTER_SIZE) {
		enable ? REG_SET_BIT(GPIO_ENABLE_W1TS_REG, pin) : REG_SET_BIT(GPIO_ENABLE_W1TC_REG, pin);
	} else {
		enable ? REG_SET_BIT(GPIO_ENABLE1_W1TS_REG, (pin-REGISTER_SIZE)) : REG_SET_BIT(GPIO_ENABLE1_W1TC_REG, (pin-REGISTER_SIZE));
	}

	return 0;
}

// Enable or disable the pin as an open-drain signal.
int32_t pin_odrain(pin_num_t pin, bool enable)
{
	// Set or clear the PAD_DRIVER bit in a PIN register
	enable ? REG_SET_BIT(GPIO_PIN_REG(pin), PAD_DRIVER) : REG_CLR_BIT(GPIO_PIN_REG(pin), PAD_DRIVER);

	return 0;
}

// Sets the output signal level if the pin is configured as an output.
int32_t pin_set_level(pin_num_t pin, int32_t level)
{
	// Set or clear the I/O pin bit in the OUT or OUT1 register
	if (pin < REGISTER_SIZE) {
		level ? REG_SET_BIT(GPIO_OUT_W1TS_REG, pin) : REG_SET_BIT(GPIO_OUT_W1TC_REG, pin);
	} else {
		level ? REG_SET_BIT(GPIO_OUT1_W1TS_REG, (pin-REGISTER_SIZE)) : REG_SET_BIT(GPIO_OUT1_W1TC_REG, (pin-REGISTER_SIZE));
	}
	
	return 0;
}

// Gets the input signal level if the pin is configured as an input.
int32_t pin_get_level(pin_num_t pin)
{
	// Get the I/O pin bit from the IN or IN1 register
	if (pin < REGISTER_SIZE) {
		return REG_GET_BIT(GPIO_IN_REG, pin);
	} else {
		return REG_GET_BIT(GPIO_IN1_REG, (pin-REGISTER_SIZE));
	}
}

// Get the value of the input registers, one pin per bit.
// The two 32-bit input registers are concatenated into a uint64_t.
uint64_t pin_get_in_reg(void)
{
	// Read the IN and IN1 registers, return the concatenated values
	return (uint64_t)REG(GPIO_IN1_REG) << REGISTER_SIZE | REG(GPIO_IN_REG);
}

// Get the value of the output registers, one pin per bit.
// The two 32-bit output registers are concatenated into a uint64_t.
uint64_t pin_get_out_reg(void)
{
	// Read the OUT and OUT1 registers, return the concatenated values
	return (uint64_t)REG(GPIO_OUT1_REG) << REGISTER_SIZE | REG(GPIO_OUT_REG);
}