#pragma once
#include <string>

namespace checklib
{

// ss - Standard Streams
// Строковые константы с предопределенными названиями
namespace [[deprecated]] ss
{
	const std::string Stdin       = "stdin";
	const std::string Stdout      = "stdout";
	const std::string Stderr      = "stderr";
	const std::string Interactive = "interactive";
} // namespace [[deprecated]]ss

enum class StandardStream
{
	Stdin,
	Stdout,
	Stderr,
	Interactive
};

} // namespace checklib
