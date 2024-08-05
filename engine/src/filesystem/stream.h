#pragma once

#include <fstream>
#include <string>

class Stream {
public:
	
	static void open(const std::string& path, std::ios_base::openmode mode);
	
	template <typename T>
	static std::istream& read(T* data, std::streamsize count);
	
	template <typename T>
	static T read(std::streamsize count);
	
	template <typename T>
	static void write(const T* data, std::streamsize count);

	static void seekg(std::streampos position);

	static std::istream& getline(std::string& line);
	
	static std::streampos tellg();
	
	static void close();
	
	static std::fstream stream;
};

template <typename T>
inline std::istream& Stream::read(T* data, std::streamsize count) {
	return stream.read(reinterpret_cast<char*>(data), sizeof(T) * count);
}
	
template <typename T>
inline T Stream::read(std::streamsize count) {
	T data;
	stream.read(reinterpret_cast<char*>(&data), sizeof(T) * count);
	return data;
}
	
template <typename T>
inline void Stream::write(const T* data, std::streamsize count) {
	stream.write(reinterpret_cast<const char*>(data), sizeof(T) * count);
}


