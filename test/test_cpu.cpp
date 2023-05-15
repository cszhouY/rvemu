#include "CPU.h"
#include "gtest/gtest.h"

#include <string>
#include <sstream>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <vector>
#include <memory>

std::string command_process(const std::string & command) {
	FILE* pipe = popen(command.c_str(), "r");
	std::string res;
	char buffer[128];
	while (fgets(buffer, 128, pipe) != NULL) {
		res += buffer;
	}
	pclose(pipe);
	return res;
}

bool write_rv_assembly(const std::string & asm_str, const std::string & filepath) {
	// create directory for test cases if not exists 
	if(!std::filesystem::exists("test")){ 
		if (!std::filesystem::create_directory("test")) {
			std::cerr << "Failed to create directory." << std::endl;
            return false;
        }
	}
	// write asm_str
	std::ofstream outfile(filepath);
	if (outfile.is_open()) {
		outfile << asm_str;
		outfile.close();
	} else {
		std::cerr << "Failed to open file.\n";
		return false;
	}
	std::cout << "write assembly file successful!" << std::endl;
	return true;
}

std::string generate_rv_assembly(const std::string & asm_str, const std::string & asm_name) {
	std::string asmfile = "./test/" + asm_name + ".S";
	std::string objfile = "./test/" + asm_name + ".o";
	std::string binfile = "./test/" + asm_name + ".bin";
	if(! write_rv_assembly(asm_str, asmfile)) {
		return "";
	}
	std::string cc = "clang", objcopy = "llvm-objcopy";
	std::string command1 = cc + " -target riscv64-unknown-elf -c -march=rv64g -mabi=lp64 -mno-relax " 
							+ asmfile + " -o " + objfile;
	std::string res1 = command_process(command1);
	if(!res1.empty()) {
		std::cout << res1 << std::endl;
		return "";
	}
	std::string command2 = objcopy + " -O binary " + objfile + " " + binfile;
	std::string res2 = command_process(command2); 
	if(!res2.empty()) {
		std::cout << res2 << std::endl;
		return "";
	}
	return binfile;
}

