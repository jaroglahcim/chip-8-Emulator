#include "chip8.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>

#define VX V[(opcode & 0x0F00) >> 8]
#define VY V[(opcode & 0x00F0) >> 4]

void gotoxy(int x, int y)
{
	COORD c;
	c.X = x - 1;
	c.Y = y - 1;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), c);
}

Chip8::Chip8()
{
	// empty
}

const unsigned char chip8_fontset[80] =
{
  0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
  0x20, 0x60, 0x20, 0x20, 0x70, // 1
  0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
  0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
  0x90, 0x90, 0xF0, 0x10, 0x10, // 4
  0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
  0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
  0xF0, 0x10, 0x20, 0x40, 0x40, // 7
  0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
  0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
  0xF0, 0x90, 0xF0, 0x90, 0x90, // A
  0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
  0xF0, 0x80, 0x80, 0x80, 0xF0, // C
  0xE0, 0x90, 0x90, 0x90, 0xE0, // D
  0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
  0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void Chip8::initialize()
{
	pc = 0x200;																// Program counter starts at 0x200
	opcode = 0;																// Reset current opcode	
	I = 0;																	// Reset index register
	sp = 0;																	// Reset stack pointer

	for (int i = 0; i < SCREEN_SIZE; ++i)									// Clear display
		gfx[i] = 0;
	for (int i = 0; i < STACK_SIZE; ++i)									// Clear stack
		stack[i] = 0;
	for (int i = 0; i < NR_OF_REGISTERS; ++i)								// Clear registers V0-VF
		V[i] = 0;
	for (int i = 0; i < MEMORY_SIZE; ++i)									// Clear memory
		memory[i] = 0;
	for (int i = 0; i < 80; ++i)											// Load fontset
		memory[i + FONTSET_START] = chip8_fontset[i];

	delay_timer = 0;														// Reset timers
	sound_timer = 0;

	srand((unsigned int) time(NULL));										// Reset time (used for rand for opcode emulation)
}

void Chip8::emulateCycle()
{
	opcode = memory[pc] << 8 | memory[pc + 1];								// Fetch opcode from memory
	
	switch (opcode & 0xF000)												// Decode opcode
	{
		case 0x0000:
			switch (opcode)
			{
				case 0x00E0:												// 00E0: Clears the screen
					for (int i = 0; i < SCREEN_SIZE; ++i)
						gfx[i] = 0;
					drawFlag = true;
					pc += 2;
				break;

				case 0x00EE:												// 00EE: Returns from a subroutine
					--sp;													// Decrease stack pointer (sp shows next stack element to be added)
					pc = stack[sp];											// Set stored address back to pc
					pc += 2;												// Increase pc to do next operation on the next cycle
					break;

				default:
					fprintf(stderr, "Unknown opcode 0x%X\n", opcode);		// Unsupported opcode if starts with four zeroes (bites) and
					pc += 2;												// not one of the two opcodes above
				break;
			}
		break;

		case 0x1000:														// 1NNN: Jumps to address NNN. 
			pc = opcode & 0x0FFF;
		break;

		case 0x2000:														// 2NNN: Calls subroutine at NNN. 
			stack[sp] = pc;													// Store current address on stack
			++sp;															// Increment stack counter
			pc = opcode & 0x0FFF;											// Set pc to NNN
		break;

		case 0x3000:														// 3XNN: Skips the next instruction if VX equals NN.
																			// (Usually the next instruction is a jump to skip a code block)
			if (VX == (opcode & 0x00FF))									// if (V[X] == NN)
				pc += 4;
			else
				pc += 2;
		break;

		case 0x4000:														// 4XNN: Skips the next instruction if VX doesn't equal NN.
																			// (Usually the next instruction is a jump to skip a code block)
			if (VX != (opcode & 0x00FF))									// if (V[X] != NN)
				pc += 4;
			else
				pc += 2;
		break;

		case 0x5000:														// 5XY0: Skips the next instruction if VX doesn't equal NN.
																			// (Usually the next instruction is a jump to skip a code block)
			if ((opcode & 0x000F) != 0)
			{
				fprintf(stderr, "Unknown opcode 0x%X\n", opcode);			// Unsupported opcode if last 4 bites not 0
				pc += 2;
				break;
			}
			if (VX == VY)		// if (V[X] == V[Y])
				pc += 4;
			else
				pc += 2;
		break;

		case 0x6000:														// 6XNN: Sets VX to NN.
			VX = opcode & 0x00FF;
			pc += 2;
		break;

		case 0x7000:														// 7XNN: Adds NN to VX. (Carry flag is not changed) 
			VX += opcode & 0x00FF;
			pc += 2;
		break;

		case 0x8000:														
			switch (opcode & 0x000F)
			{
				case 0x0000:												// 8XY0: Sets VX to the value of VY. 
					VX = VY;	
					pc += 2;
				break;

				case 0x0001:												// 8XY1 : Sets VX to VX or VY. (Bitwise OR operation)
					VX |= VY;	
					pc += 2;
				break;

				case 0x0002:												// 8XY2 : Sets VX to VX and VY. (Bitwise AND operation)
					VX &= VY;	
					pc += 2;
				break;

				case 0x0003:												// 8XY3 : Sets VX to VX xor VY. (Bitwise XOR operation)
					VX ^= VY;	
					pc += 2;
				break;

				case 0x0004:												// 8XY4 : Adds VY to VX. VF is set to 1 when there's a carry, 
																			// and to 0 when there isn't.
					if (VY > (0xFF - VX))									// V[Y] > 0xFF - V[X] <=>  V[Y] + V[X] > 0xFF
					{  														// (but there won't be an overflow on 1-byte long variables)
						V[0xF] = 1;											// Carry Flag = 1, there's a carry
					}
					else
					{
						V[0xF] = 0;											// Carry Flag = 0
					}
					VX += VY; 
					pc += 2;
				break;

				case 0x0005:												// 8XY5: VY is subtracted from VX. VF is set to 0 when there's a borrow, 
																			// and 1 when there isn't.
					if ((VX) < (VY))										// V[X] < V[Y] <=>  V[X] - V[Y] < 0
					{														// (but there won't be an underflow on 1-byte long variables)
						V[0xF] = 0;											// Borrow Flag = 0, there's a borrow
					}
					else
					{
						V[0xF] = 1;											// Borrow Flag = 1, there's no borrow
					}
					VX -= VY; 
					pc += 2;
				break;

				case 0x0006:												// 8XY6 : Stores the least significant bit of VX in VF
																			// and then shifts VX to the right by 1
					V[0xF] = VX & 0x0001;									// V[F] = V[X] & 0x0001 (least significant bit of VX)
					VX >>= 1;												// Shift VX to the right by 1
					pc += 2;
				break;

				case 0x0007:												// 8XY7: Sets VX to VY minus VX. VF is set to 0 when there's a borrow,  
																			// and 1 when there isn't.
					if (VY < VX)											// V[Y] < V[X] <=>  V[Y] - V[X] < 0
					{														// (but there won't be an underflow on 1-byte long variables)
						V[0xF] = 0;											// Borrow Flag = 0, there's a borrow
					}
					else
					{
						V[0xF] = 1;											// Borrow Flag = 1, there's no borrow
					}
					VX = VY - VX; 
					pc += 2;
				break;

				case 0x000E:												// 8XYE : Stores the most significant bit of VX in VF
																			// and then shifts VX to the left by 1
					V[0xF] = (VX) >> 7;										// V[F] = most significant bit of V[X]
					VX <<= 1;												// Shift VX to the left by 1
					pc += 2;
				break;

				default:
					fprintf(stderr, "Unknown opcode 0x%X\n", opcode);		// Unsupported opcode if last four bites differ from specified before
					pc += 2;
				break;

			}
		break;

		case 0x9000:														// 9XY0: Skips the next instruction if VX doesn't equal VY.
																			// (Usually the next instruction is a jump to skip a code block)
			if ((opcode & 0x000F) != 0)
			{
				fprintf(stderr, "Unknown opcode 0x%X\n", opcode);			// Unsupported opcode if last 4 bites not 0
				pc += 2;
				break;
			}
			if (VX != VY)		// if (V[X] != V[Y])
				pc += 4;
			else
				pc += 2;
		break;

		case 0xA000:														// ANNN: Sets I to the address NNN
			I = opcode & 0x0FFF;
			pc += 2;
		break;

		case 0xB000:														// BNNN: Jumps to the address NNN plus V0.
			pc = (opcode & 0x0FFF) + V[0];
		break;

		case 0xC000:														// CXNN: Sets VX to the result of a bitwise and operation
																			// on a random number (Typically: 0 to 255) and NN.
			VX = (rand() % 0xFF) & (opcode & 0x00FF);
			pc += 2;
		break;

		case 0xD000:
		{
			/* DXYN: Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels. Each row of 8 pixels is read
			as bit-coded starting from memory location I; I value doesn�t change after the execution of this instruction. VF is set to 1
			if any screen pixels are flipped from set to unset when the sprite is drawn, and to 0 if that doesn�t happen */

			unsigned char xStart = VX;
			unsigned char yStart = VY;
			unsigned char height = opcode & 0x000F;
			unsigned char pixels;

			V[0xF] = 0;

			for (int yLine = 0; yLine < height; ++yLine)
			{
				pixels = memory[I + yLine];									// 8 pixels from memory which are currently to be drawn
				for (int xLine = 0; xLine < 8; ++xLine)
				{
					if ((pixels & (0x80 >> xLine)) != 0)					// if pixels' (7 - xLine)th bit is 1 <=> to be drawn bit is 1
					{
						if (gfx[(yStart + yLine) * SCREEN_WIDTH	+ 			// if checked bit on screen is 1
							 (xStart + xLine)] == 1)
						{
							V[0xF] = 1;										// both bits 1, collision on screen
						}
						gfx[(yStart + yLine) * SCREEN_WIDTH					// if pixels' (7 - xLine)th bit is 1, we xor
							+ (xStart + xLine)] ^= 1;						// corresponding screen bit with 1							
					}
				}
			}
			drawFlag = true;
			pc += 2;
		}
		break;

		case 0xE000:
			switch (opcode & 0x00FF)
			{
				case 0x009E:												// EX9E: Skips the next instruction	if the key
																			// stored in VX is pressed
					if (key[VX] != 0)										// if key[V[X]] != 0
						pc += 4;
					else
						pc += 2;
				break;

				case 0xA1:													// EXA1: Skips the next instruction if the key
																			// stored in VX isn't pressed.
					if (key[VX] == 0)										// if key[V[X]] == 0
						pc += 4;
					else
						pc += 2;
				break;
			}
		break;

		case 0xF000:
			switch (opcode & 0x00FF)
			{
				case 0x0007:												// FX07: Sets VX to the value of the delay timer.
					VX = delay_timer;
					pc += 2;
				break;

				case 0x000A:												// FX0A: A key press is awaited, and then stored in VX.
																			// Blocking Operation. All instruction halted until next key event
					for (int i = 0; i < NR_OF_KEYS; ++i)					// Check all keys
					{
						if (key[i] != 0)									 
						{
							VX = i;											// If a key is pressed, send its number to VX and read next opcode
							pc += 2;										// on next cycle; else the same opcode will be read on next cycle
							break;											// until a key is pressed
						}
					}
				break;

				case 0x0015:												// FX15: Sets the delay timer to VX. 
					delay_timer = VX;
					pc += 2;
				break;

				case 0x0018:												// FX18: Sets the sound timer to VX. 
					sound_timer = VX;
					pc += 2;
				break;

				case 0x001E:												// FX18: Adds VX to I.
					if ((I + VX) > 0xFFF)									// VF is set to 1 when there is a range overflow (I+VX>0xFFF),
						V[0xF] = 1;											// and to 0 when there isn't. This is an undocumented feature of
					else													// the CHIP-8 and used by the Spacefight 2091! game.
						V[0xF] = 0;
					I += VX;
					pc += 2;
				break;

				case 0x0029:												// FX29: Sets I to the location of the sprite for the character in VX.
																			// Characters 0 - F(in hexadecimal) are represented by a 4x5 font.
					I = memory[FONTSET_START + 5 * VX];
					pc += 2;
				break;

				case 0x0033:												// FX33: Takes the decimal representation of VX, places the hundreds digit
																			// in memory at location in I, the tens digit at location I + 1,
																			// and the ones digit at location I + 2
					memory[I] = (VX) / 100;
					memory[I+1] = (VX % 100) / 10;
					memory[I+2] = (VX % 10);
					pc += 2;
				break;

				case 0x0055:												// FX55: Stores V0 to VX (including VX) in memory starting at address I
					for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)     
						memory[I + i] = V[i];
					pc += 2;
				break;

				case 0x0065:												// FX65: Fills V0 to VX (including VX) from memory starting at address I								
					for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
						V[i] = memory[I + i];
					pc += 2;
				break;
			}
		break;

		default:															
			fprintf(stderr, "Unknown opcode 0x%X\n", opcode);				// Unsupported opcode
			pc += 2;
		break;
	}
}

