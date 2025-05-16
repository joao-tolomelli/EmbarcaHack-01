/* FreeRTOS includes. */
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <timers.h>
#include <semphr.h>

/* Standard includes. */

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "pico/binary_info.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/clocks.h"

#include "hardware/adc.h"
#include "hardware/dma.h"

#include "hardware/i2c.h"

#include "lwip/tcp.h"

#include "inc/ssd1306.h"

#include "inc/neopixel.h"

// lwIP
#include "lwip/dns.h"               // Hostname resolution
#include "lwip/altcp_tls.h"         // TCP + TLS (+ HTTP == HTTPS)
#include "altcp_tls_mbedtls_structs.h"
#include "lwip/prot/iana.h"         // HTTPS port number

// Mbed TLS
#include "mbedtls/ssl.h"            // Server Name Indication TLS extension
#include "remedios_db.h"
#ifdef MBEDTLS_DEBUG_C
#include "mbedtls/debug.h"          // Mbed TLS debugging
#endif //MBEDTLS_DEBUG_C
#include "mbedtls/check_config.h"

#include "inc/remedios_db.h"   // Carrega parametros gerais do sistema
#include "tasks/tasks_parameters.h" // parametros para criação das tasks

#include "tasks/task_adc_with_dma.h"  // carrega tarefas do ADC
#include "tasks/task_display_oled.h"  // Carrega tarefas do display
#include "tasks/task_drone_control.h" // Carrega tarefas do controle de drone
#include "tasks/task_tinyML.h"        // Carrega tarefas do TinyML
#include "tasks/task_fft_filter.h"    // Carrega tarefas do filtro FFT
#include "tasks/task_http_server.h"   // Carrega tarefas do http server
#include "tasks/task_fft_filter.h"    // Carrega tarefas do filtro FFT
#include "tasks/task_vu_leds.h"       // Carrega tarefas do leds de volume
#include "main.h"                     // carrega cabeçalhos do main

// buffer de texto para o display oled
// definido no arquivo de tarefas do display oled
// tasks/task_display_oled.h
extern char text_line_oled[max_text_lines][max_text_columns];
// Preparar área de renderização para o display (ssd1306_width pixels por ssd1306_n_pages páginas)
extern struct render_area frame_area;
// zera o display inteiro
extern uint8_t ssd[ssd1306_buffer_length];

// Buffer de amostras do ADC.
static uint32_t last_time;
// flag que indica se o controle do drone é local ou remoto
bool modo_local = false;
// aceleração obitida no servidor restfull
int aceleration_x;
int aceleration_y;

QueueHandle_t xFFT_Buffer_Queue = NULL;
QueueHandle_t xTinyML_Buffer_Queue = NULL;
QueueHandle_t xIntensity_Buffer_Queue = NULL;

int main()
{
  stdio_init_all(); // Inicializa os tipos stdio padrão presentes ligados ao binário

  init_display_oled();

  show_intro();

  sleep_ms(INTER_SCREEN_DELAY);

  if (!start_VU_LED())
    return 1;

  sleep_ms(INTER_SCREEN_DELAY);

  start_bottons_control();

  sleep_ms(INTER_SCREEN_DELAY);

  if (!start_ADC_with_DMA())
    return 1;

  sleep_ms(INTER_SCREEN_DELAY);


  if (!start_network_infrastructure())
    return 1;

  sleep_ms(INTER_SCREEN_DELAY);

  if (!start_gpio_and_drone_control())
    return 1;

  sleep_ms(INTER_SCREEN_DELAY);

  if (!start_display_oled())
    return 1;

  sleep_ms(INTER_SCREEN_DELAY);

  strcpy(text_line_oled[0], "               ");
  strcpy(text_line_oled[1], "Inicializando o");
  strcpy(text_line_oled[2], "               ");
  strcpy(text_line_oled[3], "  escalonador  ");
  strcpy(text_line_oled[4], "               ");
  strcpy(text_line_oled[5], "      de       ");
  strcpy(text_line_oled[6], "    rarefas    ");
  strcpy(text_line_oled[7], "               ");

  uint8_t y = 0;
  for (uint i = 0; i < count_of(text_line_oled); i++)
  {
    ssd1306_draw_string(ssd, 5, y, text_line_oled[i]);
    y += ssd1306_line_height;
  }
  render_on_display(ssd, &frame_area);

  vTaskStartScheduler();

  return 0;
}

bool start_display_oled()
{
  BaseType_t xReturn = xTaskCreate(
      task_display_oled,
      "Task que mantem o display OLED atualizado",
      TASK_DISPLAY_OLED_STACK_SIZE,
      NULL,
      TASK_DISPLAY_OLED_PRIORITY,
      NULL);

  if (xReturn != pdPASS)
  {
    strcpy(text_line_oled[0], "     FALHA     ");
    strcpy(text_line_oled[1], " AO CRIAR TASK ");
    strcpy(text_line_oled[2], "      RTOS     ");
    strcpy(text_line_oled[3], "               ");
    strcpy(text_line_oled[4], "               ");
    strcpy(text_line_oled[5], " DISPLAY OLED  ");
    strcpy(text_line_oled[6], "               ");
    strcpy(text_line_oled[7], "               ");

    uint8_t y = 0;
    for (uint i = 0; i < count_of(text_line_oled); i++)
    {
      ssd1306_draw_string(ssd, 5, y, text_line_oled[i]);
      y += ssd1306_line_height;
    }
    render_on_display(ssd, &frame_area);

    sleep_ms(INTER_SCREEN_DELAY * 6);
    return false;
  }
  return true;
}

void init_display_oled()
{
  // Inicialização do i2c
  i2c_init(i2c1, ssd1306_i2c_clock * 1000);
  gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
  gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
  gpio_pull_up(I2C_SDA);
  gpio_pull_up(I2C_SCL);

  // Processo de inicialização completo do OLED SSD1306
  ssd1306_init();

  calculate_render_area_buffer_length(&frame_area);

  memset(ssd, 0, ssd1306_buffer_length);
  render_on_display(ssd, &frame_area);
}
/**
 * Mostra a tela de abertura do projeto.
 */
