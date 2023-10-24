idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS ".")
                    
add_prebuilt_library(libmqtt_innoway "libnb_innoway.a" 
 					PRIV_REQUIRES driver log)
target_link_libraries(${COMPONENT_LIB} PRIVATE libnb_innoway)
																				 
										