#ifndef _GENERATOR_H_
#define _GENERATOR_H_

#include <string>
#include <sstream>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <vector>
#include <memory>

class Generator {
public:
	static std::string command_process(const std::string & command) {
		FILE* pipe = popen(command.c_str(), "r");
		std::string res;
		char buffer[128];
		while (fgets(buffer, 128, pipe) != NULL) {
			res += buffer;
		}
		pclose(pipe);
		return res;
	}

	static bool write_rv_src(const std::string & src_str, const std::string & filename) {
		// create directory for test cases if not exists 
		if(!std::filesystem::exists("test")){ 
			if (!std::filesystem::create_directory("test")) {
				std::cerr << "Failed to create directory." << std::endl;
	            return false;
	        }
		}
		// write src_str
		std::string filepath = "./test/" + filename;
		std::ofstream outfile(filepath);
		if (outfile.is_open()) {
			outfile << src_str;
			outfile.close();
		} else {
			std::cerr << "Failed to open file.\n";
			return false;
		}
		return true;
	}

	static bool generate_rv_assembly(const std::string & src_filename, const std::string & out_filename) {
		std::string srcfile = "./test/" + src_filename;
		std::string outfile = "./test/" + out_filename;
		std::string cc = "clang";
		std::string command = cc + " -target riscv64-unknown-elf -S -march=rv64g -mabi=lp64 -mno-relax " 
								+ srcfile + " -o " + outfile;
		std::string res = command_process(command);
		if(!res.empty()) {
			std::cout << res << std::endl;
			return false;
		}
		return true;
	}

	static bool generate_rv_obj(const std::string & src_filename, const std::string & out_filename) {
		std::string srcfile = "./test/" + src_filename;
		std::string outfile = "./test/" + out_filename;
		std::string cc = "clang";
		std::string command = cc + " -target riscv64-unknown-elf -c -march=rv64g -mabi=lp64 -mno-relax " 
								+ srcfile + " -o " + outfile;
		std::string res = command_process(command);
		if(!res.empty()) {
			std::cout << res << std::endl;
			return false;
		}
		return true;
	}

	static bool generate_rv_binary(const std::string & src_filename, const std::string & out_filename) {
		std::string srcfile = "./test/" + src_filename;
		std::string outfile = "./test/" + out_filename;
		std::string objcopy = "llvm-objcopy";
		std::string command = objcopy + " -O binary " + srcfile + " " + outfile;
		std::string res = command_process(command); 
		if(!res.empty()) {
			std::cout << res << std::endl;
			return false;
		}
		return true;
	}
};

#endif