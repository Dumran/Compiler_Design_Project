#include "interpreter.h"

int main() 
{
    const char* programText =
        "n = 0;\n"
        "{ n - 2*5 ?\n"
        "  < n;\n"
        "  n = n + 1;\n"
        "}\n"
        ".\n";

    input = programText;
    position = 0;
    ft_memset(variables, 0, sizeof(variables));

    getNextToken();
    P();

    return 0;
}
