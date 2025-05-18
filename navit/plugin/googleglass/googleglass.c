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

#include <math.h>
#include <stdio.h>
#include <glib.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <sys/ioctl.h>
#include "config.h"
#include <navit/item.h>
#include <navit/xmlconfig.h>
#include <navit/main.h>
#include <navit/debug.h>
#include <navit/map.h>
#include <navit/navit.h>
#include <navit/callback.h>
#include <navit/file.h>
#include <navit/plugin.h>
#include <navit/event.h>
#include <navit/command.h>
#include <navit/config_.h>
#include <navit/navit_nls.h>
#include "graphics.h"
#include "color.h"
#include "osd.h"
#include "navigation.h"
#include "vehicle.h"
#include "coord.h"
#include "transform.h"
#include <sys/types.h>
#include <sys/stat.h>
#include "zipfile.h"
#include "roadprofile.h"
#include "track.h"
#include "vehicleprofile.h"
#include "debug.h"
#include <time.h>
#include <sys/time.h>
#include "xmlconfig.h"
#include "headup.h"
#include <stdbool.h>
#include "ZoneDetect/library/zonedetect.h"
#include <unistd.h>
#include <sys/socket.h>
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
#include <btio/btio.h>
#include <attrib/gatttool.h>
#else

#include "objc/objc.h"
#include "googleglass.h"

#endif

int saved_dd = 0, saved_sock = 0;
static uint16_t nus_handle = 0;
#if !defined(__APPLE__)
static int opt_mtu = 257;
GAttrib *attrib;
#endif
volatile int ccbreached = 0;


char savstreetname[30];
char savnextstreetname[30];

bool ggprocessing = false;
bool ggupdate_all = false;
bool ggposition_was_invalid = true;
bool sending=false;
int updatecnt = 0;

struct headup_privgg1 {
    struct callback_list *cbl;
    struct navit *nav;
    int device;
    char * deviceserial;
    char * deviceaddress;
    double last_time;
    char message[255];  // log messages for file
    char *filename;     // log filename
    FILE *fp;           // log file fp
    struct event_timeout *idle;
    struct callback *callback;
    int update_period;

    // comparable values to reduce data over BLE
    char nextmanlen[8];
    int gpssigstrength;
    double gpsheight;
    int gpshdop;
    char streetname[30];
    char streetsysname[30];
    char nextstreetname[30];
    char nextstreetsysname[30];
    char destlen[8];
    char desttime[25];
    unsigned int navnextturnimgtype;
    char distance[8];
    double direction;
    double handledirection;
    int imperial;
    double latitude;
    double longitude;
    double routespeed;
    double vehiclespeed;
    int navstatus;
    double obdspeed;
    double obdcoolant;
    double obdoil;
    double obdvoltage;
    int speedcam_warn;

    // bt related
    int dd; //device descriptor

};

void findHostDeviceGG1(struct headup_privgg1*);
uint16_t crc16_compute(uint8_t const *, uint32_t, uint16_t const *);
char* format_distance(double, int );
void googleglass_init_ble(struct headup_privgg1*);
void googleglass_destroy(struct headup_privgg1*);
void googleglass_connected(void*);
void googleglass_disconnected(void*);
static int googleglass_get_attr(struct headup_privgg1 *priv, enum attr_type type, struct attr *attr);
char* removeAt(char* text);

// BLE commands
enum googleglass_cmds {
    BTHEADUP_DISTANCE = 0x10,
    BTHEADUP_DIRECTION,
    BTHEADUP_IMPERIAL,
    BTHEADUP_NAVSTATUS,
    BTHEADUP_COORDINATESGEO,
    BTHEADUP_NAVNEXTTURNIMAGE,
    BTHEADUP_GPSNUMSATSUSED,
    BTHEADUP_GPSHDOP,
    BTHEADUP_STREETNAME,
    BTHEADUP_STREETSYSNAME,
    BTHEADUP_DESTTIME,
    BTHEADUP_DESTLENGTH,
    BTHEADUP_NEXTSTREETNAME,
    BTHEADUP_NEXTSTREETSYSNAME,
    BTHEADUP_NEXTMANEUVLENGTH,
    BTHEADUP_GPSSIGNALSTRENGTH,
    BTHEADUP_ROUTESPEED,
    BTHEADUP_HANDLEDIRECTION,
    BTHEADUP_VEHICLESPEED,
    BTHEADUP_GPSHEIGHT,
    BTHEADUP_CONTRAST,
    BTHEADUP_ORIENTATION,
    BTHEADUP_OBDSPEED,
    BTHEADUP_OBDVOLTAGE,
    BTHEADUP_OBDOILTEMP,
    BTHEADUP_OBDCOOLANTTEMP,
    BTHEADUP_SPEEDCAMWARN,
    BTHEADUP_TPMSALARM,
};

void googleglass_send(struct headup_privgg1 *this, enum googleglass_cmds cmd, void *data);


static int googleglass_set_attr(struct headup_privgg1 *this, struct attr *attr) {
    if(attr->type == attr_deviceserial) {
        this->deviceserial=attr->u.str;
    }
    if(attr->type == attr_deviceaddress) {
        this->deviceaddress=attr->u.str;
    }
    return 1;
}

uint16_t crc16_compute(uint8_t const *p_data, uint32_t size, uint16_t const *p_crc) {
    uint16_t crc = (p_crc == NULL) ? 0xFFFF : *p_crc;

    for (uint32_t i = 0; i < size; i++) {
        crc = (uint8_t) (crc >> 8) | (crc << 8);
        crc ^= p_data[i];
        crc ^= (uint8_t) (crc & 0xFF) >> 4;
        crc ^= (crc << 8) << 4;
        crc ^= ((crc & 0xFF) << 4) << 1;
    }

    return crc;
}

/**
 * * Format distance, choosing the unit (m or km) and precision depending on distance
 * *
 * * @param distance distance in meters
 * * @param sep separator character to be inserted between distance value and unit
 * * @returns a pointer to a string containing the formatted distance
 * */
