#define SCREEN_SIZE 64 * 32
#define SCREEN_WIDTH 64
#define MEMORY_SIZE 4096
#define STACK_SIZE 16
#define NR_OF_REGISTERS 16
#define NR_OF_KEYS 16
//#define VX V[(opcode & 0x0F00) >> 8]
/*	Memory map
	0x000 - 0x1FF - Chip 8 interpreter(contains font set in emu)
	0x050 - 0x0A0 - Used for the built in 4x5 pixel font set(0 - F)
	0x200 - 0xFFF - Program ROM and work RAM */
#define FONTSET_START 0x50
#define PROGRAM_ROM_START 0x200
class chip8 {
	public:
		chip8();
		void initialize();
		bool loadGame(const char* filename);
		void emulateCycle();
		void setKeys();
		void debugRender();
		
		bool drawFlag;

		unsigned char gfx[SCREEN_SIZE];		//black and white bitmap of the screen - each byte stores 1 if white, 0 if black
		unsigned char key[16];				//HEX based keypad, stores 0 if key isn't pressed, else stores non-zero

	private:
		unsigned char memory[MEMORY_SIZE];	//4KB of memory
		unsigned char V[NR_OF_REGISTERS];	//15 general purpose registers, VE - flags
		unsigned short stack[STACK_SIZE];	//stack, stores return addresses after jump instructions only

		unsigned short opcode;				//16-bit opcode - code of current operation
		unsigned short I;					//index register, used in some memory operations
		unsigned short pc;					//program counter PC = instruction pointer IP
		unsigned short sp;					//stack pointer

		//both timers count down at 60 Hz until they reach 0
		unsigned char delay_timer;			//timer used for timing events of games
		unsigned char sound_timer;			//timer used for sound effects

};
