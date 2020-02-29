
#include "thread_tests.h"
#include "perfutils.h"
#include <atomic>
#include <iostream>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

using hi_res_clock = std::chrono::high_resolution_clock;
using milliseconds = std::chrono::milliseconds;
using nanoseconds = std::chrono::nanoseconds;
using microseconds = std::chrono::microseconds;

namespace perf::threads
{
	void test_wait_loops()
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
					std::this_thread::yield();
				}
				const auto end = hi_res_clock::now();
				stat += (end - start).count();
			}
			std::lock_guard lock{ stat_mtx };
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

	void test_ht_workers()
	{
#ifdef _WIN32
		ULONG_PTR ht_core_mask = 0;
		DWORD entries = 0;
		GetLogicalProcessorInformation(nullptr, &entries);
		const auto procbuffer = new SYSTEM_LOGICAL_PROCESSOR_INFORMATION[entries];
		GetLogicalProcessorInformation(procbuffer, &entries);
		// just find the first HT core 
		for(DWORD entry = 0; entry < entries; ++entry)
		{
			if(procbuffer[entry].Relationship == LOGICAL_PROCESSOR_RELATIONSHIP::RelationProcessorCore)
			{
				// more than one logical core to this physical core?
				if ( ((procbuffer[entry].ProcessorMask & (procbuffer[entry].ProcessorMask-1)) != procbuffer[entry].ProcessorMask) 
					&& 
					procbuffer[entry].ProcessorCore.Flags == 1)
				{
					std::cout << "HT core 0x" << std::hex << procbuffer[entry].ProcessorMask << "\n";
					ht_core_mask = procbuffer[entry].ProcessorMask;
					break;
				}
			}
		}
		delete[] procbuffer;

		constexpr size_t kNumPoints = 100000000;
		double * big_data = new double[kNumPoints];
		double sum_1 = double(rand());
		double sum_2 = double(rand());
		std::atomic_bool start = false;

		std::cout << "generating some data...\n";
		for(auto n = 1u; n < kNumPoints; ++n)
		{
			big_data[n] = double(rand())/n;
		}
		
		const auto worker = [&big_data,&start, kNumPoints](double* sum) {

			while(!start.load())
			{
				std::this_thread::yield();
			}

			auto lsum = *sum;
			for(auto n = 0; n < kNumPoints; ++n)
			{
				lsum += sin(big_data[n]);
			}
			*sum = lsum;
		};

		std::thread t1{worker,&sum_1};
		std::thread t2{worker,&sum_2};
		
		SetThreadAffinityMask(HANDLE(t1.native_handle()), ht_core_mask);
		SetThreadAffinityMask(HANDLE(t2.native_handle()), ht_core_mask);

		std::cout << "starting threads...\n";
		const auto t_start = hi_res_clock::now();
		start.store(true);

		t1.join();
		t2.join();

		const auto t_end = hi_res_clock::now();

		std::cout << "it took " << std::dec << std::chrono::duration_cast<milliseconds>(t_end - t_start).count() << "ms\n";
		//std::cout << "sum_1 = " << sum_1 << ", sum_2 = " << sum_2 << std::endl;

		delete[] big_data;
#else
		//TODO:
#endif
	}
}