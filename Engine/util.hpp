#ifndef UTIL
#define UTIL

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <stdexcept>


class utility {
public:

	//Custom Compile Shaders : May need to Change later 
	void compile_shaders(const std::string& _file_path_v , const std::string& _file_path_f) {

		std::cout << "[Compiling Shaders Started]" << std::endl;
		const std::string path_to_compiler = "C:\\VulkanSDK\\1.3.290.0\\Bin\\glslc.exe ";
		std::string code_formatter = path_to_compiler + _file_path_v + " -o Dep/Shader/vert.spv";
		std::system(code_formatter.c_str());
		code_formatter = path_to_compiler + _file_path_f + " -o Dep/Shader/frag.spv";
		std::system(code_formatter.c_str());
		std::cout << "[Compiling Shaders Done]" << std::endl;

	}

	std::vector<char> read_from_file(const std::string& path_to_file) {

		/*NOTE : The ate mode opens the file and points the pointer at the end of the file 
		while the binary opens the file in a binary format so that no weird typing is used
		
		file.tellg() is used to retrieve the location of the pointer in the file */

		std::ifstream file{ path_to_file, std::ios::ate | std::ios::binary };
		if (!file.is_open())
		{
			throw std::runtime_error("Failed to open file to read" + path_to_file);
		}
		size_t file_size = static_cast<size_t>(file.tellg());

		std::vector<char> storage_buffer(file_size);
		file.seekg(0);
		file.read(storage_buffer.data(), file_size);

		file.close();
		return storage_buffer;
	}	
};

#endif 