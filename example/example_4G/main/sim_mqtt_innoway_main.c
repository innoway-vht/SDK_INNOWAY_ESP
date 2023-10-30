#include <stdio.h>
#include <string.h>

#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "mqtt_client.h"

#include "innoway_sim.h"


#define ECHO_TEST_TXD  		11
#define ECHO_TEST_RXD  		12
#define ECHO_UART_PORT_NUM	2

#define BROKER_INNOWAY			"tcp://mqtt.innoway.vn:1883"
#define DEVICE_ID 				"cvm123"
#define DEVICE_TOKEN 			"s6z4qI7J5mHhHrQZYTNeI0IzracynXVw"
#define USERNAME 				"TESTSDK"
#define SIM_POWERKEY            7
#define CHIP_PWR                8
#define TOPIC_MQTT_1			"messages/ca18c634-a035-48b5-96f4-b64748203f97/attributtes"
#define TOPIC_MQTT_2			"messages/ca18c634-a035-48b5-96f4-b64748203f97/status"
#define DATA_PUBLISH_1			"{\"hello\": \"hello Innoway 1\"}"
#define DATA_PUBLISH_2			"{\"hello\": \"hello Innoway 235\"}"
ESP_EVENT_DEFINE_BASE(SIMCOM_MQTT_INNOWAY_EVENTS);
static const char* TAG = "MAIN";
int check_connect = 0;
int count = 0;
int count_un = 0;
char topic[128] = "";
char data_publish[128] = "";
int count_network = 0;
int count_init = 0;
int count_sim = 0;

void simcom_innoway_mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);

simcom_innoway_mqtt_client_t client = {
	.client_id = DEVICE_ID,
	.broker = BROKER_INNOWAY,
	.keepalive = 60,
	.index = 0,
	.msg_id = 1,
	.password = DEVICE_TOKEN,
	.port = 1883,
	.tcp_connect_id = 1,
	.username = USERNAME,
	.sim_mqtt_innoway_event_handler = simcom_innoway_mqtt_event_handler,
};



void init_gpio_output(gpio_num_t pwrkey)
{
	gpio_config_t io_conf = {};
	io_conf.intr_type = GPIO_INTR_DISABLE;
	io_conf.mode = GPIO_MODE_OUTPUT;
	io_conf.pin_bit_mask = 1 << pwrkey;
	io_conf.pull_down_en = 0;
	io_conf.pull_up_en = 0;
	gpio_config(&io_conf);
	gpio_set_level(pwrkey, 1);
}



void simcom_innoway_mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
	simcom_innoway_mqtt_event_handle_t event = event_data;
	switch (event_id) {
	case SIMCOM_MQTT_INNOWAY_CONNECTED:
		check_connect = 1;
		ESP_LOGI(TAG, "SIMCOM_MQTT_INNOWAY_CONNECTED");
		break;
	case SIMCOM_MQTT_INNOWAY_DISCONNECTED:
		simcom_innoway_mqtt_reconnect(client);
		check_connect = 0;
		ESP_LOGI(TAG, "SIMCOM_MQTT_INNOWAY_DISCONNECTED");
		break;
	case SIMCOM_MQTT_INNOWAY_SUBSCRIBED:
		ESP_LOGI(TAG, "SIMCOM_MQTT_INNOWAY_SUBSCRIBED");
		break;
	case SIMCOM_MQTT_INNOWAY_PUBLISHED:
		ESP_LOGI(TAG, "SIMCOM_MQTT_INNOWAY_PUBLISHED");
		break;
	case SIMCOM_MQTT_INNOWAY_UNSUBSCRIBED:
		ESP_LOGI(TAG, "SIMCOM_MQTT_INNOWAY_UNSUBSCRIBED");
		break;
	case SIMCOM_MQTT_INNOWAY_STOP:
		ESP_LOGI(TAG, "SIMCOM_MQTT_INNOWAY_STOP");
		break;
	case SIMCOM_MQTT_INNOWAY_DATA:
		ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
		break;
	case SIMCOM_MQTT_INNOWAY_ERROR:
		ESP_LOGI(TAG, "SIMCOM_MQTT_INNOWAY_ERROR");
		break;
	}
}




