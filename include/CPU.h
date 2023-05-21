#ifndef _CPU_H_
#define _CPU_H_

#include "param.h"
#include "exception.h"
#include "Bus.h"
#include "CSR.h"
#include "interrupt.h"

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

    void handle_excption(RISCVException & e);

    void handle_interrupt(RISCVInterrupt & interrupt);

    void check_pending_interrupt();

	void circle() {
        uint32_t inst = 0, new_pc = 0;
        while (pc <= DRAM_END) {
            try {
                inst = fetch();
                new_pc = execute(inst);
                pc = new_pc;
                check_pending_interrupt();
            } catch (RISCVException & e) {
                handle_excption(e);
                if (e.is_fatal()) {
                    std::cout << "\033[1m\033[31m" << e.what() << "#" << std::hex << e.value() << "\033[0m" << std::endl;
                    break;
                }
                continue;
            } catch (RISCVInterrupt & interrupt) {
                handle_interrupt(interrupt);
            }
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

    inline void set_pc(uint64_t new_pc) {
        pc = new_pc;
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

/*!
 * the process to handle exception in S-mode and M-mode is similar.
 * include following steps:
 * 1. set xPP to current mode.
 * 2. update hart's privilege mode (M or S according to current mode and exception and exception setting).
 * 3. save current pc in epc (spec in S-mode, mpec in M-mode).
 * 4. set pc to trap vector (stvec in S-mode, mtvec in M-mode).
 * 5. set cause to exception code (scause in S-mode, mcause in M-mode).
 * 6. set trap value properly (stval in S-mode, mcause in M-mode).
 * 7. set xPIE to xPIE (SPIE in S-mode, MPIE Iin M-mode).
 * 8. clear up xIE (SIE IN S-mode, MIE in M-mode).
 * */
void CPU::handle_excption(RISCVException & e) {
    // Save current PC, mode, and cause
    uint64_t oldpc = pc, oldmode = mode;
    uint64_t cause = e.code();
    // If an exception happen in U-mode or S-mode, and the exception is delegated to S-mode.
    // then this exception should be handled in S-mode.
    bool trap_in_s_mode = (mode <= supervisor_mode) && csr.is_medelegated(cause);
    // Select the appropriate trap vector and status register values
    uint64_t STATUS, TVEC, CAUSE, TVAL, EPC, MASK_PIE, pie_i, MASK_IE, ie_i, MASK_PP, pp_i;
    if (trap_in_s_mode) {
        mode = supervisor_mode;
        STATUS = SSTATUS;
        TVEC = STVEC;
        CAUSE = SCAUSE;
        TVAL = STVAL;
        EPC = SEPC;
        MASK_PIE = MASK_SPIE;
        pie_i = 5;
        MASK_IE = MASK_SIE;
        ie_i = 1;
        MASK_PP = MASK_SPP;
        pp_i = 8;
    } else {
        mode = machine_mode;
        STATUS = MSTATUS;
        TVEC = MTVEC;
        CAUSE = MCAUSE;
        TVAL = MTVAL;
        EPC = MEPC;
        MASK_PIE = MASK_MPIE;
        pie_i = 7;
        MASK_IE = MASK_MIE;
        ie_i = 3;
        MASK_PP = MASK_MPP;
        pp_i = 11;
    }

    // 3.1.7 & 4.1.2
    // The BASE field in tvec is a WARL field that can hold any valid virtual or physical address,
    // subject to the following alignment constraints: the address must be 4-byte aligned
    pc = csr.load(TVEC) & (~0b11);

    // 3.1.14 & 4.1.7
    // When a trap is taken into S-mode (or M-mode), sepc (or mepc) is written with the virtual address 
    // of the instruction that was interrupted or that encountered the exception.
    csr.store(EPC, oldpc);

    // 3.1.15 & 4.1.8
    // When a trap is taken into S-mode (or M-mode), scause (or mcause) is written with a code indicating 
    // the event that caused the trap.
    csr.store(CAUSE, cause);

    // 3.1.16 & 4.1.9
    // If stval is written with a nonzero value when a breakpoint, address-misaligned, access-fault, or
    // page-fault exception occurs on an instruction fetch, load, or store, then stval will contain the
    // faulting virtual address.
    // If stval is written with a nonzero value when a misaligned load or store causes an access-fault or
    // page-fault exception, then stval will contain the virtual address of the portion of the access that
    // caused the fault
    csr.store(TVAL, e.value());

    // 3.1.6 covers both sstatus and mstatus.
    uint64_t status = csr.load(STATUS);

    // get SIE or MIE
    uint64_t ie = (status & MASK_IE) >> ie_i;

    // set SPIE = SIE / MPIE = MIE
    status = (status & (~MASK_PIE)) | (ie << pie_i);

    // set SIE = 0 / MIE = 0
    status &= (~MASK_IE);

    // set SPP / MPP = previous mode
    status = (status & ~MASK_PP) | (oldmode << pp_i);
    csr.store(STATUS, status);
}

/*!
 * The procedure of handling interrupt is similar to that of handling exception. 
 * 1. update hart's privilege mode (M or S according to current mode and exception setting).
 * 2. save current pc in epc (spec in S-mode, mpec in M-mode).
 * 3. set pc properly according to the MODE field of trap vector (stvex in S-mode, mtvec in M-mode).
 * 4. set cause register with exception code (Scause in S-mode, mcause in M-moded).
 * 5. set xPIE to xIE (SPIE in S-mode, MPIE in M-mode).
 * 6. clear up xIE (SIE in S-mode, MIE in M-mode).
 * 7. set xPP to previous mode. 
 * */
void CPU::handle_interrupt(RISCVInterrupt & interrupt) {
    uint64_t oldpc = pc, oldmode = mode;
    uint64_t cause = interrupt.code();
    // although cause contains a interrupt bit. Shift the cause make it out.
    bool trap_in_s_mode = (mode <= supervisor_mode) && csr.is_midelegated(cause);
    uint64_t STATUS, TVEC, CAUSE, TVAL, EPC, MASK_PIE, pie_i, MASK_IE, ie_i, MASK_PP, pp_i;
    if (trap_in_s_mode) {
        mode = supervisor_mode;
        STATUS = SSTATUS;
        TVEC = STVEC;
        CAUSE = SCAUSE;
        TVAL = STVAL;
        EPC = SEPC;
        MASK_PIE = MASK_SPIE;
        pie_i = 5;
        MASK_IE = MASK_SIE;
        ie_i = 1;
        MASK_PP = MASK_SPP;
        pp_i = 8;
    } else {
        mode = machine_mode;
        STATUS = MSTATUS;
        TVEC = MTVEC;
        CAUSE = MCAUSE;
        TVAL = MTVAL;
        EPC = MEPC;
        MASK_PIE = MASK_MPIE;
        pie_i = 7;
        MASK_IE = MASK_MIE;
        ie_i = 3;
        MASK_PP = MASK_MPP;
        pp_i = 11;
    }
    // 3.1.7 & 4.1.2
    // When MODE=Direct, all traps into machine mode cause the pc to be set to the address in the BASE field. 
    // When MODE=Vectored, all synchronous exceptions into machine mode cause the pc to be set to the address 
    // in the BASE field, whereas interrupts cause the pc to be set to the address in the BASE field plus four 
    // times the interrupt cause number.
    uint64_t tvec = csr.load(TVEC);
    uint64_t tvec_mode = tvec & 0b11;
    uint64_t tvec_base = tvec & (~0b11);
    if(0 == tvec_mode) {
        pc = tvec_base;
    } else if (1 == tvec_mode) {
        pc = tvec_base + cause << 2;
    } else {
        throw std::logic_error("Unreachable code reached");
    }
    // 3.1.14 & 4.1.7
    // When a trap is taken into S-mode (or M-mode), sepc (or mepc) is written with the virtual address 
    // of the instruction that was interrupted or that encountered the exception.
    csr.store(EPC, oldpc);
    // 3.1.15 & 4.1.8
    // When a trap is taken into S-mode (or M-mode), scause (or mcause) is written with a code indicating 
    // the event that caused the trap.
    csr.store(CAUSE, cause);
    // 3.1.16 & 4.1.9
    // When a trap is taken into M-mode, mtval is either set to zero or written with exception-specific 
    // information to assist software in handling the trap.
    csr.store(TVAL, 0);
    // 3.1.6 covers both sstatus and mstatus.
    uint64_t status = csr.load(STATUS);
    // get SIE or MIE
    uint64_t ie = (status & MASK_IE) >> ie_i;
    // set SPIE = SIE or MPIE = MIE
    status = (status & (~MASK_PIE)) | (ie << pie_i);
    // set SIE = 0 or MIE = 0
    status &= !MASK_IE;
    // set SPP or MPP = previous mode
    status = (status & (~MASK_PP)) | (oldmode << pp_i);
    csr.store(STATUS, status);
}   

void CPU::check_pending_interrupt() {
    // 3.1.6.1
    // When a hart is executing in privilege mode x, interrupts are globally enabled when x IE=1 and globally 
    // disabled when xIE=0. Interrupts for lower-privilege modes, w<x, are always globally disabled regardless 
    // of the setting of any global wIE bit for the lower-privilege mode. Interrupts for higher-privilege modes, 
    // y>x, are always globally enabled regardless of the setting of the global yIE bit for the higher-privilege 
    // mode. Higher-privilege-level code can use separate per-interrupt enable bits to disable selected higher-
    // privilege-mode interrupts before ceding control to a lower-privilege mode
 
    // 3.1.9 & 4.1.3
    // An interrupt i will trap to M-mode (causing the privilege mode to change to M-mode) if all of
    // the following are true: (a) either the current privilege mode is M and the MIE bit in the mstatus
    // register is set, or the current privilege mode has less privilege than M-mode; (b) bit i is set in both
    // mip and mie; and (c) if register mideleg exists, bit i is not set in mideleg.
    if (machine_mode == mode && 0 == (csr.load(MSTATUS) & MASK_MIE)) {
        return;
    }
    if(supervisor_mode == mode && 0 == (csr.load(SSTATUS) & MASK_SIE)) {
        return;
    }
    // In fact, we should using priority to decide which interrupt should be handled first.
    if (bus.uart_is_interrupting()) {
        bus.store(PLIC_SCLAIM, 32, UART_IRQ);
        csr.store(MIP, csr.load(MIP) | MASK_SEIP);
    }
    // 3.1.9 & 4.1.3
    // Multiple simultaneous interrupts destined for M-mode are handled in the following decreasing
    // priority order: MEI, MSI, MTI, SEI, SSI, STI.
    uint64_t pending = csr.load(MIE) & csr.load(MIP);
    if (pending & MASK_MEIP) {
        csr.store(MIP, csr.load(MIP) & (~MASK_MEIP));
        throw MachineExternalInterrupt();
    }
    if (pending & MASK_MSIP) {
        csr.store(MIP, csr.load(MIP) & (~MASK_MSIP));
        throw MachineSoftwareInterrupt();
    }
    if (pending & MASK_MTIP) {
        csr.store(MIP, csr.load(MIP) & (~MASK_MTIP));
        throw MachineTimerInterrupt();
    }
    if (pending & MASK_SEIP) {
        csr.store(MIP, csr.load(MIP) & (~MASK_SEIP));
        throw SupervisorExternalInterrupt();
    }
    if (pending & MASK_SSIP) {
        csr.store(MIP, csr.load(MIP) & (~MASK_SSIP));
        throw SupervisorSoftwareInterrupt();
    }
    if (pending & MASK_STIP) {
        csr.store(MIP, csr.load(MIP) & (~MASK_STIP));
        throw SupervisorTimerInterrupt();
    }
}

#endif