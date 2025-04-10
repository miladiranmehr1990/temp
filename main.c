#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <string.h>

// DMA driver IOCTL commands (these may vary depending on your kernel version)
#define DMA_IOCTL_MAGIC 'd'
#define DMA_IOCTL_START _IOW(DMA_IOCTL_MAGIC, 0, struct dma_transfer*)
#define DMA_IOCTL_WAIT  _IOR(DMA_IOCTL_MAGIC, 1, int)

struct dma_transfer {
    void* src;      // Source physical address
    void* dest;     // Destination physical address
    size_t length;  // Transfer length in bytes
};

int main() {
    int dma_fd;
    int ret;
    const size_t BUFFER_SIZE = 1024; // 1KB transfer
    int i;
	
    // Allocate source and destination buffers
    char *src_buf = malloc(BUFFER_SIZE);
    char *dest_buf = malloc(BUFFER_SIZE);
    
	printf("Helooooo\n" );
	
    if (!src_buf || !dest_buf) {
        perror("Failed to allocate buffers");
        return -1;
    }
    
    // Fill source buffer with some data
    for (i = 0; i < BUFFER_SIZE; i++) {
        src_buf[i] = i % 256;
    }
    
    // Open DMA device
    dma_fd = open("/dev/dma_0", O_RDWR);
    if (dma_fd < 0) {
        perror("Failed to open /dev/dma_0");
        free(src_buf);
        free(dest_buf);
        return -1;
    }
    
    // Prepare DMA transfer structure
    struct dma_transfer transfer = {
        .src = src_buf,
        .dest = dest_buf,
        .length = BUFFER_SIZE
    };
    
    // Start DMA transfer
    ret = ioctl(dma_fd, DMA_IOCTL_START, &transfer);
    if (ret < 0) {
        perror("DMA transfer failed");
        close(dma_fd);
        free(src_buf);
        free(dest_buf);
        return -1;
    }
    
    // Wait for DMA to complete
    int status;
    ret = ioctl(dma_fd, DMA_IOCTL_WAIT, &status);
    if (ret < 0) {
        perror("DMA wait failed");
    } else {
        printf("DMA transfer completed with status: %d\n", status);
        
        // Verify the transfer
        if (memcmp(src_buf, dest_buf, BUFFER_SIZE) == 0) {
            printf("Data verification successful!\n");
        } else {
            printf("Data verification failed!\n");
        }
    }
    
    // Clean up
    close(dma_fd);
    free(src_buf);
    free(dest_buf);
    
    return 0;
}