
#include "hayai/hayai.hpp"
#include <windows.h>

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
}

BENCHMARK(Wait,Sleep1SystemRes,5,10)
{
	Sleep(1);
}


struct sleep_hires_fixture : hayai::Fixture
{
	void SetUp() override
	{
		timeBeginPeriod(1);
	}

	void TearDown() override
	{
		timeEndPeriod(1);
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
BENCHMARK_F(spin_wait_fixture,Spin1,10,1000)
{
	const auto start = perf::qpc_now();
	for(;;)
	{
		const auto now = perf::qpc_now();
		if((now - start)/_freq_ms >= 1)
			break;
	}
}

BENCHMARK_F(spin_wait_fixture,Spin1Yield,10,1000)
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
    hayai::Benchmarker::RunAllTests();

	return 0;
}
