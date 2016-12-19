#include <sys/socket.h>
#include <net/if.h>
#include <linux/sockios.h>
#include <linux/can.h>

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "canmgr_proto.h"
#include "canmgr_dump.h"
#include "lxcan.h"

static const uint8_t myaddr = 0; // lie

int main (int argc, char *argv[])
{
    struct canmgr_frame in, out;
    struct rawcan_frame raw;
    struct canmgr_hdr cons;
    int c, m, n;
    int s;
    char dump[80]; 

    if (argc != 2) {
        fprintf (stderr, "Usage: cancon c,m,n\n");
        exit (1);
    }
    if (sscanf (argv[1], "%d,%d,%d", &c, &m, &n) != 3
            || c < 0 || c >= 0x10 || m < 0 || m >= 0x10 || c < 0 || c >= 0x10) {
        fprintf (stderr, "improperly specified target\n");
        exit (1);
    }

    /* construct connect request
     * for now, no routing - cluster and module are ignored
     */
    in.id.pri = 1;
    in.id.dst = n | 0x10;
    in.id.src = myaddr;

    in.hdr.pri = 1;
    in.hdr.type = CANMGR_TYPE_WO;
    in.hdr.node = in.id.dst;
    in.hdr.module = m;
    in.hdr.cluster = c;
    in.hdr.object = CANOBJ_TARGET_CONSOLECONN;
    /* construct console object (payload)
     */
    cons.pri = 1;
    cons.type = CANMGR_TYPE_DAT;
    cons.node = myaddr;
    cons.module = 0;
    cons.cluster = 0;
    cons.object = 0;
    if (canmgr_encode_hdr (&cons, &in.data[0], 4) < 0) {
        fprintf (stderr, "error encoding console object\n");
        exit (1);
    }
    in.dlen = 4;

    /* encode raw frame
     */
    if (canmgr_encode (&in, &raw) < 0) {
        fprintf (stderr, "error encoding CAN frame\n");
        exit (1);
    }

    /* send frame on can0
     * wait for ack/nak response
     */
    if ((s = lxcan_open ("can0")) < 0) {
        fprintf (stderr, "lxcan_open: %m\n");
        exit (1);
    }
    canmgr_dump (&in, dump, sizeof (dump));
    printf ("%s\n", dump);
    if (lxcan_send (s, &raw) < 0) {
        fprintf (stderr, "lxcan_send: %m\n");
        exit (1);
    }
    // TODO timeout
    for (;;) {
        if (lxcan_recv (s, &raw) < 0) {
            fprintf (stderr, "lxcan_recv: %m\n");
            exit (1);
        }
        if (canmgr_decode (&out, &raw) < 0) {
            fprintf (stderr, "canmgr_decode error, ignoring packet\n");
            continue;
        }
        canmgr_dump (&out, dump, sizeof (dump));
        printf ("%s\n", dump);
        if (out.id.src != in.id.dst || out.id.dst != in.id.src)
            continue;
        if (out.hdr.object != CANOBJ_TARGET_POWER)
            continue;
        if (out.hdr.cluster != in.hdr.cluster
                || out.hdr.module != in.hdr.module
                || out.hdr.node != in.hdr.node)
            continue;
        if (out.hdr.type == CANMGR_TYPE_ACK)
            break; // got response
        if (out.hdr.type == CANMGR_TYPE_NAK) {
            fprintf (stderr, "Received NAK response to connect request\n");
            exit (1);
        }
    }
    for (;;) {
        if (lxcan_recv (s, &raw) < 0) {
            fprintf (stderr, "lxcan_recv: %m\n");
            exit (1);
        }
        if (canmgr_decode (&out, &raw) < 0) {
            fprintf (stderr, "canmgr_decode error, ignoring packet\n");
            continue;
        }
        
        canmgr_dump (&out, dump, sizeof (dump));
        printf ("%s\n", dump);
    }

    can_close (s);

    exit (0);
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */