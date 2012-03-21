/***************************************************************************
                            zn.c  -  description
                            --------------------
    begin                : Wed April 23 2004
    copyright            : (C) 2004 by Pete Bernert
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
// 2004/04/23 - Pete
// - added ZINC zn interface
//
//*************************************************************************//

#include "stdafx.h"
#include "xa.h"

#include "externals.h"
#include "freeze.h"


//*************************************************************************//
// global marker, if ZINC is running

int iZincEmu=0;
