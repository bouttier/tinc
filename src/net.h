/*
    net.h -- header for net.c
    Copyright (C) 1998,1999,2000 Ivo Timmermans <zarq@iname.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    $Id: net.h,v 1.9.4.7 2000/08/07 14:52:15 guus Exp $
*/

#ifndef __TINC_NET_H__
#define __TINC_NET_H__

#include <sys/time.h>

#include "config.h"
#include "conf.h"

#define MAXSIZE 1700  /* should be a bit more than the MTU for the tapdevice */
#define MTU 1600

#define MAC_ADDR_S "%02x:%02x:%02x:%02x:%02x:%02x"
#define MAC_ADDR_V(x) ((unsigned char*)&(x))[0],((unsigned char*)&(x))[1], \
                      ((unsigned char*)&(x))[2],((unsigned char*)&(x))[3], \
                      ((unsigned char*)&(x))[4],((unsigned char*)&(x))[5]

#define IP_ADDR_S "%d.%d.%d.%d"

#ifdef WORDS_BIGENDIAN
# define IP_ADDR_V(x) ((unsigned char*)&(x))[0],((unsigned char*)&(x))[1], \
                      ((unsigned char*)&(x))[2],((unsigned char*)&(x))[3]
#else
# define IP_ADDR_V(x) ((unsigned char*)&(x))[3],((unsigned char*)&(x))[2], \
                      ((unsigned char*)&(x))[1],((unsigned char*)&(x))[0]
#endif

#define MAXBUFSIZE 2048 /* Probably way too much, but it must fit every possible request. */

/* flags */
#define INDIRECTDATA        0x0001 /* Used to indicate that this host has to be reached indirect */
#define EXPORTINDIRECTDATA  0x0002 /* Used to indicate uplink that it has to tell others to do INDIRECTDATA */
#define TCPONLY             0x0004 /* Tells sender to send packets over TCP instead of UDP (for firewalls) */

typedef unsigned long ip_t;
typedef short length_t;

typedef struct vpn_packet_t {
  length_t len;		/* the actual number of bytes in the `data' field */
  unsigned char data[MAXSIZE];
} vpn_packet_t;

typedef struct real_packet_t {
  length_t len;		/* the length of the entire packet */
  ip_t from;		/* where the packet came from */
  vpn_packet_t data;	/* encrypted vpn_packet_t */
} real_packet_t;

typedef struct passphrase_t {
  unsigned short len;
  unsigned char *phrase;
} passphrase_t;

typedef struct status_bits_t {
  int pinged:1;                    /* sent ping */
  int got_pong:1;                  /* received pong */
  int meta:1;                      /* meta connection exists */
  int active:1;                    /* 1 if active.. */
  int outgoing:1;                  /* I myself asked for this conn */
  int termreq:1;                   /* the termination of this connection was requested */
  int remove:1;                    /* Set to 1 if you want this connection removed */
  int timeout:1;                   /* 1 if gotten timeout */
  int validkey:1;                  /* 1 if we currently have a valid key for him */
  int waitingforkey:1;             /* 1 if we already sent out a request */
  int dataopen:1;                  /* 1 if we have a valid UDP connection open */
  int unused:21;
} status_bits_t;

typedef struct queue_element_t {
  void *packet;
  struct queue_element_t *prev;
  struct queue_element_t *next;
} queue_element_t;

typedef struct packet_queue_t {
  queue_element_t *head;
  queue_element_t *tail;
} packet_queue_t;

typedef struct enc_key_t {
  int length;
  char *key;
  time_t expiry;
} enc_key_t;

typedef struct conn_list_t {
  ip_t vpn_ip;                     /* his vpn ip */
  ip_t vpn_mask;                   /* his vpn network address */
  ip_t real_ip;                    /* his real (internet) ip */
  char *real_hostname;             /* the hostname of its real ip */
  char *vpn_hostname;              /* the hostname of the vpn ip */
  short unsigned int port;         /* his portnumber */
  int flags;                       /* his flags */
  int socket;                      /* our udp vpn socket */
  int meta_socket;                 /* our tcp meta socket */
  int protocol_version;            /* used protocol */
  status_bits_t status;            /* status info */
  passphrase_t *pp;                /* encoded passphrase */
  packet_queue_t *sq;              /* pending outgoing packets */
  packet_queue_t *rq;              /* pending incoming packets (they have no
				      valid key to be decrypted with) */
  enc_key_t *public_key;           /* the other party's public key */
  enc_key_t *key;                  /* encrypt with this key */
  char buffer[MAXBUFSIZE+1];       /* metadata input buffer */
  int buflen;                      /* bytes read into buffer */
  int reqlen;                      /* length of first request in buffer */
  time_t last_ping_time;           /* last time we saw some activity from the other end */  
  int want_ping;                   /* 0 if there's no need to check for activity */
  struct conn_list_t *nexthop;     /* nearest meta-hop in this direction */
  struct conn_list_t *next;        /* after all, it's a list of connections */
} conn_list_t;

extern int tap_fd;

extern int total_tap_in;
extern int total_tap_out;
extern int total_socket_in;
extern int total_socket_out;

extern conn_list_t *conn_list;
extern conn_list_t *myself;

extern int send_packet(ip_t, vpn_packet_t *);
extern int setup_network_connections(void);
extern void close_network_connections(void);
extern void main_loop(void);
extern int setup_vpn_connection(conn_list_t *);
extern void terminate_connection(conn_list_t *);
extern void flush_queues(conn_list_t*);

#endif /* __TINC_NET_H__ */
