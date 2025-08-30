#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define WIDTH  128
#define HEIGHT 128
#define FRAMEBUFFER_SIZE ((WIDTH * HEIGHT * 3) / 8)

uint8_t framebuffer[FRAMEBUFFER_SIZE];

// Convert 24-bit PPM RGB buffer into 3-bit framebuffer format
// inData: pointer to raw RGB data (WIDTH*HEIGHT*3 bytes)
// outData: pointer to framebuffer buffer (6144 bytes)
void convertPPMtoFB(const uint8_t* inData, uint8_t* outData) {
	int bitPos = 0;   // bit position in outData
	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
			int idx = (y * WIDTH + x) * 3;

			// Extract each channel, threshold at 128
			uint8_t r = (inData[idx + 0] > 127) ? 1 : 0;
			uint8_t g = (inData[idx + 1] > 127) ? 1 : 0;
			uint8_t b = (inData[idx + 2] > 127) ? 1 : 0;

			uint8_t val = (r << 2) | (g << 1) | b; // order: r g b in MSB..LSB

			// Insert into outData MSB-first
			int byteIndex = bitPos >> 3;
			int bitOffset = 7 - (bitPos & 7);  // MSB first

			for (int k = 2; k >= 0; k--) { // 3 bits per pixel
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
