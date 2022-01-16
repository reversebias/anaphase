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
#include "battery_status.h"
#include <src/lv_themes/lv_theme.h>
#include <zmk/usb.h>
#include <zmk/events/usb_conn_state_changed.h>
#include <zmk/event_manager.h>
#include <zmk/events/battery_state_changed.h>

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

struct battery_status_state {
    uint8_t level;
    bool usb_present;
};

static void set_battery_symbol(lv_obj_t *label, struct battery_status_state state) {
    char text[10] = "          ";

    uint8_t level = state.level;

    if (state.usb_present) {
        sprintf(text, "CHG:%3d%%", level);
    } else {
        sprintf(text, "BAT:%3d%%", level);
    }
    
    lv_label_set_text(label, text);
}

void battery_status_update_cb(struct battery_status_state state) {
    struct zmk_widget_battery_status *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) { set_battery_symbol(widget->obj, state); }
}

static struct battery_status_state battery_status_get_state(const zmk_event_t *eh) {
    return (struct battery_status_state) {
        .level = bt_bas_get_battery_level(),
        .usb_present = zmk_usb_is_powered(),
    };
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_battery_status, struct battery_status_state,
                            battery_status_update_cb, battery_status_get_state)

ZMK_SUBSCRIPTION(widget_battery_status, zmk_battery_state_changed);
ZMK_SUBSCRIPTION(widget_battery_status, zmk_usb_conn_state_changed);

int zmk_widget_battery_status_init(struct zmk_widget_battery_status *widget, lv_obj_t *parent) {
    widget->obj = lv_label_create(parent, NULL);

    lv_obj_set_size(widget->obj, 64, 10);

    sys_slist_append(&widgets, &widget->node);

    widget_battery_status_init();
    return 0;
}

lv_obj_t *zmk_widget_battery_status_obj(struct zmk_widget_battery_status *widget) {
    return widget->obj;
}