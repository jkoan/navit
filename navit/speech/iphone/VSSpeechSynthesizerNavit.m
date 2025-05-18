//
//  VSSpeechSynthesizer.m
//  speech_iphone
//
//  Created by olf on 27.05.21.
//

#import <Foundation/Foundation.h>
#include "VSSpeechSynthesizerNavit.h"
#include "navit.h"
#import <AVFoundation/AVFoundation.h>
#import <NaturalLanguage/NLLanguageRecognizer.h>

static AVSpeechUtterance *utterance;
static AVSpeechSynthesizer *synth;
static AVAudioSession *session;
static AVSpeechSynthesisVoice *voice;
static NSMutableArray<NSString*> *anmnts;
static unsigned long idx;

float volume;
float rate;
float pitch;
int use_hfp;
int force_hfp;
int use_mix_to_telephony_uplink;
int current_is_hfp;
int interrupted;
double hfpdelay;
char* lang;
char* countrylang;

char *nls_table[][3]= {
    // NLS Table compiled by Nick "Number6" Geoghegan
    // Not an exhaustive list, but supports 99% of all languages in Windows
    //{"LANGNAME", "CTRYNAME", "Language Code"},
    {"AFK", "ZAF", "af_ZA"},    // Afrikaans (South Africa)
    {"SQI", "ALB", "sq_AL"},    // Albanian (Albania)
    {"AMH", "ETH", "am_ET"},    // Amharic (Ethiopia)
    {"ARG", "DZA", "ar_DZ"},    // Arabic (Algeria)
    {"ARH", "BHR", "ar_BH"},    // Arabic (Bahrain)
    {"ARE", "EGY", "ar_EG"},    // Arabic (Egypt)
    {"ARI", "IRQ", "ar_IQ"},    // Arabic (Iraq)
    {"ARJ", "JOR", "ar_JO"},    // Arabic (Jordan)
    {"ARK", "KWT", "ar_KW"},    // Arabic (Kuwait)
    {"ARB", "LBN", "ar_LB"},    // Arabic (Lebanon)
    {"ARL", "LBY", "ar_LY"},    // Arabic (Libya)
    {"ARM", "MAR", "ar_MA"},    // Arabic (Morocco)
    {"ARO", "OMN", "ar_OM"},    // Arabic (Oman)
    {"ARQ", "QAT", "ar_QA"},    // Arabic (Qatar)
    {"ARA", "SAU", "ar_SA"},    // Arabic (Saudi Arabia)
    {"ARS", "SYR", "ar_SY"},    // Arabic (Syria)
    {"ART", "TUN", "ar_TN"},    // Arabic (Tunisia)
    {"ARU", "ARE", "ar_AE"},    // Arabic (U.A.E.)
    {"ARY", "YEM", "ar_YE"},    // Arabic (Yemen)
    {"HYE", "ARM", "hy_AM"},    // Armenian (Armenia)
    {"ASM", "IND", "as_IN"},    // Assamese (India)
    {"BAS", "RUS", "ba_RU"},    // Bashkir (Russia)
    {"EUQ", "ESP", "es_ES"},    // Basque (Basque)
    {"BEL", "BLR", "be_BY"},    // Belarusian (Belarus)
    {"BNG", "BDG", "bn_BD"},    // Bengali (Bangladesh)
    {"BNG", "IND", "bn_IN"},    // Bengali (India)
    {"BRE", "FRA", "br_FR"},    // Breton (France)
    {"BGR", "BGR", "bg_BG"},    // Bulgarian (Bulgaria)
    {"CAT", "ESP", "es_ES"},    // Catalan (Catalan)
    {"ZHH", "HKG", "zh_HK"},    // Chinese (Hong Kong S.A.R.)
    {"ZHM", "MCO", "zh_MO"},    // Chinese (Macao S.A.R.)
    {"CHS", "CHN", "zh_CN"},    // Chinese (People's Republic of China)
    {"ZHI", "SGP", "zh_SG"},    // Chinese (Singapore)
    {"CHT", "TWN", "zh_TW"},    // Chinese (Taiwan)
    {"COS", "FRA", "co_FR"},    // Corsican (France)
    {"HRV", "HRV", "hr_HR"},    // Croatian (Croatia)
    {"HRB", "BIH", "hr_BA"},    // Croatian (Latin, Bosnia and Herzegovina)
    {"CSY", "CZE", "cs_CZ"},    // Czech (Czech Republic)
    {"DAN", "DNK", "da_DK"},    // Danish (Denmark)
    {"NLB", "BEL", "nl_BE"},    // Dutch (Belgium)
    {"NLD", "NLD", "nl_NL"},    // Dutch (Netherlands)
    {"ENA", "AUS", "en_AU"},    // English (Australia)
    {"ENL", "BLZ", "en_BZ"},    // English (Belize)
    {"ENC", "CAN", "en_CA"},    // English (Canada)
    {"ENB", "CAR", "en_CB"},    // English (Caribbean)
    {"ENN", "IND", "en_IN"},    // English (India)
    {"ENI", "IRL", "en_IE"},    // English (Ireland)
    {"ENJ", "JAM", "en_JM"},    // English (Jamaica)
    {"ENM", "MYS", "en_MY"},    // English (Malaysia)
    {"ENZ", "NZL", "en_NZ"},    // English (New Zealand)
    {"ENP", "PHL", "en_PH"},    // English (Republic of the Philippines)
    {"ENE", "SGP", "en_SG"},    // English (Singapore)
    {"ENS", "ZAF", "en_ZA"},    // English (South Africa)
    {"ENT", "TTO", "en_TT"},    // English (Trinidad and Tobago)
    {"ENG", "GBR", "en_GB"},    // English (United Kingdom)
    {"ENU", "USA", "en_US"},    // English (United States)
    {"ENW", "ZWE", "en_ZW"},    // English (Zimbabwe)
    {"ETI", "EST", "et_EE"},    // Estonian (Estonia)
    {"FOS", "FRO", "fo_FO"},    // Faroese (Faroe Islands)
    {"FIN", "FIN", "fi_FI"},    // Finnish (Finland)
    {"FRB", "BEL", "fr_BE"},    // French (Belgium)
    {"FRC", "CAN", "fr_CA"},    // French (Canada)
    {"FRA", "FRA", "fr_FR"},    // French (France)
    {"FRL", "LUX", "fr_LU"},    // French (Luxembourg)
    {"FRM", "MCO", "fr_MC"},    // French (Principality of Monaco)
    {"FRS", "CHE", "fr_CH"},    // French (Switzerland)
    {"FYN", "NLD", "fy_NL"},    // Frisian (Netherlands)
    {"GLC", "ESP", "es_ES"},    // Galician (Galician)
    {"KAT", "GEO", "ka_GE"},    // Georgian (Georgia)
    {"DEA", "AUT", "de_AT"},    // German (Austria)
    {"DEU", "DEU", "de_DE"},    // German (Germany)
    {"DEC", "LIE", "de_LI"},    // German (Liechtenstein)
    {"DEL", "LUX", "de_LU"},    // German (Luxembourg)
    {"DES", "CHE", "de_CH"},    // German (Switzerland)
    {"ELL", "GRC", "el_GR"},    // Greek (Greece)
    {"KAL", "GRL", "kl_GL"},    // Greenlandic (Greenland)
    {"GUJ", "IND", "gu_IN"},    // Gujarati (India)
    {"HEB", "ISR", "he_IL"},    // Hebrew (Israel)
    {"HIN", "IND", "hi_IN"},    // Hindi (India)
    {"HUN", "HUN", "hu_HU"},    // Hungarian (Hungary)
    {"ISL", "ISL", "is_IS"},    // Icelandic (Iceland)
    {"IBO", "NGA", "ig_NG"},    // Igbo (Nigeria)
    {"IND", "IDN", "id_ID"},    // Indonesian (Indonesia)
    {"IRE", "IRL", "ga_IE"},    // Irish (Ireland)
    {"XHO", "ZAF", "xh_ZA"},    // isiXhosa (South Africa)
    {"ZUL", "ZAF", "zu_ZA"},    // isiZulu (South Africa)
    {"ITA", "ITA", "it_IT"},    // Italian (Italy)
    {"ITS", "CHE", "it_CH"},    // Italian (Switzerland)
    {"JPN", "JPN", "ja_JP"},    // Japanese (Japan)
    {"KDI", "IND", "kn_IN"},    // Kannada (India)
    {"KKZ", "KAZ", "kk_KZ"},    // Kazakh (Kazakhstan)
    {"KHM", "KHM", "km_KH"},    // Khmer (Cambodia)
    {"KIN", "RWA", "rw_RW"},    // Kinyarwanda (Rwanda)
    {"SWK", "KEN", "sw_KE"},    // Kiswahili (Kenya)
    {"KOR", "KOR", "ko_KR"},    // Korean (Korea)
    {"KYR", "KGZ", "ky_KG"},    // Kyrgyz (Kyrgyzstan)
    {"LAO", "LAO", "lo_LA"},    // Lao (Lao P.D.R.)
    {"LVI", "LVA", "lv_LV"},    // Latvian (Latvia)
    {"LTH", "LTU", "lt_LT"},    // Lithuanian (Lithuania)
    {"LBX", "LUX", "lb_LU"},    // Luxembourgish (Luxembourg)
    {"MKI", "MKD", "mk_MK"},    // Macedonian (Former Yugoslav Republic of Macedonia)
    {"MSB", "BRN", "ms_BN"},    // Malay (Brunei Darussalam)
    {"MSL", "MYS", "ms_MY"},    // Malay (Malaysia)
    {"MYM", "IND", "ml_IN"},    // Malayalam (India)
    {"MLT", "MLT", "mt_MT"},    // Maltese (Malta)
    {"MRI", "NZL", "mi_NZ"},    // Maori (New Zealand)
    {"MAR", "IND", "mr_IN"},    // Marathi (India)
    {"MON", "MNG", "mn_MN"},    // Mongolian (Cyrillic, Mongolia)
    {"NEP", "NEP", "ne_NP"},    // Nepali (Nepal)
    {"NOR", "NOR", "nb_NO"},    // Norwegian, Bokmå(Norway)
    {"NON", "NOR", "nn_NO"},    // Norwegian, Nynorsk (Norway)
    {"OCI", "FRA", "oc_FR"},    // Occitan (France)
    {"ORI", "IND", "or_IN"},    // Oriya (India)
    {"PAS", "AFG", "ps_AF"},    // Pashto (Afghanistan)
    {"FAR", "IRN", "fa_IR"},    // Persian
    {"PLK", "POL", "pl_PL"},    // Polish (Poland)
    {"PTB", "BRA", "pt_BR"},    // Portuguese (Brazil)
    {"PTG", "PRT", "pt_PT"},    // Portuguese (Portugal)
    {"PAN", "IND", "pa_IN"},    // Punjabi (India)
    {"ROM", "ROM", "ro_RO"},    // Romanian (Romania)
    {"RMC", "CHE", "rm_CH"},    // Romansh (Switzerland)
    {"RUS", "RUS", "ru_RU"},    // Russian (Russia)
    {"SMG", "FIN", "se_FI"},    // Sami, Northern (Finland)
    {"SME", "NOR", "se_NO"},    // Sami, Northern (Norway)
    {"SMF", "SWE", "se_SE"},    // Sami, Northern (Sweden)
    {"SAN", "IND", "sa_IN"},    // Sanskrit (India)
    {"TSN", "ZAF", "tn_ZA"},    // Setswana (South Africa)
    {"SIN", "LKA", "si_LK"},    // Sinhala (Sri Lanka)
    {"SKY", "SVK", "sk_SK"},    // Slovak (Slovakia)
    {"SLV", "SVN", "sl_SI"},    // Slovenian (Slovenia)
    {"ESS", "ARG", "es_AR"},    // Spanish (Argentina)
    {"ESB", "BOL", "es_BO"},    // Spanish (Bolivia)
    {"ESL", "CHL", "es_CL"},    // Spanish (Chile)
    {"ESO", "COL", "es_CO"},    // Spanish (Colombia)
    {"ESC", "CRI", "es_CR"},    // Spanish (Costa Rica)
    {"ESD", "DOM", "es_DO"},    // Spanish (Dominican Republic)
    {"ESF", "ECU", "es_EC"},    // Spanish (Ecuador)
    {"ESE", "SLV", "es_SV"},    // Spanish (El Salvador)
    {"ESG", "GTM", "es_GT"},    // Spanish (Guatemala)
    {"ESH", "HND", "es_HN"},    // Spanish (Honduras)
    {"ESM", "MEX", "es_MX"},    // Spanish (Mexico)
    {"ESI", "NIC", "es_NI"},    // Spanish (Nicaragua)
    {"ESA", "PAN", "es_PA"},    // Spanish (Panama)
    {"ESZ", "PRY", "es_PY"},    // Spanish (Paraguay)
    {"ESR", "PER", "es_PE"},    // Spanish (Peru)
    {"ESU", "PRI", "es_PR"},    // Spanish (Puerto Rico)
    {"ESN", "ESP", "es_ES"},    // Spanish (Spain)
    {"EST", "USA", "es_US"},    // Spanish (United States)
    {"ESY", "URY", "es_UY"},    // Spanish (Uruguay)
    {"ESV", "VEN", "es_VE"},    // Spanish (Venezuela)
    {"SVF", "FIN", "sv_FI"},    // Swedish (Finland)
    {"SVE", "SWE", "sv_SE"},    // Swedish (Sweden)
    {"TAM", "IND", "ta_IN"},    // Tamil (India)
    {"TTT", "RUS", "tt_RU"},    // Tatar (Russia)
    {"TEL", "IND", "te_IN"},    // Telugu (India)
    {"THA", "THA", "th_TH"},    // Thai (Thailand)
    {"BOB", "CHN", "bo_CN"},    // Tibetan (PRC)
    {"TRK", "TUR", "tr_TR"},    // Turkish (Turkey)
    {"TUK", "TKM", "tk_TM"},    // Turkmen (Turkmenistan)
    {"UIG", "CHN", "ug_CN"},    // Uighur (PRC)
    {"UKR", "UKR", "uk_UA"},    // Ukrainian (Ukraine)
    {"URD", "PAK", "ur_PK"},    // Urdu (Islamic Republic of Pakistan)
    {"VIT", "VNM", "vi_VN"},    // Vietnamese (Vietnam)
    {"CYM", "GBR", "cy_GB"},    // Welsh (United Kingdom)
    {"WOL", "SEN", "wo_SN"},    // Wolof (Senegal)
    {"III", "CHN", "ii_CN"},    // Yi (PRC)
    {"YOR", "NGA", "yo_NG"},    // Yoruba (Nigeria)
    {NULL,NULL,NULL},        // Default - Can't find the language / Language not listed above
};


