#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>
#include <list>

#include <algorithm>
#include <random>


#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb/stb_image_resize.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"


auto rng = std::default_random_engine {};

int cursor = 0;
int img_sz = 0;

unsigned char * image = new unsigned char[256 * 256];
unsigned char * debug_image = new unsigned char[128 * 128];

unsigned char * draw_order = new unsigned char[128 * 128 * 3];
int image_w = 0;
int image_h = 0;
int image_c = 0;

int image_number = 1;

std::string image_path;
int target_w = 0;

int unrolling = 0;

int randomizing_mode = 0;
int number_of_segments = 0;

int blur_resize = 0;
int zero_removal = 0;
int back_and_forth = 0;
int slowdown = 0;
int fullFOV = 0;

void load_image(){
	unsigned char * loaded_image = stbi_load(image_path.c_str(), &image_w, &image_h, &image_c, 1);
	printf("Loaded image %s with %d channels as grayscale\n", image_path.c_str(), image_c);
	image_c = 1;
	
	for(int i = 0; i < 128 * 128; i++){
		debug_image[i] = 0;
		draw_order[i] = 0;
	}

	printf("Resizing image %d x %d to %d x %d\n", image_w, image_h, target_w, target_w);
	if(blur_resize){
		stbir_resize_uint8(loaded_image , image_w , image_h , 0, 
							image, target_w, target_w, 0, 1);
	}else{
		for(int i = 0; i < target_w * target_w; i++){
			int y = (i / target_w) * image_h / target_w;
			int x = (i % target_w) * image_w / target_w;
			image[i] = loaded_image[y * image_w + x];
		}
	}
	image_w = target_w;
	image_h = target_w;

	stbi_image_free(loaded_image);
	printf("Image successfully processed\n");

	
	char txt[128];
	sprintf(txt, "processed_image%d.png", image_number);

	stbi_write_png(txt, target_w, target_w, 1, image, target_w);
}

unsigned char out[24];
unsigned char out2[32];

void putBits(int freq1, int freq2, int amplitude){
	for (int i = 7; i >= 0; i--) {
        out[7 - i] = (freq1 & 1);
        freq1 >>= 1;
    }
	for (int i = 7; i >= 0; i--) {
        out[15 - i] = (freq2 & 1);
        freq2 >>= 1;
    }
	for (int i = 7; i >= 0; i--) {
        out[23 - i] = (amplitude & 1);
        amplitude >>= 1;
    }
}

void code_color(int val, unsigned char * pixel){
	pixel[0] = std::min(std::max(0, val), 255);
	pixel[1] = std::min(std::max(0, val - 256), 255);
	pixel[2] = std::min(std::max(0, val - 512), 255);
}

void print_draw_order(int x, int y){
	unsigned char * pixel = &draw_order[y * 128 * 3 + x * 3];

	int sc = (randomizing_mode == 0 && back_and_forth == 1) ? (img_sz / 2) : img_sz;
	code_color(cursor * 768 / sc, pixel);
	cursor++;
}

void process(){
	for(int i = 0; i < 32; i++) out2[i] = 0;

	//First frequency
	for(int i = 0; i < 7; i++)
		out2[i + 2] = out[i];
	
	//Second frequency
	for(int i = 0; i < 7; i++)
		out2[13 + i] = out[8 + i];
	
	//Amplitude
	out2[9] = out[16]; //P41B9
	out2[12] = out[17]; //P51B12
	for(int i = 0; i < 5; i++)
		out2[21 + i] = out[16 + 2 + i];
	out2[28] = out[23]; //P3B28


	
	unsigned char out3[32];
	for(int i = 0; i < 32; i++) out3[i] = out2[i];
	for(int i = 0; i < 32; i++) out2[31 - i] = out3[i];

}

void writeElement(FILE * f, int freq1, int freq2, int amplitude, bool first){
	putBits(freq1, freq2, amplitude);
	process();

	if(unrolling){
		fprintf(f, "\tREG_PIOC_ODSR = 0b");
		for(int i = 0; i < 32; i++)
			fprintf(f, "%d", out2[i]);

			
		fprintf(f, ";\t //");
		for(int i = 0; i < 24; i++){
			if(i % 8 == 0)
				fprintf(f, " ");
			fprintf(f, "%d", out[i]);
		}
		
		fprintf(f, "\t freq1 = %d ; freq2 = %d ; amp = %d\n", freq1, freq2, amplitude);
		
		if(slowdown){
			fprintf(f, "\timage%d_pom = micros() - image%d_pom;\n", image_number, image_number);
		}
	}else{
		if(!first)
			fprintf(f, ",");
		else
			fprintf(f, " ");

		fprintf(f, "0b");
		for(int i = 0; i < 32; i++)
			fprintf(f, "%d", out2[i]);

			
		fprintf(f, "\t //");
		for(int i = 0; i < 24; i++){
			if(i % 8 == 0)
				fprintf(f, " ");
			fprintf(f, "%d", out[i]);
		}
		
		fprintf(f, "\t freq1 = %d ; freq2 = %d ; amp = %d\n", freq1, freq2, amplitude);
		
	}

}

