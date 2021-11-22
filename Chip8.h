
#ifndef _CHIP8_H_
#define _CHIP8_H_

#include <SFML/Window.hpp>

#include <cstdint>
#include <string>

#define HEIGHT 32
#define WIDTH 64

#define INSTRUCTION_SIZE 2

class Chip8 {
private:
    uint8_t memory[4096];
    uint8_t V[16];
    uint16_t I;
    uint16_t pc; //program counter
    uint16_t stack[16];
    uint16_t sp; //stack pointer
    uint8_t delayTimer;
    uint64_t lastTime;
    bool drawFlag;

    bool** graphics;

public:
    explicit Chip8(const std::string& filename);
    ~Chip8();
    void emulateCycle();
    bool getDrawFlag() const;
    bool** getGraphics() const;
};


#endif //_CHIP8_H_
