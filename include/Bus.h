#ifndef _BUS_H_
#define _BUS_H_

#include "Dram.h"
#include "exception.h"
#include "param.h"

#include <vector>

class Bus{
public:
    Bus(std::vector<uint8_t>& code) : dram(code) {}

    uint64_t load(uint64_t addr, uint64_t size) {
        if (addr >= DRAM_BASE && addr <= DRAM_END) {
            return dram.load(addr, size);
        } else {
            throw LoadAccessFault(addr);
        }
    }

    void store(uint64_t addr, uint64_t size, uint64_t value) {
        if (addr >= DRAM_BASE && addr <= DRAM_END) {
            dram.store(addr, size, value);
        } else {
            throw StoreAMOAccessFault(addr);
        }
    }
private:
	Dram dram;
};

#endif