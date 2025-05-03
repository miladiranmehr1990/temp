/*
This program demonstrate how to use hps communicate with FPGA through light AXI Bridge.
uses should program the FPGA by GHRD project before executing the program
refer to user manual chapter 7 for details about the demo
*/


#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "hwlib.h"
#include "socal/socal.h"
#include "socal/hps.h"
#include "socal/alt_gpio.h"
#include "hps_0.h"

#define HW_REGS_BASE ( ALT_STM_OFST )
#define HW_REGS_SPAN ( 0x04000000 )
#define HW_REGS_MASK ( HW_REGS_SPAN - 1 )

#define LASER_CONTROL_REGISTER_BASE	0x3010
#define LASER_POINT_START_ADDRESS_BASE	0x3020
#define LASER_POINT_END_ADDRESS_BASE	0x3030
#define LASER_POINT_CURRENT_ADDRESS_BASE	0x3040
#define LASER_FEEDBACK_BASE	0x3050

int main() {

	void *virtual_base;
	int fd;
	
	//void *h2p_lw_led_addr;
	
	void *laser_control_register_ptr;
	void *laser_point_start_address_ptr;
	void *laser_point_end_address_ptr;
	void *laser_point_current_address_ptr;
	void *laser_feedback_ptr;
	
	uint32_t laser_point_current_address = 0x00000000;
	uint32_t laser_point_current_address_prev = 0x00000000;
	uint32_t laser_feedback = 0x00000000;
	uint32_t laser_feedback_prev = 0x00000000;

	// map the address space for the LED registers into user space so we can interact with them.
	// we'll actually map in the entire CSR span of the HPS since we want to access various registers within that span

	if( ( fd = open( "/dev/mem", ( O_RDWR | O_SYNC ) ) ) == -1 ) {
		printf( "ERROR: could not open \"/dev/mem\"...\n" );
		return( 1 );
	}

	virtual_base = mmap( NULL, HW_REGS_SPAN, ( PROT_READ | PROT_WRITE ), MAP_SHARED, fd, HW_REGS_BASE );

	if( virtual_base == MAP_FAILED ) {
		printf( "ERROR: mmap() failed...\n" );
		close( fd );
		return( 1 );
	}
	
	//h2p_lw_led_addr = virtual_base + ( ( unsigned long  )( ALT_LWFPGASLVS_OFST + LED_PIO_BASE ) & ( unsigned long)( HW_REGS_MASK ) );
	
	laser_control_register_ptr = virtual_base + ( ( unsigned long  )( ALT_LWFPGASLVS_OFST + LASER_CONTROL_REGISTER_BASE ) & ( unsigned long)( HW_REGS_MASK ) );
	laser_point_start_address_ptr = virtual_base + ( ( unsigned long  )( ALT_LWFPGASLVS_OFST + LASER_POINT_START_ADDRESS_BASE ) & ( unsigned long)( HW_REGS_MASK ) );
	laser_point_end_address_ptr = virtual_base + ( ( unsigned long  )( ALT_LWFPGASLVS_OFST + LASER_POINT_END_ADDRESS_BASE ) & ( unsigned long)( HW_REGS_MASK ) );
	laser_point_current_address_ptr = virtual_base + ( ( unsigned long  )( ALT_LWFPGASLVS_OFST + LASER_POINT_CURRENT_ADDRESS_BASE ) & ( unsigned long)( HW_REGS_MASK ) );
	laser_feedback_ptr = virtual_base + ( ( unsigned long  )( ALT_LWFPGASLVS_OFST + LASER_FEEDBACK_BASE ) & ( unsigned long)( HW_REGS_MASK ) );
	
	
	printf("Hello World!\n");
	
	*(uint32_t *)laser_control_register_ptr = 0x00000000;
	usleep( 3000*1000 );
	
	printf("Starting laser...\n");
	
	// Start laser
	*(uint32_t *)laser_point_start_address_ptr = 0x20000000;
	*(uint32_t *)laser_point_end_address_ptr = 0x20000860;
	*(uint32_t *)laser_control_register_ptr = 0x00000001;
	
	while( 1 )
	{
		laser_point_current_address = *(uint32_t *)laser_point_current_address_ptr;
		laser_feedback = *(uint32_t *)laser_feedback_ptr;
		
		if(laser_point_current_address_prev != laser_point_current_address)
		{
			laser_point_current_address_prev = laser_point_current_address;
			printf( "Changed Value: laser_point_current_address=%#x\n", laser_point_current_address);
		}
		else{}
		if(laser_feedback_prev != laser_feedback)
		{
			laser_feedback_prev = laser_feedback;
			printf( "Changed Value: laser_feedback=%#x\n", laser_feedback);
		}
		else{}
	}

	// clean up our memory mapping and exit
	
	if( munmap( virtual_base, HW_REGS_SPAN ) != 0 ) {
		printf( "ERROR: munmap() failed...\n" );
		close( fd );
		return( 1 );
	}

	close( fd );

	return( 0 );
}
