// programma array, file es6a.cpp
#include "./../servi2.cpp"
extern "C" void raddoppia(int a[], int n);

int main(){
    int ar[5];
    int i;
    for(i=0; i<5; i++) ar[i] = leggiint();
    raddoppia(ar, 5);
    for(i=0; i<5; i++) scriviint(ar[i]);
    nuovalinea();
};