#pragma once
#include <memory>
#include <optional>

namespace checklib
{

enum ProcessStatus;

namespace details
{

class ProcessExecuteParameters;

// Some methods (platform-dependent) from checklib::Process
class IProcess
{
public:
	// FIXME: this is temporal
	static std::unique_ptr<IProcess> create() {return nullptr;}

	virtual ~IProcess() = default;

	// Wait process for finished.
	// If process finished, return true and update internal ProcessStatus value
	// If process not finished, return false
	virtual bool wait(int milliseconds) = 0;

	virtual int exit_code() const = 0;

	virtual void kill() = 0;

	virtual bool is_abnormal_exit() const = 0;

	// TODO: remove this method
	// Try to determine the final status
	virtual void determine_status() = 0;

	// Start the process
	virtual void start(const ProcessExecuteParameters &) = 0;

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
