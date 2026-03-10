#include<stdio.h>
#include<stdint.h>
#include<stdlib.h>

#define FONTSET_SIZE 80
#define FONTSET_START_ADDRESS 0x50

#define VIDEO_WIDTH 32
#define VIDEO_HEIGHT 64



    int registers[16];
    wchar_t memory[4098];
    int index;
    uint16_t pc;
    int stack[16];
    int sp;
    int delayTimer;
    int soundTimer;
    int keypad[16];
    int video[64*32];
    int opcode;

//Instruction Functions

#pragma region InstructionsFunction

    void OP_00E0(){
        memset(video, 0, sizeof(video));
    }

    void OP_00EE(){
        --sp;
        pc = stack[sp];
    }

    void OP_1nnn(){
        uint16_t address = opcode & 0x0FFFu;
        pc = address;
    }

    void OP_2nnn(){
        uint16_t address = opcode & 0x0FFFu;

        stack[sp] = pc;
        ++sp;
        pc = address;
    }

    void OP_3xkk(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t byte = opcode & 0x00FFu;

        if (registers[Vx] == byte)
        {
            pc += 2;
        }
    }

    void OP_4xkk(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t byte = opcode & 0x00FFu;

        if (registers[Vx] != byte)
        {
            pc += 2;
        }
    }

    void OP_5xy0(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t Vy = (opcode & 0x00F0u) >> 4u;

        if (registers[Vx] == registers[Vy])
        {
            pc += 2;
        }
    }

    void OP_6xkk(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t byte = opcode & 0x00FFu;

        registers[Vx] = byte;
    }

    void OP_7xkk(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t byte = opcode & 0x00FFu;

        registers[Vx] += byte;
    }

    void OP_8xy0(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t Vy = (opcode & 0x00F0u) >> 4u;

        registers[Vx] = registers[Vy];
    }

    void OP_8xy1(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t Vy = (opcode & 0x00F0u) >> 4u;

        registers[Vx] |= registers[Vy];
    }

    void OP_8xy2(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t Vy = (opcode & 0x00F0u) >> 4u;

        registers[Vx] &= registers[Vy];
    }

    void OP_8xy3(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t Vy = (opcode & 0x00F0u) >> 4u;

        registers[Vx] ^= registers[Vy];
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
    }

    void OP_8xy5(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t Vy = (opcode & 0x00F0u) >> 4u;

        if (registers[Vx] > registers[Vy]){
            registers[0xF] = 1;
        }else{
            registers[0xF] = 0;
        }

        registers[Vx] -= registers[Vy];
    }

    void OP_8xy6(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;

        // Save LSB in VF
        registers[0xF] = (registers[Vx] & 0x1u);

        registers[Vx] >>= 1;
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
    }

    void OP_8xyE(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;

        // Save MSB in VF
        registers[0xF] = (registers[Vx] & 0x80u) >> 7u;

        registers[Vx] <<= 1;
    }

    void OP_9xy0(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t Vy = (opcode & 0x00F0u) >> 4u;

        if (registers[Vx] != registers[Vy]){
            pc += 2;
        }
    }

    void OP_Annn(){
        uint16_t address = opcode & 0x0FFFu;

        index = address;
    }

    void OP_Bnnn(){
        uint16_t address = opcode & 0x0FFFu;

        pc = registers[0] + address;
    }

    void OP_Cxkk(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t byte = opcode & 0x00FFu;

        registers[Vx] = (rand()%225) & byte;
    }

    void OP_Dxyn(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t Vy = (opcode & 0x00F0u) >> 4u;
        uint8_t height = opcode & 0x000Fu;

        // Wrap if going beyond screen boundaries
        uint8_t xPos = registers[Vx] % VIDEO_WIDTH;
        uint8_t yPos = registers[Vy] % VIDEO_HEIGHT;

        registers[0xF] = 0;

        for (unsigned int row = 0; row < height; ++row){
            uint8_t spriteByte = memory[index + row];

            for (unsigned int col = 0; col < 8; ++col){
                uint8_t spritePixel = spriteByte & (0x80u >> col);
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
    }

    void OP_Ex9E(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;

        uint8_t key = registers[Vx];

        if (keypad[key]){
            pc += 2;
        }
    }

    void OP_ExA1(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;

        uint8_t key = registers[Vx];

        if (!keypad[key]){
            pc += 2;
        }
    }

    void OP_Fx07(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;

        registers[Vx] = delayTimer;
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
    }

    void OP_Fx15(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;

        delayTimer = registers[Vx];
    }

    void OP_Fx18(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;

        soundTimer = registers[Vx];
    }

    void OP_Fx1E(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;

        index += registers[Vx];
    }

    void OP_Fx29(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        uint8_t digit = registers[Vx];

        index = FONTSET_START_ADDRESS + (5 * digit);
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
    }

    void OP_Fx55(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;

        for (uint8_t i = 0; i <= Vx; ++i)
        {
            memory[index + i] = registers[i];
        }
    }

    void OP_Fx65(){
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;
        
        for (uint8_t i = 0; i <= Vx; ++i){
            registers[i] = memory[index + i];
        }
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
        (*(table0[opcode & 0x000Fu]))();
    }

    void Table8(){
        (*(table8[opcode & 0x000Fu]))();
    }

    void TableE(){
        (*(tableE[opcode & 0x000Fu]))();
    }

    void TableF(){
        (*(tableF[opcode & 0x00FFu]))();
    }

    void OP_NULL(){}

#pragma endregion
    

//CPU Cycle
void Cycle(){
	// Fetch
	opcode = (memory[pc] << 8u) | memory[pc + 1];

	// Increment the PC before we execute anything
	pc += 2;

	// Decode and Execute
	(*(table[(opcode & 0xF000u) >> 12u]))();

	// Decrement the delay timer if it's been set
	if (delayTimer > 0)
	{
		--delayTimer;
	}

	// Decrement the sound timer if it's been set
	if (soundTimer > 0)
	{
		--soundTimer;
	}
}



    int main(){
        
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
        
}