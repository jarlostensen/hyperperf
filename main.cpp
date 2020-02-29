
#include "hayai/hayai.hpp"
#include "perfutils.h"

#include <chrono>
#include <thread>
#include <mutex>
#include <vector>
#include <iostream>

#include <string>
#include <map>

namespace perf
{
#ifdef WINDOWS
    TIMECAPS _time_caps = { 0 };
    auto min_time_period()
    {
        if (!_time_caps.wPeriodMin)
            timeGetDevCaps(&_time_caps, sizeof(_time_caps));
        return unsigned(_time_caps.wPeriodMin);
    }
#else
	auto min_time_period()
	{
		//TODO:
		return 1u;
	}
#endif
}

using hi_res_clock = std::chrono::high_resolution_clock;
using milliseconds = std::chrono::milliseconds;
using nanoseconds = std::chrono::nanoseconds;
using microseconds = std::chrono::microseconds;

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

    const auto tf = [&stats, &stat_mtx]() {
        double stat = 0.0;
        constexpr auto kRuns = 4000u;
        for (auto n = 0u; n < kRuns; ++n)
        {
            const auto start = hi_res_clock::now();
            for (;;)
            {
                const auto now = hi_res_clock::now();
                if (std::chrono::duration_cast<milliseconds>(now - start).count() >= 5)
                    break;
#ifdef WINDOWS
                Sleep(0);
#endif
            }
            const auto end = hi_res_clock::now();
            stat += (end - start).count();
        }
        std::lock_guard<std::mutex> lock{ stat_mtx };
        stats.emplace_back(stat / kRuns);
    };

    std::vector<std::thread>	_threads;
    std::cout << "creating/starting " << std::thread::hardware_concurrency() << " threads\n";
    for (auto t = 0u; t < std::thread::hardware_concurrency(); ++t)
    {
        _threads.emplace_back(tf);
    }

    std::cout << "waiting for threads to finish...";
    for (auto& t : _threads)
    {
        t.join();
    }
    std::cout << "done" << std::endl;

    perf::RunningStat total_stat;
    for (auto d : stats)
    {
        total_stat.push(d);
    }

    std::cout << "\tMean " << total_stat.mean() / 1000000 << "ms, standard deviation " << total_stat.stdev() / 1000 << "us\n";
    std::cout << "\tMedian " << total_stat.median() / 1000000 << "ms, 1st quartile " << total_stat.first_quartile() / 1000000 << "ms, 3rd quartile " << total_stat.third_quartile() / 1000000 << "ms" << std::endl;
    switch (total_stat.shape())
    {
    case perf::Stats::Shape::kSymmetric:
        std::cout << "\tdistribution is ~symmetric\n";
        break;
    case perf::Stats::Shape::kLeft:
        std::cout << "\tdistribution is skewed towards the first quartile\n";
        break;
    case perf::Stats::Shape::kRight:
        std::cout << "\tdistribution is skewed towards the third quartile\n";
        break;
    }
}


int main()
{    
    //bench_hayai();
    threads_test();

    std::cout << "\nMin time period used is " << perf::min_time_period() << "ms\n";

	return 0;
}
