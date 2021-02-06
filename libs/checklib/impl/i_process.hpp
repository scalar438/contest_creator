#pragma once
#include <memory>
#include <optional>

namespace checklib
{

enum ProcessStatus;
struct ProcessExecuteParameters;

namespace details
{

// Some methods (platform-dependent) from checklib::Process
class IProcess
{
public:
	// FIXME: this is temporal
	static std::unique_ptr<IProcess> create() { return nullptr; }

	virtual ~IProcess() = default;

	/// Wait process for finished.
	/// If process finished, return true immediately
	/// If process not finished, return false
	virtual bool wait(int milliseconds) = 0;

	[[nodiscard]] virtual std::optional<int> exit_code() const = 0;

	virtual void kill() = 0;

	[[nodiscard]] virtual bool is_abnormal_exit() const = 0;

	/// Peak memory usage, in bytes for the process
	[[nodiscard]] virtual int peak_memory_usage() = 0;

	/// CPU time of the process
	[[nodiscard]] virtual int cpu_time() = 0;

	[[nodiscard]] bool is_running() const { return !exit_code().has_value(); }
};

} // namespace details
} // namespace checklib
