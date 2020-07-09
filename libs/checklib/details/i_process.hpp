#pragma once
#include <optional>

namespace checklib
{

enum ProcessStatus;

namespace details
{

class IProcess
{
public:
	virtual ~IProcess() = default;

	// Wait process for finished.
	// If process was finished return true and update internal ProcessStatus value
	// If process was not finished, return false
	virtual bool wait(int milliseconds) = 0;

	// Kill the process with the given status
	virtual bool end_process(ProcessStatus status) = 0;

	[[nodiscard]] virtual ProcessStatus exit_code() const = 0;

	[[nodiscard]] virtual bool is_running() const = 0;

	[[nodiscard]] virtual int peak_memory_usage() const = 0;

	[[nodiscard]] virtual int cpu_time() const = 0;
};

} // namespace details
} // namespace checklib
