#include "terminalset.hpp"

#include <unistd.h>  


bool TerminalSet::active = false;
struct termios TerminalSet::orig_term;

TerminalSet::TerminalSet() {
    if (!active) {
        tcgetattr(STDIN_FILENO, &orig_term);
        struct termios raw = orig_term;
        raw.c_lflag &= ~(ECHO | ICANON);
        raw.c_cc[VMIN] = 0;
        raw.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
        active = true;
    }
}

TerminalSet::~TerminalSet() {
    restore();
}

void TerminalSet::restore() {
    if (active) {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_term);
        active = false;
    }
}