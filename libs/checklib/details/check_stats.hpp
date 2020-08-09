#pragma once
#include "../rp_types.h"

namespace checklib
{

class IProcessEvents;

namespace details
{

class IProcess;
class IStatusSender;

// Checks process resources usage, send it into watcher, and terminate process if limits was exceeded
void async_checker(IProcess *process, Limits limits, IProcessEvents *watcher, IStatusSender *status_sender);

}

} // namespace checklib