char* format_distance(double distance, int imperial) {
    if (imperial) {
        distance *= FEET_PER_METER;
        if (distance <= 500) {
            return g_strdup_printf("%.0f%sft", round(distance / 10) * 10, "");
        } else {
            return g_strdup_printf("%.1f%smi", distance / FEET_PER_MILE, "");
        }
    } else {
        if (distance >= 10000)
            return g_strdup_printf("%.0f%skm", distance / 1000, "");
        else if (distance >= 1000)
            return g_strdup_printf("%.1f%skm", distance / 1000, "");
        else if (distance >= 300)
            return g_strdup_printf("%.0f%sm", round(distance / 25) * 25, "");
        else if (distance >= 50)
            return g_strdup_printf("%.0f%sm", round(distance / 10) * 10, "");
        else if (distance >= 10)
            return g_strdup_printf("%.0f%sm", distance, "");
        else
            return g_strdup_printf("%.1f%sm", distance, "");
    }
}

// Send data via BLE with our protocol
void googleglass_send(struct headup_privgg1 *this, enum googleglass_cmds cmd, void *data) {
#pragma unused (this)
    struct attr *position_attr;
    double *dir, *hdop, *routespeed, *vehiclespeed, *obdspeed, *obdvoltage, *obdcoolanttemp, *obdoiltemp;
    int *imp, *num, *strength, *spdcam, *alarm;
    char *time="", *name;
    //struct attr *position_attr;

    dbg(lvl_debug, "googleglass_send");

    uint8_t btdata[100];
    int x = 2;

    if (nus_handle > 0 && !sending) {

        sending = true;

        switch (cmd) {
        case BTHEADUP_DIRECTION:
            dir = (double*) data;
            btdata[0] = BTHEADUP_DIRECTION;

            memcpy(&btdata[x], dir, sizeof(double));
            x += sizeof(double);

            dbg(lvl_debug, "BTHEADUP_DIRECTION COMMAND CODE: %i, data: %f", cmd, *dir)
            ;

            break;

        case BTHEADUP_HANDLEDIRECTION:
            dir = (double*) data;
            btdata[0] = BTHEADUP_HANDLEDIRECTION;

            memcpy(&btdata[x], dir, sizeof(double));
            x += sizeof(double);

            dbg(lvl_debug, "BTHEADUP_HANDLEDIRECTION COMMAND CODE: %i, data: %f", cmd, *dir)
            ;

            break;

        case BTHEADUP_DISTANCE:
            name = (char*) data;
            btdata[0] = BTHEADUP_DISTANCE;

            memcpy(&btdata[x], name, strlen(name) + 1);
            x += strlen(name) + 1;

            dbg(lvl_debug, "BTHEADUP_DISTANCE COMMAND CODE: %i, data: %s", cmd, name)
            ;
            break;

        case BTHEADUP_IMPERIAL:
            imp = (int*) data;
            btdata[0] = BTHEADUP_IMPERIAL;

            memcpy(&btdata[x], imp, sizeof(int));
            x += sizeof(int);
            dbg(lvl_debug, "BTHEADUP_IMPERIAL COMMAND CODE: %i, data: %i", cmd, *imp)
            ;
            break;
                
        case BTHEADUP_SPEEDCAMWARN:
            spdcam = (int*) data;
            btdata[0] = BTHEADUP_SPEEDCAMWARN;
        
            memcpy(&btdata[x], spdcam, sizeof(int));
            x += sizeof(int);
            dbg(lvl_debug, "BTHEADUP_SPEEDCAMWARN COMMAND CODE: %i, data: %i", cmd, *spdcam)
            ;
            break;

        case BTHEADUP_NAVSTATUS:
            num = (int*) data;
            btdata[0] = BTHEADUP_NAVSTATUS;

            memcpy(&btdata[x], num, sizeof(int));
            x += sizeof(int);
            dbg(lvl_debug, "BTHEADUP_NAVSTATUS COMMAND CODE: %i, data: %i", cmd, *num)
            ;
            break;

        case BTHEADUP_COORDINATESGEO:
            position_attr = (struct attr*) data;

            btdata[0] = BTHEADUP_COORDINATESGEO;

            memcpy(&btdata[x], &position_attr->u.coord_geo->lat, sizeof(double));
            x += sizeof(double);
            memcpy(&btdata[x], &position_attr->u.coord_geo->lng, sizeof(double));
            x += sizeof(double);

            dbg(lvl_debug, "BTHEADUP_COORDINATESGEO COMMAND CODE: %i, data: %s%f %s%f", cmd,
                position_attr->u.coord_geo->lat >= 0 ? "N" : "S", position_attr->u.coord_geo->lat,
                position_attr->u.coord_geo->lng >= 0 ? "E" : "W", position_attr->u.coord_geo->lng)
            ;
            break;

        case BTHEADUP_NAVNEXTTURNIMAGE:
            num = (int*) data;

            btdata[0] = BTHEADUP_NAVNEXTTURNIMAGE;

            memcpy(&btdata[x], num, sizeof(int));
            x += sizeof(int);

            dbg(lvl_debug, "BTHEADUP_NAVNEXTTURNIMAGE COMMAND CODE: %i, data: %i", cmd, *num)
            ;
            break;

        case BTHEADUP_GPSNUMSATSUSED:
            num = (int*) data;
            btdata[0] = BTHEADUP_GPSNUMSATSUSED;

            memcpy(&btdata[x], num, sizeof(int));
            x += sizeof(int);
            dbg(lvl_debug, "BTHEADUP_GPSNUMSATSUSED COMMAND CODE: %i, data: %i", cmd, *num)
            ;
            break;

        case BTHEADUP_GPSHDOP:
            hdop = (double*) data;

            btdata[0] = BTHEADUP_GPSHDOP;

            memcpy(&btdata[x], hdop, sizeof(double));
            x += sizeof(double);
            dbg(lvl_debug, "BTHEADUP_GPSHDOP COMMAND CODE: %i, data: %f", cmd, *hdop)
            ;
            break;

        case BTHEADUP_STREETNAME:
            name = (char*) data;

            btdata[0] = BTHEADUP_STREETNAME;

            memcpy(&btdata[x], name, strlen(name) + 1);
            x += strlen(name) + 1;
            dbg(lvl_debug, "BTHEADUP_STREETNAME COMMAND CODE: %i, data: %s", cmd, name)
            ;
            break;

        case BTHEADUP_STREETSYSNAME:
            name = (char*) data;

            btdata[0] = BTHEADUP_STREETSYSNAME;

            memcpy(&btdata[x], name, strlen(name) + 1);
            x += strlen(name) + 1;
            dbg(lvl_debug, "BTHEADUP_STREETSYSNAME COMMAND CODE: %i, data: %s", cmd, name)
            ;
            break;

        case BTHEADUP_DESTTIME:
            time = (char*) data;
            btdata[0] = BTHEADUP_DESTTIME;

            memcpy(&btdata[x], time, strlen(time) + 1);
            x += strlen(time) + 1;
            dbg(lvl_debug, "BTHEADUP_DESTTIME COMMAND CODE: %i, data: %s", cmd, time)
            ;
            break;

        case BTHEADUP_DESTLENGTH:
            name = (char*) data;
            btdata[0] = BTHEADUP_DESTLENGTH;

            memcpy(&btdata[x], name, strlen(name) + 1);
            x += strlen(name) + 1;
            dbg(lvl_debug, "BTHEADUP_DESTLENGTH COMMAND CODE: %i, data: %s", cmd, name)
            ;
            break;
        case BTHEADUP_NEXTSTREETNAME:
            name = (char*) data;

            btdata[0] = BTHEADUP_NEXTSTREETNAME;

            memcpy(&btdata[x], name, strlen(name) + 1);
            x += strlen(name) + 1;
            dbg(lvl_debug, "BTHEADUP_NEXTSTREETNAME COMMAND CODE: %i, data: %s", cmd, name)
            ;
            break;

        case BTHEADUP_NEXTSTREETSYSNAME:
            name = (char*) data;

            btdata[0] = BTHEADUP_NEXTSTREETSYSNAME;

            memcpy(&btdata[x], name, strlen(name) + 1);
            x += strlen(name) + 1;
            dbg(lvl_debug, "BTHEADUP_NEXTSTREETSYSNAME COMMAND CODE: %i, data: %s", cmd, name)
            ;
            break;

        case BTHEADUP_NEXTMANEUVLENGTH:
            name = (char*) data;

            btdata[0] = BTHEADUP_NEXTMANEUVLENGTH;

            memcpy(&btdata[x], name, strlen(name) + 1);
            x += strlen(name) + 1;
            dbg(lvl_debug, "BTHEADUP_NEXTMANEUVLENGTH COMMAND CODE: %i, data: %s", cmd, name)
            ;
            break;

        case BTHEADUP_GPSSIGNALSTRENGTH:
            strength = (int*) data;
            btdata[0] = BTHEADUP_GPSSIGNALSTRENGTH;

            memcpy(&btdata[x], strength, sizeof(int));
            x += sizeof(int);

            dbg(lvl_debug, "BTHEADUP_GPSSIGNALSTRENGTH COMMAND CODE: %i, data: %i", cmd, *strength)
            ;
            break;

        case BTHEADUP_ROUTESPEED:
            routespeed = (double*) data;
            btdata[0] = BTHEADUP_ROUTESPEED;

            memcpy(&btdata[x], routespeed, sizeof(double));
            x += sizeof(double);
            dbg(lvl_debug, "BTHEADUP_ROUTESPEED COMMAND CODE: %i, data: %f", cmd, *routespeed)
            ;
            break;

        case BTHEADUP_VEHICLESPEED:
            vehiclespeed = (double*) data;

            btdata[0] = BTHEADUP_VEHICLESPEED;

            memcpy(&btdata[x], vehiclespeed, sizeof(double));
            x += sizeof(double);
            dbg(lvl_debug, "BTHEADUP_VEHICLESPEED COMMAND CODE: %i, data: %f", cmd, *vehiclespeed)
            ;
            break;

        case BTHEADUP_GPSHEIGHT:
            routespeed = (double*) data;
            btdata[0] = BTHEADUP_GPSHEIGHT;

            memcpy(&btdata[x], routespeed, sizeof(double));
            x += sizeof(double);
            dbg(lvl_debug, "BTHEADUP_GPSHEIGHT COMMAND CODE: %i, data: %f", cmd, *routespeed)
            ;
            break;

        case BTHEADUP_OBDSPEED:
            obdspeed = (double*) data;
            btdata[0] = BTHEADUP_OBDSPEED;

            memcpy(&btdata[x], obdspeed, sizeof(double));
            x += sizeof(double);
            dbg(lvl_debug, "BTHEADUP_OBDSPEED COMMAND CODE: %i, data: %f", cmd, *obdspeed)
            ;
            break;

        case BTHEADUP_OBDVOLTAGE:
            obdvoltage = (double*) data;
            btdata[0] = BTHEADUP_OBDVOLTAGE;

            memcpy(&btdata[x], obdvoltage, sizeof(double));
            x += sizeof(double);
            dbg(lvl_debug, "BTHEADUP_OBDVOLTAGE COMMAND CODE: %i, data: %f", cmd, *obdvoltage)
            ;
            break;

        case BTHEADUP_OBDCOOLANTTEMP:
            obdcoolanttemp = (double*) data;
            btdata[0] = BTHEADUP_OBDCOOLANTTEMP;

            memcpy(&btdata[x], obdcoolanttemp, sizeof(double));
            x += sizeof(double);
            dbg(lvl_debug, "BTHEADUP_OBDCOOLANTTEMP COMMAND CODE: %i, data: %f", cmd, *obdcoolanttemp)
            ;
            break;

        case BTHEADUP_OBDOILTEMP:
            obdoiltemp = (double*) data;
            btdata[0] = BTHEADUP_OBDOILTEMP;

            memcpy(&btdata[x], obdoiltemp, sizeof(double));
            x += sizeof(double);
            dbg(lvl_debug, "BTHEADUP_OBDOILTEMP COMMAND CODE: %i, data: %f", cmd, *obdoiltemp)
            ;
            break;
                
        case BTHEADUP_TPMSALARM:
                alarm = (int*) data;
                btdata[0] = BTHEADUP_TPMSALARM;
                
                memcpy(&btdata[x], alarm, sizeof(int));
                x += sizeof(int);
                
                dbg(lvl_debug, "BTHEADUP_TPMSALARM COMMAND CODE: %i, data: %i", cmd, *alarm)
                ;
                break;
                
        default:
            dbg(lvl_debug, "UNKONWN COMMAND CODE: %i", cmd)
            ;
        }
#if !defined(__APPLE__)
        GIOChannel *channel = g_attrib_get_channel(attrib);
        int fd = g_io_channel_unix_get_fd(channel);
        int status = fcntl(fd, F_GETFD);

        if (status < 0) {
            nus_handle = 0;
            event_remove_timeout(this->idle);
            this->callback = callback_new_1(callback_cast(findHostDeviceGG1), this);
            this->idle = event_add_timeout(700, 1, this->callback);
            return;
        }

        if (x > 2) {
            btdata[1] = x + 2; // length of data
            uint16_t crc;
            crc = crc16_compute(btdata, x, NULL);
            btdata[x++] = crc >> 8;
            btdata[x++] = crc & 0xFF;
            btdata[x++] = 0x0D;
            btdata[x++] = 0x0A;

            gatt_write_cmd(attrib, nus_handle, btdata, x, NULL,
                           NULL);

            for (int i = 0; i < 1; i++) {
                dbg(lvl_debug, "Data: %02x\n", btdata[i]);
            }
        }
#else
        if (x > 2) {
            btdata[1] = x + 2; // length of data
            uint16_t crc;
            crc = crc16_compute(btdata, x, NULL);
            btdata[x++] = crc >> 8;
            btdata[x++] = crc & 0xFF;
            btdata[x++] = 0x0D;
            btdata[x++] = 0x0A;

            bthbtcontrollergg1_send(btdata, x);
            //sleep(1);
        }
#endif
        sending = false;

    } else {
        dbg(lvl_debug, "No NUS handle yet.");
    }

    return;
}


