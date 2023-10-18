#ifndef _VIRTQUEUE_H_
#define _VIRTQUEUE_H_

#include <param.h>

struct VirtqDesc {
	uint64_t addr;
	uint32_t len; 
	uint16_t flags;
	uint16_t next;
};

struct VirtqAvail {
	uint16_t flags;
	uint16_t idx; 
	uint16_t ring[DESC_NUM];
	uint16_t used_event;
};

struct VirtQUsedusedElem {
	uint32_t id;
    uint32_t len;
};


struct VirtqUsed {
	uint16_t flags;
	uint16_t idx;
	VirtQUsedusedElem ring[DESC_NUM];
	uint16_t avail_event;
};


struct VirtioBlkRequest {
	uint32_t iotype;
	uint32_t reserved;
	uint64_t sector;
};

#endif  // _VIRTQUEUE_H_