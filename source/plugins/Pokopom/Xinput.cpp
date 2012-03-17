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

#include "Controller.h"
#include "General.h"

#include <input/input.h>
#include <ppc/timebase.h>
//#pragma comment(lib, "Xinput.lib")

#include <math.h>
extern "C" void exit(int e);
const double root2 = 1.4142135623730950488016887242097;

unsigned short ConvertAnalog(int X, int Y, double deadzone) {
    double radius = sqrt((double) X * X + (double) Y * Y);

    if (deadzone > 0) {
        double const max = 32768.0; // 40201 real max radius
        deadzone = max * deadzone;

        double rX = X / radius, rY = Y / radius;

        radius = radius <= deadzone ? 0 : (radius - deadzone) * max / (max - deadzone);

        X = (int) (rX * radius);
        Y = (int) (rY * radius);
    }

    if (radius > 32768.0f) {
        X = (int) (X * root2);
        Y = (int) (Y * root2);
    }

    Y = 32767 - Y;
    X = X + 32767;

    X = X < 0 ? 0 : X > 0xFFFF ? 0xFFFF : X;
    Y = Y < 0 ? 0 : Y > 0xFFFF ? 0xFFFF : Y;

    X >>= 8;
    Y >>= 8;

    unsigned short result = (unsigned short) ((Y << 8) | X);

    return result;
}

extern "C" void enableCapture();

struct controller_data_s old[2];

void Controller::poll() {
    struct controller_data_s state;
    int result = get_controller_data(&state, settings.xinputPort);

    buttons = 0xFFFF;
    analogL = analogR = 0x7F7F;

    static int lastAnalogSwitch = 0;
    if (state.select && state.start) {
        if (mftb() - lastAnalogSwitch > 1000) {

            lastAnalogSwitch = mftb();
            analogSwitch = true;
        }
    }

#if 1

    static int reset_time = 0;
    if (state.logo) {
        if(old[settings.xinputPort].logo){
            reset_time++;
            if(reset_time>50){
                if (state.select){
                    exit(0);//return to xell
                }else{
                    reset_time = 0;
                    enableCapture();
                }
            }
        }
        else{
            reset_time = 0;
        }
    }
    old[settings.xinputPort] = state;
#endif

    //if(result == ERROR_SUCCESS)
    if (1) {
        buttons = 0;
        buttons |= (state.select ? 0 : 1) << 0x0; // Select
        buttons |= (state.s1_z ? 0 : 1) << 0x1; // L3
        buttons |= (state.s2_z ? 0 : 1) << 0x2; // R3
        buttons |= (state.start ? 0 : 1) << 0x3; // Start
        buttons |= (state.up ? 0 : 1) << 0x4; // Up
        buttons |= (state.right ? 0 : 1) << 0x5; // Right
        buttons |= (state.down ? 0 : 1) << 0x6; // Down
        buttons |= (state.left ? 0 : 1) << 0x7; // Left

        buttons |= (state.lt > 100 ? 0 : 1) << 0x8; // L2
        buttons |= (state.rt > 100 ? 0 : 1) << 0x9; // R2

        buttons |= (state.lb ? 0 : 1) << 0xA; // L1
        buttons |= (state.rb ? 0 : 1) << 0xB; // R1

        buttons |= (state.y ? 0 : 1) << 0xC; // Triangle
        buttons |= (state.b ? 0 : 1) << 0xD; // Circle
        buttons |= (state.a ? 0 : 1) << 0xE; // Cross
        buttons |= (state.x ? 0 : 1) << 0xF; // Square

        buttonsStick = buttons | 0x06;
        const int threshold = 16384;

        settings.axisValue[GP_AXIS_LY] = state.s1_y * (settings.axisInverted[GP_AXIS_LY] ? -1 : 1);
        settings.axisValue[GP_AXIS_LX] = state.s1_x * (settings.axisInverted[GP_AXIS_LX] ? -1 : 1);
        settings.axisValue[GP_AXIS_RY] = state.s2_y * (settings.axisInverted[GP_AXIS_RY] ? -1 : 1);
        settings.axisValue[GP_AXIS_RX] = state.s2_x * (settings.axisInverted[GP_AXIS_RX] ? -1 : 1);

        if (settings.axisValue[GP_AXIS_LY] > threshold) buttonsStick &= ~(1 << 0x4);
        if (settings.axisValue[GP_AXIS_LX] > threshold) buttonsStick &= ~(1 << 0x5);
        if (settings.axisValue[GP_AXIS_LY] < -threshold) buttonsStick &= ~(1 << 0x6);
        if (settings.axisValue[GP_AXIS_LX] < -threshold) buttonsStick &= ~(1 << 0x7);

        if (settings.axisValue[GP_AXIS_RY] > threshold) buttonsStick &= ~(1 << 0xC);
        if (settings.axisValue[GP_AXIS_RX] > threshold) buttonsStick &= ~(1 << 0xD);
        if (settings.axisValue[GP_AXIS_RY] < -threshold) buttonsStick &= ~(1 << 0xE);
        if (settings.axisValue[GP_AXIS_RX] < -threshold) buttonsStick &= ~(1 << 0xF);

        analogL = ConvertAnalog(settings.axisValue[GP_AXIS_LX], settings.axisValue[GP_AXIS_LY], settings.deadzone);
        analogR = ConvertAnalog(settings.axisValue[GP_AXIS_RX], settings.axisValue[GP_AXIS_RY], settings.deadzone);

        //printf("Pokopom: %4X %4X\n", analogL, analogR);
    } else
        gamepadPlugged = false;

    //Return
}

