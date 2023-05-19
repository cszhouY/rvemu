#ifndef _EXCEPTION_H_
#define _EXCEPTION_H_


#include <stdexcept>

// Riscv Standard Exception
// enum Exception {
// 	LoadAccessFault,
//     StoreAMOAccessFault,
//     IllegalInstruction
// };

class RISCVException: std::exception {
public:
	RISCVException(uint64_t code, uint64_t value): code_(code), value_(value) {}
	int code() const { return code_; }
	uint64_t value() const { return value_; }

	virtual const char * what() const throw () = 0;
	virtual bool is_fatal() = 0;
protected:
	uint64_t code_;
	uint64_t value_;
};

class InstructionAddrMisaligned: public RISCVException {
public:
	InstructionAddrMisaligned(uint64_t addr): RISCVException(0, addr) {}
	const char * what() const throw () { return "Instruction Address Misaligned"; }
	bool is_fatal() { return true; }
};

class InstructionAccessFault: public RISCVException {
public:
	InstructionAccessFault(uint64_t addr): RISCVException(1, addr) {}
	const char * what() { return "Instruction Access Fault"; }
	bool is_fatal() { return true; }
};

class IllegalInstruction: public RISCVException {
public:
	IllegalInstruction(uint64_t inst): RISCVException(2, inst) {}
	const char * what () const throw () { return "Illegal Instruction"; }
	bool is_fatal() { return true; }
};

class Breakpoint: public RISCVException {
public:
	Breakpoint(uint64_t pc): RISCVException(3, pc) {}
	const char * what() { return "Breakpoint"; }
	bool is_fatal() { return false; }
};

class LoadAddrMisaligned: public RISCVException {
public:
	LoadAddrMisaligned(uint64_t addr): RISCVException(4, addr) {}
	const char * what() { return "Load Address Misaligned"; }
	bool is_fatal() { return false; }
};

class LoadAccessFault: public RISCVException {
public:
	LoadAccessFault(uint64_t addr): RISCVException(5, addr) {}
	const char * what () const throw () { return "Load Access Fault"; }
	bool is_fatal() { return true; };
};

class StoreAMOAddrMisaligned: public RISCVException {
public:
	StoreAMOAddrMisaligned(uint64_t addr): RISCVException(6, addr) {}
	const char * what() const throw () {return "Store/AMO Address Misaligned";}
	bool is_fatal() { return true; }
};

class StoreAMOAccessFault: public RISCVException {
public:
	StoreAMOAccessFault(uint64_t addr): RISCVException(7, addr) {}
	const char * what () const throw () { return "Store/AMO Access Fault"; }
	bool is_fatal() { return true; }
};

class EnvironmentCallFromUMode: public RISCVException {
public:
	EnvironmentCallFromUMode(uint64_t pc): RISCVException(8, pc) {}
	const char * what () const throw () { return "Environment Call From U-Mode"; }
	bool is_fatal() { return false; }
};

class EnvironmentCallFromSMode: public RISCVException {
public:
	EnvironmentCallFromSMode(uint64_t pc): RISCVException(9, pc) {}
	const char * what () const throw () { return "Environment Call From S-Mode"; }
	bool is_fatal() { return false; }
};

class EnvironmentCallFromMMode: public RISCVException {
public:
	EnvironmentCallFromMMode(uint64_t pc): RISCVException(11, pc) {}
	const char * what () const throw () { return "Environment Call From M-Mode"; }
	bool is_fatal() { return false; }
};

class InstructionPageFault: public RISCVException {
public:
	InstructionPageFault(uint64_t addr): RISCVException(12, addr) {}
	const char * what() const throw () { return "Instruction Page Fault"; }
	bool is_fatal() { return false; }
};

class LoadPageFault: public RISCVException {
public:
	LoadPageFault(uint64_t addr): RISCVException(13, addr) {}
	const char * what() const throw () { return "Load Page Fault"; }
	bool is_fatal() { return false; }
};

class StoreAMOPageFault: public RISCVException {
public:
	StoreAMOPageFault(uint64_t addr): RISCVException(15, addr) {}
	const char * what() const throw () {return "StoreAMOPageFault"; }
	bool is_fatal() { return false; }
};

#endif