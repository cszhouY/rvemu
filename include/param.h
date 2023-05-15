#ifndef _PARAM_H_
#define _PARAM_H_

#include <cstdint>

/*!
 * memory layout following QEMU
 * https://github.com/qemu/qemu/blob/master/hw/riscv/virt.c#L46-L63 
 * */
const uint64_t DRAM_BASE = 0x8000'0000;
const uint64_t DRAM_SIZE = 1024 * 1024 * 128;
const uint64_t DRAM_END  = 0x8000'0000 + 1024 * 1024 * 128 - 1;

#endif