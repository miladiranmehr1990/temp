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
#include <stdlib.h>  // For strtol
#include <string.h>  // For strtol

#define HW_REGS_BASE ( ALT_STM_OFST )
#define HW_REGS_SPAN ( 0x04000000 )
#define HW_REGS_MASK ( HW_REGS_SPAN - 1 )

#define GALVO_CONTROL_REGISTER_BASE	0x00003010
#define GALVO_POINT_START_ADDRESS_BASE	0x00003020
#define GALVO_POINT_END_ADDRESS_BASE	0x00003030
#define GALVO_POINT_CURRENT_ADDRESS_BASE	0x00003040
#define GALVO_FEEDBACK_BASE	0x00003050

#define GALVO_CLOCK_DIVISION_BASE 0x00003060
#define GALVO_1_PERIOD_OF_WAIT_FOR_MOVE_TO_TARGET_BASE 0x00003070
#define GALVO_2_PERIOD_OF_WAIT_FOR_MOVE_TO_TARGET_BASE 0x00003080
#define GALVO_1_PERIOD_OF_WAIT_FOR_LASER_ON_BASE 0x00003090
#define GALVO_2_PERIOD_OF_WAIT_FOR_LASER_ON_BASE 0x000030A0
#define GALVO_1_PERIOD_OF_WAIT_FOR_LASER_OFF_BASE 0x000030B0
#define GALVO_2_PERIOD_OF_WAIT_FOR_LASER_OFF_BASE 0x000030C0
#define XY2_100_WAIT_AFTER_START_TO_SET_CMD_START_PINS_TO_LOW_BASE 0x000030D0
#define XY2_100_WAIT_AFTER_LOWERING_CMD_START_PINS_TO_START_NEW_POINT_BASE 0x000030E0
#define POINT_FETCH_PROPERTIES_BASE 0x000030F0

