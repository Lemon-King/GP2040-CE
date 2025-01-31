#include "addons/board_led.h"
#include "usb_driver.h" // Required to check USB state

bool BoardLedAddon::available() {
    BoardOptions options = Storage::getInstance().getBoardOptions();
    return options.onBoardLedMode != OnBoardLedMode::BOARD_LED_OFF; // Available only when it's not set to off
}

void BoardLedAddon::setup() {
    gpio_init(BOARD_LED_PIN);
    gpio_set_dir(BOARD_LED_PIN, GPIO_OUT);
    BoardOptions options = Storage::getInstance().getBoardOptions();
    onBoardLedMode = options.onBoardLedMode;
    isConfigMode = Storage::getInstance().GetConfigMode();
    timeSinceBlink = getMillis();
    prevState = -1;
}

void BoardLedAddon::process() {
    bool state = 0;
    Gamepad * gamepad;
    switch (onBoardLedMode) {
        case OnBoardLedMode::INPUT_TEST: // Blinks on input
            gamepad = Storage::getInstance().GetProcessedGamepad();
            state = (gamepad->state.buttons != 0) || (gamepad->state.dpad != 0);
            if (prevState != state) {
                gpio_put(BOARD_LED_PIN, state ? 1 : 0);
            }
            prevState = state;
            break;
        case OnBoardLedMode::MODE_INDICATOR: // Blinks based on USB state and config mode
            if (!get_usb_mounted()) { // USB not mounted
                uint32_t millis = getMillis();
                if ((millis - timeSinceBlink) > BLINK_INTERVAL_USB_UNMOUNTED) {
                    gpio_put(BOARD_LED_PIN, prevState ? 1 : 0);
                    timeSinceBlink = millis;
                    prevState = !prevState;
                }
            } else {
                if (isConfigMode) { // Current mode is config
                    uint32_t millis = getMillis();
                    if ((millis - timeSinceBlink) > BLINK_INTERVAL_CONFIG_MODE) {
                        gpio_put(BOARD_LED_PIN, prevState ? 1 : 0);
                        timeSinceBlink = millis;
                        prevState = !prevState;
                    }
                } else { // Regular mode and functional
                    if (prevState != 1) {
                        gpio_put(BOARD_LED_PIN, 1);
                        prevState = 1;
                    }
                }
            }
            break;
        case OnBoardLedMode::BOARD_LED_OFF:
            return;
            break;
    }    
}
