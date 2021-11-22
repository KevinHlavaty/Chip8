
#include "Chip8.h"

#include <iostream>
#include <cstring>
#include <string>
#include <unordered_map>
#include <sys/time.h>
#include <chrono>

//60HZ
#define DELAY (1000 / 60)

static std::unordered_map<uint8_t, sf::Keyboard::Key> keys({
        {0x1, sf::Keyboard::Num1},
        {0x2, sf::Keyboard::Num2},
        {0x3, sf::Keyboard::Num3},
        {0xC, sf::Keyboard::Num4},
        {0x4, sf::Keyboard::Q},
        {0x5, sf::Keyboard::W},
        {0x6, sf::Keyboard::E},
        {0xD, sf::Keyboard::R},
        {0x7, sf::Keyboard::A},
        {0x8, sf::Keyboard::S},
        {0x9, sf::Keyboard::D},
        {0xE, sf::Keyboard::F},
        {0xA, sf::Keyboard::Z},
        {0x0, sf::Keyboard::X},
        {0xB, sf::Keyboard::C},
        {0xF, sf::Keyboard::V}
});

static uint64_t currentTimeMillis() {
    timeval tp{};
    gettimeofday(&tp, nullptr);
    return (uint64_t) tp.tv_sec * 1000L + tp.tv_usec / 1000;
}

Chip8::Chip8(const std::string& filename) {
    pc = 0x200;
    I = 0;
    sp = 0;
    drawFlag = false;
    delayTimer = 0;
    lastTime = currentTimeMillis();

    graphics = new bool*[HEIGHT];
    for(int row = 0; row < HEIGHT; row++) {
        graphics[row] = new bool[WIDTH];
    }

    memset(memory, 0, 4096);
    memset(stack, 0, 16);
    memset(V, 0, 16);
    for(int row = 0; row < HEIGHT; row++) {
        memset(graphics[row], false, WIDTH);
    }

    int index = pc;
    size_t bytesRead;
    uint8_t buffer[512];
    FILE* rom = fopen(filename.c_str(), "rb");
    while((bytesRead = fread(buffer, sizeof(uint8_t), 512, rom)) != 0) {
        for(int i = 0; i < bytesRead; i++) {
            memory[index + i] = buffer[i];
        }
        index = index + bytesRead;
    }
}

Chip8::~Chip8() = default;

