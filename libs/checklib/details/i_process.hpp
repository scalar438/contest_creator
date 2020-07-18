#pragma once
#include <optional>

namespace checklib
{

enum ProcessStatus;

namespace details
{

// Some methods from checklib::Process
class IProcess
{
public:
	virtual ~IProcess() = default;

	// Wait process for finished.
	// If process finished, return true and update internal ProcessStatus value
	// If process not finished, return false
	virtual bool wait(int milliseconds) = 0;

	// Calls when result process status already determined
	virtual void end_process(ProcessStatus status) = 0;

	// Try to determine the final status
	virtual void determine_status() = 0;

	[[nodiscard]] virtual bool is_running() const = 0;

	// Peak memory usage, in bytes for current or last running process
	// If the process didn't run, return zero
	[[nodiscard]] virtual int peak_memory_usage() = 0;

	// CPU time of current or last running process
	// If the process didn't run, return zero
	[[nodiscard]] virtual int cpu_time() = 0;
};

} // namespace details
} // namespace checklib
