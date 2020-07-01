#pragma once
#include "../rp_types.h"
#include "../process_events.hpp"

#include "boost/signals2.hpp"

namespace details
{

class InternalWatcher : public checklib::IProcessEvents
{
public:
	InternalWatcher();

	void update_cpu_time(int cpu_time) override;

	void update_memory_usage(int memory_usage) override;

	void finished(checklib::ProcessStatus status, int exit_code) override;

	void set_signal(boost::signals2::signal<void(int)> *signal);

private:
	[[deprecated]] boost::signals2::signal<void(int)> *m_signal;
};

} // namespace details