void Chip8::emulateCycle() {
    drawFlag = false;
    std::cout << std::hex;
    uint64_t currentTime = currentTimeMillis();
    if(delayTimer > 0 && currentTime - lastTime > DELAY) {
        delayTimer--;
        lastTime = currentTime;
    }


    uint16_t opcode = (memory[pc] << 8 | memory[pc + 1]);
    //std::cout << std::hex << opcode << std::dec << std::endl;
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Enter))
        for(int i = 0; i < 16; i++) {
            std::cout << "V[" << i << "] = " << (int)V[i] << std::endl;
        }
    switch(opcode & 0xF000) {
        case 0x0000: {
            if(opcode == 0x00E0) { //Clear screen
                drawFlag = true;
                for(int row = 0; row < HEIGHT; row++) {
                    for(int col = 0; col < WIDTH; col++) {
                        graphics[row][col] = false;
                    }
                }
                pc += INSTRUCTION_SIZE;
            } else if(opcode == 0x00EE) { //Return from subroutine
                pc = stack[sp - 1];
                pc += INSTRUCTION_SIZE;
                sp--;
            } else { //0x0NNN. I don't know..., wiki says not necessary?

            }
        } break;

        case 0x1000: {//Jump to address
            pc = opcode & 0x0FFF;
        } break;

        case 0x2000: {//Call subroutine
            stack[sp] = pc;
            sp++;
            pc = opcode & 0x0FFF;
        } break;

        case 0x3000: { //Skip if number equal
            uint8_t registerNumber = (opcode & 0x0F00) >> 8;
            uint8_t value = opcode & 0x00FF;
            //Moves 4 if equal, else only 2
            if(V[registerNumber] == value)
                pc += INSTRUCTION_SIZE;
            pc += INSTRUCTION_SIZE;
        } break;

        case 0x4000: { //Skip if number equal
            uint8_t registerNumber = (opcode & 0x0F00) >> 8;
            uint8_t value = opcode & 0x00FF;
            //Moves 4 if not equal, else only 2
            if(V[registerNumber] != value)
                pc += INSTRUCTION_SIZE;
            pc += INSTRUCTION_SIZE;
        } break;

        case 0x5000: { //Skip if register equal
            uint8_t X = (opcode & 0x0F00) >> 8;
            uint8_t Y = (opcode & 0x00F0) >> 4;
            //Moves 4 if equal, else only 2
            if(V[X] == V[Y])
                pc += INSTRUCTION_SIZE;
            pc += INSTRUCTION_SIZE;
        } break;

        case 0x6000: { //Set register to number
            uint8_t registerNumber = (opcode & 0x0F00) >> 8;
            uint8_t value = (opcode & 0x00FF);
            V[registerNumber] = value;
            pc += INSTRUCTION_SIZE;
        } break;

        case 0x7000: { //Increment register by number
            uint8_t registerNumber = (opcode & 0x0F00) >> 8;
            uint8_t value = (opcode & 0x00FF);
            V[registerNumber] += value;
            pc += INSTRUCTION_SIZE;
        } break;

        case 0x8000: {
            uint8_t nibble4 = opcode & 0x000F;
            uint8_t X = (opcode & 0x0F00) >> 8;
            uint8_t Y = (opcode & 0x00F0) >> 4;
            if(nibble4 == 0x0) { //Assign register to number
                V[X] = V[Y];
                pc += INSTRUCTION_SIZE;
            } else if(nibble4 == 0x1) { //OR 2 registers
                V[X] = V[X] | V[Y];
                pc += INSTRUCTION_SIZE;
            } else if(nibble4 == 0x2) { //AND 2 registers
                V[X] = V[X] & V[Y];
                pc += INSTRUCTION_SIZE;
             } else if(nibble4 == 0x3) { //XOR 2 registers
                V[X] = V[X] ^ V[Y];
                pc += INSTRUCTION_SIZE;
            } else if(nibble4 == 0x4) { //Add 2 registers
                //Need to check if there was a carry
                if((int)V[X] + V[Y] > 255)
                    V[0xF] = true;
                else
                    V[0xF] = false;
                V[X] = V[X] + V[Y];
                pc += INSTRUCTION_SIZE;
            } else if(nibble4 == 0x5) { //Subtract 2 registers
                //Need to check if there was borrow
                if((signed int)V[X] - V[Y] < 0)
                    V[0xF] = false;
                else
                    V[0xF] = true;
                V[X] = V[X] - V[Y];
                pc += INSTRUCTION_SIZE;
            } else if(nibble4 == 0x6) { //Shift register left 1
                //Store rightmost bit in VF
                V[0xF] = V[X] & 0x1;
                V[X] = V[X] >> 1;
                pc += INSTRUCTION_SIZE;
            } else if(nibble4 == 0x7) { //VX = VY - VX
                //Set VF to 0 if there was a borrow, else 1 (seems backwards)
                if((signed int)V[Y] - V[X] < 0)
                    V[0xF] = false;
                else
                    V[0xF] = true;
                V[X] = V[Y] - V[X];
                pc += INSTRUCTION_SIZE;
            } else if(nibble4 == 0xE) { //Right shift register by 1
                V[0xF] = (V[X] & 0x80) >> 7;
                V[X] = V[X] << 1;
                pc += INSTRUCTION_SIZE;
            }
        } break;

        case 0x9000: { //Skip instruction if VX != VY
            uint8_t X = (opcode & 0x0F00) >> 8;
            uint8_t Y = (opcode & 0x00F0) >> 4;
            //if not equal, move 4 bytes, else only 2
            if(V[X] != V[Y])
                pc += INSTRUCTION_SIZE;
            pc += INSTRUCTION_SIZE;
        } break;

        case 0xA000: { //Set I to number
            I = opcode & 0x0FFF;
            pc += INSTRUCTION_SIZE;
        } break;

        case 0xB000: { //Jump to V0 + address
            pc = V[0x0] + (opcode & 0x0FFF);
        } break;

        case 0xC000: { //Store random number in VX
            uint8_t registerNumber = (opcode & 0x0F00) >> 8;
            uint8_t number = opcode & 0x00FF;
            V[registerNumber] = (std::rand() % 256) & number;
            pc += INSTRUCTION_SIZE;
        } break;

        case 0xD000: { //Draw sprite
            drawFlag = true;
            uint8_t X = (opcode & 0x0F00) >> 8;
            uint8_t Y = (opcode & 0x00F0) >> 4;
            uint8_t N = opcode & 0x000F;
            uint16_t tempI = I;
            V[0xF] = false;
            for(int row = V[Y]; row < V[Y] + N; row++) {
                for(int colOffset = 0; colOffset < 8; colOffset++) {
                    bool pixelToDraw = (memory[tempI] & (1 << (7 - colOffset))) > 0;
                    if(pixelToDraw && graphics[row][V[X] + colOffset])
                        V[0xF] = true;
                    if(pixelToDraw)
                        graphics[row][V[X] + colOffset] = !graphics[row][V[X] + colOffset];
                }
                tempI++;
            }
            pc += INSTRUCTION_SIZE;
        } break;

        case 0xE000: {
            uint8_t secondByte = opcode & 0x00FF;
            uint8_t X = (opcode & 0x0F00) >> 8;
            if(secondByte == 0x009E) { //Skip if key pressed
                //Move 4 if key pressed, else only 2
                if(sf::Keyboard::isKeyPressed(keys[V[X]]))
                    pc += INSTRUCTION_SIZE;
                pc += INSTRUCTION_SIZE;
            } else if(secondByte == 0x00A1) { //Skip if not key pressed
                //Move 4 if key not pressed, else only 2
                if(!(sf::Keyboard::isKeyPressed(keys[V[X]])))
                    pc += INSTRUCTION_SIZE;
                pc += INSTRUCTION_SIZE;
            }
        } break;


        case 0xF000: {
            uint8_t secondByte = opcode & 0x00FF;
            uint8_t X = (opcode & 0x0F00) >> 8;
            if(secondByte == 0x0007) { //Set VX to delay timer
                V[X] = delayTimer;
                pc += INSTRUCTION_SIZE;
            } else if(secondByte == 0x000A) { //Waits for key press
                //IF NOT KEY PRESSED, pc DOES NOT CHANGE
                int keyPressed = -1;
                for(auto& kvPair : keys) {
                    if(sf::Keyboard::isKeyPressed(kvPair.second)) {
                        keyPressed = kvPair.first;
                        break;
                    }
                }
                if(keyPressed != -1) {
                    V[X] = keyPressed;
                    pc += INSTRUCTION_SIZE;
                }
            } else if(secondByte == 0x0015) { //Set delay timer to VX
                delayTimer = V[X];
                pc += INSTRUCTION_SIZE;
            } else if(secondByte == 0x0018) { //Set sound timer to VX
                //I don't care yet
                pc += INSTRUCTION_SIZE;
            } else if(secondByte == 0x001E) { //Add VX to I
                I = I + V[X];
                pc += INSTRUCTION_SIZE;
            } else if(secondByte == 0x0029) { //Set I to character sprite
                //Soon
                I = 0x0000;
                pc += INSTRUCTION_SIZE;
            } else if(secondByte == 0x0033) { //Store binary encoding of VX
                uint8_t tempVX = V[X];
                memory[I] = tempVX / 100;
                tempVX = tempVX % 100;
                memory[I + 1] = tempVX / 10;
                tempVX = tempVX % 10;
                memory[I + 2] = tempVX;
                pc += INSTRUCTION_SIZE;
            } else if(secondByte == 0x0055) { //Register dump
                for(int i = 0; i <= X; i++) {
                    memory[I + i] = V[i];
                }
                pc += INSTRUCTION_SIZE;
            } else if(secondByte == 0x0065) { //Register fill
                for(int i = 0; i <= X; i++) {
                    V[i] = memory[I + i];
                }
                pc += INSTRUCTION_SIZE;
            }
        } break;
    }
}

bool Chip8::getDrawFlag() const {
    return drawFlag;
}

bool** Chip8::getGraphics() const {
    return (bool**)(graphics);
}