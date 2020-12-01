#include "internal_watcher.hpp"

details::InternalWatcher::InternalWatcher()  {}

void details::InternalWatcher::update_cpu_time(int cpu_time)
{ // Do nothing
}

void details::InternalWatcher::update_memory_usage(int memory_usage)
{ // Do nothing
}

void details::InternalWatcher::finished(checklib::ProcessStatus status, int exit_code)
{
}
