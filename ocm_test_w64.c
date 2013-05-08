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

const unsigned int LOOP=10;
uint64_t mem[0xffff];

int main()
{
    int fd = 0, loop = 0;
    void *map_base = NULL, *virt_addr = NULL; 

    struct timeval tpstart,tpend; 
    float timeuse; 

    const uint64_t ALT_H2F_BASE 	   = 0xC0000000;
    const uint64_t ALT_H2F_OCM_OFFSET	   = 0x00000000;

    off_t target = ALT_H2F_BASE;
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

    gettimeofday(&tpstart,NULL); 
    for( loop = 0 ; loop < LOOP; loop ++ ) {
	    for( idx = 0 ; idx < 0xffff/2; idx ++ ) {
		    writeval = idx;
		    void* access_addr = virt_addr + ALT_H2F_OCM_OFFSET + idx*8;
		    *((uint64_t*) (access_addr)) = writeval;
		    read_result = *((uint64_t*) access_addr);
		    mem[idx] = read_result;
		    if( read_result != writeval ) {
			    printf(" error in r/w \n");
		    } else {
			    //		    printf(" no error in r/w %d\n",writeval); 
		    }
	    }
    }
    gettimeofday(&tpend,NULL); 
    timeuse=1000000*(tpend.tv_sec-tpstart.tv_sec)+ tpend.tv_usec-tpstart.tv_usec; 
    timeuse/=1000000; 
    printf( "Used For 64K ROM Access (Read+Write) Time:%f  throuput %f Mbsp\n", timeuse,((65536.0/2)*64.0*8.0*LOOP)/timeuse/1000.0/1000.0); 

    gettimeofday(&tpstart,NULL); 
    for( loop = 0 ; loop < LOOP; loop ++ ) {
	    for( idx = 0 ; idx < 0xffff/2; idx ++ ) {
		    writeval = idx;
		    void* access_addr = virt_addr + ALT_H2F_OCM_OFFSET + idx*8;
		    *((uint64_t*) (access_addr)) = writeval;
	    }
    }
    gettimeofday(&tpend,NULL); 
    
    timeuse=1000000*(tpend.tv_sec-tpstart.tv_sec)+ tpend.tv_usec-tpstart.tv_usec; 
    timeuse/=1000000; 
    printf( "Used For 64K ROM Access (Write Only) Time:%f  throuput %f Mbsp\n", timeuse,(65536.0/2*64.0*8.0*LOOP)/timeuse/1000.0/1000.0); 

    gettimeofday(&tpstart,NULL); 
    for( loop = 0 ; loop < LOOP; loop ++ ) {
	    for( idx = 0 ; idx < 0xffff/2; idx ++ ) {
		    writeval = idx;
		    void* access_addr = virt_addr + ALT_H2F_OCM_OFFSET +idx*8;
		    read_result = *((uint64_t*) access_addr);
		    mem[idx] = read_result;
	    }
    }
    gettimeofday(&tpend,NULL); 
    
    timeuse=1000000*(tpend.tv_sec-tpstart.tv_sec)+ tpend.tv_usec-tpstart.tv_usec; 
    timeuse/=1000000; 
    printf( "Used For 64K ROM Access (Read Only) Time:%f  throuput %f Mbsp\n", timeuse,(65536.0/2*64.0*8.0*LOOP)/timeuse/1000.0/1000.0); 


	if(munmap(map_base, MAP_SIZE) == -1) FATAL;
    close(fd);

    return;
}

