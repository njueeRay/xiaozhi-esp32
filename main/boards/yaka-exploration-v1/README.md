## yaka硬件配置信息

### 按键
> 有两个核心引脚 KEYA,KEYB，请关注 main\boards\common\button.cc，对应的头文件在同一个目录下。

#### 控制原理介绍
> 下面以KEY_ON （可指代KEYA/KEYB），PWR_EN为例进行说明

**单键开关机电路原理**：
1. 用户按下SW-PB按键，单片机上电后，程序将**KEY_ON**引脚设置为输入，**PWR_EN**设置为初始状态为0的推挽输出。持续检测**KEY_ON**引脚电平。由于用户当时的手还在按着开关，SW-PB的状态为按下，于是Q2导通，**KEY_ON**被拉低为低电平。
2. 如果检测到**KEY_ON**为低电平的时间不超过1s，由于**PWR_EN**的输出为0，当按键松开的瞬间，SW_EN就会变为低电平，整个系统立即掉电，仅保持着MP2144的0.1uA的关断电流。
3. 如果检测到**KEY_ON**引脚持续1S时间均为低电平，且此时系统状态为关机，则判断用户正在执行 长按开机动作。单片机会**执行开机过程**：将 **PWR_EN**设置输出高电平，系统持续供电，之后单片机正式初始化其他外设并运行系统。开机过程完毕。

**按键事件处理**：
> 只要监控KEY_ON引脚的电平，就可以获得按键的状态，并用于判断按键事件。
1. 开机过后，单片机建立一任务，持续检测 **KEY_ON**的电平和时间。（或者利用中断来完成这一目的）
2. 用户短按按键，**KEY_ON**电平会从高跳变到低，低电平会保持50ms，之后再跳变回高电平。
3. 单片机如果检测到**KEY_ON**电平为低，且持续时间较短，>=90ms且<=250ms 则判断用户此时正在单击按键，可执行对应的处理事件。

**按键关机过程**：

1. 用户长按按键，**KEY_ON**被持续拉低。
2. 单片机检测到**KEY_ON**电平为低，且持续时间较长，大于1S时间。则判断用户此时正在长按按键，可**执行关机过程**：将**PWR_EN**引脚输出为低电平，系统进入关机状态，MCU设置为掉电模式。待用户松开按键后，SW_EN将为低电平，MP2144不会继续供电，系统会彻底掉电。（想要系统重新上电，只有长按开机按键这一种方式。）

---

#### 目前需实现的内容
> 粗版：截至6.20前

**硬件引脚设置**：
- **KEYA**: GPIO_NUM_42
- **KEYB**: GPIO_NUM_15
- **PWR_EN**: GPIO_NUM_41 

**功能描述**：
- **长按**
	- 长按按键A或B(KEYA或KEYB检测到大于1s的低电平)，均可达到开/关机效果 (通过上面讲解的原理进行实现)
	- 同时，一旦开机，开关芯片的两个引脚 4G_EN和MIC_EN 自动使能，以启动对应模块
	- 开机后，自动将五个单色LED全部点亮
- **短按**
	- 短按按键A (KEYA检测到>=90ms且<=250ms的低电平)，可以执行五个单色LED的开和关 (toggle)

---
### LED

#### 单色LED
> 请关注 main/led/gpio_led.cc

**硬件引脚设置**：从LED1 ~ LED5 （从左到右）分别对应GPIO引脚编号 37, 36, 35, 4, 5
- 对应引脚输出高电平即可点亮单色LED

#### RGBLED - WS2812
> 请关注 main/led/single_led.cc - 采用的就是 ws2812

**硬件引脚设置**：对应GPIO引脚编号 8
- 参照single_led.cc中的逻辑对RGBLED进行控制即可

---

### 开关芯片CD4066
> 如何单独控制gpio引脚输出，请参考main\iot\things\lamp.cc中的 InitializeGpio()与gpio_set_level()

**功能描述**：用于控制4G模块和音频编解码芯片ES8388的电源 on/off， 分别由4G_EN和MIC_EN进行使能
**硬件引脚设置**：4G_EN和MIC_EN对应到ESP32S3的GPIO引脚编号分别为：14，9

---

### 4G模块

**硬件引脚设置**：
- Module_4G_RX_PIN：GPIO_NUM_43
- Module_4G_TX_PIN:GPIO_NUM_44

---

### 音频编解码芯片ES8388

**硬件引脚设置**：
```
#define AUDIO_I2S_GPIO_MCLK GPIO_NUM_38
#define AUDIO_I2S_GPIO_WS GPIO_NUM_47
#define AUDIO_I2S_GPIO_BCLK GPIO_NUM_48
#define AUDIO_I2S_GPIO_DIN GPIO_NUM_21
#define AUDIO_I2S_GPIO_DOUT GPIO_NUM_45

#define AUDIO_CODEC_I2C_SDA_PIN GPIO_NUM_1
#define AUDIO_CODEC_I2C_SCL_PIN GPIO_NUM_2
```

---
### NFC模块 - PN532
> 暂不关注，后续会进一步实现

- 