@implementation VSSpeechSynthesizerNavit

- (id)init {
    
    NSError *setCategoryErr = nil;
    bool result = false;
    self = [super init];
    
    if (self) {
        synth = [[AVSpeechSynthesizer alloc] init];
        session = [AVAudioSession sharedInstance];
        NSArray<AVSpeechSynthesisVoice *> *voices = [AVSpeechSynthesisVoice speechVoices];
        voice = [voices firstObject];
        [session setMode:AVAudioSessionModeDefault error:(&setCategoryErr)];
        result = [session setCategory:AVAudioSessionCategoryPlayback withOptions:
                  AVAudioSessionCategoryOptionDuckOthers | AVAudioSessionCategoryOptionInterruptSpokenAudioAndMixWithOthers error:&setCategoryErr];
        
        if (@available(iOS 12.0, *)) {
            result = [session setMode:(AVAudioSessionModeVoicePrompt) error:(&setCategoryErr)]; // Failed to set mode: -50
        }   else {
            result = [session setMode:(AVAudioSessionModeDefault) error:(&setCategoryErr)];
        }
        
        NSLog (@"AVAudioSession initialized: %@ Result: %i", setCategoryErr.localizedFailureReason==NULL?@"OK":setCategoryErr.localizedFailureReason, result);
        use_hfp = YES;
        force_hfp = NO;
        current_is_hfp = NO;
        lang="";
        synth.delegate = self;
        
        NSNotificationCenter *notficationcenter = NSNotificationCenter.defaultCenter;
        [notficationcenter addObserver:self selector:@selector(handleInterruption:) name:
         AVAudioSessionInterruptionNotification object: session];
        [notficationcenter addObserver:self selector:@selector(handleRouteChange:) name:
         AVAudioSessionRouteChangeNotification object: session];
        
    }
    return self;
}