void show_intro()
{
  strcpy(text_line_oled[0], "  EMBARCAHACK  ");
  strcpy(text_line_oled[1], "               ");
  strcpy(text_line_oled[2], "               ");
  strcpy(text_line_oled[3], "   Residencia  ");
  strcpy(text_line_oled[4], "               ");
  strcpy(text_line_oled[5], "  EMBARCATECH  ");
  strcpy(text_line_oled[6], "     TIC 37    ");
  strcpy(text_line_oled[7], "               ");

  uint8_t y = 0;
  for (uint i = 0; i < count_of(text_line_oled); i++)
  {
    ssd1306_draw_string(ssd, 5, y, text_line_oled[i]);
    y += ssd1306_line_height;
  }
  render_on_display(ssd, &frame_area);

  sleep_ms(INTER_SCREEN_DELAY * 3);

  strcpy(text_line_oled[0], "    SISTEMA    ");
  strcpy(text_line_oled[1], "      DE       ");
  strcpy(text_line_oled[2], "   CONTROLE    ");
  strcpy(text_line_oled[3], "      DE       ");
  strcpy(text_line_oled[4], "  MEDICAMENTOS ");
  strcpy(text_line_oled[5], "               ");
  sprintf(text_line_oled[6], "v%s", VERSION);
  strcpy(text_line_oled[7], "               ");

  y = 0;
  for (uint i = 0; i < count_of(text_line_oled); i++)
  {
    ssd1306_draw_string(ssd, 5, y, text_line_oled[i]);
    y += ssd1306_line_height;
  }
  render_on_display(ssd, &frame_area);

  sleep_ms(INTER_SCREEN_DELAY * 1.25);

  strcpy(text_line_oled[0], "      DE       ");
  strcpy(text_line_oled[1], "   CONTROLE    ");
  strcpy(text_line_oled[2], "      DE       ");
  strcpy(text_line_oled[3], "  MEDICAMENTOS ");
  strcpy(text_line_oled[4], "               ");
  sprintf(text_line_oled[5], "v%s", VERSION);
  strcpy(text_line_oled[6], "               ");
  strcpy(text_line_oled[7], "               ");

  y = 0;
  for (uint i = 0; i < count_of(text_line_oled); i++)
  {
    ssd1306_draw_string(ssd, 5, y, text_line_oled[i]);
    y += ssd1306_line_height;
  }
  render_on_display(ssd, &frame_area);

  sleep_ms(OLED_SCROOL_DELAY);

  strcpy(text_line_oled[0], "   CONTROLE    ");
  strcpy(text_line_oled[1], "      DE       ");
  strcpy(text_line_oled[2], "  MEDICAMENTOS ");
  strcpy(text_line_oled[3], "               ");
  sprintf(text_line_oled[4], "v%s", VERSION);
  strcpy(text_line_oled[5], "               ");
  strcpy(text_line_oled[6], "               ");
  strcpy(text_line_oled[7], "               ");

  y = 0;
  for (uint i = 0; i < count_of(text_line_oled); i++)
  {
    ssd1306_draw_string(ssd, 5, y, text_line_oled[i]);
    y += ssd1306_line_height;
  }
  render_on_display(ssd, &frame_area);

  sleep_ms(OLED_SCROOL_DELAY);

  strcpy(text_line_oled[0], "      DE       ");
  strcpy(text_line_oled[1], "  MEDICAMENTOS ");
  strcpy(text_line_oled[2], "               ");
  sprintf(text_line_oled[3], "v%s", VERSION);
  strcpy(text_line_oled[4], "               ");
  strcpy(text_line_oled[5], "               ");
  strcpy(text_line_oled[6], "               ");
  strcpy(text_line_oled[7], "               ");

  y = 0;
  for (uint i = 0; i < count_of(text_line_oled); i++)
  {
    ssd1306_draw_string(ssd, 5, y, text_line_oled[i]);
    y += ssd1306_line_height;
  }
  render_on_display(ssd, &frame_area);

  sleep_ms(OLED_SCROOL_DELAY);

  strcpy(text_line_oled[0], "  MEDICAMENTOS ");
  strcpy(text_line_oled[1], "               ");
  sprintf(text_line_oled[2], "v%s", VERSION);
  strcpy(text_line_oled[3], "               ");
  strcpy(text_line_oled[4], "               ");
  strcpy(text_line_oled[5], "               ");
  strcpy(text_line_oled[6], "               ");
  strcpy(text_line_oled[7], "               ");

  y = 0;
  for (uint i = 0; i < count_of(text_line_oled); i++)
  {
    ssd1306_draw_string(ssd, 5, y, text_line_oled[i]);
    y += ssd1306_line_height;
  }
  render_on_display(ssd, &frame_area);

  sleep_ms(OLED_SCROOL_DELAY);

  strcpy(text_line_oled[0], "               ");
  sprintf(text_line_oled[1], "v%s", VERSION);
  strcpy(text_line_oled[2], "               ");
  strcpy(text_line_oled[3], "               ");
  strcpy(text_line_oled[4], "               ");
  strcpy(text_line_oled[5], "               ");
  strcpy(text_line_oled[6], "               ");
  strcpy(text_line_oled[7], "               ");

  y = 0;
  for (uint i = 0; i < count_of(text_line_oled); i++)
  {
    ssd1306_draw_string(ssd, 5, y, text_line_oled[i]);
    y += ssd1306_line_height;
  }
  render_on_display(ssd, &frame_area);

  sleep_ms(OLED_SCROOL_DELAY);

  sprintf(text_line_oled[0], "v%s", VERSION);
  strcpy(text_line_oled[1], "               ");
  strcpy(text_line_oled[2], "               ");
  strcpy(text_line_oled[3], "               ");
  strcpy(text_line_oled[4], "               ");
  strcpy(text_line_oled[5], "               ");
  strcpy(text_line_oled[6], "               ");
  strcpy(text_line_oled[7], "               ");

  y = 0;
  for (uint i = 0; i < count_of(text_line_oled); i++)
  {
    ssd1306_draw_string(ssd, 5, y, text_line_oled[i]);
    y += ssd1306_line_height;
  }
  render_on_display(ssd, &frame_area);

  sleep_ms(OLED_SCROOL_DELAY);

  memset(ssd, 0, ssd1306_buffer_length);
  render_on_display(ssd, &frame_area);
}

/**
 * @brief Exibe mensagens de inicializacao e inicializa o ADC com DMA.
 *
 * @details
 *  - Exibe mensagens de inicializacao na tela.
 *  - Inicializa o ADC para capturar amostras do sinal de audio via DMA.
 *
 * @see adc_gpio_init, adc_init, adc_select_input, adc_fifo_setup, adc_set_clkdiv, dma_claim_unused_channel, channel_config_set_transfer_data_size, channel_config_set_read_increment, channel_config_set_write_increment, channel_config_set_dreq, sample_mic
 */
