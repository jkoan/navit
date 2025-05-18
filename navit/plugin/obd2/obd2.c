/**
 * Navit, a modular navigation system.
 * Copyright (C) 2005-2014 Navit Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 */

#include <stdio.h>
#include <glib.h>
#include <fcntl.h>
#include "debug.h"
#include "headup.h"
#include <unistd.h>
#include <sys/socket.h>
#include "config.h"
#include "navit.h"
#include <navit/item.h>
#include <navit/main.h>
#include <navit/debug.h>
#include <navit/callback.h>
#include <navit/plugin.h>
#include <navit/event.h>
#include <navit/config_.h>
#include <navit/navit_nls.h>
#include <sys/time.h>
#include "vehicle.h"
#include "coord.h"

#if !defined(__APPLE__)
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/l2cap.h>
#include <bluetooth/sdp.h>
#include <lib/uuid.h>
#include <attrib/att.h>
#include <attrib/gattrib.h>
#include <attrib/gatt.h>
#include <src/shared/crypto.h>
#include <src/shared/util.h>
#include <btio/btio.h>
#include <attrib/gatttool.h>

// OBD BLE definitions
#define OBD2_SERVICE_UUID      "0000fff0-0000-1000-8000-00805f9b34fb"
#define OBD2_CHARACTERISTIC_RX "0000fff1-0000-1000-8000-00805f9b34fb"
#define OBD2_CHARACTERISTIC_TX "0000fff2-0000-1000-8000-00805f9b34fb"

#else //!defined(__APPLE__)
#include <stdlib.h>
#include "objc/objc.h"
#include "obd2.h"
#endif //!defined(__APPLE__)


int saved_dd = 0, saved_sock = 0;
static uint16_t obdrx_handle = 0;
#if !defined(__APPLE__)
static uint16_t obdtx_handle = 0;
GAttrib *attrib;
#endif
char recv_buf[200];

struct headup_priv {
    struct callback_list *cbl;
    struct navit *nav;
    int device;
    char *deviceserial;
    char *deviceaddress;
    double last_time;
    char message[255];  // log messages for file
    char *filename;     // log filename
    FILE *fp;           // log file fp
    struct event_timeout *idle;
    struct callback *callback;
    struct event_timeout *cmdtimeout;
    struct headup_methods *methods;

    char senddata[30];
    int cmdcode;
    double voltage;
    double speed;
    double coolant;
    double oil;

    bool responserec;
    bool cmdsend;

    int log;
};

void obd2_findHostDevice(struct headup_priv*);
void obd2_init_ble(struct headup_priv*);
void obd2_destroy(struct headup_priv*);
void obd2_connected(void*);
void obd2_disconnected(void*);
void obd2_recv_cb(void*, const char*);
static int obd2_get_attr(struct headup_priv *priv, enum attr_type type, struct attr *attr);

// BLE commands
enum obd2_cmds {
    OBD2_PROTTYPE = 0x10, OBD2_VOLTAGE, OBD2_SPEED, OBD2_COOLANTTEMP, OBD2_OILTEMP, OBD2_RESET
};

void obd2_send(struct headup_priv *this, enum obd2_cmds cmd, void *data);

static int obd2_set_attr(struct headup_priv *this, struct attr *attr) {
    if (attr->type == attr_deviceserial) {
        this->deviceserial = attr->u.str;
    }
    if (attr->type == attr_deviceaddress) {
        this->deviceaddress = attr->u.str;
    }
    if(attr->type == attr_obd2_log) {
        this->log = (int)attr->u.num;
    }
    return 1;
}

#if !defined(__APPLE__)

