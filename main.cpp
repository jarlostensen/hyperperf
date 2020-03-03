
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
		size_t		_topology_levels = 1;
		unsigned	_smt_mask_width = 0;
		unsigned	_core_mask_width = 0;
		struct
		{
		    bool	_ht:1;
			bool	_b_leaf:1;
			bool	_1f_leaf:1;
		} _cpuid_caps;
#
        unsigned smt_select_mask() const
        {
            return ~((-1)<<_smt_mask_width);
        }
	};

	processor_info_t _proc_info;

	std::ostream& operator<<(std::ostream& os, system_info::cpuid& cpuid_)
	{
	    return os << std::hex << "[eax:0x" << cpuid_.eax() << ", ebx:0x" << cpuid_.ebx() << ", ecx:0x" << cpuid_.ecx() << ", edx:0x" << cpuid_.ebx() << "]";
	}

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
		_proc_info._cpuid_caps._ht = cpu_id.bits_set(cpuid::regs::edx, 1<<28);
		
		//NOTE: for the time being following https://software.intel.com/sites/default/files/managed/ba/f1/intel-64-architecture-processor-topology-enumeration.pdf
		//		which strangely does *not* cover leaf 0x1f
		const auto max_leaf = cpu_id.eax();
		if(max_leaf < 0xb)
		{
			std::cerr << "max leaf < 0xb: ";
			//TODO: fallback for (really) old hardware...
			std::cerr << "CPUID doesn't support leaf 0xb. Not sure how to deal with that now\n";
			return;
		}

		// check if it's properly supported
		cpu_id = {0xb,0};		
		_proc_info._cpuid_caps._b_leaf = cpu_id.ebx()!=0;
		cpu_id = {0x1f,0};
		_proc_info._cpuid_caps._1f_leaf = cpu_id.ebx()!=0;

		if(_proc_info._cpuid_caps._1f_leaf)
		{
		    //TODO:
		}
		else if ( _proc_info._cpuid_caps._b_leaf )
		{
			constexpr unsigned kSubLeaf_SMTLevel = 0;
			constexpr unsigned kSubLeaf_ProcessorCore = 1;
			
		    cpu_id = {0xb,kSubLeaf_SMTLevel};
			if(cpu_id.extract_reg_field(cpuid::regs::ecx, 8, 15)==1)
			{
			    _proc_info._smt_mask_width = cpu_id.extract_reg_field(cpuid::regs::eax, 0, 4);

				cpu_id = {0xb,kSubLeaf_ProcessorCore};
				if(cpu_id.extract_reg_field(cpuid::regs::ecx, 8, 15) == 2 )
				{
				    _proc_info._core_mask_width = cpu_id.extract_reg_field(cpuid::regs::eax, 0, 4);
					//ZZZ:
					_proc_info._phys_cores = 1<<_proc_info._core_mask_width;
				}
				//ZZZ:
				_proc_info._num_cores = _proc_info._phys_cores * (1<<_proc_info._smt_mask_width);
			}
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
	//perf::threads::test_ht_workers();
    //bench_hayai();
    //perf::threads::test_wait_loops();

	return 0;
}
