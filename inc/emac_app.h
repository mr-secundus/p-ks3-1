#ifndef EMAC_APP_H
#define EMAC_APP_H

namespace emac_app
{
void setMacAddress(uint32_t MAC_hi, uint32_t MAC_lo);

void setIpAddress(uint32_t ip, uint32_t gw, uint32_t mask);

uint32_t getLinkStatus(void);

void init(void);

void process(void);

void processTimers(void);

// return 		true	- link status changed
//						false
bool processLinkStatus(void);
};

#endif
