#include<stdio.h>
#include<stdint.h>
#include<stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <windows.h>
#include "SDL3\SDL.h"
#include "SDL3\SDL_oldnames.h"


#define FONTSET_SIZE 80
#define FONTSET_START_ADDRESS 0x50

#define VIDEO_WIDTH 64
#define VIDEO_HEIGHT 32

#define START_ADDRESS 0x200


    uint8_t registers[16];
    wchar_t memory[4098];
    uint8_t sp;
    uint8_t delayTimer;
    uint8_t soundTimer;
    uint8_t keypad[16];
    uint16_t index;
    uint16_t pc;
    uint16_t stack[16];
    uint32_t video[64*32];
    uint16_t opcode;
    
    uint64_t last_timer_update = 0;


    void printMemory(){
        for(int i = 0; i < 4096; i++){
            printf("%02hX ", memory[i]);
            //printf("\n %d : %02hX ",i , memory[i]);
            if( i%50 == 0) printf("\n");    
        }
        printf("\n");
    }
    void printRegisters(){
        for(int i = 0; i< 16 ; i++){
            printf(" %d - %d \n", i, registers[i]);
        }
    }

//Instruction Functions

#pragma region InstructionsFunction

    void OP_00E0(){
        //--printf("Starting to set video to 0");
        memset(video, 0, sizeof(video));
        //--printf("\n executing OP_00E0");
        //--printf("\n setting video to 0\n");
    }

    void OP_00EE(){
        //--printf("\n exeuting OP_00EE");
        //--printf("\n Current Sp : %d", sp);
        --sp;
        pc = stack[sp];
        //--printf("\n exeuting decreasin Stack pointer, current value %d\n", sp);
    }

    void OP_1nnn(){
        uint16_t address = opcode & 0x0FFFu;
        pc = address;
        //--printf("\n executing OP_1nnn");
        //--printf("\n jumping to %02hX\n",address);
    }

    void OP_2nnn(){
        uint16_t address = opcode & 0x0FFFu;

        stack[sp] = pc;
        ++sp;
        pc = address;
        //--printf("\n exeuting OP_2nnn");
        //--printf("\n pushing current pc to stack and current pc is %02hX\n", pc);
    }

    void OP_3xkk(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t byte = opcode & 0x00FFu;

        if (registers[Vx] == byte)
        {
            pc += 2;
        }
        //--printf("\n executing OP_3xkk");
        //--printf("\n increasing pc+=2 if Vx==byte: byte %d\n",byte);
    }

    void OP_4xkk(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t byte = opcode & 0x00FFu;

        if (registers[Vx] != byte)
        {
            pc += 2;
        }
        //--printf("\n executing OP_4xkk");
        //--printf("\n increasing pc+=2 if Vx!=byte: byte %d\n",byte);
    }

    void OP_5xy0(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t Vy = (opcode & 0x00F0u) >> 4u;

        if (registers[Vx] == registers[Vy])
        {
            pc += 2;
        }
        //--printf("\n executing OP_5xy0");
        //--printf("\n increasing pc+=2 if Vx==Vy: pc %d\n",pc);
    }

    void OP_6xkk(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t byte = opcode & 0x00FFu;

        registers[Vx] = byte;
        //--printf("\n executing OP_6xkk");
        //--printf("\n setting register Vx to %d\n",byte);
    }

    void OP_7xkk(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t byte = opcode & 0x00FFu;

        registers[Vx] += byte;
        //--printf("\n executing OP_7xkk");
        //--printf("\n Adding %d to register Vx and resulted in \n",byte, registers[Vx]);
    }

    void OP_8xy0(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t Vy = (opcode & 0x00F0u) >> 4u;

        registers[Vx] = registers[Vy];
        //--printf("\n executing OP_8xy0");
        //--printf("\n setting register Vx with Vy to %d\n",registers[Vy]);
    }

    void OP_8xy1(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t Vy = (opcode & 0x00F0u) >> 4u;

        registers[Vx] |= registers[Vy];
        //--printf("\n executing OP_8xy1");
        //--printf("\n OR Vx with Vy and store in Vx, result : %d\n",registers[Vx]);
    }

    void OP_8xy2(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t Vy = (opcode & 0x00F0u) >> 4u;

        registers[Vx] &= registers[Vy];
        //--printf("\n executing OP_8xy2");
        //--printf("\n And Vx with Vy and store in Vx, result : %d\n",registers[Vx]);
    }

    void OP_8xy3(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t Vy = (opcode & 0x00F0u) >> 4u;

        registers[Vx] ^= registers[Vy];
        //--printf("\n executing OP_8xy3");
        //--printf("\n XOR Vx with Vy and store in Vx, result : %d\n",registers[Vx]);
    }

    void OP_8xy4(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t Vy = (opcode & 0x00F0u) >> 4u;

        uint16_t sum = registers[Vx] + registers[Vy];

        if (sum > 255U){
            registers[0xF] = 1;
        }else{
            registers[0xF] = 0;
        }

        registers[Vx] = sum & 0xFFu;

        //--printf("\n executing OP_8xy4");
        //--printf("\n Adding Vx with Vy and store in Vx, result : %d\n",registers[Vx]);
    }

    void OP_8xy5(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t Vy = (opcode & 0x00F0u) >> 4u;

        //--printf("\n executing OP_8xy5");
        //--printf("\n%02hx - %d", opcode, opcode);
        //--printf("\n%d - %d ", Vx, Vy);
        //--printf("\n%d - %d ", registers[Vx], registers[Vy]);

        if (registers[Vx] > registers[Vy]){
            registers[0xF] = 1;
        }else{
            registers[0xF] = 0;
        }
        
        registers[Vx] -= registers[Vy];
        //--printf("result - %d , overflow - %d", registers[Vx], registers[0XF]);
        //--printf("\n subtracting Vx = Vx - Vy and store in Vx, result : %d\n",registers[Vx]);
    }

    void OP_8xy6(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;

        // Save LSB in VF
        registers[0xF] = (registers[Vx] & 0x1u);

        registers[Vx] >>= 1;
        //--printf("\n executing OP_8xy6");
        //--printf("\n leftshift Vx by 1 and store in Vx, result : %d\n",registers[Vx]);
    }

    void OP_8xy7(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t Vy = (opcode & 0x00F0u) >> 4u;

        if (registers[Vy] > registers[Vx]){
            registers[0xF] = 1;
        }else{
            registers[0xF] = 0;
        }

        registers[Vx] = registers[Vy] - registers[Vx];

        //--printf("\n executing OP_8xy7");
        //--printf("\n Subtracting Vx = Vy -Vx and store in Vx, result : %d\n",registers[Vx]);
    }

    void OP_8xyE(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;

        // Save MSB in VF
        registers[0xF] = (registers[Vx] & 0x80u) >> 7u;

        registers[Vx] <<= 1;
        //--printf("\n executing OP_8xyE");
        //--printf("\n rightShit Vx by 1 and store in Vx, result : %d\n",registers[Vx]);
    }

    void OP_9xy0(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t Vy = (opcode & 0x00F0u) >> 4u;

        if (registers[Vx] != registers[Vy]){
            pc += 2;
        }
        //--printf("\n executing OP_9xy0");
        //--printf("\n if Vx!=Vy, then pc+=1, pc result : %d\n",pc);
    }

    void OP_Annn(){
        uint16_t address = opcode & 0x0FFFu;

        //--printf(" opcode & 0x0FFFu : %d - %0x", opcode & 0x0FFFu , opcode & 0x0FFFu );

        index = address;
        //--printf("\n executing OP_Annn");
        //--printf("\n setting index with address : index : %d\n",index);
    }

    void OP_Bnnn(){
        uint16_t address = opcode & 0x0FFFu;

        pc = registers[0] + address;
        //--printf("\n executing OP_Bnnn");
        //--printf("\n register 0 + address and store in Pc, result of pc : %d\n",pc);
    }

    void OP_Cxkk(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t byte = opcode & 0x00FFu;

        registers[Vx] = (rand()%225) & byte;
        //--printf("\n executing OP_Cxkk");
        //--printf("\n assign rand value to Vx, result : %d\n",registers[Vx]);
    }

    void OP_Dxyn(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t Vy = (opcode & 0x00F0u) >> 4u;
        uint8_t height = opcode & 0x000Fu;

        
        //--printf("\n executing OP_Dxyn");
        ////--printf("\n ================================================================Setting video buffer with value, result : %d\n",registers[Vx]);

        // Wrap if going beyond screen boundaries
        uint8_t xPos = registers[Vx] % VIDEO_WIDTH;
        uint8_t yPos = registers[Vy] % VIDEO_HEIGHT;

        registers[0xF] = 0;
        //printMemory();
        //--printf("\nbefore updating video array : xPos, %d: yPos : %d, Vx : %d, Vy : %d , index : %d\n", xPos, yPos, registers[Vx], registers[Vy], index);
        for (unsigned int row = 0; row < height; ++row){
            uint8_t spriteByte = memory[index + row];

            for (unsigned int col = 0; col < 8; ++col){
                uint8_t spritePixel = spriteByte & (0x80u >> col);
                if (((yPos + row) * VIDEO_WIDTH + (xPos + col)) > (64*32)){
                    //printf("\n\n\n\n EXEEDING VIDEO LENGTH \n\n\n\n\n\n");
                    continue;
                }
                uint32_t* screenPixel = &video[(yPos + row) * VIDEO_WIDTH + (xPos + col)];
                //--printf(" %d ", video[(yPos + row) * VIDEO_WIDTH + (xPos + col)]);
                
            }
            //--printf("\n");
        }
        //--printf("\nSprite Pixel \n");
        for (unsigned int row = 0; row < height; ++row){
            if(index+row > 4098){
                //printf("\n\n\n\n EXEEDING MEMORY LENGTH \n\n\n\n\n\n");
            }
            uint8_t spriteByte = memory[index + row];
            //--printf("\nByte : %d", spriteByte);
            for (unsigned int col = 0; col < 8; ++col){
                uint8_t spritePixel = spriteByte & (0x80u >> col);
                //--printf(" %d ", spritePixel);
                
            }
            //--printf("\n");
        }

        for (unsigned int row = 0; row < height; ++row){
            uint8_t spriteByte = memory[index + row];

            for (unsigned int col = 0; col < 8; ++col){
                uint8_t spritePixel = spriteByte & (0x80u >> col);
                if (((yPos + row) * VIDEO_WIDTH + (xPos + col)) > (64*32)){
                    //printf("\n\n\n\n EXEEDING VIDEO LENGTH \n\n\n\n\n\n");
                    continue;
                }
                uint32_t* screenPixel = &video[(yPos + row) * VIDEO_WIDTH + (xPos + col)];

                // Sprite pixel is on
                if (spritePixel){
                    // Screen pixel also on - collision
                    if (*screenPixel == 0xFFFFFFFF){
                        registers[0xF] = 1;
                    }

                    // Effectively XOR with the sprite pixel
                    *screenPixel ^= 0xFFFFFFFF;
                }
            }
        }
        //--printf("\nAfter updating video array \n");
        for (unsigned int row = 0; row < height; ++row){
            uint8_t spriteByte = memory[index + row];

            for (unsigned int col = 0; col < 8; ++col){
                uint8_t spritePixel = spriteByte & (0x80u >> col);
                if (((yPos + row) * VIDEO_WIDTH + (xPos + col)) > (64*32)){
                    //--printf("\n\n\n\n EXEEDING VIDEO LENGTH \n\n\n\n\n\n");
                    continue;
                }
                uint32_t* screenPixel = &video[(yPos + row) * VIDEO_WIDTH + (xPos + col)];
                //--printf(" %d ", video[(yPos + row) * VIDEO_WIDTH + (xPos + col)]);
                
            }
            //--printf("\n");
        }
        //--printf("\n executing OP_Dxyn");
        ////--printf("\n ================================================================Setting video buffer with value, result : %d\n",registers[Vx]);
    }

    void OP_Ex9E(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;

        uint8_t key = registers[Vx];

        if (keypad[key]){
            pc += 2;
        }
        //--printf("\n executing OP_Ex9E"); //***************************************************************************************************************************");
        //--printf("\n if keypad do pc+=2, pc result : %d ,key : %d\n",pc, key);
    }

    void OP_ExA1(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;

        uint8_t key = registers[Vx];

        if (!keypad[key]){
            pc += 2;
        }
        //--printf("\n executing OP_Ex9E");//***************************************************************************************************************************");
        //--printf("\n if ! keypad do pc+=2, pc result : %d, Key : %d\n",pc, key);
    }

    void OP_Fx07(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;

        registers[Vx] = delayTimer;
        //--printf("\n executing OP_Fx07");
        //--printf(" ***************************************************************************************************************************");
        //--printf("\n setting Vx with delayTime : register Vx %d\n",registers[Vx]);
    }

    void OP_Fx0A(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;

        if (keypad[0]){
            registers[Vx] = 0;
        }else if (keypad[1]){
            registers[Vx] = 1;
        }else if (keypad[2]){
            registers[Vx] = 2;
        }else if (keypad[3]){
            registers[Vx] = 3;
        }else if (keypad[4]){
            registers[Vx] = 4;
        }else if (keypad[5]){
            registers[Vx] = 5;
        }else if (keypad[6]){
            registers[Vx] = 6;
        }else if (keypad[7]){
            registers[Vx] = 7;
        }else if (keypad[8]){
            registers[Vx] = 8;
        }else if (keypad[9]){
            registers[Vx] = 9;
        }else if (keypad[10]){
            registers[Vx] = 10;
        }else if (keypad[11]){
            registers[Vx] = 11;
        }else if (keypad[12]){
            registers[Vx] = 12;
        }else if (keypad[13]){
            registers[Vx] = 13;
        }else if (keypad[14]){
            registers[Vx] = 14;
        }else if (keypad[15]){
            registers[Vx] = 15;
        }else{
            pc -= 2;
        }

        //--printf("\n executing OP_Fx0A");
        //--printf("\n capturing keypad and setting it in Vx register : Vx %d\n",registers[Vx]);
    }

    void OP_Fx15(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;

        delayTimer = registers[Vx];
        //--printf("\n executing OP_Fx15");
        //--printf(" ***************************************************************************************************************************");
        //--printf("\n setting delay timer from Vx register : delayTime %d\n",delayTimer);
    }

    void OP_Fx18(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;

        soundTimer = registers[Vx];
        //--printf("\n executing OP_Fx18");
        //--printf("\n setting sound timer from Vx register : %d\n",soundTimer);
    }

    void OP_Fx1E(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;

        index += registers[Vx];
        //--printf("\n executing OP_Fx1E");
        //--printf("\n setting index with += Vx register : index %d\n",index);
    }

    void OP_Fx29(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t digit = registers[Vx];

        index = FONTSET_START_ADDRESS + (5 * digit);
        //--printf("\n executing OP_Fx29");
        //--printf("\n setting index with digit from Vx register : digit %d\n",digit);
    }

    void OP_Fx33(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t value = registers[Vx];

        // Ones-place
        memory[index + 2] = value % 10;
        value /= 10;

        // Tens-place
        memory[index + 1] = value % 10;
        value /= 10;

        // Hundreds-place
        memory[index] = value % 10;
        
        //--printf("\n executing OP_Fx33");
        //--printf("\n Store BCD representation of Vx in memory locations I, I+1, and I+2 : value %d\n",value);
    }

    void OP_Fx55(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;

        for (uint8_t i = 0; i <= Vx; ++i)
        {
            memory[index + i] = registers[i];
        }
        //--printf("\n executing OP_Fx55");
        //--printf("\n assigning memory from index to index+Vx with registers till Vx : Vx address %02hX\n",Vx);
    }

    void OP_Fx65(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        
        for (uint8_t i = 0; i <= Vx; ++i){
            registers[i] = memory[index + i];
        }
        //--printf("\n executing OP_Fx65");
        //--printf("\n assigning registers from 1 to Vx with memory starting from index : starting address %02hX\n",Vx);
    }
    
