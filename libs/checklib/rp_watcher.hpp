#pragma once
#include "rp_types.h"

namespace checklib
{

class Watcher
{
public:
	virtual ~Watcher() {}

	virtual void update_cpu_time(int cpu_time) = 0;

	virtual void update_memory_usage(int memory_usage) = 0;

	virtual void finished(ProcessStatus status, int exit_code) = 0;
};

}