#include <termios.h>
#include <sys/select.h>
#include <unistd.h>  

class TerminalSet {
public:
    TerminalSet();
    ~TerminalSet();

    TerminalSet(const TerminalSet&) = delete;
    TerminalSet& operator=(const TerminalSet&) = delete;

    static void restore();

private:
    static bool active;
    static struct termios orig_term;
};