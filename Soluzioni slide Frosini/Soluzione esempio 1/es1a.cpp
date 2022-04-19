// programma sommaintGlob, file es1a.cpp
#include "./../servi2.cpp"
extern "C" int elab1(int n, int m);
int alfa, beta;

int main(){
	int ris;
	alfa = leggiint();
	beta = leggiint();
	ris = elab1(alfa, beta);
	scriviint(ris);
	nuovalinea();
	return 0;
}
