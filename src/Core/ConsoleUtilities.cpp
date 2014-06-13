#include "stdafx.h"
#include "ConsoleUtilities.h"

#ifdef __linux
#include <sys/select.h>
#include <termios.h>
#include <stropts.h>
#include <sys/ioctl.h> // for FIONREAD
#endif // __linux


#if __linux

static const int STDIN = 0;
static bool initialized = false;

int _kbhit( )
{
    if( !initialized )
    {
        termios term;
        tcgetattr( STDIN, &term );
        term.c_lflag &= ~(ICANON | ECHO); // turn off echo also
        tcsetattr( STDIN, TCSANOW, &term );
        setbuf( stdin, NULL );
        initialized = true;
    }

    int bytesWaiting;
    ioctl( STDIN, FIONREAD, &bytesWaiting );
    return bytesWaiting;
}

void RestoreTermIOS( )
{
    if( initialized )
    {
        termios term;
        tcgetattr( STDIN, &term );
        term.c_lflag |= (ICANON | ECHO);
        tcsetattr( STDIN, TCSANOW, &term );
    }
}

int _getch( )
{
    return getchar( );
}
#endif // __linux