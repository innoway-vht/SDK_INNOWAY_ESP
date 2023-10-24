#ifndef NB_INNOWAY_H_
#define NB_INNOWAY_H_

#include <stdbool.h>

#include "esp_err.h"
#include "esp_event.h"
#include "esp_event_base.h"

#include "driver/uart.h"

#define TIMER_ATC_PERIOD 	100

#define BROKER				"mqtt.innoway.vn"
#define BAUDRATE			115200
#define RX_BUF_SIZE 		4096
#define TX_BUF_SIZE			1024

ESP_EVENT_DECLARE_BASE(NB_QUECTEL_MODE_EVENTS);

enum
{
	UNKNOW,
	NORMAL,
	DEEPSLEEP,
	LIGHTSLEEP,
	NETWORK_REGISTERED,
	NETWORK_NOT_REGISTERED,
};

ESP_EVENT_DECLARE_BASE(NB_MQTT_INNOWAY_EVENTS);

enum
{
	NB_MQTT_INNOWAY_CONNECTED,
	NB_MQTT_INNOWAY_DISCONNECTED,
	NB_MQTT_INNOWAY_SUBSCRIBED,
	NB_MQTT_INNOWAY_PUBLISHED,
	NB_MQTT_INNOWAY_UNSUBSCRIBED,
	NB_MQTT_INNOWAY_DATA,
	NB_MQTT_INNOWAY_ERROR,
};

typedef struct{
	char host[128];
	char client_id[128];
	char username[128];
	char password[128];
	uint32_t    port;
	int         keepalive;
	uint32_t	msg_id;
	uint8_t		tcp_connect_id;
	esp_event_handler_t nb_mqtt_innoway_event_handler;
}nb_innoway_mqtt_client_t;

/* NB-IoT Quectel BC660K-GL ESP32 APIs*/

esp_err_t nb_quectel_innoway_init(uart_port_t uart_num, int tx_io_num, int rx_io_num);
const char* nb_quectel_get_firmware_version();

/* Response: +CREG: <n>,<stat>
 * STAT: Integer type
 * 0 = Not registered, MT is not currently searching a new operator to register to
 * 1 = Registered, home network
 * 2 = Not registered, but MT is searching a new operator to register to
 * 3 = Registration denied
 * 4 = Unknown. (for example, out GERAN/UTRAN/E-UTRAN coverage)
 * 5 = Registered, roaming

   // Reply is:
   // +CEREG: <n>,<stat>
   //
   // OK
    *
    */
uint8_t nb_quectel_get_status_code();
bool nb_quectel_is_registered();
bool nb_quectel_set_mode();

//const char* nb_quectel_get_date_time();
//int8_t 	nb_quetecl_get_RSSI();
//uint8_t nb_quectel_get_BER();

/* MQTT protocol APIs*/
bool nb_quectel_innoway_mqtt_start(nb_innoway_mqtt_client_t client);
bool nb_quectel_innoway_mqtt_stop(nb_innoway_mqtt_client_t client);
bool nb_quectel_innoway_mqtt_subscribe(nb_innoway_mqtt_client_t client, const char* topic, uint8_t qos);
bool nb_quectel_innoway_mqtt_unsubscribe(nb_innoway_mqtt_client_t client, const char* topic);
bool nb_quectel_innoway_mqtt_publish(nb_innoway_mqtt_client_t client, const char* topic, const char* data, int len, uint8_t qos, int retain);
bool nb_quectel_innoway_mqtt_reconnect(nb_innoway_mqtt_client_t client);

#endif /* NB_INNOWAY_H_ */
