// Copyright (C) 2016 Pit Kleyersburg <pitkley@googlemail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
// OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/extensions/dpms.h>
#include <ddccontrol/ddcci.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include "main.h"

volatile sig_atomic_t stop;
static struct monitors *mons = NULL;
static Display *dpy = NULL;

void sighand(int signum) {
    stop = 1;
}

int init_dpms() {
    dpy = XOpenDisplay(NULL);

    if (dpy == NULL) {
        return 1;
    }

    int i1, i2;
    if (!(DPMSQueryExtension(dpy, &i1, &i2) && DPMSCapable(dpy))) {
        return 1;
    }

    return 0;
}

Bool dpms_active() {
    BOOL state;
    CARD16 power_level;
    DPMSInfo(dpy, &power_level, &state);

    if (state) {
        switch (power_level) {
            case DPMSModeStandby:
            case DPMSModeSuspend:
            case DPMSModeOff:
                return True;
            case DPMSModeOn:
            default:
                return False;
        }
    } else {
        return False;
    }
}

void get_monitors() {
    struct monitorlist *monlist = ddcci_probe();
    struct monitorlist *current = monlist;

    int ret;

    struct monitors *headmon = NULL;
    struct monitors *currentmon = NULL;

    while (current) {
        currentmon = (struct monitors *) malloc(sizeof(struct monitors));
        ret /* TODO ret */ = ddcci_open(&currentmon->mon, current->filename, 0);
        currentmon->next = headmon;
        headmon = currentmon;

        current = current->next;
    }
    mons = currentmon;
}

void close_monitors() {
    struct monitors *currentmon = mons;
    while (currentmon) {
        ddcci_close(&currentmon->mon);
        currentmon = currentmon->next;
    }
    free(mons);
}

void check_dpms(Bool active) {
    struct monitors *current = mons;
    while (current) {
        unsigned short value;
        unsigned short maximum;
        /* TODO int ret */ ddcci_readctrl(&current->mon, DDCCI_CTRL_POWER, &value, &maximum);

#ifdef DEBUG
        printf("[check_dpms] mon %d, active %d, value: %d\n", current->mon.fd, active, value);
#endif
        switch (value) {
            case 1: // On
                if (active) {
#ifdef DEBUG
                    printf("[check_dpms] DPMS is active, monitor %d is on though. Setting to standby...\n", current->mon.fd);
#endif
                    ddcci_writectrl(&current->mon, DDCCI_CTRL_POWER, DDCCI_CTRL_POWER_STANDBY, 0);
                }
                break;
            case 2: // Standby
            case 3: // Suspend
            case 4: // Off
                if (!active) {
#ifdef DEBUG
                    printf("[check_dpms] DPMS is inactive, monitor %d is off though. Turning on...\n", current->mon.fd);
#endif
                    ddcci_writectrl(&current->mon, DDCCI_CTRL_POWER, DDCCI_CTRL_POWER_ON, 0);
                }
                break;
            case 0: // Reserved
            default:
                break;
        }

        current = current->next;
    }
}

void main() {
    signal(SIGTERM, sighand);
    signal(SIGINT, sighand);

    int ret = init_dpms();
    if (ret != 0) {
        fprintf(stderr, "Initializing DPMS extension failed");
    }

    ret = ddcci_init(NULL);
    if (ret == 0) {
        fprintf(stderr, "Initializing ddcci failed");
    }

    get_monitors();

    while (!stop) {
        sleep(2);

        check_dpms(dpms_active());
    }
    fprintf(stderr, "Received stop, cleaning up and exiting\n");

    close_monitors();
    ddcci_release();

    XCloseDisplay(dpy);
}
