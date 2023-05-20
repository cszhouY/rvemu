#ifndef _CLINT_H_
#define _CLINT_H_

#include "exception.h"
#include "param.h"

#include <stdexcept>

#define CLINT_MTIMECMP 0x02004000
#define CLINT_MTIME 0x0200bff8

class Clint {
public:
    Clint() : mtime(0), mtimecmp(0) {}

    uint64_t load(uint64_t addr, uint64_t size) {
        if (size != 64) {
            throw LoadAccessFault(addr);
        }
        if (CLINT_MTIMECMP == addr) {
            return mtimecmp;
        } else if (CLINT_MTIME == addr) {
            return mtime;
        } else{
            throw LoadAccessFault(addr);
        }
    }

    void store(uint64_t addr, uint64_t size, uint64_t value) {
        if (size != 64) {
            throw StoreAMOAccessFault(addr);
        }
        if (CLINT_MTIMECMP == addr) {
            mtimecmp = value;
        } else if (CLINT_MTIME == addr) {
            mtime = value;
        } else {
            throw StoreAMOAccessFault(addr);
        }
    }

private:
    uint64_t mtime;
    uint64_t mtimecmp;
};


#endif