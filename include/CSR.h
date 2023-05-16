#ifndef _CSR_H_
#define _CSR_H_

#include <cstring>

const size_t NUM_CSRS = 4096;
// Machine-level CSRs.

// Hardware thread ID.
const size_t MHARTID = 0xf14;
// Machine status register.
const size_t MSTATUS = 0x300;
// Machine exception delefation register.
const size_t MEDELEG = 0x302;
// Machine interrupt delefation register.
const size_t MIDELEG = 0x303;
// Machine interrupt-enable register.
const size_t MIE = 0x304;
// Machine trap-handler base address.
const size_t MTVEC = 0x305;
// Machine counter enable.
const size_t MCOUNTEREN = 0x306;
// Scratch register for machine trap handlers.
const size_t MSCRATCH = 0x340;
// Machine exception program counter.
const size_t MEPC = 0x341;
// Machine trap cause.
const size_t MCAUSE = 0x342;
// Machine bad address or instruction.
const size_t MTVAL = 0x343;
// Machine interrupt pending.
const size_t MIP = 0x344;


// Supervisor-level CSRs.
// Supervisor status register.
const size_t SSTATUS = 0x100;
// Supervisor interrupt-enable register.
const size_t SIE = 0x104;
// Supervisor trap handler base address.
const size_t STVEC = 0x105;
// Scratch register for supervisor trap handlers.
const size_t SSCRATCH = 0x140;
// Supervisor exception program counter.
const size_t SEPC = 0x141;
// Supervisor trap cause.
const size_t SCAUSE = 0x142;
// Supervisor bad address or instruction.
const size_t STVAL = 0x143;
// Supervisor interrupt pending.
const size_t SIP = 0x144;
// Supervisor address translation and protection.
const size_t SATP = 0x180;


// mstatus and sstatus field mask
const uint64_t MASK_SIE = 1ull << 1;
const uint64_t MASK_MIE = 1ull << 3;
const uint64_t MASK_SPIE = 1ull << 5;
const uint64_t MASK_UBE = 1ull << 6;
const uint64_t MASK_MPIE = 1ull << 7;
const uint64_t MASK_SPP = 1ull << 8;
const uint64_t MASK_VS = 0b11ull << 9;
const uint64_t MASK_MPP = 0b11ull << 11;
const uint64_t MASK_FS = 0b11ull << 13;
const uint64_t MASK_XS = 0b11ull << 15;
const uint64_t MASK_MPRV = 1ull << 17;
const uint64_t MASK_SUM = 1ull << 18;
const uint64_t MASK_MXR = 1ull << 19;
const uint64_t MASK_TVM = 1ull << 20;
const uint64_t MASK_TW = 1ull << 21;
const uint64_t MASK_TSR = 1ull << 22;
const uint64_t MASK_UXL = 0b11ull << 32;
const uint64_t MASK_SXL = 0b11ull << 34;
const uint64_t MASK_SBE = 1ull << 36;
const uint64_t MASK_MBE = 1ull << 37;
const uint64_t MASK_SD = 1ull << 63;
const uint64_t MASK_SSTATUS = MASK_SIE | MASK_SPIE | MASK_UBE | MASK_SPP | MASK_FS
| MASK_XS  | MASK_SUM  | MASK_MXR | MASK_UXL | MASK_SD;
// MIP / SIP field mask
const uint64_t MASK_SSIP = 1ull << 1;
const uint64_t MASK_MSIP = 1ull << 3;
const uint64_t MASK_STIP = 1ull << 5;
const uint64_t MASK_MTIP = 1ull << 7;
const uint64_t MASK_SEIP = 1ull << 9;
const uint64_t MASK_MEIP = 1ull << 11;

class CSR {
public:
	CSR() {
		csrs = new uint64_t [NUM_CSRS];
		memset(csrs, 0, NUM_CSRS * sizeof(uint64_t));
	}

	~CSR() {
		delete [] csrs;
	}
	uint64_t load(size_t addr) {
		switch(addr) {
		case SIE:
			return csrs[MIE] & csrs[MIDELEG];
		case SIP: 
			return csrs[MIP] & csrs[MIDELEG];
		case SSTATUS:
			return csrs[MSTATUS] & MASK_SSTATUS;
		default:
			return csrs[addr];
		}
	}

	void store(size_t addr, uint64_t value) {
		switch (addr) {
        case SIE:
            csrs[MIE] = (csrs[MIE] & ~csrs[MIDELEG]) | (value & csrs[MIDELEG]);
            break;
        case SIP:
        	/// MIP OR MIE ?
            csrs[MIP] = (csrs[MIP] & ~csrs[MIDELEG]) | (value & csrs[MIDELEG]);
            break;
        case SSTATUS:
            csrs[MSTATUS] = (csrs[MSTATUS] & ~MASK_SSTATUS) | (value & MASK_SSTATUS);
            break;
        default:
            csrs[addr] = value;
            break;
    	}
	}

	bool is_medelegated(uint64_t cause) {
		return (csrs[MEDELEG] >> cause) & 1;
	}

	bool is_midelegated(uint64_t cause) {
		return (csrs[MIDELEG] >> cause) & 1;
	}

private:
	uint64_t *csrs;
};

#endif