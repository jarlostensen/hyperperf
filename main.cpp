
#include "hayai/hayai.hpp"
#include "perfutils.h"

#include <chrono>
#include <thread>
#include <mutex>
#include <vector>
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
	std::vector<double>	stats;
	std::mutex stat_mtx;
	
	const auto tf = [&stats,&stat_mtx]() {
		double stat = 0.0;
		constexpr auto kRuns = 10000u;
        for (auto n = 0u; n < kRuns; ++n)
		{
			const auto start = hi_res_clock::now();
			for (;;)
			{
				const auto now = hi_res_clock::now();
				if (std::chrono::duration_cast<milliseconds>(now - start).count() >= 5)
					break;
				Sleep(0);
			}
			const auto end = hi_res_clock::now();
			stat += (end - start).count();
		}
		std::lock_guard<std::mutex> lock{stat_mtx};
		stats.emplace_back(stat/kRuns);
	};

	std::vector<std::thread>	_threads;
	std::cout << "creating/starting " << std::thread::hardware_concurrency() << " threads\n";
	for(auto t = 0u; t < std::thread::hardware_concurrency(); ++t)
	{
	    _threads.emplace_back(tf);
	}

	std::cout << "waiting for threads to finish...";
	for(auto & t : _threads)
	{
	    t.join();
	}
	std::cout << "done" << std::endl;

	perf::RunningStat total_stat;
	for(auto d : stats)
	{
	    total_stat.Push(d);
	}

	std::cout << "\tMean " << total_stat.Mean()/1000000 << "ms, standard deviation " << total_stat.StandardDeviation() << "ns\n";
}

int main(int argc, char* argv[])
{
	(void)argc;
	(void)argv;

	//bench_hayai();
	threads_test();

	std::cout << "\nMin time period used is " << perf::min_time_period() << "ms\n";

	return 0;
}
