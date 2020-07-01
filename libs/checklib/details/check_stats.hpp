#pragma once
#include "../rp_types.h"

namespace checklib
{

class IStatsGetter;
class Watcher;

namespace details
{

// Checks process resources usage, send it into watcher, and terminate process if limits was exceded
void async_checker(IStatsGetter *getter, Limits limits, Watcher *watcher);

}

} // namespace checklib