std::unique_ptr<CPU> get_cpu_test(const std::string & asm_str, const std::string & case_name) {
	std::string binfile = generate_rv_assembly(asm_str, case_name);
	if(binfile.empty()) {
		return nullptr;
	}
	std::ifstream file(binfile, std::ios::binary);
    if(!file){
        std::cerr << "open file error" << std::endl;
        return nullptr;
    }
    std::vector<uint8_t> code((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    std::unique_ptr<CPU> cpu = std::make_unique<CPU>(code);
    cpu->circle();
    return cpu;
}

TEST(test_inst, add) {
	std::stringstream asm_str;
	asm_str << "addi x31, x0, 42";
	std::unique_ptr<CPU> cpu = get_cpu_test(asm_str.str(), "addi");
	ASSERT_NE(cpu, nullptr);
	EXPECT_EQ(cpu->get_reg_value((Reg_t)31), 42);
}

TEST(test_inst, simple) {
	std::stringstream asm_str;
	asm_str << "addi sp, sp, -16\n"
			<< "sd s0, 8(sp)\n"
			<< "addi s0, sp, 16\n"
            << "li a5, 42\n"
            << "mv a0, a5\n"
            << "ld s0, 8(sp)\n"
            << "addi sp, sp, 16\n"
            << "jr ra";
    std::unique_ptr<CPU> cpu = get_cpu_test(asm_str.str(), "simple");
	ASSERT_NE(cpu, nullptr);
	EXPECT_EQ(cpu->get_reg_value(A0), 42);
}

TEST(test_inst, lui) {
	std::stringstream asm_str;
	asm_str << "lui a0, 42";
    std::unique_ptr<CPU> cpu = get_cpu_test(asm_str.str(), "lui");
	ASSERT_NE(cpu, nullptr);
	EXPECT_EQ(cpu->get_reg_value(A0), 42 << 12);
}

TEST(test_inst, auipc) {
	std::stringstream asm_str;
	asm_str << "auipc a0, 42";
    std::unique_ptr<CPU> cpu = get_cpu_test(asm_str.str(), "auipc");
	ASSERT_NE(cpu, nullptr);
	EXPECT_EQ(cpu->get_reg_value(A0), DRAM_BASE + (42 << 12));
}

TEST(test_inst, jal) {
	std::stringstream asm_str;
	asm_str << "jal a0, 42";
    std::unique_ptr<CPU> cpu = get_cpu_test(asm_str.str(), "jal");
	ASSERT_NE(cpu, nullptr);
	EXPECT_EQ(cpu->get_reg_value(A0), DRAM_BASE + 4);
	EXPECT_EQ(cpu->get_pc_value(), DRAM_BASE + 42);
}

TEST(test_inst, jalr) {
	std::stringstream asm_str;
	asm_str << "addi a1, zero, 42\n"
            << "jalr a0, -8(a1)";
    std::unique_ptr<CPU> cpu = get_cpu_test(asm_str.str(), "jalr");
	ASSERT_NE(cpu, nullptr);
	EXPECT_EQ(cpu->get_reg_value(A0), DRAM_BASE + 8);
	EXPECT_EQ(cpu->get_pc_value(), 34);
}

TEST(test_inst, beq) {
	std::stringstream asm_str;
	asm_str << "beq x0, x0, 42";
    std::unique_ptr<CPU> cpu = get_cpu_test(asm_str.str(), "beq");
	ASSERT_NE(cpu, nullptr);
	EXPECT_EQ(cpu->get_pc_value(), DRAM_BASE + 42);
}

TEST(test_inst, bne) {
	std::stringstream asm_str;
	asm_str << "addi x1, x0, 10\n"
            << "bne  x0, x1, 42";
    std::unique_ptr<CPU> cpu = get_cpu_test(asm_str.str(), "bne");
	ASSERT_NE(cpu, nullptr);
	EXPECT_EQ(cpu->get_pc_value(), DRAM_BASE + 42 + 4);
}

TEST(test_inst, blt) {
	std::stringstream asm_str;
	asm_str << "addi x1, x0, 10\n"
            << "addi x2, x0, 20\n"
            << "blt  x1, x2, 42";
    std::unique_ptr<CPU> cpu = get_cpu_test(asm_str.str(), "blt");
	ASSERT_NE(cpu, nullptr);
	EXPECT_EQ(cpu->get_pc_value(), DRAM_BASE + 42 + 8);
}

TEST(test_inst, bge) {
	std::stringstream asm_str;
	asm_str << "addi x1, x0, 10\n"
            << "addi x2, x0, 20\n"
            << "bge  x2, x1, 42";
    std::unique_ptr<CPU> cpu = get_cpu_test(asm_str.str(), "bge");
	ASSERT_NE(cpu, nullptr);
	EXPECT_EQ(cpu->get_pc_value(), DRAM_BASE + 42 + 8);
}

TEST(test_inst, bltu) {
	std::stringstream asm_str;
	asm_str << "addi x1, x0, 10\n"
            << "addi x2, x0, 20\n"
            << "bltu x1, x2, 42";
    std::unique_ptr<CPU> cpu = get_cpu_test(asm_str.str(), "bltu");
	ASSERT_NE(cpu, nullptr);
	EXPECT_EQ(cpu->get_pc_value(), DRAM_BASE + 42 + 8);
}

TEST(test_inst, bgeu) {
	std::stringstream asm_str;
	asm_str << "addi x1, x0, 10\n"
            << "addi x2, x0, 20\n"
            << "bgeu x2, x1, 42";
    std::unique_ptr<CPU> cpu = get_cpu_test(asm_str.str(), "bgeu");
	ASSERT_NE(cpu, nullptr);
	EXPECT_EQ(cpu->get_pc_value(), DRAM_BASE + 42 + 8);
}

TEST(test_inst, store_and_load) {
	std::stringstream asm_str;
	asm_str << "addi s0, zero, 256\n"
            << "addi sp, sp, -16\n"
            << "sd   s0, 8(sp)\n"
            << "lb   t1, 8(sp)\n"
            << "lh   t2, 8(sp)";
    std::unique_ptr<CPU> cpu = get_cpu_test(asm_str.str(), "store_load");
	ASSERT_NE(cpu, nullptr);
	EXPECT_EQ(cpu->get_reg_value(T1), 0);
	EXPECT_EQ(cpu->get_reg_value(T2), 256);
}

TEST(test_inst, slt) {
	std::stringstream asm_str;
	asm_str << "addi t0, zero, 14\n"
            << "addi t1, zero, 24\n"
            << "slt  t2, t0, t1\n"
            << "slti t3, t0, 42\n"
            << "sltiu t4, t0, 84";
    std::unique_ptr<CPU> cpu = get_cpu_test(asm_str.str(), "slt");
	ASSERT_NE(cpu, nullptr);
	EXPECT_EQ(cpu->get_reg_value(T2), 1);
	EXPECT_EQ(cpu->get_reg_value(T3), 1);
	EXPECT_EQ(cpu->get_reg_value(T4), 1);
}

TEST(test_inst, xor) {
	std::stringstream asm_str;
	asm_str << "addi a0, zero, 0b10\n"
            << "xori a1, a0, 0b01\n"
            << "xor a2, a1, a1";
    std::unique_ptr<CPU> cpu = get_cpu_test(asm_str.str(), "xor");
	ASSERT_NE(cpu, nullptr);
	EXPECT_EQ(cpu->get_reg_value(A1), 3);
	EXPECT_EQ(cpu->get_reg_value(A2), 0);
}

TEST(test_inst, or) {
	std::stringstream asm_str;
	asm_str << "addi a0, zero, 0b10\n"
            << "ori  a1, a0, 0b01\n"
            << "or   a2, a0, a0";
    std::unique_ptr<CPU> cpu = get_cpu_test(asm_str.str(), "or");
	ASSERT_NE(cpu, nullptr);
	EXPECT_EQ(cpu->get_reg_value(A1), 0b11);
	EXPECT_EQ(cpu->get_reg_value(A2), 0b10);
}

TEST(test_inst, and) {
	std::stringstream asm_str;
	asm_str << "addi a0, zero, 0b10\n"
            << "andi a1, a0, 0b11\n"
            << "and  a2, a0, a1";
    std::unique_ptr<CPU> cpu = get_cpu_test(asm_str.str(), "and");
	ASSERT_NE(cpu, nullptr);
	EXPECT_EQ(cpu->get_reg_value(A1), 0b10);
	EXPECT_EQ(cpu->get_reg_value(A2), 0b10);
}

TEST(test_inst, sll) {
	std::stringstream asm_str;
	asm_str << "addi a0, zero, 1\n"
            << "addi a1, zero, 5\n"
            << "sll  a2, a0, a1\n"
            << "slli a3, a0, 5\n"
            << "addi s0, zero, 64\n"
            << "sll  a4, a0, s0";
    std::unique_ptr<CPU> cpu = get_cpu_test(asm_str.str(), "sll");
	ASSERT_NE(cpu, nullptr);
	EXPECT_EQ(cpu->get_reg_value(A2), 1 << 5);
	EXPECT_EQ(cpu->get_reg_value(A3), 1 << 5);
	EXPECT_EQ(cpu->get_reg_value(A4), 1);
}

TEST(test_inst, sra_srl) {
	std::stringstream asm_str;
	asm_str << "addi a0, zero, -8\n"
            << "addi a1, zero, 1\n"
            << "sra  a2, a0, a1\n"
            << "srai a3, a0, 2\n"
            << "srli a4, a0, 2\n"
            << "srl  a5, a0, a1";
    std::unique_ptr<CPU> cpu = get_cpu_test(asm_str.str(), "sra_srl");
	ASSERT_NE(cpu, nullptr);
	EXPECT_EQ(cpu->get_reg_value(A2), (uint64_t)(int64_t)(-4));
	EXPECT_EQ(cpu->get_reg_value(A3), (uint64_t)(int64_t)(-2));
	EXPECT_EQ(cpu->get_reg_value(A4), (uint64_t)(int64_t)(-8) >> 2);
	EXPECT_EQ(cpu->get_reg_value(A5), (uint64_t)(int64_t)(-8) >> 1);
}

TEST(test_inst, wordOp) {
	std::stringstream asm_str;
	asm_str << "addi a0, zero, 42\n" 
            << "lui  a1, 0x7f000\n"
            << "addw a2, a0, a1";
    std::unique_ptr<CPU> cpu = get_cpu_test(asm_str.str(), "sra_srl");
	ASSERT_NE(cpu, nullptr);
	EXPECT_EQ(cpu->get_reg_value(A2), 0x7f00002a);
}