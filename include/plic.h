#ifndef _PLIC_H_
#define _PLIC_H_

#include "exception.h"
#include "param.h"

class Plic {
public:
    Plic() : pending(0), senable(0), spriority(0), sclaim(0) {}

    uint64_t load(uint64_t addr, uint64_t size) {
        if (size != 32) {
            throw LoadAccessFault(addr);
        }
        if(PLIC_PENDING == addr) {
            return pending;
        } else if (PLIC_SENABLE == addr){
            return senable;
        } else if (PLIC_SPRIORITY == addr) {
            return spriority;
        } else if (PLIC_SCLAIM == addr) {
            return sclaim;
        } else {
            return 0;
        }
    }

    void store(uint64_t addr, uint64_t size, uint64_t value) {
        if (size != 32) {
            throw StoreAMOAccessFault(addr);
        }
        if (addr == PLIC_PENDING) {
	        pending = value;
	    } else if (addr == PLIC_SENABLE) {
	        senable = value;
	    } else if (addr == PLIC_SPRIORITY) {
	        spriority = value;
	    } else if (addr == PLIC_SCLAIM) {
	        sclaim = value;
	    }
    }

private:
    uint64_t pending;
    uint64_t senable;
    uint64_t spriority;
    uint64_t sclaim;
};

#endif