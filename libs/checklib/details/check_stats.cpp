#include "check_stats.hpp"

#include "../process_events.hpp"
#include "i_process.hpp"

void checklib::details::async_checker(IProcess *process, Limits limits, IProcessEvents *ev)
{
	constexpr int ms_delay         = 100;
	constexpr int idle_count_limit = 20;

	bool is_running       = true;
	int prev_cpu_time     = -1;
	int non_changed_count = 0;
	while (is_running)
	{
		process->wait(ms_delay);

		auto current_cpu_time = process->cpu_time();
		ev->update_cpu_time(current_cpu_time);

		auto current_memory_usage = process->peak_memory_usage();
		ev->update_memory_usage(current_memory_usage);

		if (limits.useTimeLimit && limits.timeLimit < current_cpu_time)
		{
			process->end_process(ProcessStatus::psTimeLimitExceeded);
			return;
		}
		if (prev_cpu_time == current_cpu_time)
		{
			++non_changed_count;
			if (non_changed_count == idle_count_limit)
			{
				process->end_process(ProcessStatus::psIdlenessLimitExceeded);
				return;
			}
		}

		if (limits.useMemoryLimit && limits.memoryLimit < current_memory_usage)
		{
			process->end_process(ProcessStatus::psMemoryLimitExceeded);
			return;
		}

		is_running = process->is_running();
	}
}
