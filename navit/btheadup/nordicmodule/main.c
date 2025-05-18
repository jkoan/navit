/**
 * Copyright (c) 2014 - 2020, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/** @example examples/ble_peripheral/ble_app_hrs/main.c
 *
 * @brief Heart Rate Service Sample Application main file.
 *
 * This file contains the source code for a sample application using the Heart Rate service
 * (and also Battery and Device Information services). This application uses the
 * @ref srvlib_conn_params module.
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "nrf_sdm.h"
#include "app_error.h"
#include "ble.h"
#include "ble_err.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_bas.h"
#include "ble_hrs.h"
#include "ble_dis.h"
#include "ble_conn_params.h"
#include "sensorsim.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "nrf_sdh_soc.h"
#include "app_timer.h"
#include "bsp_btn_ble.h"
#include "peer_manager.h"
#include "peer_manager_handler.h"
#include "fds.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_lesc.h"
#include "nrf_ble_qwr.h"
#include "ble_conn_state.h"
#include "nrf_pwr_mgmt.h"
#include "ble_nus.h"
#include "nrf_delay.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "u8g2.h"
#include "u8x8.h"
#include "crc16.h"

#define USE_TWI 0
#define USE_SPI 1

#define SPI_INSTANCE 0

#include <stdio.h>
#include "boards.h"
#include "app_util_platform.h"
#include "app_error.h"
#if USE_TWI
#include "nrf_drv_twi.h"
#endif

#if USE_SPI
#include <nrfx_spim.h>
#endif

#include "nrf_delay.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "u8g2.h"
#include "u8x8.h"

#include "math.h"

#include <images/ez.h>
#include <images/navit.h>
#include <images/speedsign.h>
#include <images/condspeed.h>
#include <images/eisenzelt.h>
#include <images/navit_plain_bk_96_96.h>
#include "imageheaders.h"

enum item_type {
#define ITEM2(x,y) type_##y=x,
#define ITEM(x) type_##x,
#include "imageitem_def.h"
#undef ITEM2
#undef ITEM
};

struct item_name {
    enum item_type item;
    char *name;
};

struct item_name item_names[] = {
#define ITEM2(x,y) ITEM(y)
#define ITEM(x) { type_##x, x##_bk_bits },
#include "imageitem_def.h"
#undef ITEM2
#undef ITEM
};

#define DEVICE_NAME                         "BT HEADUP"                            /**< Name of device. Will be included in the advertising data. */
#define MANUFACTURER_NAME                   "NordicSemiconductor"                   /**< Manufacturer. Will be passed to Device Information Service. */
#define APP_ADV_INTERVAL                    64                                     /**< The advertising interval (in units of 0.625 ms. This value corresponds to 187.5 ms). */

#define APP_ADV_DURATION                    1800                                   /**< The advertising duration (180 seconds) in units of 10 milliseconds. */

#define APP_BLE_CONN_CFG_TAG                1                                       /**< A tag identifying the SoftDevice BLE configuration. */
#define APP_BLE_OBSERVER_PRIO               3                                       /**< Application's BLE observer priority. You shouldn't need to modify this value. */

#define DISPLAY_UPDATE_TIMER_INTERVAL         APP_TIMER_TICKS(900)                   /**< Battery level measurement interval (ticks). */
//#define MIN_BATTERY_LEVEL                   81                                      /**< Minimum simulated battery level. */
//#define MAX_BATTERY_LEVEL                   100                                     /**< Maximum simulated 7battery level. */
//#define BATTERY_LEVEL_INCREMENT             1                                       /**< Increment between each simulated battery level measurement. */
//
//#define HEART_RATE_MEAS_INTERVAL            APP_TIMER_TICKS(1000)                   /**< Heart rate measurement interval (ticks). */
//#define MIN_HEART_RATE                      140                                     /**< Minimum heart rate as returned by the simulated measurement function. */
//#define MAX_HEART_RATE                      300                                     /**< Maximum heart rate as returned by the simulated measurement function. */
//#define HEART_RATE_INCREMENT                10                                      /**< Value by which the heart rate is incremented/decremented for each call to the simulated measurement function. */
//
//#define RR_INTERVAL_INTERVAL                APP_TIMER_TICKS(300)                    /**< RR interval interval (ticks). */
//#define MIN_RR_INTERVAL                     100                                     /**< Minimum RR interval as returned by the simulated measurement function. */
//#define MAX_RR_INTERVAL                     500                                     /**< Maximum RR interval as returned by the simulated measurement function. */
//#define RR_INTERVAL_INCREMENT               1                                       /**< Value by which the RR interval is incremented/decremented for each call to the simulated measurement function. */
//
//#define SENSOR_CONTACT_DETECTED_INTERVAL    APP_TIMER_TICKS(5000)                   /**< Sensor Contact Detected toggle interval (ticks). */

#define MIN_CONN_INTERVAL                   MSEC_TO_UNITS(400, UNIT_1_25_MS)        /**< Minimum acceptable connection interval (0.4 seconds). */
#define MAX_CONN_INTERVAL                   MSEC_TO_UNITS(400, UNIT_1_25_MS)        /**< Maximum acceptable connection interval (0.65 second). */
#define SLAVE_LATENCY                       0                                       /**< Slave latency. */
#define CONN_SUP_TIMEOUT                    MSEC_TO_UNITS(4000, UNIT_10_MS)         /**< Connection supervisory timeout (4 seconds). */

#define FIRST_CONN_PARAMS_UPDATE_DELAY      APP_TIMER_TICKS(5000)                   /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY       APP_TIMER_TICKS(30000)                  /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT        3                                       /**< Number of attempts before giving up the connection parameter negotiation. */

#define LESC_DEBUG_MODE                     0                                       /**< Set to 1 to use LESC debug keys, allows you to use a sniffer to inspect traffic. */

#define SEC_PARAM_BOND                      1                                       /**< Perform bonding. */
#define SEC_PARAM_MITM                      0                                       /**< Man In The Middle protection not required. */
#define SEC_PARAM_LESC                      1                                       /**< LE Secure Connections enabled. */
#define SEC_PARAM_KEYPRESS                  0                                       /**< Keypress notifications not enabled. */
#define SEC_PARAM_IO_CAPABILITIES           BLE_GAP_IO_CAPS_NONE                    /**< No I/O capabilities. */
#define SEC_PARAM_OOB                       0                                       /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE              7                                       /**< Minimum encryption key size. */
#define SEC_PARAM_MAX_KEY_SIZE              16                                      /**< Maximum encryption key size. */

#define DEAD_BEEF                           0xDEADBEEF                              /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

BLE_NUS_DEF(m_nus, NRF_SDH_BLE_TOTAL_LINK_COUNT);

bool cmdreceived = false;
int first = 1;

NRF_BLE_GATT_DEF(m_gatt); /**< GATT module instance. */
NRF_BLE_QWR_DEF(m_qwr); /**< Context for the Queued Write module.*/
BLE_ADVERTISING_DEF(m_advertising); /**< Advertising module instance. */
APP_TIMER_DEF(m_displayupdate_timer_id); /**< Battery timer. */

static uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID; /**< Handle of the current connection. */

#define NUS_SERVICE_UUID_TYPE BLE_UUID_TYPE_VENDOR_BEGIN
static uint16_t m_ble_nus_max_data_len = 50; //BLE_GATT_ATT_MTU_DEFAULT - 3;

static ble_uuid_t m_adv_uuids[] = /**< Universally unique service identifiers. */
{
    { BLE_UUID_NUS_SERVICE, NUS_SERVICE_UUID_TYPE }
};

// BLE commands
enum btheadup_cmds {
    BTHEADUP_DISTANCE = 0x10,
    BTHEADUP_DIRECTION,
    BTHEADUP_IMPERIAL,
    BTHEADUP_NAVIMAGENAME,
    BTHEADUP_COORDINATESGEO,
    BTHEADUP_NAVNEXTTURNIMAGE,
    BTHEADUP_GPSNUMSATSUSED,
    BTHEADUP_GPSHDOP,
    BTHEADUP_STREETNAME,
    BTHEADUP_STREETSYSNAME,
    BTHEADUP_DESTTIME, // 0x1A
    BTHEADUP_DESTLENGTH, //
    BTHEADUP_NEXTSTREETNAME,
    BTHEADUP_NEXTSTREETSYSNAME,
    BTHEADUP_NEXTMANEUVLENGTH, //
    BTHEADUP_GPSSIGNALSTRENGTH, //0x20
    BTHEADUP_ROUTESPEED,
    BTHEADUP_HANDLEDIRECTION,
    BTHEADUP_VEHICLESPEED,
    BTHEADUP_GPSHEIGHT,
};

