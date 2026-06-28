idf_component_register(
    SRCS "../src/main.c" "../src/primitives.c"
    INCLUDE_DIRS "../src"
    REQUIRES mbedtls esp_timer esp_hw_support
)