/**
 * * Format time (duration)
 * *
 * * @param tm pointer to a tm structure specifying the time
 * * @param days days
 * * @returns a pointer to a string containing the formatted time
 * */
static char* format_time(struct tm *tm, int days) {
    if (days)
        return g_strdup_printf("%d+%02d:%02d", days, tm->tm_hour, tm->tm_min);
    else
        return g_strdup_printf("%02d:%02d", tm->tm_hour, tm->tm_min);
}

#define min(a,b) \
  ({ __typeof__ (a) _a = (a); \
      __typeof__ (b) _b = (b); \
    _a < _b ? _a : _b; })

/**
 * @brief   Function called when navit is idle. Does the continuous reading
 * @param   this the googleglass struct containing the state of the plugin
 *
 * @return  nothing
 *
 * This is the main function of this plugin. It is called when navit is idle,
 * collects all data needed from navit and feeds the display via BLE.
 *
 */
static void googleglass_idle(struct headup_privgg1 *this) {

    struct attr position_attr, maxspeed_attr, speed_attr, vehicle_attr, imperial_attr, direction_attr, destination_attr,
               posfixtype_attr, posheight_attr, position_sats_used_attr, position_hdop_attr, streetname_attr,
               streetnamesys_attr, dest_time_attr, nav_attr, dest_length_attr, streetnamenext_attr,
               streetnamesysnext_attr, next_length_attr, valid, spdcamwarn_attr;
    struct vehicle *curr_vehicle;
    int imperial = 0;
    double vehicledir;
    double dir;
    double *height;
    enum projection pro;
    struct coord c1, c2;
    struct navigation *nav = NULL;
    char *name = "unknown";
    struct map *map = NULL;
    struct map_rect *mr = NULL;
    struct item *item = NULL;
    int strength;
    struct tracking *tracking = NULL;

    if (ggprocessing) {
        dbg(lvl_error, "IDLE - Blocking execution");
        return;
    }

    ggprocessing = true;
    updatecnt++;

    if(updatecnt==10) {
        ggupdate_all = true;
        updatecnt = 0;
    }

// TODO:
// Support Android and iOS BT connections

// send data from obd, compass, position, speed, next turn instruction, ETA, speedlimit, conditional speedlimit, street, next street, gps status via Nordic NUS to the display device

    int tpms = 0;
    
    navit_get_tpmsconnected(this->nav, &tpms);
    
    if(tpms) {
        long alarm;
        navit_get_tpms_alarm(this->nav, &alarm);
        googleglass_send(this, BTHEADUP_TPMSALARM, &alarm);
    }
    
    
    
    int obd=1;
    //navit_get_obdconnected(this->nav, &obd);
    
    

    if(obd) {

        double obdspeed=0.0;
        navit_get_obdspeed(this->nav, &obdspeed);
        if(this->obdspeed != obdspeed || ggupdate_all) {
            googleglass_send(this, BTHEADUP_OBDSPEED, &obdspeed);
            this->obdspeed = obdspeed;
        }

        double obdoil=0.0;
        navit_get_obdoiltemp(this->nav, &obdoil);
        if(this->obdoil != obdoil || ggupdate_all) {
            googleglass_send(this, BTHEADUP_OBDOILTEMP, &obdoil);
            this->obdoil = obdoil;
        }

        double obdcoolant=0.0;
        navit_get_obdcoolanttemp(this->nav, &obdcoolant);
        if(this->obdcoolant != obdcoolant || ggupdate_all) {
            googleglass_send(this, BTHEADUP_OBDCOOLANTTEMP, &obdcoolant);
            this->obdcoolant = obdcoolant;
        }

        double obdvoltage=0.0;
        navit_get_obdvoltage(this->nav, &obdvoltage);
        if(this->obdvoltage != obdvoltage || ggupdate_all) {
            googleglass_send(this, BTHEADUP_OBDVOLTAGE, &obdvoltage);
            this->obdvoltage = obdvoltage;
        }

    }

    if (navit_get_attr(this->nav, attr_imperial, &imperial_attr, NULL)) {
        imperial = (int)imperial_attr.u.num;

        if (imperial != this->imperial || ggupdate_all) {
            googleglass_send(this, BTHEADUP_IMPERIAL, &imperial);
            this->imperial = imperial;
        }
    }

    navit_get_attr(this->nav, attr_vehicle, &vehicle_attr, NULL);

    if (vehicle_attr.u.vehicle) {
        curr_vehicle = vehicle_attr.u.vehicle;
    } else {
        curr_vehicle = 0;
    }

    if (0 == curr_vehicle) {
        ggprocessing = false;
        return;
    }

    if (vehicle_get_attr(curr_vehicle, attr_position_valid, &valid, NULL)) {
        if(valid.u.num!=attr_position_valid_valid) {
            ggposition_was_invalid = true;
        } else {

            struct osd_priv_common {
                struct osd_item osd_item;
                struct osd_priv *data;
                int (*spec_set_attr_func)(struct osd_priv_common *opc, struct attr *attr);
            };
            if(ggposition_was_invalid) {
                ggupdate_all=true;
                ggposition_was_invalid=false;
            }
        }
    }

// Coordinates
    if (vehicle_get_attr(curr_vehicle, attr_position_coord_geo, &position_attr, NULL)) {
        if (round(100000 * position_attr.u.coord_geo->lat) / 100000 != this->latitude
                || round(100000 * position_attr.u.coord_geo->lng) / 100000 != this->longitude || ggupdate_all) {
            googleglass_send(this, BTHEADUP_COORDINATESGEO, (void*) &position_attr);
            this->longitude = round(100000 * position_attr.u.coord_geo->lng) / 100000;
            this->latitude = round(100000 * position_attr.u.coord_geo->lat) / 100000;
        }
    }

// Direction for compass
    if (vehicle_get_attr(curr_vehicle, attr_position_direction, &direction_attr, NULL)) {
        vehicledir = *direction_attr.u.numd * -1;
        if (vehicledir != this->direction || ggupdate_all) {
            googleglass_send(this, BTHEADUP_DIRECTION, (void*) &vehicledir);
            this->direction = vehicledir;
        }
    }

// Speedcam warning
    if (navit_get_attr(this->nav, attr_speedcam_warn, &spdcamwarn_attr, NULL)) {
           
                int speedcam_warn = (int)spdcamwarn_attr.u.num;
                if(this->speedcam_warn != speedcam_warn || ggupdate_all) {
                    googleglass_send(this, BTHEADUP_SPEEDCAMWARN, &speedcam_warn);
                    this->speedcam_warn = speedcam_warn;
                }

    }

// Distance to destination
    if (navit_get_attr(this->nav, attr_destination, &destination_attr, NULL)
            && vehicle_get_attr(curr_vehicle, attr_position_coord_geo, &position_attr, NULL)) {
        pro = destination_attr.u.pcoord->pro;
        transform_from_geo(pro, position_attr.u.coord_geo, &c1);
        if (destination_attr.u.pcoord->x != 0 && destination_attr.u.pcoord->y != 0) {
            c2.x = destination_attr.u.pcoord->x;
            c2.y = destination_attr.u.pcoord->y;
            double dist = transform_distance(pro, &c1, &c2);
            char *distnew = format_distance(dist, imperial);

            if (strcmp(distnew, this->distance) || ggupdate_all) {
                googleglass_send(this, BTHEADUP_DISTANCE, distnew);
                strncpy(this->distance, distnew, sizeof(this->distance));
            }

            dir = atan2(c2.x - c1.x, c2.y - c1.y) * 180.0 / M_PI;
            dir += vehicledir;

            if (dir != this->handledirection || ggupdate_all) {
                googleglass_send(this, BTHEADUP_HANDLEDIRECTION, &dir);
                this->handledirection = dir;
            }
        }

    }

// Navigation status
    nav = navit_get_navigation(this->nav);
    if (nav) {
        if (navigation_get_attr(nav, attr_nav_status, &nav_attr, NULL)) {
            int navstatus = (int)nav_attr.u.num; //nav_status_to_text(nav_attr.u.num);
            if(this->navstatus != navstatus || ggupdate_all) {
                googleglass_send(this, BTHEADUP_NAVSTATUS, &navstatus);
                this->navstatus = navstatus;
            }
        }
    }

// Next turn image name and destination data
    if (nav)
        map = navigation_get_map(nav);
    if (map)
        mr = map_rect_new(map, NULL);
    if (mr)
        while ((item = map_rect_get_item(mr)) && (item->type == type_nav_position || item->type == type_nav_none))
            ;

    if (item) {
        name = item_to_name(item->type);
        dbg(lvl_debug, "name=%s", name);

        if (this->navnextturnimgtype != item->type || ggupdate_all) {
            googleglass_send(this, BTHEADUP_NAVNEXTTURNIMAGE, &item->type);
            this->navnextturnimgtype = item->type;
        }
        if (mr)
            map_rect_destroy(mr);
    }

    if (nav)
        map = navigation_get_map(nav);
    if (map)
        mr = map_rect_new(map, NULL);

    item = map_rect_get_item(mr);

    if (item) {

        if (item->type == type_nav_position) {

//            if (item_attr_get(item, attr_street_name_systematic, &streetnamesys_attr) || ggupdate_all) {
//                char *street = streetnamesys_attr.u.str;
//                dbg(lvl_debug, "streetsystematic=%s", street);
//                if (street)
//                    strncpy(this->streetsysname, street, sizeof(this->streetsysname));
//            } else {
//                this->streetsysname[0] = 0;
//            }
//
//            // We transfer streetsysname + streetname at once
//            if (item_attr_get(item, attr_street_name, &streetname_attr) || ggupdate_all) {
//                char *street = streetname_attr.u.str;
//                if (street) {
//                    if (!strcmp(this->streetsysname, "")) {
//                        strncpy(this->streetname, street, sizeof(this->streetname));
//                    } else {
//                        strncpy(this->streetname, this->streetsysname, sizeof(this->streetname));
//                        strncat(this->streetname, " ", sizeof(this->streetname) - strlen(this->streetname)-1);
//                        strncat(this->streetname, street, sizeof(this->streetname) - strlen(this->streetname)-1);
//                    }
//                }
//            } else {
//                if (strcmp(this->streetsysname, "")) {
//                    strncpy(this->streetname, this->streetsysname, sizeof(this->streetname));
//                } else {
//                    this->streetname[0] = 0;
//                }
//            }
//
//            if (strcmp(this->streetname, savstreetname) || ggupdate_all) {
//                dbg(lvl_debug, "street=%s", this->streetname);
//                googleglass_send(this, BTHEADUP_STREETNAME, this->streetname);
//                strncpy(savstreetname, this->streetname, sizeof(savstreetname));
//            }

            if (item_attr_get(item, attr_street_name_systematic, &streetnamesys_attr) || ggupdate_all) {
                char *street = removeAt(streetnamesys_attr.u.str);
                dbg(lvl_debug, "streetsystematic=%s", street);
                if (street)
                    snprintf(this->streetsysname, sizeof this->streetsysname, "%s", street);
                else
                    this->streetsysname[0] = 0;
            } else {
                this->streetsysname[0] = 0;
            }

            // We transfer streetsysname + streetname at once
            if (item_attr_get(item, attr_street_name, &streetname_attr) || ggupdate_all) {
                char *street = removeAt(streetname_attr.u.str);
                if (street) {
                    if (!strcmp(this->streetsysname, "")) {
                        snprintf(this->streetname, sizeof this->streetname, "%s", street);
                    } else {
                        snprintf(this->streetname, sizeof this->streetname, "%s %s", this->streetsysname, street);
                    }
                } else if (strcmp(this->streetsysname, "")) {
                    snprintf(this->streetname, sizeof this->streetname, "%s", this->streetsysname);
                }
            } else {
                if (strcmp(this->streetsysname, "")) {
                    snprintf(this->streetname, sizeof this->streetname, "%s", this->streetsysname);
                } else {
                    this->streetname[0] = 0;
                }
            }

            if (strcmp(this->streetname, savstreetname) || ggupdate_all) {
                dbg(lvl_debug, "street=%s", this->streetname);
                googleglass_send(this, BTHEADUP_STREETNAME, this->streetname);
                snprintf(savstreetname, sizeof savstreetname, "%s", this->streetname);
            }

            if (item_attr_get(item, attr_destination_time, &dest_time_attr)) {
                int days = 0;
                struct tm text_tm, tm, text_tm0;

                time_t textt = time(NULL);
                tm = *localtime(&textt);
                //            // remaining
                //            textt -= tm.tm_hour * 3600 + tm.tm_min * 60 + tm.tm_sec;
                //            tm = *localtime(&textt);

                textt += dest_time_attr.u.num / 10;

                // Adapt ETA to currenmt timezone

                char *zd_file=g_strjoin(NULL, navit_get_user_data_directory(TRUE), "/timezone21.bin", NULL);
                ZoneDetect *const cd = ZDOpenDatabase(zd_file);

                if(cd) {
                    float safezone = 0;
                    ZoneDetectResult *results = ZDLookup(cd, this->latitude, this->longitude, &safezone);
                    char timezone[100];
                    timezone[0]=0;
                    unsigned int index = 0;

                    while(results[index].lookupResult != ZD_LOOKUP_END) {
                        if(results[index].data) {
                            for(unsigned int i = 0; i < results[index].numFields; i++) {
                                if(results[index].fieldNames[i] && results[index].data[i]) {
                                    strcat(timezone, results[index].data[i]);
                                    if(i==1)
                                        break;
                                }
                            }
                        }

                        index++;
                    }

                    ZDFreeResults(results);
                    ZDCloseDatabase(cd);

                    if(timezone[0]!=0)
                        setenv("TZ", timezone, 1);
                }

                text_tm = *localtime(&textt);
                if (tm.tm_year != text_tm.tm_year || tm.tm_mon != text_tm.tm_mon || tm.tm_mday != text_tm.tm_mday) {
                    text_tm0 = text_tm;
                    text_tm0.tm_sec = 0;
                    text_tm0.tm_min = 0;
                    text_tm0.tm_hour = 0;
                    tm.tm_sec = 0;
                    tm.tm_min = 0;
                    tm.tm_hour = 0;
                    days = (int)(mktime(&text_tm0) - mktime(&tm) + 43200) / 86400;
                }
                char *stime = format_time(&text_tm, days);
                //            dbg(lvl_debug, "destination_time=%i", time);
                if (strcmp(stime, this->desttime) || ggupdate_all) {
                    googleglass_send(this, BTHEADUP_DESTTIME, stime);
                    strncpy(this->desttime, stime, sizeof(this->desttime));
                }
            }

            if (item_attr_get(item, attr_destination_length, &dest_length_attr)) {
                int len = (int)dest_length_attr.u.num;
                dbg(lvl_debug, "destination_length=%i", len);
                char *distnew = format_distance(len, imperial);

                if (strcmp(distnew, this->destlen) || ggupdate_all) {
                    googleglass_send(this, BTHEADUP_DESTLENGTH, distnew);
                    strncpy(this->destlen, distnew, sizeof(this->destlen));
                }
            }

            item = map_rect_get_item(mr);

            if (item) {

//                if (item_attr_get(item, attr_street_name_systematic, &streetnamesysnext_attr)) {
//                    char *street = streetnamesysnext_attr.u.str;
//                    dbg(lvl_debug, "nextstreetsystematic=%s   sizeof(this->nextstreetsysname) %lu  sizeof(street)%lu", street, sizeof(this->nextstreetsysname)-1, sizeof(street));
//                    strncpy(this->nextstreetsysname, street, min(sizeof(this->nextstreetsysname)-1, strlen(street)+1));
//                } else {
//                    this->nextstreetsysname[0] = 0;
//                }
//
//                // We transfer nextstreetsysname + nextstreetname at once
//                if (item_attr_get(item, attr_street_name, &streetnamenext_attr)) {
//                    char *street = streetnamenext_attr.u.str;
//                    if (street) {
//                        if (!strcmp(this->nextstreetsysname, "")) {
//                            strncpy(this->nextstreetname, street, min(sizeof(this->nextstreetname)-1, strlen(street)+1));
//                        } else {
//                            strncpy(this->nextstreetname, this->nextstreetsysname, min(sizeof(this->nextstreetname)-1, strlen(this->nextstreetsysname)));
//                            if(strlen(this->nextstreetname)<(sizeof(this->nextstreetname)-1))
//                                strncat(this->nextstreetname, " ", 1);
//                            if(strlen(this->nextstreetname)<(sizeof(this->nextstreetname)-1))
//                                strncat(this->nextstreetname, street, min(sizeof(this->nextstreetname) - strlen(this->nextstreetname) - 1, strlen(street)));
//                        }
//                    }
//                } else {
//                    if (strcmp(this->nextstreetsysname, "")) {
//                        strncpy(this->nextstreetname, this->nextstreetsysname, min(sizeof(this->nextstreetname) -1, strlen(this->nextstreetsysname)+1));
//                    } else {
//                        this->nextstreetname[0] = 0;
//                    }
//                }
//
//                if (strcmp(this->nextstreetname, savnextstreetname) || ggupdate_all) {
//                    dbg(lvl_debug, "nextstreet=%s", this->nextstreetname);
//                    googleglass_send(this, BTHEADUP_NEXTSTREETNAME, this->nextstreetname);
//                    strncpy(savnextstreetname, this->nextstreetname, sizeof(savnextstreetname));
//                }

                if (item_attr_get(item, attr_street_name_systematic, &streetnamesysnext_attr)) {
                    char *street = removeAt(streetnamesysnext_attr.u.str);
                    dbg(lvl_debug, "nextstreetsystematic=%s", street);
                    snprintf(this->nextstreetsysname, sizeof this->nextstreetsysname, "%s", street);
                } else {
                    this->nextstreetsysname[0] = 0;
                }

                // We transfer nextstreetsysname + nextstreetname at once
                if (item_attr_get(item, attr_street_name, &streetnamenext_attr)) {
                    char *street = removeAt(streetnamenext_attr.u.str);
                    if (street) {
                        if (!strcmp(this->nextstreetsysname, "")) {
                            snprintf(this->nextstreetname, sizeof this->nextstreetname, "%s%s", " ", street);
                        } else {
                            snprintf(this->nextstreetname, sizeof this->nextstreetname, "%s %s", this->nextstreetsysname, street);
                        }
                    }
                } else {
                    if (strcmp(this->nextstreetsysname, "")) {
                        snprintf(this->nextstreetname, sizeof this->nextstreetname, "%s", this->nextstreetsysname);
                    } else {
                        this->nextstreetname[0] = 0;
                    }
                }

                if (strcmp(this->nextstreetname, savnextstreetname) || ggupdate_all) {
                    dbg(lvl_debug, "nextstreet=%s", this->nextstreetname);
                    googleglass_send(this, BTHEADUP_NEXTSTREETNAME, this->nextstreetname);
                    snprintf(savnextstreetname, sizeof savnextstreetname, "%s", this->nextstreetname);
                }

                if (item_attr_get(item, attr_length, &next_length_attr)) {

                    int len = (int)next_length_attr.u.num;
                    char *distnew = format_distance(len, imperial);

                    if (strcmp(distnew, (char*) this->nextmanlen) || ggupdate_all) {
                        googleglass_send(this, BTHEADUP_NEXTMANEUVLENGTH, distnew);
                        strncpy(this->nextmanlen, distnew, sizeof(this->nextmanlen));
                    }
                }
            }
        }
    }

    if (mr)
        map_rect_destroy(mr);

// GPS signal strength
    if (vehicle_get_attr(curr_vehicle, attr_position_fix_type, &posfixtype_attr, NULL)) {
        switch (posfixtype_attr.u.num) {
        case 1:
        case 2:
        case 3:
        case 4:
            strength = 2;
            if (vehicle_get_attr(curr_vehicle, attr_position_sats_used, &position_sats_used_attr, NULL)) {
                //dbg(lvl_debug, "numsatsused=%ld", position_sats_used_attr.u.num);
                if (position_sats_used_attr.u.num >= 3)
                    strength = (int)position_sats_used_attr.u.num - 1;
                if (strength > 5)
                    strength = 5;
                if (strength > 3) {
                    if (vehicle_get_attr(curr_vehicle, attr_position_hdop, &position_hdop_attr, NULL)) {
                        if (*position_hdop_attr.u.numd > 2.0 && strength > 4)
                            strength = 4;
                        if (*position_hdop_attr.u.numd > 4.0 && strength > 3)
                            strength = 3;
//                        dbg(lvl_debug, "hdop=%f", *position_hdop_attr.u.numd);
                        if (*position_hdop_attr.u.numd != this->gpshdop || ggupdate_all) {
                            googleglass_send(this, BTHEADUP_GPSHDOP, position_hdop_attr.u.numd);
                            this->gpshdop = *position_hdop_attr.u.numd;
                        }
                    }
                }
            }
            break;
        default:
            strength = 1;
        }

    } else {
        strength = 1;
    }

    if (strength != this->gpssigstrength || ggupdate_all) {
        googleglass_send(this, BTHEADUP_GPSSIGNALSTRENGTH, &strength);
        this->gpssigstrength = strength;
    }

    if (vehicle_get_attr(curr_vehicle, attr_position_height, &posheight_attr, NULL)) {
        height = posheight_attr.u.numd;
        if ((int) *height != (int) this->gpsheight || ggupdate_all) {
            googleglass_send(this, BTHEADUP_GPSHEIGHT, height);
            this->gpsheight = *height;
        }
    } else {
        dir = -1;
        height = &dir;
        if ((int) *height != (int) this->gpsheight || ggupdate_all) {
            googleglass_send(this, BTHEADUP_GPSHEIGHT, height);
            this->gpsheight = *height;
        }
    }

// maxspeed
    tracking = navit_get_tracking(this->nav);

    if (tracking) {
        item = tracking_get_current_item(tracking);

        double routespeed = -1;
        double vehiclespeed = -1;
        int *flags = tracking_get_current_flags(tracking);

        if (flags && (*flags & AF_SPEED_LIMIT) && tracking_get_attr(tracking, attr_maxspeed, &maxspeed_attr, NULL)) {
            routespeed = maxspeed_attr.u.num;
        }

// Dont't show profile speed (make it configurable)
//        if (routespeed == -1) {
//            struct vehicleprofile *prof = navit_get_vehicleprofile(this->nav);
//            struct roadprofile *rprof = NULL;
//            if (prof && item)
//                rprof = vehicleprofile_get_roadprofile(prof, item->type);
//            if (rprof) {
//                routespeed = rprof->speed;
//            }
//        }

        if (imperial) {
            routespeed = routespeed * 1000 * FEET_PER_METER / FEET_PER_MILE;
        }

        if (routespeed != this->routespeed || ggupdate_all) {
            googleglass_send(this, BTHEADUP_ROUTESPEED, &routespeed);
            this->routespeed = routespeed;
        }

        tracking_get_attr(tracking, attr_position_speed, &speed_attr, NULL);
        vehiclespeed = *speed_attr.u.numd;

        if (imperial) {
            vehiclespeed = vehiclespeed * 1000 * FEET_PER_METER / FEET_PER_MILE;
        }

        if (vehiclespeed != this->vehiclespeed || ggupdate_all) {
            googleglass_send(this, BTHEADUP_VEHICLESPEED, &vehiclespeed);
            this->vehiclespeed = vehiclespeed;
        }

    }

    ggprocessing = false;
    ggupdate_all = false;

//    sleep(1);
}

