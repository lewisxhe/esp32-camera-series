
<h1 align = "center">🌟 T Camear V16 🌟</h1>


|    Name     | T-Camear V16 |
| :---------: | :----------: |
|   DVP Y9    |      36      |
|   DVP Y8    |      15      |
|   DVP Y7    |      12      |
|   DVP Y6    |      39      |
|   DVP Y5    |      35      |
|   DVP Y4    |      14      |
|   DVP Y3    |      13      |
|   DVP Y2    |      34      |
|  DVP VSNC   |      5       |
|  DVP HREF   |      27      |
|  DVP PCLK   |      25      |
|   DVP PWD   |     N/A      |
|  DVP XCLK   |      4       |
|  DVP SIOD   |      18      |
|  DVP SIOC   |      23      |
|  DVP RESET  |     N/A      |
|     SDA     |      21      |
|     SCL     |      22      |
|   Button    |      0       |
|     PIR     |      19      |
| OLED Model  |   SSD1306    |
| OLED Width  |     128      |
| OLED Height |      64      |
|   IIS SCK   |      26      |
|   IIS WS    |      32      |
|  IIS DOUT   |      33      |

* Note: **PIR** Pin not **RTC IO**, unable to wake from deep sleep

### Programming Notes:
1. When using **T-Camear V16** ,uncomment **T_Camera_V16_VERSION** in **sketch.ino**
1. The following libraries need to be installed to compile
    - [mathertel/OneButton](https://github.com/mathertel/OneButton) 
    - [ThingPulse/esp8266-oled-ssd1306](https://github.com/ThingPulse/esp8266-oled-ssd1306)