#include "stream.hpp"

std::fstream Stream::stream;

std::streamsize Stream::m_offset = 0;

bool Stream::open(const std::string& path, std::ios_base::openmode mode) {
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

std::fstream &Stream::GetStream() {
	return stream;
}
