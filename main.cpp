#include "chip8emu.hpp"
#include "terminalset.hpp"

#include <chrono>
#include <thread>


int main() {

    TerminalSet terminalSet;
    Chip8 chip8;
    
    if (!chip8.load_rom("roms/Pong1.ch8")) {
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