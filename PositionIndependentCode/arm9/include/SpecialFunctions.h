#ifndef SPECIAL_FN
#define SPECIAL_FN

#define MARK_END_OF_FUNCTION(funcname) void funcname ## _eof_marker() { }
#define SIZEOF_FUNCTION(funcname) ((unsigned long)&funcname ## _eof_marker - (unsigned long)&funcname)

#endif


#ifdef __cplusplus
extern "C"{
#endif

//Coto: the format must be as shown here in this template (1/2: SpecialFunctions.h) , and C code. So you can then in main.cpp get the function Size

extern int my_function1(int x, int y);
extern int my_function2(int x, int y);

//void xxx_eof_marker() (simple bx lr; )
extern void my_function1_eof_marker();
extern void my_function2_eof_marker();

#ifdef __cplusplus
}
#endif
