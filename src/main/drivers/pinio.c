/*
 * This file is part of Cleanflight and Betaflight.
 *
 * Cleanflight and Betaflight are free software. You can redistribute
 * this software and/or modify this software under the terms of the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * Cleanflight and Betaflight are distributed in the hope that they
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdint.h>

#include "platform.h"

#ifdef USE_PINIO

#include "build/debug.h"

#include "pg/pinio.h"

#include "drivers/io.h"

typedef struct pinioRuntime_s {
    IO_t io;
    bool inverted;
    bool input;
    bool state;
} pinioRuntime_t;

static pinioRuntime_t pinioRuntime[PINIO_COUNT];

void pinioInit(const pinioConfig_t *pinioConfig)
{
    for (int i = 0; i < PINIO_COUNT; i++) {
        IO_t io = IOGetByTag(pinioConfig->ioTag[i]);

        if (!io) {
            continue;
        }

        IOInit(io, OWNER_PINIO, RESOURCE_INDEX(i));

        const bool inverted = pinioConfig->config[i] & PINIO_CONFIG_OUT_INVERTED;
        bool input = false;

        switch (pinioConfig->config[i] & PINIO_CONFIG_MODE_MASK) {
        case PINIO_CONFIG_MODE_OUT_PP:
            // Initial state after reset is input, pull-up.
            // Avoid momentary off by presetting the output to hi.
            if (inverted) {
                IOHi(io);
            }
            IOConfigGPIO(io, IOCFG_OUT_PP);
            break;
        case PINIO_CONFIG_MODE_IN_FLOATING:
            IOConfigGPIO(io, IOCFG_IN_FLOATING);
            input = true;
            break;
        case PINIO_CONFIG_MODE_IN_PU:
            IOConfigGPIO(io, IOCFG_IPU);
            input = true;
            break;
        case PINIO_CONFIG_MODE_IN_PD:
            IOConfigGPIO(io, IOCFG_IPD);
            input = true;
            break;
        }

        pinioRuntime[i].inverted = inverted;
        pinioRuntime[i].input = input;
        pinioRuntime[i].state = inverted;
        // Never drive an input; the output latch can select the pull direction on some MCUs
        if (!input) {
            IOWrite(io, inverted);
        }
        pinioRuntime[i].io = io;
    }
}

void pinioSet(int index, bool on)
{
    if (index < 0 || index >= PINIO_COUNT || pinioRuntime[index].input) {
        return;
    }
    const bool newState = on ^ pinioRuntime[index].inverted;
    if (newState != pinioRuntime[index].state) {
        IOWrite(pinioRuntime[index].io, newState);
        pinioRuntime[index].state = newState;
    }
}

bool pinioIsInput(int index)
{
    return index >= 0 && index < PINIO_COUNT && pinioRuntime[index].io && pinioRuntime[index].input;
}

// Logical level of the pin, with PINIO_CONFIG_OUT_INVERTED applied
bool pinioGet(int index)
{
    if (index < 0 || index >= PINIO_COUNT || !pinioRuntime[index].io) {
        return false;
    }

    return IORead(pinioRuntime[index].io) ^ pinioRuntime[index].inverted;
}
#endif
