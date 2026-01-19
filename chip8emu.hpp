#include <cstdint>
#include <array>
#include <fcntl.h>


class Chip8 {
public:
    Chip8();
    void initialize();
    bool load_rom(const char* fn);
    void emulate_cycle();
    void update_timers();
    void render_display();

    bool is_running() const { return running; };
    void handle_input();

    std::array<std::array<uint8_t, 64>, 32> display;
    std::array<uint8_t, 16> keys;
    bool drawFlag;

private:
    std::array<uint8_t, 4096> mem; //4kb memory
    std::array<uint8_t, 16> V; // regs V0 - VF
    uint16_t I; // index
    uint16_t pc; //counter
    std::array<uint16_t, 16> stack;
    uint8_t sp;

    uint8_t delayTimer;
    uint8_t soundTimer;
    bool running = true;

    void execute_instruction(uint16_t op);
    void draw_sprite(uint8_t x, uint8_t y, uint8_t height);
    void load_font();
};