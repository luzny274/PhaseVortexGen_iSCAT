const short image9_size = 1;
const short image9_width = 1;

inline void drawImage9(){
	REG_PIOC_ODSR = 0b00010011111000000001001000000000;	 // 00000000 00000000 11111111	 freq1 = 0 ; freq2 = 0 ; amp = 255

}
