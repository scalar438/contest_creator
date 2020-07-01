#pragma once
#include "../rp_types.h"

namespace checklib
{

class IStatsGetter;
class IProcessEvents;

namespace details
{

// Checks process resources usage, send it into watcher, and terminate process if limits was exceeded
void async_checker(IStatsGetter *getter, Limits limits, IProcessEvents *watcher);

}

} // namespace checklib