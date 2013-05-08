/*
 * devmem2.c: Simple program to read/write from/to any location in memory.
 *
 *  Copyright (C) 2000, Jan-Derk Bakker (jdb@lartmaker.nl)
 *
 *
 * This software has been developed for the LART computing board
 * (http://www.lart.tudelft.nl/). The development has been sponsored by
 * the Mobile MultiMedia Communications (http://www.mmc.tudelft.nl/)
 * and Ubiquitous Communications (http://www.ubicom.tudelft.nl/)
 * projects.
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
/* kfchou at altera dot com just modify it for GHRD */
/* mapleelpam at gmail dot com */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/mman.h>
  
#define FATAL do { fprintf(stderr, "Error at line %d, file %s (%d) [%s]\n", \
  __LINE__, __FILE__, errno, strerror(errno)); exit(1); } while(0)
 
//#define MAP_SIZE 4096UL
#define MAP_SIZE 0x44000 
#define MAP_MASK (MAP_SIZE - 1)

void delay()
{
 //   int idx = 0;
//    for(idx = 0 ; idx < 1024*768 ; idx ++)
        usleep(55688);
}

int main()
{
	int fd = 0;
	void *map_base = NULL, *virt_addr = NULL; 

	struct timeval tpstart,tpend; 
	float timeuse; 

	const uint32_t ALT_LWFPGA_BASE         = 0xFF200000;
	const uint32_t ALT_LWFPGA_SYSID_OFFSET = 0x00010000;
	const uint32_t ALT_LWFPGA_LED_OFFSET   = 0x00010040;

	off_t target = ALT_LWFPGA_BASE ;
	unsigned long read_result, writeval;
	int idx = 0;

	if((fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1) 
		FATAL;
	printf("/dev/mem opened.\n"); fflush(stdout);

	map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, target & ~MAP_MASK);
	//map_base = mmap(0, 0x10100, PROT_READ | PROT_WRITE, MAP_SHARED, fd, target);

	if(map_base == (void *) -1) 
		FATAL;
	printf("Memory mapped at address %p.\n", map_base); fflush(stdout);

	virt_addr = map_base + (target & MAP_MASK);

	// Attempt to read the system ID peripheral
	uint32_t sysid = (*((unsigned long*) (virt_addr+ALT_LWFPGA_SYSID_OFFSET)) ) ;
	printf("INFO: LWFPGA Slave => System ID Peripheral value = 0x%x.\n", (unsigned int)sysid);

	printf("INFO: Toggling LEDs ...\n");

	for( idx = 0 ; idx < 1<<4 ; idx ++  ) {
		uint32_t gray = (idx >> 1) ^ idx;
		void* access_addr = virt_addr + ALT_LWFPGA_LED_OFFSET;
		*((unsigned long*) (access_addr) )= gray; 
		printf(" set LED %u\n",gray); fflush(stdout);
		delay();
	}

	if(munmap(map_base, MAP_SIZE) == -1) FATAL;
	close(fd);

	return;
}

