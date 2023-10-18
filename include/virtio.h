#ifndef _VIRTIO_H_
#define _VIRTIO_H_

#include <CPU.h>
#include <exception.h>
#include <param.h>
#include <Bus.h>
#include <vector>

#define MAX_BLOCK_QUEUE 1

class VirtioBlock {
public:
	VirtioBlock(std::vector<uint8_t> & disk_image): 
	id(0), 
	driver_features(0), 
	page_size(0), 
	queue_sel(0),
	queue_num(0),
	queue_pfn(0),
	queue_notify(MAX_BLOCK_QUEUE), 
	status(0),
	disk(disk_image){}

	bool is_interrupting();

	uint64_t load(uint64_t addr, uint64_t size);

	void store(uint64_t addr, uint64_t size, uint64_t value);

	uint64_t get_new_id();

	uint64_t desc_addr();

	uint64_t read_disk(uint64_t addr);

	void write_disk(uint64_t addr, uint64_t value);
private:
	uint64_t id;
	uint32_t driver_features;
	uint32_t page_size;
	uint32_t queue_sel;
	uint32_t queue_num;
	uint32_t queue_pfn;
	uint32_t queue_notify;
	uint32_t status;
	std::vector<uint8_t> disk;
};

bool VirtioBlock::is_interrupting() {
	if (queue_notify < MAX_BLOCK_QUEUE) {
		queue_notify = MAX_BLOCK_QUEUE;
		return true;
	}
	return false;
}

uint64_t VirtioBlock::load(uint64_t addr, uint64_t size) {
	if (32 != size) {
		throw LoadAccessFault(addr);
	}
	switch (addr) {
	case VIRTIO_MAGIC: return 0x74726976;
	case VIRTIO_VERSION: return 0x1;
	case VIRTIO_DEVICE_ID: return 0x2;
	case VIRTIO_VENDOR_ID: return 0x554d4551;
	case VIRTIO_DEVICE_FEATURES: return 0;
	case VIRTIO_DRIVER_FEATURES: return (uint64_t)driver_features;
	case VIRTIO_QUEUE_NUM_MAX: return 8;
	case VIRTIO_QUEUE_PFN: return (uint64_t)queue_pfn;
	case VIRTIO_STATUS: return (uint64_t)status;
	default: break;
	}
	return 0;
}

void VirtioBlock::store(uint64_t addr, uint64_t size, uint64_t value) {
	if (32 != size) {
		throw StoreAMOAccessFault(addr);
	}

	switch (addr) {
	case VIRTIO_DEVICE_FEATURES: driver_features = value; break;
	case VIRTIO_GUEST_PAGE_SIZE: page_size = value; break;
	case VIRTIO_QUEUE_SEL: queue_sel = value; break;
	case VIRTIO_QUEUE_NUM: queue_num = value; break;
	case VIRTIO_QUEUE_PFN: queue_pfn = value; break;
	case VIRTIO_QUEUE_NOTIFY: queue_notify = value; break;
	case VIRTIO_STATUS: status = value; break;
	default: break;
	}
}

uint64_t VirtioBlock::get_new_id() {
	return ++id;
}

// get the address of the virtqueue
uint64_t VirtioBlock::desc_addr() {
	return (uint64_t)queue_pfn * (uint64_t)page_size;
}

uint64_t VirtioBlock::read_disk(uint64_t addr) {
	return (uint64_t)disk[addr];
}

void VirtioBlock::write_disk(uint64_t addr, uint64_t value) {
	disk[addr] = (uint8_t)value;
}

#endif
