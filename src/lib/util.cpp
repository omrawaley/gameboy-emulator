#include "util.h"

namespace Util
{
    bool isAddressBetween(const u16 addr, const u16 start, const u16 end)
    {
        return (addr >= start && addr <= end);
    }
};