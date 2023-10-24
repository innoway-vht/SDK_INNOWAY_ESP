idf_component_register(SRCS "wifi_innoway_main.c"
                    INCLUDE_DIRS ".")
add_prebuilt_library(libmqtt_innoway "libmqtt_innoway.a" 
 					PRIV_REQUIRES mqtt log)
target_link_libraries(${COMPONENT_LIB} PRIVATE libmqtt_innoway)