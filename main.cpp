
#include "hayai/hayai.hpp"
#include "thread_tests.h"

#include <chrono>
#include <thread>
#include <mutex>
#include <vector>
#include <ctime>
#include <iostream>

#include <cpuid.h>

namespace perf
{
#ifdef _WIN32
    TIMECAPS _time_caps = { 0 };
    auto min_time_period()
    {
        if (!_time_caps.wPeriodMin)
            timeGetDevCaps(&_time_caps, sizeof(_time_caps));
        return unsigned(_time_caps.wPeriodMin);
    }
#else
	auto min_time_period() -> unsigned
	{
		static unsigned _res = 0;
		if(!_res)
		{
			clock_t t1, t2;			
			t1 = t2 = clock();
			while(t1==t2)
			{
				t2 = clock();
			}
			_res = unsigned(1000.0 * (double(t2)-double(t1))/CLOCKS_PER_SEC);
		}
		return _res;
	}
#endif 

	// https://wiki.osdev.org/Detecting_CPU_Topology_(80x86)
	void print_info()
	{	
		int numCores = 0;	
		int numLogical = 0;
#ifdef _WIN32
		//TODO:
#else		
		unsigned int eax=0,ebx=0,ecx=0,edx=0;

		__cpuid(0x1,eax,ebx,ecx,edx);
		if((edx&(1<<28))==(1<<28))
		{
			//WIP: determine processor topology
			std::cout << "HT enabled\n";
			// Intel IA Dev Guide, Table 3.8
			__cpuid(0x1f,eax=0x1f,ebx,ecx,edx);
			std::cout << "eax = " << eax << " ebx = " << ebx << " ecx = " << ecx << " edx = " << edx << "\n";
			//const auto logical_CPU_bits = ecx;
			//__cpuid(0xb,eax=0xb,ebx,ecx = 1,edx);
			//std::cout << "eax = " << eax << " ebx = " << ebx << " ecx = " << ecx << " edx = " << edx << "\n";
			//numCores = ecx - logical_CPU_bits;
			//numLogical = ebx;
		}
		else
		{
			std::cout << "No HT support\n";
			numCores = numLogical = std::thread::hardware_concurrency();
		}		
#endif
		std::cout << numCores << " physical cores available, " << numLogical << " logical\n";
		
		std::cout << "System time caps:\n";
	    std::cout << "\tclock measurement resolution " << perf::min_time_period() << "ms\n";

#if !defined(_WIN32)
		struct timespec res;
		clock_getres(CLOCK_PROCESS_CPUTIME_ID, &res);
		std::cout << "\tCLOCK_PROCESS_CPUTIME_ID: " << res.tv_sec << "s:" << res.tv_nsec << "ns\n";
		clock_getres(CLOCK_REALTIME, &res);
		std::cout << "\tCLOCK_REALTIME: " << res.tv_sec << "s:" << res.tv_nsec << "ns\n";
#ifndef __MACH__
		clock_getres(CLOCK_REALTIME_COARSE, &res);
		std::cout << "\tCLOCK_REALTIME_COARSE: " << res.tv_sec << "s:" << res.tv_nsec << "ns\n";
#else
		clock_getres(CLOCK_REALTIME, &res); 
		std::cout << "\tCLOCK_REALTIME: " << res.tv_sec << "s:" << res.tv_nsec << "ns\n";
#endif
		clock_getres(CLOCK_MONOTONIC, &res);
		std::cout << "\tCLOCK_MONOTONIC: " << res.tv_sec << "s:" << res.tv_nsec << "ns\n";
#endif
	}
}

void bench_hayai()
{
    hayai::ConsoleOutputter consoleOutputter;
    hayai::Benchmarker::AddOutputter(consoleOutputter);
    std::cout << "Running benchmarks...please wait while Hayai starts...\n";
    hayai::Benchmarker::RunAllTests();
}

int main()
{    
	perf::print_info();
	perf::threads::test_ht_workers();
    //bench_hayai();
    //perf::threads::test_wait_loops();

	return 0;
}
