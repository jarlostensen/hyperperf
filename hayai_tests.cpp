#include "hayai/hayai.hpp"
#include <chrono>

using hi_res_clock = std::chrono::high_resolution_clock;
using milliseconds = std::chrono::milliseconds;
using nanoseconds = std::chrono::nanoseconds;
using microseconds = std::chrono::microseconds;

#define PERF_WAIT_TIME_MS 2

namespace perf
{
    extern unsigned min_time_period();
}

#ifdef WINDOWS
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
#endif 

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
#ifdef WINDOWS
        Sleep(0);
#endif
    }
}
