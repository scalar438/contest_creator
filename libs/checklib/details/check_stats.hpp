#pragma once
#include "../rp_types.h"

namespace checklib
{

class IProcessEvents;

namespace details
{

class IProcess;

// Checks process resources usage, send it into watcher, and terminate process if limits was exceeded
void async_checker(IProcess *process, Limits limits, IProcessEvents *watcher);

}

} // namespace checklib