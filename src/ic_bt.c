/**
 * @file ic_bt.c
 * @Lucas Peixoto (lpdac@ic.ufal.br)
 * @brief
 * @version 0.1
 * @date 2018-12-25
 *
 * @copyright Copyright (c) 2018
 *
 */

#include "ic_bt.h"

const struct bt_mesh_model_op generic_onoff_cli_op[] = {
    {BT_MESH_MODEL_OP_GENERIC_ONOFF_STATUS, 1, generic_onoff_status},
    BT_MESH_MODEL_OP_END,
};

const struct bt_mesh_model_op generic_onoff_srv_op[] = {
    {BT_MESH_MODEL_OP_GENERIC_ONOFF_GET, 0, generic_onoff_get},
    {BT_MESH_MODEL_OP_GENERIC_ONOFF_SET, 2, generic_onoff_set},
    {BT_MESH_MODEL_OP_GENERIC_ONOFF_SET_UNACK, 2, generic_onoff_set_unack},
    BT_MESH_MODEL_OP_END,
};
const struct bt_mesh_model_op generic_level_srv_op[] = {
    {BT_MESH_MODEL_OP_GENERIC_LEVEL_GET, 0, generic_level_get},
    {BT_MESH_MODEL_OP_GENERIC_LEVEL_SET, 2, generic_level_set},
    {BT_MESH_MODEL_OP_GENERIC_LEVEL_SET_UNACK, 2, generic_level_set_unack},
    BT_MESH_MODEL_OP_END,
};

const struct bt_mesh_model_op generic_level_cli_op[] = {
    {BT_MESH_MODEL_OP_GENERIC_LEVEL_STATUS, 1, generic_level_status},
    BT_MESH_MODEL_OP_END,
};

void generic_onoff_get(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx,
                       struct net_buf_simple *buf)
{
    printk("[0x%04x]: Received a get msg from group address 0x%04x, sended by 0x%04x.\n",
           bt_mesh_model_elem(model)->addr, ctx->recv_dst, ctx->addr);
    struct bt_mesh_model_pub *pub_cli;
    pub_cli                    = model->pub;
    struct onoff_srv *cur_elem = model->user_data;


    // 2 bytes for the opcode
    // 1 bytes parameters: present onoff value
    // 2 optional bytes for target onoff and remaining time
    // 4 additional bytes for the TransMIC


    printk("Sending onoff status msg to 0x%04x, value -> %d\n", pub_cli->addr, !*cur_elem->state);
    bt_mesh_model_msg_init(pub_cli->msg, BT_MESH_MODEL_OP_GENERIC_ONOFF_STATUS);
    net_buf_simple_add_u8(pub_cli->msg, *cur_elem->state);
    int err = bt_mesh_model_publish(model);
    if (err) {
        printk("bt_mesh_model_publish err %d, sending msg to 0x%04x\n", err, pub_cli->addr);
    }
}


void generic_onoff_set(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx,
                       struct net_buf_simple *buf)
{
    generic_onoff_set_unack(model, ctx, buf);
    generic_onoff_get(model, ctx, buf);
}


void generic_onoff_set_unack(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx,
                             struct net_buf_simple *buf)
{
    printk("[0x%04x]: Received a set msg from group address 0x%04x, sended by 0x%04x.\n",
           bt_mesh_model_elem(model)->addr, ctx->recv_dst, ctx->addr);
    struct onoff_srv *elem = model->user_data;
    u8_t buflen            = buf->len;
    u8_t new_onoff_state   = net_buf_simple_pull_u8(buf);
    u8_t tid               = net_buf_simple_pull_u8(buf);

    if (buflen > 2) {
        printk("[wrn]: message contains transition_time field - processing not implemented\n");
    }

    if (buflen > 3) {
        printk("[wrn]: message contains delay field - processing not implemented\n");
    }

    elem->set_state(new_onoff_state);
    printk("[0x%04x]: Value msg -> %d/ TID -> %d\n", bt_mesh_model_elem(model)->addr,
           !new_onoff_state, tid);
}


void generic_onoff_status(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx,
                          struct net_buf_simple *buf)
{
    u8_t state            = net_buf_simple_pull_u8(buf);
    struct onoff_cli *cli = model->user_data;
    cli->state            = state;

    printk("Node 0x%04x OnOff status from 0x%04x with state 0x%02x\n",
           bt_mesh_model_elem(model)->addr, ctx->addr, !state);
}


void generic_level_get(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx,
                       struct net_buf_simple *buf)
{
    printk("[0x%04x]: Received a get level msg from group address 0x%04x, sended by 0x%04x.\n",
           bt_mesh_model_elem(model)->addr, ctx->recv_dst, ctx->addr);
    struct bt_mesh_model_pub *pub_cli;
    pub_cli                = model->pub;
    struct level_srv *elem = model->user_data;
    map(s16_t level, s16_t, *elem->level_state, 0, LEVEL_MAX, S16_MIN, S16_MAX);
    map(s16_t level_perc, s16_t, level, S16_MIN, S16_MAX, 0, 100);

    // 2 bytes for the opcode
    // 1 bytes parameters: present onoff value
    // 2 optional bytes for target onoff and remaining time
    // 4 additional bytes for the TransMIC

