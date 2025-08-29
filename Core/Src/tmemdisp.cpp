#include "tmemdisp.h"

TMemoryDisplay<128, 128, 3> md;

void lcd_test(void) {
	static int state = 0;
	static int pat=0x0;

	if (state == 0) {
		md.init();
		state++;
	}
	if (state == 1) {
			md.update_disp();
			state++;
		}
	if (state == 2) {
		md.set_pattern(pat);
				md.update_disp();
				//state++;
				pat++;
				HAL_Delay(100);
			}
	if (state == 3) {
		md.disp_on(0);
		state++;
	}

}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi){
	md.spi_tx_callback();
}
