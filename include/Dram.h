#ifndef _DRAM_H_
#define _DRAM_H_

#include <vector>

#include "param.h"
#include "exception.h"

class Dram{
public:
	Dram(std::vector<uint8_t> & code): dram(DRAM_SIZE) {
		std::copy(code.begin(), code.end(), dram.begin());
	}

	// addr/size must be valid. Check in bus
	uint64_t load(uint64_t addr, uint64_t size) {
        if (size != 8 && size != 16 && size != 32 && size != 64) {
            throw LoadAccessFault();
        }
        uint64_t nbytes = size / 8;
        uint64_t index = (addr - DRAM_BASE);
        uint64_t code = dram[index];
        for (uint64_t i = 1; i < nbytes; ++i) {
            code |= (dram[index + i] << (i * 8));
        }
        return code;
    }

    // addr/size must be valid. Check in bus
    void store(uint64_t addr, uint64_t size, uint64_t value) {
        if (size != 8 && size != 16 && size != 32 && size != 64) {
            throw StoreAMOAccessFault();
        }
        uint64_t nbytes = size / 8;
        uint64_t index = (addr - DRAM_BASE);
        for (uint64_t i = 0; i < nbytes; ++i) {
            uint64_t offset = 8 * i;
            dram[index + i] = ((value >> offset) & 0xff);
        }
    }

private:
	std::vector<uint8_t> dram;
};

#endif