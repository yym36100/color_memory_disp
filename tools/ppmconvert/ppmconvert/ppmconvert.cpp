#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define WIDTH   128
#define HEIGHT  128
#define LINE_PIXELS  WIDTH
#define LINE_BYTES  ((LINE_PIXELS * 3) / 8 + 2)   // 48 + 2 = 50 bytes
#define FRAMEBUFFER_SIZE (HEIGHT * LINE_BYTES)    // 6400 bytes

uint8_t framebuffer[FRAMEBUFFER_SIZE];

static uint8_t reverseBitOrder(uint8_t b) {
	uint8_t temp = b;
	temp = (temp & 0xF0) >> 4 | (temp & 0x0F) << 4;
	temp = (temp & 0xCC) >> 2 | (temp & 0x33) << 2;
	temp = (temp & 0xAA) >> 1 | (temp & 0x55) << 1;
	return temp;
}

void convertPPMtoFB(const uint8_t* inData, uint8_t* outData) {	
	for (int y = 0; y < HEIGHT; y++) {
		int bitPos = 0;
		int lineStart = y * LINE_BYTES;
		outData[lineStart] = 0;
		outData[lineStart+1] = reverseBitOrder(y+1);
		lineStart += 2;

		for (int x = 0; x < WIDTH; x++) {
			int idx = (y * WIDTH + x) * 3;

			// threshold each channel
			uint8_t r = (inData[idx + 0] > 127) ? 1 : 0;
			uint8_t g = (inData[idx + 1] > 127) ? 1 : 0;
			uint8_t b = (inData[idx + 2] > 127) ? 1 : 0;

			uint8_t val = (r << 2) | (g << 1) | b;  // RGB 3-bit

			int byteIndex = lineStart + (bitPos >> 3);
			int bitOffset = 7 - (bitPos & 7);  // MSB-first

			// write 3 bits
			for (int k = 2; k >= 0; k--) {
				int bit = (val >> k) & 1;
				outData[byteIndex] &= ~(1 << bitOffset);
				outData[byteIndex] |= (bit << bitOffset);

				bitPos++;
				if (--bitOffset < 0) {
					bitOffset = 7;
					byteIndex++;
				}
			}
		}

		// leave the last 2 padding bytes untouched
	}
}


// Example loader for binary PPM (P6 format, 24-bit RGB, no comments)
uint8_t* loadPPM(const char* filename) {
	FILE* f = fopen(filename, "rb");
	if (!f) return NULL;

	char header[3];
	int w, h, maxval;
	if (fscanf(f, "%2s %d %d %d", header, &w, &h, &maxval) != 4) {
		fclose(f);
		return NULL;
	}
	fgetc(f); // skip one whitespace after header

	if (w != WIDTH || h != HEIGHT || maxval != 255 || header[0] != 'P' || header[1] != '6') {
		fclose(f);
		return NULL;
	}

	uint8_t* data = (uint8_t*)malloc(WIDTH * HEIGHT * 3);
	fread(data, 1, WIDTH * HEIGHT * 3, f);
	fclose(f);
	return data;
}

// Example usage
int main(int argc, char* argv[])
{
	uint8_t* ppmData = loadPPM(argv[1]);
	if (!ppmData) {
		fprintf(stderr, "Failed to load PPM\n");
		return 1;
	}

	convertPPMtoFB(ppmData, framebuffer);

	// framebuffer now contains the 128x128x3bit data
	// You could fwrite it to a file if needed:
	FILE* out = fopen("framebuffer.bin", "wb");
	fwrite(framebuffer, 1, FRAMEBUFFER_SIZE, out);
	fclose(out);

	free(ppmData);
	return 0;
}