void app_main(void)
{
	esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
	ESP_ERROR_CHECK(esp_event_loop_create_default());
	/* Power on module sim 4G, pull down gpio pwrkey
	init_gpio_output(8);
	Simcom_powerOn(8);
	*/
	lte_sim_init();
	vTaskDelay(1000/portTICK_RATE_MS);
	simcom_innoway_init(ECHO_UART_PORT_NUM, ECHO_TEST_TXD, ECHO_TEST_RXD, 115200);
	INIT_SIM:
	if(echoATSwtich(0,3) == true)
	{
		printf("Init successful\r\n");
	}
	else
	{
		count_init++;
		vTaskDelay(1000/portTICK_RATE_MS);
		if(count_init >= 4){
			lte_sim_power_off();
			esp_restart();
		}
		goto INIT_SIM;
	}
	vTaskDelay(1000/portTICK_RATE_MS);

	SIM_STATUS:
	if(AT_CheckSimStatus(3) == true)
	{
		printf("SIM Status: Ready\r\n");
	}
	else
	{
		count_sim++;
		vTaskDelay(1000/portTICK_RATE_MS);
		if(count_sim >= 4){
			lte_sim_power_off();
			esp_restart();
		}
		goto SIM_STATUS;
	}
	vTaskDelay(8000/portTICK_RATE_MS);

	NETWORK:
	if (simcom_is_registered(3))
	{
		ESP_LOGI(TAG, "NETWORK IS REGISTERED");
		vTaskDelay(2000/portTICK_RATE_MS);
		if(simcom_innoway_mqtt_start(client))
		{
			simcom_innoway_mqtt_subscribe(client, TOPIC_MQTT_1, 0);
			vTaskDelay(1000/portTICK_RATE_MS);
    		simcom_innoway_mqtt_subscribe(client, TOPIC_MQTT_2, 0);
			printf("Connected\r\n");
		}
	}
	else
	{
		count_network++;
		vTaskDelay(1000/portTICK_RATE_MS);
		if(count_network >= 4){
			lte_sim_power_off();
			esp_restart();
		}
		goto NETWORK;
	}


	while(1){
		if(check_connect == 1)
		{
			sprintf(data_publish, "{\"Temp\":\"20\"}");
			simcom_innoway_mqtt_publish(client,TOPIC_MQTT_1,data_publish,strlen(data_publish),0,0);
			vTaskDelay(1000/portTICK_RATE_MS);
			sprintf(data_publish, "{\"Temp\":\"36\"}");
			simcom_innoway_mqtt_publish(client,TOPIC_MQTT_2,data_publish,strlen(data_publish),0,0);
			vTaskDelay(1000/portTICK_RATE_MS);
			count++;
		}
		if((count >= 5) && (count_un == 0))
		{
			sprintf(topic, TOPIC_MQTT_1);
			simcom_innoway_mqtt_unsubscribe(client, topic);
			vTaskDelay(1000/portTICK_RATE_MS);
			count = 0;
			count_un = 1;
		}
		else if((count >= 5) && (count_un == 1))
		{
			printf("Disconnect INNOWAY start\r\n");
			simcom_innoway_mqtt_disconnect(client);
			vTaskDelay(2500/portTICK_RATE_MS);
			count = 0;
			count_un = 2;
		}
		else if(count_un == 2)
		{
			if(check_connect == 0)
			{
				printf("Reconnect INNOWAY start\r\n");
				simcom_innoway_mqtt_reconnect(client);
				vTaskDelay(1000/portTICK_RATE_MS);
			}
			count_un = 3;
		}
		else if(count_un == 3)
		{
			printf("STOP MQTT INNOWAY\r\n");
			check_connect = 0;
			simcom_innoway_mqtt_stop(client);
			count_un = 4;
		}
		else if(count_un == 4)
		{
			if (simcom_is_registered(3))
			{
				ESP_LOGI(TAG, "NETWORK IS REGISTERED");
				vTaskDelay(2000/portTICK_RATE_MS);
				if(simcom_innoway_mqtt_start(client))
				{
					simcom_innoway_mqtt_subscribe(client, TOPIC_MQTT_1, 0);
					vTaskDelay(1000/portTICK_RATE_MS);
					simcom_innoway_mqtt_subscribe(client, TOPIC_MQTT_2, 0);
					printf("Connected\r\n");
				}
			}
			count_un = 0;
			count = 0;
		}
		vTaskDelay(2000/portTICK_RATE_MS);
	}
}

