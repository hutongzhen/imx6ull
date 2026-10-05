#include <stdio.h>
#include "System/Time/GetTime.h"

int main(void)
{
    if (GetTime("192.168.87.128", 9000, 5000) < 0) {
        perror("GetTime: failed to update system clock");
        return 1;
    }
    puts("System clock updated from the VM. Run date to view it.");
    return 0;
}
