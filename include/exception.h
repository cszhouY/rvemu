#ifndef _EXCEPTION_H_
#define _EXCEPTION_H_


#include <stdexcept>

// Riscv Standard Exception
// enum Exception {
// 	LoadAccessFault,
//     StoreAMOAccessFault,
//     IllegalInstruction
// };

class IllegalInstruction: public std::exception {
public:
	IllegalInstruction(uint64_t inst): inst(inst) {}
	const char * what () const throw () { return "IllegalInstruction"; }
	uint64_t illegal_inst() const  { return inst; }
private:
	uint64_t inst;
};

class StoreAMOAccessFault: public std::exception {
public:
	StoreAMOAccessFault() {}
	const char * what () const throw () { return "StoreAMOAccessFault"; }
};

class LoadAccessFault: public std::exception {
public:
	LoadAccessFault() {}
	const char * what () const throw () { return "LoadAccessFault"; }
};

#endif