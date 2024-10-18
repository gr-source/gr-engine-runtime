#pragma once

#include <fstream>
#include <string>
#include <vector>
#include <tuple>

class Stream : private std::fstream {
public:
	Stream(const std::string &path, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out | std::ios_base::binary);
	Stream(const std::string &path);

	bool write(const void *data, std::size_t size);

	bool read(void *data, std::size_t size);

private:
	std::vector<std::tuple<void *, std::streamsize>> m_data;
};

