#include "internal_watcher.hpp"

details::InternalWatcher::InternalWatcher() : m_signal(nullptr) {}

void details::InternalWatcher::update_cpu_time(int cpu_time)
{ // Do nothing
}

void details::InternalWatcher::update_memory_usage(int memory_usage)
{ // Do nothing
}

void details::InternalWatcher::finished(checklib::ProcessStatus status, int exit_code)
{
	if (m_signal)
	{
		(*m_signal)(exit_code);
	}
}

void details::InternalWatcher::set_signal(boost::signals2::signal<void(int)> *signal)
{
	m_signal = signal;
}
