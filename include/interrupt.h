#ifndef _INTERRUPT_H_
#define _INTERRUPT_H_

#include <exception>
#include <iostream>

const uint64_t MASK_INTERRUPT_BIT = 1ull << 63;

class RISCVInterrupt: public std::exception {
public:
	RISCVInterrupt(uint64_t code): code_(code) {}
	uint64_t code() { return code_; }

	virtual const char * what() = 0;
protected:
	uint64_t code_;
};

class SupervisorSoftwareInterrupt: public RISCVInterrupt {
public:
	SupervisorSoftwareInterrupt(): RISCVInterrupt(1 | MASK_INTERRUPT_BIT) {}
	const char * what() { return "Supervisor Software Interrupt"; }
};

class MachineSoftwareInterrupt: public RISCVInterrupt {
public:
	MachineSoftwareInterrupt(): RISCVInterrupt(3 | MASK_INTERRUPT_BIT) {}
	const char * what() { return "Machine Software Interrupt"; }
};

class SupervisorTimerInterrupt: public RISCVInterrupt {
public:
	SupervisorTimerInterrupt(): RISCVInterrupt(5 | MASK_INTERRUPT_BIT) {}
	const char * what() { return "Supervisor Timer Interrupt"; }
};

class MachineTimerInterrupt: public RISCVInterrupt {
public:
	MachineTimerInterrupt(): RISCVInterrupt(7 | MASK_INTERRUPT_BIT) {}
	const char * what () { return "Machine Timer Interrupt"; }
};

class SupervisorExternalInterrupt: public RISCVInterrupt {
public:
	SupervisorExternalInterrupt(): RISCVInterrupt(9 | MASK_INTERRUPT_BIT) {}
	const char * what() { return "Supervisor External Interrupt"; }
};

class MachineExternalInterrupt: public RISCVInterrupt {
public:
	MachineExternalInterrupt(): RISCVInterrupt(11 | MASK_INTERRUPT_BIT) {}
	const char * what () { return "Machine External Interrupt"; }
};

#endif