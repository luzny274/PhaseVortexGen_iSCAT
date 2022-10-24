const short image1_size = 2;
const short image1_width = 1;

inline void drawImage1(){
	REG_PIOC_ODSR = 0b00010011111000000001001000000000;	 // 00000000 00000000 11111111	 freq1 = 0 ; freq2 = 0 ; amp = 255
	REG_PIOC_ODSR = 0b00010011111000000001001000000000;	 // 00000000 00000000 11111111	 freq1 = 0 ; freq2 = 0 ; amp = 255

}
