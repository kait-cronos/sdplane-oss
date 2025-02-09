#include "include.h"

#include "nettlp_support.h"

int tlp_calculate_lstdw(uintptr_t addr, size_t count)
{
        uintptr_t end, end_start, start;

        start = (addr >> 2) << 2;
        end = addr + count;
        if ((end & 0x3) == 0)
                end_start = end - 4;
        else
                end_start = (end >> 2) << 2;

        /* corner case. count is smaller than 8 */
        if (end_start <= start)
                end_start = addr + 4;
        if (end < end_start)
                return 0;

        return ~(0xF << (end - end_start)) & 0xF;
}

int tlp_calculate_fstdw(uintptr_t addr, size_t count)
{
        uint8_t be = 0xF;

        if (count < 4)
                be = ~(0xF << count) & 0xF;

        return (be << (addr & 0x3)) & 0xF;
}

int tlp_calculate_length(uintptr_t addr, size_t count)
{
        size_t len = 0;
        uintptr_t start, end;

        start = addr & 0xFFFFFFFFFFFFFFFc;
        end = addr + count;

        len = (end - start) >> 2;

        if ((end - start) & 0x3)
                len++;

        return len;
}


