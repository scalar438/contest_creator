#pragma once
#include "rp_types.h"

namespace checklib
{

class IStatsGetter
{
public:
	~IStatsGetter() {}

	/// Peak memory usage of the process
	virtual int peak_memory_usage() const = 0;

	/// CPU time of the process
	virtual int cpu_time() const = 0;
};

} // namespace checklib