static void char_read_cb(guint8 status, const guint8 *pdu, guint16 plen, gpointer user_data) {
    uint8_t value[plen];
    ssize_t vlen;
    int i;
    struct headup_priv *this = user_data;

    event_remove_timeout(this->cmdtimeout);
    this->responserec = true;
    this->cmdtimeout = NULL;
    this->cmdsend = false;


    if (status != 0) {
        dbg(lvl_error, "Characteristic value/descriptor read failed: %s\n", att_ecode2str(status));
        return;
    }

    vlen = dec_read_resp(pdu, plen, value, sizeof(value));
    if (vlen < 0) {
        dbg(lvl_error, "Protocol error\n");
        return;
    }

    if (!strncmp(value, this->senddata, strlen(this->senddata))) {
        char *data = &value[strlen(this->senddata)];
        dbg(lvl_error, "%s ", data);

        switch (this->cmdcode) {
        case OBD2_VOLTAGE:
            this->voltage = strtod(data, NULL);
            break;
        case OBD2_SPEED:
            dbg(lvl_debug, "Command %i COMPLETED", this->cmdcode);
            break;
        case OBD2_COOLANTTEMP:
            dbg(lvl_debug, "Command %i COMPLETED", this->cmdcode);
            break;
        case OBD2_OILTEMP:
            dbg(lvl_debug, "Command %i COMPLETED", this->cmdcode);
            break;

        default:
            dbg(lvl_error, "Command can't be handled.");
            break;

        }

    } else {
        if(!strcmp(value, ">\r\n"))
            return;
        dbg(lvl_error, "Command %i failed", this->cmdcode);
    }

}

void cmd_timeout(struct headup_priv *this) {
    this->cmdtimeout = NULL;
    this->responserec = true;
    this->cmdsend = false;
    dbg(lvl_error, "Command %i timed out", this->cmdcode);
}

#endif

// Send data via BLE with ELM327 protocol
void obd2_send(struct headup_priv *this, enum obd2_cmds cmd, void *data) {
#pragma unused (data)

    dbg(lvl_debug, "obd2_send");

    unsigned long x = 0;
    memset(this->senddata, 0, sizeof(this->senddata));

    if (obdrx_handle > 0) {

        switch (cmd) {

        case OBD2_RESET:
            strncpy(this->senddata, "atz", sizeof(this->senddata));
            x = strlen(this->senddata);
            this->cmdcode = cmd;
            break;

        case OBD2_PROTTYPE:
            strncpy(this->senddata, "at sp 0", sizeof(this->senddata));
            x = strlen(this->senddata);
            this->cmdcode = cmd;
            break;

        case OBD2_VOLTAGE:
            strncpy(this->senddata, "at rv", sizeof(this->senddata));
            x = strlen(this->senddata);
            this->cmdcode = cmd;
            break;

        case OBD2_SPEED:  //010D
            this->senddata[0] = 0x30;
            this->senddata[1] = 0x31;
            this->senddata[2] = 0x30;
            this->senddata[3] = 0x44;
            x = 4;
            this->cmdcode = cmd;
            break;

        case OBD2_COOLANTTEMP: //0105
            this->senddata[0] = 0x30;
            this->senddata[1] = 0x31;
            this->senddata[2] = 0x30;
            this->senddata[3] = 0x35;
            x = 4;
            this->cmdcode = cmd;
            break;

        case OBD2_OILTEMP: //015C
            this->senddata[0] = 0x30;
            this->senddata[1] = 0x31;
            this->senddata[2] = 0x35;
            this->senddata[3] = 0x43;
            x = 4;
            this->cmdcode = cmd;
            break;

        default:
            dbg(lvl_debug, "UNKONWN COMMAND CODE: %i", cmd)
            ;
            return;
        }

#if !defined(__APPLE__)
        GIOChannel *channel = g_attrib_get_channel(attrib);
        int fd = g_io_channel_unix_get_fd(channel);
        int status = fcntl(fd, F_GETFD);

        if (status < 0) {
            obdrx_handle = 0;
            obdtx_handle = 0;
            event_remove_timeout(this->idle);
            this->callback = callback_new_1(callback_cast(obd2_findHostDevice), this);
            this->idle = event_add_timeout(1000, 1, this->callback);
            return;
        }

        this->senddata[x++] = 0x0D;
        this->senddata[x++] = 0x0A;

        this->responserec = false;
        this->cmdsend = true;

        gatt_write_cmd(attrib, obdtx_handle, this->senddata, x, NULL, NULL);

        for (int i = 0; i < x; i++) {
            dbg(lvl_debug, "Data: %02x\n", this->senddata[i]);
        }

        struct callback *cmdtocb = callback_new_1(callback_cast(cmd_timeout), this);
        this->cmdtimeout = event_add_timeout(5000, 0, cmdtocb);

        struct timespec tim, tim2;
        tim.tv_sec = 0;
        tim.tv_nsec = 50000L;
        nanosleep(&tim, &tim2);

        gatt_read_char(attrib, obdrx_handle, char_read_cb, this);

#else
        this->senddata[x++] = 0x0D;
        this->senddata[x++] = 0x0A;

        this->responserec = false;
        this->cmdsend = true;

        obd2btcontroller_send(this->senddata, x);

#endif
    } else {
        dbg(lvl_debug, "No OBD handle yet.");
    }

    return;
}