- (BOOL)isInterrupted {
    return interrupted;
}

- (void) handleInterruption:(NSNotification*)note {
    AVAudioSessionInterruptionType interruptionType = [[[note userInfo]
                                                        objectForKey:AVAudioSessionInterruptionTypeKey] unsignedIntegerValue];
    if (AVAudioSessionInterruptionTypeBegan == interruptionType) {
        NSLog (@"AVAudioSession interrupted");
        [synth stopSpeakingAtBoundary:(AVSpeechBoundaryWord)];
        utterance=nil;
    } else if (AVAudioSessionInterruptionTypeEnded == interruptionType) {
        NSLog (@"AVAudioSession interruption ended");
    }
}

- (void) handleRouteChange:(NSNotification*)note {
    UInt8 reasonValue = [[note.userInfo valueForKey:AVAudioSessionRouteChangeReasonKey] intValue];
    //AVAudioSessionRouteDescription *routeDescription = [note.userInfo valueForKey:AVAudioSessionRouteChangePreviousRouteKey];
    //NSLog(@"Route change: %i", reasonValue);
    switch (reasonValue) {
        case AVAudioSessionRouteChangeReasonNewDeviceAvailable:
            NSLog(@"     NewDeviceAvailable");
            //            [self useHFP:YES force:NO];
            break;
        case AVAudioSessionRouteChangeReasonOldDeviceUnavailable:
            NSLog(@"     OldDeviceUnavailable");
            //            [self useHFP:YES force:NO];
            break;
        case AVAudioSessionRouteChangeReasonCategoryChange:
            //NSLog(@"     CategoryChange");
            NSLog(@"     New Category: %@", [session category] );
            //[self useHFP:YES force:NO];
            break;
        case AVAudioSessionRouteChangeReasonOverride:
            NSLog(@"     Override");
            break;
        case AVAudioSessionRouteChangeReasonWakeFromSleep:
            NSLog(@"     WakeFromSleep");
            break;
        case AVAudioSessionRouteChangeReasonNoSuitableRouteForCategory:
            NSLog(@"     NoSuitableRouteForCategory");
            break;
        case AVAudioSessionRouteChangeReasonRouteConfigurationChange:
            NSLog(@"     RouteConfigurationChange");
            break;
        default:
            NSLog(@"     ReasonUnknown");
    }
    
    //    NSLog(@"Previous route:\n");
    //    NSLog(@"%@", routeDescription.outputs.firstObject);
    
}

