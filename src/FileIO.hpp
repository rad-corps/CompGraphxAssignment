#pragma once

#include <string>

class FileIO
{
public:
	static std::string read_file(const std::string& file_name_);

private:
	FileIO();
};