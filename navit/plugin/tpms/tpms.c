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
 *
 * Supported TPMS sensors:
 * https://www.aliexpress.com/item/1005006123969088.html   with SNP736 chipset
 *
 * Info:
 * https://github.com/OLFDB/TPMS_BLE_BR forked from https://github.com/omadon/TPMS_BLE_BR
 * Battery: CR1632
 *
 * Config sample for navit.xml:
 *
 * <headup type="tpms" name="BR" frontleftaddr="71BB1555" frontrightaddr="F654D555" rearleftaddr="E0F2C555" rearrightaddr="CD05F555" pressure_max_thd_fl="500"  pressure_max_thd_fr="3500"  pressure_max_thd_rl="3500"  pressure_max_thd_rr="3500" pressure_min_thd_fl="2300" pressure_min_thd_fr="2300" pressure_min_thd_rl="2300" pressure_min_thd_rr="2300" temp_max_thd="50" />
 *
 * To get the address of the sensors use SYTPMS app on iOS or Android mount only one sensor and drive to find out which one has which address.
 *
 * https://www.aliexpress.com/item/32815317757.html
 * Battery: CR1632
 * Battery can't be exchanged easily as sealed with non-flexible plastic.
 * Bad quality.
 *
 * Info:
 * https://www.eisenzelt.de/ez/wordpress/?p=3424
 *
 * Config sample for navit.xml:
 *
 * <headup type="tpms" name="TPMS" frontleftaddr="TPMS1_118155" frontrightaddr="TPMS2_218A55" rearleftaddr="TPMS3_317355" rearrightaddr="TPMS4_418055" pressure_max_thd_fl="3500"  pressure_max_thd_fr="3500"  pressure_max_thd_rl="3500"  pressure_max_thd_rr="3500" pressure_min_thd_fl="2300" pressure_min_thd_fr="2300" pressure_min_thd_rl="2300" pressure_min_thd_rr="2300" temp_max_thd="50" />
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

#else //!defined(__APPLE__)
#include <stdlib.h>
#include "objc/objc.h"
#include "tpms.h"
#endif //!defined(__APPLE__)

#if !defined(__APPLE__)
static uint16_t tpmstx_handle = 0;
GAttrib *attrib;
#endif

#define TPMS_RECV_TIMEOUT 300000

struct headup_priv {
    struct callback_list *cbl;
    struct navit *nav;
    int device;
    char *name;
    char *deviceserial;
    char *deviceaddress;
    char *frontleftaddr;
    char *frontrightaddr;
    char *rearleftaddr;
    char *rearrightaddr;
    double last_time;
    char message[255];  // log messages for file
    char *filename;     // log filename
    FILE *fp;           // log file fp
    struct callback *callback;
    struct event_timeout *fltimeout;
    struct event_timeout *frtimeout;
    struct event_timeout *rltimeout;
    struct event_timeout *rrtimeout;
    struct headup_methods *methods;
    
    char senddata[30];
    int cmdcode;
    
    int temp_max_thd;
    
    uint8_t alarm;
    
    double fltemp;
    double flpressure;
    double flbatt;
    uint8_t flalarm;
    double pressure_max_thd_fl;
    double pressure_min_thd_fl;
    
    double frtemp;
    double frpressure;
    double frbatt;
    uint8_t fralarm;
    double pressure_max_thd_fr;
    double pressure_min_thd_fr;
    
    double rltemp;
    double rlpressure;
    double rlbatt;
    uint8_t rlalarm;
    double pressure_max_thd_rl;
    double pressure_min_thd_rl;
    
    double rrtemp;
    double rrpressure;
    double rrbatt;
    uint8_t rralarm;
    double pressure_max_thd_rr;
    double pressure_min_thd_rr;
    
    bool responserec;
    bool cmdsend;
    bool connected;
    bool connected_priv;
    
    int log;
};

void tpms_init_ble(struct headup_priv*);
void tpms_destroy(struct headup_priv*);
void tpms_timeout(void *this, char* address);
void tpms_recv_cb(void*, const char*, const unsigned char*, const char*);
static int tpms_get_attr(struct headup_priv *priv, enum attr_type type, struct attr *attr);
static int tpms_set_attr(struct headup_priv *this, struct attr *attr);

struct headup_methods tpms_methods = { tpms_destroy, tpms_set_attr, tpms_get_attr,};

