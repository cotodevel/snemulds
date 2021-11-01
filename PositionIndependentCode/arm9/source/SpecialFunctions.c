#include "SpecialFunctions.h" 
#include "typedefsTGDS.h"

//Coto: the format must be as shown here in this template (2/2: SpecialFunctions.c) , and C code. So you can then in main.cpp get the function Size

int my_function1(int x, int y)
{
	while (x > y){
		x--;
	}
    return x << y;
 
}
MARK_END_OF_FUNCTION(my_function1)

//---------------------------------------------------------------

int my_function2(int x, int y)
{
    int abc = 0;
    while(x > y){
        x--;
        abc++;
    }
    if(abc > x){
        abc = y;
    }
    else{
        abc = x;
    }
    char hello[512];
    return (abc);
 
}
MARK_END_OF_FUNCTION(my_function2)

//---------------------------------------------------------------
