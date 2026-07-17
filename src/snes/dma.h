#ifndef DMA_H
#define DMA_H

#include "../types.h"
#include "hardware.h"

// DMA Queue Entry
struct DMATransfer {
    bool active;
    uint8 channel;
    uint8 mode;
    uint32 src_addr;
    uint16 dst_ppu;
    uint16 size;
};

// DMA Queue size
constexpr int DMA_QUEUE_SIZE = 5;

// DMA Queue
extern DMATransfer dma_queue[DMA_QUEUE_SIZE];

// DMA Functions
void dma_transfer(uint32 src, uint16 dst_ppu, uint16 size,
                  uint8 channel, uint8 mode);

void dma_engine_run(void);

void dma_queue_transfer(uint8 channel, uint8 mode, 
                        uint32 src, uint16 dst, uint16 size);

void dma_room_to_vram(void);

// HDMA Functions
void hdma_setup(uint8 channel, uint8 mode, uint8 bbus_addr,
                const uint8* table_addr, uint8 indirect_mode);

void hdma_enable(uint8 channels);
void hdma_disable(void);

#endif // DMA_H