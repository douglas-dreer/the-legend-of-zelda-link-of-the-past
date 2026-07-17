#include "dma.h"

// DMA Transfer
void dma_transfer(uint32 src, uint16 dst_ppu, uint16 size,
                  uint8 channel, uint8 mode) {
    // In a real implementation, this would:
    // 1. Configure DMA channel registers
    // 2. Set source and destination addresses
    // 3. Set transfer size
    // 4. Fire DMA
    
    // For PC emulation, we simulate the transfer
    DMAChannel* ch = &dma_channels[channel];
    ch->mode = mode;
    ch->bbus_addr = 0x18;      // $2118: VRAM data write
    ch->src_addr = src;
    ch->size = size;
    
    // Fire DMA
    uint8 bit = 1 << channel;
    // io_write(0x420B, bit);
    
    // Em PC: copiar dados diretamente do ROM/RAM para VRAM
    // for (int i = 0; i < size; i++) {
    //     uint8 byte = read_bus(src + i);
    //     ppu_write_vram(dst_ppu + (i / 2), byte);
    // }
}

// DMA Engine Run
void dma_engine_run(void) {
    // Check each channel for pending transfers
    for (int i = 0; i < DMA_QUEUE_SIZE; i++) {
        if (dma_queue[i].active) {
            dma_transfer(
                dma_queue[i].src_addr,
                dma_queue[i].dst_ppu,
                dma_queue[i].size,
                dma_queue[i].channel,
                dma_queue[i].mode
            );
            dma_queue[i].active = false;
        }
    }
}

// Queue a DMA transfer
void dma_queue_transfer(uint8 channel, uint8 mode, 
                        uint32 src, uint16 dst, uint16 size) {
    if (channel < DMA_QUEUE_SIZE) {
        dma_queue[channel].active = true;
        dma_queue[channel].channel = channel;
        dma_queue[channel].mode = mode;
        dma_queue[channel].src_addr = src;
        dma_queue[channel].dst_ppu = dst;
        dma_queue[channel].size = size;
    }
}

// DMA room to VRAM (placeholder)
void dma_room_to_vram(void) {
    // In a real implementation, this would:
    // 1. Queue DMA transfers for room data
    // 2. Execute DMA engine
}

// HDMA Setup (placeholder)
void hdma_setup(uint8 channel, uint8 mode, uint8 bbus_addr,
                const uint8* table_addr, uint8 indirect_mode) {
    // Configure HDMA channel
}

// HDMA Enable
void hdma_enable(uint8 channels) {
    // Enable HDMA channels
}

// HDMA Disable
void hdma_disable(void) {
    // Disable all HDMA channels
}