#include "wifi_board.h"
#include "es8388_audio_codec.h"
#include "display/lcd_display.h"
#include "application.h"
#include "button.h"
#include "config.h"
#include "i2c_device.h"
#include "iot/thing_manager.h"
#include "yaka_led_controller.h"

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

#define TAG "YAKA_EXPLORATION_V1"

LV_FONT_DECLARE(font_puhui_20_4);
LV_FONT_DECLARE(font_awesome_20_4);
class YAKA_EXPLORATION_V1 : public WifiBoard {
private:
    i2c_master_bus_handle_t i2c_bus_;
    
    // 按键
    Button key_a_button_;           // 按键A
    Button key_b_button_;           // 按键B
    
    // LED控制器
    YakaLedController* led_controller_;
    
    // 电源和模块控制
    bool power_enabled_;            // 电源状态
    bool modules_enabled_;          // 模块状态
    
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
        // vTaskDelay(1000);
        ESP_LOGI(TAG, "PWR_EN_GPIO=%d", gpio_get_level(PWR_EN_GPIO));
        // printf("PWR_EN_GPIO=%d\n", gpio_get_level(PWR_EN_GPIO));

        // LED控制引脚
        gpio_config_t led_config = {
            .pin_bit_mask = (1ULL << LED1_GPIO) | (1ULL << LED2_GPIO) | (1ULL << LED3_GPIO) | (1ULL << LED4_GPIO) | (1ULL << LED5_GPIO),
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_ENABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE
        };
        ESP_ERROR_CHECK(gpio_config(&led_config));
        gpio_set_level(LED1_GPIO, 1); 
        gpio_set_level(LED2_GPIO, 1); 
        gpio_set_level(LED3_GPIO, 1);
        gpio_set_level(LED4_GPIO, 1);
        gpio_set_level(LED5_GPIO, 1); 
        // vTaskDelay(1000);
        ESP_LOGI(TAG, "LED1_GPIO=%d, LED2_GPIO=%d, LED3_GPIO=%d, LED4_GPIO=%d, LED5_GPIO=%d",
                 gpio_get_level(LED1_GPIO), gpio_get_level(LED2_GPIO),
                 gpio_get_level(LED3_GPIO), gpio_get_level(LED4_GPIO), gpio_get_level(LED5_GPIO));
        printf("LED1_GPIO=%d, LED2_GPIO=%d, LED3_GPIO=%d, LED4_GPIO=%d, LED5_GPIO=%d\n",
               gpio_get_level(LED1_GPIO), gpio_get_level(LED2_GPIO),
               gpio_get_level(LED3_GPIO), gpio_get_level(LED4_GPIO), gpio_get_level(LED5_GPIO));

