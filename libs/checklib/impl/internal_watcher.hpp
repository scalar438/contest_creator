#pragma once
#include "rp_types.h"
#include "process_events.hpp"

namespace details
{

class InternalWatcher : public checklib::IProcessEvents
{
public:
	InternalWatcher();

	void update_cpu_time(int cpu_time) override;

	void update_memory_usage(int memory_usage) override;

	void finished(checklib::ProcessStatus status, int exit_code) override;


private:
};

} // namespace details