bool start_ADC_with_DMA()
{
  strcpy(text_line_oled[0], "   Ativando    ");
  strcpy(text_line_oled[1], "               ");
  strcpy(text_line_oled[2], "      ADC      ");
  strcpy(text_line_oled[3], "               ");
  strcpy(text_line_oled[4], "    Por DMA    ");
  strcpy(text_line_oled[5], "               ");
  strcpy(text_line_oled[6], "               ");
  strcpy(text_line_oled[7], "               ");

  // ssd1306_write_array(ssd, &frame_area, &text);
  uint8_t y = 0;
  for (uint i = 0; i < count_of(text_line_oled); i++)
  {
    ssd1306_draw_string(ssd, 5, y, text_line_oled[i]);
    y += ssd1306_line_height;
  }
  render_on_display(ssd, &frame_area);

  xIntensity_Buffer_Queue = xQueueCreate(ADC_INTENSITY_QUEUE_LENGTH, (UBaseType_t)sizeof(uint));
  BaseType_t xReturn = xTaskCreate(
      task_adc_with_dma,
      "Task ADC com DMA",
      TASK_ADC_DMA_STACK_SIZE,
      NULL,
      TASK_ADC_DMA_PRIORITY,
      NULL);
  if (xReturn != pdPASS)
  {
    strcpy(text_line_oled[0], "     FALHA     ");
    strcpy(text_line_oled[1], " AO CRIAR TASK ");
    strcpy(text_line_oled[2], "      RTOS     ");
    strcpy(text_line_oled[3], "               ");
    strcpy(text_line_oled[4], "               ");
    strcpy(text_line_oled[5], "  ADC COM DMA  ");
    strcpy(text_line_oled[6], "               ");
    strcpy(text_line_oled[7], "               ");

    uint8_t y = 0;
    for (uint i = 0; i < count_of(text_line_oled); i++)
    {
      ssd1306_draw_string(ssd, 5, y, text_line_oled[i]);
      y += ssd1306_line_height;
    }
    render_on_display(ssd, &frame_area);

    sleep_ms(INTER_SCREEN_DELAY * 6);
    return false;
  }
  return true;
}

/**
 * @brief Exibe mensagens de inicializacao e inicializa o VU de LEDs.
 *
 * @details
 * INicializa o VU de LEDs para que seja possível monitorar o nível de audio
 * captado.
 *
 * @see npInit
 */
bool start_VU_LED()
{
  strcpy(text_line_oled[0], "   Ativando    ");
  strcpy(text_line_oled[1], "               ");
  strcpy(text_line_oled[2], "               ");
  strcpy(text_line_oled[3], "  VU de LEDs   ");
  strcpy(text_line_oled[4], "               ");
  strcpy(text_line_oled[5], "               ");
  strcpy(text_line_oled[6], "               ");
  strcpy(text_line_oled[7], "               ");

  // ssd1306_write_array(ssd, &frame_area, &text);
  uint8_t y = 0;
  for (uint i = 0; i < count_of(text_line_oled); i++)
  {
    ssd1306_draw_string(ssd, 5, y, text_line_oled[i]);
    y += ssd1306_line_height;
  }
  render_on_display(ssd, &frame_area);

  npInit(MATRIZ_LED_PIN, MATRIZ_LED_COUNT);

  printf("VU de LEDs inicializada \n");
  // acente os para diagnostico da task com vu
  npSetLED(12, 0, 0, 80); // Acende apenas o centro.
  printf("Acende apenas o centro.\n");
  npWrite();
  sleep_ms((500));

  npSetLED(12, 0, 0, 120); // Acente o centro.
  printf("Acente o centro.\n");
  
  // Primeiro anel.
  npSetLED(7, 0, 0, 80);
  npSetLED(11, 0, 0, 80);
  npSetLED(13, 0, 0, 80);
  npSetLED(17, 0, 0, 80);
  printf("Primeiro anel.\n");
  npWrite();
  sleep_ms((500));

  // Centro.
  npSetLED(12, 60, 60, 0);
  printf("Centro.\n");

  // Primeiro anel.
  npSetLED(7, 0, 0, 120);
  npSetLED(11, 0, 0, 120);
  npSetLED(13, 0, 0, 120);
  npSetLED(17, 0, 0, 120);
  printf("Primeiro anel.\n");

  // Segundo anel.
  npSetLED(2, 0, 0, 80);
  npSetLED(6, 0, 0, 80);
  npSetLED(8, 0, 0, 80);
  npSetLED(10, 0, 0, 80);
  npSetLED(14, 0, 0, 80);
  npSetLED(16, 0, 0, 80);
  npSetLED(18, 0, 0, 80);
  npSetLED(22, 0, 0, 80);
  printf("Segundo anel.\n");
  npWrite();
  sleep_ms((500));

  // Centro.
  npSetLED(12, 80, 0, 0);
  printf("Centro.\n");

  // Primeiro anel.
  npSetLED(7, 60, 60, 0);
  npSetLED(11, 60, 60, 0);
  npSetLED(13, 60, 60, 0);
  npSetLED(17, 60, 60, 0);
  printf("Primeiro anel.\n");

  // Segundo anel.
  npSetLED(2, 0, 0, 120);
  npSetLED(6, 0, 0, 120);
  npSetLED(8, 0, 0, 120);
  npSetLED(10, 0, 0, 120);
  npSetLED(14, 0, 0, 120);
  npSetLED(16, 0, 0, 120);
  npSetLED(18, 0, 0, 120);
  npSetLED(22, 0, 0, 120);
  printf("Segundo anel.\n");

  // Terceiro anel.
  npSetLED(1, 0, 0, 80);
  npSetLED(3, 0, 0, 80);
  npSetLED(5, 0, 0, 80);
  npSetLED(9, 0, 0, 80);
  npSetLED(15, 0, 0, 80);
  npSetLED(19, 0, 0, 80);
  npSetLED(21, 0, 0, 80);
  npSetLED(23, 0, 0, 80);
  printf("Terceiro anel.\n");
  npWrite();
  sleep_ms((500));

  npClear();
  npWrite();

  BaseType_t xReturn = xTaskCreate(
      task_vu_leds,
      "Task de VU LEDs",
      TASK_VU_LEDS_STACK_SIZE,
      NULL,
      TASK_VU_LEDS_PRIORITY,
      NULL);

  if (xReturn != pdPASS)
  {
    strcpy(text_line_oled[0], "     FALHA     ");
    strcpy(text_line_oled[1], " AO CRIAR TASK ");
    strcpy(text_line_oled[2], "      RTOS     ");
    strcpy(text_line_oled[3], "               ");
    strcpy(text_line_oled[4], "               ");
    strcpy(text_line_oled[5], "     VU LEDs   ");
    strcpy(text_line_oled[6], "               ");
    strcpy(text_line_oled[7], "               ");

    uint8_t y = 0;
    for (uint i = 0; i < count_of(text_line_oled); i++)
    {
      ssd1306_draw_string(ssd, 5, y, text_line_oled[i]);
      y += ssd1306_line_height;
    }
    render_on_display(ssd, &frame_area);

    return false;
  }
  return true;
}

