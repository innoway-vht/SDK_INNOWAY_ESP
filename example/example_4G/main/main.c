#include <stdio.h>
#include <string.h>

#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_system.h"
#include "esp_log.h"

#include "nb_innoway.h"

#define ECHO_TEST_TXD  		17
#define ECHO_TEST_RXD  		16
#define ECHO_UART_PORT_NUM	2

#define BROKER 			"mqtt.innoway.vn"
#define DEVICE_ID 		"41018142-00ca-4098-8a25-b819fa289141"
#define DEVICE_TOKEN 	"zdEKVoH4mKPXCk4wmh92Knz7zyklMeLR"
#define USERNAME 		"TEST LIB"

static const char* TAG = "MAIN";

static void nb_mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
	char* data = (char*) event_data;

	switch (event_id) {
	case NB_MQTT_INNOWAY_CONNECTED:
		ESP_LOGI(TAG, "NB_MQTT_INNOWAY_CONNECTED");
		break;
	case NB_MQTT_INNOWAY_DISCONNECTED:
		ESP_LOGI(TAG, "NB_MQTT_INNOWAY_DISCONNECTED");
		break;

	case NB_MQTT_INNOWAY_SUBSCRIBED:
		ESP_LOGI(TAG, "NB_MQTT_INNOWAY_SUBSCRIBED");
		break;
	case NB_MQTT_INNOWAY_UNSUBSCRIBED:
		ESP_LOGI(TAG, "NB_MQTT_INNOWAY_UNSUBSCRIBED");
		break;
	case NB_MQTT_INNOWAY_PUBLISHED:
		ESP_LOGI(TAG, "NB_MQTT_INNOWAY_PUBLISHED");
		break;
	case NB_MQTT_INNOWAY_DATA:
		ESP_LOGI(TAG, "NB_MQTT_INNOWAY_DATA, DATA = %s", data);
		break;
	case NB_MQTT_INNOWAY_ERROR:
		ESP_LOGI(TAG, "NB_MQTT_INNOWAY_ERROR");
		break;
	}
}

nb_innoway_mqtt_client_t client = {
	.client_id = DEVICE_ID,
	.host = BROKER,
	.keepalive = 60,
	.msg_id = 1,
	.password = DEVICE_TOKEN,
	.port = 1883,
	.tcp_connect_id = 1,
	.username = USERNAME,
	.nb_mqtt_innoway_event_handler = nb_mqtt_event_handler,
};

void app_main(void)
{

	nb_quectel_innoway_init(ECHO_UART_PORT_NUM, ECHO_TEST_TXD, ECHO_TEST_RXD);
	ESP_LOGI(TAG, "Firmware Version: %s", nb_quectel_get_firmware_version());
	if (nb_quectel_is_registered())
	{
		ESP_LOGI(TAG, "NETWORK IS REGISTERED");
		nb_quectel_innoway_mqtt_start(client);
		char topic[128] = "";
		sprintf(topic, "messages/%s/dht11", DEVICE_ID);
		nb_quectel_innoway_mqtt_subscribe(client, topic, 0);
	}
}
