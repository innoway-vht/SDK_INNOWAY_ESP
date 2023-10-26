idf_component_register(SRCS "sim_mqtt_innoway_main.c"
                    INCLUDE_DIRS ".")
add_prebuilt_library(libsim_mqtt_innoway "libsim_mqtt_innoway.a" 
 					PRIV_REQUIRES mqtt log)
target_link_libraries(${COMPONENT_LIB} PRIVATE libsim_mqtt_innoway)
# add_prebuilt_library(libmqtt_innoway "libmqtt_innoway.a" 
#  					PRIV_REQUIRES mqtt log)
# target_link_libraries(${COMPONENT_LIB} PRIVATE libmqtt_innoway)