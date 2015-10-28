#ifndef KERNEL_H
#define KERNEL_H

#define KB(s) ((s) << 10)
#define MB(s) (KB(s) << 10)

#define STACK_SIZE KB(128)

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define kdebug(str) _kdebug(__FILE__ ":" STR(__LINE__) ": " str)
#define kdebugs(str) (kdebug(""), _kdebug(str), kdebug_nl())

void _kdebug(const char* s);
void kdebug_char(char c);
void kdebug_base(uint64_t value, uint32_t base);
void kdebug_nl();

void active_screensaver();

void reschedule();

void intson();
void intsoff();

//void gdt_init();

uint64_t get_ms_since_boot();

#endif
