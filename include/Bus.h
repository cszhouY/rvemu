#ifndef _BUS_H_
#define _BUS_H_

#include "Dram.h"
#include "exception.h"
#include "param.h"
#include "plic.h"
#include "clint.h"
#include "uart.h"
#include "virtio.h"

#include <vector>
#include <iostream>

class Bus{
public:
    Bus(std::vector<uint8_t>& code, std::vector<uint8_t> & disk_image) : dram(code), virtio_blk(disk_image) {}

    uint64_t load(uint64_t addr, uint64_t size) {
        if(addr >= UART_BASE && addr <= UART_END) {
            return uart.load(addr, size);
        } else if (addr >= CLINT_BASE && addr <= CLINT_END) {
            return clint.load(addr, size);
        } else if (addr >= PLIC_BASE && addr <= PLIC_END) {
            return plic.load(addr, size);
        } else if (addr >= DRAM_BASE && addr <= DRAM_END) {
            return dram.load(addr, size);
        } else if (addr >= VIRTIO_BASE && addr <= VIRTIO_END) {
            return virtio_blk.load(addr, size);
        } else {
            std::cerr << "bus addr out\n";
            throw LoadAccessFault(addr);
        }
    }

    void store(uint64_t addr, uint64_t size, uint64_t value) {
        if(addr >= UART_BASE && addr <= UART_END) {
            uart.store(addr, size, value);
        } else if (addr >= CLINT_BASE && addr <= CLINT_END) {
            clint.store(addr, size, value);
        } else if (addr >= PLIC_BASE && addr <= PLIC_END) {
            plic.store(addr, size, value);
        } else if (addr >= DRAM_BASE && addr <= DRAM_END) {
            dram.store(addr, size, value);
        } else if (addr >= VIRTIO_BASE && addr <= VIRTIO_END) {
            virtio_blk.store(addr, size, value);
        } else {
            std::cout << std::hex << addr << std::endl;
            throw StoreAMOAccessFault(addr);
        }
    }

    bool uart_is_interrupting() {
        return uart.is_interrupting();
    }

    VirtioBlock & get_virtio_blk() {
        return virtio_blk;
    }
private:
	Dram dram;
    Plic plic;
    Clint clint;
    Uart uart;
    VirtioBlock virtio_blk;
};

#endif