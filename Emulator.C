#include<stdio.h>
#include<stdint.h>

#define FONTSET_SIZE 80
#define FONTSET_START_ADDRESS 0x50

//Instruction Functions

#pragma region InstructionsFunction



#pragma endregion


int main(){
    
    int registers[16];
    wchar_t memory[4098];
    int index;
    int pc;
    int stack[16];
    int sp;
    int delayTimer;
    int soundTimer;
    int keypad[16];
    int video[64*32];
    int opcode;
    
    int startAddress = 0x200;
    pc = startAddress;
    
    //Loading ROM
    {
        FILE* rom = fopen(".\\1-chip8-logo.ch8", "r");
        
        if(rom == NULL){
            printf("Unable to load ROM file");
            exit(1);
        }else{
            
            int size = -1;
            
            fseek(rom, 0, SEEK_END);
            size = ftell(rom);
            
            // char buffer[size];
            rewind(rom);
            printf(" Memory contents: \n");
            for(int i = 0; i < 4096; i++){
                memory[i] = 0;
                printf("%02hX ", memory[i]);
                if( i%50 == 0) printf("\n");
            }
            printf("\n");
            fgetws(&memory[startAddress], size, rom);
            
            
            printf("\n\nMemory contents After Loading ROM: \n");
            for(int i = 0; i < 4096; i++){
                printf("%02hX ", memory[i]);
                if( i%50 == 0) printf("\n");
            }
            printf("\n");
            
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
    }

}