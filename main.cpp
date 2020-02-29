
#include "hayai/hayai.hpp"
#include "thread_tests.h"

#include <chrono>
#include <thread>
#include <mutex>
#include <vector>
#include <ctime>
#include <iostream>

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
		clock_getres(CLOCK_REALTIME_COARSE, &res);
		std::cout << "\tCLOCK_REALTIME_COARSE: " << res.tv_sec << "s:" << res.tv_nsec << "ns\n";
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
