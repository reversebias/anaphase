/*
 * Copyright (c) 2020 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#include <kernel.h>
#include <bluetooth/services/bas.h>

#include <logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/display.h>
#include "output_status.h"
#include <zmk/event_manager.h>
#include <zmk/events/usb_conn_state_changed.h>
#include <zmk/events/ble_active_profile_changed.h>
#include <zmk/events/endpoint_selection_changed.h>
#include <zmk/usb.h>
#include <zmk/ble.h>
#include <zmk/endpoints.h>

#if defined(CONFIG_ZMK_SPLIT_BLE_ROLE_CENTRAL)
static const char name_array[5][8] = {
    {"HOME PC"},
    {"WORK PC"},
    {"TABLET"},
    {"PHONE"},
    {"MISC"}
};
#endif

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

struct output_status_state {
    enum zmk_endpoint selected_endpoint;
    bool active_profile_connected;
    bool active_profile_bonded;
    uint8_t active_profile_index;
};

static struct output_status_state get_state(const zmk_event_t *_eh) {
    return (struct output_status_state){.selected_endpoint = zmk_endpoints_selected(),
                                        .active_profile_connected =
                                            zmk_ble_active_profile_is_connected(),
                                        .active_profile_bonded = !zmk_ble_active_profile_is_open(),
                                        .active_profile_index = zmk_ble_active_profile_index()};
    ;
}

static void set_status_symbol(lv_obj_t *status_label, lv_obj_t *name_label, struct output_status_state state) {
    char status_text[10] = "          ";
    char name_text[10] = "          ";

    switch (state.selected_endpoint) {
    case ZMK_ENDPOINT_USB:
        snprintf(status_text, 10, "USB MODE");
        break;
    case ZMK_ENDPOINT_BLE:
        if (state.active_profile_bonded) {
            if (state.active_profile_connected) {
                snprintf(status_text, 10, "PAIRED");
            } else {
                snprintf(status_text, 10, "WAITING");
            }
        } else {
            snprintf(status_text, 10, "UNPAIRED");
        }
#if defined(CONFIG_ZMK_SPLIT_BLE_ROLE_CENTRAL)
        snprintf(name_text, 10, "%s", name_array[state.active_profile_index]);
#else
        snprintf(name_text, 10, "ANAPHASE");
#endif
        break;
    }

    lv_label_set_text(status_label, status_text);
    lv_label_set_text(name_label, name_text);
}

static void output_status_update_cb(struct output_status_state state) {
    struct zmk_widget_output_status *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) { set_status_symbol(widget->status_obj, widget->name_obj, state); }
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_output_status, struct output_status_state,
                            output_status_update_cb, get_state)
ZMK_SUBSCRIPTION(widget_output_status, zmk_endpoint_selection_changed);

ZMK_SUBSCRIPTION(widget_output_status, zmk_usb_conn_state_changed);
ZMK_SUBSCRIPTION(widget_output_status, zmk_ble_active_profile_changed);

int zmk_widget_output_status_init(struct zmk_widget_output_status *widget, lv_obj_t *parent) {
    widget->status_obj = lv_label_create(parent, NULL);
    widget->name_obj = lv_label_create(parent, NULL);

    lv_obj_set_size(widget->status_obj, 64, 10);
    lv_obj_set_size(widget->name_obj, 64, 10);

    sys_slist_append(&widgets, &widget->node);

    widget_output_status_init();
    return 0;
}

lv_obj_t *zmk_widget_output_status_obj(struct zmk_widget_output_status *widget) {
    return widget->status_obj;
}

lv_obj_t *zmk_widget_output_name_obj(struct zmk_widget_output_status *widget) {
    return widget->name_obj;
}
