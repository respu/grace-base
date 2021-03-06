//
//  benchmark.cpp
//  grace
//
//  Created by Simon Ask Ulsnes on 06/04/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#include "base/benchmark.hpp"
#include "base/log.hpp"
#include "io/formatters.hpp"

namespace grace {
	BenchmarkResults benchmark(StringRef description, uint32 iterations, Function<void()> function) {
		if (iterations == 0) iterations = 1;
		BenchmarkResults results;
		results.description = description;
		results.iterations = 0;
		results.worst_time = ProcessTime::microseconds(0);
		results.best_time = ProcessTimeDelta::forever();
		ProcessTimeDelta total_time;
		for (uint32 i = 0; i < iterations; ++i) {
			ProcessTime before;
			ProcessTime after;
			try {
				before = process_now();
				function();
				after = process_now();
			}
			catch (...) {
				Error() << "Unhandled exception in benchmark, stopping!";
				break;
			}
			ProcessTimeDelta elapsed = after - before;
			total_time += elapsed;
			results.add_time(elapsed, i);
		}
		results.avg_time = ProcessTime::microseconds(total_time.microseconds() / results.iterations);
		results.med_time = results.best_time + ProcessTime::microseconds((results.worst_time - results.best_time).microseconds() / 2);
		return move(results);
	}
	
	BenchmarkCompareResults benchmark_compare(StringRef description, uint32 iterations, ArrayRef<Function<void()>> functions, ArrayRef<StringRef> descriptions) {
		BenchmarkCompareResults results;
		results.results.reserve(functions.size());
		results.description = description;
		
		for (size_t i = 0; i < functions.size(); ++i) {
			StringRef d = i >= descriptions.size() ? "<unknown>" : descriptions[i];
			BenchmarkResults r = benchmark(d, iterations, functions[i]);
			results.results.push_back(move(r));
		}
		
		return move(results);
	}
	
	BenchmarkCompareResults benchmark_compare_interleaved(StringRef description, uint32 iterations, ArrayRef<Function<void()>> functions, ArrayRef<StringRef> descriptions) {
		BenchmarkCompareResults results;
		results.results.resize(functions.size());
		results.description = description;
		
		for (size_t i = 0; i < functions.size(); ++i) {
			BenchmarkResults& r = results.results[i];
			r.description = i >= descriptions.size() ? "<unknown>" : descriptions[i];
			r.iterations = 0;
			r.worst_time = ProcessTime::microseconds(0);
			r.best_time = ProcessTimeDelta::forever();
		}
		
		for (uint32 i = 0; i < iterations; ++i) {
			for (size_t j = 0; j < functions.size(); ++j) {
				auto& f = functions[j];
				BenchmarkResults& r = results.results[j];
				ProcessTime before;
				ProcessTime after;
				try {
					before = process_now();
					f();
					after = process_now();
				}
				catch (...) {
					Error() << "Unhandled exception in benchmark, stopping!";
					goto stop;
				}
				ProcessTimeDelta elapsed = after - before;
				// using avg_time as total time for now
				r.avg_time += elapsed;
				r.add_time(elapsed, i);
			}
		}
		
		stop:
		for (auto& r: results.results) {
			r.avg_time = ProcessTime::microseconds(r.avg_time.microseconds() / r.iterations);
			r.med_time = r.best_time + ProcessTime::microseconds((r.worst_time - r.best_time).microseconds() / 2);
		}
		
		return move(results);
	}
	
	void BenchmarkResults::add_time(ProcessTimeDelta elapsed, uint32 iteration_no) {
		++iterations;
		if (elapsed < best_time) {
			best_time = elapsed;
			best_iteration = iteration_no;
		}
		if (elapsed > worst_time) {
			worst_time = elapsed;
			worst_iteration = iteration_no;
		}
	}
	
	void print_benchmark_results(FormattedStream& os, const BenchmarkResults& bm, bool include_heading = true) {
		if (include_heading) {
			os << "=== BENCHMARK: " << bm.description << '\n';
		}
		os << "  " << pad_or_truncate("Iterations: ", 13, ' ', true) << bm.iterations << '\n';
		os << "  " << pad_or_truncate("Best time: ", 13, ' ', true) << format_time_delta(bm.best_time) << '\n';
		os << "  " << pad_or_truncate("Worst time: ", 13, ' ', true) << format_time_delta(bm.worst_time) << '\n';
		os << "  " << pad_or_truncate("Avg. time: ", 13, ' ', true) << format_time_delta(bm.avg_time) << '\n';
	}
	
	FormattedStream& operator<<(FormattedStream& os, const BenchmarkResults& bm) {
		print_benchmark_results(os, bm, true);
		return os;
	}
	
	FormattedStream& operator<<(FormattedStream& os, const BenchmarkCompareResults& bm) {
		os << "=== BENCHMARK/COMPARE: " << bm.description << '\n';
		if (bm.results.size()) {
			os << "BEST: " << bm.results.front().description << '\n';
			print_benchmark_results(os, bm.results.front(), false);
		}
		if (bm.results.size() > 1) {
			os << "WORST: " << bm.results.back().description << '\n';
			print_benchmark_results(os, bm.results.back(), false);
		}
		return os;
	}
	
	namespace {
		static byte benchmark_manager_memory[sizeof(BenchmarkManager)];
	}
	
	BenchmarkManager& BenchmarkManager::get() {
		static BenchmarkManager* p = new(benchmark_manager_memory) BenchmarkManager;
		return *p;
	}
	
	void BenchmarkManager::begin_frame() {
		frame.force_clear();
		current = nullptr;
		frame_alloc.reset(frame_alloc.begin());
	}
	
	void BenchmarkManager::add_scoped_times(const BareLinkList<BenchmarkScopeResults> &times) {
		for (auto it = times.begin(); it != times.end(); ++it) {
			BenchmarkResults& results = accum[it->name];
			results.add_time(it->time, results.iterations);
			add_scoped_times(it->children);
		}
	}
	
	void BenchmarkManager::finish_frame() {
		add_scoped_times(frame);
	}
	
	void BenchmarkManager::clear() {
		begin_frame();
		accum.clear();
	}
	
	void BenchmarkManager::enter_scope(StringRef name) {
		BenchmarkScopeResults* results = new(frame_alloc) BenchmarkScopeResults;
		results->name = name;
		if (current) {
			current->children.link_tail(results);
			results->parent = current;
		} else {
			frame.link_tail(results);
		}
		current = results;
	}
	
	void BenchmarkManager::leave_scope(ProcessTimeDelta elapsed) {
		ASSERT(current);
		current->time = elapsed;
		current = current->parent;
	}
}