+ (BOOL)isSystemSpeaking {
    return synth.speaking;
}

- (void)speechSynthesizer:(AVSpeechSynthesizer *)synthesizer
 didFinishSpeechUtterance:(AVSpeechUtterance *)utterance1 {
    NSLog(@"didFinishSpeechUtterance enter");
    utterance=nil;
    NSError *activationErr = nil;
    
    idx++;
    
    // Deactivate session
    if(idx < [anmnts count]) {
        utterance = [AVSpeechUtterance speechUtteranceWithString:[anmnts[idx] stringByReplacingOccurrencesOfString:@"@" withString:@""]];
       
        NSLog (@"AVSpeechUtterance String: %@", utterance.speechString);
        if([anmnts[idx] containsString:@"@"]){
            voice = [AVSpeechSynthesisVoice voiceWithLanguage: [NSString stringWithUTF8String: countrylang]];
            NSLog (@"Local Language: %s", countrylang);
        } else {
            voice = [AVSpeechSynthesisVoice voiceWithLanguage:[NSString stringWithUTF8String: lang]];
            NSLog (@"Default Language: %s", lang);
        }
        utterance.voice = voice;
        [utterance setPreUtteranceDelay:0.0];
        [utterance setPostUtteranceDelay:0.0];
        [synth speakUtterance:utterance];
    } else {
       
        [session setActive:false error:&activationErr];
    }
    NSLog(@"didFinishSpeechUtterance exit");
}

