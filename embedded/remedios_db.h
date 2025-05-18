#ifndef REMEDIOS_DB_H
#define REMEDIOS_DB_H


// Parâmetros e macros do I2C.
#define I2C_SDA  14
#define I2C_SCL  15

#define INTER_SCREEN_DELAY (400)
#define OLED_SCROOL_DELAY (INTER_SCREEN_DELAY / 2.5)


// Pino e número de LEDs da matriz de LEDs.
#define MATRIZ_LED_PIN 7
#define MATRIZ_LED_COUNT 25

// Pino e número de LEDs RGB
#define LED_PIN_RED 11
#define LED_PIN_GREEN 12   
#define LED_PIN_BLUE 13

// Parâmetros botões A e B.
#define BUTTON_A_PIN 5      // Pino do botão A
#define BUTTON_B_PIN 6      // Pino do botão B

// Parâmetros e macros do WiFi
#define WIFI_SSID "ArvoreDosSaberes"
#define WIFI_PASSWORD "Arduino2022"

#define BACKEND_URL "http://192.168.1.100:3000"

#define VERSION "0.1.0"

#endif // REMEDIOS_DB_H