/**
 * @brief       Initialize the googleglass plugin
 * @param[in]   googleglass    - the googleglass struct containing the state of the plugin
 *              nav     - the navit object
 *
 * @return      nothing
 *
 * Initialize the googleglass plugin
 *
 */
static void googleglass_init(struct headup_privgg1 *this) {
    googleglass_init_ble(this);
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

        dbg(lvl_error, "Found BT-Headup device with NUS service:\nhandle = 0x%04x, char properties = 0x%02x, char value "
            "handle = 0x%04x, uuid = %s\n", chars->handle, chars->properties, chars->value_handle, chars->uuid);

        if (!strcmp(chars->uuid, NUS_CHARACTERISTIC_RX)) {
            nus_handle = chars->value_handle;
            ggupdate_all = true;
        }
    }

done:
    return;
}

static void connect_cb(GIOChannel *io, GError *err, gpointer user_data) {
    uint16_t mtu;
    uint16_t cid;
    GError *gerr = NULL;

    ccbreached = 1;

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
}

static void exchange_mtu_cb(guint8 status, const guint8 *pdu, guint16 plen, gpointer user_data) {
    uint16_t mtu;

    if (status != 0) {
        dbg(lvl_error, "Exchange MTU Request failed: %s\n", att_ecode2str(status));
        return;
    }

    if (!dec_mtu_resp(pdu, plen, &mtu)) {
        dbg(lvl_error, "Protocol error\n");
        return;
    }

    mtu = MIN(mtu, opt_mtu);
    /* Set new value for MTU in client */
    if (g_attrib_set_mtu(attrib, mtu)) {
        dbg(lvl_debug, "MTU was exchanged successfully: %d\n", mtu);
    } else {
        dbg(lvl_error, "Error exchanging MTU");
    }
}