- (id)startSpeakingString:(id)string {
    NSLog (@"startSpeakingString enter");
    NSArray<NSString*> *myanmnts = [string componentsSeparatedByString:@"@@"]; // Split into maneuver and streetname by marker @@, so we can use local language to speak street names
    
    anmnts = [[NSMutableArray<NSString*> alloc]init];
    int cnt=0;
    for(NSString *cur in myanmnts) {
        cnt++;
        if(![cur isEqual:@""] && ![cur isEqual:@" "]) {
            if([anmnts count] > 0) {
                if(![[NSString stringWithFormat:@"%@%@", @"@",[cur copy]] isEqual:anmnts[[anmnts count]-1]]) { // only different strings
                    if(cnt%2 == 0) {
                        [anmnts addObject:[NSString stringWithFormat:@"%@%@", @"@",[cur copy]]];
                    } else {
                        [anmnts addObject:[cur copy]];
                    }
                }
            } else {
                [anmnts addObject:[cur copy]]; // First element
            }
        }
    }
    
    
    if(utterance != nil || !strcmp(lang,"") || [anmnts count]==0) {
        NSLog (@"AVSpeechUtterance skipped. utterance:%@, lang: %s, count: %lu", utterance, lang, [anmnts count]);
        return 0;
    }

    idx=0;
    utterance = [AVSpeechUtterance speechUtteranceWithString:anmnts[0]];
    
    NSError *activationErr = nil;
    
    [self useHFP:1 force:0];    // Checks before each announcement if there is background audio playing
    // If that is detected, we use A2DP and not HFP. force=2 will keep current
    // settings for force_hfp and useHFP.
    
    
    
    
    NSLog (@"AVSpeechUtterance String: %@", utterance.speechString);
    voice = [AVSpeechSynthesisVoice voiceWithLanguage:[NSString stringWithUTF8String: lang]];
    
    // We use a configurable delay (speech_hfp_delay in speech tag in navit.xml)
    // to give time to establish a HFP connection so the navigation
    // announcement will not get trunkated at the beginning
    if(current_is_hfp) {
        NSLog (@"AVSpeechUtterance delayed HFP by: %1.2fs %@", hfpdelay, activationErr.localizedFailureReason==NULL?@"OK":activationErr.localizedFailureReason);
        [utterance setPreUtteranceDelay:(hfpdelay)];
    }
    
    utterance.rate=rate;
    utterance.pitchMultiplier=pitch;
    utterance.volume=volume;
    utterance.voice = voice;
    
    //        if(!session.isOtherAudioPlaying || current_is_hfp)
    //            [session setActive:true error:&activationErr];
    [session setActive:true error: &activationErr];
    [synth speakUtterance:utterance];
    NSLog (@"AVSpeechUtterance play navigation announcement: %@ %@", utterance.speechString,
           activationErr.localizedFailureReason==NULL?@"OK":activationErr.localizedFailureReason);
    
    
    NSLog (@"startSpeakingString exit");
    return 0;
}

