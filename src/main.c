#include "ic_bt.h"
#include "ic_leds.h"
#include "ic_version.h"

#define SLEEP_TIME 250

// Initializing variables
void set_state(u8_t state);
ic_leds_device_t leds_device = {0};
u8_t leds_states[4]          = {0};

static struct onoff_srv led_onoff_elems[] = {
    {
        .state     = &leds_states[0],
        .set_state = set_state,
        .tid       = 0,

    },
};

// security keys

// 0123456789abcdef0123456789abcdef
static const u8_t dev_key[16] = {
    0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
};

// 0123456789abcdef0123456789abcdef
static const u8_t net_key[16] = {
    0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
};

// 0123456789abcdef0123456789abcdef
static const u8_t app_key[16] = {
    0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
};


// 4.3.1.1 Key indices
static const u16_t net_idx;
static const u16_t app_idx;
// 3.8.4 IV Index
static const u32_t iv_index;

// other

static u8_t flags;
static u8_t tid;

// Addresses

#define NODE_ADDR 0x0007
#define GROUP_ADDR 0xc000

static u16_t addr = NODE_ADDR;

// device UUID
// cfa0ea7e-17d9-11e8-86d1-5f1ce28adea1
static const uint8_t dev_uuid[16] = {0xcf, 0xa0, 0xea, 0x7e, 0x17, 0xd9, 0x11, 0xe8,
                                     0x86, 0xd1, 0x5f, 0x1c, 0xe2, 0x8a, 0xde, 0xa1};


// Defining the msg publishers

// 2 Bytes for opcode
// 1 Bytes for msg content
// 1 Byte for delay and transient
BT_MESH_MODEL_PUB_DEFINE(pub_onoff_srv, NULL, 2 + 1 + 1);


// Defining server config
static struct bt_mesh_cfg_srv cfg_srv = {
    .relay            = BT_MESH_RELAY_DISABLED,
    .beacon           = BT_MESH_BEACON_ENABLED,
    .frnd             = BT_MESH_FRIEND_NOT_SUPPORTED,
    .gatt_proxy       = BT_MESH_GATT_PROXY_ENABLED,
    .default_ttl      = 7,
    .net_transmit     = BT_MESH_TRANSMIT(2, 20),
    .relay_retransmit = BT_MESH_TRANSMIT(2, 20),
};

// Health server model
BT_MESH_HEALTH_PUB_DEFINE(health_pub, 0);

static struct bt_mesh_health_srv health_srv = {};
static struct bt_mesh_cfg_cli cfg_cli       = {};

static struct bt_mesh_model root_models[] = {
    BT_MESH_MODEL_CFG_SRV(&cfg_srv),
    BT_MESH_MODEL_CFG_CLI(&cfg_cli),
    BT_MESH_MODEL_HEALTH_SRV(&health_srv, &health_pub),
    BT_MESH_MODEL(BT_MESH_MODEL_ID_GEN_ONOFF_SRV, generic_onoff_srv_op, &pub_onoff_srv,
                  &led_onoff_elems[0]),
};

struct bt_mesh_elem elements[] = {
    BT_MESH_ELEM(0, root_models, BT_MESH_MODEL_NONE),
};

static const struct bt_mesh_comp comp = {
    .cid        = BT_COMP_ID_LF,
    .elem       = elements,
    .elem_count = ARRAY_SIZE(elements),
};

static int output_number(bt_mesh_output_action_t action, u32_t number);
static void prov_complete(u16_t net_idx, u16_t addr);
static void prov_reset(void);

static const struct bt_mesh_prov prov = {
    .uuid           = dev_uuid,
    .output_size    = 1,
    .output_actions = BT_MESH_DISPLAY_NUMBER,
    .output_number  = output_number,
    .complete       = prov_complete,
    .reset          = prov_reset,
};

void set_state(u8_t state)
{
    if (state) {
        ic_leds_turn_on_led(&leds_device, LED0);
    } else {
        ic_leds_turn_off_led(&leds_device, LED0);
    }
}

static int self_provision()
{
    // now we provision ourselves... this is not how it would normally be done!
    int err = bt_mesh_provision(net_key, net_idx, flags, iv_index, addr, dev_key);
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

    /* Add Application Key */
    err = bt_mesh_cfg_app_key_add(net_idx, addr, net_idx, app_idx, app_key, NULL);
    if (err) {
        printk("ERROR adding appkey (err %d)\n", err);
        return err;
    } else {
        printk("added appkey\n");
    }

    /* Bind to generic onoff server model */
    err = bt_mesh_cfg_mod_app_bind(net_idx, addr, addr, app_idx, BT_MESH_MODEL_ID_GEN_ONOFF_SRV,
                                   NULL);
    if (err) {
        printk("ERROR binding to generic onoff server model (err %d)\n", err);
        return err;
    } else {
        printk("bound appkey to generic onoff server model\n");
    }

    /* Bind to Health model */
    err = bt_mesh_cfg_mod_app_bind(net_idx, addr, addr, app_idx, BT_MESH_MODEL_ID_HEALTH_SRV, NULL);
    if (err) {
        printk("ERROR binding to health server model (err %d)\n", err);
        return err;
    } else {
        printk("bound appkey to health server model\n");
    }

    // subscribe to the group address
    err = bt_mesh_cfg_mod_sub_add(net_idx, NODE_ADDR, NODE_ADDR, GROUP_ADDR,
                                  BT_MESH_MODEL_ID_GEN_ONOFF_SRV, NULL);
    if (err) {
        printk("ERROR subscribing to group address (err %d)\n", err);
        return err;
    } else {
        printk("subscribed to group address\n");
    }

    return 0;
}

// Bluetooth mesh initialization
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

    /*if (IS_ENABLED(CONFIG_SETTINGS)) {*/
    /*settings_load();*/
    /*}*/

    /* This will be a no-op if settings_load() loaded provisioning info */
    /*bt_mesh_prov_enable(BT_MESH_PROV_ADV | BT_MESH_PROV_GATT);*/
    printk("Mesh initialized.\n");
}


// GATT PROVISIONING CALLBACK
static int output_number(bt_mesh_output_action_t action, u32_t number)
{
    printk("OOB Number: %u\n", number);
    return 0;
}


static void prov_complete(u16_t net_idx, u16_t addr)
{
    printk("Provisioning was completed.\n");
}

static void prov_reset(void)
{
    bt_mesh_prov_enable(BT_MESH_PROV_ADV | BT_MESH_PROV_GATT);
}

int configure_board()
{
    ic_leds_init_device(&leds_device);
    ic_leds_configure(&leds_device);
    return 0;
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
