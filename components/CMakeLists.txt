set(src_dirs
            LED
            QMI
            WIFI
            MQTT_APP)

set(include_dirs
            LED
            QMI
            WIFI
            MQTT_APP)

set(requires
            esp_wifi
            nvs_flash
            esp_event
            driver
            mqtt
            )

idf_component_register(SRC_DIRS ${src_dirs} INCLUDE_DIRS ${include_dirs} REQUIRES ${requires})

component_compile_options(-ffast-math -O3)
