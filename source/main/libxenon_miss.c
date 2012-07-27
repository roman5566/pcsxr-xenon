#include <_ansi.h>
#include <_syslist.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/iosupport.h>
#include <sys/dirent.h>
#include <sys/errno.h>
#include <sys/time.h>

#include <assert.h>

#include <ppc/atomic.h>
#include <ppc/register.h>

#include <xenon_soc/xenon_power.h>
#include <xenon_smc/xenon_smc.h>

void buffer_dump(void * buf, int size);

#define	RTC_BASE	1005782400UL

void usleep(int s) {
    udelay(s);
}



static int new_xenon_gettimeofday(struct _reent *ptr, struct timeval *tp, struct timezone *tz) {
	unsigned char msg[16] = {0x04};
	union{
		uint8_t u8[8];
		uint64_t t;
	} time;
	time.t = 0;
	uint64_t msec = 0;

	xenon_smc_send_message(msg);
	xenon_smc_receive_response(msg);
	
	time.u8[3] = msg[5];
	time.u8[4] = msg[4];
	time.u8[5] = msg[3];
	time.u8[6] = msg[2];
	time.u8[7] = msg[1];
	
	msec = (time.t / 1000) + RTC_BASE;	
	
	tp->tv_sec = (time.t / 1000) + RTC_BASE;
	tp->tv_usec = (time.t % 1000) * 1000;

	return 0;
}

void init_miss(){	
__syscalls.gettod_r = new_xenon_gettimeofday;
}