void Controller::Recheck() {
    //	XINPUT_STATE state;
    //	DWORD result = XInputGetState(settings.xinputPort, &state);
    //
    //	if(result == ERROR_SUCCESS) gamepadPlugged = true;
    gamepadPlugged = true;
}

inline int Clamp(double input) {
    unsigned int result = (unsigned int) input;
    result = result > 0xFFFF ? 0xFFFF : result;
    return (int) result;
}

void Controller::vibration(unsigned char smalldata, unsigned char bigdata) {
    return;
    //	XINPUT_STATE state;
    //	DWORD result = XInputGetState(settings.xinputPort, &state);

    //if(result == ERROR_SUCCESS)
    if (1) {
        //printf("Vibrate! [%X] [%X]\n", smalldata, bigdata);

        static int vib_r;
        static int vib_l;
        static uint64_t timerS = 0, timerB = 0;

        if (smalldata) {
            vib_r = Clamp(0xFFFF * settings.rumble);
            timerS = mftb();
        } else if (vib_r && mftb() - timerS > 150) {
            vib_r = 0;
        }

        /*
        3.637978807091713*^-11 +
        156.82454281087692 * x + -1.258165252213538 *  x^2 +
        0.006474549734772402 * x^3;
         */

        if (bigdata) {
            double broom = 0.006474549734772402 * pow(bigdata, 3.0) -
                    1.258165252213538 * pow(bigdata, 2.0) +
                    156.82454281087692 * bigdata +
                    3.637978807091713e-11;


            /*
            unsigned int broom = bigdata;

            if(bigdata <= 0x2C) broom *= 0x72;
            else if(bigdata <= 0x53) broom = 0x13C7 + bigdata * 0x24;
            else broom *= 0x205;
             */

            vib_l = Clamp(broom * settings.rumble);
            timerB = mftb();
        } else if (vib_l && mftb() - timerB > 150) {
            vib_l = 0;
        }

        /*

        vib.wRightMotorSpeed = smalldata == 0? 0 : 0xFFFF;
        vib.wLeftMotorSpeed = bigdata * 0x101;

        vib.wRightMotorSpeed = Clamp(vib.wRightMotorSpeed * settings.rumble);
        vib.wLeftMotorSpeed = Clamp(vib.wLeftMotorSpeed * settings.rumble);
         */

        //printf("Vibrate! [%X] [%X]\n", vib.wLeftMotorSpeed, vib.wRightMotorSpeed);


        //set_controller_rumble(settings.xinputPort, vib_l >> 8, vib_r >> 8);
    } else
        gamepadPlugged = false;
}

void XInputPaused(bool pewpew) {
    //    XInputEnable(!pewpew);
}