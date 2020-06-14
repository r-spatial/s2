
#ifndef CPP_COMPAT_H
#define CPP_COMPAT_H

#include <streambuf>

void cpp_compat_printf(const char* fmt, ...);
[[ noreturn ]] void cpp_compat_abort();
[[ noreturn ]] void cpp_compat_exit(int code);
int cpp_compat_random();
void cpp_compat_srandom(int seed);

extern std::ostream& cpp_compat_cerr;
extern std::ostream& cpp_compat_cout;

#endif
