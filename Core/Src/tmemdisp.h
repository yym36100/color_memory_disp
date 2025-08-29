#pragma once

#ifdef __cplusplus
// C++-specific code here

extern "C" {
#include <stdint.h>
#include <memory.h>
#include "main.h"
//#include "stm32f4xx_hal.h"

extern SPI_HandleTypeDef hspi2;
extern DMA_HandleTypeDef hdma_spi2_tx;

void lcd_test(void);
}
template<uint16_t Width, uint16_t Height, uint8_t bpp>
class TFrameBuff {

public:
	static constexpr size_t bits = (Width * bpp + 16) * Height + 16; // each line has 16 extra bits 8 mode select/dummy  + 8 line address + 16 extra data transfer at the end
	static constexpr uint8_t stride = (Width * bpp + 16) / 8;

	uint8_t buff[bits / 8];
	uint8_t *pData = buff + 2;

	uint8_t GetByte(uint16_t x, uint16_t y) {
		return pData[y * stride + (x >> 3)];
	}

	void SetByte(uint16_t x, uint16_t y, uint8_t b) {
		pData[y * stride + (x >> 3)] = b;
	}

	uint8_t GetSubPixel(uint16_t x, uint16_t y) {
		const uint8_t m = 0x80;
		return (GetByte(x, y) & (m >> (x & 7))) != 0;
	}

	void SetSubPixel(uint16_t x, uint16_t y, uint8_t p) {
		constexpr uint8_t m = 0x80;
		uint8_t b = GetByte(x, y);
		if (p)
			b |= (m >> (x & 7));
		else
			b &= ~(m >> (x & 7));
		SetByte(x, y, b);
	}
	//ai code
	uint16_t pixel_bit_index(uint16_t x, uint16_t y) {
		return y * (Width * bpp  + 16 ) +  x * 3;
	}

	void setPixel(uint16_t x, uint16_t y, uint8_t rgb) {
		if(x>=Width) return;
		if(y>=Height) return;
		uint16_t bitIndex = pixel_bit_index(x, y);
		uint16_t byteIndex = bitIndex >> 3;
		uint16_t bitOffset = bitIndex & 7;


		uint16_t value = rgb<<8;

		// Load 16 bits covering this pixel
		uint16_t data = (pData[byteIndex]) <<8| (pData[byteIndex + 1] << 0);
		//data = reverse_byte_hw2(data);

		// Clear the 3 bits, then insert new value
		data &= ~(0xe000 >> (bitOffset));
		data |= (value >> (bitOffset));

		// Store back
		pData[byteIndex] = data >> 8;
		pData[byteIndex + 1] = data & 0xFF;
	}
	void fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t rgb) {

		for (int j = 0; j < h; j++)
			for (int i = 0; i < w; i++)
				setPixel(x + i, y + j, rgb);
	}

};

template<uint16_t Width, uint16_t Height, uint8_t bpp>
class TMemoryDisplay {
public:
	TFrameBuff<Width, Height, bpp> fb;
	uint8_t com_inv;
	uint8_t com_inv_cnt;
	uint8_t spi_done = 1;

	static constexpr uint8_t com_inv_cycle = 5;
	static constexpr uint8_t MLCD_WR = 0x80;

	void cominv(void) {
		if (++com_inv_cnt >= com_inv_cycle) {
			com_inv ^= 0x40;
			com_inv_cnt = 0;
		}
	}

	static uint8_t reverseBitOrder(uint8_t b) {
		uint8_t temp = b;
		temp = (temp & 0xF0) >> 4 | (temp & 0x0F) << 4;
		temp = (temp & 0xCC) >> 2 | (temp & 0x33) << 2;
		temp = (temp & 0xAA) >> 1 | (temp & 0x55) << 1;
		return temp;
	}

	static uint8_t reverse_byte_hw(uint8_t b) {
		return __RBIT(b) >> 24;
	}
public:
	void update_disp(void) {

		if (spi_done) {
			spi_done = 0;
			cominv();
			fb.buff[0] = MLCD_WR | com_inv;

			scs(1);
			HAL_Delay(1);

			HAL_SPI_Transmit_DMA(&hspi2, fb.buff, fb.bits / 8);
		}
	}

	void set_pattern(uint8_t pat) {
		uint16_t y;
		memset(fb.buff, pat, sizeof(fb.buff));
		for (y = 0; y < Height; y++) {
			fb.buff[y * fb.stride + 0] = 0; 					// cmd or dummy
			fb.buff[y * fb.stride + 1] = reverse_byte_hw(y + 1); // line address
		}
	}

	void spi_tx_callback() {
		spi_done = 1;
		scs(0);
	}

	static void disp_on(uint8_t on) {
		HAL_GPIO_WritePin(DISP_GPIO_Port, DISP_Pin, (GPIO_PinState) on);
	}
	static void scs(uint8_t on) {
		HAL_GPIO_WritePin(SCS_GPIO_Port, SCS_Pin, (GPIO_PinState) on);
	}

	void init(void) {
		disp_on(0);
		scs(0);

		//disp init m2 all clear flag
		uint8_t all_clear[2] = { 0x20, 0 };
		scs(1);
		HAL_Delay(1);
		HAL_SPI_Transmit(&hspi2, all_clear, 2, 10);
		HAL_Delay(1);
		scs(0);

		// disp on
		disp_on(1);

		//t3 100us or more
		HAL_Delay(1);

		set_pattern(0xfe);

	}

};
#endif

#ifdef __cplusplus
extern "C" {
#endif

void lcd_test(void);
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi);

#ifdef __cplusplus
}
#endif
