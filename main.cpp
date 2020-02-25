
#include "hayai/hayai.hpp"
#include <chrono>
#include <thread>
#include <iostream>

using hi_res_clock = std::chrono::high_resolution_clock;
using milliseconds = std::chrono::milliseconds;
using nanoseconds = std::chrono::nanoseconds;
using microseconds = std::chrono::microseconds;

#define PERF_WAIT_TIME_MS 2

namespace perf
{
	TIMECAPS _time_caps = { 0 };
	UINT min_time_period()
	{
		if (!_time_caps.wPeriodMin)
			timeGetDevCaps(&_time_caps, sizeof(_time_caps));
		return _time_caps.wPeriodMin;
	}
}

BENCHMARK(Wait, SleepSystemRes, 5, 10)
{
	Sleep(PERF_WAIT_TIME_MS);
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
BENCHMARK_F(sleep_hires_fixture, SleepHiRes, 5, 10)
{
	Sleep(PERF_WAIT_TIME_MS);
}

BENCHMARK(SpinWait, SpinHot, 5, 1000)
{
	const auto start = hi_res_clock::now();
	for (;;)
	{
		const auto now = hi_res_clock::now();
		if (std::chrono::duration_cast<milliseconds>(now - start).count() >= PERF_WAIT_TIME_MS)
			break;
	}
}

BENCHMARK(SpinWait, SpinYield, 5, 1000)
{
	const auto start = hi_res_clock::now();
	for (;;)
	{
		const auto now = hi_res_clock::now();
		if (std::chrono::duration_cast<milliseconds>(now - start).count() >= PERF_WAIT_TIME_MS)
			break;
		Sleep(0);
	}
}

void bench_hayai()
{
	hayai::ConsoleOutputter consoleOutputter;
	hayai::Benchmarker::AddOutputter(consoleOutputter);
	std::cout << "Running benchmarks...please wait while Hayai starts...\n";
	hayai::Benchmarker::RunAllTests();
}

void threads_test()
{
	const auto tf = []() {
		for (auto n = 0u; n < 10000; ++n)
		{
			const auto start = hi_res_clock::now();
			for (;;)
			{
				const auto now = hi_res_clock::now();
				if (std::chrono::duration_cast<milliseconds>(now - start).count() >= 5)
					break;
				Sleep(0);
			}
		}
	};

	std::thread t1{tf};
	std::thread t2{tf};
	std::thread t3{tf};
	std::thread t4{tf};
	std::thread t5{tf};
	std::thread t6{tf};

	t1.join();
	t2.join();
	t3.join();
	t4.join();
	t5.join();
	t6.join();
}

int main(int argc, char* argv[])
{
	(void)argc;
	(void)argv;

	bench_hayai();
	//threads_test();

	std::cout << "\nMin time period used is " << perf::min_time_period() << "ms\n";

	return 0;
}