void findHostDeviceGG1(struct headup_privgg1 *this) {
    int dev_id, sock;
    char addr[19] = { 0 };  // 7F:C1:D3:37:66:31 + \0

    if (nus_handle > 0) {
        event_remove_timeout(this->idle);
        this->callback = callback_new_1(callback_cast(googleglass_idle), this);
        this->idle = event_add_timeout(1000, 1, this->callback);
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

    char *opt_dst_type = g_strdup("random");
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
        attrib = g_attrib_new(chan, 257, 0);

        gatt_exchange_mtu(attrib, opt_mtu, exchange_mtu_cb, NULL);

        bt_uuid_t uuid;
        bt_string_to_uuid(&uuid, NUS_CHARACTERISTIC_RX);

        dbg(lvl_debug, "Discover Characteristics");
        gatt_discover_char(attrib, 0x0001, 0xFFFF, &uuid, char_discovered_cb, chan);

        return;
    } else {
        dbg(lvl_error, "GATT connect: GIOChannel is NULL");
    }

    return;
}

#else

void findHostDeviceGG1(struct headup_privgg1 *this) {

    if (nus_handle > 0) {
        event_remove_timeout(this->idle);
        this->callback = callback_new_1(callback_cast(googleglass_idle), this);
        this->idle = event_add_timeout(700, 1, this->callback);
        callback_list_call_attr_0(this->cbl, attr_glass_connected);
        return;
    }

    if(this->deviceserial != NULL)
        bthbtcontrollergg1_init(this, googleglass_disconnected, googleglass_connected, this->deviceserial);
}

