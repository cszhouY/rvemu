#ifndef _CPU_H_
#define _CPU_H_

#include "param.h"
#include "exception.h"
#include "Bus.h"
#include "CSR.h"

#include <vector>
#include <sstream>
#include <cstdint>
#include <iostream>
#include <iomanip>

static const std::string RVABI[32] = {
    "zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2", 
    "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5", 
    "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7", 
    "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6",
};

enum Reg_t {
    ZERO = 0, RA = 1, SP, GP, TP, T0, T1, T2, S0, S1, A0, A1, A2, A3, A4, A5, 
    A6, A7, S2, S3, S4, S5, S6, S7, S8, S9, S10, S11, T3, T4, T5, T6 
};

// Riscv Privilege Mode
typedef uint64_t Mode;
const Mode user_mode = 0b00;
const Mode supervisor_mode = 0b01;
const Mode machine_mode = 0b11;

class CPU {
public:
	CPU(std::vector<uint8_t>& code) : bus(code), mode(machine_mode) {
        for(int i = 0; i < 32; ++i) {
        	regs[i] = 0;
        }
        regs[2] = DRAM_END;
        pc = DRAM_BASE;
    }

    // Load a value from a dram.
    uint64_t load(uint64_t addr, uint64_t size) {
        return bus.load(addr, size);
    }

    // Store a value to a dram.
    void store(uint64_t addr, uint64_t size, uint64_t value) {
        bus.store(addr, size, value);
    }

    // Get an instruction from the dram.
    uint64_t fetch() {
        return bus.load(pc, 32);
    } 

    uint64_t get_reg_value(Reg_t t) const {
        return regs[t];
    }

    uint64_t get_csr_value(size_t t) {
        return csr.load(t);
    }

    uint64_t get_pc_value() const {
        return pc;
    }

    uint64_t execute(uint64_t inst);

	void circle() {
		try{
            while (pc <= DRAM_END) {
                uint32_t inst = 0, new_pc = 0;
    			inst = fetch();
                new_pc = execute(inst);
                pc = new_pc;
            }
		}catch(const IllegalInstruction & e) {
			std::cerr << e.what() << "#" << std::hex << std::setw(8) << std::setfill('0') << e.illegal_inst() << std::endl;
		}catch(const StoreAMOAccessFault& e) {
            std::cerr << e.what() << std::endl;
        }catch(const LoadAccessFault & e) {
            std::cerr << e.what() << std::endl;
        }
	}

	void dump_registers() {
	    std::cout << std::string(80, '-') << std::endl;
	    std::stringstream output;
	    regs[0] = 0;
	    for (int i = 0; i < 32; i += 4) {
	        output << "x" << std::dec << i   << "(" << RVABI[i]   << ") = " << std::hex << regs[i]   << " "
	               << "x" << std::dec << i+1 << "(" << RVABI[i+1] << ") = " << std::hex << regs[i+1] << " "
	               << "x" << std::dec << i+2 << "(" << RVABI[i+2] << ") = " << std::hex << regs[i+2] << " "
	               << "x" << std::dec << i+3 << "(" << RVABI[i+3] << ") = " << std::hex << regs[i+3] << " "
	               << std::endl; 
	    }
	    std::cout << output.str();
	}

	inline uint64_t update_pc() {
		return pc + 4;
	}
private:
    // 32 64-bit integer registers.
    uint64_t regs[32];
    // pc register contains the memory address of next instruction
    uint64_t pc;
    // System bus that transfers data between CPU and peripheral devices.
    Bus bus;
    // Control and status registers. RISC-V ISA sets aside a 12-bit encoding space (csr[11:0]) for
    // up to 4096 CSRs.
    CSR csr;
    // The current priviledge mode.
    Mode mode;
};

