#include "FileIO.hpp"
#include <fstream>

std::string FileIO::read_file(const std::string& file_name_)
{
	std::ifstream ifs(file_name_);
	std::string content((std::istreambuf_iterator<char>(ifs)),
		(std::istreambuf_iterator<char>()));

	return content;
}