        // 模块控制引脚
        gpio_config_t module_config = {
            .pin_bit_mask = (1ULL << ML307_EN_GPIO) | (1ULL << MIC_EN_GPIO),
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_ENABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE
        };
        ESP_ERROR_CHECK(gpio_config(&module_config));
        gpio_set_level(ML307_EN_GPIO, 1);
        gpio_set_level(MIC_EN_GPIO, 1);
        vTaskDelay(1000);
        ESP_LOGI(TAG, "ML307_EN_GPIO=%d, MIC_EN_GPIO=%d",
                gpio_get_level(ML307_EN_GPIO), gpio_get_level(MIC_EN_GPIO));
        printf("ML307_EN_GPIO=%d, MIC_EN_GPIO=%d\n",
               gpio_get_level(ML307_EN_GPIO), gpio_get_level(MIC_EN_GPIO));
        power_enabled_ = true;
        modules_enabled_ = true;

    }

    void InitializeLEDs() {
        ESP_LOGI(TAG, "Initializing YAKA LED Controller");
        led_controller_ = new YakaLedController();
        led_controller_->Initialize();
    }

    void InitializeButtons() {
        ESP_LOGI(TAG, "Initializing buttons with enhanced functionality");
        
        // 按键A - 支持长按开关机和短按LED控制
        key_a_button_.OnLongPress([this]() {
            ESP_LOGI(TAG, "Key A long pressed (>1s) - Power toggle");
            if (power_enabled_) {
                PowerOff();
            } else {
                PowerOn();
            }
        });
        
        key_a_button_.OnClick([this]() {
            ESP_LOGI(TAG, "Key A short click (90-250ms) - Toggle LEDs");
            if (power_enabled_ && led_controller_) {
                led_controller_->ToggleLEDs();
            }
        });
        
        // 按键B - 支持长按开关机和短按系统功能
        key_b_button_.OnLongPress([this]() {
            ESP_LOGI(TAG, "Key B long pressed (>1s) - Power toggle");
            if (power_enabled_) {
                PowerOff();
            } else {
                PowerOn();
            }
        });
        
        key_b_button_.OnClick([this]() {
            ESP_LOGI(TAG, "Key B short click - System functions");
            // 按键B短按用于系统功能
            auto& app = Application::GetInstance();
            if (app.GetDeviceState() == kDeviceStateStarting && 
                !WifiStation::GetInstance().IsConnected()) {
                ResetWifiConfiguration();
            }
            app.ToggleChatState();
        });
    }

    void EnableModules() {
        ESP_LOGI(TAG, "Enabling modules (4G and MIC)");
        gpio_set_level(ML307_EN_GPIO, 1);
        gpio_set_level(MIC_EN_GPIO, 1);
        modules_enabled_ = true;
        
        // 给模块一些时间来启动
        vTaskDelay(pdMS_TO_TICKS(100));
        ESP_LOGI(TAG, "Modules enabled successfully");
    }

    void DisableModules() {
        ESP_LOGI(TAG, "Disabling modules (4G and MIC)");
        gpio_set_level(ML307_EN_GPIO, 0);
        gpio_set_level(MIC_EN_GPIO, 0);
        modules_enabled_ = false;
        ESP_LOGI(TAG, "Modules disabled");
    }

    void PowerOn() {
        ESP_LOGI(TAG, "Starting Power ON sequence");
        if (!power_enabled_) {
            // 1. 设置电源控制
            gpio_set_level(PWR_EN_GPIO, 1);
            power_enabled_ = true;
            
            // 2. 启动模块
            EnableModules();
            
            // 3. 启动LED序列
            if (led_controller_) {
                led_controller_->ShowStartupSequence();
            }
            
            ESP_LOGI(TAG, "System powered on successfully");
        } else {
            ESP_LOGI(TAG, "System already powered on");
        }
    }

    void PowerOff() {
        ESP_LOGI(TAG, "Starting Power OFF sequence");
        if (power_enabled_) {
            // 1. 显示关机LED序列
            if (led_controller_) {
                led_controller_->ShowShutdownSequence();
            }
            
            // 2. 关闭模块
            DisableModules();
            
            // 3. 延迟确保模块完全关闭
            vTaskDelay(pdMS_TO_TICKS(500));
            
            // 4. 关闭电源控制
            gpio_set_level(PWR_EN_GPIO, 0);
            power_enabled_ = false;
            
            ESP_LOGI(TAG, "System powered off successfully");
        } else {
            ESP_LOGI(TAG, "System already powered off");
        }
    }

    void InitializeI2c() {
        ESP_LOGI(TAG, "Initializing I2C bus");
        // Initialize I2C peripheral
        i2c_master_bus_config_t i2c_bus_cfg = {
            .i2c_port = (i2c_port_t)I2C_NUM_0,
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
        ESP_LOGI(TAG, "I2C bus initialized successfully");
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
    YAKA_EXPLORATION_V1() : 
        key_a_button_(KEYA_GPIO, false),  // 1秒长按，50ms短按检测
        key_b_button_(KEYB_GPIO, false),  // 1秒长按，50ms短按检测
        led_controller_(nullptr),
        power_enabled_(false),
        modules_enabled_(false){
        
        ESP_LOGI(TAG, "Initializing YAKA Exploration V1 Board");
        
        // 按顺序初始化各个子系统
        InitializeI2c();
        InitializeGPIO();
        InitializeLEDs();
        InitializeButtons();
        InitializeIot();
                
        ESP_LOGI(TAG, "YAKA Exploration V1 initialization completed successfully");
    }

    virtual ~YAKA_EXPLORATION_V1() {
        ESP_LOGI(TAG, "Cleaning up YAKA Exploration V1");
        
        // 执行关机序列
        PowerOff();
        
        // 清理资源
        if (led_controller_) {
            delete led_controller_;
        }               
    }

    // virtual Led* GetLed() override {
    //     ESP_LOGI(TAG, "Getting LED instance");
    //     static SingleLed led(BUILTIN_LED_GPIO);
    //     return &led;
    // }

    virtual AudioCodec* GetAudioCodec() override {
        ESP_LOGI(TAG, "Initializing audio codec");
        static Es8388AudioCodec audio_codec(
            i2c_bus_, 
            I2C_NUM_0, 
            AUDIO_INPUT_SAMPLE_RATE, 
            AUDIO_OUTPUT_SAMPLE_RATE,
            AUDIO_I2S_GPIO_MCLK, 
            AUDIO_I2S_GPIO_BCLK, 
            AUDIO_I2S_GPIO_WS, 
            AUDIO_I2S_GPIO_DOUT, 
            AUDIO_I2S_GPIO_DIN,
            AUDIO_CODEC_PA_PIN, 
            AUDIO_CODEC_ES8388_ADDR
        );
        ESP_LOGI(TAG, "Audio codec initialized successfully");
        return &audio_codec;
    }

};

DECLARE_BOARD(YAKA_EXPLORATION_V1);
