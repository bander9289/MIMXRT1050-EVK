/*

Copyright 2011-2018 Stratify Labs, Inc

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	 http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

#include <string.h>
#include <fcntl.h>
#include <sos/sos.h>
#include <mcu/debug.h>
#include <cortexm/task.h>
#include <sos/link/types.h>
#include <mcu/arch/imxrt/mimxrt1052/fsl_common.h>
#include <mcu/arch/imxrt/mimxrt1052/fsl_iomuxc.h>

#include "board_config.h"

#define TRACE_COUNT 8
#define TRACE_FRAME_SIZE sizeof(link_trace_event_t)
#define TRACE_BUFFER_SIZE (sizeof(link_trace_event_t)*TRACE_COUNT)
static char trace_buffer[TRACE_FRAME_SIZE*TRACE_COUNT];
const ffifo_config_t board_trace_config = {
	.frame_count = TRACE_COUNT,
	.frame_size = sizeof(link_trace_event_t),
	.buffer = trace_buffer
};
ffifo_state_t board_trace_state;

extern void SystemClock_Config();

void board_trace_event(void * event){
	link_trace_event_header_t * header = event;
	devfs_async_t async;
	const devfs_device_t * trace_dev = &(devfs_list[0]);

	//write the event to the fifo
	memset(&async, 0, sizeof(devfs_async_t));
	async.tid = task_get_current();
	async.buf = event;
	async.nbyte = header->size;
	async.flags = O_RDWR;
	trace_dev->driver.write(&(trace_dev->handle), &async);
}

void board_event_handler(int event, void * args){
	switch(event){
		case MCU_BOARD_CONFIG_EVENT_ROOT_TASK_INIT:
			break;

		case MCU_BOARD_CONFIG_EVENT_ROOT_FATAL:
			//start the bootloader on a fatal event
			//mcu_core_invokebootloader(0, 0);
			if( args != 0 ){
				mcu_debug_log_error(MCU_DEBUG_SYS, "Fatal Error %s", (const char*)args);
			} else {
				mcu_debug_log_error(MCU_DEBUG_SYS, "Fatal Error unknown");
			}

			sos_led_root_error(0);
			while(1){
				;
			}
			break;

		case MCU_BOARD_CONFIG_EVENT_ROOT_INITIALIZE_CLOCK:

			//configure the IO pins -- functionality needs to move to mcu_pio_setattr()

			CLOCK_EnableClock(kCLOCK_Iomuxc);          /* iomuxc clock (iomuxc_clk_enable): 0x03u */

			//LED
			IOMUXC_SetPinMux(
						IOMUXC_GPIO_AD_B0_09_GPIO1_IO09,
						0);

			IOMUXC_SetPinConfig(
						IOMUXC_GPIO_AD_B0_09_GPIO1_IO09,
						IOMUXC_SW_PAD_CTL_PAD_DSE(7));

			IOMUXC_SetPinMux(
				 IOMUXC_GPIO_AD_B0_12_LPUART1_TX,        /* GPIO_AD_B0_12 is configured as LPUART1_TX */
				 0U);                                    /* Software Input On Field: Input Path is determined by functionality */
			IOMUXC_SetPinMux(
				 IOMUXC_GPIO_AD_B0_13_LPUART1_RX,        /* GPIO_AD_B0_13 is configured as LPUART1_RX */
				 0U);                                    /* Software Input On Field: Input Path is determined by functionality */
			IOMUXC_SetPinConfig(
				 IOMUXC_GPIO_AD_B0_12_LPUART1_TX,        /* GPIO_AD_B0_12 PAD functional properties : */
				 0x10B0u);                               /* Slew Rate Field: Slow Slew Rate
																		  Drive Strength Field: R0/6
																		  Speed Field: medium(100MHz)
																		  Open Drain Enable Field: Open Drain Disabled
																		  Pull / Keep Enable Field: Pull/Keeper Enabled
																		  Pull / Keep Select Field: Keeper
																		  Pull Up / Down Config. Field: 100K Ohm Pull Down
																		  Hyst. Enable Field: Hysteresis Disabled */
			IOMUXC_SetPinConfig(
				 IOMUXC_GPIO_AD_B0_13_LPUART1_RX,        /* GPIO_AD_B0_13 PAD functional properties : */
				 0x10B0u);                               /* Slew Rate Field: Slow Slew Rate
																		  Drive Strength Field: R0/6
																		  Speed Field: medium(100MHz)
																		  Open Drain Enable Field: Open Drain Disabled
																		  Pull / Keep Enable Field: Pull/Keeper Enabled
																		  Pull / Keep Select Field: Keeper
																		  Pull Up / Down Config. Field: 100K Ohm Pull Down
																		  Hyst. Enable Field: Hysteresis Disabled */

#if 1
			IOMUXC_SetPinMux(
				 IOMUXC_GPIO_AD_B1_06_LPUART3_TX,
				 0U);
			IOMUXC_SetPinMux(
				 IOMUXC_GPIO_AD_B1_07_LPUART3_RX,
				 0U);
			IOMUXC_SetPinConfig(
				 IOMUXC_GPIO_AD_B1_06_LPUART3_TX,
				 0x10B0u);
			IOMUXC_SetPinConfig(
				 IOMUXC_GPIO_AD_B1_07_LPUART3_RX,
				 0x10B0u);
#endif


			SystemClock_Config();


			break;

		case MCU_BOARD_CONFIG_EVENT_START_INIT:

			break;

		case MCU_BOARD_CONFIG_EVENT_START_LINK:
			mcu_debug_log_info(MCU_DEBUG_USER1, "Start LED %d");
			sos_led_startup();
			break;

		case MCU_BOARD_CONFIG_EVENT_START_FILESYSTEM:
			break;
	}
}
