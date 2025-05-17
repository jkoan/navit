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

#include <stdlib.h>
#include <glib.h>
#include "config.h"
#include "item.h"
#include "debug.h"
#include "plugin.h"
#include "speech.h"
#include "attr.h"
#include "vehicle.h"
#import "VSSpeechSynthesizerNavit.h"
#import <UIKit/UIKit.h>
#include "ZoneDetect/library/zonedetect.h"
#include "glib.h"
#include "navit.h"
#include "item.h"
#include "coord.h"
#include "attr.h"

#define DEFAULT_HFP_DELAY 0.5

struct speech_priv {
    VSSpeechSynthesizerNavit *speech;
    struct navit* navit;
};

static int speech_iphone_say(struct speech_priv *this, const char *text) {
    dbg(lvl_debug,"enter %s",text);
    char* lang;

    NSString *s=[[NSString alloc]initWithUTF8String: text];

    lang = navit_get_locallanguage(this->navit);

    [this->speech setCountryLanguage:lang];

    [this->speech startSpeakingString:s];

    dbg(lvl_debug,"ok");
    return 1;
}

static void speech_iphone_destroy(struct speech_priv *this) {
    //[this->speech release];
    g_free(this);
}

static struct speech_methods speech_iphone_meth = {
    speech_iphone_destroy,
    speech_iphone_say,
};

static struct speech_priv *speech_iphone_new(struct speech_methods *meth, struct attr **attrs, struct attr *parent) {
#pragma unused(parent)
    struct speech_priv *this;
    struct attr *attr;

    *meth=speech_iphone_meth;
    this=g_new0(struct speech_priv,1);
    this->speech= [VSSpeechSynthesizerNavit alloc];
    [this->speech init];
    this->navit = parent->u.navit;
    dbg(lvl_debug,"this->speech=%p",this->speech);

    [this->speech setPitch:0.8];
    [this->speech setRate:0.5];
    [this->speech setVolume:0.8];

    NSLog(@"iOS version: %f", [[[UIDevice currentDevice] systemVersion] floatValue]);

    // With the attribute speech_use_hfp="1" user can force to use HFP always.
    // This will force background music to HFP as well.
    // If not set we set HFP, but will automatically switch to A2DP when background music is being played
    // A2DP will not stop your radio from playing when an announcement is spoken
    if ((attr=attr_search(attrs, attr_speech_use_hfp)))
        [this->speech useHFP:(int)attr->u.num force:YES];
    else
        [this->speech useHFP:YES force:NO]; // AUTO

    if ((attr=attr_search(attrs, attr_speech_hfp_delay)))
        [this->speech setHFPDelay: attr->u.num*0.001];
    else
        [this->speech setHFPDelay:DEFAULT_HFP_DELAY];

    [this->speech setLanguage:getenv("LANG")];

    return this;
}

void plugin_init(void) {
    dbg(lvl_debug,"enter");
    plugin_register_category_speech("iphone", speech_iphone_new);
}
