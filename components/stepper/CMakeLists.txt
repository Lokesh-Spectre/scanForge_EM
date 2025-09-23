file(GLOB_RECURSE MY_COMPONENT_SRCS "src/*.c" "src/api.c")

idf_component_register(
    SRCS ${MY_COMPONENT_SRCS}
    INCLUDE_DIRS "inc"
    REQUIRES main esp_driver_gpio
)

