#include <stdlib.h>
#include "tmemdisp.h"

TMemoryDisplay<128, 128, 3> md;

uint8_t fbd[6400+2];

void lcd_test(void) {
	static int state = 0;
	static int pat=0x0;
	static uint8_t rgb=1;

	if (state == 0) {
		md.init();
		state++;
	}
	if (state == 1) {
			md.update_disp();
			state++;
		}
	if (state == 2) {
		//md.set_pattern(pat);
				md.fb.fillRect(rand()&0x7f,rand()&0x7f,rand()&0x7f,rand()&0x7f,rand()&0xe0);
				md.update_disp();
				//state++;
				pat++;
				//HAL_Delay(5);
			}
	if (state == 3) {
		md.disp_on(0);
		state++;
	}
	if(state == 5){
		state++;
		//restore c:\Users\yym\STM32CubeIDE\workspace_1.19.0\color_memory_disp\tools\ppmconvert\ppmconvert\framebuffer.bin binary &fbd
		memcpy((void*)md.fb.buff,fbd,6402);
		md.update_disp();
	}
	if(state == 6){
		md.update_vcom();
		HAL_Delay(100);

	}
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi){
	md.spi_tx_callback();
}
