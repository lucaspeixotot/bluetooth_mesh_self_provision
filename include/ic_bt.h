/**
 * @file ic_bt.h
 * @Lucas Peixoto (lpdac@ic.ufal.br)
 * @brief
 * @version 0.1
 * @date 2018-12-25
 *
 * @copyright Copyright (c) 2018
 *
 */

#ifndef _IC_BT_H
#define _IC_BT_H

#include <bluetooth/bluetooth.h>
#include <bluetooth/mesh.h>
#include <misc/printk.h>
#include <settings/settings.h>


extern const struct bt_mesh_model_op generic_onoff_cli_op[];
extern const struct bt_mesh_model_op generic_onoff_srv_op[];
extern const struct bt_mesh_model_op generic_level_srv_op[];
extern const struct bt_mesh_model_op generic_level_cli_op[];

// Generic OnOff Messages Types
#define BT_MESH_MODEL_OP_GENERIC_ONOFF_GET BT_MESH_MODEL_OP_2(0x82, 0x01)
#define BT_MESH_MODEL_OP_GENERIC_ONOFF_SET BT_MESH_MODEL_OP_2(0x82, 0x02)
#define BT_MESH_MODEL_OP_GENERIC_ONOFF_SET_UNACK BT_MESH_MODEL_OP_2(0x82, 0x03)
#define BT_MESH_MODEL_OP_GENERIC_ONOFF_STATUS BT_MESH_MODEL_OP_2(0x82, 0x04)


#define BT_MESH_MODEL_OP_GENERIC_LEVEL_GET BT_MESH_MODEL_OP_2(0x82, 0x05)
#define BT_MESH_MODEL_OP_GENERIC_LEVEL_SET BT_MESH_MODEL_OP_2(0x82, 0x06)
#define BT_MESH_MODEL_OP_GENERIC_LEVEL_SET_UNACK BT_MESH_MODEL_OP_2(0x82, 0x07)
#define BT_MESH_MODEL_OP_GENERIC_LEVEL_STATUS BT_MESH_MODEL_OP_2(0x82, 0x08)

#define IS_SERVER_ONOFF 0x100
#define IS_CLIENT_ONOFF 0x101
#define IS_SERVER_AND_CLIENT_ONOFF 0x110

#define S16_MIN -32768
#define S16_MAX 32767

#define LEVEL_MIN 0
#define LEVEL_MAX 20000

#define map(var, _out_type, _x, _in_min, _in_max, _out_min, _out_max) \
    var = (_out_type)((_x - _in_min) * (_out_max - _out_min) / (_in_max - _in_min) + _out_min)

struct onoff_srv {
    u8_t *state;
    u8_t tid;
    void (*set_state)(u8_t value);
};

struct onoff_cli {
    struct bt_mesh_model *model_cli;
    struct k_work callback_work;
    u8_t state;
    u8_t act;
    u8_t tid;
};

struct level_srv {
    u16_t *level_state;
    u8_t tid;
    void (*set_level)(u16_t pulse_width);
};

struct level_cli {
    struct bt_mesh_model *model_cli;
    struct k_work callback_work;
    u16_t *level_state;
    u16_t act;
    u8_t tid;
};

/**
 * @brief
 *
 * @param model
 * @param ctx
 * @param buf
 */
void generic_onoff_get(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx,
                       struct net_buf_simple *buf);


/**
 * @brief
 *
 * @param model
 * @param ctx
 * @param buf
 */
void generic_onoff_set(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx,
                       struct net_buf_simple *buf);


/**
 * @brief
 *
 * @param model
 * @param ctx
 * @param buf
 */
void generic_onoff_set_unack(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx,
                             struct net_buf_simple *buf);

void generic_onoff_status(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx,
                          struct net_buf_simple *buf);

/**
 * @brief
 *
 * @param model
 * @param ctx
 * @param buf
 */
void generic_level_get(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx,
                       struct net_buf_simple *buf);


/**
 * @brief
 *
 * @param model
 * @param ctx
 * @param buf
 */
void generic_level_set(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx,
                       struct net_buf_simple *buf);


/**
 * @brief
 *
 * @param model
 * @param ctx
 * @param buf
 */
void generic_level_set_unack(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx,
                             struct net_buf_simple *buf);


/**
 * @brief
 *
 * @param model
 * @param ctx
 * @param buf
 */
void generic_level_status(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx,
                          struct net_buf_simple *buf);

void send_generic_onoff_set(struct onoff_cli *bt_cli, u16_t message_type);


void send_generic_onoff_get(struct onoff_cli *bt_cli, u16_t message_type);


void send_generic_level_set(struct level_cli *bt_cli, u16_t message_type);

void send_generic_level_get(struct level_cli *bt_cli, u16_t message_type);

// u16_t map_level(s16_t x, s16_t in_min, s16_t in_max, u16_t out_min, u16_t out_max);

#endif