/**
 * @brief Exibe mensagens de inicializacao e configura os botoes A e B
 *        para serem monitorados por interrupcoes.
 *
 * @details
 *  - Exibe mensagens de inicializacao na tela.
 *  - Configura os botoes A e B para serem monitorados por interrupcoes.
 *
 * @see task_buttons_control
 */
void start_bottons_control()
{
  strcpy(text_line_oled[0], "   Ativando    ");
  strcpy(text_line_oled[1], "               ");
  strcpy(text_line_oled[2], "               ");
  strcpy(text_line_oled[3], " configuracoes ");
  strcpy(text_line_oled[4], "               ");
  strcpy(text_line_oled[5], "  dos botoes   ");
  strcpy(text_line_oled[6], "               ");
  strcpy(text_line_oled[7], "               ");

  // ssd1306_write_array(ssd, &frame_area, &text);
  uint8_t y = 0;
  for (uint i = 0; i < count_of(text_line_oled); i++)
  {
    ssd1306_draw_string(ssd, 5, y, text_line_oled[i]);
    y += ssd1306_line_height;
  }
  render_on_display(ssd, &frame_area);

  gpio_init(BUTTON_A_PIN);
  gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
  gpio_pull_up(BUTTON_A_PIN);
  gpio_set_irq_enabled_with_callback(BUTTON_A_PIN, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &monitor_buttons_callback);

  strcpy(text_line_oled[7], "   Botao A   ");

  // ssd1306_write_array(ssd, &frame_area, &text);
  y = 0;
  for (uint i = 0; i < count_of(text_line_oled); i++)
  {
    ssd1306_draw_string(ssd, 5, y, text_line_oled[i]);
    y += ssd1306_line_height;
  }
  render_on_display(ssd, &frame_area);

  sleep_ms(500);
  gpio_init(BUTTON_B_PIN);
  gpio_set_dir(BUTTON_B_PIN, GPIO_IN);
  gpio_pull_up(BUTTON_B_PIN);
  gpio_set_irq_enabled_with_callback(BUTTON_B_PIN, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &monitor_buttons_callback);

  strcpy(text_line_oled[7], "   Botao B   ");

  // ssd1306_write_array(ssd, &frame_area, &text);
  y = 0;
  for (uint i = 0; i < count_of(text_line_oled); i++)
  {
    ssd1306_draw_string(ssd, 5, y, text_line_oled[i]);
    y += ssd1306_line_height;
  }
  render_on_display(ssd, &frame_area);
}

/**
 * Inicia a infraestrutura de rede.
 */