static int tpms_set_attr(struct headup_priv *this, struct attr *attr) {
    
    if(attr->type == attr_name) {
        dbg(lvl_error, "name %s", attr->u.str);
        this->name = g_strup(attr->u.str);
    }
    
    if(attr->type == attr_frontleftaddr) {
        dbg(lvl_error, "address %s", attr->u.str);
        this->frontleftaddr = g_strup(attr->u.str);
    }
    
    if(attr->type == attr_frontrightaddr) {
        dbg(lvl_error, "address %s", attr->u.str);
        this->frontrightaddr = g_strup(attr->u.str);
    }
    
    if(attr->type == attr_rearleftaddr) {
        dbg(lvl_error, "address %s", attr->u.str);
        this->rearleftaddr = g_strup(attr->u.str);
    }
    
    if(attr->type == attr_rearrightaddr) {
        dbg(lvl_error, "address %s", attr->u.str);
        this->rearrightaddr = g_strup(attr->u.str);
    }
    
    if(attr->type == attr_pressure_max_thd_fl) {
        dbg(lvl_error, "pressure_max_thd_fl %li", attr->u.num);
        this->pressure_max_thd_fl = attr->u.num / 1000.0;
    }
    
    if(attr->type == attr_pressure_max_thd_fr) {
        dbg(lvl_error, "pressure_max_thd_fr %li", attr->u.num);
        this->pressure_max_thd_fr = attr->u.num / 1000.0;
    }
    
    if(attr->type == attr_pressure_max_thd_rl) {
        dbg(lvl_error, "pressure_max_thd_rl %li", attr->u.num);
        this->pressure_max_thd_rl = attr->u.num / 1000.0;
    }
    
    if(attr->type == attr_pressure_max_thd_rr) {
        dbg(lvl_error, "pressure_max_thd_rr %li", attr->u.num);
        this->pressure_max_thd_rr = attr->u.num / 1000.0;
    }
    
    if(attr->type == attr_pressure_min_thd_fl) {
        dbg(lvl_error, "pressure_min_thd_fl %li", attr->u.num);
        this->pressure_min_thd_fl = attr->u.num / 1000.0;
    }
    
    if(attr->type == attr_pressure_min_thd_fr) {
        dbg(lvl_error, "pressure_min_thd_fr %li", attr->u.num);
        this->pressure_min_thd_fr = attr->u.num / 1000.0;
    }
    
    if(attr->type == attr_pressure_min_thd_rl) {
        dbg(lvl_error, "pressure_min_thd_rl %li", attr->u.num);
        this->pressure_min_thd_rl = attr->u.num / 1000.0;
    }
    
    if(attr->type == attr_pressure_min_thd_rr) {
        dbg(lvl_error, "pressure_min_thd_rr %li", attr->u.num);
        this->pressure_min_thd_rr = attr->u.num / 1000.0;
    }
    
    if(attr->type == attr_temp_max_thd) {
        dbg(lvl_error, "temp_max_thd %li", attr->u.num);
        this->temp_max_thd = (int)attr->u.num;
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
            case tpms_VOLTAGE:
                this->voltage = strtod(data, NULL);
                break;
            case tpms_SPEED:
                dbg(lvl_debug, "Command %i COMPLETED", this->cmdcode);
                break;
            case tpms_COOLANTTEMP:
                dbg(lvl_debug, "Command %i COMPLETED", this->cmdcode);
                break;
            case tpms_OILTEMP:
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


/**
 * @brief   Function called when navit is idle. Does the continuous reading
 * @param   this the tpms struct containing the state of the plugin
 *
 * @return  nothing
 *
 * This is the main function of this plugin. It is called when navit is idle,
 * collects all data needed from navit and feeds the display via BLE.
 *
 */
//static void tpms_idle(struct headup_priv *this) {
//
//
//}

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
 * @brief       Initialize the tpms plugin
 * @param[in]   tpms    - the tpms struct containing the state of the plugin
 *              nav     - the navit object
 *
 * @return      nothing
 *
 * Initialize the tpms plugin
 *
 */
static void tpms_init(struct headup_priv *this) {
    tpms_init_ble(this);
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
        
        dbg(lvl_error, "Found tpms device with tpms service:\nhandle = 0x%04x, char properties = 0x%02x, char value "
            "handle = 0x%04x, uuid = %s\n", chars->handle, chars->properties, chars->value_handle, chars->uuid);
        
        if (!strcmp(chars->uuid, tpms_CHARACTERISTIC_RX)) {
            tpmsrx_handle = chars->value_handle;
            dbg(lvl_error, "RXHANDLE: %i", tpmsrx_handle);
            //            g_idle_add(listen_start, attrib);
        }
        
        if (!strcmp(chars->uuid, tpms_CHARACTERISTIC_TX)) {
            tpmstx_handle = chars->value_handle;
            dbg(lvl_error, "TXHANDLE: %i", tpmstx_handle);
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
    bt_string_to_uuid(&uuidrx, tpms_CHARACTERISTIC_RX);
    bt_string_to_uuid(&uuidtx, tpms_CHARACTERISTIC_TX);
    
    dbg(lvl_debug, "Discover Characteristics");
    gatt_discover_char(attrib, 0x0001, 0xFFFF, &uuidrx, char_discovered_cb, NULL);
    gatt_discover_char(attrib, 0x0001, 0xFFFF, &uuidtx, char_discovered_cb, NULL);
}

void tpms_findHostDevice(struct headup_priv *this) {
    int dev_id, sock;
    char addr[19] = { 0 };  // 7F:C1:D3:37:66:31 + \0
    
    dbg(lvl_error, "RXHANDLE: %i", tpmsrx_handle);
    dbg(lvl_error, "TXHANDLE: %i", tpmstx_handle);
    
    if (tpmsrx_handle > 0) {
        event_remove_timeout(this->idle);
        this->callback = callback_new_1(callback_cast(tpms_idle), this);
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

void tpms_timeout(void *this, char* address) {
    // Handle timeout of interval between receive of sensor values
    // OR the alarm status of the specific sensor with 2 (will show its value as yellow in osd)
    
    if(this==NULL || address==NULL)
        return;
    
    dbg(lvl_error, "TPMS timeout for sensor with address: %s", address);
    struct headup_priv* _this = (struct headup_priv*) this;
    
    if(address == _this->frontleftaddr) {
        _this->flalarm = _this->flalarm | 2;
        _this->fltimeout = 0;
    }
    
    if(address == _this->frontrightaddr){
        _this->fralarm = _this->fralarm | 2;
        _this->frtimeout = 0;
    }
    if(address == _this->rearleftaddr){
        _this->rlalarm = _this->rlalarm | 2;
        _this->rltimeout = 0;
    }
    
    if(address == _this->rearrightaddr){
        _this->rralarm = _this->rralarm | 2;
        _this->rrtimeout = 0;
    }
    
}

void tpms_recv_cb(void *this, const char *name, const unsigned char *bytes, const char *type) {
    // name is the name of the sensor
    // bytes are the data from a sensor
    // type is the type of tpms ("BR" or "TPMS")
    // Compare name with addrresses, decode data and store it
    
    if(this==NULL || name==NULL || bytes==NULL || type==NULL)
        return;
    
    struct headup_priv* _this = (struct headup_priv*) this;
    
    dbg(lvl_error, "Name: %s", name);
    
    
    
    double pressure, temp;
    int batt, alarm;
    uint8_t status;
    //bool alarm_zero_press, rotating, stop15min, startrot, decpressbelow20700, risingpress, decpressabove20700, unknown;
    
    if(!strcmp(type, "BR")) {
        alarm=0;
        name+=28;
        status = (bytes[0]);
        dbg(lvl_error, "Alarm: %u", status);
        batt=(bytes[1]/10.0);
        dbg(lvl_error, "Batterie: %u%%", (int)((batt - 2.1f) / 0.009 )); // according to datasheet of SNP736 min operating voltage is 2.1 V, Vbatt = 3V -> 0.009 for 100%
        temp=(bytes[2]);
        dbg(lvl_error, "Reifentemperatur: %2.0f°C", temp);
        pressure=((bytes[3]*256+bytes[4])/10-14.5)/14.5; // https://github.com/OLFDB/TPMS_BLE_BR/blob/main/images/TPMS1.jpeg forked from https://github.com/omadon/TPMS_BLE_BR
        dbg(lvl_error, "Reifendruck: %2.1fbar", pressure);
        
    } else {
        status=0;
        pressure=(bytes[10]*65535+bytes[9]*256+bytes[8])/100000.0;
        dbg(lvl_error, "Reifendruck: %2.2fbar", pressure);
        dbg(lvl_error, "Reifendruck: %d", bytes[10]*65535+bytes[9]*256+bytes[8]);
        temp=(bytes[13]*256+bytes[12])/100.0;
        dbg(lvl_error, "Reifentemperatur: %2.2f°C", temp);
        batt=(bytes[16]);
        dbg(lvl_error, "Batterie: %2i%%", batt);
        alarm=(bytes[17]>0);
        dbg(lvl_error, "Alarm: %u", alarm);
    }
    
    if(_this->frontleftaddr==0 || _this->frontrightaddr==0 || _this->rearleftaddr==0 || _this->rearrightaddr==0)
        return;
    
    
    
    if(!strcmp(name, _this->frontleftaddr)) {
        _this->connected = true;
        _this->flpressure = pressure;
        _this->fltemp = temp;
        _this->flbatt = batt;
        _this->flalarm = alarm || (status & 0x88) ;
        if(_this->flalarm  || _this->flpressure < _this->pressure_min_thd_fl || _this->flpressure > _this->pressure_max_thd_fl || _this->fltemp > _this->temp_max_thd) {
            _this->alarm=1;
            _this->flalarm |= 1;
        } else
            _this->alarm=0;
        
        if(_this->fltimeout) {
            event_remove_timeout(_this->fltimeout);
            _this->fltimeout=0;
        }
        struct callback *cb = callback_new_2(callback_cast(tpms_timeout), _this, _this->frontleftaddr);
        _this->fltimeout = event_add_timeout(TPMS_RECV_TIMEOUT, 0, cb);
    }
    
    if(!strcmp(name, _this->frontrightaddr)) {
        _this->connected = true;
        _this->frpressure = pressure;
        _this->frtemp = temp;
        _this->frbatt = batt;
        _this->fralarm = alarm || (status & 0x88) ;
        if(_this->fralarm  || _this->frpressure < _this->pressure_min_thd_fr || _this->frpressure > _this->pressure_max_thd_fr || _this->frtemp > _this->temp_max_thd) {
            _this->alarm=1;
            _this->fralarm |= 1;
        } else
            _this->alarm=0;
        
        if(_this->frtimeout) {
            event_remove_timeout(_this->frtimeout);
            _this->frtimeout=0;
        }
        struct callback *cb = callback_new_2(callback_cast(tpms_timeout), _this, _this->frontrightaddr);
        _this->frtimeout = event_add_timeout(TPMS_RECV_TIMEOUT, 0, cb);
    }
    
    if(!strcmp(name, _this->rearleftaddr)) {
        _this->connected = true;
        _this->rlpressure = pressure;
        _this->rltemp = temp;
        _this->rlbatt = batt;
        _this->rlalarm = alarm || (status & 0x88) ;
        if(_this->rlalarm  || _this->rlpressure < _this->pressure_min_thd_rl || _this->rlpressure > _this->pressure_max_thd_rl || _this->rltemp > _this->temp_max_thd) {
            _this->alarm=1;
            _this->rlalarm |= 1;
        } else
            _this->alarm=0;
        
        if(_this->rltimeout) {
            event_remove_timeout(_this->rltimeout);
            _this->rltimeout=0;
        }
        struct callback *cb = callback_new_2(callback_cast(tpms_timeout), _this, _this->rearleftaddr);
        _this->rltimeout = event_add_timeout(TPMS_RECV_TIMEOUT, 0, cb);
    }
    
    if(!strcmp(name, _this->rearrightaddr)) {
        _this->connected = true;
        _this->rrpressure = pressure;
        _this->rrtemp = temp;
        _this->rrbatt = batt;
        _this->rralarm = alarm || (status & 0x88) ;
        if(_this->rralarm  || _this->rrpressure < _this->pressure_min_thd_rr || _this->rrpressure > _this->pressure_max_thd_rr || _this->rrtemp > _this->temp_max_thd) {
            _this->alarm=1;
            _this->rralarm |= 1;
        } else
            _this->alarm=0;
        
        if(_this->rrtimeout) {
            event_remove_timeout(_this->rrtimeout);
            _this->rrtimeout=0;
        }
        struct callback *cb = callback_new_2(callback_cast(tpms_timeout), _this, _this->rearrightaddr);
        _this->rrtimeout = event_add_timeout(TPMS_RECV_TIMEOUT, 0, cb);
    }
    
    if(_this->connected_priv != _this->connected) {
        callback_list_call_attr_0(_this->cbl, attr_tpms_connected);
        _this->connected_priv = _this->connected;
    }
    
}

#endif

/**
 * @brief   Initialize the tpms object
 * @param[in]   this - the tpms struct containing the state of the plugin
 *
 * @return  nothing
 *
 * Initializes the controller
 *
 */
void tpms_init_ble(struct headup_priv *this) {
    tpmsbtcontroller_init(this, tpms_recv_cb, this->frontleftaddr, this->frontrightaddr, this->rearleftaddr, this->rearrightaddr, this->name);
    return;
}

#if !defined(__APPLE__)
void tpms_destroy(struct headup_priv *ptr) {
    // stop any active scan here
    if (saved_dd) {
        hci_le_set_scan_enable(saved_dd, 0, 0, 1000); // disable in case already enabled
        hci_close_dev(saved_dd);
    }
    if (saved_sock)
        close(saved_sock);
}
#else

void tpms_destroy(struct headup_priv* ptr) {
#pragma unused (ptr)
}

#endif

/**
 * @brief   Returns an attribute of the tpms object
 * @param[in]   priv - the headup_priv object
 *              type    - the attr_type
 *              attrs   - pointer to the attributes
 *
 * @return  1 of attribute found else 0
 *
 *
 *
 */
static int tpms_get_attr(struct headup_priv *priv, enum attr_type type, struct attr *attr) {
    
    if(type==attr_pressure_fl) {
        attr->type = type;
        attr->u.numd = &priv->flpressure;
        return 1;
    }
    
    if(type==attr_temp_fl) {
        attr->type = type;
        attr->u.numd = &priv->fltemp;
        return 1;
    }
    
    if(type==attr_batt_fl) {
        attr->type = type;
        attr->u.numd = &priv->flbatt;
        return 1;
    }
    
    if(type==attr_alarm_fl) {
        attr->type = type;
        attr->u.num = priv->flalarm;
        return 1;
    }
    
    if(type==attr_pressure_fr) {
        attr->type = type;
        attr->u.numd = &priv->frpressure;
        return 1;
    }
    
    if(type==attr_temp_fr) {
        attr->type = type;
        attr->u.numd = &priv->frtemp;
        return 1;
    }
    
    if(type==attr_batt_fr) {
        attr->type = type;
        attr->u.numd = &priv->frbatt;
        return 1;
    }
    
    if(type==attr_alarm_fr) {
        attr->type = type;
        attr->u.num = priv->fralarm;
        return 1;
    }
    
    if(type==attr_pressure_rr) {
        attr->type = type;
        attr->u.numd = &priv->rrpressure;
        return 1;
    }
    
    if(type==attr_temp_rr) {
        attr->type = type;
        attr->u.numd = &priv->rrtemp;
        return 1;
    }
    
    if(type==attr_batt_rr) {
        attr->type = type;
        attr->u.numd = &priv->rrbatt;
        return 1;
    }
    
    if(type==attr_alarm_rr) {
        attr->type = type;
        attr->u.num = priv->rralarm;
        return 1;
    }
    
    if(type==attr_pressure_rl) {
        attr->type = type;
        attr->u.numd = &priv->rlpressure;
        return 1;
    }
    
    if(type==attr_temp_rl) {
        attr->type = type;
        attr->u.numd = &priv->rltemp;
        return 1;
    }
    
    if(type==attr_batt_rl) {
        attr->type = type;
        attr->u.numd = &priv->rlbatt;
        return 1;
    }
    
    if(type==attr_alarm_rl) {
        attr->type = type;
        attr->u.num = priv->rlalarm;
        return 1;
    }
    
    if(type==attr_alarm) {
        attr->type = type;
        attr->u.num = priv->alarm;
        return 1;
    }
    
    if(type==attr_tpms_connected) {
        attr->type = type;
        attr->u.num = priv->connected;
        return 1;
    }
    
    return 0;
}


/**
 * @brief   Creates the tpms plugin and set some default properties
 * @param[in]   nav - the navit object
 *              meth    - the tpms_methods
 *              cbl     - the pounter to the call back list
 *              attrs   - pointer to the attributes
 *
 * @return  nothing
 *
 * Creates the tpms plugin and set some default properties
 *
 */
static struct headup_priv* tpms_new(struct navit *nav, struct headup_methods *meth, struct callback_list *cbl,
                                    struct attr **attrs) {
    
    struct headup_priv *ret;
    
    dbg(lvl_debug, "enter");
    ret = g_new0(struct headup_priv, 1);
    // Timeout alarm as default so osdcore will display all sensors not yet received in yellow;
    ret->flalarm = 2;
    ret->fralarm = 2;
    ret->rralarm = 2;
    ret->rlalarm = 2;
    ret->nav = nav;
    ret->cbl = cbl;
    ret->log = 0;
    ret->responserec=true;
    ret->cmdsend = false;
    ret->callback = callback_new_1(callback_cast(tpms_init), ret);
    *meth = tpms_methods;
    while (attrs && *attrs)
        tpms_set_attr(ret, *attrs++);
    navit_add_callback(nav, callback_new_attr_1(callback_cast(tpms_init), attr_graphics_ready, ret));
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
    plugin_register_category_headup("tpms", tpms_new);
    dbg(lvl_debug, "plugin_init completed");
}
