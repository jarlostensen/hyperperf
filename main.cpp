
#include "hayai/hayai.hpp"
#include "thread_tests.h"

#include <chrono>
#include <thread>
#include <mutex>
#include <vector>
#include <ctime>
#include <iostream>

#include "cpuid.h"

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

	struct processor_info_t
	{
		size_t		_phys_cores = 1;
		size_t		_num_cores = 1;
		bool		_ht = false;
	};

	processor_info_t _proc_info;

	void init_processor_info()
	{
		using namespace system_info;

		cpuid cpu_id;
		char _vendor_string[13];
		cpu_id = 0;
    	memcpy(_vendor_string + 0, &cpu_id.ebx(), sizeof(int));
    	memcpy(_vendor_string + 4, &cpu_id.edx(), sizeof(int));
    	memcpy(_vendor_string + 8, &cpu_id.ecx(), sizeof(int));
    	_vendor_string[12] = 0;
		std::cout << "cpuid vendor \"" << _vendor_string << "\"\n";

		cpu_id = 1;
		_proc_info._ht = cpu_id.bits_set(cpuid::Register::edx, 1<<28);
		if(_proc_info._ht)
		{
			std::cout << "hyperthreading enabled\n";

			// Inte IA Dev guide, table 3.8
			cpu_id.ecx() = 0;
			cpu_id = 0x1f;
			if(!cpu_id.is_ok())
			{
				// unsupported, try 0xb
				cpu_id.ecx() = 1;
				cpu_id = 0x0b;

				if(!cpu_id.is_ok())
				{
					std::cerr << "unable to determine processor topology\n";
					return;
				}
			}
			//TODO: this isn't strictly correct, ebx is the number of logical processors "at this level"
			_proc_info._phys_cores = cpu_id.edx();
			_proc_info._num_cores = cpu_id.ebx();
		}

		std::cout << "phys cores " << _proc_info._phys_cores << ", logical cores " << _proc_info._num_cores << "\n";
	}

	void set_thread_affinity(std::thread& t, size_t phys_core)
	{
		(void)t;
		(void)phys_core;
	}

	// https://wiki.osdev.org/Detecting_CPU_Topology_(80x86)
	void print_info()
	{	
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
	perf::init_processor_info();
	perf::print_info();
	perf::threads::test_ht_workers();
    //bench_hayai();
    //perf::threads::test_wait_loops();

	return 0;
}
