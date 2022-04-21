// programma struttura, file es10b.cpp
struct s{
    int n1;
    char c;
    int n2;
};
extern "C" s fai(s st){
    s ss;
    ss.n1 = st.n1 + 5;
    ss.c = st.c + 1;
    ss.n2 = st.n2 + 10;
    return ss;
};