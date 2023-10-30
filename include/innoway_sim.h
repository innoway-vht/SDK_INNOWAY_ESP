#ifndef INNOWAY_SIM_H_
#define INNOWAY_SIM_H_

#include <stdbool.h>

#include "esp_err.h"
#include "esp_event.h"
#include "esp_event_base.h"
#include "driver/gpio.h"
#include "mqtt_client.h"
#include "driver/uart.h"


#define BROKER				"mqtt.innoway.vn"
#define BAUDRATE			115200
#define RX_BUF_SIZE 		4096
#define TX_BUF_SIZE			1024
#define BUF_SIZE 			(2048)
#define ECHO_TEST_RTS 		(UART_PIN_NO_CHANGE)
#define ECHO_TEST_CTS 		(UART_PIN_NO_CHANGE)


ESP_EVENT_DECLARE_BASE(SIMCOM_MQTT_INNOWAY_EVENTS);

enum
{
	SIMCOM_MQTT_INNOWAY_CONNECTED,
	SIMCOM_MQTT_INNOWAY_DISCONNECTED,
    SIMCOM_MQTT_INNOWAY_STOP,
	SIMCOM_MQTT_INNOWAY_SUBSCRIBED,
	SIMCOM_MQTT_INNOWAY_PUBLISHED,
	SIMCOM_MQTT_INNOWAY_UNSUBSCRIBED,
	SIMCOM_MQTT_INNOWAY_DATA,
	SIMCOM_MQTT_INNOWAY_ERROR,
};


typedef struct simcom_innoway_mqtt_client
{
	char host[128];
	char client_id[60];
	char username[50];
	char password[60];
	char broker[50];
	uint32_t    port;
	int         keepalive;
	uint32_t	msg_id;
	uint8_t		tcp_connect_id;
	int			index; //default is 0 else it can be 1
	int 		server_type; // 0 if TCP, 1 if SSL/TTS
	esp_event_handler_t sim_mqtt_innoway_event_handler;
}simcom_innoway_mqtt_client_t;

typedef struct {
    esp_mqtt_event_id_t event_id;       /*!< MQTT event type */
    esp_mqtt_client_handle_t client;    /*!< MQTT client handle for this event */
    void *user_context;                 /*!< User context passed from MQTT client config */
    char data[200];                         /*!< Data associated with this event */
    int data_len;                       /*!< Length of the data for this event */
    int total_data_len;                 /*!< Total length of the data (longer data are supplied with multiple events) */
    int current_data_offset;            /*!< Actual offset for the data associated with this event */
    char topic[200];                        /*!< Topic associated with this event */
    int topic_len;                      /*!< Length of the topic for this event associated with this event */
    int msg_id;                         /*!< MQTT messaged id of message */
    int session_present;                /*!< MQTT session_present flag for connection event */
    esp_mqtt_error_codes_t *error_handle; /*!< esp-mqtt error handle including esp-tls errors as well as internal mqtt errors */
    bool retain;                        /*!< Retained flag of the message associated with this event */
    int qos;                            /*!< qos of the messages associated with this event */
    bool dup;                           /*!< dup flag of the message associated with this event */
} simcom_innoway_mqtt_event_t;

typedef simcom_innoway_mqtt_event_t *simcom_innoway_mqtt_event_handle_t;



/* 4G SIMCOM A76xx ESP32 APIs*/

esp_err_t simcom_innoway_init(uart_port_t uart_num, int tx_io_num, int rx_io_num, int baud_rate);

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
bool simcom_is_registered(int retry);

/* MQTT protocol APIs*/
bool simcom_innoway_mqtt_start(simcom_innoway_mqtt_client_t client);
bool simcom_innoway_mqtt_stop(simcom_innoway_mqtt_client_t client);
bool simcom_innoway_mqtt_subscribe(simcom_innoway_mqtt_client_t client, char* topic, uint8_t qos);
bool simcom_innoway_mqtt_unsubscribe(simcom_innoway_mqtt_client_t client, char* topic);
bool simcom_innoway_mqtt_publish(simcom_innoway_mqtt_client_t client, char* topic, char* data, int len, uint8_t qos, int retain);
bool simcom_innoway_mqtt_reconnect(simcom_innoway_mqtt_client_t client);
bool simcom_innoway_mqtt_disconnect(simcom_innoway_mqtt_client_t client);
bool echoATSwtich(int enable, int retry);
bool AT_CheckSimStatus( int retry);
void lte_sim_init();
void lte_sim_power_off();
void lte_sim_power_on();
#endif /* INNOWAY_SIM_H_ */
