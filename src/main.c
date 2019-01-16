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
#include <misc/printk.h>
#include <zephyr.h>

#include "ic_bt.h"
#include "ic_buttons.h"
#include "ic_leds.h"
#include "ic_version.h"

#define SLEEP_TIME 250
#define PRIORITY 7
#define STACKSIZE 1024

// LED STUFF ------------------------------------------------------------------
u8_t leds_states[4]       = {0};
ic_leds_device_t leds_dev = {0};

// BUTTON STUFF ---------------------------------------------------------------
struct k_work pressed_work[BUTTON_NUMBERS];
ic_buttons_device_t buttons_dev;
u32_t time      = 0;
u32_t last_time = 0;

// BLE STUFF ------------------------------------------------------------------

// Addresses

#define ROOT_ADDR 0x00e1
#define LED_ADDR 0x00e2
#define BUTTON_ADDR 0x00e3

#define GROUP_ADDR 0xcea0
#define GROUP_PUB_ADDR 0xcae0

static const uint8_t dev_uuid[16] = {0xcf, 0xa0, 0xea, 0x7e, 0x17, 0xd9, 0x11, 0xe8,
                                     0x86, 0xd1, 0x5f, 0x1c, 0xe2, 0x8a, 0xde, 0xa7};

// Security keys
static const u8_t net_key[16] = {
    0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
};
static const u8_t dev_key[16] = {
    0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
};
static const u8_t app_key[16] = {
    0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
};

// Variables for configuration and self provisioning
static const u16_t net_idx;
static const u16_t app_idx;
static const u32_t iv_index;
static u8_t flags;

// Node construction
// 2 bytes for opcode
// 1 byte for msg
// 1 byte for delay e etc
BT_MESH_MODEL_PUB_DEFINE(pub_onoff_srv, NULL, 2 + 2);
BT_MESH_MODEL_PUB_DEFINE(pub_onoff_cli, NULL, 2 + 2);

// Health server model
BT_MESH_HEALTH_PUB_DEFINE(health_pub, 0);

// Defining config server
static struct bt_mesh_cfg_srv cfg_srv = {
    .relay            = BT_MESH_RELAY_DISABLED,
    .beacon           = BT_MESH_BEACON_ENABLED,
    .frnd             = BT_MESH_FRIEND_NOT_SUPPORTED,
    .gatt_proxy       = BT_MESH_GATT_PROXY_ENABLED,
    .default_ttl      = 7,
    .net_transmit     = BT_MESH_TRANSMIT(2, 20),
    .relay_retransmit = BT_MESH_TRANSMIT(2, 20),
};


static struct bt_mesh_health_srv health_srv = {};
static struct bt_mesh_cfg_cli cfg_cli       = {};
static void set_state(u8_t state)
{
    if (!state) {
        ic_leds_turn_on_led(&leds_dev, LED0);
    } else {
        ic_leds_turn_off_led(&leds_dev, LED0);
    }
}

static struct onoff_srv led_onoff_elems[] = {
    {
        .state     = &leds_states[0],
        .tid       = 0,
        .set_state = set_state,
    },
};

static struct onoff_cli cli_onoff_elems[] = {
    {
        .state = 0,
        .tid   = 0,
    },
};

static struct bt_mesh_model root_models[] = {
    BT_MESH_MODEL_CFG_SRV(&cfg_srv),
    BT_MESH_MODEL_CFG_CLI(&cfg_cli),
    BT_MESH_MODEL_HEALTH_SRV(&health_srv, &health_pub),
};

static struct bt_mesh_model led_models[] = {
    BT_MESH_MODEL(BT_MESH_MODEL_ID_GEN_ONOFF_SRV, generic_onoff_srv_op, &pub_onoff_srv,
                  &led_onoff_elems[0]),
};

static struct bt_mesh_model button_models[] = {
    BT_MESH_MODEL(BT_MESH_MODEL_ID_GEN_ONOFF_CLI, generic_onoff_cli_op, &pub_onoff_cli,
                  &cli_onoff_elems[0]),
};
struct bt_mesh_elem elements[] = {
    BT_MESH_ELEM(0, root_models, BT_MESH_MODEL_NONE),
    BT_MESH_ELEM(0, led_models, BT_MESH_MODEL_NONE),
    BT_MESH_ELEM(0, button_models, BT_MESH_MODEL_NONE),
};

static const struct bt_mesh_comp comp = {
    .cid        = BT_COMP_ID_LF,
    .elem       = elements,
    .elem_count = ARRAY_SIZE(elements),
};

static const struct bt_mesh_prov prov = {
    .uuid = dev_uuid,
};


// BOARD STUFF ----------------------------------------------------------------

void button1_handler(struct k_work *work)
{
    printk("Button 1 handler started.\n");
    struct onoff_cli *h = CONTAINER_OF(work, struct onoff_cli, callback_work);
    h->act              = !h->state;
    send_generic_onoff_set(h, BT_MESH_MODEL_OP_GENERIC_ONOFF_SET);
}

void ic_buttons_callback(struct device *buttons_device, struct gpio_callback *callback,
                         u32_t button_pin_mask)
{
    time = k_uptime_get_32();

    if (time < last_time + BUTTON_DEBOUNCE_DELAY_MS) {
        last_time = time;
        return;
    }

    if (ic_buttons_pin_to_i(button_pin_mask) == 0) {
        printk("Button 1 was pressed.\n");
        k_work_submit(&cli_onoff_elems[0].callback_work);
    } else if (ic_buttons_pin_to_i(button_pin_mask) == 1) {
        printk("The button 2 was not configured.\n");
    } else if (ic_buttons_pin_to_i(button_pin_mask) == 2) {
        printk("The button 3 was not configured.\n");
    } else if (ic_buttons_pin_to_i(button_pin_mask) == 3) {
        printk("The button 4 was not configured.\n");
    }
    last_time = time;
}

