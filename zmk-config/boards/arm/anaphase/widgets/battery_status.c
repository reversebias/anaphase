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

K_MUTEX_DEFINE(battery_status_mutex);

struct battery_status_state {
    uint8_t level;
    bool usb_present;
} battery_status_state;

void set_battery_symbol(lv_obj_t *label) {

    k_mutex_lock(&battery_status_mutex, K_FOREVER);

    char text[10] = "  ";

    uint8_t level = battery_status_state.level;

    if (battery_status_state.usb_present) {
        sprintf(text, "CHG:%3d%%", level);
    } else {
        sprintf(text, "BAT:%3d%%", level);
    }
    
    lv_label_set_text(label, text);

    k_mutex_unlock(&battery_status_mutex);
}

void battery_status_update_cb(struct k_work *work) {
    struct zmk_widget_battery_status *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) { set_battery_symbol(widget->obj); }
}

struct battery_status_state battery_status_get_state() {
    k_mutex_lock(&battery_status_mutex, K_FOREVER);
    
    battery_status_state.level = bt_bas_get_battery_level();
    battery_status_state.usb_present = zmk_usb_is_powered();

    k_mutex_unlock(&battery_status_mutex);

    return battery_status_state;
}

int zmk_widget_battery_status_init(struct zmk_widget_battery_status *widget, lv_obj_t *parent) {
    widget->obj = lv_label_create(parent, NULL);

    lv_obj_set_size(widget->obj, 64, 10);
    
    battery_status_get_state();
    set_battery_symbol(widget->obj);
    sys_slist_append(&widgets, &widget->node);

    return 0;
}

lv_obj_t *zmk_widget_battery_status_obj(struct zmk_widget_battery_status *widget) {
    return widget->obj;
}

K_WORK_DEFINE(battery_status_update_work, battery_status_update_cb);

int battery_status_listener(const zmk_event_t *eh) {
    if (zmk_display_is_initialized()) { 
        battery_status_get_state();

        k_work_submit_to_queue(zmk_display_work_q(), &battery_status_update_work);
    }
    return 0;
}

ZMK_LISTENER(widget_battery_status, battery_status_listener);
ZMK_SUBSCRIPTION(widget_battery_status, zmk_battery_state_changed);
ZMK_SUBSCRIPTION(widget_battery_status, zmk_usb_conn_state_changed);