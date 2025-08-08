#ifndef _BOARD_CONFIG_H_
#define _BOARD_CONFIG_H_

#include <driver/gpio.h>

#define AUDIO_INPUT_SAMPLE_RATE      24000
#define AUDIO_OUTPUT_SAMPLE_RATE     24000

#define AUDIO_I2S_GPIO_MCLK GPIO_NUM_38
#define AUDIO_I2S_GPIO_WS GPIO_NUM_47
#define AUDIO_I2S_GPIO_BCLK GPIO_NUM_48
#define AUDIO_I2S_GPIO_DIN GPIO_NUM_45
#define AUDIO_I2S_GPIO_DOUT GPIO_NUM_21
// #define AUDIO_I2S_GPIO_DIN GPIO_NUM_21
// #define AUDIO_I2S_GPIO_DOUT GPIO_NUM_45

#define AUDIO_CODEC_PA_PIN      GPIO_NUM_7  // PA控制引脚
#define AUDIO_CODEC_I2C_SDA_PIN GPIO_NUM_1
#define AUDIO_CODEC_I2C_SCL_PIN GPIO_NUM_2
#define AUDIO_CODEC_ES8388_ADDR ES8388_CODEC_DEFAULT_ADDR

// 按键定义
#define KEYA_GPIO                 GPIO_NUM_42
#define KEYB_GPIO                 GPIO_NUM_15  
#define PWR_EN_GPIO               GPIO_NUM_41
#define BOOT_BUTTON_GPIO          GPIO_NUM_42  // 复用KEYA作为BOOT按键

// LED定义
#define LED1_GPIO                 GPIO_NUM_37
#define LED2_GPIO                 GPIO_NUM_36
#define LED3_GPIO                 GPIO_NUM_35
#define LED4_GPIO                 GPIO_NUM_4
#define LED5_GPIO                 GPIO_NUM_5
#define BUILTIN_LED_GPIO          GPIO_NUM_8   // WS2812 RGB LED

// 模块控制
#define ML307_EN_GPIO                GPIO_NUM_14
#define MIC_EN_GPIO               GPIO_NUM_9

// 4G模块
#define MODULE_4G_RX_PIN          GPIO_NUM_43
#define MODULE_4G_TX_PIN          GPIO_NUM_44

#endif // _BOARD_CONFIG_H_