bool start_network_infrastructure()
{
  if (cyw43_arch_init_with_country(CYW43_COUNTRY_BRAZIL))
  {
    strcpy(text_line_oled[0], "    ATENCAO    ");
    strcpy(text_line_oled[1], "    FALHA NA   ");
    strcpy(text_line_oled[2], "               ");
    strcpy(text_line_oled[3], " Infraestrutra ");
    strcpy(text_line_oled[4], "    de Rede    ");
    strcpy(text_line_oled[5], "               ");
    strcpy(text_line_oled[6], "     Wi-FI     ");
    strcpy(text_line_oled[7], "               ");

    uint8_t y = 0;
    for (uint i = 0; i < count_of(text_line_oled); i++)
    {
      ssd1306_draw_string(ssd, 5, y, text_line_oled[i]);
      y += ssd1306_line_height;
    }
    render_on_display(ssd, &frame_area);
    return false;
  }

  cyw43_arch_enable_sta_mode();
  uint8_t count = 0;
  while (count < 3 && cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 10000) != 0)
  {
    strcpy(text_line_oled[0], "    ATENCAO    ");
    strcpy(text_line_oled[1], "    FALHA NA   ");
    strcpy(text_line_oled[2], "               ");
    strcpy(text_line_oled[3], " Infraestrutra ");
    strcpy(text_line_oled[4], "    de Rede    ");
    strcpy(text_line_oled[5], "     Wi-FI     ");
    strcpy(text_line_oled[6], " NAO CONECTADO ");
    sprintf(text_line_oled[7], " Tentativa: %02d", ++count);
    uint8_t y = 0;
    for (uint i = 0; i < count_of(text_line_oled); i++)
    {
      ssd1306_draw_string(ssd, 5, y, text_line_oled[i]);
      y += ssd1306_line_height;
    }
    render_on_display(ssd, &frame_area);
    sleep_ms(INTER_SCREEN_DELAY / (1.0 / 3));
  }

  if (count == 3)
    return false;

  // Read the ip address in a human readable way
  uint8_t *ip_address = (uint8_t *)&(cyw43_state.netif[0].ip_addr.addr);
  strcpy(text_line_oled[0], " INICIALIZANDO ");
  strcpy(text_line_oled[1], "               ");
  strcpy(text_line_oled[3], " Infraestrutra ");
  strcpy(text_line_oled[4], "    de Rede    ");
  strcpy(text_line_oled[5], "     Wi-FI     ");
  strcpy(text_line_oled[6], "   CONECTADO   ");
  strcpy(text_line_oled[2], "               ");
  sprintf(text_line_oled[7], "%03d.%03d.%03d.%03d", ip_address[0], ip_address[1], ip_address[2], ip_address[3]);
  uint8_t y = 0;
  for (uint i = 0; i < count_of(text_line_oled); i++)
  {
    ssd1306_draw_string(ssd, 5, y, text_line_oled[i]);
    y += ssd1306_line_height;
  }
  render_on_display(ssd, &frame_area);

  // Inicia o servidor HTTP
  start_http_server();
  BaseType_t xReturn = xTaskCreate(
      task_http_server,
      "Task HTTP Server",
      TASK_HTTP_SERVER_STACK_SIZE,
      NULL,
      TASK_HTTP_SERVER_PRIORITY,
      NULL);

  if (xReturn != pdPASS)
  {
    strcpy(text_line_oled[0], "     FALHA     ");
    strcpy(text_line_oled[1], " AO CRIAR TASK ");
    strcpy(text_line_oled[2], "      RTOS     ");
    strcpy(text_line_oled[3], "               ");
    strcpy(text_line_oled[4], "               ");
    strcpy(text_line_oled[5], " INFRAESTRUTURA");
    strcpy(text_line_oled[6], "     REDE      ");
    strcpy(text_line_oled[7], "               ");

    uint8_t y = 0;
    for (uint i = 0; i < count_of(text_line_oled); i++)
    {
      ssd1306_draw_string(ssd, 5, y, text_line_oled[i]);
      y += ssd1306_line_height;
    }
    render_on_display(ssd, &frame_area);

    sleep_ms(INTER_SCREEN_DELAY * 6);
    return false;
  }

  // Resolve server hostname
  char* char_ipaddr;
  printf("Resolving %s\n", BACKEND_URL);
  if(!resolve_hostname(&ipaddr)){
      printf("Failed to resolve %s\n", BACKEND_URL);
      cyw43_arch_deinit();            // Deinit Pico W wireless hardware
      strcpy(text_line_oled[0], "     FALHA     ");
      strcpy(text_line_oled[1], "  AO RESOLVER  ");
      sprintf(text_line_oled[2], "%s", BACKEND_URL);
      strcpy(text_line_oled[3], "               ");
      strcpy(text_line_oled[4], "               ");
      strcpy(text_line_oled[5], " INFRAESTRUTURA");
      strcpy(text_line_oled[6], "     REDE      ");
      strcpy(text_line_oled[7], "               ");

      uint8_t y = 0;
      for (uint i = 0; i < count_of(text_line_oled); i++)
      {
        ssd1306_draw_string(ssd, 5, y, text_line_oled[i]);
        y += ssd1306_line_height;
      }
      render_on_display(ssd, &frame_area);

      sleep_ms(INTER_SCREEN_DELAY * 6);
      return false;
  }

  cyw43_arch_lwip_begin();
  char_ipaddr = ipaddr_ntoa(&ipaddr);
  cyw43_arch_lwip_end();
  printf("Resolved %s (%s)\n", PICOHTTPS_HOSTNAME, char_ipaddr);
  strcpy(text_line_oled[0], "   RESOLVIDO   ");
  strcpy(text_line_oled[1], " NOME BACKEND  ");
  strcpy(text_line_oled[2], "               ");
  sprintf(text_line_oled[3], "%s", BACKEND_URL);
  strcpy(text_line_oled[4], "               ");
  strcpy(text_line_oled[5], " INFRAESTRUTURA");
  strcpy(text_line_oled[6], "     REDE      ");
  strcpy(text_line_oled[7], "               ");

  uint8_t y = 0;
  for (uint i = 0; i < count_of(text_line_oled); i++)
  {
    ssd1306_draw_string(ssd, 5, y, text_line_oled[i]);
    y += ssd1306_line_height;
  }
  render_on_display(ssd, &frame_area);

  // Establish TCP + TLS connection with server
#ifdef MBEDTLS_DEBUG_C
  mbedtls_debug_set_threshold(PICOHTTPS_MBEDTLS_DEBUG_LEVEL);
#endif //MBEDTLS_DEBUG_C
  struct altcp_pcb* pcb = NULL;
  printf("Connecting to https://%s:%d\n", char_ipaddr, LWIP_IANA_PORT_HTTPS);
  if(!connect_to_host(&ipaddr, &pcb)){
    printf("Failed to connect to https://%s:%d\n", char_ipaddr, LWIP_IANA_PORT_HTTPS);
                              // TODO: Disconnect from network
    cyw43_arch_deinit();            // Deinit Pico W wireless hardware
    return false;
  }
  printf("Connected to https://%s:%d\n", char_ipaddr, LWIP_IANA_PORT_HTTPS);

  // Send HTTP request to server
  printf("Sending request\n");
  if(!send_request(pcb)){
    printf("Failed to send request\n");
    altcp_free_config(              // Free connection configuration
      ((struct altcp_callback_arg*)(pcb->arg))->config
    );
    altcp_free_arg(                 // Free connection callback argument
      (struct altcp_callback_arg*)(pcb->arg)
    );
    altcp_free_pcb(pcb);            // Free connection PCB
                                    // TODO: Disconnect from network
    cyw43_arch_deinit();            // Deinit Pico W wireless hardware
    return;
  }
  printf("Request sent\n");

  // Await HTTP response
  printf("Awaiting response\n");
  sleep_ms(5000);
  printf("Awaited response\n");

  return true;
}

// #########################################
//                WIFI e WEB Server

// Função de setup do servidor TCP
static void start_http_server(void)
{
  struct tcp_pcb *pcb = tcp_new();
  if (!pcb)
  {
    strcpy(text_line_oled[0], " INICIALIZANDO ");
    strcpy(text_line_oled[1], "               ");
    strcpy(text_line_oled[2], " Infraestrutra ");
    strcpy(text_line_oled[3], "    de Rede    ");
    strcpy(text_line_oled[4], "     Wi-FI     ");
    strcpy(text_line_oled[5], "               ");
    strcpy(text_line_oled[6], " Erro criando  ");
    ;
    strcpy(text_line_oled[7], "      PCB      ");

    uint8_t y = 0;
    for (uint i = 0; i < count_of(text_line_oled); i++)
    {
      ssd1306_draw_string(ssd, 5, y, text_line_oled[i]);
      y += ssd1306_line_height;
    }
    render_on_display(ssd, &frame_area);
    return;
  }

  // Liga o servidor na porta 80
  if (tcp_bind(pcb, IP_ADDR_ANY, 80) != ERR_OK)
  {
    strcpy(text_line_oled[0], " INICIALIZANDO ");
    strcpy(text_line_oled[1], "               ");
    strcpy(text_line_oled[2], " Infraestrutra ");
    strcpy(text_line_oled[3], "    de Rede    ");
    strcpy(text_line_oled[4], "     Wi-FI     ");
    strcpy(text_line_oled[5], "               ");
    strcpy(text_line_oled[6], " Erro conectar ");
    ;
    strcpy(text_line_oled[7], " serv. porta 80");

    uint8_t y = 0;
    for (uint i = 0; i < count_of(text_line_oled); i++)
    {
      ssd1306_draw_string(ssd, 5, y, text_line_oled[i]);
      y += ssd1306_line_height;
    }
    render_on_display(ssd, &frame_area);
    return;
  }

  pcb = tcp_listen(pcb);                // Coloca o PCB em modo de escuta
  tcp_accept(pcb, connection_callback); // Associa o callback de conexão

  strcpy(text_line_oled[0], " INICIALIZANDO ");
  strcpy(text_line_oled[1], "               ");
  strcpy(text_line_oled[2], " Infraestrutra ");
  strcpy(text_line_oled[3], "    de Rede    ");
  strcpy(text_line_oled[4], "     Wi-FI     ");
  strcpy(text_line_oled[5], "               ");
  strcpy(text_line_oled[6], " SERRVIDOR WEB ");
  ;
  strcpy(text_line_oled[7], "   CONECTADO   ");

  uint8_t y = 0;
  for (uint i = 0; i < count_of(text_line_oled); i++)
  {
    ssd1306_draw_string(ssd, 5, y, text_line_oled[i]);
    y += ssd1306_line_height;
  }
  render_on_display(ssd, &frame_area);
}

