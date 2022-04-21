// programma strutturaRif, file es11b.cpp
struct s{
    int n1;
    char c;
    int n2;
};
extern "C" void fair(s& ss){
    ss.n1 = ss.n1 + 5;
    ss.c = ss.c + 1;
    ss.n2 = ss.n2 + 10;
};