    printk("Sending level status msg to 0x%04x, value -> %d\n", pub_cli->addr, level_perc);
    bt_mesh_model_msg_init(pub_cli->msg, BT_MESH_MODEL_OP_GENERIC_LEVEL_STATUS);
    net_buf_simple_add_le16(pub_cli->msg, level);
    int err = bt_mesh_model_publish(model);
    if (err) {
        printk("bt_mesh_model_publish err %d, sending msg to 0x%04x\n", err, pub_cli->addr);
    }
}


void generic_level_set(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx,
                       struct net_buf_simple *buf)
{
    generic_level_set_unack(model, ctx, buf);
    generic_level_get(model, ctx, buf);
}


void generic_level_set_unack(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx,
                             struct net_buf_simple *buf)
{
    printk("[0x%04x]: Received a set level msg from group address 0x%04x, sended by 0x%04x.\n",
           bt_mesh_model_elem(model)->addr, ctx->recv_dst, ctx->addr);
    struct level_srv *elem = model->user_data;
    u8_t buflen            = buf->len;
    s16_t new_level_state  = net_buf_simple_pull_le16(buf);
    u8_t tid               = net_buf_simple_pull_u8(buf);
    map(u16_t new_pulse_state, u16_t, new_level_state, S16_MIN, S16_MAX, 0, LEVEL_MAX);
    map(u16_t new_pulse_state_perc, u16_t, new_pulse_state, 0, LEVEL_MAX, 0, 100);

    if (buflen > 3) {
        printk("[wrn]: message contains transition_time field - processing not implemented\n");
    }

    if (buflen > 4) {
        printk("[wrn]: message contains delay field - processing not implemented\n");
    }

    elem->set_level(new_pulse_state);
    printk("[0x%04x]: Value msg -> %d / TID -> %d\n", bt_mesh_model_elem(model)->addr,
           new_pulse_state_perc, tid);
}


void generic_level_status(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx,
                          struct net_buf_simple *buf)
{
    s16_t level_signed       = net_buf_simple_pull_le16(buf);
    struct level_cli *bt_cli = model->user_data;
    map(u16_t level_unsigned, u16_t, level_signed, S16_MIN, S16_MAX, 0, LEVEL_MAX);
    map(u16_t perc, u16_t, level_unsigned, 0, LEVEL_MAX, 0, 100);

    *bt_cli->level_state = level_unsigned;
    printk("Node 0x%04x level status from 0x%04x with level %d\n", bt_mesh_model_elem(model)->addr,
           ctx->addr, perc);
}

void send_generic_onoff_get(struct onoff_cli *bt_cli, u16_t message_type)
{
    struct bt_mesh_model_pub *pub_cli;
    pub_cli = bt_cli->model_cli->pub;
    printk("Sending onoff get msg to 0x%04x\n", pub_cli->addr);
    bt_mesh_model_msg_init(pub_cli->msg, message_type);
    int err = bt_mesh_model_publish(bt_cli->model_cli);
    if (err) {
        printk("bt_mesh_model_publish err %d, sending msg to 0x%04x\n", err, pub_cli->addr);
    }
}

void send_generic_onoff_set(struct onoff_cli *cli, u16_t message_type)
{
    struct bt_mesh_model_pub *pub_cli;
    pub_cli = cli->model_cli->pub;
    printk("Sending onoff set msg to 0x%04x, value -> %d\n", pub_cli->addr, !cli->act);
    bt_mesh_model_msg_init(pub_cli->msg, message_type);
    net_buf_simple_add_u8(pub_cli->msg, cli->act);
    net_buf_simple_add_u8(pub_cli->msg, cli->tid++);
    int err = bt_mesh_model_publish(cli->model_cli);
    if (err) {
        printk("bt_mesh_model_publish err %d, sending msg to 0x%04x\n", err, pub_cli->addr);
    }
}

void send_generic_level_set(struct level_cli *bt_cli, u16_t message_type)
{
    struct bt_mesh_model_pub *pub_cli;
    pub_cli = bt_cli->model_cli->pub;
    map(s16_t level, s16_t, bt_cli->act, 0, LEVEL_MAX, S16_MIN, S16_MAX);
    map(s16_t level_perc, s16_t, level, S16_MIN, S16_MAX, 0, 100);
    printk("Sending level set msg to 0x%04x, value -> %d\n", pub_cli->addr, level_perc);
    bt_mesh_model_msg_init(pub_cli->msg, message_type);
    net_buf_simple_add_le16(pub_cli->msg, level);
    net_buf_simple_add_u8(pub_cli->msg, bt_cli->tid++);
    int err = bt_mesh_model_publish(bt_cli->model_cli);
    if (err) {
        printk("bt_mesh_model_publish err %d, sending msg to 0x%04x\n", err, pub_cli->addr);
    }
}
void send_generic_level_get(struct level_cli *bt_cli, u16_t message_type)
{
    struct bt_mesh_model_pub *pub_cli;
    pub_cli = bt_cli->model_cli->pub;
    printk("Sending level get msg to 0x%04x\n", pub_cli->addr);
    bt_mesh_model_msg_init(pub_cli->msg, message_type);
    int err = bt_mesh_model_publish(bt_cli->model_cli);
    if (err) {
        printk("bt_mesh_model_publish err %d, sending msg to 0x%04x\n", err, pub_cli->addr);
    }
}
