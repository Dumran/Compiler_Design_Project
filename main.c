#include <stdio.h>
#include "interpreter.h"

int main(void)
{
    const char* myProgram =
        "n = 0;\n"
        "{ n - 2*5 ?\n"
        "  < n;\n"
        "  n = n + 1;\n"
        "}\n"
        ".\n";

    interpret(myProgram);

    return 0;
}
