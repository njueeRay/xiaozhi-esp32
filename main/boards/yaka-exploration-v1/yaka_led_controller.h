#ifndef _YAKA_LED_CONTROLLER_H_
#define _YAKA_LED_CONTROLLER_H_

#include "led/gpio_led.h"
#include "led/single_led.h"
#include <driver/gpio.h>
#include <esp_log.h>

class YakaLedController {
private:
    static const int LED_COUNT = 1;
    GpioLed* single_leds_[LED_COUNT];
    SingleLed* rgb_led_;
    bool leds_state_;
    
    static const char* TAG;

public:
    YakaLedController();
    ~YakaLedController();
    
    void Initialize();
    void SetAllLEDs(bool state);
    void ToggleLEDs();
    void SetSingleLED(int index, bool state);
    void ShowStartupSequence();
    void ShowShutdownSequence();
    
    bool GetLEDState() const { return leds_state_; }
    SingleLed* GetRGBLed() const { return rgb_led_; }
};

#endif // _YAKA_LED_CONTROLLER_H_ 