bool btconnected = false; // flag representing the Bluetooth connection status
bool onbtdisconnect = false; // flag triggering actions on disconnect
bool onbtconnect = false; // flag triggering actions on connect

// cmd buffer
typedef struct {
    char data[50];
    bool used;
} commands;

// size
#define CMDBUFFERSIZE 8

static volatile commands cmds[CMDBUFFERSIZE];

bool update_display = false; // flag to trigger display update
bool receive_data = false; // flag to enable receive of NUS data

static u8g2_t u8g2;
#define DISPLAYWIDTH 128
//#define EISENZELT_BUILD

struct point p;
int r = DISPLAYWIDTH / 10 + 1;
int cr = DISPLAYWIDTH / 5;

int dir = 0;
int hdir = 180;
int satcnt = 0;

//double speedf = 10.0f;
//char speed[10] = "";
double navcoordlon = 0.0;
double navcoordlat = 0.0;
char navcoordlons[20] = "";
char navcoordlats[20] = "";

char distances[8] = "";
char distancedests[8] = "";
char etas[20] = "";
char times[20] = "";
char dlengths[20] = "";
char nmls[20] = "";
char routespeeds[4] = "";
char condspeeds[4] = "";
char vehiclespeeds[4] = "";
char currentstreetname[30] = "";
char nextstreetname[30] = "";
char directlengths[20] = "";
char gpsheights[8] = "";
char *navimage;

//char *item_to_name(enum item_type item) {
//    int i;
//
//    for (i=0 ; i < sizeof(item_names)/sizeof(struct item_name) ; i++) {
//        if (item_names[i].item == item)
//            return item_names[i].name;
//    }
//    return NULL;
//}

/**@brief Callback function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num   Line number of the failing ASSERT call.
 * @param[in] file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t *p_file_name) {
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

/**@brief Clear bond information from persistent storage.
 */
static void delete_bonds(void) {
    ret_code_t err_code;

    NRF_LOG_INFO("Erase bonds!");

    err_code = pm_peers_delete();
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for starting advertising.
 */
void advertising_start(bool erase_bonds) {
    if (erase_bonds == true) {
        delete_bonds();
        // Advertising is started by PM_EVT_PEERS_DELETE_SUCCEEDED event.
    } else {
        ret_code_t err_code;

        err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
        APP_ERROR_CHECK(err_code);
    }
}

/**@brief Function for handling Peer Manager events.
 *
 * @param[in] p_evt  Peer Manager event.
 */
static void pm_evt_handler(pm_evt_t const *p_evt) {
    pm_handler_on_pm_evt(p_evt);
    pm_handler_flash_clean(p_evt);

    switch (p_evt->evt_id) {
    case PM_EVT_PEERS_DELETE_SUCCEEDED:
        advertising_start(false);
        break;

    default:
        break;
    }
}

///**@brief Function for performing battery measurement and updating the Battery Level characteristic
// *        in Battery Service.
// */
//static void battery_level_update(void)
//{
//
//}

/**@brief Function for handling the Battery measurement timer timeout.
 *
 * @details This function will be called each time the battery level measurement timer expires.
 *
 * @param[in] p_context  Pointer used for passing some arbitrary information (context) from the
 *                       app_start_timer() call to the timeout handler.
 */
static void battery_level_meas_timeout_handler(void *p_context) {
    UNUSED_PARAMETER(p_context);
    update_display = true;
}

/**@brief Function for the Timer initialization.
 *
 * @details Initializes the timer module. This creates and starts application timers.
 */
static void timers_init(void) {
    ret_code_t err_code;

    // Initialize timer module.
    err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);

    // Create timers.
    err_code = app_timer_create(&m_displayupdate_timer_id, APP_TIMER_MODE_REPEATED, battery_level_meas_timeout_handler);
    APP_ERROR_CHECK(err_code);

}

/**@brief Function for the GAP initialization.
 *
 * @details This function sets up all the necessary GAP (Generic Access Profile) parameters of the
 *          device including the device name, appearance, and the preferred connection parameters.
 */
