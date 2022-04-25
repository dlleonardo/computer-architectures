// programma strutturaLungaRif, file es13b.cpp
struct s{
    int n1;
    int n2;
    char a[10];
};
extern "C" void fstructr(s& st, int a, char c){
    int i;
    st.n1 = a;
    st.n2 = 2*a;
    for(i=0; i<10; i++) st.a[i] = c + i;
}