void Chip8::timersTick()
{
	if (delay_timer > 0)													// update delay timer
		--delay_timer;
	if (sound_timer > 0)													// update sound timer
	{
		if (sound_timer == 1)
			printf("BEEP!\n");
		--sound_timer;
	}

}

void Chip8::debugRender()													//for testing only
{
	// Draw
	//system("CLS");
	gotoxy(0, 0);
	for (int y = 0; y < 32; ++y)
	{
		for (int x = 0; x < 64; ++x)
		{
			if (gfx[(y * 64) + x] == 0)
				printf(" ");
			else
				printf("#");
		}
		printf("\n");
	}
	printf("\n");
}

bool Chip8::loadGame(const char* filename)
{
	printf("Loading: %s\n", filename);

	FILE* f;
	fopen_s(&f, filename, "rb");											// Open file
	if (f == NULL)
	{
		fprintf(stderr, "Error loading file.\n");
		return false;
	}

	fseek(f, 0L, SEEK_END);													// Check file size - position indicator used in fseek keeps track of 
																			// how many bytes it went through, ergo contains size of the file.
	long size = ftell(f);													// we find out its value through ftell																
	fseek(f, 0L, SEEK_SET);													// set the position indicator back to the beginning of the file
	
	if (size > MEMORY_SIZE - PROGRAM_ROM_START)
	{
		fprintf(stderr, "ROM is too big for CHIP-8 memory.\n");
		return false;
	}
	char* buffer = (char*)malloc(size);										// allocating memory
	if (buffer == NULL)
	{
		fprintf(stderr, "Error allocating memory.\n");
		return false;
	}
	if (fread(buffer, 1, size, f) != size)									// Storing data from stream to allocated memory buffer
	{
		fprintf(stderr, "Error reading from memory.\n");
		return false;
	}
	for (int i = 0; i < size; ++i)											// Storing data in emulated CHIP-8 memory
		memory[i + PROGRAM_ROM_START] = buffer[i];
	
	fclose(f);																// Closing file, freeing memory from buffer
	free(buffer);
	return true;
}

