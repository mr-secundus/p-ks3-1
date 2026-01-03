// emac_app.cpp
// 
// Интерфейс к нижнему уровню управления EMAC.

#include "lwip/init.h"
#include "lwip/opt.h"
#include "lwip/sys.h"
#include "lwip/memp.h"
#include "lwip/tcpip.h"
#include "lwip/ip_addr.h"
#include "lwip/netif.h"
#include "lwip/timers.h"
#include "netif/etharp.h"

#if LWIP_DHCP
#include "lwip/dhcp.h"
#endif

#include "hal.h"
#include "lpc_phy.h"
#include "arch/lpc18xx_43xx_emac.h"
#include "arch/lpc_arch.h"
#include "debug.h"

#include "udp_server.h"

//-----------------------------------------------------------------------------
//                             Local macros
//-----------------------------------------------------------------------------

// Извлечение октетов из Uint32
#define U32B3(u)		(((u) >> 24) & 0xFF)
#define U32B2(u)		(((u) >> 16) & 0xFF)
#define U32B1(u)		(((u) >>  8) & 0xFF)
#define U32B0(u)		((u) & 0xFF)


//-----------------------------------------------------------------------------
//                               Static data
//-----------------------------------------------------------------------------

namespace emac_app
{
uint8_t macAddress[6];

struct 
{
	ip_addr_t ipaddr, netmask, gw;
} ipAddress;
	
struct netif lpc_netif;

uint32_t linkStatus;

bool printStatus;
};


//-----------------------------------------------------------------------------
//                             App interface
//-----------------------------------------------------------------------------

// Returns the MAC address assigned to this board
extern "C" void Board_ENET_GetMacADDR(uint8_t *mcaddr)
{
	for(uint32_t i = 0; i < sizeof(emac_app::macAddress); i++)
		mcaddr[i] = emac_app::macAddress[i];
}


//
void APP_PRINT_IP(char const* str, ip_addr_t* ipaddr)
{
#ifdef DEBUG_EMAC_APP
	static char tmp_buff[16];
	DEBUGOUT("%s : %s\r\n", str, ipaddr_ntoa_r((const ip_addr_t *)ipaddr, tmp_buff, 16));
//	puts(str);
//	puts(" : ");
//	puts(ipaddr_ntoa_r((const ip_addr_t *)ipaddr, tmp_buff, 16));
//	puts("\n");
#endif		
}


//-----------------------------------------------------------------------------
//                                 Public
//-----------------------------------------------------------------------------

namespace emac_app
{
//
void setMacAddress(uint32_t MAC_hi, uint32_t MAC_lo)
{
	macAddress[0] = U32B1(MAC_hi);
	macAddress[1] = U32B0(MAC_hi);
	macAddress[2] = U32B3(MAC_lo);
	macAddress[3] = U32B2(MAC_lo);
	macAddress[4] = U32B1(MAC_lo);
	macAddress[5] = U32B0(MAC_lo);
}


//
void setIpAddress(uint32_t ip, uint32_t gw, uint32_t mask)
{
	ipAddress.ipaddr.addr = ip;
	ipAddress.gw.addr = gw;
	ipAddress.netmask.addr = mask;
}


//
uint32_t getLinkStatus(void)
{
	return linkStatus;
}


//
void init(void)
{
#if LWIP_DHCP
	IP4_ADDR(&ipAddress.gw, 0, 0, 0, 0);
	IP4_ADDR(&ipAddress.ipaddr, 0, 0, 0, 0);
	IP4_ADDR(&ipAddress.netmask, 0, 0, 0, 0);
#else
//	IP4_ADDR(&ipAddress.gw,      192, 168, 0, 1);
//	IP4_ADDR(&ipAddress.ipaddr,  192, 168, 0, 136);
//	IP4_ADDR(&ipAddress.netmask, 255, 255, 255, 0);
//	APP_PRINT_IP("IP_ADDR", &ipaddr);
#endif

	linkStatus = 0;
	printStatus = false;

	lwip_init();

	netif_add(&lpc_netif, &ipAddress.ipaddr, &ipAddress.netmask, &ipAddress.gw, NULL,
						lpc_enetif_init, ethernet_input);
	netif_set_default(&lpc_netif);
	netif_set_up(&lpc_netif);

#if LWIP_DHCP
	dhcp_start(&lpc_netif);
#endif
}


// 
void process(void)
{
	lpc_enetif_input(&lpc_netif);

	/* lpc_rx_queue will re-qeueu receive buffers. This normally occurs
		 automatically, but in systems were memory is constrained, pbufs
		 may not always be able to get allocated, so this function can be
		 optionally enabled to re-queue receive buffers. */
#if 0
	while (lpc_rx_queue(&lpc_netif)) {}
#endif

	/* Free TX buffers that are done sending */
	lpc_tx_reclaim(&lpc_netif);
}


// 
void processTimers(void)
{
	/* LWIP timers - ARP, DHCP, TCP, etc. */
	sys_check_timeouts();
}


// return 		true	- link status changed
//						false	- no change
bool processLinkStatus(void)
{
	/* Call the PHY status update state machine once in a while to keep the link status up-to-date */
	uint32_t physts = lpcPHYStsPoll();

	/* Only check for connection state when the PHY status has changed */
	if(physts & PHY_LINK_CHANGED) 
	{
		if(physts & PHY_LINK_CONNECTED) 
		{
			/* Set interface speed and duplex */
			Chip_ENET_SetSpeed(LPC_ETHERNET, physts & PHY_LINK_SPEED100 ? 1 : 0);
			NETIF_INIT_SNMP(&lpc_netif, snmp_ifType_ethernet_csmacd, 100000000);
			
			Chip_ENET_SetDuplex(LPC_ETHERNET, physts & PHY_LINK_FULLDUPLX ? true : false);

			netif_set_link_up(&lpc_netif);
			linkStatus = 1;
			
#ifdef DEBUG_EMAC_APP
			printStatus = true;
#endif			
		}
		else 
		{
			netif_set_link_down(&lpc_netif);
			linkStatus = 0;
		}
	}
	
#ifdef DEBUG_EMAC_APP
	if(physts & PHY_LINK_CHANGED)
	{
		DEBUGOUT("Link connect status: %d\n", ((physts & PHY_LINK_CONNECTED) != 0));

//		if(printStatus  &&  lpc_netif.ip_addr.addr != 0) 
		if((physts & PHY_LINK_CONNECTED)  &&  lpc_netif.ip_addr.addr != 0) 
		{
			static char tmp_buff[16];
			DEBUGOUT("IP address : %s\n", ipaddr_ntoa_r((const ip_addr_t *) &lpc_netif.ip_addr, tmp_buff, 16));
			DEBUGOUT("Net mask   : %s\n", ipaddr_ntoa_r((const ip_addr_t *) &lpc_netif.netmask, tmp_buff, 16));
			DEBUGOUT("Gateway    : %s\n", ipaddr_ntoa_r((const ip_addr_t *) &lpc_netif.gw, tmp_buff, 16));
			printStatus = 0;
		}
	}
#endif

	return (physts & PHY_LINK_CHANGED) != 0 ? true : false; 
}

};