/**
 * @brief   Function called when navit is idle. Does the continuous reading
 * @param   this the obd2 struct containing the state of the plugin
 *
 * @return  nothing
 *
 * This is the main function of this plugin. It is called when navit is idle,
 * collects all data needed from navit and feeds the display via BLE.
 *
 */
static void obd2_idle(struct headup_priv *this) {

    if(this->cmdsend == true)
        return;

    if(this->cmdcode == 0) {
        obd2_send(this, OBD2_RESET, NULL);
        dbg(lvl_debug, "Sent OBD2_RESET");
        return;
    }

    if(this->cmdcode == OBD2_RESET) {
        obd2_send(this, OBD2_PROTTYPE, NULL);
        dbg(lvl_debug, "Sent OBD2_PROTTYPE");
        return;
    }

    if(this->cmdcode==OBD2_PROTTYPE || this->cmdcode==OBD2_VOLTAGE) {
        obd2_send(this, OBD2_SPEED, NULL);
        dbg(lvl_debug, "Sent OBD2_SPEED");
        return;
    }

    if(this->cmdcode==OBD2_SPEED) {
        obd2_send(this, OBD2_COOLANTTEMP, NULL);
        dbg(lvl_debug, "Sent OBD2_COOLANTTEMP");
        return;
    }

    if(this->cmdcode==OBD2_COOLANTTEMP) {
        obd2_send(this, OBD2_OILTEMP, NULL);
        dbg(lvl_debug, "Sent OBD2_OILTEMP");
        return;
    }

    if(this->cmdcode == OBD2_OILTEMP) {
        obd2_send(this, OBD2_VOLTAGE, NULL);
        dbg(lvl_debug, "Sent OBD2_VOLTAGE");
        return;
    }

}

//static gboolean listen_start(gpointer user_data)
//{
//    GAttrib *attrib = user_data;
//
//    g_attrib_register(attrib, ATT_OP_HANDLE_NOTIFY, GATTRIB_ALL_HANDLES,
//                        events_handler, attrib, NULL);
//    g_attrib_register(attrib, ATT_OP_HANDLE_IND, GATTRIB_ALL_HANDLES,
//                        events_handler, attrib, NULL);
//
//    return FALSE;
//}

/**
 * @brief       Initialize the obd2 plugin
 * @param[in]   obd2    - the obd2 struct containing the state of the plugin
 *              nav     - the navit object
 *
 * @return      nothing
 *
 * Initialize the obd2 plugin
 *
 */
static void obd2_init(struct headup_priv *this) {
    obd2_init_ble(this);
}

#if !defined(__APPLE__)

static void char_discovered_cb(uint8_t status, GSList *characteristics, void *user_data) {
    GSList *l;

    if (status) {
        g_printerr("Discover all characteristics failed: %s\n", att_ecode2str(status));
        goto done;
    }

    for (l = characteristics; l; l = l->next) {
        struct gatt_char *chars = l->data;

        dbg(lvl_error, "Found OBD2 device with OBD service:\nhandle = 0x%04x, char properties = 0x%02x, char value "
            "handle = 0x%04x, uuid = %s\n", chars->handle, chars->properties, chars->value_handle, chars->uuid);

        if (!strcmp(chars->uuid, OBD2_CHARACTERISTIC_RX)) {
            obdrx_handle = chars->value_handle;
            dbg(lvl_error, "RXHANDLE: %i", obdrx_handle);
//            g_idle_add(listen_start, attrib);
        }

        if (!strcmp(chars->uuid, OBD2_CHARACTERISTIC_TX)) {
            obdtx_handle = chars->value_handle;
            dbg(lvl_error, "TXHANDLE: %i", obdtx_handle);
        }

    }

done:
    return;
}

