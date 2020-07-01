#include "check_stats.hpp"

#include "../rp_watcher.hpp"
#include "../stats_getter.hpp"

void checklib::details::async_checker(IStatsGetter *getter, Limits limits, Watcher *watcher)
{
	// TODO: write right condition
	bool is_running = true;
	while (is_running)
	{
		auto current_cpu_time = getter->CPUTime();
		watcher->update_cpu_time(current_cpu_time);

		auto current_memory_usage = getter->peakMemoryUsage();
		watcher->update_memory_usage(current_memory_usage);

		if (limits.useMemoryLimit && limits.memoryLimit < current_memory_usage)
		{
			// Terminate, memory limit exceded
		}
		if (limits.useTimeLimit && limits.timeLimit < current_cpu_time)
		{
			// Terminate, cpu time limit exceded
		}
	}

	// TODO: add exit code getting
	watcher->finished(checklib::ProcessStatus::psExited, -42);
}