// Estado dos botões (inicialmente sem mensagens)
char button_A_message[50] = "Nenhum evento no botão 1";
char button_B_message[50] = "Nenhum evento no botão 2";

// Buffer para resposta HTTP
char http_response[1024];
char json_response[1024 * 8]; // tamanho a ser definido

/**
 * Função para criar a resposta JSON
 */

void create_json_update_response()
{
  // constroi a respsota json
  // a resposta deve conter a situação atual do sistema
  // isso inclue a localização obtida no gps do drone
  // aquecimento interno do processador
  // e a situação do acoplador acustico
  snprintf(http_response, sizeof(http_response),
           "HTTP/1.1 200 OK\r\nContent-Type: text/json; charset=UTF-8\r\n\r\n"
           "{\r\n"
           // em data vai os valores para cada amostra obtida
           "  \"localizacao\": {\r\n "
           "    \"latitude\": 0, \"longitude\": 0},\r\n"
           "  \"temperatura\": 0,\r\n"
           "  \"acoplador\": 0,\r\n"
           "}\r\n"
           "}\r\n"
           "\r\n");
}
void create_json_noise_response()
{
  /// constroi resposta em sjson,
  /// talvez cjson, ou uma solução mais simples já que será enviados
  /// apenas uma sequência de números que representa as amostragem do ADC
  snprintf(http_response, sizeof(http_response),
           "HTTP/1.1 200 OK\r\nContent-Type: text/json; charset=UTF-8\r\n\r\n"
           "{\r\n"
           // em data vai os valores para cada amostra obtida
           "  \"amostragens\": {\r\n "
           "    \"order\": 0, \"data\":[123, 123, 123, 123, 123, 123, 123, 123, 123, 123]},\r\n"
           "    \"order\": 1, \"data\":[43, 43, 43, 43, 43, 43, 43, 43, 43, 43]},\r\n"
           "    \"order\": 2, \"data\":[43, 43, 43, 43, 43, 43, 43, 43, 43, 43]},\r\n"
           "    \"order\": 3, \"data\":[43, 43, 43, 43, 43, 43, 43, 43, 43, 43]},\r\n"
           "  }\r\n"
           "}\r\n"
           "\r\n");
}

/**
 * Função para criar a resposta HTTP generica
 */
void create_http_response()
{
  snprintf(http_response, sizeof(http_response), HTTP_RESPONSE,
           button_A_message, button_B_message);
}

/**
 * Função para processar (callback) as requisições HTTP
 */
static err_t http_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
  if (p == NULL)
  {
    // Cliente fechou a conexão
    tcp_close(tpcb);
    return ERR_OK;
  }

  // Processa a requisição HTTP
  char *request = (char *)p->payload;

  // Verifica qual é a requisição
  if (strstr(request, "GET /acustic_angle/"))
  {
    /**
     * ajusta o angulo de captação do acoplador acustico
     */
  }
  else if (strstr(request, "POST /aceleration/xy"))
  {
    if (!modo_local)
    {
      // obtem na requisição a posição x e y desejada
      // aceleration_x = x;
      // aceleration_y = y;
    }

    // Envia a resposta HTTP
    create_http_response();
    tcp_write(tpcb, http_response, strlen(http_response), TCP_WRITE_FLAG_COPY);
  }
  else if (strstr(request, "POST /aceleration/x"))
  {
    // obtem na requisição o valor do eixo X
    if (!modo_local)
    {
      // obtem na requisição a posição x desejada
      // aceleration_x = x;
    }

    // Envia a resposta HTTP
    create_http_response();
    tcp_write(tpcb, http_response, strlen(http_response), TCP_WRITE_FLAG_COPY);
  }
  else if (strstr(request, "POST /aceleration/y"))
  {
    // obtem na requisição o valor do eixo Y
    if (!modo_local)
    {
      // obtem na requisição a posição y desejada
      // aceleration_y = y;
    }

    // Envia a resposta HTTP
    create_http_response();
    tcp_write(tpcb, http_response, strlen(http_response), TCP_WRITE_FLAG_COPY);
  }
  else if (strstr(request, "GET /update"))
  {
    // deve enviar a situação do drone
    // Envia a resposta HTTP
    create_json_update_response();
    tcp_write(tpcb, json_response, strlen(json_response), TCP_WRITE_FLAG_COPY);
  }
  else if (strstr(request, "GET /noise"))
  {
    // Envia a resposta HTTP
    // obtem o array de amostras de ruidos coletados

    create_json_noise_response();
    tcp_write(tpcb, json_response, strlen(json_response), TCP_WRITE_FLAG_COPY);
  }

  // Libera o buffer recebido
  pbuf_free(p);

  return ERR_OK;
}

// Callback de conexão: associa o http_callback à conexão
static err_t connection_callback(void *arg, struct tcp_pcb *newpcb, err_t err)
{
  tcp_recv(newpcb, http_callback); // Associa o callback HTTP
  return ERR_OK;
}