static void connect_cb(GIOChannel *io, GError *err, gpointer user_data) {
    uint16_t mtu;
    uint16_t cid;
    GError *gerr = NULL;

    if (err) {
        g_printerr("%s\n", err->message);
    }

    bt_io_get(io, &gerr, BT_IO_OPT_IMTU, &mtu, BT_IO_OPT_CID, &cid, BT_IO_OPT_INVALID);

    if (gerr) {
        g_printerr("Can't detect MTU, using default: %s", gerr->message);
        g_error_free(gerr);
        mtu = ATT_DEFAULT_LE_MTU;
    }

    if (cid == ATT_CID)
        mtu = ATT_DEFAULT_LE_MTU;

    attrib = g_attrib_new(io, mtu, false);

    bt_uuid_t uuidrx, uuidtx;
    bt_string_to_uuid(&uuidrx, OBD2_CHARACTERISTIC_RX);
    bt_string_to_uuid(&uuidtx, OBD2_CHARACTERISTIC_TX);

    dbg(lvl_debug, "Discover Characteristics");
    gatt_discover_char(attrib, 0x0001, 0xFFFF, &uuidrx, char_discovered_cb, NULL);
    gatt_discover_char(attrib, 0x0001, 0xFFFF, &uuidtx, char_discovered_cb, NULL);
}

void obd2_findHostDevice(struct headup_priv *this) {
    int dev_id, sock;
    char addr[19] = { 0 };  // 7F:C1:D3:37:66:31 + \0

    dbg(lvl_error, "RXHANDLE: %i", obdrx_handle);
    dbg(lvl_error, "TXHANDLE: %i", obdtx_handle);

    if (obdrx_handle > 0) {
        event_remove_timeout(this->idle);
        this->callback = callback_new_1(callback_cast(obd2_idle), this);
        this->idle = event_add_timeout(700, 1, this->callback);
        return;
    }

    dev_id = hci_get_route(NULL);   // Get device id
    sock = hci_open_dev(dev_id);    // Open socket for device

    if (dev_id < 0 || sock < 0) {
        dbg(lvl_debug, "error opening socket - device: %s - socket: %s ", strerror(dev_id), strerror(sock));
        return;
    }

    bdaddr_t src_addr;

// get host Bluetooth address
    if (hci_devba(dev_id, &src_addr) < 0) {
        dbg(lvl_error, "Can't get hci%d info: %s (%d)\n", sock, strerror(errno), errno);
        return;
    } else {
        ba2str(&src_addr, addr);
        dbg(lvl_debug, "Found host device with address %s\n", addr);

    }

    char *opt_dst_type = g_strdup("public");
    char *opt_sec_level = g_strdup("low");
    char srcaddress[19];
    ba2str(&src_addr, srcaddress);
    int opt_psm = 0;
    int opt_mtu = 257;
    GError *gerr = NULL;

    GIOChannel *chan = gatt_connect(srcaddress, this->deviceaddress, opt_dst_type, opt_sec_level, opt_psm, opt_mtu,
                                    connect_cb, &gerr);

    if (chan) {

        dbg(lvl_debug, "GATT connected. Change MTU.");

        return;
    } else {
        dbg(lvl_error, "GATT connect: GIOChannel is NULL");
    }

    return;
}

#else

