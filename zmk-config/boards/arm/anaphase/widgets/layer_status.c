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
#include "layer_status.h"
#include <zmk/events/layer_state_changed.h>
#include <zmk/event_manager.h>
#include <zmk/endpoints.h>
#include <zmk/keymap.h>

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

K_MUTEX_DEFINE(layer_status_mutex);

struct layer_status_state {
    uint8_t index;
    const char *label;
} layer_status_state;

void set_layer_symbol(lv_obj_t *label) {

    k_mutex_lock(&layer_status_mutex, K_FOREVER);

    if (layer_status_state.label == NULL) {
        char text[6] = {};

        sprintf(text, "BASE");

        lv_label_set_text(label, text);
    } else {
        char text[12] = {};

        snprintf(text, 12, "%s", layer_status_state.label);

        lv_label_set_text(label, text);
    }

    k_mutex_unlock(&layer_status_mutex);
}

void layer_status_update_cb(struct k_work *work) {
    struct zmk_widget_layer_status *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) { set_layer_symbol(widget->obj); }
}

void layer_status_get_state() {
    k_mutex_lock(&layer_status_mutex, K_FOREVER);

    uint8_t index = zmk_keymap_highest_layer_active();
    layer_status_state.index = index;
    layer_status_state.label = zmk_keymap_layer_label(index);

    k_mutex_unlock(&layer_status_mutex);
}

int zmk_widget_layer_status_init(struct zmk_widget_layer_status *widget, lv_obj_t *parent) {
    widget->obj = lv_label_create(parent, NULL);

    lv_obj_set_size(widget->obj, 64, 10);

    set_layer_symbol(widget->obj);
    sys_slist_append(&widgets, &widget->node);

    return 0;
}

lv_obj_t *zmk_widget_layer_status_obj(struct zmk_widget_layer_status *widget) {
    return widget->obj;
}

K_WORK_DEFINE(layer_status_update_work, layer_status_update_cb);

int layer_status_listener(const zmk_event_t *eh) {
    layer_status_get_state();

    k_work_submit_to_queue(zmk_display_work_q(), &layer_status_update_work);
    return 0;
}

ZMK_LISTENER(widget_layer_status, layer_status_listener)
ZMK_SUBSCRIPTION(widget_layer_status, zmk_layer_state_changed);