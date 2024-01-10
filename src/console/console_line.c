#include "console/console_line.h"


struct ConLine console_line_make( screenpos_deprecated const start, usize const width )
{
    return (struct ConLine) {
        .startPos = start,
        .endPos = SCREENPOS_DEPRECATED( start.x + ( width - 1 ), start.y ),
        .width = width
    };
}