uint64_t CPU::execute(uint64_t inst) {
    uint64_t opcode = inst & 0x7f;
    uint64_t rd = (inst >> 7) & 0x1f;
    uint64_t rs1 = (inst >> 15) & 0x1f;
    uint64_t rs2 = (inst >> 20) & 0x1f;
    uint64_t funct3 = (inst >> 12) & 0x7;
    uint64_t funct7 = (inst >> 25) & 0x7f;


    // x0 is hardwired zero
    regs[0] = 0;

    switch (opcode) {
        case 0x03: { // LOAD
        	// offset[11:0] = inst[31:20]
        	uint64_t offset = (uint64_t)((int64_t)(int32_t)inst >> 20);
        	uint64_t addr = regs[rs1] + offset;
            switch (funct3) {
                case 0x00: { // LB
                    uint64_t val = load(addr, 8);
                    regs[rd] = (uint64_t)(int64_t)(int8_t)val;
                    return update_pc();
                }
                case 0x01: { // LH
                    uint64_t val = load(addr, 16);
                    regs[rd] = (uint64_t)(int64_t)(int16_t)val;
                    return update_pc();
                }
                case 0x02: { // LW
                    uint64_t val = load(addr, 16);
                    regs[rd] = (uint64_t)(int64_t)(int32_t)val;
                    return update_pc();
                }
                case 0x03: { // LD
                	uint64_t val = load(addr, 32);
                    regs[rd] = (uint64_t)(int64_t)(int64_t)val;
                    return update_pc();
                }
                case 0x04: { // LBU
                    uint64_t val = load(addr, 8);
                    regs[rd] = val;
                    return update_pc();
                }
                case 0x05: { // LHU
                    uint64_t val = load(addr, 16);
                    regs[rd] = val;
                    return update_pc();
                }
                case 0x06: {// LWU
                	uint64_t val = load(addr, 32);
                    regs[rd] = val;
                    return update_pc();
                }
                default: {
                    throw IllegalInstruction(inst);
                }
            }
        }
        case 0x0f: {
            // A fence instruction does nothing because this emulator executes an
            // instruction sequentially on a single thread.
            if (0x0 == funct3) {  // FENCE
                // Do nothing.
                return update_pc();
            }
            else {
                throw IllegalInstruction(inst);
            }
        }
        case 0x13: { // OP-IMM
        	uint64_t imm = (uint64_t)((int64_t)(int32_t)(inst & 0xfff00000) >> 20);
            uint32_t shamt = (uint32_t)(imm & 0x3f);
            switch (funct3) {
                case 0x00: { // ADDI
                    regs[rd] = regs[rs1] + imm;
                    return update_pc();
                }
                case 0x01: { // SLLI
                    regs[rd] = regs[rs1] << shamt;
                    return update_pc();
                }
                case 0x02: { // SLTI
                    regs[rd] = ((int64_t)regs[rs1] < (int64_t)imm ? 1 : 0);
                    return update_pc();
                }
                case 0x03: { // SLTIU
                    regs[rd] = (regs[rs1] < imm ? 1 : 0);
                    return update_pc();
                }
                case 0x04: { // XORI
                    regs[rd] = regs[rs1] ^ imm;
                    return update_pc();
                }
                case 0x05: { // SRLI/SRAI
                    switch (funct7) {
                        case 0x00: {// SRLI
                            regs[rd] = regs[rs1] >> shamt;
                            return update_pc();
                        }
                        case 0x20: { // SRAI
                            regs[rd] = (uint64_t)((int64_t)regs[rs1] >> shamt);
                            return update_pc();
                        }
                        default: {
                            throw IllegalInstruction(inst);
                        }
                    }
                }
                case 0x06: { // ORI
                    regs[rd] = regs[rs1] | imm;
                    return update_pc();
                }
                case 0x07: { // ANDI
                    regs[rd] = regs[rs1] & imm;
                    return update_pc();
                }
                default: {
                    throw IllegalInstruction(inst);
                }
            }
        }
        case 0x17: { // AUIPC
            uint64_t imm = (uint64_t)(int64_t)(int32_t)(inst & 0xfffff000);
            regs[rd] = pc + imm;
            return update_pc();
        }
        case 0x1b: {
        	uint64_t imm = (uint64_t)((int64_t)(int32_t)inst >> 20);
        	// "SLLIW, SRLIW, and SRAIW encodings with imm[5] ̸= 0 are reserved."
        	uint32_t shamt = (uint32_t)(imm & 0x1f);
        	switch(funct3) {
        	case 0x0: { // ADDIW
        		regs[rd] = (uint64_t)(int64_t)(int32_t)(regs[rs1] + imm);
        		return update_pc();
            }
        	case 0x1: { // SLLIW
        		regs[rd] = (uint64_t)(int64_t)(int32_t)(regs[rs1] << shamt);
        		return update_pc();
            }
        	case 0x5: {
        		switch(funct7) {
        		case 0x00: { // SRLIW
        			regs[rd] = (uint64_t)(int64_t)(int32_t)((uint32_t)regs[rs1] >> shamt);
        			return update_pc();
                }
        		case 0x20: { // SRAIW
        			regs[rd] = (uint64_t)(int64_t)((int32_t)regs[rs1] >> shamt);
        			return update_pc();
                }
        		default: {
        			throw IllegalInstruction(inst);
                }
        		}
            }
        	default:{
        		throw IllegalInstruction(inst);
            }
        	}
        }
        case 0x23: { // STORE
            // imm[11:5|4:0] = inst[31:25|11:7]
            uint64_t imm = (uint64_t)((int64_t)(int32_t)(inst & 0xfe000000) >> 20) | ((inst >> 7) & 0x1f);
            uint64_t addr = regs[rs1] + imm; 
            switch (funct3) {
                case 0x00: { // SB
                    store(addr, 8, regs[rs2]);
                    return update_pc();
                }
                case 0x01: { // SH
                    store(addr, 16, regs[rs2]);
                    return update_pc();
                }
                case 0x02: { // SW
                    store(addr, 32, regs[rs2]);
                    return update_pc();
                }
                case 0x03: { // SD
                	store(addr, 64, regs[rs2]);
                	return update_pc();
                }
                default: {
                    throw IllegalInstruction(inst);
                }
            }
        }
        case 0x2f: {
            // RV64A: "A" standard extension for atomic instructions
            uint64_t funct5 = (funct7 & 0b1111100) >> 2;
            uint64_t _aq = (funct7 & 0b0000010) >> 1;
            uint64_t _rl = funct7 & 0b0000001;
            if (0x2 == funct3 && 0x00 == funct5) {  // amoadd.w
                uint64_t t = load(regs[rs1], 32);
                store(regs[rs1], 32, t + regs[rs2]);
                regs[rd] = t;
                return update_pc();
            }
            else if (0x3 == funct3 && 0x00 == funct5) {  // amoadd.d
                uint64_t t = load(regs[rs1], 64);
                store(regs[rs1], 64, t + regs[rs2]);
                regs[rd] = t;
                return update_pc();
            }
            else if (0x2 == funct3 && 0x01 == funct5) {  // amoswap.w
                uint64_t t = load(regs[rs1], 32);
                store(regs[rs1], 32, regs[rs2]);
                regs[rd] = t;
                return update_pc();
            }
            else if (0x3 == funct3 && 0x01 == funct5) {  // amoswap.d
                uint64_t t = load(regs[rs1], 64);
                store(regs[rs1], 64, regs[rs2]);
                regs[rd] = t;
                return update_pc();
            }
            else {
                throw IllegalInstruction(inst);
            }
        }
        case 0x33: { // OP
            // "SLL, SRL, and SRA perform logical left, logical right, and arithmetic right
            // shifts on the value in register rs1 by the shift amount held in register rs2.
            // In RV64I, only the low 6 bits of rs2 are considered for the shift amount."
            uint64_t shamt = (uint32_t)(uint64_t)(regs[rs2] & 0x3f);
            switch (funct3) {
                case 0x00: // ADD/SUB
                    switch (funct7) {
                        case 0x00: { // ADD
                            regs[rd] = regs[rs1] + regs[rs2];
                            return update_pc();
                        }
                        case 0x01: { //MUL
                        	regs[rd] = regs[rs1] * regs[rs2];
                        	return update_pc();
                        }
                        case 0x20: { // SUB
                            regs[rd] = regs[rs1] - regs[rs2];
                            return update_pc();
                        }
                        default: {
                            throw IllegalInstruction(inst);
                        }
                    }
                case 0x01: { // SLL
                    regs[rd] = regs[rs1] << shamt;
                    return update_pc();
                }
                case 0x02: { // SLT
                    regs[rd] = ((int64_t)regs[rs1] < (int64_t)regs[rs2] ? 1 : 0);
                    return update_pc();
                }
                case 0x03: { // SLTU
                    regs[rd] = (regs[rs1] < regs[rs2] ? 1 : 0);
                    return update_pc();
                }
                case 0x04: { // XOR
                    regs[rd] = regs[rs1] ^ regs[rs2];
                    return update_pc();
                }
                case 0x05: { // SRL/SRA
                    switch (funct7) {
                        case 0x00: { // SRL
                            regs[rd] = regs[rs1] >> shamt;
                            return update_pc();
                        }
                        case 0x20: { // SRA
                            regs[rd] = (uint64_t)((int64_t)regs[rs1] >> shamt);
                            return update_pc();
                        }
                        default: {
                            throw IllegalInstruction(inst);
                        }
                    }
                }
                case 0x06: { // OR
                    regs[rd] = regs[rs1] | regs[rs2];
                    return update_pc();
                }
                case 0x07: { // AND
                    regs[rd] = regs[rs1] & regs[rs2];
                    return update_pc();
                }
                default: {
                    throw IllegalInstruction(inst);
                }
            }
        }
        case 0x37: { // LUI
            regs[rd] = (uint64_t)(int64_t)(int32_t)(inst & 0xfffff000);
            return update_pc();
        }
        case 0x3b: {
            // "The shift amount is given by rs2[4:0]."
            uint32_t shamt = (uint32_t)(regs[rs2] & 0x1f); 
        	switch(funct3) {
        	case 0x0: {
        		if(0x00 == funct7) {  // ADDW
                    regs[rd] = (uint64_t)(int64_t)(int32_t)(regs[rs1] + regs[rs2]);
                    return update_pc();
        		}
        		else if(0x20 == funct7) { // SUBW
                    regs[rd] = (uint64_t)(int64_t)(int32_t)(regs[rs1] - regs[rs2]);
                    return update_pc();
        		}
        		else {
        			throw IllegalInstruction(inst);
        		}
            }
        	case 0x1: { // SLLW
                if(0x00 != funct7) {
                    throw IllegalInstruction(inst);
                }
        		regs[rd] = (uint64_t)(int32_t)((uint32_t)regs[rs1] << shamt);
        		return update_pc();
            }
        	case 0x5: {
        		if (0x00 == funct7) {  // SRLW
                    regs[rd] = (uint64_t)(int32_t)((uint32_t)regs[rs1] >> shamt);
                    return update_pc();
        		}
                else if (0x01 == funct7) {  //DIVU
                    regs[rd] = (0 == regs[rs2] ? 0xffffffff'ffffffff : regs[rs1] / regs[rs2]);
                    return update_pc();
                }
        		else if (0x20 == funct7) { // SRAW
                    regs[rd] = (uint64_t)((int32_t)regs[rs1] >> (int32_t)shamt);
                    return update_pc();
        		}
        		else {
        			throw IllegalInstruction(inst);
        		}
            }
            case 0x7: {
                if (0x01 == funct7) {  // REMUW
                    regs[rd] = (0 == regs[rs2] ? regs[rs1] : (uint64_t)(int32_t)((uint32_t)regs[rs1] % (uint32_t)regs[rs2]));
                    return update_pc();
                }
                else {
                    throw IllegalInstruction(inst);
                }
            }
        	default: {
        		throw IllegalInstruction(inst);
            }
        	}
        }
        case 0x63: { // BRANCH
            // imm[12|10:5|4:1|11] = inst[31|30:25|11:8|7]
            uint64_t imm = (uint64_t)((int64_t)(int32_t)(inst & 0x80000000) >> 19)
                        | ((inst & 0x80) << 4)   // imm[11]
                        | ((inst >> 20) & 0x7e0) // imm[10:5]
                        | ((inst >> 7) & 0x1e);  // imm[4:1]
            switch (funct3) {
                case 0x00: { // BEQ
                    if(regs[rs1] == regs[rs2]) {
                        return pc + imm;
                    }
                    return update_pc();
                }
                case 0x01: { // BNE
                    if(regs[rs1] != regs[rs2]) {
                        return pc + imm;
                    }
                    return update_pc();
                }
                case 0x04: { // BLT
                    if((int64_t)regs[rs1] < (int64_t)regs[rs2]) {
                        return pc + imm;
                    }
                    return update_pc();
                }
                case 0x05: { // BGE
                    if((int64_t)regs[rs1] >= (int64_t)regs[rs2]) {
                        return pc + imm;
                    }
                    return update_pc();
                }
                case 0x06: { // BLTU
                    if(regs[rs1] < regs[rs2]) {
                        return pc + imm;
                    }
                    return update_pc();
                }
                case 0x07: { // BGEU
                    if(regs[rs1] >= regs[rs2]) {
                        return pc + imm;
                    }
                    return update_pc();
                }
                default: {
                    throw IllegalInstruction(inst);
                }
            }
        }
        case 0x67: {// JALR
            uint64_t t = pc + 4;
            uint64_t imm = (uint64_t)((int64_t)(int32_t)(inst & 0xfff00000) >> 20);
            uint64_t new_pc = (regs[rs1] + imm) & (~1);
            regs[rd] = t;
            return new_pc;
        }
        case 0x6f: { // JAL
            regs[rd] = pc + 4;
            // imm[20|10:1|11|19:12] = inst[31|30:21|20|19:12]
            uint64_t imm = (uint64_t)((int64_t)(int32_t)(inst & 0x80000000) >> 11)
                         | (inst & 0xff000)         // imm[19:12]
                         | ((inst >> 9) & 0x800)    // imm[11]
                         | ((inst >> 20) & 0x7fe);  // imm[10:1]
            return pc + imm;
        }
        case 0x73: {
            size_t csr_addr = (size_t)((inst & 0xfff00000) >> 20);
            switch(funct3) {
            case 0x0: {
                if (0x2 == rs2 && 0x8 == funct7) {  // SRET
                    // When the SRET instruction is executed to return from the trap
                    // handler, the privilege level is set to user mode if the SPP
                    // bit is 0, or supervisor mode if the SPP bit is 1. The SPP bit
                    // is SSTATUS[8].
                    uint64_t sstatus = csr.load(SSTATUS);
                    mode = (sstatus & MASK_SPP) >> 8;
                    // The SPIE bit is SSTATUS[5] and the SIE bit is the SSTATUS[1]
                    uint64_t spie = (sstatus & MASK_SPIE) >> 5;
                    // set SIE = SPIE
                    sstatus = (sstatus & (~MASK_SIE)) | (spie << 1);
                    // set SPIE = 1
                    sstatus |= MASK_SPIE;
                    // set SPP the least priviledge mode (u-mode)
                    sstatus &= ~MASK_SPP;
                    csr.store(SSTATUS, sstatus);
                    // set the pc to CSRs[sepc].
                    // whenever IALIGN=32, bit sepc[1] is masked on reads so that it appears to be 0. This
                    // masking occurs also for the implicit read by the SRET instruction. 
                    uint64_t new_pc = csr.load(SEPC) & (~0b11);
                    return new_pc;
                }
                else if (0x2 == rs2 && 0x18 == funct7) {  // MRET
                    uint64_t mstatus = csr.load(MSTATUS);
                    // MPP is two bits wide at MSTATUS[12:11]
                    mode = (mstatus & MASK_MPP) >> 11;
                    // The MPIE bit is MSTATUS[7] and the MIE bit is the MSTATUS[3].
                    uint64_t mpie = (mstatus & MASK_MPIE) >> 7;
                    // set MIE = MPIE
                    mstatus = (mstatus & (~MASK_MIE)) | (mpie << 3);
                    // set MPIE = 1
                    mstatus |= MASK_MPIE;
                    // set MPP the least priviledge mode (u-mode)
                    mstatus &= ~MASK_MPP;
                    // if MPP != M, set MPRV = 0
                    mstatus &= ~MASK_MPRV;
                    csr.store(MSTATUS, mstatus);
                    // set the pc to CSRs[mepc].
                    uint64_t new_pc = csr.load(MEPC) & (~0b11);
                    return new_pc;
                }
                else if (funct7 = 0x9) {  // SFENCE.VMA
                    // Do nothing.
                    return update_pc();
                }
                else {
                    throw IllegalInstruction(inst);
                }
            }
            case 0x1: {  // CSRRW
                uint64_t t = csr.load(csr_addr);
                csr.store(csr_addr, regs[rs1]);
                regs[rd] = t;
                return update_pc();
            }
            case 0x2: {  // CSRRS
                uint64_t t = csr.load(csr_addr);
                csr.store(csr_addr, t | regs[rs1]);
                regs[rd] = t;
                return update_pc();
            }
            case 0x3: {  // CSRRC
                uint64_t t = csr.load(csr_addr);
                csr.store(csr_addr, t & (~regs[rs1]));
                regs[rd] = t;
                return update_pc();
            }
            case 0x5: {  // CSRRWI
                uint64_t zimm = (uint64_t)rs1;
                uint64_t t = csr.load(csr_addr);
                csr.store(csr_addr, zimm);
                regs[rd] = t;
                return update_pc();
            }
            case 0x6: { // CSRRSI
                uint64_t zimm = (uint64_t)rs1;
                uint64_t t = csr.load(csr_addr);
                csr.store(csr_addr, t | zimm);
                regs[rd] = t;
                return update_pc();
            }
            case 0x7: { // CSRRCI
                uint64_t zimm = (uint64_t)rs1;
                uint64_t t = csr.load(csr_addr);
                csr.store(csr_addr, t & (~zimm));
                regs[rd] = t;
                return update_pc();
            }
            default: {
                throw IllegalInstruction(inst);
            }
        }
        } 
        default: {
            throw IllegalInstruction(inst);
        }
    }
}
      

#endif