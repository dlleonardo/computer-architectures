// programma strutturaLunga, file es12b.cpp
struct s{
    int n1;
    int n2;
    char a[10];
};
extern "C" s fstruct(int a, char c){
    int i;
    s st;
    st.n1 = a;
    st.n2 = 2*a;
    for(i=0; i<10; i++) st.a[i] = c + i;
    return st;
};