void obd2_recv_cb(void *this, const char *value) {
    struct headup_priv* _this = (struct headup_priv*) this;

    // TIMEOUT
    if(value == NULL) {
        _this->cmdsend = false;
        recv_buf[0]=0x00;
        return;
    }

    struct timeval tv;
    gettimeofday(&tv, NULL);
    unsigned long long millisecondsSinceEpoch =
        (unsigned long long)(tv.tv_sec) * 1000 +
        (unsigned long long)(tv.tv_usec) / 1000;

    struct attr vehicle, position;

    FILE *fp;

    if(_this->log) {


        char dir[200];
        strncpy(dir, getenv("NAVIT_USER_DATADIR"), sizeof(dir));
        strcat(dir, "/obd2.log");
        fp = fopen(dir,"a");
        fprintf(fp, "%llu: Command: %s\n", millisecondsSinceEpoch, _this->senddata);

        for (unsigned long i = 0; i<strlen(value); i++) {
            fprintf(fp, "%llu: %02x -> %c\n", millisecondsSinceEpoch, value[i], value[i]);
        }

        if (navit_get_attr(_this->nav, attr_vehicle, &vehicle, NULL) && vehicle.u.vehicle) {
            if(vehicle_get_attr(vehicle.u.vehicle, attr_position_coord_geo, &position, NULL)) {

                fprintf(fp, "%llu: %f, %f\n", millisecondsSinceEpoch, position.u.coord_geo->lat, position.u.coord_geo->lng);
            }
        }

        fclose(fp);
    }

    strncat(recv_buf, value, sizeof(recv_buf) - 1 - strlen(recv_buf));

    if(strstr(recv_buf, "SEARCHING...")) {
        recv_buf[0]=0x00;
        return;
    }

    switch (_this->cmdcode) {

    case OBD2_PROTTYPE:
    case OBD2_RESET:
        break;

    case OBD2_VOLTAGE:
        if (!strncmp(recv_buf, _this->senddata, strlen(_this->senddata))) {
            char* data = (char*)&recv_buf[strlen(_this->senddata)];
            _this->voltage = strtod(data, NULL);
            if(_this->log)
                fprintf(fp, "%llu: OBD2_VOLTAGE - %li\n", millisecondsSinceEpoch, strtol(&recv_buf[12], NULL, 16), NULL);
        }
        break;

    case OBD2_SPEED:
        if(!strncmp(recv_buf, "010D\r\n41 0D ", 12)) {
            _this->speed = strtol(&recv_buf[12], NULL, 16);  // 0-255
            if(_this->log)
                fprintf(fp, "%llu: OBD2_SPEED - %li\n", millisecondsSinceEpoch, strtol(&recv_buf[12], NULL, 16), NULL);
        } else {
            dbg(lvl_debug, "Command %i PID NOT FOUND IN RESPONSE", _this->cmdcode);
        }
        dbg(lvl_debug, "Command %i COMPLETED", _this->cmdcode);
        break;

    case OBD2_COOLANTTEMP:
        if(!strncmp(recv_buf, "0105\r\n41 05 ", 12)) {
            _this->coolant = strtol(&recv_buf[12], NULL, 16) - 40;  // A - 40
            if(_this->log)
                fprintf(fp, "%llu: OBD2_COOLANTTEMP - %li\n", millisecondsSinceEpoch, strtol(&recv_buf[12], NULL, 16), NULL);
        } else {
            dbg(lvl_debug, "Command %i PID NOT FOUND IN RESPONSE", _this->cmdcode);
        }
        dbg(lvl_debug, "Command %i COMPLETED", _this->cmdcode);
        break;

    case OBD2_OILTEMP:
        if(!strncmp(recv_buf, "015C\r\n41 5C ", 12)) {
            _this->oil = strtol(&recv_buf[12], NULL, 16) - 40;  // A - 40
            if(_this->log)
                fprintf(fp, "%llu: OBD2_OILTEMP - %li\n", millisecondsSinceEpoch, strtol(&recv_buf[12], NULL, 16), NULL);
        } else {
            dbg(lvl_debug, "Command %i PID NOT FOUND IN RESPONSE", _this->cmdcode);
        }
        dbg(lvl_debug, "Command %i COMPLETED", _this->cmdcode);
        break;

    default:
        dbg(lvl_error, "Command can't be handled.");
        _this->cmdsend = false;
        recv_buf[0]=0x00;
        break;

    }

    if(strstr(recv_buf, "\r\n>") || strstr(recv_buf, ">\r\n")) {
        _this->cmdsend = false;
        recv_buf[0]=0x00;
    }

    if(_this->log)
        fclose(fp);

}

void obd2_findHostDevice(struct headup_priv *this) {

    if (obdrx_handle > 0) {
        event_remove_timeout(this->idle);
        this->callback = callback_new_1(callback_cast(obd2_idle), this);
        this->idle = event_add_timeout(700, 1, this->callback);
        return;
    }

    obd2btcontroller_init(this, obd2_disconnected, obd2_connected, obd2_recv_cb, this->deviceserial);
}

