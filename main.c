#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include "hwlib.h"
#include "socal/socal.h"
#include "socal/hps.h"
#include "alt_dma.h"
#include "alt_dma_common.h"

int main() {
    // Initialize DMA
    alt_dma_init();

    // Source & destination buffers
    uint8_t source[10] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A};
    uint8_t destination[10] = {0};

    // Flush cache (if caching is enabled)
    alt_dcache_flush(source, 10);

    // Perform DMA transfer
    int ret = alt_dma_memory_to_memory(
        ALT_DMA_CHANNEL_0,
        (void*)source,
        (void*)destination,
        10,
        0
    );

    if (ret != ALT_DMA_SUCCESS) {
        printf("DMA Transfer Failed! Error: %d\n", ret);
        return -1;
    }

    // Wait for completion
    while (alt_dma_is_busy(ALT_DMA_CHANNEL_0)) {
        usleep(100);
    }

    // Invalidate cache (if caching is enabled)
    alt_dcache_invalidate(destination, 10);

    // Verify data
    for (int i = 0; i < 10; i++) {
        printf("dest[%d] = 0x%02X\n", i, destination[i]);
    }

    return 0;
}