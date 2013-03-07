/**
 *  Bluetooth keystroke logger (hopefully :])
 */


#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <pcap.h>
#include <pcap/pcap.h>
#include <pcap/bluetooth.h>
#include <sys/ioctl.h>
#include<net/if.h>
#include <netinet/ether.h>
#include <sys/socket.h>
#include <netpacket/packet.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "checksum.h"
#include "smartalloc.h"

#define ERRBUF_SIZE 1000

/*
 * This function handles raw packets.
 */
void packet_handler(u_char *args, const struct pcap_pkthdr *header,
	    const u_char *packet);

char* dev;

/**
 * @param argc Number of arguements.
 * @param argv Arguements on the command line.
 */
int
main(int argc, char *argv[])
{
    pcap_t* packets;
    char errbuf[ERRBUF_SIZE];
    struct bpf_program fp;
    char filter_exp[] = "arp";
    bpf_u_int32 net,mask;

    dev = pcap_lookupdev(errbuf);
    if (dev == NULL) {
        fprintf(stderr, "Couldn't find default device: %s\n", errbuf);
	return EXIT_FAILURE;
    }
    if (pcap_lookupnet(dev, &net, &mask, errbuf) == -1) {
	     fprintf(stderr, "Couldn't get netmask for device %s: %s\n", dev, errbuf);
	     net = 0;
	     mask = 0;
    }

    packets = pcap_open_live("Broadcom Corp.", 1518, 1, 1000, errbuf);
    if(packets == NULL)
    {
        fprintf(stderr, "PCAP OPEN_LIVE ERROR: %s\n", errbuf);
        return EXIT_FAILURE;
    }
    if (pcap_compile(packets, &fp, filter_exp, 0, net) == -1) 
    {
	     fprintf(stderr, "Couldn't parse filter %s: %s\n", filter_exp, pcap_geterr(packets));
	     return EXIT_FAILURE;
    }
    if (pcap_setfilter(packets, &fp) == -1) 
    {
        fprintf(stderr, "Couldn't install filter %s: %s\n", filter_exp, pcap_geterr(packets));
	     return EXIT_FAILURE;
    }
    pcap_loop(packets, 100000, packet_handler, NULL);

    pcap_close(packets);    
    return EXIT_SUCCESS;
}

/**
 * This function handles bluetooth packets after the filter.
 */
void 
packet_handler(u_char *args, const struct pcap_pkthdr *header, const u_char *packet)
{
   fprintf(stderr, "Bluetooth packet!\n");
}

/* vim: set et ai sts=2 sw=2: */
