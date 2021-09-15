#include "util.hpp"

/* TODO: implement "reverse" */

void itoa( int value, char* str )
{
    int sign;
    if ( ( sign = value ) < 0 )
    {
        value = -value;
    }

    int i = 0;
    do
    {
        str[i++] = value % 10 + '0';
    } while ( ( value /= 10 ) > 0 );

    if ( sign < 0 )
    {
        str[i++] = '-';
    }

    str[i] = '\0';
}