- (void) setHFPDelay:(double) delay {
    hfpdelay = delay;
    NSLog (@"setHFPDelay: %1.2f", delay);
}

- (id)startSpeakingString:(id)string toURL:(id)url {
    NSLog (@"startSpeakingString:(id)string toURL:(id)url");
    return 0;
}

- (id)startSpeakingString:(id)string toURL:(id)url withLanguageCode:(id)code {
    NSLog (@"startSpeakingString:(id)string toURL:(id)url withLanguageCode:(id)code");
    return 0;
}

- (float)rate {
    return rate;
}

- (id)setRate:(float)newrate {
    rate = newrate;
    return 0;
}

- (float)pitch {
    return pitch;
}

- (id)setPitch:(float)newpitch {
    pitch = newpitch;
    return 0;
}

- (float)volume {
    return volume;
}

- (id)setVolume:(float)newvolume {
    volume = newvolume;
    return 0;
}

- (id)useHFP:(int)newuse_hfp force:(int)force {
    
    NSError *setCategoryErr = nil;
    
    // Change only for NO and YES
    if(force < 2) {
        force_hfp = force;
        use_hfp = newuse_hfp;
    }
    
    // Get current options and route
    AVAudioSessionCategoryOptions options = [session categoryOptions];
    AVAudioSessionRouteDescription *activeRoute = [session currentRoute];
    bool btavailable=NO;
    
    for (AVAudioSessionPortDescription *output in activeRoute.outputs) {
        NSLog(@"PortType: %@", [output portType]);
        //AVAudioSessionPort port = [output portType];
        if (([[output portType] isEqualToString:AVAudioSessionPortBluetoothA2DP] ||
             [[output portType] isEqualToString:AVAudioSessionPortBluetoothHFP])) {
            btavailable = YES;
            //break;
        }
    }
    
    // Use Speaker when no Blutooth connection is available
    if(!btavailable) {
        if(!session.isOtherAudioPlaying) {
            // Use Speaker
            if(session.categoryOptions != (AVAudioSessionCategoryOptionDefaultToSpeaker |
                                           AVAudioSessionCategoryOptionDuckOthers) && ([[session availableCategories] containsObject:@"AVAudioSessionCategoryOptionDefaultToSpeaker" ])) {
                [session setCategory:AVAudioSessionCategoryPlayback withOptions:AVAudioSessionCategoryOptionDefaultToSpeaker |
                 AVAudioSessionCategoryOptionDuckOthers | AVAudioSessionCategoryOptionInterruptSpokenAudioAndMixWithOthers  error:&setCategoryErr];
                //[session setActive:true error:nil];
                NSLog(@"Configure AVAudioSession using Speaker: %@", setCategoryErr.localizedFailureReason==NULL?@"OK":setCategoryErr.localizedFailureReason);
            }
        } else {
            // Use current device playing audio already
            if(session.categoryOptions != (AVAudioSessionCategoryOptionDuckOthers)) {
                [session setCategory:AVAudioSessionCategoryPlayback withOptions:AVAudioSessionCategoryOptionDuckOthers | AVAudioSessionCategoryOptionInterruptSpokenAudioAndMixWithOthers   error:&setCategoryErr];
                //[session setActive:true error:nil];
                NSLog(@"Configure AVAudioSession using Speaker: %@", setCategoryErr.localizedFailureReason==NULL?@"OK":setCategoryErr.localizedFailureReason);
            }
        }
    } else {
        NSLog(@"isOtherAudioPlaying: %i", [session isOtherAudioPlaying]);
        if((use_hfp && force_hfp) || (((use_hfp || newuse_hfp) && !force_hfp) && (![session isOtherAudioPlaying]))) {
            // Force usage of HFP (only if no background music is playing) or if you explicily requested to use it
            // always by setting 'speech_use_hfp="1"' in speech tag in navit.xml:
            // radio gets muted while playing announcements,
            // but music playback in background would switch to HFP as well during announcement playback.
            
            current_is_hfp = YES;
            
            NSLog(@"Options %lu", (unsigned long)(options & (AVAudioSessionCategoryOptionAllowBluetooth | AVAudioSessionCategoryOptionDuckOthers)));
            options = [session categoryOptions];
            //            if(!(options & (AVAudioSessionCategoryOptionAllowBluetooth | AVAudioSessionCategoryOptionDuckOthers))) {
            [session setCategory:AVAudioSessionCategoryPlayAndRecord withOptions:AVAudioSessionCategoryOptionAllowBluetooth |
             AVAudioSessionCategoryOptionDuckOthers | AVAudioSessionCategoryOptionInterruptSpokenAudioAndMixWithOthers  error:&setCategoryErr];
            //[session setActive:true error:nil];
            NSLog(@"Configure AVAudioSession using HFP: %@", setCategoryErr.localizedFailureReason==NULL?@"OK":setCategoryErr.localizedFailureReason);
            //            }
            
            NSLog(@"AVAudioSession using HFP: %@", setCategoryErr.localizedFailureReason==NULL?@"OK":setCategoryErr.localizedFailureReason);
            
        } else {
            
            current_is_hfp = NO;
            options = [session categoryOptions];
            // Allow A2DP: No radio mute, but better voice quality
            //            if(!(options & (AVAudioSessionCategoryOptionAllowBluetoothA2DP |
            //                 AVAudioSessionCategoryOptionDuckOthers))) {
            [session setCategory:AVAudioSessionCategoryPlayback withOptions:
             AVAudioSessionCategoryOptionDuckOthers | AVAudioSessionCategoryOptionInterruptSpokenAudioAndMixWithOthers   error:&setCategoryErr];
            //[session setActive:true error:nil];
            NSLog(@"Configure AVAudioSession using A2DP: %@", setCategoryErr.localizedFailureReason==NULL?@"OK":setCategoryErr.localizedFailureReason);
            //            }
            
            NSLog(@"AVAudioSession using A2DP: %@", setCategoryErr.localizedFailureReason==NULL?@"OK":setCategoryErr.localizedFailureReason);
        }
        
        AVAudioSessionRouteDescription *routeDescription = [session currentRoute];
        NSLog(@"AVAudioSession current route: %@", routeDescription);
        
    }
    
    return 0;
}

- (BOOL)is_useHFP {
    return use_hfp;
}

- (void)setLanguage:(char *)language {
    lang = language;
}

- (void)setCountryLanguage:(char *)language {
    if(language==NULL)
        return;
    char* lang = NULL;
    int i = 0;
    while(nls_table[i][0] != NULL) {
        if(!strcmp(nls_table[i][0], language)) {
            lang = nls_table[i][2];
            break;
        }
        i++;
    }
    
    // e.g. Greek will only be found using nls_table[i][1], but we want to find e.g. fr_FR instead of br_FR for FRA
    i=0;
    if(lang==NULL) {
        while(nls_table[i][1] != NULL) {
            if(!strcmp(nls_table[i][1], language)) {
                lang = nls_table[i][2];
                break;
            }
            i++;
        }
    }
    
    if(lang)
        countrylang = lang;
}

- (BOOL)is_speaking {
    return utterance!=nil;
}

@end
