/*
 * Copyright (C) 2018 Sam88651.
 * 
 * Module Name:
 *     HaspInfo.c
 * Abstract:
 *     Utility to find and check USB HASP4 keys attached to the system
 * Notes:
 * Revision History:
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <linux/limits.h>
#include <unistd.h>
#include "hasp.h"

/*
 * Plug to support old libhasplnx.a
 */
int __ctype_toupper(int c) {
    return toupper(c);
}

/* Found keys descriptions  */
static Hasp4 keys [MAX_1C_KEYS];
/* Number of found keys     */
static int nKeys = sizeof(keys)/sizeof(keys[0]);
static char nethaspiniPath [PATH_MAX] = "/etc/hasplm/nethasp.ini";


int main(int argc, char *argv[]) {
    register int i, r;
    int     opt;

    while ((opt=getopt(argc,argv, "?hc:")) != -1) {
        switch (opt) {
        case 'c':
            strncpy(nethaspiniPath, (char *)optarg, sizeof(nethaspiniPath));
            nethaspiniPath[sizeof(nethaspiniPath)-1] = '\0';
            break;
        default:
        case '?':
        case 'h':
            fprintf (stderr,"Usage: #%s [-c] <nethasp.ini path>\n", argv[0]);
            return -1;
        }
    }
    
    r = enumKeys(keys, &nKeys, nethaspiniPath);
    if (!r) {
        if (!nKeys) {
            r = HASPERR_HASP_NOT_FOUND;
            printHaspErrorMessage(r);
        } else {
            printf (" %-6s  %-12s  %-8s  %-7s  %6s  %6s\n", "Series", "Model", "SN", "Loc/Net", "Used", "Total");
            printf ("%s\n", "---------------------------------------------------------");
            for (i = 0; i < nKeys; i++) {
                if (keys[i].pass1 == 0x3B6D && keys[i].pass2 == 0x70CB) {
                    printf (" %-6s", "ORGL8");
                } else if (keys[i].pass1 == 0x701D && keys[i].pass2 == 0x25D3) {
                    printf (" %-6s", "ORG8A");
                } else if (keys[i].pass1 == 0x53B1 && keys[i].pass2 == 0x6C33) {
                    printf (" %-6s", "ORG8B");
                } else if (keys[i].pass1 == 0x4125 && keys[i].pass2 == 0x237A) {
                    printf (" %-6s", "EN8SA");
                } else if (keys[i].pass1 == 0x4E31 && keys[i].pass2 == 0x10E4) {
                    printf (" %-6s", "ENSR8");
                } else {
                    printf (" %-6s", "UNKNOW");
                }
                printf ("  %-12s  %08lX  %-7s", getHaspType(keys[i].type),
                        keys[i].id, keys[i].licensesTotal==1?"local":"network");
                if (keys[i].licensesTotal > 1) {
                    printf("  % 6d", keys[i].licensesUsed);
                } else {
                    printf("  %-6s", "");
                }
                printf ("  % 6d\n", keys[i].licensesTotal);
            }
        }
    } else {
        printHaspErrorMessage(r);
    }
    printf ("\n");
    return(!r?NAGIOS_OK:NAGIOS_CRITICAL);
}

