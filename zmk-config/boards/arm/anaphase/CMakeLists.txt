# SPDX-License-Identifier: MIT

zephyr_library()
zephyr_library_sources(board.c)
zephyr_library_include_directories(${ZEPHYR_BASE}/drivers)

# -b argument must match &code_partition address in numble.dts
set_property(GLOBAL APPEND PROPERTY extra_post_build_commands
  COMMAND ${PYTHON_EXECUTABLE} ${ZEPHYR_BASE}/../tools/uf2/utils/uf2conv.py
  -c
  -b 0x1000
  -f 0xADA52840
  -o ${PROJECT_BINARY_DIR}/${CONFIG_KERNEL_BIN_NAME}.uf2
  ${PROJECT_BINARY_DIR}/${CONFIG_KERNEL_BIN_NAME}.bin
)

target_sources_ifdef(CONFIG_CUSTOM_WIDGET_BATTERY_STATUS app PRIVATE widgets/battery_status.c)
#target_sources_ifdef(CONFIG_CUSTOM_WIDGET_OUTPUT_STATUS app PRIVATE widgets/output_status.c)
#target_sources_ifdef(CONFIG_CUSTOM_WIDGET_LAYER_STATUS app PRIVATE widgets/layer_status.c)
#target_sources_ifdef(CONFIG_CUSTOM_WIDGET_LAYER_STATUS app PRIVATE widgets/wpm_status.c)


zephyr_library_include_directories(${ZEPHYR_LVGL_MODULE_DIR})
zephyr_library_include_directories(${ZEPHYR_BASE}/lib/gui/lvgl/)
zephyr_library_sources_ifdef(CONFIG_ZMK_DISPLAY custom_status_screen.c)
zephyr_library_sources(${ZEPHYR_BASE}/misc/empty_file.c)
zephyr_library_include_directories(${CMAKE_SOURCE_DIR}/include)
zephyr_library_include_directories(${ZEPHYR_BASE}/drivers)