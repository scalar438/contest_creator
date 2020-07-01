#pragma once
#include "rp_types.h"

namespace checklib
{

class IStatsGetter
{
public:
	~IStatsGetter() {}

	/// Peak memory usage of the process
	virtual int peakMemoryUsage() const = 0;

	/// CPU time of the process
	virtual int CPUTime() const = 0;
};

} // namespace checklib