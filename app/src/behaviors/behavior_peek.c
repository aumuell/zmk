/*
 * Copyright (c) 2020 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_behavior_peek

#include <device.h>
#include <drivers/behavior.h>
#include <logging/log.h>

#include <zmk/behavior.h>
#include <zmk/keymap.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

struct behavior_peek_data {
    zmk_keymap_layers_state_t layer_state;
};


static int behavior_peek_init(const struct device *dev) { return 0; };

static int on_keymap_binding_pressed(struct zmk_behavior_binding *binding,
                                     struct zmk_behavior_binding_event event) {
    const struct device *dev = device_get_binding(binding->behavior_dev);
    struct behavior_peek_data *data = dev->data;
    data->layer_state = zmk_keymap_layer_state();
    LOG_INF("peek layer %d: enabled=%x", binding->param1, (unsigned)zmk_keymap_layer_state());

    int layer = binding->param1;
    if (layer < 0)
        layer = event.layer - layer;
    if (layer >= event.layer)
        layer = event.layer-1;
    for (; layer >= zmk_keymap_layer_default(); layer--) {
        if (zmk_keymap_layer_active_with_state(layer, data->layer_state)) {
            LOG_INF("peek: forwarding to layer %d, layers=%x", layer, (unsigned)data->layer_state);
            return zmk_keymap_apply_position_state(event.source, layer, event.position, true, event.timestamp);
        }
    }

    return ZMK_BEHAVIOR_OPAQUE;
}

static int on_keymap_binding_released(struct zmk_behavior_binding *binding,
                                      struct zmk_behavior_binding_event event) {
    const struct device *dev = device_get_binding(binding->behavior_dev);
    struct behavior_peek_data *data = dev->data;

    int layer = binding->param1;
    if (layer < 0)
        layer = event.layer - layer;
    if (layer >= event.layer)
        layer = event.layer-1;
    for (; layer >= zmk_keymap_layer_default(); layer--) {
        if (zmk_keymap_layer_active_with_state(layer, data->layer_state)) {
            return zmk_keymap_apply_position_state(event.source, layer, event.position, false, event.timestamp);
        }
    }

    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_peek_driver_api = {
    .binding_pressed = on_keymap_binding_pressed,
    .binding_released = on_keymap_binding_released,
};

#define KP_INST(n)                                                                                 \
    static struct behavior_peek_data behavior_peek_data_##n = {};                        \
DEVICE_DT_INST_DEFINE(n, behavior_peek_init, NULL, &behavior_peek_data_##n, NULL, APPLICATION, \
                      CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &behavior_peek_driver_api);
DT_INST_FOREACH_STATUS_OKAY(KP_INST)


#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */
