# GLCD ST7920 Serial Lib

That is Edited version of  [Original version](https://controllerstech.com/glcd-128x64-st7920-interfacing-with-stm32/) for stm32.

for Enable Serial mode for GLCD ST7920 `Pin15="PSB"` must connect to `GND`!

**Other Pins:**

| GLCD Pin | Name (PSB=Hi) | Name (PSB=LOW) |
| :------: | :-----------: | :------------: |
|    4     |      RS       |       CS       |
|    5     |      R/W      |      SID       |
|    6     |       E       |      SCK       |

**Note:**
- don't need hardware SPI , the library use GPIO to generate serial mode for st7920!
- All 3 pins must set as output.

#### Example Define Pins in `main.h`:
```c
#define GLCD_SID_Pin 		GPIO_PIN_1
#define GLCD_SID_GPIO_Port 	GPIOA

#define GLCD_SCK_Pin 		GPIO_PIN_2
#define GLCD_SCK_GPIO_Port 	GPIOB

#define GLCD_RST_Pin 		GPIO_PIN_12
#define GLCD_RST_GPIO_Port 	GPIOB

#define GLCD_CS_Pin 		GPIO_PIN_8
#define GLCD_CS_GPIO_Port 	GPIOA
```