int power(int b, int ex){
	int val = 1;
	for(int i = 0; i < ex; i++) val *= b;
	return val;
}

bool first = true;

void draw_pixel(FILE * f, int x, int y, bool rec_order){
	int amp = image[y * target_w + x];
	if(amp > 0 || !zero_removal){
		int scale = fullFOV ? 128 / target_w : 1;
		int freq1 = x * scale;
		int freq2 = y * scale;
		writeElement(f, freq1, freq2, amp, first);
		if(rec_order)
			print_draw_order(freq1, freq2);
		debug_image[freq1 * 128 + freq2] = amp;
		first = false;
	}
}

void line_by_line(FILE * f){
	for(int i = 0; i < target_w; i++)
		for(int ii = 0; ii < target_w; ii++ ){
			int off = i % 2 ? target_w - ii - 1 : ii;
			draw_pixel(f, off, i, true);
		}

	if(back_and_forth)
		for(int i = target_w - 1; i >= 0; i--)
			for(int ii = target_w - 1; ii >= 0; ii--){
				int off = i % 2 ? target_w - ii - 1 : ii;
				draw_pixel(f, off, i, false);
			}
}

void randomized(FILE * f){
	std::vector<int> positions;
	for(int i = 0; i < target_w * target_w; i++)
		positions.push_back(i);
		
	std::shuffle(std::begin(positions), std::end(positions), rng);
	
	for(int pos : positions){
		int i = pos / target_w;
		int ii = pos % target_w;

		int off = i % 2 ? target_w - ii - 1 : ii;
		draw_pixel(f, off, i, true);
	}
}

void segmented_randomized(FILE * f){
	int segment_w = target_w / number_of_segments;

	std::vector<int> segment_positions;
	for(int i = 0; i < number_of_segments * number_of_segments; i++)
		segment_positions.push_back(i);
		
	std::shuffle(std::begin(segment_positions), std::end(segment_positions), rng);

	for(int seg : segment_positions){
		int seg_x = (seg % number_of_segments) * segment_w;
		int seg_y = (seg / number_of_segments) * segment_w;
		
		std::vector<int> positions;
		for(int i = 0; i < segment_w * segment_w; i++)
			positions.push_back(i);
		std::shuffle(std::begin(positions), std::end(positions), rng);

		for(int pos : positions){
			int x = seg_x + (pos % segment_w);
			int y = seg_y + (pos / segment_w);
			draw_pixel(f, x, y, true);
		}
	}

}

void put_image(FILE * f){
	switch(randomizing_mode){
		case 0:
			line_by_line(f);
			break;
		case 1:
			randomized(f);
			break;
		case 2:
			segmented_randomized(f);
			break;
	}
}



void printBitInfoLine(FILE * f, int off, const char * txt){
	fprintf(f, "//");
	for(int i = 0; i < off; i++) fprintf(f, " ");
	fprintf(f, "%s\n", txt);
}

void printBitInfo(FILE * f){
	int off = 32;
	printBitInfoLine(f, off--, "PXB0");
	printBitInfoLine(f, off--, "P33B1");

	fprintf(f, "\n\n");

	printBitInfoLine(f, off--, "P34B2F1_0");
	printBitInfoLine(f, off--, "P35B3F1_1");
	printBitInfoLine(f, off--, "P36B4F1_2");
	printBitInfoLine(f, off--, "P37B5F1_3");
	printBitInfoLine(f, off--, "P38B6F1_4");
	printBitInfoLine(f, off--, "P39B7F1_5");
	printBitInfoLine(f, off--, "P40B8F1_6");

	fprintf(f, "\n\n");
	
	printBitInfoLine(f, off--, "P41B9A1_0");
	
	fprintf(f, "\n\n");
	
	printBitInfoLine(f, off--, "PXB10");
	printBitInfoLine(f, off--, "PXB11");
	
	fprintf(f, "\n\n");

	printBitInfoLine(f, off--, "P51B12A1_1");
	
	fprintf(f, "\n\n");

	printBitInfoLine(f, off--, "P50B13F2_0");
	printBitInfoLine(f, off--, "P49B14F2_1");
	printBitInfoLine(f, off--, "P48B15F2_2");
	printBitInfoLine(f, off--, "P47B16F2_3");
	printBitInfoLine(f, off--, "P46B17F2_4");
	printBitInfoLine(f, off--, "P45B18F2_5");
	printBitInfoLine(f, off--, "P44B19F2_6");
	
	fprintf(f, "\n\n");
	
	printBitInfoLine(f, off--, "PXB20");
	
	fprintf(f, "\n\n");
	
	printBitInfoLine(f, off--, "P9B21A1_2");
	printBitInfoLine(f, off--, "P8B22A1_3");
	printBitInfoLine(f, off--, "P7B23A1_4");
	printBitInfoLine(f, off--, "P6B24A1_5");
	printBitInfoLine(f, off--, "P5B25A1_6");
	
	fprintf(f, "\n\n");
	
	printBitInfoLine(f, off--, "PXB26");
	printBitInfoLine(f, off--, "PXB27");
	
	fprintf(f, "\n\n");
	
	printBitInfoLine(f, off--, "P3B28A1_7");
	
	fprintf(f, "\n\n");
}

