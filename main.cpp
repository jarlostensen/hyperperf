
#include "hayai/hayai.hpp"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <iostream>

namespace perf
{
	uint64_t qpc_now()
	{
		LARGE_INTEGER q;
		QueryPerformanceCounter(&q);
		return q.QuadPart;
	}

	uint64_t qpc_freq()
	{
		static uint64_t _freq = 0;
		if(!_freq)
		{
			LARGE_INTEGER freq;
			QueryPerformanceFrequency(&freq);
			_freq = freq.QuadPart;
		}
		return _freq;
	}

	TIMECAPS _time_caps = {0};
	UINT min_time_period()
	{
		if(!_time_caps.wPeriodMin)
			timeGetDevCaps(&_time_caps,sizeof(_time_caps));
		return _time_caps.wPeriodMin;
	}
}

BENCHMARK(Wait,Sleep1SystemRes,5,10)
{
	Sleep(1);
}


struct sleep_hires_fixture : hayai::Fixture
{
	void SetUp() override
	{
		timeBeginPeriod(perf::min_time_period());
	}

	void TearDown() override
	{
		timeEndPeriod(perf::min_time_period());
	}
};
BENCHMARK_F(sleep_hires_fixture,Sleep1HiRes,5,10)
{
	Sleep(1);
}

struct spin_wait_fixture : hayai::Fixture
{
	void SetUp() override
	{
		_freq_ms = perf::qpc_freq()/1000;
	}

	uint64_t _freq_ms;
};
BENCHMARK_F(spin_wait_fixture,Spin1,5,1000)
{
	const auto start = perf::qpc_now();
	for(;;)
	{
		const auto now = perf::qpc_now();
		if((now - start)/_freq_ms >= 1)
			break;
	}
}

BENCHMARK_F(spin_wait_fixture,Spin1Yield,5,1000)
{
	const auto start = perf::qpc_now();
	for(;;)
	{
		const auto now = perf::qpc_now();
		if((now - start)/_freq_ms >= 1)
			break;
		Sleep(0);
	}
}

int main(int argc, char * argv[])
{
	(void)argc;
	(void)argv;
	
	hayai::ConsoleOutputter consoleOutputter;
    hayai::Benchmarker::AddOutputter(consoleOutputter);
	std::cout << "Running benchmarks...please wait while Hayai starts...\n";
    hayai::Benchmarker::RunAllTests();

	std::cout << "\nMin time period used is " << perf::min_time_period() << "ms\n";

	return 0;
}
