idf_component_register(SRCS "scanForge.c"
                    INCLUDE_DIRS "."
                    REQUIRES bt nvs_flash
                    PRIV_REQUIRES comms
                    )
