#define SCREEN_SIZE 64 * 32
#define MEMORY_SIZE 4096
#define STACK_SIZE 16
#define NR_OF_REGISTERS 16
/*	Memory map
	0x000 - 0x1FF - Chip 8 interpreter(contains font set in emu)
	0x050 - 0x0A0 - Used for the built in 4x5 pixel font set(0 - F)
	0x200 - 0xFFF - Program ROM and work RAM */

class chip8 {
	public:
		chip8();
		void initialize();
		void loadGame(const char* filename);
		void emulateCycle();
		void setKeys();
		
		bool drawFlag;

		unsigned char gfx[SCREEN_SIZE];		//black and white bitmap of the screen
		unsigned char key[16];				//HEX based keypad, stores current state of keys

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
