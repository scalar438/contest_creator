#pragma once
#include "../rp_types.h"
#include <atomic>
#include <memory>

namespace checklib
{

class IProcessEvents;

namespace details
{

class IProcess;
class IStatusUpdater;

// Checks process resources usage, send it into watcher, and terminate process if limits was exceeded
void async_checker(IProcess *process, Limits limits, IStatusUpdater *status_sender,
                   std::shared_ptr<std::atomic_bool> force_exit);

}

} // namespace checklib