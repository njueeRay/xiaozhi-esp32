#ifndef _BOARD_CONFIG_H_
#define _BOARD_CONFIG_H_

#include <driver/gpio.h>

#define AUDIO_INPUT_SAMPLE_RATE      24000
#define AUDIO_OUTPUT_SAMPLE_RATE     24000

#define AUDIO_INPUT_REFERENCE    true

#define AUDIO_I2S_GPIO_MCLK GPIO_NUM_38
#define AUDIO_I2S_GPIO_WS GPIO_NUM_13
#define AUDIO_I2S_GPIO_BCLK GPIO_NUM_14
#define AUDIO_I2S_GPIO_DIN  GPIO_NUM_12
#define AUDIO_I2S_GPIO_DOUT GPIO_NUM_45

#define AUDIO_CODEC_USE_PCA9557
#define AUDIO_CODEC_I2C_SDA_PIN  GPIO_NUM_1
#define AUDIO_CODEC_I2C_SCL_PIN  GPIO_NUM_2
#define AUDIO_CODEC_ES8311_ADDR  ES8311_CODEC_DEFAULT_ADDR
#define AUDIO_CODEC_ES7210_ADDR  0x82

// 按键定义
#define BUILTIN_LED_GPIO          GPIO_NUM_8
#define KEYA_GPIO                 GPIO_NUM_42
#define KEYB_GPIO                 GPIO_NUM_15  
#define PWR_EN_GPIO               GPIO_NUM_41
#define BOOT_BUTTON_GPIO          GPIO_NUM_0  
#define VOLUME_UP_BUTTON_GPIO     GPIO_NUM_NC
#define VOLUME_DOWN_BUTTON_GPIO   GPIO_NUM_NC

// LED定义
#define LED1_IOEX                 3
#define LED2_IOEX                 4
#define LED3_IOEX                 5
#define LED4_IOEX                 6
#define LED5_IOEX                 7


// 4G模块
// #define MODULE_4G_RX_PIN          GPIO_NUM_43
// #define MODULE_4G_TX_PIN          GPIO_NUM_44

#endif // _BOARD_CONFIG_H_

