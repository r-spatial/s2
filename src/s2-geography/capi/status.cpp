
#include <cstring>
#include <cstdarg>
#include <cstdio>


// capi typedef start
typedef struct {
    int code;
    char message[8096];
    void* private_data;
} s2_status_t;
// capi typedef end


void s2_status_reset(s2_status_t* status) {
    memset(status->message, 0, sizeof(status->message));
    status->code = 0;
}


void s2_status_set_error(s2_status_t* status, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int chars_written = vsnprintf(status->message, sizeof(status->message) - 1, fmt, args);
    va_end(args);
    status->message[chars_written] = '\0';
}