static void gap_params_init(void) {
    ret_code_t err_code;
    ble_gap_conn_params_t gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code = sd_ble_gap_device_name_set(&sec_mode, (const uint8_t*) DEVICE_NAME, strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    err_code = sd_ble_gap_appearance_set(BLE_APPEARANCE_GENERIC_DISPLAY);
    APP_ERROR_CHECK(err_code);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}

/**@brief GATT module event handler.
 */
static void gatt_evt_handler(nrf_ble_gatt_t *p_gatt, nrf_ble_gatt_evt_t const *p_evt) {

    if ((m_conn_handle == p_evt->conn_handle) && (p_evt->evt_id == NRF_BLE_GATT_EVT_ATT_MTU_UPDATED)) {
        m_ble_nus_max_data_len = p_evt->params.att_mtu_effective - OPCODE_LENGTH - HANDLE_LENGTH;
        NRF_LOG_INFO("Data len is set to 0x%X(%d)", m_ble_nus_max_data_len, m_ble_nus_max_data_len);
    }
    NRF_LOG_DEBUG("ATT MTU exchange completed. central 0x%x peripheral 0x%x", p_gatt->att_mtu_desired_central,
                  p_gatt->att_mtu_desired_periph);

}

/**@brief Function for initializing the GATT module.
 */
static void gatt_init(void) {
    ret_code_t err_code = nrf_ble_gatt_init(&m_gatt, gatt_evt_handler);
    APP_ERROR_CHECK(err_code);

}

/**@brief Function for handling Queued Write Module errors.
 *
 * @details A pointer to this function will be passed to each service which may need to inform the
 *          application about an error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void nrf_qwr_error_handler(uint32_t nrf_error) {
    APP_ERROR_HANDLER(nrf_error);
}

/**@brief Function for handling the data from the Nordic UART Service.
 *
 * @details This function will process the data received from the Nordic UART BLE Service and send
 *          it to the UART module.
 *
 * @param[in] p_evt       Nordic UART Service event.
 */
/**@snippet [Handling the data received over BLE] */
static void nus_data_handler(ble_nus_evt_t *p_evt) {

    if ((p_evt->type == BLE_NUS_EVT_RX_DATA) && (receive_data == true)) {

//		NRF_LOG_HEXDUMP_DEBUG(p_evt->params.rx_data.p_data, p_evt->params.rx_data.length);

        int index = 0xFFFF;

        for (int i = 0; i < CMDBUFFERSIZE; i++) {
            if (cmds[i].used == 0) {
                index = i;
                cmds[i].used = 1;
                break;
            }
        }

        if (index == 0xFFFF) {
            NRF_LOG_DEBUG("No buffer for RX data!");
            return;
        }

        //copy to buffer and set flag for main loop
        for (uint32_t i = 0; i < (p_evt->params.rx_data.length <= 50 ? p_evt->params.rx_data.length : 50); i++) {
            cmds[index].data[i] = p_evt->params.rx_data.p_data[i];
            //NRF_LOG_DEBUG("CMD: %i index: %i",nus_buffer[0], index);
        }

        if (p_evt->params.rx_data.p_data[p_evt->params.rx_data.length - 2] == '\r'
                && p_evt->params.rx_data.p_data[p_evt->params.rx_data.length - 1] == '\n') {
//				NRF_LOG_DEBUG("CMD complete: %2x. Length: %i bytes, index: %i", cmds[index].data[0], p_evt->params.rx_data.length, index);
            cmdreceived = true;

        }
    }
}

/**@brief Function for initializing services that will be used by the application.
 *
 * @details Initialize the Heart Rate, Battery and Device Information services.
 */
static void services_init(void) {
    ret_code_t err_code;
    ble_nus_init_t nus_init;
    ble_dis_init_t dis_init;
    nrf_ble_qwr_init_t qwr_init =
    { 0 };

    // Initialize Queued Write Module.
    qwr_init.error_handler = nrf_qwr_error_handler;

    err_code = nrf_ble_qwr_init(&m_qwr, &qwr_init);
    APP_ERROR_CHECK(err_code);

    // Initialize Device Information Service.
    memset(&dis_init, 0, sizeof(dis_init));

    ble_srv_ascii_to_utf8(&dis_init.manufact_name_str, (char*) MANUFACTURER_NAME);

    dis_init.dis_char_rd_sec = SEC_OPEN;

    err_code = ble_dis_init(&dis_init);
    APP_ERROR_CHECK(err_code);

    // Initialize NUS.
    memset(&nus_init, 0, sizeof(nus_init));

    nus_init.data_handler = nus_data_handler;

    err_code = ble_nus_init(&m_nus, &nus_init);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for starting application timers.
 */
static void application_timers_start(void) {
    ret_code_t err_code;

    // Start application timers.
    err_code = app_timer_start(m_displayupdate_timer_id, DISPLAY_UPDATE_TIMER_INTERVAL, NULL);
    APP_ERROR_CHECK(err_code);

}

/**@brief Function for handling the Connection Parameters Module.
 *
 * @details This function will be called for all events in the Connection Parameters Module which
 *          are passed to the application.
 *          @note All this function does is to disconnect. This could have been done by simply
 *                setting the disconnect_on_fail config parameter, but instead we use the event
 *                handler mechanism to demonstrate its use.
 *
 * @param[in] p_evt  Event received from the Connection Parameters Module.
 */
static void on_conn_params_evt(ble_conn_params_evt_t *p_evt) {
    ret_code_t err_code;

    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED) {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}

/**@brief Function for handling a Connection Parameters error.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error) {
    APP_ERROR_HANDLER(nrf_error);
}

/**@brief Function for initializing the Connection Parameters module.
 */
static void conn_params_init(void) {
    ret_code_t err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.disconnect_on_fail = false;
    cp_init.evt_handler = on_conn_params_evt;
    cp_init.error_handler = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for putting the chip into sleep mode.
 *
 * @note This function will not return.
 */
static void sleep_mode_enter(void) {
    ret_code_t err_code;

    err_code = bsp_indication_set(BSP_INDICATE_IDLE);
    APP_ERROR_CHECK(err_code);

    // Prepare wakeup buttons.
    err_code = bsp_btn_ble_sleep_mode_prepare();
    APP_ERROR_CHECK(err_code);

    // Go to system-off mode (this function will not return; wakeup will cause a reset).
    err_code = sd_power_system_off();
    //APP_ERROR_CHECK(err_code);
}

/**@brief Function for handling advertising events.
 *
 * @details This function will be called for advertising events which are passed to the application.
 *
 * @param[in] ble_adv_evt  Advertising event.
 */
static void on_adv_evt(ble_adv_evt_t ble_adv_evt) {
    ret_code_t err_code;

    switch (ble_adv_evt) {
    case BLE_ADV_EVT_FAST:
        NRF_LOG_INFO("Fast advertising.")
        ;
        err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
        APP_ERROR_CHECK(err_code);
        break;

    case BLE_ADV_EVT_IDLE:
        //sleep_mode_enter();
        advertising_start(false);
        break;

    default:
        break;
    }
}

/**@brief Function for handling BLE events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 * @param[in]   p_context   Unused.
 */
static void ble_evt_handler(ble_evt_t const *p_ble_evt, void *p_context) {
    ret_code_t err_code;

    switch (p_ble_evt->header.evt_id) {
    case BLE_GAP_EVT_CONNECTED:
        NRF_LOG_INFO("Connected.")
        ;
        err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
        APP_ERROR_CHECK(err_code);
        m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
        err_code = nrf_ble_qwr_conn_handle_assign(&m_qwr, m_conn_handle);
        APP_ERROR_CHECK(err_code);
        btconnected = true;
        onbtconnect = true;
        break;

    case BLE_GAP_EVT_DISCONNECTED:
        NRF_LOG_INFO("Disconnected, reason %d.", p_ble_evt->evt.gap_evt.params.disconnected.reason)
        ;
        m_conn_handle = BLE_CONN_HANDLE_INVALID;
        btconnected = false;
        onbtdisconnect = true;
        break;

    case BLE_GAP_EVT_PHY_UPDATE_REQUEST: {
        NRF_LOG_DEBUG("PHY update request.");
        ble_gap_phys_t const phys =
        { .rx_phys = BLE_GAP_PHY_AUTO, .tx_phys = BLE_GAP_PHY_AUTO, };
        err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
        APP_ERROR_CHECK(err_code);
    }
    break;

    case BLE_GATTC_EVT_TIMEOUT:
        // Disconnect on GATT Client timeout event.
        NRF_LOG_DEBUG("GATT Client Timeout.")
        ;
        err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                         BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
        APP_ERROR_CHECK(err_code);
        break;

    case BLE_GATTS_EVT_TIMEOUT:
        // Disconnect on GATT Server timeout event.
        NRF_LOG_DEBUG("GATT Server Timeout.")
        ;
        err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                         BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
        APP_ERROR_CHECK(err_code);
        break;

    case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
        NRF_LOG_DEBUG("BLE_GAP_EVT_SEC_PARAMS_REQUEST")
        ;
        break;

    case BLE_GAP_EVT_AUTH_KEY_REQUEST:
        NRF_LOG_INFO("BLE_GAP_EVT_AUTH_KEY_REQUEST")
        ;
        break;

    case BLE_GAP_EVT_LESC_DHKEY_REQUEST:
        NRF_LOG_INFO("BLE_GAP_EVT_LESC_DHKEY_REQUEST")
        ;
        break;

    case BLE_GAP_EVT_AUTH_STATUS:
        NRF_LOG_INFO("BLE_GAP_EVT_AUTH_STATUS: status=0x%x bond=0x%x lv4: %d kdist_own:0x%x kdist_peer:0x%x",
                     p_ble_evt->evt.gap_evt.params.auth_status.auth_status, p_ble_evt->evt.gap_evt.params.auth_status.bonded,
                     p_ble_evt->evt.gap_evt.params.auth_status.sm1_levels.lv4,
                     *((uint8_t* )&p_ble_evt->evt.gap_evt.params.auth_status.kdist_own),
                     *((uint8_t* )&p_ble_evt->evt.gap_evt.params.auth_status.kdist_peer))
        ;
        break;

    default:
        // No implementation needed.
        break;
    }
}

/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void) {
    ret_code_t err_code;

    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

    // Configure the BLE stack using the default settings.
    // Fetch the start address of the application RAM.
    uint32_t ram_start = 0;
    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);

    // Enable BLE stack.
    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);

    // Register a handler for BLE events.
    NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);
}

/**@brief Function for handling events from the BSP module.
 *
 * @param[in]   event   Event generated by button press.
 */
