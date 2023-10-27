#include <stdio.h>
#include "esp_log.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "string.h"
#include "mqtt_innoway.h"


#define TOPIC_MQTT_1			    "messages/ca18c634-a035-48b5-96f4-b64748203f97/attributtes"
#define TOPIC_MQTT_2			    "messages/ca18c634-a035-48b5-96f4-b64748203f97/status"
#define DATA_PUBLISH_1			    "{\"hello\": \"hello Innoway 1\"}"
#define DATA_PUBLISH_2			    "{\"hello\": \"hello Innoway 235\"}"
#define deviceID                    "ca18c634-a035-48b5-96f4-b64748203f97"
#define deviceToken                 "s6z4qI7J5mHhHrQZYTNeI0IzracynXVw"
#define WIFI_CONNECTED_BIT          BIT0
#define WIFI_FAIL_BIT               BIT1
#define EXAMPLE_ESP_WIFI_SSID       "IOT_3"
#define EXAMPLE_ESP_WIFI_PASS       "0987654321"
#define EXAMPLE_ESP_MAXIMUM_RETRY   10

esp_mqtt_client_handle_t Innoway_Mqtt_client;
static EventGroupHandle_t s_wifi_event_group;
static int s_retry_num = 0;
static const char *TAG = "MQTT_INNOWAY_EXAMPLE";
int check_connect_mqtt = 0;
TaskHandle_t mqtt_task;
/*Initial wifi*/

static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS,
	        .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }
}


void Innoway_MQTT_event_callback(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
	ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id){
        case MQTT_EVENT_CONNECTED:
            check_connect_mqtt = 1;
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            mqtt_innoway_subscribe(Innoway_Mqtt_client, TOPIC_MQTT_1, 0);
            mqtt_innoway_subscribe(Innoway_Mqtt_client, TOPIC_MQTT_2, 0);
            break;
        case MQTT_EVENT_DISCONNECTED:
            check_connect_mqtt = 0;
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;
        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED");
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED");
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED");
            break;
        case MQTT_EVENT_DATA:
        {
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);
            break;
        }
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
        default:
            ESP_LOGI(TAG, "Other event id");
            break;
	}
}

void mqtt_example(void * arg)
{
	while(1)
	{
		for(int i = 0; i < 10; i++)
		{
			mqtt_innoway_publish(Innoway_Mqtt_client, TOPIC_MQTT_1, DATA_PUBLISH_1, strlen(DATA_PUBLISH_1), 0, 0);
			vTaskDelay(1000/portTICK_RATE_MS);
			mqtt_innoway_publish(Innoway_Mqtt_client, TOPIC_MQTT_2, DATA_PUBLISH_2, strlen(DATA_PUBLISH_2), 0, 0);
			vTaskDelay(1000/portTICK_RATE_MS);
		}

		mqtt_innoway_unsubscribe(Innoway_Mqtt_client, TOPIC_MQTT_2);

		for(int i = 0; i < 10; i++)
		{
			mqtt_innoway_publish(Innoway_Mqtt_client, TOPIC_MQTT_1, DATA_PUBLISH_1, strlen(DATA_PUBLISH_1), 0, 0);
			vTaskDelay(1000/portTICK_RATE_MS);
			mqtt_innoway_publish(Innoway_Mqtt_client, TOPIC_MQTT_2, DATA_PUBLISH_2, strlen(DATA_PUBLISH_2), 0, 0);
			vTaskDelay(1000/portTICK_RATE_MS);
		}
        
        if(check_connect_mqtt == 1){
            ESP_LOGI(TAG, "INNOWAY_DISCONNECTED\r\n");
            mqtt_innoway_disconnect(Innoway_Mqtt_client);
            vTaskDelay(1000/portTICK_RATE_MS);
        }
        
        if(check_connect_mqtt == 0){
            ESP_LOGI(TAG, "INNOWAY_RECONNECTED\r\n");
            mqtt_innoway_reconnect(Innoway_Mqtt_client);
            vTaskDelay(2000/portTICK_RATE_MS);
        }

        for(int i = 0; i < 10; i++)
		{
			mqtt_innoway_publish(Innoway_Mqtt_client, TOPIC_MQTT_1, DATA_PUBLISH_1, strlen(DATA_PUBLISH_1), 0, 0);
			vTaskDelay(1000/portTICK_RATE_MS);
			mqtt_innoway_publish(Innoway_Mqtt_client, TOPIC_MQTT_2, DATA_PUBLISH_2, strlen(DATA_PUBLISH_2), 0, 0);
			vTaskDelay(1000/portTICK_RATE_MS);
		}

        if(check_connect_mqtt == 1){
            check_connect_mqtt = 0;
            ESP_LOGI(TAG, "INNOWAY_STOP\r\n");
            mqtt_innoway_stop(Innoway_Mqtt_client);
            vTaskDelay(1000/portTICK_RATE_MS);
        }
        vTaskDelete(NULL);
	}
}

void app_main(void)
{
	//Init nvs_flash to store wifi information
	esp_err_t err;
    err = nvs_flash_init();
    if (err) {
    	ESP_ERROR_CHECK(nvs_flash_deinit());
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    wifi_init_sta();
    Innoway_Mqtt_client = mqtt_innoway_start(deviceID, deviceToken, Innoway_MQTT_event_callback);
	if(Innoway_Mqtt_client)
    {
		xTaskCreate(mqtt_example, "mqtt_example", 4096, NULL, 3, NULL);
	}
}