static int configure_board()
{
    int err;

    // Led configuration
    err = ic_leds_init_device(&leds_dev);
    if (err) {
        printk("Error initializing leds device\n");
        return err;
    }

    err = ic_leds_configure(&leds_dev);
    if (err) {
        printk("Error configuring leds device\n");
        return err;
    }

    ic_leds_turn_off_led(&leds_dev, LED1);
    ic_leds_turn_off_led(&leds_dev, LED2);
    ic_leds_turn_off_led(&leds_dev, LED3);

    // Button configuration
    ic_buttons_init_device(&buttons_dev);
    ic_buttons_configure(&buttons_dev);
    ic_buttons_configure_callback(&buttons_dev, ic_buttons_callback);
    cli_onoff_elems[0].callback_work = pressed_work[0];
    cli_onoff_elems[0].model_cli     = &button_models[0];
    k_work_init(&cli_onoff_elems[0].callback_work, button1_handler);

    return 0;
}

static int self_provision()
{
    // now we provision ourselves... this is not how it would normally be done!
    int err = bt_mesh_provision(net_key, net_idx, flags, iv_index, ROOT_ADDR, dev_key);
    if (err) {
        printk("Provisioning failed (err %d)\n", err);
        return err;
    }
    printk("Provisioning completed\n");
    return 0;
}

static int self_configure()
{
    int err;
    printk("configuring...\n");

    /* Add Application Key to root models*/
    err = bt_mesh_cfg_app_key_add(net_idx, ROOT_ADDR, net_idx, app_idx, app_key, NULL);
    if (err) {
        printk("ERROR adding appkey (err %d)\n", err);
        return err;
    } else {
        printk("added appkey\n");
    }

    /* Bind to generic onoff server model */
    err = bt_mesh_cfg_mod_app_bind(net_idx, ROOT_ADDR, LED_ADDR, app_idx,
                                   BT_MESH_MODEL_ID_GEN_ONOFF_SRV, NULL);
    if (err) {
        printk("ERROR binding to generic onoff server model (err %d)\n", err);
        return err;
    } else {
        printk("bound appkey to generic onoff server model\n");
    }

    /* Bind to Health model */
    err = bt_mesh_cfg_mod_app_bind(net_idx, ROOT_ADDR, ROOT_ADDR, app_idx,
                                   BT_MESH_MODEL_ID_HEALTH_SRV, NULL);
    if (err) {
        printk("ERROR binding to health server model (err %d)\n", err);
        return err;
    } else {
        printk("bound appkey to health server model\n");
    }

    // subscribe srv model to the group address
    err = bt_mesh_cfg_mod_sub_add(net_idx, ROOT_ADDR, LED_ADDR, GROUP_ADDR,
                                  BT_MESH_MODEL_ID_GEN_ONOFF_SRV, NULL);
    if (err) {
        printk("ERROR subscribing to group address (err %d)\n", err);
        return err;
    } else {
        printk("subscribed to group address\n");
    }

    /* Bind to generic onoff client model */
    err = bt_mesh_cfg_mod_app_bind(net_idx, ROOT_ADDR, BUTTON_ADDR, app_idx,
                                   BT_MESH_MODEL_ID_GEN_ONOFF_CLI, NULL);
    if (err) {
        printk("ERROR binding to generic onoff client model (err %d)\n", err);
        return err;
    } else {
        printk("bound appkey to generic onoff server model\n");
    }

    // subscribe cli model to the group address
    err = bt_mesh_cfg_mod_sub_add(net_idx, ROOT_ADDR, BUTTON_ADDR, GROUP_PUB_ADDR,
                                  BT_MESH_MODEL_ID_GEN_ONOFF_CLI, NULL);
    if (err) {
        printk("ERROR subscribing to group address (err %d)\n", err);
        return err;
    } else {
        printk("subscribed to group address\n");
    }

    // publish cli and srv model to its respective groups addresses
    pub_onoff_cli.addr = GROUP_ADDR;
    pub_onoff_srv.addr = GROUP_PUB_ADDR;

    return 0;
}


static void bt_ready(int err)
{
    if (err) {
        printk("bt_enable init failed with err %d\n", err);
        return;
    }

    printk("Bluetooth initialised OK\n");

    // self-provision and initialise with node composition data
    err = bt_mesh_init(&prov, &comp);
    if (err) {
        printk("bt_mesh_init failed with err %d\n", err);
        return;
    }

    err = self_provision();
    if (err) {
        printk("ERROR: SELF-PROVISIONING FAILED");
    } else {
        printk("self-provisioned OK\n");
    }

    err = self_configure();

    if (err) {
        printk("ERROR: INITIALISATION FAILED");
    } else {
        printk("Mesh initialised OK\n");
    }
}

void main(void)
{
    printk("Firmware version: %d.%d.%d\n", ic_version_get_major(), ic_version_get_minor(),
           ic_version_get_build());

    int err;
    err = configure_board();
    if (err) {
        return;
    }

    err = bt_enable(bt_ready);
    if (err) {
        printk("bt_enable failed with err %d\n", err);
        return;
    }

    while (1) {
        k_sleep(SLEEP_TIME);
    }
}
