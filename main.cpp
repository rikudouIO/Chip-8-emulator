#include "chip8emu.hpp"
#include "terminalset.hpp"

#include <chrono>
#include <thread>
#include <iostream>

int main(int argc, char* argv[]) {
    const char* rom_file = "";
    
    TerminalSet terminalSet;
    Chip8 chip8;

    if (argc >= 2) {
        rom_file = argv[1];
    } else {
        std::cout << "Usage: ./chip8 <romfile.ch8>\n";
    }
    
    if (!chip8.load_rom(rom_file)) {
        return 1;
    }
    
    const int FPS = 60;
    const int cycles_per_frame = 10;
    
    while (chip8.is_running()) {
        chip8.handle_input();
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < cycles_per_frame; i++) {
            chip8.emulate_cycle();
        }
        
        chip8.update_timers();
        chip8.render_display();
        
        auto elapsed = std::chrono::high_resolution_clock::now() - start;
        auto frame_time = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed);
        
        if (frame_time.count() < 1000 / FPS) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000 / FPS - frame_time.count()));
        }
    } 
    return 0;
}