#pragma endregion
    
//Function Table pointers

#pragma region FunctionPointerTable
    typedef void (*Chip8Func)();
    Chip8Func table[0xF + 1];
    Chip8Func table0[0xE + 1];
    Chip8Func table8[0xE + 1];
    Chip8Func tableE[0xE + 1];
    Chip8Func tableF[0x65 + 1];

    void Table0(){
        //--printf("\ncalling %d function in Table0",(opcode & 0x000Fu));
        (*(table0[opcode & 0x000Fu]))();
    }

    void Table8(){
        //--printf("\ncalling %d function in Table8",(opcode & 0x000Fu));
        (*(table8[opcode & 0x000Fu]))();
    }

    void TableE(){
        //--printf("\ncalling %d function in TableE",(opcode & 0x000Fu));
        (*(tableE[opcode & 0x000Fu]))();
    }

    void TableF(){
        //--printf("\ncalling %d function in TableF",(opcode & 0x000Fu));
        (*(tableF[opcode & 0x00FFu]))();
    }

    void OP_NULL(){
        //--printf("\nNULLLLLLLLL");
    }

#pragma endregion
    
#pragma region ProcessInput

    bool ProcessInput(uint8_t* keys){
		bool quit = false;
		SDL_Event event;
		while (SDL_PollEvent(&event)){
			switch (event.type){
				case SDL_EVENT_QUIT:{
					quit = true;
				} break;

				case SDL_EVENT_KEY_DOWN:{
					switch (event.key.key){
						case SDLK_ESCAPE:{
							quit = true;
						} break;

						case SDLK_X:{
							keys[0] = 1;
						} break;

						case SDLK_1:{
							keys[1] = 1;
						} break;

						case SDLK_2:{
							keys[2] = 1;
						} break;

						case SDLK_3:{
							keys[3] = 1;
						} break;

						case SDLK_Q:{
							keys[4] = 1;
						} break;

						case SDLK_W:{
							keys[5] = 1;
						} break;

						case SDLK_E:{
							keys[6] = 1;
						} break;

						case SDLK_A:{
							keys[7] = 1;
						} break;

						case SDLK_S:{
							keys[8] = 1;
						} break;

						case SDLK_D:{
							keys[9] = 1;
						} break;

						case SDLK_Z:{
							keys[0xA] = 1;
						} break;

						case SDLK_C:{
							keys[0xB] = 1;
						} break;

						case SDLK_4:{
							keys[0xC] = 1;
						} break;

						case SDLK_R:{
							keys[0xD] = 1;
						} break;

						case SDLK_F:{
							keys[0xE] = 1;
						} break;

						case SDLK_V:{
							keys[0xF] = 1;
						} break;
					}
				} break;

				case SDL_EVENT_KEY_UP:{
					switch (event.key.key){
						case SDLK_X:{
							keys[0] = 0;
						} break;

						case SDLK_1:{
							keys[1] = 0;
						} break;

						case SDLK_2:{
							keys[2] = 0;
						} break;

						case SDLK_3:{
							keys[3] = 0;
						} break;

						case SDLK_Q:{
							keys[4] = 0;
						} break;

						case SDLK_W:{
							keys[5] = 0;
						} break;

						case SDLK_E:{
							keys[6] = 0;
						} break;

						case SDLK_A:{
							keys[7] = 0;
						} break;

						case SDLK_S:{
							keys[8] = 0;
						} break;

						case SDLK_D:{
							keys[9] = 0;
						} break;

						case SDLK_Z:{
							keys[0xA] = 0;
						} break;

						case SDLK_C:{
							keys[0xB] = 0;
						} break;

						case SDLK_4:{
							keys[0xC] = 0;
						} break;

						case SDLK_R:{
							keys[0xD] = 0;
						} break;

						case SDLK_F:{
							keys[0xE] = 0;
						} break;

						case SDLK_V:{
							keys[0xF] = 0;
						} break;
					}
				} break;
			}
		}

		return quit;
	}

