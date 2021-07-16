#ifndef _DEBUGT_H
#define _DEBUGT_H

void my_assert(const char *file, const int line);

void my_traceW(const wchar_t *msg, ...);
void my_traceA(const char *msg, ...);

#endif