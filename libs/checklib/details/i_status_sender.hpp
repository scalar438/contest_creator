#pragma once

namespace checklib
{

enum ProcessStatus;

namespace details
{

class IStatusSender
{
public:
	virtual ~IStatusSender() = 0;

	void set_status(ProcessStatus status);
};
} // namespace details
} // namespace checklib