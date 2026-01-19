#include "chip8emu.hpp"

#include <fstream>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <sys/select.h>


Chip8::Chip8() {
    initialize();
}

void Chip8::initialize() {
    mem.fill(0);
    V.fill(0);
    I = 0;
    pc = 0x200;
    stack.fill(0);
    sp = 0;
    delayTimer = 0;
    soundTimer = 0;

    for (auto &row : display) {
        row.fill(0);
    }

    keys.fill(0);
    drawFlag = false;

    load_font();

}

void Chip8::load_font() {
    static const uint8_t font[80] = {
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
    std::memcpy(&mem[0x50], font, sizeof(font));
}


bool Chip8::load_rom(const char * fn) {
    std::ifstream f(fn, std::ios::binary | std::ios::ate);

    if(!f.is_open()) {
        std::cerr << "can't open file: " << fn << std::endl;
        return false;
    }

    std::streamsize size = f.tellg();
    f.seekg(0, std::ios::beg);

    if (size > 4096 - 0x200) {
        std::cerr << "ROM too large: " << size << "bytes" << std::endl;
        return false;
    }

    if (!f.read(reinterpret_cast<char*>(&mem[0x200]), size)) {
        std::cerr << "failed to read file: " << fn << std::endl;
        return false;
    }

    std::cout << "ROM loaded: " << fn << " (" << size << " bytes)" << std::endl;
    return true;
}

// 0xA456
void Chip8::execute_instruction(uint16_t op) {
    uint16_t nnn = op & 0x0FFF; // 0x456
    uint8_t nn = op & 0x00FF; // 0x56
    uint8_t n = op & 0x000F; // 0x6
    uint8_t x = (op & 0x0F00) >> 8; // (0x400) >> 8 = 0x4
    uint8_t y = (op & 0x00F0) >> 4; // (0x50) >> 4 = 0x5

    switch(op & 0xF000) {
        case 0x0000:
            switch(op) {
                case 0x00E0: // cls
                    for (auto& row : display) 
                        row.fill(0);

                    drawFlag = true;
                    break;

                case 0x00EE: // return @
                    pc = stack[--sp];
                    break;
            }
            break;

        case 0x1000:
            pc = nnn;
            break;
        
        case 0x2000:
            stack[sp++] = pc;
            pc = nnn;
            break;
        
        case 0x3000:
            if (V[x] == nn) pc += 2;
            break;

        case 0x4000:
            if (V[x] != nn) pc += 2;
            break;

        case 0x5000:
            if (V[x] == V[y]) pc += 2;
            break;
        
        case 0x6000:
            V[x] = nn;
            break;

        case 0x7000:
            V[x] += nn;
            break;
        
        case 0x8000:
            switch(op & 0x000F) {
                case 0x0000:
                    V[x] = V[y];
                    break;

                case 0x0001:
                    V[x] |= V[y];
                    break;

                case 0x0002:
                    V[x] &= V[y];
                    break;
                
                case 0x0003:
                    V[x] ^= V[y];
                    break;
                
                case 0x0004:
                    V[0xF] = (V[x] + V[y] > 0xFF) ? 1 : 0;
                    V[x] += V[y];
                    break;
                
                case 0x0005:
                    V[0xF] = (V[x] > V[y]) ? 1 : 0;
                    V[x] -= V[y];
                    break;

                case 0x0006:
                    V[0xF] = (V[x] & 0x1) ? 1 : 0;
                    V[x] >>= 1;
                    break;

                case 0x0007:
                    V[0xF] = (V[y] > V[x]) ? 1 : 0;
                    V[x] = V[y] - V[x];
                    break;
                
                case 0x000E:
                    V[0xF] = (V[x] & 0x80) >> 7;
                    V[x] <<= 1;
                    break;
            }
            break;

        case 0x9000:
            if (V[x] != V[y]) pc += 2;
            break;
        
        case 0xA000:
            I = nnn;
            break;
        
        case 0xB000:
            pc = nnn + V[0];
            break;

        case 0xC000:
            V[x] = (rand() % 256) & nn;
            break;
        
        case 0xD000:
            draw_sprite(V[x], V[y], n);
            break;
        
        case 0xE000:
            switch(op & 0x00FF) {
                case 0x009E:
                    if (keys[V[x]] != 0) pc +=2;
                    break;
                case 0x00A1:
                    if (keys[V[x]] == 0) pc +=2;
                    break;
            }
            break;
        
        case 0xF000:
            switch(op & 0x00FF) {
                case 0x0007:
                    V[x] = delayTimer;
                    break;
                
                case 0x000A:
                    for (int i=0; i<16; i++){
                        if (keys[i] != 0) {
                            V[x] = i;
                            return;
                        }
                    }
                    pc -= 2;
                    break;

                case 0x0015:
                    delayTimer = V[x];
                    break;
                
                case 0x0018:
                    soundTimer = V[x];
                    break;
                
                case 0x001E:
                    I += V[x];
                    break;
                
                case 0x0029:
                    I = V[x] * 5;
                    break;
                
                case 0x0033:
                    mem[I] = V[x] / 100;
                    mem[I + 1] = (V[x] / 10) % 10;
                    mem[I + 2] = V[x] % 10;
                    break;
                
                case 0x0055: // push
                    for (int i=0; i<= x; i++)
                        mem[I + i] = V[i];
                    break;

                case 0x0065: // pop
                    for (int i=0; i<= x; i++)
                        V[i] = mem[I + i];
                    break;
            }
            break;

        default:
            break;
                   
    }
}

void Chip8::render_display() {
    std::cout << "\033[2J\033[H";  
    
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 64; x++) {
            int pixel = display[y][x];
            std::cout << (pixel ? "#" : " ");
        }
        std::cout << "\n";
    }
}

