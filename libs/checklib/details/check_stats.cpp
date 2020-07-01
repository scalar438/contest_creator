#include "check_stats.hpp"

#include "../process_events.hpp"
#include "../stats_getter.hpp"

void checklib::details::async_checker(IStatsGetter *getter, Limits limits, IProcessEvents *ev)
{
	// TODO: write right condition
	bool is_running = true;
	while (is_running)
	{
		auto current_cpu_time = getter->cpu_time();
		ev->update_cpu_time(current_cpu_time);

		auto current_memory_usage = getter->peak_memory_usage();
		ev->update_memory_usage(current_memory_usage);

		if (limits.useMemoryLimit && limits.memoryLimit < current_memory_usage)
		{
			// Terminate, memory limit exceeded
		}
		if (limits.useTimeLimit && limits.timeLimit < current_cpu_time)
		{
			// Terminate, cpu time limit exceeded
		}
	}

	// TODO: add exit code getting
	ev->finished(checklib::ProcessStatus::psExited, -42);
}