void bsp_event_handler(bsp_event_t event) {
    ret_code_t err_code;

    switch (event) {
    case BSP_EVENT_SLEEP:
        sleep_mode_enter();
        break;

    case BSP_EVENT_DISCONNECT:
        err_code = sd_ble_gap_disconnect(m_conn_handle,
                                         BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
        if (err_code != NRF_ERROR_INVALID_STATE) {
            APP_ERROR_CHECK(err_code);
        }
        break;

    case BSP_EVENT_WHITELIST_OFF:
        if (m_conn_handle == BLE_CONN_HANDLE_INVALID) {
            err_code = ble_advertising_restart_without_whitelist(&m_advertising);
            if (err_code != NRF_ERROR_INVALID_STATE) {
                APP_ERROR_CHECK(err_code);
            }
        }
        break;

    default:
        break;
    }
}

/**@brief Function for the Peer Manager initialization.
 */
static void peer_manager_init(void) {
    ble_gap_sec_params_t sec_param;
    ret_code_t err_code;

    err_code = pm_init();
    APP_ERROR_CHECK(err_code);

    memset(&sec_param, 0, sizeof(ble_gap_sec_params_t));

    // Security parameters to be used for all security procedures.
    sec_param.bond = SEC_PARAM_BOND;
    sec_param.mitm = SEC_PARAM_MITM;
    sec_param.lesc = SEC_PARAM_LESC;
    sec_param.keypress = SEC_PARAM_KEYPRESS;
    sec_param.io_caps = SEC_PARAM_IO_CAPABILITIES;
    sec_param.oob = SEC_PARAM_OOB;
    sec_param.min_key_size = SEC_PARAM_MIN_KEY_SIZE;
    sec_param.max_key_size = SEC_PARAM_MAX_KEY_SIZE;
    sec_param.kdist_own.enc = 1;
    sec_param.kdist_own.id = 1;
    sec_param.kdist_peer.enc = 1;
    sec_param.kdist_peer.id = 1;

    err_code = pm_sec_params_set(&sec_param);
    APP_ERROR_CHECK(err_code);

    err_code = pm_register(pm_evt_handler);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing the Advertising functionality.
 */
static void advertising_init(void) {
    ret_code_t err_code;
    ble_advertising_init_t init;

    memset(&init, 0, sizeof(init));

    init.advdata.name_type = BLE_ADVDATA_FULL_NAME;
    init.advdata.include_appearance = false;
    init.advdata.flags = BLE_GAP_ADV_FLAGS_LE_ONLY_LIMITED_DISC_MODE;
//	init.advdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
//	init.advdata.uuids_complete.p_uuids = m_adv_uuids;

    init.srdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    init.srdata.uuids_complete.p_uuids = m_adv_uuids;

    init.config.ble_adv_fast_enabled = true;
    init.config.ble_adv_fast_interval = APP_ADV_INTERVAL;
    init.config.ble_adv_fast_timeout = APP_ADV_DURATION;

    init.evt_handler = on_adv_evt;

    err_code = ble_advertising_init(&m_advertising, &init);
    APP_ERROR_CHECK(err_code);

    ble_advertising_conn_cfg_tag_set(&m_advertising, APP_BLE_CONN_CFG_TAG);
}

/**@brief Function for initializing buttons and leds.
 *
 * @param[out] p_erase_bonds  Will be true if the clear bonding button was pressed to wake the application up.
 */
static void buttons_leds_init(bool *p_erase_bonds) {
    ret_code_t err_code;
    bsp_event_t startup_event;

    err_code = bsp_init(BSP_INIT_LEDS | BSP_INIT_BUTTONS, bsp_event_handler);
    APP_ERROR_CHECK(err_code);

    err_code = bsp_btn_ble_init(NULL, &startup_event);
    APP_ERROR_CHECK(err_code);

    *p_erase_bonds = (startup_event == BSP_EVENT_CLEAR_BONDING_DATA);
}

/**@brief Function for initializing the nrf log module.
 */
static void log_init(void) {
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}

/**@brief Function for initializing power management.
 */
static void power_management_init(void) {
    ret_code_t err_code;
    err_code = nrf_pwr_mgmt_init();
    APP_ERROR_CHECK(err_code);
}

///**@brief Function for handling the idle state (main loop).
// *
// * @details If there is no pending log operation, then sleep until next the next event occurs.
// */
//static void idle_state_handle(void)
//{
//	ret_code_t err_code;
//
//	err_code = nrf_ble_lesc_request_handler();
//	APP_ERROR_CHECK(err_code);
//
//	if (NRF_LOG_PROCESS() == false)
//	{
//		nrf_pwr_mgmt_run();
//	}
//}

#if USE_TWI
#define TWI_ADDRESSES      127

#define OLED_I2C_PIN_SCL 27
#define OLED_I2C_PIN_SDA 26
#define OLED_ADDR        0x3C
static uint8_t m_sample;
static bool readsomething = false;
#endif

#if USE_SPI

static const nrfx_spim_t m_spi = NRFX_SPIM_INSTANCE(SPI_INSTANCE);

#define NRFX_SPIM_SCK_PIN  20 // yellow wire (CLK)
#define NRFX_SPIM_MOSI_PIN 22 // blue wire (DIN)
#define NRFX_SPIM_CS_PIN 13 // orange wire (CS)
#define NRFX_SPIM_DC_PIN 17 // green wire (DC)
#define NRFX_SPIM_RESET_PIN 15 // white wire (Reset)

void spi_handler(nrfx_spim_evt_t const *p_event, void *p_context);
uint8_t u8x8_HW_com_spi_nrf52832(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8g2_nrf_gpio_and_delay_spi_cb(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
/* Indicates if SPI operation has ended. */
static volatile bool m_xfer_done = false;
#endif

#if USE_TWI
#if TWI0_ENABLED
#define TWI_INSTANCE_ID     0
#elif TWI1_ENABLED
#define TWI_INSTANCE_ID     1
#endif

static const nrf_drv_twi_t m_twi = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID);
#endif

#if USE_TWI
/**
 * @brief TWI events handler.
 */
void twi_handler(nrf_drv_twi_evt_t const * p_event, void * p_context) {
    switch(p_event->type) {
    case NRF_DRV_TWI_EVT_ADDRESS_NACK: {
        // NRF_LOG_ERROR("Got back NACK");
        m_xfer_done = true;
        break;
    }
    case NRF_DRV_TWI_EVT_DATA_NACK: {
        // NRF_LOG_ERROR("Got back D NACK");
        m_xfer_done = true;
        break;
    }
    case NRF_DRV_TWI_EVT_DONE: {
        if (p_event->xfer_desc.type == NRF_DRV_TWI_XFER_RX) {
            readsomething=true;
        }
        m_xfer_done = true;
        break;
    }
    default: {
        m_xfer_done = false;
        break;
    }
    }
}

/**
 * @brief UART initialization.
 */
void twi_init (void) {
    ret_code_t err_code;

    const nrf_drv_twi_config_t twi_oled_config = {
        .scl                = OLED_I2C_PIN_SCL,
        .sda                = OLED_I2C_PIN_SDA,
        .frequency          = NRF_DRV_TWI_FREQ_400K,
        .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
        .clear_bus_init     = false,
    };

    err_code = nrf_drv_twi_init(&m_twi, &twi_oled_config, twi_handler, NULL);
    APP_ERROR_CHECK(err_code);

    nrf_drv_twi_enable(&m_twi);
}

uint8_t u8g2_nrf_gpio_and_delay_twi_cb(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
    switch(msg) {
    case U8X8_MSG_DELAY_MILLI:
        // NRF_LOG_INFO("nrf_delay_ms(%d)", arg_int);
        nrf_delay_ms(arg_int);
        break;

    case U8X8_MSG_DELAY_10MICRO:
        // NRF_LOG_INFO("nrf_delay_us(%d)", 10*arg_int);
        nrf_delay_us(10*arg_int);
        break;

    default:
        u8x8_SetGPIOResult(u8x8, 1); // default return value
        break;
    }
    return 1;
}

uint8_t u8x8_HW_com_twi_nrf52832(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
    uint8_t *data;
    bool res = false;
    ret_code_t err_code;
    static uint8_t buffer[32];
    static uint8_t buf_idx;
    switch(msg) {
    case U8X8_MSG_BYTE_SEND: {
        data = (uint8_t *)arg_ptr;
        while( arg_int > 0 ) {
            buffer[buf_idx++] = *data;
            data++;
            arg_int--;
        }
        break;
    }
    case U8X8_MSG_BYTE_START_TRANSFER: {
        buf_idx = 0;
        m_xfer_done = false;
        break;
    }
    case U8X8_MSG_BYTE_END_TRANSFER: {
        uint8_t addr = u8x8_GetI2CAddress(u8x8);

        err_code = nrf_drv_twi_tx(&m_twi, u8x8_GetI2CAddress(u8x8), buffer, buf_idx, false);
        APP_ERROR_CHECK(err_code);
        while (!m_xfer_done) {
            __WFE();
        }
        break;
    }
    default:
        return 0;
    }
    return 1;
}

#endif

#if USE_SPI
uint8_t u8g2_nrf_gpio_and_delay_spi_cb(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
    switch (msg) {
    case U8X8_MSG_GPIO_DC:
        nrf_gpio_pin_write(NRFX_SPIM_DC_PIN, arg_int);
        break;

    case U8X8_MSG_GPIO_RESET:
        nrf_gpio_pin_write(NRFX_SPIM_RESET_PIN, arg_int);
        break;

    case U8X8_MSG_DELAY_MILLI:
        nrf_delay_ms(arg_int);
        break;

    case U8X8_MSG_DELAY_10MICRO:
        nrf_delay_us(10 * arg_int);
        break;

    default:
        u8x8_SetGPIOResult(u8x8, 1); // default return value
        break;
    }
    return 1;
}

void spi_init(void) {
    ret_code_t err_code;

    nrfx_spim_config_t spi_oled_config = NRFX_SPIM_DEFAULT_CONFIG;

    spi_oled_config.sck_pin = NRFX_SPIM_SCK_PIN;
    spi_oled_config.mosi_pin = NRFX_SPIM_MOSI_PIN;
    spi_oled_config.ss_pin = NRFX_SPIM_CS_PIN;
    spi_oled_config.frequency = NRF_SPIM_FREQ_32M; // The SSD1326 can go up to 10 MHz clock
    spi_oled_config.mode = NRF_SPIM_MODE_0;
    spi_oled_config.ss_active_high = false;

    err_code = nrfx_spim_init(&m_spi, &spi_oled_config, spi_handler, NULL);
    APP_ERROR_CHECK(err_code);

    // Enable the out-of-band GPIOs
    nrf_gpio_cfg_output(NRFX_SPIM_DC_PIN);
    nrf_gpio_cfg_output(NRFX_SPIM_RESET_PIN);
    nrf_gpio_cfg_output(NRFX_SPIM_CS_PIN);

}

void spi_handler(nrfx_spim_evt_t const *p_event, void *p_context) {
    switch (p_event->type) {
    case NRFX_SPIM_EVENT_DONE:
        m_xfer_done = true;
        break;
    }
}

uint8_t u8x8_HW_com_spi_nrf52832(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
    uint8_t *data;
//    bool res = false;
    ret_code_t err_code;
    static uint8_t buffer[128];
    static uint8_t buf_idx = 0;

    switch (msg) {
    case U8X8_MSG_BYTE_SEND: {
        buf_idx = 0;
        data = (uint8_t*) arg_ptr;
        while (arg_int > 0) {
            buffer[buf_idx++] = *data;
            data++;
            arg_int--;
        }

        m_xfer_done = false;
        nrfx_spim_xfer_desc_t spim_xfer_desc = NRFX_SPIM_XFER_TX(&buffer, buf_idx);
        err_code = nrfx_spim_xfer(&m_spi, &spim_xfer_desc, 0);
        APP_ERROR_CHECK(err_code);
        while (!m_xfer_done) {
            __WFE();
        }
        break;
    }
    case U8X8_MSG_BYTE_SET_DC: {
        u8x8_gpio_SetDC(u8x8, arg_int);
        break;
    }
    case U8X8_MSG_BYTE_START_TRANSFER: {
        buf_idx = 0;

        break;
    }
    case U8X8_MSG_BYTE_END_TRANSFER: {
        break;
    }
    default:
        return 0;
    }
    return 1;
}

#endif

void drawLogo(void) {

#ifdef EISENZELT_BUILD
    u8g2_DrawXBM(&u8g2, DISPLAYWIDTH / 2 - 24, DISPLAYWIDTH / 2 - 48, 48, 42, eisenzelt_bits);
#else
    u8g2_DrawXBM(&u8g2, DISPLAYWIDTH / 2 - 48, DISPLAYWIDTH / 2 - 48, 96, 96, navit_plain_bk_96_96_bits);
#endif
}

struct point {
    int x;
    int y;
};

struct point_rect {
    struct point lu;
    struct point rl;
};

#define M_PI		3.14159265358979323846
#define FEET_PER_METER  3.2808399
#define FEET_PER_MILE   5280
#define KILOMETERS_TO_MILES	0.62137119

/* ISO C `broken-down time' structure.  */
struct tm {
    int tm_sec; /* Seconds.	[0-60] (1 leap second) */
    int tm_min; /* Minutes.	[0-59] */
    int tm_hour; /* Hours.	[0-23] */
    int tm_mday; /* Day.		[1-31] */
    int tm_mon; /* Month.	[0-11] */
    int tm_year; /* Year	- 1900.  */
    int tm_wday; /* Day of week.	[0-6] */
    int tm_yday; /* Days in year.[0-365]	*/
    int tm_isdst; /* DST.		[-1/0/1]*/

# ifdef	__USE_MISC
    long int tm_gmtoff;		/* Seconds east of UTC.  */
    const char *tm_zone;		/* Timezone abbreviation.  */
# else
    long int __tm_gmtoff; /* Seconds east of UTC.  */
    const char *__tm_zone; /* Timezone abbreviation.  */
# endif
};

///**
// * * Format time (duration)
// * *
// * * @param tm pointer to a tm structure specifying the time
// * * @param days days
// * * @returns a pointer to a string containing the formatted time
// * */
//static char* format_time(struct tm *tm, int days)
//{
//	if (days)
//		return g_strdup_printf("%d+%02d:%02d", days, tm->tm_hour, tm->tm_min);
//	else
//		return g_strdup_printf("%02d:%02d", tm->tm_hour, tm->tm_min);
//}
//
///**
// * * Format speed in km/h
// * *
// * * @param speed speed in km/h
// * * @param sep separator character to be inserted between speed value and unit
// * * @returns a pointer to a string containing the formatted speed
// * */
//static char* format_speed(double speed, char *sep, char *format, int imperial)
//{
//	char *unit = "km/h";
//	if (imperial)
//	{
//		speed = speed * 1000 * FEET_PER_METER / FEET_PER_MILE;
//		unit = "mph";
//	}
//	if (!format || !strcmp(format, "named"))
//		return g_strdup_printf((speed < 10) ? "%.1f%s%s" : "%.0f%s%s", speed, sep, unit);
//	else if (!strcmp(format, "value") || !strcmp(format, "unit"))
//	{
//		if (!strcmp(format, "value"))
//			return g_strdup_printf((speed < 10) ? "%.1f" : "%.0f", speed);
//		else
//			return g_strdup(unit);
//	}
//	return g_strdup("");
//}
//
//static char* format_float_0(double num)
//{
//	return g_strdup_printf("%.0f", num);
//}

///**
// * @brief Move a group of points in a direction (adding @p dx and @p dy to their x and y coordinates)
// * @param dx The shift to perform to the x axis
// * @param dy The shift to perform to the y axis
// * @param[in,out] p An array of points to move
// * @param count The number of points stored inside @p p
// */
//static void transform_move(int dx, int dy, struct point *p, int count)
//{
//	int i;
//	for (i = 0; i < count; i++)
//	{
//		p->x += dx;
//		p->y += dy;
//		p++;
//	}
//}

/**
 * @brief Rotate a group of points around a @p center
 * @param center The coordinates of the center of the rotation to apply
 * @param angle The angle of the rotation
 * @param[in,out] p An array of points to rotate
 * @param count The number of points stored inside @p p
 */
void transform_rotate(struct point *center, int angle, struct point *p, int count) {
    int i, x, y;
    double dx, dy;
    for (i = 0; i < count; i++) {
        dx = sin(M_PI * angle / 180.0);
        dy = cos(M_PI * angle / 180.0);
        x = dy * p->x - dx * p->y;
        y = dx * p->x + dy * p->y;

        p->x = center->x + x;
        p->y = center->y + y;
        p++;
    }
}

/**
 * @brief Draw an arrow of length @p r, centered at point @p p, with color @p gc, pointing to direction @p dir
 *
 * @param u8g2 The u8g2 instance on which to draw
 * @param p The center of the compass
 * @param r The radius of the compass (around the center point @p p)
 * @param dir The direction the arrow points to (0 being up, value is in degrees counter-clockwise)
 */
static void draw_handle(u8g2_t *u8g2, struct point *p, int r, int dir) {
    struct point ph[6];
    double l = r * 0.4;
    //double s = l * 0.4;

    ph[0].x = 0; /* Compute details for the body of the arrow */
    ph[0].y = r - 8;
    ph[1].x = 0;
    ph[1].y = -r + 8;
    transform_rotate(p, dir, ph, 2); /* Rotate to the correct direction */
    u8g2_DrawLine(u8g2, ph[0].x, ph[0].y, ph[1].x, ph[1].y); /* Draw the body */

    ph[0].x = -l / 2; /* Compute details for the head of the arrow */
    ph[0].y = -r + 4 + l / 2;
    ph[1].x = 0;
    ph[1].y = -r + 4;
    ph[2].x = l / 2;
    ph[2].y = -r + 4 + l / 2;
    transform_rotate(p, dir, ph, 3); /* Rotate to the correct direction */
    /* Draw the head */
//	u8g2_DrawLine(u8g2, ph[0].x, ph[0].y, ph[1].x, ph[1].y);
//	u8g2_DrawLine(u8g2, ph[1].x, ph[1].y, ph[2].x, ph[2].y);
    u8g2_DrawTriangle(u8g2, ph[0].x, ph[0].y, ph[1].x, ph[1].y, ph[2].x, ph[2].y);

//	ph[0].x = -s; /* Compute details for the tail of the arrow */
//	ph[0].y = r - l + s;
//	ph[1].x = 0;
//	ph[1].y = r - l;
//	ph[2].x = s;
//	ph[2].y = r - l + s;
//	ph[3] = ph[0]; /* Save these 3 points for future re-use */
//	ph[4] = ph[1];
//	ph[5] = ph[2];
//	transform_rotate(p, dir, ph, 3); /* Rotate to the correct direction */
//	/* Draw the tail */
//	u8g2_DrawLine(u8g2, ph[0].x, ph[0].y, ph[1].x, ph[1].y);
//	u8g2_DrawLine(u8g2, ph[1].x, ph[1].y, ph[2].x, ph[2].y);
//	ph[0] = ph[3]; /* Restore saved points */
//	ph[1] = ph[4];
//	ph[2] = ph[5];
//	transform_move(0, s, ph, 3);
//	ph[3] = ph[0]; /* Save these 3 points for future re-use */
//	ph[4] = ph[1];
//	ph[5] = ph[2];
//	transform_rotate(p, dir, ph, 3); /* Rotate to the correct direction */
//	u8g2_DrawLine(u8g2, ph[0].x, ph[0].y, ph[1].x, ph[1].y);
//	u8g2_DrawLine(u8g2, ph[1].x, ph[1].y, ph[2].x, ph[2].y);
//	ph[0] = ph[3]; /* Restore saved points */
//	ph[1] = ph[4];
//	ph[2] = ph[5];
//	transform_move(0, s, ph, 3);
//	transform_rotate(p, dir, ph, 3); /* Rotate to the correct direction */
//	/* Draw the tail */
//	u8g2_DrawLine(u8g2, ph[0].x, ph[0].y, ph[1].x, ph[1].y);
//	u8g2_DrawLine(u8g2, ph[1].x, ph[1].y, ph[2].x, ph[2].y);
}

/**
 * @brief Draw a compass handle of length @p r, centered at point @p p, pointing to direction @p dir
 *
 * @param gr The graphics instance on which to draw
 * @param gc_n The color to use for the north half of the compass
 * @param gc_s The color to use for the south half of the compass
 * @param p The center of the compass
 * @param r The radius of the compass (around the center point @p p)
 * @param dir The direction the compass points to (0 being up, value is in degrees counter-clockwise)
 */
static void draw_compass(u8g2_t *u8g2, struct point *p, int r, int dir) {
    struct point ph[3];

    int l = r * 0.25;

    ph[0].x = -l;
    ph[0].y = 0;
    ph[1].x = 0;
    ph[1].y = -r;
    ph[2].x = l;
    ph[2].y = 0;
    transform_rotate(p, dir, ph, 3); /* Rotate to the correct direction */

    u8g2_DrawTriangle(u8g2, ph[0].x, ph[0].y, ph[1].x, ph[1].y, ph[2].x, ph[2].y);

    ph[0].x = -l;
    ph[0].y = 0;
    ph[1].x = 0;
    ph[1].y = r;
    ph[2].x = l;
    ph[2].y = 0;
    transform_rotate(p, dir, ph, 3); /* Rotate to the correct direction */

    u8g2_DrawLine(u8g2, ph[0].x, ph[0].y, ph[1].x, ph[1].y);
    u8g2_DrawLine(u8g2, ph[1].x, ph[1].y, ph[2].x, ph[2].y);
    u8g2_DrawLine(u8g2, ph[0].x, ph[0].y, ph[2].x, ph[2].y);

}

/**
 * @brief Draw a strength of GNSS signal
 * @param u8g2 pointer to u8g2 instance
 * @param x x-position
 * @param y y-position
 * @param satcnt count of satellites used
 */
static void draw_gnss_sats_used(u8g2_t *u8g2, int x, int y, int satcnt) {
//	while (satcnt)
//	{
//		u8g2_DrawBox(u8g2, x, y, 1, satcnt);
//		satcnt--;
//	}

    u8g2_SetFont(u8g2, u8g2_font_siji_t_6x10);
    char *strength;
    switch (satcnt) {
    case 0:
    case 1:
    case 2:
        strength = "\ue21f";
        break;
    case 3:
        strength = "\ue220";
        break;
    case 4:
        strength = "\ue221";
        break;
    case 5:
        strength = "\ue222";
        break;
    default:
        strength = "\ue223";
    }

    u8g2_DrawUTF8(u8g2, x, y, strength);

}

//TODO: activate for truck build
//static void draw_flags(u8g2_t *u8g2)
//{
////	int strwidth = u8g2_GetStrWidth(u8g2, "NO_LEZ");
//	u8g2_DrawStr(u8g2, DISPLAYWIDTH / 6 * 4 - 4, DISPLAYWIDTH / 6 - 1, "ADR");
////	strwidth = u8g2_GetStrWidth(u8g2, "ADR");
//	u8g2_DrawStr(u8g2, DISPLAYWIDTH / 6 * 4 - 4, DISPLAYWIDTH / 6 + 4 + 1, "NOLEZs");
//}

static void draw_navnext(u8g2_t *u8g2, int x, int y, const char *text) {
    u8g2_SetFont(u8g2, u8g2_font_4x6_tf);
    int strwidth = u8g2_GetStrWidth(u8g2, text);
    u8g2_DrawUTF8(u8g2, x - strwidth / 2, y, text);
}

static void draw_navcurrent(u8g2_t *u8g2, int x, int y, const char *text) {
    u8g2_SetFont(u8g2, u8g2_font_4x6_tf);
    int strwidth = u8g2_GetStrWidth(u8g2, text);
    u8g2_DrawUTF8(u8g2, x - strwidth / 2, y, text);
}

static void draw_navcoord(u8g2_t *u8g2, int lonx, int lony, int latx, int laty, char *lon, char *lat) {
    int strwidth = u8g2_GetStrWidth(u8g2, lat);
    u8g2_DrawStr(u8g2, latx - strwidth / 2, laty, lat);
    strwidth = u8g2_GetStrWidth(u8g2, lon);
    u8g2_DrawStr(u8g2, lonx - strwidth / 2, lony, lon);
}

static void draw_btconn(u8g2_t *u8g2, int x, int y) {
    if (btconnected != 0) {
        u8g2_DrawLine(u8g2, x + 2, y, x + 2, y + 8);
        u8g2_DrawLine(u8g2, x + 2, y, x + 4, y + 2);
        u8g2_DrawLine(u8g2, x + 4, y + 2, x, y + 6);
        u8g2_DrawLine(u8g2, x, y + 2, x + 4, y + 6);
        u8g2_DrawLine(u8g2, x + 4, y + 6, x + 2, y + 8);
    }
}

void dimon(void) {
    int contrast;
    for (contrast = 100; contrast < 200; contrast = contrast + 1) {
        u8g2_SetContrast(&u8g2, contrast);
        nrf_delay_ms(5);
    }
}

//Dim to off
void dimoff(void) {
    int contrast;
    for (contrast = 205; contrast > 100; contrast = contrast - 1) {
        u8g2_SetContrast(&u8g2, contrast);
        nrf_delay_ms(5);
    }
}

void display_update(void) {

    int strwidth;

    if (btconnected != 0) {
        if (onbtconnect) {
            //dimoff();
            first = 1;
            onbtconnect = false;
        }

        u8g2_ClearBuffer(&u8g2);
        //u8g2_DrawCircle(&u8g2, DISPLAYWIDTH / 2, DISPLAYWIDTH / 2, DISPLAYWIDTH / 2 - 4, U8G2_DRAW_ALL);

        // Next turn icon
        p.x = DISPLAYWIDTH / 4 - 32;
        p.y = DISPLAYWIDTH / 2 - 23;

        if (navimage != NULL) {
            u8g2_DrawXBM(&u8g2, p.x, p.y, 48, 48, (const uint8_t*) navimage);
        }

        // street name after next turn
        p.x = DISPLAYWIDTH / 2;
        p.y = 39;
        draw_navnext(&u8g2, p.x, p.y, nextstreetname);

        // Current street name
        p.y = DISPLAYWIDTH / 2 + cr + 5;
        p.x = DISPLAYWIDTH / 2;
        draw_navcurrent(&u8g2, p.x, p.y, currentstreetname);

        // Position
        p.x = DISPLAYWIDTH / 2;
        p.y = 82;
        draw_navcoord(&u8g2, p.x, p.y, p.x, p.y + 6, navcoordlats, navcoordlons);

        // Allowed Speed
        if (strcmp(routespeeds, "") && strcmp(routespeeds, "-1")) {
            u8g2_SetFont(&u8g2, u8g2_font_6x10_mf);
            p.x = DISPLAYWIDTH / 2;
            strwidth = u8g2_GetStrWidth(&u8g2, routespeeds);
            u8g2_DrawXBM(&u8g2, p.x - 14 + 1, (DISPLAYWIDTH / 2) - cr - 16 - 18, 28, 28, speedsign_bits);
            u8g2_DrawStr(&u8g2, p.x - strwidth / 2 + 1, (DISPLAYWIDTH / 2) - cr - 17, routespeeds);
        }

        // Conditional Speed
        if (strcmp(condspeeds, "")) {
            u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
            p.x = DISPLAYWIDTH / 16 * 7;
            strwidth = u8g2_GetStrWidth(&u8g2, "120");

            u8g2_DrawXBM(&u8g2, p.x - 18, (DISPLAYWIDTH / 2) + cr + 7, 19, 19, condspeed_bits);
            u8g2_DrawStr(&u8g2, p.x - strwidth / 2 - 9, (DISPLAYWIDTH / 2) + cr + 19, "120");

            u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
            u8g2_DrawUTF8(&u8g2, p.x + 5, (DISPLAYWIDTH / 2) + cr + 12, "Bei Nässe   ");
            u8g2_DrawUTF8(&u8g2, p.x + 5, (DISPLAYWIDTH / 2) + cr + 19, "Bedingung   ");
            u8g2_DrawUTF8(&u8g2, p.x + 5, (DISPLAYWIDTH / 2) + cr + 26, "22h-6h      ");
        }

        // distance to next turn
        u8g2_SetFont(&u8g2, u8g2_font_12x6LED_tf);
        strwidth = u8g2_GetStrWidth(&u8g2, nmls);
        p.x = DISPLAYWIDTH / 4 - strwidth / 2 - 4;
        u8g2_SetDrawColor(&u8g2, 0);
        u8g2_DrawBox(&u8g2, DISPLAYWIDTH / 4 - strwidth / 2 - 5, (DISPLAYWIDTH / 2) + cr - 1 - 13, strwidth + 2, 13);
        u8g2_SetDrawColor(&u8g2, 1);
        u8g2_DrawStr(&u8g2, p.x, (DISPLAYWIDTH / 2) + cr - 1, nmls);

        // Flags TODO: enable for truck build
        //		u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
        //		draw_flags(&u8g2);

        //ETA
        u8g2_SetFont(&u8g2, u8g2_font_5x7_mf);
        strwidth = u8g2_GetStrWidth(&u8g2, etas);
        u8g2_DrawStr(&u8g2, DISPLAYWIDTH / 2 - strwidth / 2, DISPLAYWIDTH / 2 - r - 4, etas);

        //Speed
        u8g2_SetFont(&u8g2, u8g2_font_helvB18_tf);
        strwidth = u8g2_GetStrWidth(&u8g2, vehiclespeeds);
        u8g2_DrawStr(&u8g2, DISPLAYWIDTH / 2 - strwidth / 2, DISPLAYWIDTH / 2 + 3, vehiclespeeds);
        u8g2_SetFont(&u8g2, u8g2_font_6x10_mf);

        //Dist to destination
        strwidth = u8g2_GetStrWidth(&u8g2, dlengths);
        u8g2_DrawStr(&u8g2, DISPLAYWIDTH / 2 - strwidth / 2, DISPLAYWIDTH / 2 + r - 2, dlengths);

        //center point of compass
        p.x = DISPLAYWIDTH / 4 * 3 + 4;
        p.y = DISPLAYWIDTH / 2;

        //Direct Distance to Destination
        u8g2_SetFont(&u8g2, u8g2_font_5x7_mf);
        strwidth = u8g2_GetStrWidth(&u8g2, distances);
        u8g2_DrawStr(&u8g2, p.x - strwidth / 2, p.y - cr / 2 - 5, distances);

        //GPS Height
        if (strcmp(gpsheights, "-1m")) {
            u8g2_SetFont(&u8g2, u8g2_font_5x7_mf);
            strwidth = u8g2_GetStrWidth(&u8g2, gpsheights);
            u8g2_DrawStr(&u8g2, p.x - strwidth / 2, p.y + cr / 2 + 10, gpsheights);
        }

        //Bluetooth conn
        draw_btconn(&u8g2, 79, 9);

        // Danger TODO: enable with speedcam etc.
        //		u8g2_SetFont(&u8g2, u8g2_font_unifont_t_77);
        //		u8g2_DrawUTF8(&u8g2, 26, 31, "\u26a0");
        //		u8g2_SetFont(&u8g2, u8g2_font_4x6_tf);
        //		u8g2_SetDrawColor(&u8g2, 0);
        //		u8g2_DrawBox(&u8g2, 43, 32 - 6, 186, 8);
        //		u8g2_SetDrawColor(&u8g2, 1);
        //		u8g2_DrawUTF8(&u8g2, 43, 32, "Check Speed!");

        // GNSS sats used
        draw_gnss_sats_used(&u8g2, 40, 17, satcnt);

        //Compass
        u8g2_DrawCircle(&u8g2, p.x, p.y, cr / 2, U8G2_DRAW_ALL);
        draw_handle(&u8g2, &p, cr - 3, hdir);
        draw_compass(&u8g2, &p, r, dir);
        // Draw buffer
        u8g2_SendBuffer(&u8g2);
    } else {
        if (onbtdisconnect) {
            if (!first)
                dimoff();
            //reset global variables
            dir = 0;
            hdir = 180;
            satcnt = 0;
            navcoordlon = 0.0;
            navcoordlat = 0.0;
            navcoordlons[0] = 0;
            navcoordlats[0] = 0;
            distances[0] = 0;
            distancedests[0] = 0;
            etas[0] = 0;
            times[0] = 0;
            dlengths[0] = 0;
            nmls[0] = 0;
            routespeeds[0] = 0;
            condspeeds[0] = 0;
            vehiclespeeds[0] = 0;
            currentstreetname[0] = 0;
            nextstreetname[0] = 0;
            directlengths[0] = 0;
            gpsheights[0] = 0;
            u8g2_ClearBuffer(&u8g2);
            drawLogo();
            u8g2_SetFont(&u8g2, u8g2_font_12x6LED_tf);
            int strwidth = u8g2_GetStrWidth(&u8g2, "NO BT CONN");
            u8g2_SetDrawColor(&u8g2, 0);
            u8g2_DrawBox(&u8g2, DISPLAYWIDTH / 2 - strwidth / 2 - 1, DISPLAYWIDTH / 2 - 13 + 28, strwidth + 2, 14);
            u8g2_SetDrawColor(&u8g2, 1);
            u8g2_DrawStr(&u8g2, DISPLAYWIDTH / 2 - strwidth / 2, DISPLAYWIDTH / 2 + 28, "NO BT CONN");
            u8g2_SendBuffer(&u8g2);
            if (!first)
                dimon();
            onbtdisconnect = false;
        }

    }

    // Dim to on in first cycle
    if (first == 1) {
        dimon();
        first = 0;
        receive_data = true;
    }

}

#define __ASM __asm /*!< asm keyword for GNU Compiler */
#define __INLINE inline /*!< inline keyword for GNU Compiler */
#define __STATIC_INLINE static inline

/**
 \brief Get Link Register
 \details Returns the current value of the Link Register (LR).
 \return LR Register value
 */
__attribute__( ( always_inline ))                     __STATIC_INLINE uint32_t __get_LR(void) {
    register uint32_t result;

    __ASM volatile ("MOV %0, LR\n" : "=r" (result) );
    return (result);
}

void HardFault_Handler(void) {
    uint32_t *sp = (uint32_t*) __get_MSP(); // Get stack pointer
    uint32_t ia = sp[12]; // Get instruction address from stack
    uint32_t *psp = (uint32_t*) __get_PSP();

    NRF_LOG_ERROR("Hard Fault at address: 0x%08x  SP: 0x%08x PSP: %0x08x   LR: %0x08x\r\n", (unsigned int )ia,
                  (unsigned int )sp, (unsigned int ) psp,
                  __get_LR());

    NRF_LOG_FLUSH();

    __disable_irq();

    while (1)
        ;
}

void display_init(void) {
#if USE_TWI
    twi_init();

    readsomething=false;
    for (address = 0x0; address <= TWI_ADDRESSES; address++) {
        if (detected_device) break;
        m_xfer_done = false;
        err_code = nrf_drv_twi_rx(&m_twi, address, &m_sample, sizeof(m_sample));
        while (!m_xfer_done) {
            __WFE();
        }
        if (readsomething) {
            detected_device = true;
            NRF_LOG_INFO("TWI device detected at address 0x%x: Read value 0x%02x", address, m_sample);
        }
        NRF_LOG_FLUSH();
    }

    if (!detected_device) {
        NRF_LOG_INFO("No device was found.");
        NRF_LOG_FLUSH();
        while (true) {

        }
    }


    u8g2_Setup_sh1106_i2c_128x64_noname_f(&u8g2, U8G2_R0, u8x8_HW_com_twi_nrf52832, u8g2_nrf_gpio_and_delay_twi_cb);
    u8g2_SetI2CAddress(&u8g2, OLED_ADDR);
#elif USE_SPI
    spi_init();
    u8g2_Setup_st7571_128x128_f(&u8g2, U8G2_R0, u8x8_HW_com_spi_nrf52832, u8g2_nrf_gpio_and_delay_spi_cb);
#endif

    //TODO: remove for production
    u8g2_SetDisplayRotation(&u8g2, U8G2_R2);

    u8g2_InitDisplay(&u8g2);
    u8g2_ClearDisplay(&u8g2);
    u8g2_ClearBuffer(&u8g2);
    u8g2_SetContrast(&u8g2, 0);
    u8g2_SetPowerSave(&u8g2, 0);
    drawLogo();
    u8g2_SetFont(&u8g2, u8g2_font_12x6LED_tf);
    int strwidth = u8g2_GetStrWidth(&u8g2, "BT-HEADUP V1.0");
    u8g2_SetDrawColor(&u8g2, 0);
    u8g2_DrawBox(&u8g2, DISPLAYWIDTH / 2 - strwidth / 2 - 1, DISPLAYWIDTH / 2 - 13 + 28, strwidth + 2, 14);
    u8g2_SetDrawColor(&u8g2, 1);
    u8g2_DrawStr(&u8g2, DISPLAYWIDTH / 2 - strwidth / 2, DISPLAYWIDTH / 2 + 28, "BT-HEADUP V1.0");
    u8g2_SendBuffer(&u8g2);

    dimon();
    nrf_delay_ms(1000);
    dimoff();
}

char* item_to_name(enum item_type item) {
    int i;

    for (i = 0; i < sizeof(item_names) / sizeof(struct item_name); i++) {
        if (item_names[i].item == item)
            return item_names[i].name;
    }
    return NULL;
}

void processcmd(void) {
    char *token;
    int cmdidx;
    int *pint;
    char *pchar;
    double *cnv;
    double dbuffer = 0.0;

    for (cmdidx = 0; cmdidx < CMDBUFFERSIZE; cmdidx++) {
        if (cmds[cmdidx].used == 1) {

            token = (char*) &cmds[cmdidx].data;

            // check CRC
            uint16_t crc = crc16_compute((uint8_t*) token, token[1] - 2, NULL);
            uint16_t rcrc = (token[token[1] - 2] << 8) + token[token[1] - 1];
//		    NRF_LOG_DEBUG("Computed CRC: %04x - Received CRC: %04x", crc, rcrc);

            if (crc == rcrc) {
                switch (token[0]) {
                case BTHEADUP_DIRECTION:

                    memcpy(&dbuffer, &token[2], sizeof(double));

                    cnv = (double*) &dbuffer;
                    dir = *cnv;

                    NRF_LOG_DEBUG("BTHEADUP_DIRECTION: " NRF_LOG_FLOAT_MARKER, NRF_LOG_FLOAT(dir))
                    ;

                    break;

                case BTHEADUP_HANDLEDIRECTION:

                    memcpy(&dbuffer, &token[2], sizeof(double));

                    cnv = (double*) &dbuffer;
                    hdir = *cnv;

                    NRF_LOG_DEBUG("BTHEADUP_HANDLEDIRECTION: "NRF_LOG_FLOAT_MARKER, NRF_LOG_FLOAT(hdir))
                    ;

                    break;

                case BTHEADUP_DISTANCE:

                    pchar = (char*) &token[2];
                    strncpy(distances, pchar, sizeof(distances));

                    NRF_LOG_DEBUG("BTHEADUP_DISTANCE: %s", distances)
                    ;

                    break;

                case BTHEADUP_COORDINATESGEO:

                    memcpy(&dbuffer, &token[2], sizeof(double));

                    cnv = (double*) &dbuffer;
                    navcoordlat = *cnv;
                    snprintf(navcoordlats, sizeof(navcoordlats), navcoordlat >= 0 ? "N%.5f" : "S%.5f",
                             navcoordlat < 0 ? navcoordlat * -1 : navcoordlat);

                    memcpy(&dbuffer, &token[2 + sizeof(double)], sizeof(double));

                    cnv = (double*) &dbuffer;
                    navcoordlon = *cnv;
                    snprintf(navcoordlons, sizeof(navcoordlons), navcoordlon >= 0 ? "E%.5f" : "W%.5f",
                             navcoordlon < 0 ? navcoordlon * -1 : navcoordlon);

                    NRF_LOG_DEBUG("BTHEADUP_COORDINATESGEO: %s %s", navcoordlats, navcoordlons)
                    ;

                    break;

                case BTHEADUP_DESTTIME:

                    pchar = (char*) &token[2];
                    strncpy(etas, pchar, sizeof(etas));

                    NRF_LOG_DEBUG("BTHEADUP_DESTTIME: %s", etas)
                    ;

                    break;

                case BTHEADUP_DESTLENGTH:

                    pchar = (char*) &token[2];
                    strncpy(dlengths, pchar, sizeof(dlengths));

                    NRF_LOG_DEBUG("BTHEADUP_DESTLENGTH %s", dlengths)
                    ;

                    break;

                case BTHEADUP_NEXTMANEUVLENGTH:
                    pchar = (char*) &token[2];
                    strncpy(nmls, pchar, sizeof(nmls));

                    NRF_LOG_DEBUG("BTHEADUP_NEXTMANEUVLENGTH: %s", nmls)
                    ;

                    break;

                case BTHEADUP_ROUTESPEED:

                    memcpy(&dbuffer, &token[2], sizeof(double));

                    cnv = (double*) &dbuffer;
                    snprintf(routespeeds, sizeof(routespeeds), "%.0f", *cnv);

                    NRF_LOG_DEBUG("BTHEADUP_ROUTESPEED: %s", routespeeds)
                    ;

                    break;

                case BTHEADUP_VEHICLESPEED:

                    memcpy(&dbuffer, &token[2], sizeof(double));

                    cnv = (double*) &dbuffer;
                    snprintf(vehiclespeeds, sizeof(vehiclespeeds), "%.0f", *cnv);

                    NRF_LOG_DEBUG("BTHEADUP_VEHICLESPEED: %s", vehiclespeeds)
                    ;

                    break;

                case BTHEADUP_GPSSIGNALSTRENGTH:

                    pint = (int*) &token[2];
                    satcnt = *pint;

                    NRF_LOG_DEBUG("BTHEADUP_GPSSIGNALSTRENGTH %i", satcnt)
                    ;
                    ;

                    break;

                case BTHEADUP_GPSHEIGHT:

                    memcpy(&dbuffer, &token[2], sizeof(double));

                    cnv = (double*) &dbuffer;
                    snprintf(gpsheights, sizeof(gpsheights), "%.0fm", *cnv);

                    NRF_LOG_DEBUG("BTHEADUP_GPSHEIGHT: %s", gpsheights)
                    ;

                    break;

                case BTHEADUP_STREETNAME:

                    pchar = (char*) &token[2];
                    strncpy(currentstreetname, pchar, sizeof(currentstreetname));

                    NRF_LOG_DEBUG("BTHEADUP_STREETNAME %s", currentstreetname)
                    ;
                    ;

                    break;

                case BTHEADUP_NEXTSTREETNAME:

                    pchar = (char*) &token[2];
                    strncpy(nextstreetname, pchar, sizeof(nextstreetname));

                    NRF_LOG_DEBUG("BTHEADUP_NEXTSTREETNAME %s", nextstreetname)
                    ;
                    ;

                    break;

                case BTHEADUP_NAVNEXTTURNIMAGE:

                    pint = (int*) &token[2];

                    navimage = item_to_name(*pint);

                    NRF_LOG_DEBUG("BTHEADUP_NAVNEXTTURNIMAGE: %i", *pint)
                    ;

                    break;

                case BTHEADUP_GPSHDOP:
                case BTHEADUP_NAVIMAGENAME:

                    // ignore

                    break;

                default:
                    NRF_LOG_ERROR("Unknown Command: %02x", token[0])
                    ;
                    break;
                }

            } else {
                NRF_LOG_ERROR("CRC failure");
            }

            cmds[cmdidx].used = 0;

        }
        cmdreceived = false;
    }
}

/**
 * @brief Function for main application entry.
 */
int main(void) {
    bool erase_bonds;

    // Initialize.
    log_init();
    ble_version_t ble_vers;
    sd_ble_version_get(&ble_vers);
    timers_init();
    buttons_leds_init(&erase_bonds);
    power_management_init();
    ble_stack_init();
    gap_params_init();
    gatt_init();
    services_init(); // before advertising init when using a custom UUID like the NUS
    advertising_init();
    conn_params_init();
    peer_manager_init();

    // Start execution.
    NRF_LOG_INFO("BT-Headup started.");
    application_timers_start();
    advertising_start(true);

    display_init();
    onbtdisconnect = true;

    app_timer_start(m_displayupdate_timer_id, DISPLAY_UPDATE_TIMER_INTERVAL, NULL);

    receive_data = true;

    while (true) {

        if (cmdreceived) {
            processcmd();
        }

        if (update_display) {
            display_update();
            update_display = false;
        }

        //TODO: set to 100ms for production
        //nrf_delay_ms(100);
        //idle_state_handle();
    }
}

