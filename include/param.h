#ifndef _PARAM_H_
#define _PARAM_H_

#include <cstdint>

/*!
 * memory layout following QEMU
 * https://github.com/qemu/qemu/blob/master/hw/riscv/virt.c#L46-L63 
 * */
const uint64_t DRAM_BASE = 0x8000'0000;
const uint64_t DRAM_SIZE = 1024 * 1024 * 128;
const uint64_t DRAM_END  = DRAM_BASE + DRAM_SIZE - 1;


// The address which the core-local interruptor (CLINT) starts. It contains the timer and
// generates per-hart software interrupts and timer interrupts.
const uint64_t CLINT_BASE = 0x200'0000;
const uint64_t CLINT_SIZE = 0x10000;
const uint64_t CLINT_END  = CLINT_BASE + CLINT_SIZE - 1;

const uint64_t CLINT_MTIMECMP = CLINT_BASE + 0x4000;
const uint64_t CLINT_MTIME = CLINT_BASE + 0xbff8;

// The address which the platform-level interrupt controller (PLIC) starts. The PLIC connects all external interrupts in the
// system to all hart contexts in the system, via the external interrupt source in each hart.
const uint64_t PLIC_BASE = 0xc00'0000;
const uint64_t PLIC_SIZE = 0x4000000;
const uint64_t PLIC_END  = PLIC_BASE + PLIC_SIZE - 1;

const uint64_t PLIC_PENDING   = PLIC_BASE + 0x1000;
const uint64_t PLIC_SENABLE   = PLIC_BASE + 0x2000;
const uint64_t PLIC_SPRIORITY = PLIC_BASE + 0x201000;
const uint64_t PLIC_SCLAIM    = PLIC_BASE + 0x201004;

// UART
const uint64_t UART_BASE = 0x1000'0000;
const uint64_t UART_SIZE = 0x100;
const uint64_t UART_END  = UART_BASE + UART_SIZE - 1;
// uart interrupt request
const uint64_t UART_IRQ  = 10;
// Receive holding register (for input bytes).
const uint64_t UART_RHR  = 0;
// Transmit holding register (for output bytes).
const uint64_t UART_THR  = 0;
// Line control register.
const uint64_t UART_LCR  = 3;
// Line status register.
// LSR BIT 0:
//     0 = no data in receive holding register or FIFO.
//     1 = data has been receive and saved in the receive holding register or FIFO.
// LSR BIT 5:
//     0 = transmit holding register is full. 16550 will not accept any data for transmission.
//     1 = transmitter hold register (or FIFO) is empty. CPU can load the next character.
const uint64_t UART_LSR = 5;
// The receiver (RX) bit MASK.
const uint64_t MASK_UART_LSR_RX = 1;
// The transmitter (TX) bit MASK.
const uint64_t MASK_UART_LSR_TX = 1 << 5;

#endif