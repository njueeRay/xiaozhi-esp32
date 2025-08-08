#include "wifi_board.h"
#include "audio_codecs/box_audio_codec.h"
#include "display/lcd_display.h"
#include "application.h"
#include "button.h"
#include "config.h"
#include "i2c_device.h"
#include "iot/thing_manager.h"
#include "led/single_led.h"

#include <esp_log.h>
#include <esp_lcd_panel_vendor.h>
#include <driver/i2c_master.h>
#include <driver/spi_common.h>
#include <driver/gpio.h>
#include <driver/ledc.h>
#include <wifi_station.h>
#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define TAG "YAKA_EXPLORATION_V2"

LV_FONT_DECLARE(font_puhui_20_4);
LV_FONT_DECLARE(font_awesome_20_4);

class Pca9557 : public I2cDevice {
public:
    Pca9557(i2c_master_bus_handle_t i2c_bus, uint8_t addr) : I2cDevice(i2c_bus, addr) {
        WriteReg(0x01, 0x03);
        WriteReg(0x03, 0xf8);
    }

    void SetOutputState(uint8_t bit, uint8_t level) {
        uint8_t data = ReadReg(0x01);
        data = (data & ~(1 << bit)) | (level << bit);
        WriteReg(0x01, data);
    }
};

class YAKA_EXPLORATION_V2 : public WifiBoard {
private:
    i2c_master_bus_handle_t i2c_bus_;
    i2c_master_dev_handle_t pca9557_handle_;
    Pca9557* pca9557_;

    // 按键
    Button key_a_button_;           // 按键A
    Button key_b_button_;           // 按键B
    Button boot_button_;
    
    void InitializeI2c() {
        ESP_LOGI(TAG, "Initializing I2C bus");
        // Initialize I2C peripheral
        i2c_master_bus_config_t i2c_bus_cfg = {
            .i2c_port = (i2c_port_t)1,
            .sda_io_num = AUDIO_CODEC_I2C_SDA_PIN,
            .scl_io_num = AUDIO_CODEC_I2C_SCL_PIN,
            .clk_source = I2C_CLK_SRC_DEFAULT,
            .glitch_ignore_cnt = 7,
            .intr_priority = 0,
            .trans_queue_depth = 0,
            .flags = {
                .enable_internal_pullup = 1,
            },
        };
        ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_cfg, &i2c_bus_));
        // Initialize PCA9557
        pca9557_ = new Pca9557(i2c_bus_, 0x19);
        ESP_LOGI(TAG, "I2C bus initialized successfully");
    }

    void InitializeGPIO() {
        ESP_LOGI(TAG, "Initializing GPIO");
        // 电源控制引脚
        gpio_config_t pwr_config = {
            .pin_bit_mask = (1ULL << PWR_EN_GPIO) ,
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_ENABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE
        };
        ESP_ERROR_CHECK(gpio_config(&pwr_config));
        gpio_set_level(PWR_EN_GPIO, 1); // 初始化为开机状态，等待按键触发
        vTaskDelay(500);
        ESP_LOGI(TAG, "PWR_EN_GPIO=%d", gpio_get_level(PWR_EN_GPIO));
    }

    void InitializeLEDs() {
        ESP_LOGI(TAG, "Initializing YAKA LED Controller");
        pca9557_->SetOutputState(LED1_IOEX, 1);
        pca9557_->SetOutputState(LED2_IOEX, 1);
        pca9557_->SetOutputState(LED3_IOEX, 1);
        pca9557_->SetOutputState(LED4_IOEX, 1);
        pca9557_->SetOutputState(LED5_IOEX, 1);
    }

    void InitializeButtons() {
        boot_button_.OnClick([this]() {
            auto& app = Application::GetInstance();
            if (app.GetDeviceState() == kDeviceStateStarting && !WifiStation::GetInstance().IsConnected()) {
                ResetWifiConfiguration();
            }
            app.ToggleChatState();
        });
        key_a_button_.OnLongPress([this]() {
            ESP_LOGI(TAG, "Key A pressed, shutdown");
            gpio_set_level(PWR_EN_GPIO, 0); // 关闭电源
        });
        key_b_button_.OnClick([this]() {
            ESP_LOGI(TAG, "Key B pressed, change led state");
            pca9557_->SetOutputState(LED1_IOEX, 0);            
        });
    }

    // 物联网初始化，添加对 AI 可见设备 
    void InitializeIot() {
        ESP_LOGI(TAG, "Initializing IoT things");
        auto& thing_manager = iot::ThingManager::GetInstance();
        thing_manager.AddThing(iot::CreateThing("Speaker"));
        thing_manager.AddThing(iot::CreateThing("Screen"));
        ESP_LOGI(TAG, "IoT things initialized");
    }

public:
    YAKA_EXPLORATION_V2() :
        key_a_button_(KEYA_GPIO),           // 需要添加KEY_A_GPIO的定义
        key_b_button_(KEYB_GPIO),           // 需要添加KEY_B_GPIO的定义
        boot_button_(BOOT_BUTTON_GPIO) {
        InitializeI2c();
        InitializeButtons();
        InitializeIot();
        ESP_LOGI(TAG, "YAKA Exploration V2 initialization completed successfully");
    }

    virtual Led* GetLed() override {
        static SingleLed led(BUILTIN_LED_GPIO);
        return &led;
    }

    virtual AudioCodec* GetAudioCodec() override {
        static BoxAudioCodec audio_codec(
            i2c_bus_, 
            AUDIO_INPUT_SAMPLE_RATE, 
            AUDIO_OUTPUT_SAMPLE_RATE,
            AUDIO_I2S_GPIO_MCLK, 
            AUDIO_I2S_GPIO_BCLK, 
            AUDIO_I2S_GPIO_WS, 
            AUDIO_I2S_GPIO_DOUT, 
            AUDIO_I2S_GPIO_DIN,
            GPIO_NUM_NC, 
            AUDIO_CODEC_ES8311_ADDR, 
            AUDIO_CODEC_ES7210_ADDR, 
            AUDIO_INPUT_REFERENCE);
        return &audio_codec;
    }

};

DECLARE_BOARD(YAKA_EXPLORATION_V2);
