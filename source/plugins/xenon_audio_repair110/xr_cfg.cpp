/***************************************************************************
cfg.c  -  description
-------------------
begin                : Wed May 15 2002
copyright            : (C) 2002 by Pete Bernert
email                : BlackDove@addcom.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version. See also the license.txt file for *
 *   additional informations.                                              *
 *                                                                         *
 ***************************************************************************/

//*************************************************************************//
// History of changes:
//
// 2003/06/07 - Pete
// - added Linux NOTHREADLIB define
//
// 2003/02/28 - Pete
// - added option for kode54's interpolation and linuzappz's mono mode
//
// 2003/01/19 - Pete
// - added Neill's reverb
//
// 2002/08/04 - Pete
// - small linux bug fix: now the cfg file can be in the main emu directory as well
//
// 2002/06/08 - linuzappz
// - Added combo str for SPUasync, and MAXMODE is now defined as 2
//
// 2002/05/15 - Pete
// - generic cleanup for the Peops release
//
//*************************************************************************//

#include "stdafx.h"

#define _IN_CFG

#include "externals.h"

#include <unistd.h>

extern int iZincEmu;



char * pConfigFile = NULL;



#include "../../main/gui.h"

////////////////////////////////////////////////////////////////////////
// START EXTERNAL CFG TOOL
////////////////////////////////////////////////////////////////////////

void StartCfgTool(char * pCmdLine) {

}

#define INI_VERSION "110"
#define INI_VERSION_EXPERT "110_2"

/////////////////////////////////////////////////////////
// READ CONFIG called by spu funcs
/////////////////////////////////////////////////////////

