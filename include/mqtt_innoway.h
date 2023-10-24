#ifndef MQTT_INNOWAY_H_
#define MQTT_INNOWAY_H_

#include "mqtt_client.h"

#define KEEPALIVE	60  // second


typedef void (*mqtt_innoway_callback)(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);

esp_mqtt_client_handle_t mqtt_innoway_start(char *client_id, char *passowrd,  mqtt_innoway_callback user_callback);


esp_err_t mqtt_innoway_disconnect(esp_mqtt_client_handle_t client);

esp_err_t mqtt_innoway_reconnect(esp_mqtt_client_handle_t client);


esp_err_t mqtt_innoway_stop(esp_mqtt_client_handle_t client);


int mqtt_innoway_publish(esp_mqtt_client_handle_t client, const char *topic, const char *data, int len, int qos, int retain);


int mqtt_innoway_subscribe(esp_mqtt_client_handle_t client, const char *topic, int qos);


int mqtt_innoway_unsubscribe(esp_mqtt_client_handle_t client, const char *topic);

#endif /* MQTT_INNOWAY_H_ */