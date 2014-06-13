/* 
 * File:   ConsoleUtilities.h
 * Author: nicco
 *
 * Created on June 12, 2014, 12:44 PM
 */

#pragma once

#if __linux
/*
 * Warning: This method is not thread safe.
 */
int _kbhit();
void RestoreTermIOS();
int _getch();
#endif // __linux