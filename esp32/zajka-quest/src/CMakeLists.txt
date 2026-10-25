# Composant principal de l'env PlatformIO odroid-go-idf (framework espidf).
# L'env Arduino (odroid-go) ignore ce fichier et compile main_odroid.cpp.
file(GLOB_RECURSE GAME_SRCS "${CMAKE_CURRENT_LIST_DIR}/game/*.cpp")

idf_component_register(
    SRCS "main_odroid_idf.cpp" ${GAME_SRCS}
    INCLUDE_DIRS "."
    REQUIRES driver nvs_flash app_update esp_timer spi_flash)

target_compile_options(${COMPONENT_LIB} PRIVATE -std=gnu++17)