void Chip8::draw_sprite(uint8_t xPos, uint8_t yPos, uint8_t height) {
    V[0xF] = 0;

    for (int row = 0; row < height; row++) {
        uint8_t spriteByte = mem[I + row];
        for (int col = 0; col < 8; col++) {
            if ((spriteByte & (0x80 >> col)) != 0) {
                uint8_t x = (xPos + col) % 64;
                uint8_t y = (yPos + row) % 32;

                if (display[y][x] == 1) {
                    V[0xF] = 1;
                }
                display[y][x] ^= 1;
            }
        }
    }

    drawFlag = true;
}

void Chip8::update_timers() {
    if (delayTimer > 0) delayTimer--;
    if (soundTimer > 0) soundTimer--;
}

void Chip8::emulate_cycle() {
    if (!running) return;
    uint16_t op = (mem[pc] << 8) | mem[pc + 1];
    pc += 2;
    execute_instruction(op);
}

void Chip8::handle_input() {
    keys.fill(0);
    
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);
    
    struct timeval tv = {0, 0};
    if (select(STDIN_FILENO + 1, &readfds, NULL, NULL, &tv) > 0) {
        char c;
        while (read(STDIN_FILENO, &c, 1) > 0) {
            switch (c) {
                case '1': keys[0x1] = 1; break;
                case '2': keys[0x2] = 1; break;
                case '3': keys[0x3] = 1; break;
                case '4': keys[0xC] = 1; break;
                case 'q': case 'Q': keys[0x4] = 1; break;
                case 'w': case 'W': keys[0x5] = 1; break;
                case 'e': case 'E': keys[0x6] = 1; break;
                case 'r': case 'R': keys[0xD] = 1; break;
                case 'a': case 'A': keys[0x7] = 1; break;
                case 's': case 'S': keys[0x8] = 1; break;
                case 'd': case 'D': keys[0x9] = 1; break;
                case 'f': case 'F': keys[0xE] = 1; break;
                case 'z': case 'Z': keys[0xA] = 1; break;
                case 'x': case 'X': keys[0x0] = 1; break;
                case 'c': case 'C': keys[0xB] = 1; break;
                case 'v': case 'V': keys[0xF] = 1; break;
                case 27: running = false; break;  
            }
        }
    }
}