int main(){
	
	/*printf("Loop unrolling (0/1): ");
	int outp;
	scanf("%d", &outp);
	unrolling = outp > 0;

	printf("Randomizing (0/1): ");
	scanf("%d", &outp);
	randomizing = outp > 0;
	
	printf("Blur when resizing (0/1): ");
	scanf("%d", &outp);
	blur_resize = outp > 0;

	printf("Remove zeros (0/1): ");
	scanf("%d", &outp);
	zero_removal = outp > 0;

	printf("Slow down drawing (0/1): ");
	scanf("%d", &outp);
	slowdown = outp > 0;

	if(!randomizing){
		printf("Go back and forth (0/1): ");
		scanf("%d", &outp);
		back_and_forth = outp > 0;
	}//*/

	//printf("sizeof(bool) = %llu\n\n", sizeof(bool));

	FILE * settings = fopen("settings.txt", "r");
	fscanf(settings, "Loop_unrolling =%d\n"		, &unrolling);
	fscanf(settings, "Line_by_line(0), Randomizing(1), Segment_random(2) =%d\n"		, &randomizing_mode);
	fscanf(settings, "Number_of_segmentations =%d\n"	, &number_of_segments);
	fscanf(settings, "Blur_when_resizing =%d\n"	, &blur_resize);
	fscanf(settings, "Remove_zeros =%d\n"		, &zero_removal);
	fscanf(settings, "Slow_down_drawing =%d\n"	, &slowdown);
	fscanf(settings, "Go_back_and_forth =%d\n"	, &back_and_forth);
	fscanf(settings, "Full_FOV =%d\n"			, &fullFOV);
	fclose(settings);
	
	
	printf("Loop_unrolling = %d\n"			, unrolling);
	printf("Line_by_line(0), Randomizing(1), Segment_random(2) = %d\n"				, randomizing_mode);
	printf("Number_of_segmentations = %d\n"	, number_of_segments);
	printf("Blur_when_resizing = %d\n"		, blur_resize);
	printf("Remove_zeros = %d\n"			, zero_removal);
	printf("Slow_down_drawing = %d\n"		, slowdown);
	printf("Go_back_and_forth = %d\n"		, back_and_forth);
	printf("Full_FOV = %d\n\n\n"			, fullFOV);


	printf("Image number: ");
	scanf("%d", &image_number);
	
	printf("Image path: ");
	std::cin >> image_path;

	while(target_w <= 0 || target_w > 128){
		printf("\nTarget width: ");
		scanf("%d", &target_w);
	}
	printf("\n");
	load_image();

	char txt[128];
	sprintf(txt, "image%d.h", image_number);

	FILE * f = fopen(txt, "w");

	if(zero_removal){
		for(int i = 0; i < target_w * target_w; i++)
			if(image[i] > 0)
				img_sz++;
	}else{
		img_sz = target_w * target_w;
	}
	if(back_and_forth && randomizing_mode == 0)
		img_sz *= 2;

	fprintf(f, "const short image%d_size = %d;\n", image_number, img_sz);
	fprintf(f, "const short image%d_width = %d;\n\n", image_number, target_w);

	if(slowdown)
		fprintf(f, "long image%d_pom = 0;\n\n", image_number);

	if(unrolling){
		fprintf(f, "inline void drawImage%d(){\n", image_number);
		put_image(f);

		fprintf(f, "\n}\n");
	}else{
		
		fprintf(f, "const int image%d_array[] = {\n", image_number);
		put_image(f);
		fprintf(f, "\n};\n");
		
		fprintf(f, "inline void drawImage%d(){\n\tfor(int i = 0; i < image%d_size; i++){\n\t\tREG_PIOC_ODSR = image%d_array[i];\n", image_number, image_number, image_number);
		if(slowdown)
			fprintf(f, "\t\timage%d_pom = micros() - image%d_pom;\n", image_number, image_number);
		fprintf(f, "\t}\n}\n");
	}


	fclose(f);
	
	sprintf(txt, "debug_image%d.png", image_number);
	stbi_write_png(txt, 128, 128, 1, debug_image, 128);

	sprintf(txt, "draw_order%d.png", image_number);
	stbi_write_png(txt, 128, 128, 3, draw_order, 128 * 3);

	getchar();
	printf("Press enter to exit: ");
	getchar();

	return 1;
}