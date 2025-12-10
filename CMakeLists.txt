cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME miratherm-radiator-thermostat-software)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Add driver include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    Drivers/ssd1306/ssd1306
    Drivers/
    Drivers/lvgl_port_display
    Drivers/rotary_encoder
    Drivers/buttons
    Drivers/motor
)

# Add driver sources
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    Drivers/ssd1306/ssd1306/ssd1306.c
    Drivers/ssd1306/ssd1306/ssd1306_fonts.c
    Drivers/lvgl_port_display/lvgl_port_display.c
    Drivers/rotary_encoder/rotary_encoder.c
    Drivers/buttons/buttons.c
    Drivers/motor/motor.c
)

# Add LVGL subdirectory with embedded configuration (disables demos/examples)
set(CONFIG_LV_BUILD_DEMOS OFF CACHE BOOL "Disable LVGL demos for embedded" FORCE)
set(CONFIG_LV_BUILD_EXAMPLES OFF CACHE BOOL "Disable LVGL examples for embedded" FORCE)
set(LV_CONF_PATH "${CMAKE_SOURCE_DIR}/Drivers/lv_conf.h" CACHE STRING "Path to lv_conf.h" FORCE)
add_subdirectory(Drivers/lvgl)

# For embedded systems without pthread support, exclude OSAL implementations that require OS features
# This is done by modifying the LVGL library after it's created
if(TARGET lvgl)
    get_target_property(LVGL_SOURCES lvgl SOURCES)
    list(FILTER LVGL_SOURCES EXCLUDE REGEX "src/osal/lv_pthread\\.c$")
    list(FILTER LVGL_SOURCES EXCLUDE REGEX "src/osal/lv_windows\\.c$")
    list(FILTER LVGL_SOURCES EXCLUDE REGEX "src/osal/lv_sdl2\\.c$")
    list(FILTER LVGL_SOURCES EXCLUDE REGEX "src/osal/lv_linux\\.c$")
    set_target_properties(lvgl PROPERTIES SOURCES "${LVGL_SOURCES}")
endif()

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
    Core/Inc/Presenters
    Core/Inc/Views
)

# Add sources to executable
file(GLOB PRESENTER_SOURCES Core/Src/Presenters/*.c)
file(GLOB VIEW_SOURCES Core/Src/Views/*.c)
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
    Core/Src/input_task.c
    Core/Src/sensor_task.c
    Core/Src/tests.c
    Core/Src/view_presenter_task.c
    Core/Src/view_presenter_router.c
    ${PRESENTER_SOURCES}
    ${VIEW_SOURCES}
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    Core/Inc
    Core/Inc/Presenters
    Core/Inc/Views
    Core/Inc/ViewModels
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx
    lvgl
    
    # Add user defined libraries
)
