#ifdef USE_GUI
#include <string.h>
#include "gui_debug.h"
#include "settings.h"
#include "psxcommon.h"

#define enabled_disabled {"enabled","disabled"}
#define disabled_enabled {"disabled","enabled"}

settings * c_emu_settings=NULL;

void settings_load() {
    if(c_emu_settings==NULL){
        TR;
        c_emu_settings = new settings();
        c_emu_settings->addEntry("Cpu")->addValue("Dynarec")->addValue("Interpreter");
        c_emu_settings->addEntry("Sio")->addValue("Enabled")->addValue("Disabled");
        c_emu_settings->addEntry("Mdec")->addValue("Enabled")->addValue("Disabled");
        c_emu_settings->addEntry("PsxAuto")->addValue("Enabled")->addValue("Disabled");
        c_emu_settings->addEntry("Cdda")->addValue("Enabled")->addValue("Disabled");
        c_emu_settings->addEntry("HLE")->addValue("Enabled")->addValue("Disabled");
        c_emu_settings->addEntry("SlowBoot")->addValue("Enabled")->addValue("Disabled");
        c_emu_settings->addEntry("Debug")->addValue("Enabled")->addValue("Disabled");
        c_emu_settings->addEntry("PsxOut")->addValue("Enabled")->addValue("Disabled");
        c_emu_settings->addEntry("SpuIrq")->addValue("Enabled")->addValue("Disabled");
        c_emu_settings->addEntry("RCntFix")->addValue("Enabled")->addValue("Disabled");
        c_emu_settings->addEntry("UseNet")->addValue("Enabled")->addValue("Disabled");
        c_emu_settings->addEntry("VSyncWA")->addValue("Enabled")->addValue("Disabled");
        TR;
    }
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