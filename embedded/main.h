#ifndef _MAIN_H_
#define _MAIN_H_

#include "lwip/tcp.h"

#define abs(x) ((x < 0) ? (-x) : (x))

// Buffer para respostas HTTP
#define HTTP_RESPONSE  "<!DOCTYPE html>" \
                   "<html>" \
                   "<head>" \
                   "  <meta charset=\"UTF-8\">" \
                   "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">" \
                   "  <title>Controle do LED e Botões</title>" \
                   "  <style>" \
                   "    body {" \
                   "      font-family: Arial, sans-serif;" \
                   "      background-color: #f4f4f9;" \
                   "      color: #333;" \
                   "      margin: 0;" \
                   "      padding: 0;" \
                   "      display: flex;" \
                   "      flex-direction: column;" \
                   "      align-items: center;" \
                   "    }" \
                   "    h1 {" \
                   "      background-color: #4CAF50;" \
                   "      color: white;" \
                   "      padding: 20px;" \
                   "      border-radius: 8px;" \
                   "      margin: 20px 0;" \
                   "      text-align: center;" \
                   "      width: 100%;" \
                   "      max-width: 600px;" \
                   "    }" \
                   "    p {" \
                   "      font-size: 1.2em;" \
                   "      margin: 10px 0;" \
                   "    }" \
                   "    a {" \
                   "      display: inline-block;" \
                   "      text-decoration: none;" \
                   "      background-color: #007BFF;" \
                   "      color: white;" \
                   "      padding: 10px 20px;" \
                   "      border-radius: 5px;" \
                   "      margin: 5px;" \
                   "      font-weight: bold;" \
                   "    }" \
                   "    a:hover {" \
                   "      background-color: #0056b3;" \
                   "    }" \
                   "    .button-state {" \
                   "      border: 1px solid #ddd;" \
                   "      border-radius: 8px;" \
                   "      padding: 15px;" \
                   "      max-width: 600px;" \
                   "      background-color: #fff;" \
                   "      margin: 20px auto;" \
                   "      box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);" \
                   "    }" \
                   "  </style>" \
                   "</head>" \
                   "<body>" \
                   "  <h1>Controle do LED e Botões</h1>" \
                   "  <div>" \
                   "    <p><a href=\"/led/on\">Ligar LED</a></p>" \
                   "    <p><a href=\"/led/off\">Desligar LED</a></p>" \
                   "    <p><a href=\"/update\">Atualizar Estado</a></p>" \
                   "  </div>" \
                   "  <div class=\"button-state\">" \
                   "    <h2>Estado dos Botões:</h2>" \
                   "    <p>Botão A: %s</p>" \
                   "    <p>Botão B: %s</p>" \
                   "  </div>" \
                   "</body>" \
                   "</html>"


static void start_http_server(void);
static void create_http_response();
static err_t http_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
static err_t connection_callback(void *arg, struct tcp_pcb *newpcb, err_t err);
static void monitor_buttons_callback(unsigned int gpio, long unsigned int events);

static void show_intro();
static bool start_network_infrastructure();
static void start_bottons_control();
static bool start_VU_LED();

static bool start_display_oled();
static void init_display_oled();

static bool start_ADC_with_DMA();

// HTTP server hostname
#define PICOHTTPS_HOSTNAME                          "example.edu"

// DNS response polling interval
//
//  Interval with which to poll for responses to DNS queries.
//
#define PICOHTTPS_RESOLVE_POLL_INTERVAL             100             // ms

// Certificate authority root certificate
//
//  CA certificate used to sign the HTTP server's certificate. DER or PEM
//  formats (char array representation).
//
//  This is most readily obtained via inspection of the server's certificate
//  chain, e.g. in a browser.
//
#define PICOHTTPS_CA_ROOT_CERT                          \
{                                                       \
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,     \
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f      \
}
//
//  or
//
//#define PICOHTTPS_CA_ROOT_CERT                                       \
//"-----BEGIN CERTIFICATE-----\n"                                      \
//"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/\n" \
//"-----END CERTIFICATE-----\n"

// TCP + TLS connection establishment polling interval
//
//  Interval with which to poll for establishment of TCP + TLS connection
//
#define PICOHTTPS_ALTCP_CONNECT_POLL_INTERVAL       100             // ms

// TCP + TLS idle connection polling interval
//
//  Interval with which to poll application (i.e. call registered polling
//  callback function) when TCP + TLS connection is idle.
//
//  The callback function should be registered with altcp_poll(). The polling
//  interval is given in units of 'coarse grain timer shots'; one shot
//  corresponds to approximately 500 ms.
//
//  https://www.nongnu.org/lwip/2_1_x/group__altcp.html
//
#define PICOHTTPS_ALTCP_IDLE_POLL_INTERVAL          2               // shots

// HTTP request
//
//  Plain-text HTTP request to send to server
//
#define PICOHTTPS_REQUEST                   \
    "GET / HTTP/1.1\r\n"                    \
    "Host: " PICOHTTPS_HOSTNAME "\r\n"      \
    "\r\n"


// HTTP response polling interval
//
//  Interval with which to poll for HTTP response from server.
//
#define PICOHTTPS_HTTP_RESPONSE_POLL_INTERVAL       100             // ms

// Mbed TLS debug levels
//
//  Seemingly not defined in Mbed TLS‽
//
//  https://github.com/Mbed-TLS/mbedtls/blob/62e79dc913325a18b46aaea554a2836a4e6fc94b/include/mbedtls/debug.h#L141
//
#define PICOHTTPS_MBEDTLS_DEBUG_LEVEL               3


/* Macros *********************************************************************/

// Array length
#define LEN(array) (sizeof array)/(sizeof array[0])



/* Data structures ************************************************************/

// lwIP errors
//
//  typedef here to make source of error code more explicit
//
typedef err_t lwip_err_t;

// Mbed TLS errors
//
//  typedef here to make source of error code more explicit
//
typedef int mbedtls_err_t;

// TCP connection callback argument
//
//  All callbacks associated with lwIP TCP (+ TLS) connections can be passed a
//  common argument. This is intended to allow application state to be accessed
//  from within the callback context. The argument should be registered with
//  altcp_arg().
//
//  The following structure is used for this argument in order to supply all
//  the relevant application state required by the various callbacks.
//
//  https://www.nongnu.org/lwip/2_1_x/group__altcp.html
//
struct altcp_callback_arg{

    // TCP + TLS connection configurtaion
    //
    //  Memory allocated to the connection configuration structure needs to be
    //  freed (with altcp_tls_free_config) in the connection error callback
    //  (callback_altcp_err).
    //
    //  https://www.nongnu.org/lwip/2_1_x/group__altcp.html
    //  https://www.nongnu.org/lwip/2_1_x/group__altcp__tls.html
    //
    struct altcp_tls_config* config;

    // TCP + TLS connection state
    //
    //  Successful establishment of a connection needs to be signaled to the
    //  application from the connection connect callback
    //  (callback_altcp_connect).
    //
    //  https://www.nongnu.org/lwip/2_1_x/group__altcp.html
    //
    bool connected;

    // Data reception acknowledgement
    //
    //  The amount of data acknowledged as received by the server needs to be
    //  communicated to the application from the connection sent callback
    //  (callback_altcp_sent) for validatation of successful transmission.
    //
    u16_t acknowledged;

};

void altcp_free_config(struct altcp_tls_config* config);
bool resolve_hostname(ip_addr_t* ipaddr);
bool connect_to_host(ip_addr_t* ipaddr, struct altcp_pcb** pcb);
bool send_request(struct altcp_pcb* pcb);
bool start_gpio(void);
lwip_err_t callback_altcp_recv(
  void* arg,
  struct altcp_pcb* pcb,
  struct pbuf* buf,
  lwip_err_t err
);
lwip_err_t callback_altcp_connect(
  void* arg,
  struct altcp_pcb* pcb,
  lwip_err_t err
);
lwip_err_t callback_altcp_poll(void* arg, struct altcp_pcb* pcb);
lwip_err_t callback_altcp_sent(void* arg, struct altcp_pcb* pcb, u16_t len);
void callback_gethostbyname(
  const char* name,
  const ip_addr_t* resolved,
  void* ipaddr
);
void callback_altcp_err(void* arg, lwip_err_t err);
void altcp_free_arg(struct altcp_callback_arg* arg);
void altcp_free_pcb(struct altcp_pcb* pcb);
#endif