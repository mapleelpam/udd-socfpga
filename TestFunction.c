/////////////속도측정 2010 12 13///////////////////////////
typedef long _nh_fixed;
typedef long long _nh_fixed_LONG;
#define 	_nh_fixed_resolution_number					17
#define	_nh_fixed_resolution						131072L
#define _nh_int2fp(x) ((x) << _nh_fixed_resolution_number)
#define _nh_fp2int(x) ((x) >> _nh_fixed_resolution_number)
#define _nh_float2fp(x) ((_nh_fixed)((x) * _nh_fixed_resolution))
#define _nh_fpmul(fp1,fp2) ((_nh_fixed)(((_nh_fixed_LONG)(fp1)*(fp2)) >> _nh_fixed_resolution_number))

/* floating-point */
void test_fpuv2();
void dcal_new_fea_one_1(long dim, const float* old_vec, const double* EV, double* new_vec);
/* fixed-point (integer programming) */
void cal_new_fea_one_fixed_1(long dim, const _nh_fixed* old_vec, const _nh_fixed* EV, _nh_fixed* new_vec);

float fOld[200];
double dNew[200];
double dEV[40000];

_nh_fixed ov[200];
_nh_fixed nv[200];
_nh_fixed ev[40000];

/* add by maple - begin */
#include <stdint.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <signal.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#define SYSTICKS system_tick_time()
#define DebugText printf

#define FATAL do { fprintf(stderr, "Error at line %d, file %s (%d) [%s]\n", \
  __LINE__, __FILE__, errno, strerror(errno)); exit(1); } while(0)
#define MAP_SIZE 0x44000 
#define MAP_MASK (MAP_SIZE - 1)

uint32_t system_tick_time()
{
    struct timeval current_time; 
    uint32_t ticks = 0;
    gettimeofday(&current_time,NULL); 
    ticks=1000000*current_time.tv_sec+current_time.tv_usec; 

    return ticks;
}
int main()
{
	test_fpuv2();
}
/* add by maple - end */

void test_fpuv2()
{
	int k, dim = 200;
	const double _div = 12345;
	unsigned int start_time=0;
	unsigned int end_time=0;

	/* prepare data */
// mark by maple - read ov and ev from FPGA ! 
// just toggle bellow value to read data from FPGA or generate it from cpu
#if 0
	for(k=0; k<dim; k++)
	{
		fOld[k] = (float)(k / _div);
		ov[k] = _nh_float2fp(fOld[k]);
	}

	for(k=0; k<dim*dim; k++)
	{
		dEV[k] = k / _div;
		ev[k] = _nh_float2fp(dEV[k]);
	}
#else
// add by maple - read ov and ev from FPGA !
/* add by maple - begin */
	{
		int fd = 0, k= 0;
		void *map_base = NULL, *virt_addr = NULL; 

		const uint32_t ALT_H2F_BASE		= 0xC0000000;
		const uint32_t ALT_H2F_OCM_OFFSET	= 0x00000000;

    		off_t target = ALT_H2F_BASE;
		if((fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1) 
			FATAL;
		printf("/dev/mem opened.\n"); fflush(stdout);

		map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, target & ~MAP_MASK);
    
		virt_addr = map_base + (target & MAP_MASK);

		for(k=0; k<dim; k++)
		{
			void* access_addr = virt_addr + ALT_H2F_OCM_OFFSET + k*4;
			fOld[k] = (float)(k / _div);
			ov[k] = *((float*) access_addr);
		}

		for(k=0; k<dim*dim; k++)
		{
			void* access_addr = virt_addr + ALT_H2F_OCM_OFFSET + k*4;
			dEV[k] = k / _div;
			ev[k] = *((float*) access_addr);
		}

		if(munmap(map_base, MAP_SIZE) == -1) FATAL;
		close(fd);
	}
#endif
/* add by maple - end */
	


	/***
	*** start calculating elapsed time
	***/
	DebugText("Cal START!!\n");									// debug code를 출력하는 함수
	start_time=SYSTICKS;										// SYSTICKS은 TIMER에서 매1ms마다 증가함
	k=0;
	while(k++ < 1000)
	{
		dcal_new_fea_one_1(dim, fOld, dEV, dNew);
	}
	end_time=SYSTICKS;
	DebugText("start_time=%u end_time=%u Cal_Time1=%d\n",start_time,end_time,(end_time - start_time));

	start_time=SYSTICKS;
	k=0;
	while(k++ < 1000)
	{
		cal_new_fea_one_fixed_1(dim, ov, ev, nv);
	}
	end_time=SYSTICKS;
	DebugText("start_time=%u end_time=%u Cal_Time2=%d\n",start_time,end_time,(end_time - start_time));
}

void dcal_new_fea_one_1(long dim, const float* old_vec, const double* EV, double* new_vec)
{
	register double* rd;
	register const float* crf;
	register const double* crd;

	for(rd=new_vec; rd-new_vec<dim; rd++) *rd = 0;

	for(crd=EV,crf=old_vec; crf-old_vec<dim; crf++)
	{
		for(rd = new_vec; rd-new_vec<dim; rd++) *rd += *crf * *crd++;
	}
}

/* fixed-point (integer programming) */
void cal_new_fea_one_fixed_1(long dim, const _nh_fixed* old_vec, const _nh_fixed* EV, _nh_fixed* new_vec)
{
	register _nh_fixed* rd;
	register const _nh_fixed* crf;
	register const _nh_fixed* crd;

	for(rd=new_vec; rd-new_vec<dim; rd++) *rd = 0;

	for(crd=EV,crf=old_vec; crf-old_vec<dim; crf++)
	{
		for(rd = new_vec; rd-new_vec<dim; rd++,crd++) *rd += _nh_fpmul(*crf, *crd);
	}
}


