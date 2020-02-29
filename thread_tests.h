#pragma once

#include <chrono>
#include <thread>
#include <mutex>
#include <vector>
#include <ctime>

namespace perf::threads
{
	void test_wait_loops();
	void test_ht_workers();
}