void obd2_connected(void* _this) {
    struct headup_priv * this = (struct headup_priv *) _this;
    this->cmdsend = false;
    this->cmdcode = OBD2_RESET;
    this->oil = -255;
    this->speed = -255;
    this->coolant = -255;
    this->voltage = -255;
    obdrx_handle = 1;
    callback_list_call_attr_0(this->cbl, attr_obd_connected);
    navit_say(this->nav, _("OBD Interface connected"));
}

void obd2_disconnected(void* _this) {
    struct headup_priv * this = (struct headup_priv *) _this;
    if(obdrx_handle)
        navit_say(this->nav, _("OBD Interface disconnected"));
    obdrx_handle = 0;
    callback_list_call_attr_0(this->cbl, attr_obd_connected);
    this->oil = -255;
    this->speed = -255;
    this->coolant = -255;
    this->voltage = -255;
    this->cmdsend = false;
}

#endif

/**
 * @brief   Opens the serial port and saves state to the obd2 object
 * @param[in]   obd2 - the obd2 struct containing the state of the plugin
 *
 * @return  nothing
 *
 * Opens the serial port and saves state to the obd2 object
 *
 */
void obd2_init_ble(struct headup_priv *this) {
// Check for BT host device and display every 10 seconds
    this->callback = callback_new_1(callback_cast(obd2_findHostDevice), this);
    this->idle = event_add_timeout(1000, 1, this->callback);
    return;

}

#if !defined(__APPLE__)
void obd2_destroy(struct headup_priv *ptr) {
// stop any active scan here
    if (saved_dd) {
        hci_le_set_scan_enable(saved_dd, 0, 0, 1000); // disable in case already enabled
        hci_close_dev(saved_dd);
    }
    if (saved_sock)
        close(saved_sock);
}
#else

void obd2_destroy(struct headup_priv* ptr) {
#pragma unused (ptr)
}

#endif

static int obd2_get_attr(struct headup_priv *priv, enum attr_type type, struct attr *attr) {

    if(type==attr_speed) {
        attr->type = type;
        attr->u.numd = &priv->speed;
        return 1;
    }

    if(type==attr_voltage) {
        attr->type = type;
        attr->u.numd = &priv->voltage;
        return 1;
    }

    if(type==attr_coolant) {
        attr->type = type;
        attr->u.numd = &priv->coolant;
        return 1;
    }

    if(type==attr_oil) {
        attr->type = type;
        attr->u.numd = &priv->oil;
        return 1;
    }

    if(type==attr_obd_connected) {
        attr->type = type;
        attr->u.num = obdrx_handle;
        return 1;
    }

    return 0;
}

struct headup_methods obd2_methods = { obd2_destroy, obd2_set_attr, obd2_get_attr,};



/**
 * @brief   Creates the obd2 plugin and set some default properties
 * @param[in]   nav - the navit object
 *              meth    - the osd_methods
 *      attrs   - pointer to the attributes
 *
 * @return  nothing
 *
 * Creates the obd2 plugin and set some default properties
 *
 */
static struct headup_priv* obd2_new(struct navit *nav, struct headup_methods *meth, struct callback_list *cbl,
                                    struct attr **attrs) {

//return NULL;
    struct headup_priv *ret;

    dbg(lvl_debug, "enter");
    ret = g_new0(struct headup_priv, 1);
    ret->nav = nav;
    ret->cbl = cbl;
    ret->log = 0;
    ret->responserec=true;
    ret->cmdsend = false;
    ret->speed = 0;
    ret->coolant = 0;
    ret->oil = 0;
    ret->callback = callback_new_1(callback_cast(obd2_init), ret);
    *meth = obd2_methods;
    while (attrs && *attrs)
        obd2_set_attr(ret, *attrs++);
    navit_add_callback(nav, callback_new_attr_1(callback_cast(obd2_init), attr_graphics_ready, ret));
    return ret;
}

/**
 * @brief   The plugin entry point
 *
 * @return  nothing
 *
 * The plugin entry point
 *
 */
void plugin_init(void) {
    plugin_register_category_headup("obd2", obd2_new);
    dbg(lvl_debug, "plugin_init completed");
}
