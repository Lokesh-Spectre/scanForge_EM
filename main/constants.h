
// #define GATTS_TAG "GATTS_DEMO"

// ///Declare the static function
// static void gatts_service_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
// // static void gatts_profile_b_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);

// #define GATTS_SERVICE_UUID_TEST_A   0x00FF
// #define GATTS_CHAR_UUID_TEST_A      0xFF01
// #define GATTS_DESCR_UUID_TEST_A     0x3333
// #define GATTS_NUM_HANDLE_TEST_A     4

// #define GATTS_SERVICE_UUID_TEST_B   0x00EE
// #define GATTS_CHAR_UUID_TEST_B      0xEE01
// #define GATTS_CHAR_UUID_TEST_B_2      0xEE02
// #define GATTS_DESCR_UUID_TEST_B     0x2222
// #define GATTS_DESCR_UUID_TEST_B_2     0x4444
// #define GATTS_NUM_HANDLE_TEST_B     4

// static char test_device_name[ESP_BLE_ADV_NAME_LEN_MAX] = "ESP_GATTS_DEMO";

// #define TEST_MANUFACTURER_DATA_LEN  17

// #define GATTS_DEMO_CHAR_VAL_LEN_MAX 0x40

// #define PREPARE_BUF_MAX_SIZE 1024

// static uint8_t char1_str[] = {0x11,0x22,0x33};
// static esp_gatt_char_prop_t a_property = 0;
// static esp_gatt_char_prop_t b_property = 0;

// static esp_attr_value_t gatts_demo_char1_val =
// {
//     .attr_max_len = GATTS_DEMO_CHAR_VAL_LEN_MAX,
//     .attr_len     = sizeof(char1_str),
//     .attr_value   = char1_str,
// };

// static uint8_t adv_config_done = 0;
// #define adv_config_flag      (1 << 0)
// #define scan_rsp_config_flag (1 << 1)

// #ifdef CONFIG_EXAMPLE_SET_RAW_ADV_DATA
// static uint8_t raw_adv_data[] = {
//     /* Flags */
//     0x02, ESP_BLE_AD_TYPE_FLAG, 0x06,               // Length 2, Data Type ESP_BLE_AD_TYPE_FLAG, Data 1 (LE General Discoverable Mode, BR/EDR Not Supported)
//     /* TX Power Level */
//     0x02, ESP_BLE_AD_TYPE_TX_PWR, 0xEB,             // Length 2, Data Type ESP_BLE_AD_TYPE_TX_PWR, Data 2 (-21)
//     /* Complete 16-bit Service UUIDs */
//     0x03, ESP_BLE_AD_TYPE_16SRV_CMPL, 0xAB, 0xCD    // Length 3, Data Type ESP_BLE_AD_TYPE_16SRV_CMPL, Data 3 (UUID)
// };

// static uint8_t raw_scan_rsp_data[] = {
//     /* Complete Local Name */
//     0x0F, ESP_BLE_AD_TYPE_NAME_CMPL, 'E', 'S', 'P', '_', 'G', 'A', 'T', 'T', 'S', '_', 'D', 'E', 'M', 'O'   // Length 15, Data Type ESP_BLE_AD_TYPE_NAME_CMPL, Data (ESP_GATTS_DEMO)
// };
// #else

// static uint8_t adv_service_uuid128[32] = {
//     /* LSB <--------------------------------------------------------------------------------> MSB */
//     //first uuid, 16bit, [12],[13] is the value
//     0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xEE, 0x00, 0x00, 0x00,
//     //second uuid, 32bit, [12], [13], [14], [15] is the value
//     0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00,
// };

// // The length of adv data must be less than 31 bytes
// //static uint8_t test_manufacturer[TEST_MANUFACTURER_DATA_LEN] =  {0x12, 0x23, 0x45, 0x56};
// //adv data
// static esp_ble_adv_data_t adv_data = {
//     .set_scan_rsp = false,
//     .include_name = true,
//     .include_txpower = false,
//     .min_interval = 0x0006, //slave connection min interval, Time = min_interval * 1.25 msec
//     .max_interval = 0x0010, //slave connection max interval, Time = max_interval * 1.25 msec
//     .appearance = 0x00,
//     .manufacturer_len = 0, //TEST_MANUFACTURER_DATA_LEN,
//     .p_manufacturer_data =  NULL, //&test_manufacturer[0],
//     .service_data_len = 0,
//     .p_service_data = NULL,
//     .service_uuid_len = sizeof(adv_service_uuid128),
//     .p_service_uuid = adv_service_uuid128,
//     .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
// };
// // scan response data
// static esp_ble_adv_data_t scan_rsp_data = {
//     .set_scan_rsp = true,
//     .include_name = true,
//     .include_txpower = true,
//     //.min_interval = 0x0006,
//     //.max_interval = 0x0010,
//     .appearance = 0x00,
//     .manufacturer_len = 0, //TEST_MANUFACTURER_DATA_LEN,
//     .p_manufacturer_data =  NULL, //&test_manufacturer[0],
//     .service_data_len = 0,
//     .p_service_data = NULL,
//     .service_uuid_len = sizeof(adv_service_uuid128),
//     .p_service_uuid = adv_service_uuid128,
//     .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
// };

// #endif /* CONFIG_SET_RAW_ADV_DATA */

// static esp_ble_adv_params_t adv_params = {
//     .adv_int_min        = 0x20,
//     .adv_int_max        = 0x40,
//     .adv_type           = ADV_TYPE_IND,
//     .own_addr_type      = BLE_ADDR_TYPE_PUBLIC,
//     //.peer_addr            =
//     //.peer_addr_type       =
//     .channel_map        = ADV_CHNL_ALL,
//     .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
// };

// #define PROFILE_NUM 2
// #define PROFILE_A_APP_ID 0
// #define PROFILE_B_APP_ID 1

// struct gatts_profile_inst {
//     esp_gatts_cb_t gatts_cb;
//     uint16_t gatts_if;
//     uint16_t app_id;
//     uint16_t conn_id;
//     uint16_t service_handle;
//     esp_gatt_srvc_id_t service_id;
    
//     uint16_t char_handle;
//     esp_bt_uuid_t char_uuid;
//     esp_gatt_perm_t perm;
//     esp_gatt_char_prop_t property;
//     uint16_t descr_handle;
//     esp_bt_uuid_t descr_uuid;

//     uint16_t char_handle_ss;
//     esp_bt_uuid_t char_uuid_ss;
//     uint16_t descr_handle_ss;
//     esp_bt_uuid_t descr_uuid_ss;
// };

// /* One gatt-based profile one app_id and one gatts_if, this array will store the gatts_if returned by ESP_GATTS_REG_EVT */
// static struct gatts_profile_inst gl_profile_tab[PROFILE_NUM] = {
//     [PROFILE_A_APP_ID] = {
//         .gatts_cb = gatts_service_event_handler,
//         .gatts_if = ESP_GATT_IF_NONE,       /* Not get the gatt_if, so initial is ESP_GATT_IF_NONE */
//     },
// };

// typedef struct {
//     uint8_t                 *prepare_buf;
//     int                     prepare_len;
// } prepare_type_env_t;

// static prepare_type_env_t a_prepare_write_env;
// static prepare_type_env_t b_prepare_write_env;
