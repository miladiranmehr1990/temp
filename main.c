#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <signal.h>

#define PRIV_TIMER_BASE 0xFFFEC600
#define TIMER_SPAN 0x1000

volatile unsigned int *timer_load;
volatile unsigned int *timer_value;
volatile unsigned int *timer_control;
volatile unsigned int *timer_int_status;

int timer_fd;
void *timer_virtual;

// Interrupt handler
void timer_handler(int sig) {
    printf("Timer interrupt occurred!\n");
    
    // Clear interrupt status
    *timer_int_status = 1;
}

int main() {
    // Open /dev/mem
    timer_fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (timer_fd < 0) {
        perror("Failed to open /dev/mem");
        return -1;
    }

    // Map timer registers
    timer_virtual = mmap(NULL, TIMER_SPAN, PROT_READ | PROT_WRITE, MAP_SHARED, timer_fd, PRIV_TIMER_BASE);
    if (timer_virtual == MAP_FAILED) {
        perror("Failed to mmap timer");
        close(timer_fd);
        return -1;
    }

    // Set up pointers to timer registers
    timer_load = (unsigned int *)(timer_virtual + 0x00);
    timer_value = (unsigned int *)(timer_virtual + 0x04);
    timer_control = (unsigned int *)(timer_virtual + 0x08);
    timer_int_status = (unsigned int *)(timer_virtual + 0x0C);

    // Configure signal handler
    signal(SIGIO, timer_handler);

    // Configure timer
    unsigned int prescaler = 0;  // No prescaling
    unsigned int load_value = 1000000;  // Example value (1MHz clock -> 1Hz interrupt)
    
    *timer_load = load_value;
    *timer_control = (1 << 2) | (1 << 1) | (1 << 0);  // Enable interrupt, auto-reload, enable timer

    printf("Timer configured. Waiting for interrupts...\n");

    // Keep program running
    while (1) {
        sleep(1);
    }

    // Cleanup (unreachable in this example)
    munmap(timer_virtual, TIMER_SPAN);
    close(timer_fd);
    return 0;
}