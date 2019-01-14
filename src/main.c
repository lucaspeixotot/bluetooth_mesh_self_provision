#include "ic_bt.h"
#include "ic_leds_pwm.h"
#include "ic_version.h"

#define SLEEP_TIME 250

#define PERIOD (USEC_PER_SEC / 50)
#define FADESTEP 2000
#define FADESTEP_LEVEL 6553

// Initializing variables
void set_level(u16_t pulse_width);
void set_state(u8_t state);
ic_leds_pwm_device_t leds_pwm_device = {0};

static struct level_srv led_level_elems[] = {
    {
        .level_state = &leds_pwm_device.pulse_width,
        .set_level   = set_level,
        .tid         = 0,
    },
};

static struct onoff_srv led_onoff_elems[] = {
    {
        .state     = &leds_pwm_device.state,
        .set_state = set_state,
        .tid       = 0,

    },
};

// Initializing BLE
static const uint8_t dev_uuid[16] = {0xdd, 0xdd};


// Defining the msg publishers
// 2 Bytes for opcode
// 2 Bytes for msg content
// 1 Byte for delay and transient
BT_MESH_MODEL_PUB_DEFINE(pub_onoff_srv, NULL, 2 + 2);


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
        ic_leds_pwm_turn_led_on(&leds_pwm_device);
    } else {
        ic_leds_pwm_turn_led_off(&leds_pwm_device);
    }
}

static void bt_ready(int err)
{
    if (err) {
        printk("bt_enable init failed with err %d\n", err);
        return;
    }

    printk("Bluetooth initialized.\n");
    err = bt_mesh_init(&prov, &comp);
    if (err) {
        printk("bt_mesh init failed with err %d\n", err);
        return;
    }

    if (IS_ENABLED(CONFIG_SETTINGS)) {
        settings_load();
    }

    /* This will be a no-op if settings_load() loaded provisioning info */
    bt_mesh_prov_enable(BT_MESH_PROV_ADV | BT_MESH_PROV_GATT);
    printk("Mesh initialized.\n");
}


// Bluetooth mesh initialization

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
