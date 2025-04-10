#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <string.h>
#include "hwlib.h"
#include "socal/socal.h"
#include "socal/hps.h"
#include "socal/alt_gpio.h"
#include "hps_0.h"

#define HW_REGS_BASE (ALT_STM_OFST)
#define HW_REGS_SPAN (0x04000000)
#define HW_REGS_MASK (HW_REGS_SPAN - 1)

// DMA related defines
#define DMA_BASE 0xFFE02000  // Base address of DMA controller
#define DMA_LEN_REG_OFFSET 0x10  // Length register offset
#define DMA_CSR_REG_OFFSET 0x00  // Control/Status register offset
#define DMA_READ_ADDR_OFFSET 0x04  // Source address register offset
#define DMA_WRITE_ADDR_OFFSET 0x08  // Destination address register offset

#define BUFFER_SIZE 1024  // 1KB buffer

int main() {
    void *virtual_base;
    int fd;
    void *h2p_lw_led_addr;
    
    // DMA related variables
    void *dma_virtual_base;
    uint32_t *dma_csr_reg;
    uint32_t *dma_read_addr_reg;
    uint32_t *dma_write_addr_reg;
    uint32_t *dma_len_reg;
    
    // Create a source buffer
    uint8_t *source_buffer = malloc(BUFFER_SIZE);
    if (!source_buffer) {
        printf("ERROR: Failed to allocate source buffer\n");
        return 1;
    }
    
    // Fill the buffer with some pattern (for example, incrementing values)
    for (int i = 0; i < BUFFER_SIZE; i++) {
        source_buffer[i] = i % 256;
    }

    // map the address space for the registers into user space
    if ((fd = open("/dev/mem", (O_RDWR | O_SYNC))) == -1) {
        printf("ERROR: could not open \"/dev/mem\"...\n");
        free(source_buffer);
        return 1;
    }

    virtual_base = mmap(NULL, HW_REGS_SPAN, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, HW_REGS_BASE);
    if (virtual_base == MAP_FAILED) {
        printf("ERROR: mmap() failed...\n");
        close(fd);
        free(source_buffer);
        return 1;
    }
    
    // Get the LED address
    h2p_lw_led_addr = virtual_base + ((unsigned long)(ALT_LWFPGASLVS_OFST + LED_PIO_BASE) & (unsigned long)(HW_REGS_MASK));
    
    // Map DMA controller registers
    dma_virtual_base = mmap(NULL, 0x1000, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, DMA_BASE);
    if (dma_virtual_base == MAP_FAILED) {
        printf("ERROR: DMA mmap() failed...\n");
        munmap(virtual_base, HW_REGS_SPAN);
        close(fd);
        free(source_buffer);
        return 1;
    }
    
    // Setup DMA register pointers
    dma_csr_reg = (uint32_t *)(dma_virtual_base + DMA_CSR_REG_OFFSET);
    dma_read_addr_reg = (uint32_t *)(dma_virtual_base + DMA_READ_ADDR_OFFSET);
    dma_write_addr_reg = (uint32_t *)(dma_virtual_base + DMA_WRITE_ADDR_OFFSET);
    dma_len_reg = (uint32_t *)(dma_virtual_base + DMA_LEN_REG_OFFSET);
    
    // Configure DMA transfer
    *dma_read_addr_reg = (uint32_t)source_buffer;  // Source address (virtual address of our buffer)
    *dma_write_addr_reg = (uint32_t)h2p_lw_led_addr;  // Destination address (FPGA LED register)
    *dma_len_reg = BUFFER_SIZE;  // Transfer length
    
	while(1)
	{
		// Start DMA transfer (set GO bit in CSR)
		*dma_csr_reg = 0x1;  // Assuming bit 0 is the GO bit
		
		// Wait for transfer to complete (poll BUSY bit)
		while (*dma_csr_reg & 0x2) {  // Assuming bit 1 is the BUSY bit
			usleep(100);
		}
		
		printf("DMA transfer completed!\n");
	}
    
    
    
    // Clean up
    free(source_buffer);
    munmap(dma_virtual_base, 0x1000);
    
    if (munmap(virtual_base, HW_REGS_SPAN) != 0) {
        printf("ERROR: munmap() failed...\n");
        close(fd);
        return 1;
    }

    close(fd);
    return 0;
}