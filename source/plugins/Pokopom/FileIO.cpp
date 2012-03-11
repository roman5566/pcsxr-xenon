/*  Pokopom - Input Plugin for PSX/PS2 Emulators
 *  Copyright (C) 2012  KrossX
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <stdio.h>

#include "Settings.h"


extern _Settings settings[2];

bool SaveEntry(wchar_t * section, int sectionNumber, wchar_t * key, int value, wchar_t * filename) {
    return true;
}

int ReadEntry(wchar_t * section, int sectionNumber, wchar_t * key, wchar_t * filename) {
    return 0;
}

void INI_SaveSettings() {


}

void INI_LoadSettings() {
    settings[0].xinputPort = 0;
    settings[1].xinputPort = 1;
}