#pragma once

namespace checklib
{

enum ProcessStatus;

namespace details
{

class IStatusUpdater
{
public:
	virtual ~IStatusUpdater() = 0;

	virtual void set_status(ProcessStatus status) = 0;

	virtual void set_cpu_time(int cpu_time) = 0;

	virtual void set_peak_memory(int memory) = 0;
};
} // namespace details
} // namespace checklib