#ifndef _PARAM_H_
#define _PARAM_H_

#include <cstdint>
#include <iostream>

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

// Virtio
// The address which virtio starts.
const uint64_t VIRTIO_BASE = 0x1000'1000;
// the size of virtio
const uint64_t VIRTIO_SIZE = 0x1000;
// the interupt request of virtio.
const uint64_t VIRTIO_END = VIRTIO_BASE + VIRTIO_SIZE - 1;
const uint64_t VIRTIO_IRQ = 1;

// the number of virtio descriptors that must be power of 2
const uint64_t DESC_NUM = 8;

// always return 0x74726976
const uint64_t VIRTIO_MAGIC = VIRTIO_BASE + 0x000;
// The version. 1 is legacy.
const uint64_t VIRTIO_VERSION = VIRTIO_BASE + 0x004;
// device type; 1 is net, 2 is disk
const uint64_t VIRTIO_DEVICE_ID = VIRTIO_BASE + 0x008;
// always return 0x554d4551
const uint64_t VIRTIO_VENDOR_ID = VIRTIO_BASE + 0x00c;
// Device feature.
const uint64_t VIRTIO_DEVICE_FEATURES = VIRTIO_BASE + 0x010;
// Driver features
const uint64_t VIRTIO_DRIVER_FEATURES = VIRTIO_BASE + 0X020;
// Page size for PFN, write-only
const uint64_t VIRTIO_GUEST_PAGE_SIZE = VIRTIO_BASE + 0x028;
// Select queue, write-only
const uint64_t VIRTIO_QUEUE_SEL = VIRTIO_BASE + 0x30;
// Max size of current queue, read-only. In QEMU, `VIRTIO_COUNT = 8`.
const uint64_t VIRTIO_QUEUE_NUM_MAX = VIRTIO_BASE + 0x034;
// size of current queue, write-only
const uint64_t VIRTIO_QUEUE_NUM = VIRTIO_BASE + 0x38;
// Physical page number for queue, read and write.
const uint64_t VIRTIO_QUEUE_PFN = VIRTIO_BASE + 0x040; 
// Notify the queue number, write-only.
const uint64_t VIRTIO_QUEUE_NOTIFY = VIRTIO_BASE + 0x050;
// Device status, read and write. Reading from this register returns the current device status flags.
// Writing non-zero values to this register sets the status flags, indicating the OS/driver
// progress. Writing zero (0x0) to this register triggers a device reset.
const uint64_t VIRTIO_STATUS = VIRTIO_BASE + 0x070;

const uint64_t PAGE_SIZE = 4096;
const uint64_t SECTOR_SIZE = 512;

// virtio block request type
const uint32_t VIRTIO_BLK_T_IN = 0;
const uint32_t VIRTIO_BLK_T_OUT = 1;

// virtqueue descriptor flags
const uint16_t VIRTQ_DESC_F_NEXT = 1;
const uint16_t VIRTQ_DESC_F_WRITE = 2;
const uint64_t VIRTQ_DESC_F_INDIRECT = 4;

#endif