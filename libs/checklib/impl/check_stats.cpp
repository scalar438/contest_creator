#include "check_stats.hpp"
#include "i_process.hpp"
#include "i_status_updater.hpp"
#include "process_events.hpp"

void checklib::details::async_checker(IProcess *process, Limits limits,
                                      IStatusUpdater *status_sender,
                                      std::shared_ptr<std::atomic_bool> force_exit)
{
	constexpr int ms_delay         = 100;
	constexpr int idle_count_limit = 20;

	bool is_running       = true;
	int prev_cpu_time     = -1;
	int non_changed_count = 0;
	while (is_running && !*force_exit)
	{
		process->wait(ms_delay);

		auto current_cpu_time = process->cpu_time();
		status_sender->set_cpu_time(current_cpu_time);

		auto current_memory_usage = process->peak_memory_usage();
		status_sender->set_peak_memory(current_memory_usage);

		if (limits.useTimeLimit && limits.timeLimit < current_cpu_time)
		{
			status_sender->set_status(ProcessStatus::psTimeLimitExceeded);
			process->kill();
			return;
		}
		if (prev_cpu_time == current_cpu_time)
		{
			++non_changed_count;
			if (non_changed_count == idle_count_limit)
			{
				status_sender->set_status(ProcessStatus::psIdlenessLimitExceeded);
				process->kill();
				return;
			}
		}

		if (limits.useMemoryLimit && limits.memoryLimit < current_memory_usage)
		{
			status_sender->set_status(ProcessStatus::psMemoryLimitExceeded);
			process->kill();
			return;
		}

		is_running = !process->exit_code().has_value();
	}
	if (process->is_abnormal_exit())
		status_sender->set_status(ProcessStatus::psRuntimeError);
	else
		status_sender->set_status(ProcessStatus::psExited);
}
