#include "random.h"

#include <stdlib.h>
#include <time.h>


bool random_init( void )
{
    srand( time( NULL ) );
    return true;
}

