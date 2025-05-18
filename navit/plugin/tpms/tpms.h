/**
 * Navit, a modular navigation system.
 * Copyright (C) 2005-2008 Navit Team
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

#ifndef TPMS_H
#define TPMS_H

typedef void(*RECEIVE_CB)(void*, const char*, const unsigned char*, const char*);
void tpmsbtcontroller_init(void* pbth_arg, RECEIVE_CB recv_cb, char *frontleftaddr, char *frontrightaddr, char *rearleftaddr, char *rearrightaddr, char* name);
void tpmsbtcontroller_send(char* data, unsigned long length);

#ifdef TPMS_OBJC
#import <CoreBluetooth/CoreBluetooth.h>
#import "TPMSSensor.h"

@interface TPMS : NSObject <CBCentralManagerDelegate, CBPeripheralDelegate> {
    
@public
    RECEIVE_CB recv_cb;
    void * pbth_arg;
    char *name;
    char *frontleftaddr;
    char *frontrightaddr;
    char *rearleftaddr;
    char *rearrightaddr;
    NSTimer *recv_timeout;
}

@property (strong, nonatomic) CBCentralManager *centralManager;
@property (strong, nonatomic) NSMutableData *data;
@property (nonatomic) void * pbth_arg;
@property (nonatomic) RECEIVE_CB recv_cb;

@end

#endif
#endif
