// programma puntatoreRif, file es5b.cpp
extern "C" void trovamin(int*& p, int* pa, int* pb){
    if(*pa <= *pb) p = pa;
    else p = pb;
};