// Função para monitorar o estado dos botões
static void monitor_buttons_callback(unsigned int gpio, long unsigned int events)
{
  static bool button_A_last_state = false;
  static bool button_B_last_state = false;

  bool button_A_state = !gpio_get(BUTTON_A_PIN); // Botão pressionado = LOW
  bool button_B_state = !gpio_get(BUTTON_B_PIN);

  if (button_A_state != button_A_last_state)
  {
    button_A_last_state = button_A_state;
    if (button_A_state)
    {
      snprintf(button_A_message, sizeof(button_A_message), "Botão A foi pressionado!");
      uint8_t *ip_address = (uint8_t *)&(cyw43_state.netif[0].ip_addr.addr);
      sprintf(text_line_oled[7], "%03d.%03d.%03d.%03d", ip_address[0], ip_address[1], ip_address[2], ip_address[3]);
    }
    else
    {
      snprintf(button_A_message, sizeof(button_A_message), "Botão A foi solto!");
      strcpy(text_line_oled[7], "               ");
    }
    uint y = 0;
  }

  if (button_B_state != button_B_last_state)
  {
    button_B_last_state = button_B_state;
    if (button_B_state)
    {
      snprintf(button_B_message, sizeof(button_B_message), "Botão B foi pressionado!");
      modo_local = !modo_local;
      if (modo_local)
        strcpy(text_line_oled[7], "   modo local  ");
      else
        strcpy(text_line_oled[7], "  modo remoto  ");
    }
    else
    {
      snprintf(button_B_message, sizeof(button_B_message), "Botão B foi solto!");
    }
  }
}

// Resolve hostname
bool resolve_hostname(ip_addr_t* ipaddr){

  // Zero address
  ipaddr->addr = IPADDR_ANY;

  // Attempt resolution
  cyw43_arch_lwip_begin();
  lwip_err_t lwip_err = dns_gethostbyname(
      PICOHTTPS_HOSTNAME,
      ipaddr,
      callback_gethostbyname,
      ipaddr
  );
  cyw43_arch_lwip_end();
  if(lwip_err == ERR_INPROGRESS){

      // Await resolution
      //
      //  IP address will be made available shortly (by callback) upon DNS
      //  query response.
      //
      while(ipaddr->addr == IPADDR_ANY)
          sleep_ms(PICOHTTPS_RESOLVE_POLL_INTERVAL);
      if(ipaddr->addr != IPADDR_NONE)
          lwip_err = ERR_OK;

  }

  // Return
  return !((bool)lwip_err);

}

// Free TCP + TLS protocol control block
void altcp_free_pcb(struct altcp_pcb* pcb){
  cyw43_arch_lwip_begin();
  lwip_err_t lwip_err = altcp_close(pcb);         // Frees PCB
  cyw43_arch_lwip_end();
  while(lwip_err != ERR_OK)
      sleep_ms(PICOHTTPS_ALTCP_CONNECT_POLL_INTERVAL);
      cyw43_arch_lwip_begin();
      lwip_err = altcp_close(pcb);                // Frees PCB
      cyw43_arch_lwip_end();
}

// Free TCP + TLS connection configuration
void altcp_free_config(struct altcp_tls_config* config){
  cyw43_arch_lwip_begin();
  altcp_tls_free_config(config);
  cyw43_arch_lwip_end();
}

// Free TCP + TLS connection callback argument
void altcp_free_arg(struct altcp_callback_arg* arg){
  if(arg){
      free(arg);
  }
}

// Establish TCP + TLS connection with server
bool connect_to_host(ip_addr_t* ipaddr, struct altcp_pcb** pcb){

  // Instantiate connection configuration
  u8_t ca_cert[] = PICOHTTPS_CA_ROOT_CERT;
  cyw43_arch_lwip_begin();
  struct altcp_tls_config* config = altcp_tls_create_config_client(
      ca_cert,
      LEN(ca_cert)
  );
  cyw43_arch_lwip_end();
  if(!config) return false;

  // Instantiate connection PCB
  //
  //  Can also do this more generically using;
  //
  //    altcp_allocator_t allocator = {
  //      altcp_tls_alloc,       // Allocator function
  //      config                 // Allocator function argument (state)
  //    };
  //    altcp_new(&allocator);
  //
  //  No benefit in doing this though; altcp_tls_alloc calls altcp_tls_new
  //  under the hood anyway.
  //
  cyw43_arch_lwip_begin();
  *pcb = altcp_tls_new(config, IPADDR_TYPE_V4);
  cyw43_arch_lwip_end();
  if(!(*pcb)){
      altcp_free_config(config);
      return false;
  }

  // Configure hostname for Server Name Indication extension
  //
  //  Many servers nowadays require clients to support the [Server Name
  //  Indication[wiki-sni] (SNI) TLS extension. In this extension, the
  //  hostname is included in the in the ClientHello section of the TLS
  //  handshake.
  //
  //  Mbed TLS provides client-side support for SNI extension
  //  (`MBEDTLS_SSL_SERVER_NAME_INDICATION` option), but requires the
  //  hostname in order to do so. Unfortunately, the Mbed TLS port supplied
  //  with lwIP (ALTCP TLS) does not currently provide an interface to pass
  //  the hostname to Mbed TLS. This is a [known issue in lwIP][gh-lwip-pr].
  //
  //  As a workaround, the hostname can instead be set using the underlying
  //  Mbed TLS interface (viz. `mbedtls_ssl_set_hostname` function). This is
  //  somewhat inelegant as it tightly couples our application code to the
  //  underlying TLS library (viz. Mbed TLS). Given that the Pico SDK already
  //  tightly couples us to lwIP though, and that any fix is unlikely to be
  //  backported to the lwIP version in the Pico SDK, this doesn't feel like
  //  too much of a crime…
  //
  //  [wiki-sni]: https://en.wikipedia.org/wiki/Server_Name_Indication
  //  [gh-lwip-pr]: https://github.com/lwip-tcpip/lwip/pull/47/commits/c53c9d02036be24a461d2998053a52991e65b78e
  //
  cyw43_arch_lwip_begin();
  mbedtls_err_t mbedtls_err = mbedtls_ssl_set_hostname(
      &(
          (
              (altcp_mbedtls_state_t*)((*pcb)->state)
          )->ssl_context
      ),
      PICOHTTPS_HOSTNAME
  );
  cyw43_arch_lwip_end();
  if(mbedtls_err){
      altcp_free_pcb(*pcb);
      altcp_free_config(config);
      return false;
  }

  // Configure common argument for connection callbacks
  //
  //  N.b. callback argument must be in scope in callbacks. As callbacks may
  //  fire after current function returns, cannot declare argument locally,
  //  but rather should allocate on the heap. Must then ensure allocated
  //  memory is subsequently freed.
  //
  struct altcp_callback_arg* arg = malloc(sizeof(*arg));
  if(!arg){
      altcp_free_pcb(*pcb);
      altcp_free_config(config);
      return false;
  }
  arg->config = config;
  arg->connected = false;
  cyw43_arch_lwip_begin();
  altcp_arg(*pcb, (void*)arg);
  cyw43_arch_lwip_end();

  // Configure connection fatal error callback
  cyw43_arch_lwip_begin();
  altcp_err(*pcb, callback_altcp_err);
  cyw43_arch_lwip_end();

  // Configure idle connection callback (and interval)
  cyw43_arch_lwip_begin();
  altcp_poll(
      *pcb,
      callback_altcp_poll,
      PICOHTTPS_ALTCP_IDLE_POLL_INTERVAL
  );
  cyw43_arch_lwip_end();

  // Configure data acknowledge callback
  cyw43_arch_lwip_begin();
  altcp_sent(*pcb, callback_altcp_sent);
  cyw43_arch_lwip_end();

  // Configure data reception callback
  cyw43_arch_lwip_begin();
  altcp_recv(*pcb, callback_altcp_recv);
  cyw43_arch_lwip_end();

  // Send connection request (SYN)
  cyw43_arch_lwip_begin();
  lwip_err_t lwip_err = altcp_connect(
      *pcb,
      ipaddr,
      LWIP_IANA_PORT_HTTPS,
      callback_altcp_connect
  );
  cyw43_arch_lwip_end();

  // Connection request sent
  if(lwip_err == ERR_OK){

      // Await connection
      //
      //  Sucessful connection will be confirmed shortly in
      //  callback_altcp_connect.
      //
      while(!(arg->connected))
          sleep_ms(PICOHTTPS_ALTCP_CONNECT_POLL_INTERVAL);

  } else {

      // Free allocated resources
      altcp_free_pcb(*pcb);
      altcp_free_config(config);
      altcp_free_arg(arg);

  }

  // Return
  return !((bool)lwip_err);

}

