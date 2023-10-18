#ifndef _UART_H_
#define _UART_H_

#include "param.h"
#include "Bus.h"
#include "exception.h"

#include <iostream>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <thread>

class Uart {
public:
	Uart() {
		uart_ = new uint8_t [UART_SIZE];
		std::fill_n(uart_, UART_SIZE, 0);
		uart_[UART_LSR] |= MASK_UART_LSR_TX;
		interrupt_ = false;
		std::thread receive_thread([this]() {
	        char byte;
	        while (true) {
	            std::cin.read(&byte, 1);
	            std::unique_lock<std::mutex> lock(mutex_);
	            // if data have been received but not yet be transferred.
	            // this thread wait for it to be transferred.
	            while ((uart_[UART_LSR] & MASK_UART_LSR_RX) == 1) {
	                cvar_.wait(lock);
	            }
	            // data have been transferred, so receive next one.
	            uart_[UART_RHR] = byte;
	            interrupt_ = true;
	            uart_[UART_LSR] |= MASK_UART_LSR_RX;
	            cvar_.notify_one();
	        }
	    });
	    receive_thread.detach();
	}

	~Uart() {
		delete [] uart_;
	}

	bool is_interrupting() {
	    return std::atomic_exchange(&interrupt_, false);
	}

	uint64_t load(uint64_t addr, uint64_t size) {
	    if (size != 8) {
	    	std::cerr << "uart LoadAccessFault\n";
	        throw LoadAccessFault(addr);
	    }
	    std::unique_lock<std::mutex> lock(mutex_);
	    uint64_t index = addr - UART_BASE;
	    if (UART_RHR == index) {
            cvar_.notify_one();
            uart_[UART_LSR] &= ~MASK_UART_LSR_RX;
            return uart_[UART_RHR];
	    }
	    else {
	    	return uart_[index];
	    }
	}

	void store(uint64_t addr, uint64_t size, uint64_t value) {
	    if (size != 8) {
	    	std:: cout << "uart.store" << std::endl;
	        throw StoreAMOAccessFault(addr);
	    }
	    std::unique_lock<std::mutex> lock(mutex_);
	    uint64_t index = addr - UART_BASE;
	    if (UART_THR == index) {
	            std::cout << (char)(value & 0xff);
	            std::cout.flush();
	    }
	    else {
	    	uart_[index] = value & 0xff;
	    }
	}
private:
	uint8_t * uart_;
	std::mutex mutex_;
	std::condition_variable cvar_;
	std::atomic<bool> interrupt_;
};

#endif