void ReadConfig(void) {
    FILE *fp;
    int reset;
    char ini_version[256];


    reset = 1;


    fp = fopen("uda:/plugins/spuPeopsSound.ini", "r");
    if (!fp) fp = fopen("uda:/spuPeopsSound.ini", "r");


    if (fp) {
        fscanf(fp, "Version = %s\n", &ini_version);

        if (strcmp(ini_version, INI_VERSION) == 0) {
            fscanf(fp, "SoundDriver = %d\n", &iOutputDriver);
            fscanf(fp, "Timer = %d\n", &iUseTimer);
            fscanf(fp, "Volume = %d\n", &iVolume);
            fscanf(fp, "Interpolation = %d\n", &iUseInterpolation);
            fscanf(fp, "Reverb = %d\n", &iUseReverb);
            fscanf(fp, "Mono = %d\n", &iDisStereo);
            fscanf(fp, "UseXA = %d\n", &iUseXA);
            fscanf(fp, "XAPitch = %d\n", &iXAPitch);
            fscanf(fp, "IRQWait = %d\n", &iSPUIRQWait);
            fscanf(fp, "Debugger = %d\n", &iDebugMode);
            fscanf(fp, "Recorder = %d\n", &iRecordMode);
            fscanf(fp, "EmuType = %d\n", &iEmuType);
            fscanf(fp, "ReverbBoost = %d\n", &iReverbBoost);

            fscanf(fp, "Latency = %d\n", &iLatency);
            fscanf(fp, "XAFilter = %d\n", &iXAInterp);
            fscanf(fp, "CDDAFilter = %d\n", &iCDDAInterp);
            fscanf(fp, "Output1Filter = %d\n", &iOutputInterp1);
            fscanf(fp, "Output2Filter = %d\n", &iOutputInterp2);
            fscanf(fp, "XAVol = %d\n", &iVolXA);
            fscanf(fp, "CDDAVol = %d\n", &iVolCDDA);
            fscanf(fp, "VoiceVol = %d\n", &iVolVoices);
            fscanf(fp, "XAStrength = %d\n", &iXAStrength);
            fscanf(fp, "CDDAStrength = %d\n", &iCDDAStrength);
            fscanf(fp, "Output2Strength = %d\n", &iOutput2Strength);


            reset = 0;
        }

        fclose(fp);
    }


    if (reset) {
        // Defaults
        iUseXA = 1;
        iVolume = 10;
        iXAPitch = 0;
        iUseTimer = 3;
        iSPUIRQWait = 0;
        iDebugMode = 0;
        iRecordMode = 0;
        iUseReverb = 2;
        iUseInterpolation = 2;
        iDisStereo = 0;
        iEmuType = 0;
        iXAStrength = 0;
        iCDDAStrength = 0;
        iOutput2Strength = 0;
        iReleaseIrq = 0;
        iLatency = 8;

        iXAInterp = 2;
        iCDDAInterp = 1;
        iOutputInterp1 = 0;
        iOutputInterp2 = 0;
        iVolCDDA = 10;
        iVolXA = 10;
        iVolVoices = 10;


        fp = fopen("uda:/plugins/spuPeopsSound.ini", "w");
        if (!fp) fp = fopen("uda:/spuPeopsSound.ini", "w");


        if (fp) {
            fprintf(fp, "Version = %s\n", INI_VERSION);
            fprintf(fp, "SoundDriver = %d\n", iOutputDriver);
            fprintf(fp, "Timer = %d\n", iUseTimer);
            fprintf(fp, "Volume = %d\n", iVolume);
            fprintf(fp, "Interpolation = %d\n", iUseInterpolation);
            fprintf(fp, "Reverb = %d\n", iUseReverb);
            fprintf(fp, "Mono = %d\n", iDisStereo);
            fprintf(fp, "UseXA = %d\n", iUseXA);
            fprintf(fp, "XAPitch = %d\n", iXAPitch);
            fprintf(fp, "IRQWait = %d\n", iSPUIRQWait);
            fprintf(fp, "Debugger = %d\n", iDebugMode);
            fprintf(fp, "Recorder = %d\n", iRecordMode);
            fprintf(fp, "EmuType = %d\n", iEmuType);
            fprintf(fp, "ReverbBoost = %d\n", iReverbBoost);

            fprintf(fp, "Latency = %d\n", iLatency);
            fprintf(fp, "XAFilter = %d\n", iXAInterp);
            fprintf(fp, "CDDAFilter = %d\n", iCDDAInterp);
            fprintf(fp, "Output1Filter = %d\n", iOutputInterp1);
            fprintf(fp, "Output2Filter = %d\n", iOutputInterp2);
            fprintf(fp, "XAVol = %d\n", iVolXA);
            fprintf(fp, "CDDAVol = %d\n", iVolCDDA);
            fprintf(fp, "VoiceVol = %d\n", iVolVoices);
            fprintf(fp, "XAStrength = %d\n", iXAStrength);
            fprintf(fp, "CDDAStrength = %d\n", iCDDAStrength);
            fprintf(fp, "Output2Strength = %d\n", iOutput2Strength);

            fclose(fp);
        }
    }


    latency_target = (iLatency + 1) * 10;

    // =================================
    // =================================
    // =================================

    // set defaults here
    reverb_target = 33;
    if (iReverbBoost) reverb_target = 38;

    latency_restart = 15;
    sound_stretcher = 0;

    debug_sound_buffer = 0;
    debug_cdxa_buffer = 0;

    phantom_padder = 0;
    phantom_pad_size = 50;
    phantom_post_pad = 500;
    APU_run = 45;

    upload_timer = 25;
    upload_low_reset = 2;
    upload_high_full = 30;
    upload_high_reset = 60;

    async_wait_block = 13;
    async_ondemand_block = 8;


    reset = 1;


    fp = fopen("uda:/plugins/spuPeopsSound_expert.ini", "r");
    if (!fp) fp = fopen("uda:/spuPeopsSound_expert.ini", "r");


    if (fp) {
        int temp;

        fscanf(fp, "Version = %s\n", &ini_version);

        if (strcmp(ini_version, INI_VERSION_EXPERT) == 0) {
            fscanf(fp, "LatencyTarget = %d\n", &temp);
            if (temp != -1) latency_target = temp;

            fscanf(fp, "UploadTimer = %d\n", &upload_timer);
            fscanf(fp, "SoundStretcher = %d\n", &sound_stretcher);
            fscanf(fp, "\n");

            fscanf(fp, "APUcycles = %d\n", &APU_run);
            fscanf(fp, "PhantomPad = %d\n", &phantom_padder);
            fscanf(fp, "PhantomPadSize = %d\n", &phantom_pad_size);
            fscanf(fp, "PhantomPostPad = %d\n", &phantom_post_pad);
            fscanf(fp, "LatencyRestart = %d\n", &latency_restart);
            fscanf(fp, "UploadLowReset = %d\n", &upload_low_reset);
            fscanf(fp, "UploadHighFull = %d\n", &upload_high_full);
            fscanf(fp, "UploadHighReset = %d\n", &upload_high_reset);

            fscanf(fp, "ReverbTarget = %d\n", &temp);
            if (temp != -1) reverb_target = temp;

            fscanf(fp, "AsyncWaitBlocker = %d\n", &async_wait_block);
            fscanf(fp, "AsyncOndemandBlocker = %d\n", &async_ondemand_block);
            fscanf(fp, "DebugSoundBuffer = %d\n", &debug_sound_buffer);
            fscanf(fp, "DebugCdxaBuffer = %d\n", &debug_cdxa_buffer);


            reset = 0;
        }
        fclose(fp);
    }


    if (reset) {
        // Defaults
        fp = fopen("uda:/plugins/spuPeopsSound_expert.ini", "w");
        if (!fp) fp = fopen("uda:/spuPeopsSound_expert.ini", "w");


        if (fp) {
            fprintf(fp, "Version = %s\n", INI_VERSION_EXPERT);
            fprintf(fp, "LatencyTarget = %d\n", -1);
            fprintf(fp, "UploadTimer = %d\n", upload_timer);
            fprintf(fp, "SoundStretcher = %d\n", sound_stretcher);
            fprintf(fp, "\n");

            fprintf(fp, "APUcycles = %d\n", APU_run);
            fprintf(fp, "PhantomPad = %d\n", phantom_padder);
            fprintf(fp, "PhantomPadSize = %d\n", phantom_pad_size);
            fprintf(fp, "PhantomPostPad = %d\n", phantom_post_pad);
            fprintf(fp, "LatencyRestart = %d\n", latency_restart);
            fprintf(fp, "UploadLowReset = %d\n", upload_low_reset);
            fprintf(fp, "UploadHighFull = %d\n", upload_high_full);
            fprintf(fp, "UploadHighReset = %d\n", upload_high_reset);
            fprintf(fp, "ReverbTarget = %d\n", -1);
            fprintf(fp, "AsyncWaitBlocker = %d\n", async_wait_block);
            fprintf(fp, "AsyncOndemandBlocker = %d\n", async_ondemand_block);
            fprintf(fp, "DebugSoundBuffer = %d\n", debug_sound_buffer);
            fprintf(fp, "DebugCdxaBuffer = %d\n", debug_cdxa_buffer);

            fclose(fp);
        }
    }


    // clip limit
    if (APU_run > 50) APU_run = 50;

    // =================================
    // =================================
    // =================================

    if (iZincEmu) {
        // don't do this - creates problems
        //iVolume=1;    // with ZINC, max volume is needed (or qsound will be too loud)
        iUseTimer = 1; // with ZINC, only timer mode is possible
        iDebugMode = 0; // with ZINC, no debug mode possible (we don't get SPUasyncs)
        iDisStereo = 0; // with ZINC, no mono possible (or qsound mixing troubles)
        iRecordMode = 0; // with ZINC, no debug mode possible (we don't get SPUasyncs)
        phantom_padder = 0;
    }



    if (iUseTimer > 4) iUseTimer = 4; // some checks
    if (iVolume < 0) iVolume = 0;
    if (iVolume > 20) iVolume = 20;


    switch (iEmuType) {
            // Generic NTSC
        case 0:
            cpu_clock = 33868800;
            break;


            // Generic PAL
        case 1:
            cpu_clock = 33864900;
            break;
    }


    // fast async drain problem
    if (iUseTimer >= 2) {
        /*
        Novastorm - lowest value allowed
        - keeps intro cutscene in sync (see video playback)

                10ms run speed (ePSXe 170 shark)
         */

        // need drainage for async methods
        cpu_clock -= 200000;
    }


    // always enable
    iUseDBufIrq = 1;



//    iXAPitch=SpuConfig.change_xa_speed;
//    iSPUIRQWait=SpuConfig.irq_wait;
//    iUseXA=SpuConfig.enable_xa;
}

