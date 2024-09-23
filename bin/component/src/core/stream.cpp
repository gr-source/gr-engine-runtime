#include "core/stream.hpp"

std::fstream Stream::stream;

const bool Stream::open(const std::string& path, std::ios_base::openmode mode) {
	stream.open(path, mode);
	return stream.is_open();
}

void Stream::seekg(std::streampos position) {
    stream.seekg(position);
}

std::istream& Stream::getline(std::string &line) {
    return std::getline(stream, line);
}

std::streampos Stream::tellg() {
	return stream.tellg();
}
	
void Stream::close() {
	stream.close();
}
