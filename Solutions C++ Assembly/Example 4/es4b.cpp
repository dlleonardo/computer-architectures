// programma puntatore, file es4b.cpp
extern "C" void add(int* p, int i){
    *p = *p + i;    // Attenzione: non p=p+i, ma *p=*p+i
                    // aritmetica degli interi e non degli indirizzi
                    // *p contiene l'indirizzo dell'intero a
};