void googleglass_connected(void* _this) {
    struct headup_privgg1 * this = (struct headup_privgg1 *) _this;
    dbg(lvl_error, "googleglass_connected %p", _this);
    nus_handle = 1;
    ggupdate_all=true;
    callback_list_call_attr_0(this->cbl, attr_glass_connected);
    navit_say(this->nav, _("Google Glass connected"));
}

void googleglass_disconnected(void* _this) {
    struct headup_privgg1 * this = (struct headup_privgg1 *) _this;
    dbg(lvl_error, "googleglass_disconnected: %p", _this);

    if(nus_handle)
        navit_say(this->nav, _("Google Glass disconnected"));

    nus_handle = 0;
    callback_list_call_attr_0(this->cbl, attr_glass_connected);


}

#endif

/**
 * @brief   Opens the serial port and saves state to the googleglass object
 * @param[in]   googleglass - the googleglass struct containing the state of the plugin
 *
 * @return  nothing
 *
 * Opens the serial port and saves state to the googleglass object
 *
 */
void googleglass_init_ble(struct headup_privgg1 *this) {
// Check for BT host device and display every 10 seconds
    this->callback = callback_new_1(callback_cast(findHostDeviceGG1), this);
    this->idle = event_add_timeout(1000, 1, this->callback);
    return;

}

