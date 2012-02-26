#if 0
#include <string.h>
#include "gui_debug.h"
#include "settings.h"
#include "psxcommon.h"

#define enabled_disabled {"enabled","disabled"}
#define disabled_enabled {"disabled","enabled"}

#if 0
enum gpu_s{
    dynarec,
    xa,
    sio,
    mdec,
    psxauto,
    cdda,
    hle,
    slowboot,
    debug,
    psxout,
    spuirq,
    rcntfix,
    usenet,
    vsyncwa
};

struct settings_emu emu_settings = {
    14,
    {
        {2,0, "Cpu", {"Dynarec","Interpreter"}},
        {2,0, "Xa", enabled_disabled},
        {2,0, "Sio", enabled_disabled},
        {2,0, "Mdec", enabled_disabled},
        {2,0, "PsxAuto", enabled_disabled},
        {2,0, "Cdda", enabled_disabled},
        {2,0, "HLE", enabled_disabled},
        {2,0, "SlowBoot", enabled_disabled},
        {2,0, "Debug", enabled_disabled},
        {2,0, "PsxOut", enabled_disabled},
        {2,0, "SpuIrq", enabled_disabled},
        {2,0, "RCntFix", enabled_disabled},
        {2,0, "UseNet", enabled_disabled},
        {2,0, "VSyncWA", enabled_disabled},
        
    }
};

struct settings_emu gpu_settings= {
    7,
    {
        {2,0, "Gte accuracy", disabled_enabled},
        {6,0, "Filter type", "None","Standard","Extended","Standard w/o Sprites","Extended w/o Sprites","Standard + smoothed Sprites","Extended + smoothed Sprites"},
        {2,1, "Frame limit", disabled_enabled},
        {2,0, "Frame skip", disabled_enabled},
        {2,1, "Opaque pass", disabled_enabled},
        {2,1, "Advenced blend", disabled_enabled},
        {2,1, "Use mask", disabled_enabled},
        
    }
};;
struct settings_emu spu_settings= {
    1,
    {
        {2,0, "SPUIrq wait", enabled_disabled},
        
    }
};;
#endif

settings c_emu_settings;

void settings_load() {
    TR;
    c_emu_settings.addEntry("Cpu")->addValue("Dynarec")->addValue("Interpreter");
    TR;
    c_emu_settings.addEntry("Sio")->addValue("Enabled")->addValue("Disabled");
    c_emu_settings.addEntry("Mdec")->addValue("Enabled")->addValue("Disabled");
    c_emu_settings.addEntry("PsxAuto")->addValue("Enabled")->addValue("Disabled");
    c_emu_settings.addEntry("Cdda")->addValue("Enabled")->addValue("Disabled");
    c_emu_settings.addEntry("HLE")->addValue("Enabled")->addValue("Disabled");
    c_emu_settings.addEntry("SlowBoot")->addValue("Enabled")->addValue("Disabled");
    c_emu_settings.addEntry("Debug")->addValue("Enabled")->addValue("Disabled");
    c_emu_settings.addEntry("PsxOut")->addValue("Enabled")->addValue("Disabled");
    c_emu_settings.addEntry("SpuIrq")->addValue("Enabled")->addValue("Disabled");
    c_emu_settings.addEntry("RCntFix")->addValue("Enabled")->addValue("Disabled");
    c_emu_settings.addEntry("UseNet")->addValue("Enabled")->addValue("Disabled");
    c_emu_settings.addEntry("VSyncWA")->addValue("Enabled")->addValue("Disabled");
    TR;
}

void settings_save() {

}

void settings_apply(){
    // apply emu settings
//    Config.Cdda = emu_settings.settings[cdda].value;
//    Config.Debug = emu_settings.settings[debug].value;
//    Config.HLE = emu_settings.settings[hle].value;
//    Config.Mdec = emu_settings.settings[mdec].value;
//    Config.PsxAuto = emu_settings.settings[psxauto].value;
//    Config.PsxOut = emu_settings.settings[psxout].value;
//    Config.RCntFix = emu_settings.settings[rcntfix].value;
//    Config.SlowBoot = emu_settings.settings[slowboot].value;
//    Config.SpuIrq = emu_settings.settings[spuirq].value;
//    Config.UseNet = emu_settings.settings[usenet].value;
//    Config.VSyncWA = emu_settings.settings[vsyncwa].value;
//    Config.Xa = emu_settings.settings[xa].value;
    
    
    
}
#endif