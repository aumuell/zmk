/*
 *
 * Copyright (c) 2021 Darryl deHaan
 * SPDX-License-Identifier: MIT
 *
 */

#include <kernel.h>
#include <logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/display.h>
#include "layer_status.h"
#include <zmk/events/layer_state_changed.h>
#include <zmk/event_manager.h>
#include <zmk/endpoints.h>
#include <zmk/keymap.h>

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

struct layer_status_state {
    uint8_t index;
    const char *label;
};

#if IS_ENABLED(CONFIG_ZMK_DISPLAY_HIDE_MOMENTARY_LAYERS)
uint8_t last_perm_index = 255;
#endif

static void set_layer_symbol(lv_obj_t *label, struct layer_status_state state) {
    const char *layer_label = state.label;
    uint8_t active_layer_index = state.index;
    static bool prev_label_valid = false;
    static char prev_label[10] = "";

#if IS_ENABLED(CONFIG_ZMK_DISPLAY_HIDE_MOMENTARY_LAYERS)
    if (!zmk_keymap_layer_momentary(active_layer_index) && last_perm_index != active_layer_index) {
        last_perm_index = active_layer_index;
        LOG_DBG("Last perm layer index updated to %i", active_layer_index);
    } else {
        return;
    }
#endif

    if (layer_label == NULL) {
        char text[6] = {};

        sprintf(text, " %i", active_layer_index);

        if (!prev_label_valid || strncmp(text, prev_label, sizeof(prev_label)) != 0)
            lv_label_set_text(label, text);
        strncpy(prev_label, text, sizeof(prev_label));
    } else {
        if (!prev_label_valid || strncmp(layer_label, prev_label, sizeof(prev_label)) != 0)
            lv_label_set_text(label, layer_label);
        strncpy(prev_label, layer_label, sizeof(prev_label));
    }
    prev_label[sizeof(prev_label)-1] = '\0';
    prev_label_valid = true;
}

static void layer_status_update_cb(struct layer_status_state state) {
    struct zmk_widget_layer_status *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) { set_layer_symbol(widget->obj, state); }
}

static struct layer_status_state layer_status_get_state(const zmk_event_t *eh) {
    uint8_t index = zmk_keymap_highest_layer_active();
    return (struct layer_status_state){.index = index, .label = zmk_keymap_layer_label(index)};
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_layer_status, struct layer_status_state, layer_status_update_cb,
                            layer_status_get_state)

ZMK_SUBSCRIPTION(widget_layer_status, zmk_layer_state_changed);

int zmk_widget_layer_status_init(struct zmk_widget_layer_status *widget, lv_obj_t *parent) {
    widget->obj = lv_label_create(parent, NULL);

    sys_slist_append(&widgets, &widget->node);

    widget_layer_status_init();
    return 0;
}

lv_obj_t *zmk_widget_layer_status_obj(struct zmk_widget_layer_status *widget) {
    return widget->obj;
}
