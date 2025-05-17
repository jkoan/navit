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

#include "start_real.h"
#import <Foundation/Foundation.h>
#include <glib.h>
#include <stdlib.h>
#include "debug.h"

int main(int argc, char **argv) {
    int ret;
    //NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
    NSString *appFolderPath = [[NSBundle mainBundle] resourcePath];
    //NSString *locale = [[NSLocale currentLocale] localeIdentifier];
    NSString *language = [[NSLocale preferredLanguages] firstObject];
    if(language.UTF8String[2]=='-') //IOS returns e.g. de-DE instead of de_DE
        language=[language stringByReplacingOccurrencesOfString:@"-" withString:@"_"];
    if(strlen(language.UTF8String)==
            2) //IOS can have language "en" e.g. then the country selection in country.c does not work
        language=[NSString stringWithFormat:@"%@_%@", language, language.uppercaseString];
    char *lang=g_strdup_printf("%s.UTF-8",[language UTF8String]);
    dbg(lvl_debug,"lang %s",lang);
    setenv("LANG",lang,0);
    setlocale(LC_ALL, NULL);
    setlocale(LC_NUMERIC,"C");

    const char *s=[appFolderPath UTF8String];
#if IOS
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *documentsDirectory = [paths objectAtIndex:0];
    char *user=(char *)[documentsDirectory cStringUsingEncoding:[NSString defaultCStringEncoding]];
    NSFileManager *fMgr = [NSFileManager defaultManager];
    NSString *mapPath = [documentsDirectory stringByAppendingPathComponent:[NSString stringWithFormat:@"%@.bin",
                         @"osm_bbox_11.3,47.9,11.7,48.2"]];
    NSError *error = nil;
    NSString *rsrcPath = [[NSBundle mainBundle] pathForResource:@"share/navit/osm_bbox_11.3,47.9,11.7,48.2" ofType:@"bin"];

    if(rsrcPath!=nil) {
        if(![fMgr fileExistsAtPath:mapPath]) {
            [fMgr copyItemAtPath:rsrcPath toPath:mapPath error:&error];
            if (error) {
                NSLog(@"Error on copying file: %@\nfrom path: %@\ntoPath: %@", error, rsrcPath, mapPath);
            }
        }
    }
#else
    char *user=g_strdup_printf("%s/../Documents",s);
#endif
    chdir(s);
    argv[0]=g_strdup_printf("%s/bin/navit",s);
    setenv("NAVIT_USER_DATADIR",user,0);

    dbg(lvl_debug,"calling main_real");
    ret=main_real(argc, argv);
    g_free(argv[0]);
    g_free(user);
    //[pool release];
    return ret;
}