// Send HTTP request
bool send_request(struct altcp_pcb* pcb){

  const char request[] = PICOHTTPS_REQUEST;

  // Check send buffer and queue length
  //
  //  Docs state that altcp_write() returns ERR_MEM on send buffer too small
  //  _or_ send queue too long. Could either check both before calling
  //  altcp_write, or just handle returned ERR_MEM — which is preferable?
  //
  //if(
  //  altcp_sndbuf(pcb) < (LEN(request) - 1)
  //  || altcp_sndqueuelen(pcb) > TCP_SND_QUEUELEN
  //) return -1;

  // Write to send buffer
  cyw43_arch_lwip_begin();
  lwip_err_t lwip_err = altcp_write(pcb, request, LEN(request) -1, 0);
  cyw43_arch_lwip_end();

  // Written to send buffer
  if(lwip_err == ERR_OK){

      // Output send buffer
      ((struct altcp_callback_arg*)(pcb->arg))->acknowledged = 0;
      cyw43_arch_lwip_begin();
      lwip_err = altcp_output(pcb);
      cyw43_arch_lwip_end();

      // Send buffer output
      if(lwip_err == ERR_OK){

          // Await acknowledgement
          while(
              !((struct altcp_callback_arg*)(pcb->arg))->acknowledged
          ) sleep_ms(PICOHTTPS_HTTP_RESPONSE_POLL_INTERVAL);
          if(
              ((struct altcp_callback_arg*)(pcb->arg))->acknowledged
              != (LEN(request) - 1)
          ) lwip_err = -1;

      }

  }

  // Return
  return !((bool)lwip_err);

}

// DNS response callback
void callback_gethostbyname(
  const char* name,
  const ip_addr_t* resolved,
  void* ipaddr
){
  if(resolved) *((ip_addr_t*)ipaddr) = *resolved;         // Successful resolution
  else ((ip_addr_t*)ipaddr)->addr = IPADDR_NONE;          // Failed resolution
}

// TCP + TLS connection error callback
void callback_altcp_err(void* arg, lwip_err_t err){

  // Print error code
  printf("Connection error [lwip_err_t err == %d]\n", err);

  // Free ALTCP TLS config
  if( ((struct altcp_callback_arg*)arg)->config )
      altcp_free_config( ((struct altcp_callback_arg*)arg)->config );

  // Free ALTCP callback argument
  altcp_free_arg((struct altcp_callback_arg*)arg);

}

// TCP + TLS connection idle callback
lwip_err_t callback_altcp_poll(void* arg, struct altcp_pcb* pcb){
  // Callback not currently used
  return ERR_OK;
}

// TCP + TLS data acknowledgement callback
lwip_err_t callback_altcp_sent(void* arg, struct altcp_pcb* pcb, u16_t len){
  ((struct altcp_callback_arg*)arg)->acknowledged = len;
  return ERR_OK;
}

// TCP + TLS data reception callback
lwip_err_t callback_altcp_recv(
  void* arg,
  struct altcp_pcb* pcb,
  struct pbuf* buf,
  lwip_err_t err
){

  // Store packet buffer at head of chain
  //
  //  Required to free entire packet buffer chain after processing.
  //
  struct pbuf* head = buf;

  switch(err){

      // No error receiving
      case ERR_OK:

          // Handle packet buffer chain
          //
          //  * buf->tot_len == buf->len — Last buf in chain
          //    * && buf->next == NULL — last buf in chain, no packets in queue
          //    * && buf->next != NULL — last buf in chain, more packets in queue
          //

          if(buf){

              // Print packet buffer
              u16_t i;
              while(buf->len != buf->tot_len){
                  for(i = 0; i < buf->len; i++) putchar(((char*)buf->payload)[i]);
                  buf = buf->next;
              }
              for(i = 0; i < buf->len; i++) putchar(((char*)buf->payload)[i]);
              assert(buf->next == NULL);

              // Advertise data reception
              altcp_recved(pcb, head->tot_len);

          }

          // …fall-through…

      case ERR_ABRT:

          // Free buf
          pbuf_free(head);        // Free entire pbuf chain

          // Reset error
          err = ERR_OK;           // Only return ERR_ABRT when calling tcp_abort()

  }

  // Return error
  return err;

}

// TCP + TLS connection establishment callback
lwip_err_t callback_altcp_connect(
  void* arg,
  struct altcp_pcb* pcb,
  lwip_err_t err
){
  ((struct altcp_callback_arg*)arg)->connected = true;
  return ERR_OK;
}