int main(int argc, char *argv[])
{

	void *virtual_base;
	int fd;
	int i;
	
	//void *h2p_lw_led_addr;
	
	void *galvo_control_register_ptr;
	void *galvo_point_start_address_ptr;
	void *galvo_point_end_address_ptr;
	void *galvo_point_current_address_ptr;
	void *galvo_feedback_ptr;
	
	void *galvo_clock_division_ptr;
	void *galvo_1_period_of_wait_for_move_to_target_ptr;
	void *galvo_2_period_of_wait_for_move_to_target_ptr;
	void *galvo_1_period_of_wait_for_laser_on_ptr;
	void *galvo_2_period_of_wait_for_laser_on_ptr;
	void *galvo_1_period_of_wait_for_laser_off_ptr;
	void *galvo_2_period_of_wait_for_laser_off_ptr;
	void *xy2_100_wait_after_start_to_set_cmd_start_pins_to_low_ptr;
	void *xy2_100_wait_after_lowering_cmd_start_pins_to_start_new_point_ptr;
	void *point_fetch_properties_ptr;
	
	
	
	// Default values
    uint32_t galvo_point_start_address_value = 0x20000000;
    uint32_t galvo_point_end_address_value = 0x20000008;
	uint32_t galvo_clock_division = 100; // For 250KHz  (   Formula:  galvo_clock_division = (50000000 / freq) / 2    ).
	uint32_t galvo_1_period_of_wait_for_move_to_target = 1000; // Number of galvo clocks for this purpose.
	uint32_t galvo_2_period_of_wait_for_move_to_target = 1000; // Number of galvo clocks for this purpose.
	uint32_t galvo_1_period_of_wait_for_laser_on = 10000; // Number of galvo clocks for this purpose.
	uint32_t galvo_2_period_of_wait_for_laser_on = 10000; // Number of galvo clocks for this purpose.
	uint32_t galvo_1_period_of_wait_for_laser_off = 10000; // Number of galvo clocks for this purpose.
	uint32_t galvo_2_period_of_wait_for_laser_off = 10000; // Number of galvo clocks for this purpose.
	uint32_t xy2_100_wait_after_start_to_set_cmd_start_pins_to_low = 1000; // Number of FPGA clocks for this purpose.
	uint32_t xy2_100_wait_after_lowering_cmd_start_pins_to_start_new_point = 40000000; // Number of FPGA clocks for this purpose.
	uint32_t point_fetch_properties = 0;
	int verbose = 1;  // Default to verbose output
	// Parse command line arguments
    for (i = 1; i < argc; i++)
	{
        if (strcmp(argv[i], "--galvo_point_start_address_value") == 0 && i+1 < argc)
		{
            galvo_point_start_address_value = strtoul(argv[++i], NULL, 0);
        } 
        else if (strcmp(argv[i], "--galvo_point_end_address_value") == 0 && i+1 < argc)
		{
            galvo_point_end_address_value = strtoul(argv[++i], NULL, 0);
        }
		else if (strcmp(argv[i], "--galvo_clock_division") == 0 && i+1 < argc)
		{
            galvo_clock_division = strtoul(argv[++i], NULL, 0);
        }
		else if (strcmp(argv[i], "--galvo_1_period_of_wait_for_move_to_target") == 0 && i+1 < argc)
		{
            galvo_1_period_of_wait_for_move_to_target = strtoul(argv[++i], NULL, 0);
        }
		else if (strcmp(argv[i], "--galvo_2_period_of_wait_for_move_to_target") == 0 && i+1 < argc)
		{
            galvo_2_period_of_wait_for_move_to_target = strtoul(argv[++i], NULL, 0);
        }
		else if (strcmp(argv[i], "--galvo_1_period_of_wait_for_laser_on") == 0 && i+1 < argc)
		{
            galvo_1_period_of_wait_for_laser_on = strtoul(argv[++i], NULL, 0);
        }
		else if (strcmp(argv[i], "--galvo_2_period_of_wait_for_laser_on") == 0 && i+1 < argc)
		{
            galvo_2_period_of_wait_for_laser_on = strtoul(argv[++i], NULL, 0);
        }
		else if (strcmp(argv[i], "--galvo_1_period_of_wait_for_laser_off") == 0 && i+1 < argc)
		{
            galvo_1_period_of_wait_for_laser_off = strtoul(argv[++i], NULL, 0);
        }
		else if (strcmp(argv[i], "--galvo_2_period_of_wait_for_laser_off") == 0 && i+1 < argc)
		{
            galvo_2_period_of_wait_for_laser_off = strtoul(argv[++i], NULL, 0);
        }
		else if (strcmp(argv[i], "--xy2_100_wait_after_start_to_set_cmd_start_pins_to_low") == 0 && i+1 < argc)
		{
            xy2_100_wait_after_start_to_set_cmd_start_pins_to_low = strtoul(argv[++i], NULL, 0);
        }
		else if (strcmp(argv[i], "--xy2_100_wait_after_lowering_cmd_start_pins_to_start_new_point") == 0 && i+1 < argc)
		{
            xy2_100_wait_after_lowering_cmd_start_pins_to_start_new_point = strtoul(argv[++i], NULL, 0);
        }
		else if (strcmp(argv[i], "--point_fetch_properties") == 0 && i+1 < argc)
		{
            point_fetch_properties = strtoul(argv[++i], NULL, 0);
        }
        else if (strcmp(argv[i], "--quiet") == 0)
		{
            verbose = 0;
        }
        else if (strcmp(argv[i], "--help") == 0)
		{
            printf("Usage: %s [options]\n", argv[0]);
            printf("Options:\n");
            printf("  --galvo_point_start_address_value <addr>    Set galvo_point_start_address_value (32-bit)\n");
            printf("  --galvo_point_end_address_value <addr>      Set galvo_point_end_address_value (32-bit)\n");
			
			printf("  --galvo_clock_division <value>                                            Set galvo_clock_division (32-bit)\n");
			printf("  --galvo_1_period_of_wait_for_move_to_target <value>                       Set galvo_1_period_of_wait_for_move_to_target (32-bit)\n");
			printf("  --galvo_2_period_of_wait_for_move_to_target <value>                       Set galvo_2_period_of_wait_for_move_to_target (32-bit)\n");
			printf("  --galvo_1_period_of_wait_for_laser_on <value>                             Set galvo_1_period_of_wait_for_laser_on (32-bit)\n");
			printf("  --galvo_2_period_of_wait_for_laser_on <value>                             Set galvo_2_period_of_wait_for_laser_on (32-bit)\n");
			printf("  --galvo_1_period_of_wait_for_laser_off <value>                            Set galvo_1_period_of_wait_for_laser_off (32-bit)\n");
			printf("  --galvo_2_period_of_wait_for_laser_off <value>                            Set galvo_2_period_of_wait_for_laser_off (32-bit)\n");
			printf("  --xy2_100_wait_after_start_to_set_cmd_start_pins_to_low <value>           Set xy2_100_wait_after_start_to_set_cmd_start_pins_to_low (32-bit)\n");
			printf("  --xy2_100_wait_after_lowering_cmd_start_pins_to_start_new_point <value>   Set xy2_100_wait_after_lowering_cmd_start_pins_to_start_new_point (32-bit)\n");
			printf("  --point_fetch_properties <value>   Set point_fetch_properties (32-bit)\n");
			
            printf("  --quiet           Suppress output messages\n");
            printf("  --help            Show this help message\n");
            return 0;
        }
    }
    if (verbose)
	{
        printf("Starting with parameters:\n");
        printf("  galvo_point_start_address_value: 0x%08x\n", galvo_point_start_address_value);
        printf("  galvo_point_end_address_value:   0x%08x\n", galvo_point_end_address_value);
		printf("  galvo_clock_division:   %d\n", galvo_clock_division);
		printf("  galvo_1_period_of_wait_for_move_to_target:   %d\n", galvo_1_period_of_wait_for_move_to_target);
		printf("  galvo_2_period_of_wait_for_move_to_target:   %d\n", galvo_2_period_of_wait_for_move_to_target);
		printf("  galvo_1_period_of_wait_for_laser_on:   %d\n", galvo_1_period_of_wait_for_laser_on);
		printf("  galvo_2_period_of_wait_for_laser_on:   %d\n", galvo_2_period_of_wait_for_laser_on);
		printf("  galvo_1_period_of_wait_for_laser_off:   %d\n", galvo_1_period_of_wait_for_laser_off);
		printf("  galvo_2_period_of_wait_for_laser_off:   %d\n", galvo_2_period_of_wait_for_laser_off);
		printf("  xy2_100_wait_after_start_to_set_cmd_start_pins_to_low:   %d\n", xy2_100_wait_after_start_to_set_cmd_start_pins_to_low);
		printf("  xy2_100_wait_after_lowering_cmd_start_pins_to_start_new_point:   %d\n", xy2_100_wait_after_lowering_cmd_start_pins_to_start_new_point);
		printf("  point_fetch_properties:   %d\n", point_fetch_properties);
    }
    /*// [Keep your existing mmap setup code...]
    if (verbose) {
        printf("Hello World!\n");
    }*/
	
	
	
	
	
	
	
	
	
	
	
	
	
	uint32_t galvo_point_current_address = 0x00000000;
	uint32_t galvo_point_current_address_prev = 0x00000000;
	uint32_t galvo_feedback = 0x00000000;
	uint32_t galvo_feedback_prev = 0x00000000;

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
	
	galvo_control_register_ptr = virtual_base + ( ( unsigned long  )( ALT_LWFPGASLVS_OFST + GALVO_CONTROL_REGISTER_BASE ) & ( unsigned long)( HW_REGS_MASK ) );
	
	galvo_point_start_address_ptr = virtual_base + ( ( unsigned long  )( ALT_LWFPGASLVS_OFST + GALVO_POINT_START_ADDRESS_BASE ) & ( unsigned long)( HW_REGS_MASK ) );
	galvo_point_end_address_ptr = virtual_base + ( ( unsigned long  )( ALT_LWFPGASLVS_OFST + GALVO_POINT_END_ADDRESS_BASE ) & ( unsigned long)( HW_REGS_MASK ) );
	
	
	galvo_clock_division_ptr = virtual_base + ( ( unsigned long  )( ALT_LWFPGASLVS_OFST + GALVO_CLOCK_DIVISION_BASE ) & ( unsigned long)( HW_REGS_MASK ) );
	galvo_1_period_of_wait_for_move_to_target_ptr = virtual_base + ( ( unsigned long  )( ALT_LWFPGASLVS_OFST + GALVO_1_PERIOD_OF_WAIT_FOR_MOVE_TO_TARGET_BASE ) & ( unsigned long)( HW_REGS_MASK ) );
	galvo_2_period_of_wait_for_move_to_target_ptr = virtual_base + ( ( unsigned long  )( ALT_LWFPGASLVS_OFST + GALVO_2_PERIOD_OF_WAIT_FOR_MOVE_TO_TARGET_BASE ) & ( unsigned long)( HW_REGS_MASK ) );
	galvo_1_period_of_wait_for_laser_on_ptr = virtual_base + ( ( unsigned long  )( ALT_LWFPGASLVS_OFST + GALVO_1_PERIOD_OF_WAIT_FOR_LASER_ON_BASE ) & ( unsigned long)( HW_REGS_MASK ) );
	galvo_2_period_of_wait_for_laser_on_ptr = virtual_base + ( ( unsigned long  )( ALT_LWFPGASLVS_OFST + GALVO_2_PERIOD_OF_WAIT_FOR_LASER_ON_BASE ) & ( unsigned long)( HW_REGS_MASK ) );
	galvo_1_period_of_wait_for_laser_off_ptr = virtual_base + ( ( unsigned long  )( ALT_LWFPGASLVS_OFST + GALVO_1_PERIOD_OF_WAIT_FOR_LASER_OFF_BASE ) & ( unsigned long)( HW_REGS_MASK ) );
	galvo_2_period_of_wait_for_laser_off_ptr = virtual_base + ( ( unsigned long  )( ALT_LWFPGASLVS_OFST + GALVO_2_PERIOD_OF_WAIT_FOR_LASER_OFF_BASE ) & ( unsigned long)( HW_REGS_MASK ) );
	xy2_100_wait_after_start_to_set_cmd_start_pins_to_low_ptr = virtual_base + ( ( unsigned long  )( ALT_LWFPGASLVS_OFST + XY2_100_WAIT_AFTER_START_TO_SET_CMD_START_PINS_TO_LOW_BASE ) & ( unsigned long)( HW_REGS_MASK ) );
	xy2_100_wait_after_lowering_cmd_start_pins_to_start_new_point_ptr = virtual_base + ( ( unsigned long  )( ALT_LWFPGASLVS_OFST + XY2_100_WAIT_AFTER_LOWERING_CMD_START_PINS_TO_START_NEW_POINT_BASE ) & ( unsigned long)( HW_REGS_MASK ) );
	point_fetch_properties_ptr = virtual_base + ( ( unsigned long  )( ALT_LWFPGASLVS_OFST + POINT_FETCH_PROPERTIES_BASE ) & ( unsigned long)( HW_REGS_MASK ) );
	
	
	
	galvo_point_current_address_ptr = virtual_base + ( ( unsigned long  )( ALT_LWFPGASLVS_OFST + GALVO_POINT_CURRENT_ADDRESS_BASE ) & ( unsigned long)( HW_REGS_MASK ) );
	galvo_feedback_ptr = virtual_base + ( ( unsigned long  )( ALT_LWFPGASLVS_OFST + GALVO_FEEDBACK_BASE ) & ( unsigned long)( HW_REGS_MASK ) );
	
	
	
	
	
	
	printf("Hello World!\n");
	
	*(uint32_t *)galvo_control_register_ptr = 0x00000000;
	usleep( 3000*1000 );
	
	printf("Starting laser...\n");
	
	// Start laser
	
	*(uint32_t *)galvo_point_start_address_ptr = galvo_point_start_address_value;
	*(uint32_t *)galvo_point_end_address_ptr = galvo_point_end_address_value;
	
	*(uint32_t *)galvo_clock_division_ptr = galvo_clock_division;
	*(uint32_t *)galvo_1_period_of_wait_for_move_to_target_ptr = galvo_1_period_of_wait_for_move_to_target;
	*(uint32_t *)galvo_2_period_of_wait_for_move_to_target_ptr = galvo_2_period_of_wait_for_move_to_target;
	*(uint32_t *)galvo_1_period_of_wait_for_laser_on_ptr = galvo_1_period_of_wait_for_laser_on;
	*(uint32_t *)galvo_2_period_of_wait_for_laser_on_ptr = galvo_2_period_of_wait_for_laser_on;
	*(uint32_t *)galvo_1_period_of_wait_for_laser_off_ptr = galvo_1_period_of_wait_for_laser_off;
	*(uint32_t *)galvo_2_period_of_wait_for_laser_off_ptr = galvo_2_period_of_wait_for_laser_off;
	*(uint32_t *)xy2_100_wait_after_start_to_set_cmd_start_pins_to_low_ptr = xy2_100_wait_after_start_to_set_cmd_start_pins_to_low;
	*(uint32_t *)xy2_100_wait_after_lowering_cmd_start_pins_to_start_new_point_ptr = xy2_100_wait_after_lowering_cmd_start_pins_to_start_new_point;
	*(uint32_t *)point_fetch_properties_ptr = point_fetch_properties;
	
	*(uint32_t *)galvo_control_register_ptr = 0x00000001;
	
	while( 1 )
	{
		galvo_point_current_address = *(uint32_t *)galvo_point_current_address_ptr;
		galvo_feedback = *(uint32_t *)galvo_feedback_ptr;
		
		if(galvo_point_current_address_prev != galvo_point_current_address)
		{
			galvo_point_current_address_prev = galvo_point_current_address;
			printf( "Changed Value: galvo_point_current_address=%#x\n", galvo_point_current_address);
		}
		else{}
		if(galvo_feedback_prev != galvo_feedback)
		{
			galvo_feedback_prev = galvo_feedback;
			printf( "Changed Value: galvo_feedback=%#x\n", galvo_feedback);
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
