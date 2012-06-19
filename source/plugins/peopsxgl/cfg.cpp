/***************************************************************************
						   cfg.c  -  description
							 -------------------
	begin                : Sun Mar 08 2009
	copyright            : (C) 1999-2009 by Pete Bernert
	web                  : www.pbernert.com
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
// 2009/03/08 - Pete
// - generic cleanup for the Peops release
//
//*************************************************************************//

#define _IN_CFG

#include "stdafx.h"
#include "externals.h"
#include "cfg.h"



namespace xegpu {
	
	XEGPU_CONFIG peops_cfg;

	void ReadConfig(void) // read config (linux file)
	{
		peops_cfg.iResX = 1280;
		peops_cfg.iResY = 720;
		peops_cfg.bGteAccuracy = TRUE; //bGteAccuracy = TRUE;
		peops_cfg.bFullVRam = FALSE;

		//         0: None
		//         1: Standard - Glitches will happen
		//         2: Extended - Removes black borders
		//         3: Standard w/o Sprites - unfiltered 2D
		//         4: Extended w/o Sprites - unfiltered 2D
		//         5: Standard + smoothed Sprites
		//         6: Extended + smoothed Sprites
		peops_cfg.iFilterType = 6;

		peops_cfg.bUseFrameLimit = TRUE; //bUseFrameLimit = TRUE;
		peops_cfg.bUseFrameSkip = FALSE;
		peops_cfg.iFrameLimit = 2;
		peops_cfg.fFrameRate = 200.0f;
		peops_cfg.iOffscreenDrawing = 2; //2: Standard - OK for most games
		// bOpaquePass = FALSE;//TRUE
		peops_cfg.bOpaquePass = TRUE; //TRUE
		peops_cfg.iTexQuality = 4; // 4 - B8 G8 R8 A8 - Slightly faster with some cards | 3 - R8 G8 B8 A8 - Best colors, more ram needed
		peops_cfg.iUseMask = TRUE;
		peops_cfg.iZBufferDepth = 16;
		peops_cfg.bUseFastMdec = TRUE;
		peops_cfg.dwCfgFixes = 0;
		peops_cfg.bUseFixes = FALSE;

		//         0: Emulated vram - effects need FVP
		//         1: Black - Fast but no special effects
		//         2: Gfx card buffer - Can be slow
		//         3: Gfx card buffer & software - slow
		peops_cfg.iFrameTexType = 0;

		//        0: Emulated vram - OK for most games <- speed up !!
		//        1: Gfx card buffer reads
		//        2: Gfx card buffer moves
		//        3: Gfx card buffer reads & moves
		//        4: Full software drawing (FVP) <- slow !!
		peops_cfg.iFrameReadType = 0;

		if (peops_cfg.iFrameReadType == 4) peops_cfg.bFullVRam = TRUE;
		else peops_cfg.bFullVRam = FALSE;

		peops_cfg.bKeepRatio = FALSE;
		peops_cfg.iVRamSize = 256;
		peops_cfg.iTexGarbageCollection = 1;
		peops_cfg.iHiResTextures = 1; //0: None (standard) ,1: 2xSaI (much vram needed) ,2: Stretched (filtering needed)

		if (peops_cfg.iUseMask) peops_cfg.iZBufferDepth = 16; // set zbuffer depth
		else peops_cfg.iZBufferDepth = 0;


		/**
		 * ff7
		 */
		peops_cfg.bUseFixes = TRUE;
		peops_cfg.dwCfgFixes = 0x1; //f7 fixe
		if (peops_cfg.bUseFixes) peops_cfg.dwActFixes = peops_cfg.dwCfgFixes; // init game fix global

	}

}