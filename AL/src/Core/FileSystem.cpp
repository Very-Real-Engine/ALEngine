#include "alpch.h"

#include "Core/FileSystem.h"

namespace ale
{
SBuffer FileSystem::readFileBinary(const std::filesystem::path &filepath)
{
	std::ifstream stream(filepath, std::ios::binary | std::ios::ate);

	if (!stream)
	{
		// Failed to open the file
		return {};
	}

	std::streampos end = stream.tellg();
	stream.seekg(0, std::ios::beg);
	uint64_t size = end - stream.tellg();

	if (size == 0)
	{
		// File is empty
		return {};
	}
	// AL_CORE_TRACE("{0}", size);

	SBuffer buffer(size);

	stream.read(buffer.as<char>(), size);
	stream.close();
	return buffer;
}

} // namespace ale