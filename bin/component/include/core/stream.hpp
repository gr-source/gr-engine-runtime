#pragma once

#include <fstream>
#include <string>
#include <vector>
#include <tuple>

class Stream : private std::fstream {
public:
	Stream(const std::string &path, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out | std::ios_base::binary);

	bool write(const void *data, std::size_t size);

	bool read(void *data, std::size_t size);

	void seekg(std::streampos pos, std::ios_base::seekdir seek = std::ios_base::beg);

	std::streampos tellg();

	std::string getPath() const;

private:
	std::vector<std::tuple<void *, std::streamsize>> m_data;
	
	std::string m_path;
};