#if !defined(__APPLE__)
void googleglass_destroy(struct headup_privgg1* ptr) {
// stop any active scan here
    if (saved_dd) {
        hci_le_set_scan_enable(saved_dd, 0, 0, 1000); // disable in case already enabled
        hci_close_dev(saved_dd);
    }
    if (saved_sock)
        close(saved_sock);
}
#else

void googleglass_destroy(struct headup_privgg1* ptr) {
#pragma unused (ptr)
}

#endif

static int googleglass_get_attr(struct headup_privgg1 *priv, enum attr_type type, struct attr *attr) {
#pragma unused (priv)
    if(type==attr_glass_connected) {
        attr->type = type;
        attr->u.num = nus_handle;
        return 1;
    }

    return 0;
}
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wincompatible-function-pointer-types"
struct headup_methods googleglass_methods = {
    googleglass_destroy,
    googleglass_set_attr,
    googleglass_get_attr,
};


/**
 * @brief   Creates the googleglass plugin and set some default properties
 * @param[in]   nav - the navit object
 *              meth    - the osd_methods
 *      attrs   - pointer to the attributes
 *
 * @return  nothing
 *
 * Creates the googleglass plugin and set some default properties
 *
 */
static struct headup_privgg1* googleglass_new(struct navit *nav, struct headup_methods *meth, struct callback_list *cbl,
        struct attr **attrs) {

//return NULL;
    struct headup_privgg1 *ret;

    dbg(lvl_debug, "enter");
    ret = g_new0(struct headup_privgg1, 1);
    ret->nav=nav;
    ret->cbl = cbl;
    ret->callback=callback_new_1(callback_cast(googleglass_init), ret);
    *meth = googleglass_methods;
    while (attrs && *attrs)
        googleglass_set_attr(ret, *attrs++);
    navit_add_callback(nav, callback_new_attr_1(callback_cast(googleglass_init), attr_graphics_ready, ret));
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
    plugin_register_category_headup("googleglass", googleglass_new );
    dbg(lvl_debug, "plugin_init completed");
}

char* removeAt(char* text) {
    if(text) {
        char **split = g_strsplit(text, "@", -1);
        text = g_strjoinv("", split);
        g_strfreev(split);
    }
    return text;
}

//#pragma clang diagnostic pop3