#pragma endregion

void update_timers() {
    uint64_t current_time = SDL_GetTicks();
    
    // 1000ms / 60Hz ~16.66ms. 
    // We check if 16ms or more has passed since the last decrement.
    if (current_time - last_timer_update >= 10) {
        
        if ( delayTimer > 0) {
            delayTimer--;
        }
        
        if (soundTimer > 0) {
            soundTimer--;
        }
        
        last_timer_update = current_time;
    }
}

//CPU Cycle
void Cycle(){
    // Fetch
	opcode = (memory[pc] << 8u) | memory[pc + 1];
    
    //--printf("\n Extracted opCode \n");
    //--printf("%04hX ", opcode);
    //--printf("\n%02hX ", (opcode & 0xF000u) >> 12u);
	// Increment the PC before we execute anything
	pc += 2;
    
	// Decode and Execute
	(*(table[(opcode & 0xF000u) >> 12u]))();
    
    //--printf("Exceuted Function\n");
	
    update_timers();
    
    //--printf("Exciting Cycle\n");
}



int main(int argc, char * argv[]){
    
    int startAddress = START_ADDRESS;
    pc = startAddress;
    int cycleDelay = 1;
    int videoScale = 10;
    int videoPitch = sizeof(video[0]) * VIDEO_WIDTH;
    
    if (argc < 2){
        printf("Usage: <ROM> <option: CycleDelay 1-10>\n");
        exit(EXIT_FAILURE);
    }
        char const* romFilename = argv[1];
        //Loading ROM
        if(argc == 3){
            cycleDelay =  (argv[2][0] - '0')*100;
        }
    {
        
        FILE* rom = fopen( romFilename , "rb");
        
        if(rom == NULL){
            printf("Unable to load ROM file");
            exit(1);
        }else{
            
            int size = -1;
            
            fseek(rom, 0, SEEK_END);
            size = ftell(rom);
            
            char buffer[4096];
            rewind(rom);
            printf(" Memory contents: \n");
            printMemory();  
            
            size_t bytesRead = fread(&buffer, 1, size, rom);

            for (long i = 0; i < size; ++i) {
                memory[startAddress + i] = (uint8_t)buffer[i];
            }
            
            
            printf("\n\nMemory contents After Loading ROM: \n");
            
            printMemory();
            
            printf("\n\nSucessfully Loaded ROM into Memory\n\n");
        }
    }
    
    //Loading Fonts
    {
        uint8_t fontset[FONTSET_SIZE] =
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

        for(int i=0; i< FONTSET_SIZE; i++){
            memory[FONTSET_START_ADDRESS+i] = fontset[i];
        }
        printf("Loaded Fontset\n");

        printf("\n Memory content after loading FONTSET ");
        printMemory();
    }

    // Mapping Table Function pointers
    {
        table[0x0] = Table0;
        table[0x1] = OP_1nnn;
        table[0x2] = OP_2nnn;
        table[0x3] = OP_3xkk;
        table[0x4] = OP_4xkk;
        table[0x5] = OP_5xy0;
        table[0x6] = OP_6xkk;
        table[0x7] = OP_7xkk;
        table[0x8] = Table8;
        table[0x9] = OP_9xy0;
        table[0xA] = OP_Annn;
        table[0xB] = OP_Bnnn;
        table[0xC] = OP_Cxkk;
        table[0xD] = OP_Dxyn;
        table[0xE] = TableE;
        table[0xF] = TableF;

        for (size_t i = 0; i <= 0xE; i++)
        {
            table0[i] = OP_NULL;
            table8[i] = OP_NULL;
            tableE[i] = OP_NULL;
        }

        table0[0x0] = OP_00E0;
        table0[0xE] = OP_00EE;

        table8[0x0] = OP_8xy0;
        table8[0x1] = OP_8xy1;
        table8[0x2] = OP_8xy2;
        table8[0x3] = OP_8xy3;
        table8[0x4] = OP_8xy4;
        table8[0x5] = OP_8xy5;
        table8[0x6] = OP_8xy6;
        table8[0x7] = OP_8xy7;
        table8[0xE] = OP_8xyE;

        tableE[0x1] = OP_ExA1;
        tableE[0xE] = OP_Ex9E;

        for (size_t i = 0; i <= 0x65; i++)
        {
            tableF[i] = OP_NULL;
        }

        tableF[0x07] = OP_Fx07;
        tableF[0x0A] = OP_Fx0A;
        tableF[0x15] = OP_Fx15;
        tableF[0x18] = OP_Fx18;
        tableF[0x1E] = OP_Fx1E;
        tableF[0x29] = OP_Fx29;
        tableF[0x33] = OP_Fx33;
        tableF[0x55] = OP_Fx55;
        tableF[0x65] = OP_Fx65;    
    }


    //SDL Initialization
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;

    printf("\n\nStarting SDL init");
    if (SDL_Init(SDL_INIT_VIDEO) == 0) {
        printf(stderr, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }
    window = SDL_CreateWindow("Chip8 Emulator", VIDEO_WIDTH*videoScale, VIDEO_HEIGHT*videoScale, 0);
    printf("\n\n\nCreated SDL Window");
    
    renderer = SDL_CreateRenderer(window, NULL);
    
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, VIDEO_WIDTH, VIDEO_HEIGHT);

    last_timer_update = SDL_GetTicks();


    bool quit = false;
	while (!quit){
        
        Sleep(cycleDelay);

		quit = ProcessInput(keypad);

        Cycle();
        //--printf("\nStarting updation");
        SDL_UpdateTexture(texture, NULL, video, videoPitch);
        
        SDL_RenderClear(renderer);
        
        SDL_RenderTexture(renderer, texture, NULL, NULL);
        
        SDL_RenderPresent(renderer);
        
        //printRegisters();
	}

    //--printf("\n\nLoop Exited");

	return 0;
        
}

//cl.exe .\Emulator.c /FeEmulator.exe /I C:\SDL\include "C:\Users\blesan\Downloads\SDL3-devel-3.4.2-VC\SDL3-3.4.2\lib\x64\SDL3.lib"