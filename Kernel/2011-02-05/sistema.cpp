// sistema.cpp
//
#include "costanti.h"
#include "libce.h"

/////////////////////////////////////////////////////////////////////////////////
//                     PROCESSI                                                //
/////////////////////////////////////////////////////////////////////////////////
const natl MAX_PRIORITY	= 0xfffffff;
const natl MIN_PRIORITY	= 0x0000001;
const natl DUMMY_PRIORITY = 0x0000000;
const int N_REG = 16;	// numero di registri nel campo contesto

// si veda in PAGINAZIONE per il significato di questi typedef
typedef natq vaddr;
typedef natq faddr;
typedef natq tab_entry;

// descrittore di processo
struct des_proc {
	// parte richiesta dall'hardware
	struct __attribute__ ((packed)) {
		natl riservato1;
		vaddr punt_nucleo;
		// due quad  a disposizione (puntatori alle pile ring 1 e 2)
		natq disp1[2];
		natq riservato2;
		//entry della IST, non usata
		natq disp2[7];
		natq riservato3;
		natw riservato4;
		natw iomap_base; // si veda crea_processo()
	};
	//finiti i campi obbligatori
	faddr cr3;
	natq contesto[N_REG];
	natl cpl;
};

// numero di processi utente attivi
volatile natl processi;
// distrugge il processo puntato da esecuzione
extern "C" void c_abort_p();
// dato un id, restiusce il puntatore al corrispondente des_proc
// (definita in sistema.s)
extern "C" des_proc *des_p(natl id);
// id del procsso dummy (creato durante l'inizializzazione)
natl dummy_proc;

//indici nell'array contesto
enum { I_RAX, I_RCX, I_RDX, I_RBX,
	I_RSP, I_RBP, I_RSI, I_RDI, I_R8, I_R9, I_R10,
	I_R11, I_R12, I_R13, I_R14, I_R15 };
// )

// elemento di una coda di processi
struct proc_elem {
	natl id;
	natl precedenza;
	proc_elem *puntatore;
};
proc_elem *esecuzione;
proc_elem *pronti;

void inserimento_lista(proc_elem *&p_lista, proc_elem *p_elem)
{
// ( inserimento in una lista semplice ordinata
//   (tecnica dei due puntatori)
	proc_elem *pp, *prevp;

	pp = p_lista;
	prevp = 0;
	while (pp != 0 && pp->precedenza >= p_elem->precedenza) {
		prevp = pp;
		pp = pp->puntatore;
	}

	p_elem->puntatore = pp;

	if (prevp == 0)
		p_lista = p_elem;
	else
		prevp->puntatore = p_elem;

// )
}

// rimuove da p_lista il processo a piu' altra priorita' e
// lo restituisce in p_elem
void rimozione_lista(proc_elem *&p_lista, proc_elem *&p_elem)
{
// ( estrazione dalla testa
	p_elem = p_lista;  	// 0 se la lista e' vuota

	if (p_lista)
		p_lista = p_lista->puntatore;

	if (p_elem)
		p_elem->puntatore = 0;
// )
}

// inserisce esecuzione in testa alla lista pronti
extern "C" void inspronti()
{
// (
	esecuzione->puntatore = pronti;
	pronti = esecuzione;
// )
}

// sceglie il prossimo processo da mettere in esecuzione
extern "C" void schedulatore(void)
{
// ( poiche' la lista e' gia' ordinata in base alla priorita',
//   e' sufficiente estrarre l'elemento in testa
	rimozione_lista(pronti, esecuzione);
// )
}

/////////////////////////////////////////////////////////////////////////////////
//                     SEMAFORI                                                //
/////////////////////////////////////////////////////////////////////////////////

// descrittore di semaforo
struct des_sem {
	int counter;
	proc_elem *pointer;
};

// vettore dei descrittori di semaforo
des_sem array_dess[MAX_SEM];

// ( ESAME 2011-02-05

struct des_rw {
	natl readers[MAX_RW_READERS];
	natl writer;
	natl nreaders;
	proc_elem* w_readers;
	proc_elem* w_writers;
};

des_rw array_desrw[MAX_RW];
natl rw_allocati = 0;

natl alloca_rw()
{
	natl i;

	if (rw_allocati >= MAX_RW)
		return 0xFFFFFFFF;

	i = rw_allocati;
	rw_allocati++;
	return i;
}

bool rw_valido(natl rw)
{
	return rw < rw_allocati;
}
// ESAME 2011-02-05 )

// - per sem_ini, si veda [P_SEM_ALLOC] avanti
extern "C" natl sem_ini(int);

//   sem_valido: restituisce true se sem e' un semaforo effettivamente allocato
bool sem_valido(natl sem);
// )


extern "C" void c_sem_wait(natl sem)
{
	des_sem *s;

// (* una primitiva non deve mai fidarsi dei parametri
	if (!sem_valido(sem)) {
		flog(LOG_WARN, "semaforo errato: %d", sem);
		c_abort_p();
		return;
	}
// *)

	s = &array_dess[sem];
	(s->counter)--;

	if ((s->counter) < 0) {
		inserimento_lista(s->pointer, esecuzione);
		schedulatore();
	}
}

extern "C" void c_sem_signal(natl sem)
{
	des_sem *s;
	proc_elem *lavoro;

// (* una primitiva non deve mai fidarsi dei parametri
	if (!sem_valido(sem)) {
		flog(LOG_WARN, "semaforo errato: %d", sem);
		c_abort_p();
		return;
	}
// *)

	s = &array_dess[sem];
	(s->counter)++;

	if ((s->counter) <= 0) {
		rimozione_lista(s->pointer, lavoro);
		inspronti();	// preemption
		inserimento_lista(pronti, lavoro);
		schedulatore();	// preemption
	}
}

// ( ESAME 2011-02-05
natl alloca_rw();
extern "C" natl c_rw_init()
{
	natl rw = alloca_rw();

	if (rw == 0xFFFFFFFF)
		return 0xFFFFFFFF;

	des_rw *p_des = &array_desrw[rw];

	p_des->nreaders = 0;
	p_des->writer = 0xFFFFFFFF;
	p_des->w_readers = 0;
	p_des->w_writers = 0;
	for (int i = 0; i < MAX_RW_READERS; i++)
		p_des->readers[i] = 0xFFFFFFFF;
	return rw;
}

natl rw_find_reader(des_rw* p_des, natl r)
{
	for (int i = 0; i < MAX_RW_READERS; i++)
		if (p_des->readers[i] == r)
			return i;
	return 0xFFFFFFFF;
}


extern "C" void c_rw_acq_write(natl rw)
{
	if (!rw_valido(rw)) {
		flog(LOG_WARN, "rw non valido: %d", rw);
		c_abort_p();
		return;
	}
	
	des_rw *p_des = &array_desrw[rw];

	if (p_des->writer == esecuzione->id ||
	    rw_find_reader(p_des, esecuzione->id) != 0xFFFFFFFF) {
		flog(LOG_WARN, "tenta di ri-acquisire un diritto su %d", rw);
		c_abort_p();
		return;
	}

	if (p_des->nreaders > 0 || p_des->writer != 0xFFFFFFFF) {
		inserimento_lista(p_des->w_writers, esecuzione);
		schedulatore();
	} else {
		p_des->writer = esecuzione->id;
	}
}

extern "C" void c_rw_acq_read(natl rw)
{
	if (!rw_valido(rw)) {
		flog(LOG_WARN, "rw non valido: %d", rw);
		c_abort_p();
		return;
	}
	
	des_rw *p_des = &array_desrw[rw];

	if (p_des->writer == esecuzione->id ||
	    rw_find_reader(p_des, esecuzione->id) != 0xFFFFFFFF) {
		flog(LOG_WARN, "tenta di ri-acquisire la lettura su %d", rw);
		c_abort_p();
		return;
	}

	if (p_des->writer != 0xFFFFFFFF || p_des->nreaders == MAX_RW_READERS) {
		inserimento_lista(p_des->w_readers, esecuzione);
		schedulatore();
	} else {
		natl pos = rw_find_reader(p_des, 0xFFFFFFFF);
		p_des->readers[pos] = esecuzione->id;
		p_des->nreaders++;
	}
}

// ESAME 2011-02-05 )

// ( SOLUZIONE 2011-02-05
extern "C" c_rw_rel_write(natl rw)
{

	if(!rw_valido(rw)){
		flog(LOG_WARN, "Rw non valido \n");
		c_abort_p();
		return;
	}

	des_rw* p_des = &array_desrw[rw]; 

	natl pos = rw_find_reader(p_des, esecuzione->id);

	if(pos == natl(-1)){
		flog(LOG_WARN, "Posizione rw non valida \n");
		c_abort_p();
		return;
	}

	if(p_des->writer != esecuzione->id){
		flog(LOG_WARN, "Non ho il diritto di scrittura \n");
		c_abort_p();
		return;
	}

	inspronti();
	p_des->writer = natl(-1);

	// Dobbiamo dare la precedenza ai lettori
	if(p_des->w_readers){
			while(p_des->w_readers && p_des->nreaders < MAX_RW_READERS){
			des_proc *work = rimozione_lista(p_des->w_readers);
			// C'e' un solo scrittore per volta
			p_des->readers[p_des->nreaders] = work->id;
			p_des->nreaders++;
			inserimento_lista(pronti, work);
		}
	}
	else if(p_des->w_writers){ // presenza di scrittori
		des_proc *work = rimozione_lista(p_des->w_writers);
		p_des->writer = work->id;
		inserimento_lista(pronti, work);
	}

	schedulatore();

}
extern "C" c_rw_rel_read(nat rw)
{

	if(!rw_valido(rw)){
		flog(LOG_WARN, "Rw non valido \n");
		c_abort_p();
		return;
	}

	des_rw* p_des = &array_desrw[rw]; 

	natl pos = rw_find_reader(p_des, esecuzione->id);

	if(pos == natl(-1)){
		flog(LOG_WARN, "Posizione rw non valida \n");
		c_abort_p();
		return;
	}

	inspronti();

	// Rilascio il diritto di lettura
	// Devo dare priorita' agli scrittori
	// p_des->nreaders == 1 siamo gli unici lettori
	// p_des->w_writers ci sono scrittori
	if(p_des->nreaders == 1 || p_des->w_writers){
		// Non ci sono altri lettori
		p_des->nreaders--;
		des_proc *work = rimozione_lista(p_des->w_writers);
		p_des->readers[pos] = natl(-1);
		p_des->writer = work->id;
		inserimento_lista(pronti, work);
	}
	// Se ci sono lettori in attesa devo cedere il rw
	else if(p_des->w_readers){
		des_proc *work = rimozione_lista(p_des->w_readers);
		p_des->readers[pos] = work->id;
		inserimento_lista(pronti, work);
	}
	else{
		p_des->nreaders--;
		p_des->readers[pos] = natl(-1);
	}

	schedulatore();

}
// SOLUZIONE 2011-02-05 )


/////////////////////////////////////////////////////////////////////////////////
//                         TIMER                                               //
/////////////////////////////////////////////////////////////////////////////////

// richiesta al timer
struct richiesta {
	natl d_attesa;
	richiesta *p_rich;
	proc_elem *pp;
};

richiesta *p_sospesi;

void inserimento_lista_attesa(richiesta *p);
// parte "C++" della primitiva delay
extern "C" void c_delay(natl n)
{
	richiesta *p;

	p = static_cast<richiesta*>(alloca(sizeof(richiesta)));
	p->d_attesa = n;
	p->pp = esecuzione;

	inserimento_lista_attesa(p);
	schedulatore();
}

// inserisce P nella coda delle richieste al timer
void inserimento_lista_attesa(richiesta *p)
{
	richiesta *r, *precedente;
	bool ins;

	r = p_sospesi;
	precedente = 0;
	ins = false;

	while (r != 0 && !ins)
		if (p->d_attesa > r->d_attesa) {
			p->d_attesa -= r->d_attesa;
			precedente = r;
			r = r->p_rich;
		} else
			ins = true;

	p->p_rich = r;
	if (precedente != 0)
		precedente->p_rich = p;
	else
		p_sospesi = p;

	if (r != 0)
		r->d_attesa -= p->d_attesa;
}


// (* in caso di errori fatali, useremo la segunte funzione, che blocca il sistema:
extern "C" void panic(cstr msg) __attribute__ (( noreturn ));
// implementazione in [P_PANIC]
// *)

/////////////////////////////////////////////////////////////////////////////////
//                         ECCEZIONI                                           //
/////////////////////////////////////////////////////////////////////////////////

static const char *eccezioni[] = {
	"errore di divisione", 		// 0
	"debug",			// 1
	"interrupt non mascherabile",	// 2
	"breakpoint",			// 3
	"overflow",			// 4
	"bound check",			// 5
	"codice operativo non valido",	// 6
	"dispositivo non disponibile",	// 7
	"doppio fault",			// 8
	"coprocessor segment overrun",	// 9
	"TSS non valido",		// 10
	"segmento non presente",	// 11
	"errore sul segmento stack",	// 12
	"errore di protezione",		// 13
	"page fault",			// 14
	"riservato",			// 15
	"errore su virgola mobile",	// 16
	"errore di allineamento",	// 17
	"errore interno",		// 18
	"errore SIMD",			// 19
};
// gestore generico di eccezioni (chiamata da tutti i gestori di eccezioni in
// sistema.s, tranne il gestore di page fault e di non-maskable-interrupt)
void process_dump(natl id, log_sev sev);
extern "C" void gestore_eccezioni(int tipo, natq errore, addr rip)
{
	flog(LOG_WARN, "Eccezione %d (%s), errore %x", tipo, eccezioni[tipo], errore);
	process_dump(esecuzione->id, LOG_WARN);
	c_abort_p();
}
// (*il microprogramma di gestione delle eccezioni di page fault lascia in cima
//   alla pila (oltre ai valori consueti) una doppia parola, i cui 4 bit meno
//   significativi specificano piu' precisamente il motivo per cui si e'
//   verificato un page fault. Il significato dei bit e' il seguente:
//   - prot: se questo bit vale 1, il page fault si e' verificato per un errore
//   di protezione: il processore si trovava a livello utente e la pagina
//   era di livello sistema (bit US = 0 in una qualunque delle tabelle
//   dell'albero che porta al descrittore della pagina). Se prot = 0, la pagina
//   o una delle tabelle erano assenti (bit P = 0)
//   - write: l'accesso che ha causato il page fault era in scrittura (non
//   implica che la pagina non fosse scrivibile)
//   - user: l'accesso e' avvenuto mentre il processore si trovava a livello
//   utente (non implica che la pagina fosse invece di livello sistema)
//   - res: uno dei bit riservati nel descrittore di pagina o di tabella non
//   avevano il valore richiesto (il bit D deve essere 0 per i descrittori di
//   tabella, e il bit pgsz deve essere 0 per i descrittori di pagina)
struct pf_error {
	natq prot  : 1;
	natq write : 1;
	natq user  : 1;
	natq res   : 1;
	natq pad   : 60; // bit non significativi
};
// *)

bool c_routine_pf();
extern "C" vaddr readCR2();
extern "C" faddr readCR3();
extern "C" natq end;	// ultimo indirizzo del codice sistema (fornito dal collegatore)
bool in_pf = false;	//* true mentre stiamo gestendo un page fault
// (* c_pre_routine_pf() e' la routine che viene chiamata in caso di page
//    fault. Effettua dei controlli aggiuntivi prima di chiamare la
//    routine c_routine_pf() che provvede a caricare le tabelle e pagine
//    mancanti
// *)
extern "C" void c_pre_routine_pf(
		int tipo,		/* 14 */
		pf_error errore,	/* vedi sopra */
		addr rip		/* ind. dell'istruzione che ha causato il fault */
	)
{

	// (* se durante la gestione di un page fault si verifica un altro page fault
	//    c'e' un bug nel modulo sistema.
	if (in_pf) {
		panic("page fault ricorsivo: STOP");
	}
	// *)

	in_pf = true;	//* inizia la gestione del page fault
	// (* il sistema non e' progettato per gestire page fault causati
	//   dalle primitie di nucleo. Se cio' si e' verificato,
	//   si tratta di un bug
	if ((errore.user == 0 && rip < &end)|| errore.res == 1) {
		vaddr v = readCR2();
		flog(LOG_ERR, "PAGE FAULT a %p, rip=%lx", v, rip);
		if (v < DIM_PAGINA)
			flog(LOG_ERR, "Probabile puntatore NULL");
		flog(LOG_ERR, "dettagli: %s, %s, %s, %s",
			errore.prot  ? "protezione"	: "pag/tab assente",
			errore.write ? "scrittura"	: "lettura",
			errore.user  ? "da utente"	: "da sistema",
			errore.res   ? "bit riservato"	: "");
		panic("errore di sistema");
	}
	// *)

	// (* l'errore di protezione non puo' essere risolto: il processo ha
	//    tentato di accedere ad indirizzi proibiti (cioe', allo spazio
	//    sistema)
	if (errore.prot == 1) {
		flog(LOG_WARN, "errore di protezione: rip=%lx, ind=%lx, %s, %s", rip, readCR2(),
			errore.write ? "scrittura"	: "lettura",
			errore.user  ? "da utente"	: "da sistema");
		c_abort_p();
		goto out;
	}

	// (* proviamo a caricare la pagina/tabella. Il caricamento potrebbe
	// ancora fallire perche' abbiamo finito lo spazio nello swap o perche'
	// il processo ha tentato di accedere ad una zona non mappata.)
	if (!c_routine_pf()) {
		vaddr v = readCR2();
		flog(LOG_WARN, "PAGE FAULT a %p, rip=%lx non risolto", v, rip);
		c_abort_p();
	}
out:
	in_pf = false;	//* fine della gestione del page fault
}


/////////////////////////////////////////////////////////////////////////////////
//                         PAGINE FISICHE                                      //
/////////////////////////////////////////////////////////////////////////////////

// avremo un descrittore di pagina fisica per ogni pagina fisica della parte
// M2.  Lo scopo del descrittore e' di contenere alcune informazioni relative
// al contenuto della pagina fisica descritta. Tali informazioni servono
// principalmente a facilitare o rendere possibile il rimpiazzamento del
// contenuto stesso.
struct des_frame {
	int	livello;	// 0=pagina, -1=libera, >0=livello tabella
	bool	residente;	// pagina residente o meno
	// identificatore del processo a cui appartiene l'entita'
	// contenuta nel frame.
	natl	processo;
	natl	contatore;	// contatore per le statistiche
	// blocco da cui l'entita' contenuta nel frame era stata caricata
	natq	ind_massa;
	// per risparmiare un po' di spazio uniamo due campi che
	// non servono mai insieme:
	// - ind_virtuale serve solo se il frame e' occupato
	// - prossimo_libero serve solo se il frame e' libero
	union {
		// indirizzo virtuale che permette di risalire al
		// descrittore che punta all'entita' contenuta nel
		// frame. Per le pagine si tratta di un qualunque
		// indirizzo virtuale interno alla pagina. Per le
		// tabelle serve un qualunque indirizzo virtuale la
		// cui traduzione passa dalla tabella.
		vaddr	ind_virtuale;
		des_frame*	prossimo_libero;
	};
};

des_frame* vdf;		// vettore di descrittori di frame
			// (allocato in M1, si veda init_dpf())
faddr primo_frame_utile;	// indirizzo fisico del primo frame di M2
natq N_DF;			// numero di frame in M2
des_frame* frame_liberi;	// indice del descrittore della prima pagina libera

// dato un indirizzo di un frame restituisce un puntatore al
// corrispondente descrittore
des_frame* descrittore_frame(faddr indirizzo_frame)
{
	if (indirizzo_frame < primo_frame_utile)
		return 0;
	natq indice = (indirizzo_frame - primo_frame_utile) / DIM_PAGINA;
	if (indice >= N_DF)
		return 0;
	return &vdf[indice];
}

// dato un puntatore ad un descrittore di frameestituisce
// l'indirizzo del primo byte del frame corrispondente
faddr indirizzo_frame(des_frame* df)
{
	natq indice = df - &vdf[0];
	return primo_frame_utile + indice * DIM_PAGINA;
}

// restituisce il piu' piccolo numero maggiore o uguale ad a
// e multiplo di m
natq allinea(natq a, natq m)
{
	return (a + m - 1) & ~(m - 1);
}

// ( [P_MEM_PHYS]
// init_des_frame viene chiamata in fase di inizalizzazione.  Tutta la
// memoria non ancora occupata viene usata per i frame.  La funzione
// si preoccupa anche di allocare lo spazio per i descrittori di frame
// e di inizializzarli in modo che tutti frame risultino liberi
// &end e' l'indirizzo del primo byte non occupato dal modulo sistema
// (e' calcolato dal collegatore).
bool init_des_frame()
{
	faddr vdf_start;
	// L'array di decrittori di pagine fisiche comincia subito dopo
	// la fine del programma
	vdf_start = allinea(reinterpret_cast<natq>(&end), sizeof(natq));
	// N_DPF e' il numero di pagine fisiche di cui sara' composta M2.
	// Per calcolarlo dobbiamo tenere conto che ci servira' un des_frame
	// per ogni pagina fisica.
	N_DF = (MEM_TOT - vdf_start) / (DIM_PAGINA + sizeof(des_frame));
	// M1 finisce dopo la fine dell'array dpf;
	natq fine_M1 = vdf_start + N_DF * sizeof(des_frame);
	// primo_frame_utile e' il primo frame che inizia dopo la fine di M1
	primo_frame_utile = allinea(fine_M1, DIM_PAGINA);

	// creiamo la lista dei frame liberi, che inizialmente contiene
	// tutti i frame di M2
	vdf = reinterpret_cast<des_frame*>(vdf_start);
	frame_liberi = &vdf[0];
	for (natl i = 0; i < N_DF - 1; i++) {
		vdf[i].livello = -1;
		vdf[i].prossimo_libero = &vdf[i + 1];
	}
	vdf[N_DF - 1].livello = -1;
	vdf[N_DF - 1].prossimo_libero = 0;

	return true;
}

// estrea una pagina libera dalla lista, se non vuota
des_frame* alloca_frame_libero()
{
	des_frame* p = frame_liberi;
	if (frame_liberi != 0)
		frame_liberi = frame_liberi->prossimo_libero;
	return p;
}

// (* rende di nuovo libera la pagina fisica descritta da df
void rilascia_frame(des_frame* df)
{
	df->livello = -1;
	df->prossimo_libero = frame_liberi;
	frame_liberi = df;
}
// *)

des_frame* scegli_vittima(natl proc, int liv, vaddr ind_virtuale); // piu' avanti
des_frame* alloca_frame(natl proc, int livello, vaddr ind_virt); // piu' avanti
// )


/////////////////////////////////////////////////////////////////////////////////
//                         PAGINAZIONE                                         //
/////////////////////////////////////////////////////////////////////////////////

static const natq BIT_SEGNO = (1UL << 47);
static const natq MASCHERA_MODULO = BIT_SEGNO - 1;
// restituisce la versione normalizzata (16 bit piu' significativi uguali al
// bit 47) dell'indirizzo a
static inline vaddr norm(vaddr a)
{
	return (a & BIT_SEGNO) ? (a | ~MASCHERA_MODULO) : (a & MASCHERA_MODULO);
}

// restituisce la dimensione di una pagina di livello liv
static inline natq dim_pag(int liv)
{
	natq v = 1UL << ((liv - 1) * 9 + 12);
	return v;
}

// dato un indirizzo 'a', restituisce l'indirizzo del primo byte della
// pagina di livello 'liv' a cui 'a' appartiene
static inline vaddr base(vaddr v, int liv)
{
	natq mask = dim_pag(liv + 1) - 1;
	return v & ~mask;
}

// copia 'n' descrittori a partire da quello di indice 'i' dalla
// tabella di indirizzo 'src' in quella di indirizzo 'dst'
void copy_des(faddr src, faddr dst, natl i, natl n)
{
	natq *pdsrc = reinterpret_cast<natq*>(src),
	     *pddst = reinterpret_cast<natq*>(dst);
	for (natl j = i; j < i + n && j < 512; j++)
		pddst[j] = pdsrc[j];
}

// indirizzo virtuale di partenza delle varie zone della memoria
// virtuale dei proceii

const vaddr ini_sis_c = norm(I_SIS_C * dim_pag(4)); // sistema condivisa
const vaddr ini_sis_p = norm(I_SIS_P * dim_pag(4)); // sistema privata
const vaddr ini_mio_c = norm(I_MIO_C * dim_pag(4)); // modulo IO
const vaddr ini_utn_c = norm(I_UTN_C * dim_pag(4)); // utente condivisa
const vaddr ini_utn_p = norm(I_UTN_P * dim_pag(4)); // utente privata

// indirizzo del primo byte che non appartiene alla zona specificata
const vaddr fin_sis_c = ini_sis_c + dim_pag(4) * N_SIS_C;
const vaddr fin_sis_p = ini_sis_p + dim_pag(4) * N_SIS_P;
const vaddr fin_mio_c = ini_mio_c + dim_pag(4) * N_MIO_C;
const vaddr fin_utn_c = ini_utn_c + dim_pag(4) * N_UTN_C;
const vaddr fin_utn_p = ini_utn_p + dim_pag(4) * N_UTN_P;

//   ( definiamo alcune costanti utili per la manipolazione dei descrittori
//     di pagina e di tabella. Assegneremo a tali descrittori il tipo "natl"
//     e li manipoleremo tramite maschere e operazioni sui bit.
const natq BIT_P    = 1U << 0; // il bit di presenza
const natq BIT_RW   = 1U << 1; // il bit di lettura/scrittura
const natq BIT_US   = 1U << 2; // il bit utente/sistema(*)
const natq BIT_PWT  = 1U << 3; // il bit Page Wright Through
const natq BIT_PCD  = 1U << 4; // il bit Page Cache Disable
const natq BIT_A    = 1U << 5; // il bit di accesso
const natq BIT_D    = 1U << 6; // il bit "dirty"
const natq BIT_PS   = 1U << 7; // il bit "page size"
const natq BIT_ZERO = 1U << 9; // (* nuova pagina, da azzerare *)

const natq ACCB_MASK  = 0x00000000000000FF; // maschera per il byte di accesso
const natq ADDR_MASK  = 0x7FFFFFFFFFFFF000; // maschera per l'indirizzo
const natq INDMASS_MASK = 0x7FFFFFFFFFFFF000; // maschera per l'indirizzo in mem. di massa
const natq INDMASS_SHIFT = 12;	    // primo bit che contiene l'ind. in mem. di massa

// )

bool  extr_P(tab_entry descrittore)
{ // (
	return (descrittore & BIT_P); // )
}
bool extr_D(tab_entry descrittore)
{ // (
	return (descrittore & BIT_D); // )
}
bool extr_A(tab_entry descrittore)
{ // (
	return (descrittore & BIT_A); // )
}
bool extr_ZERO(tab_entry descrittore)
{ // (
	return (descrittore & BIT_ZERO); // )
}
faddr extr_IND_FISICO(tab_entry descrittore)
{ // (
	return descrittore & ADDR_MASK; // )
}
natq extr_IND_MASSA(tab_entry descrittore)
{ // (
	return (descrittore & INDMASS_MASK) >> INDMASS_SHIFT; // )
}
void set_P(tab_entry& descrittore, bool bitP)
{ // (
	if (bitP)
		descrittore |= BIT_P;
	else
		descrittore &= ~BIT_P; // )
}
void set_A(tab_entry& descrittore, bool bitA)
{ // (
	if (bitA)
		descrittore |= BIT_A;
	else
		descrittore &= ~BIT_A; // )
}
void set_ZERO(tab_entry& descrittore, bool bitZERO)
{
	if (bitZERO)
		descrittore |= BIT_ZERO;
	else
		descrittore &= ~BIT_ZERO;
}
// (* definiamo anche la seguente funzione:
//    clear_IND_M: azzera il campo M (indirizzo in memoria di massa)
void clear_IND_MASSA(tab_entry& descrittore)
{
	descrittore &= ~INDMASS_MASK;
}
// *)
void  set_IND_FISICO(tab_entry& descrittore, faddr ind_fisico) //
{ // (
	clear_IND_MASSA(descrittore);
	descrittore |= ind_fisico & ADDR_MASK; // )
}
void set_IND_MASSA(tab_entry& descrittore, natq ind_massa) //
{ // (
	clear_IND_MASSA(descrittore);
	descrittore |= (ind_massa << INDMASS_SHIFT); // )
}

void set_D(tab_entry& descrittore, bool bitD) //
{ // (
	if (bitD)
		descrittore |= BIT_D;
	else
		descrittore &= ~BIT_D; // )
}

bool  extr_PS(tab_entry descrittore)
{ // (
	return (descrittore & BIT_PS); // )
}

// dato un indirizzo virtuale 'ind_virt' ne restituisce
// l'indice del descrittore corrispondente nella tabella di livello 'liv'
int i_tab(vaddr ind_virt, int liv)
{
	int shift = 12 + (liv - 1) * 9;
	natq mask = 0x1ffUL << shift;
	return (ind_virt & mask) >> shift;
}
// dato l'indirizzo di una tabella e un indice, restituisce un
// riferimento alla corrispondente entrata
tab_entry& get_entry(faddr tab, natl index)
{
	natq *pd = reinterpret_cast<natq*>(tab);
	return  pd[index];
}

// dato un identificatore di processo, un livello e
// un indirizzo virtuale 'ind_virt', restituisce un riferimento
// all'entrata di quel livello relativo alla pagina che
// contiene 'ind_virt' (tutte le tabelle di livello
// precedente devono essere gia' presenti)
tab_entry& get_des(natl processo, int livello, vaddr ind_virt)
{
	des_proc *p = des_p(processo);
	if (!p) {
		flog(LOG_ERR, "get_des(%d): processo non trovato", processo);
		panic("errore interno");
	}
	faddr tab = p->cr3;
	for (int i = 4; i > livello; i--) {
		tab_entry e = get_entry(tab, i_tab(ind_virt, i));
		if (!extr_P(e))
			panic("P=0 non ammesso");
		tab = extr_IND_FISICO(e);
	}
	return get_entry(tab, i_tab(ind_virt, livello));
}


// ( [P_MEM_VIRT]

// carica un nuovo valore in cr3 [vedi sistema.S]
extern "C" void loadCR3(faddr dir);

// restituisce il valore corrente di cr3 [vedi sistema.S]
extern "C" faddr readCR3();

//invalida il TLB
extern "C" void invalida_TLB();

// mappa la memoria fisica in memoria virtuale, inclusa l'area PCI
// (copiamo la finestra gia' creata dal boot loader)
bool crea_finestra_FM(faddr tab4)
{
	faddr boot_dir = readCR3();
	copy_des(boot_dir, tab4, I_SIS_C, N_SIS_C);
	return true;
}

// )
const natl MAX_IRQ  = 24;
proc_elem *a_p[MAX_IRQ];  //
// )

natq alloca_blocco();
des_frame* swap(natl proc, int livello, vaddr ind_virt);
bool crea(natl proc, vaddr ind_virt, int liv, natl priv)
{
	tab_entry& dt = get_des(proc, liv + 1, ind_virt);
	bool bitP = extr_P(dt);
	if (!bitP) {
		natl blocco = extr_IND_MASSA(dt);
		if (!blocco) {
			if (! (blocco = alloca_blocco()) ) {
				flog(LOG_ERR, "swap pieno");
				return false;
			}
			set_IND_MASSA(dt, blocco);
			set_ZERO(dt, true);
			dt = dt | BIT_RW;
			if (priv == LIV_UTENTE) dt = dt | BIT_US;
		}
		if (liv > 0) {
			des_frame *df = swap(proc, liv, ind_virt);
			if (!df) {
				flog(LOG_ERR, "swap(%d, %d, %p) fallita",
					proc, liv, ind_virt);
				return false;
			}
			df->residente = (priv == LIV_SISTEMA);
		}
	}
	return true;
}

bool crea_pagina(natl proc, vaddr ind_virt, natl priv)
{
	for (int i = 3; i >= 0; i--) {
		if (!crea(proc, ind_virt, i, priv))
			return false;
	}
	return true;
}

bool crea_pila(natl proc, vaddr bottom, natq size, natl priv)
{
	size = allinea(size, DIM_PAGINA);

	for (vaddr ind = bottom - size; ind != bottom; ind += DIM_PAGINA)
		if (!crea_pagina(proc, ind, priv))
			return false;
	return true;
}

faddr carica_pila_sistema(natl proc, vaddr bottom, natq size)
{
	des_frame *dp = 0;
	for (vaddr ind = bottom - size; ind != bottom; ind += DIM_PAGINA) {
		dp = swap(proc, 0, ind);
		if (!dp)
			return 0;
		dp->residente = true;
	}
	return indirizzo_frame(dp) + DIM_PAGINA;
}


faddr crea_tab4()
{
	des_frame* df = alloca_frame_libero();
	if (df == 0) {
		flog(LOG_ERR, "Impossibile allocare tab4");
		panic("errore");
	}
	df->livello = 4;
	df->residente = true;
	faddr tab4 = indirizzo_frame(df);
	memset(reinterpret_cast<void *>(tab4), 0, DIM_PAGINA);

	return tab4;
}


extern "C" natl alloca_tss(des_proc*);
extern "C" void rilascia_tss(int indice);
const natl BIT_IF = 1L << 9;
extern "C" natl tss_to_id(natl tss_off);
extern "C" natl id_to_tss(natl id);

void crea_tab4(faddr dest)
{
	faddr pdir = readCR3();

	memset(reinterpret_cast<void*>(dest), 0, DIM_PAGINA);

	copy_des(pdir, dest, I_SIS_C, N_SIS_C);
	copy_des(pdir, dest, I_MIO_C, N_MIO_C);
	copy_des(pdir, dest, I_UTN_C, N_UTN_C);
}

void rilascia_tutto(faddr tab4, natl i, natl n);
proc_elem* crea_processo(void f(int), int a, int prio, char liv, bool IF)
{
	proc_elem	*p;			// proc_elem per il nuovo processo
	natl		tss_off;		// offset del descrittore tss nella gdt
	natl 		identifier;		// identificatore del processo
	des_proc	*pdes_proc;		// descrittore di processo
	des_frame*	dpf_tab4;		// tab4 del processo
	faddr		pila_sistema;

	// ( allocazione (e azzeramento preventivo) di un des_proc
	//   (parte del punto 3 in)
	pdes_proc = static_cast<des_proc*>(alloca(sizeof(des_proc)));
	if (pdes_proc == 0) goto errore1;
	memset(pdes_proc, 0, sizeof(des_proc));
	// )

	// ( selezione di un identificatore (punto 1 in)
	tss_off = alloca_tss(pdes_proc);
	if (tss_off == 0) goto errore2;
	identifier = tss_to_id(tss_off);
	// )

	// ( allocazione e inizializzazione di un proc_elem
	//   (punto 3 in)
	p = static_cast<proc_elem*>(alloca(sizeof(proc_elem)));
        if (p == 0) goto errore3;
        p->id = identifier;
        p->precedenza = prio;
	p->puntatore = 0;
	// )

	// ( creazione della tab4 del processo (vedi
	dpf_tab4 = alloca_frame(p->id, 4, 0);
	if (dpf_tab4 == 0) goto errore4;
	dpf_tab4->livello = 4;
	dpf_tab4->residente = true;
	dpf_tab4->processo = identifier;
	pdes_proc->cr3 = indirizzo_frame(dpf_tab4);
	crea_tab4(pdes_proc->cr3);
	// )

	// ( creazione della pila sistema .
	if (!crea_pila(p->id, fin_sis_p, DIM_SYS_STACK, LIV_SISTEMA))
		goto errore5;
	pila_sistema = carica_pila_sistema(p->id, fin_sis_p, DIM_SYS_STACK);
	if (pila_sistema == 0)
		goto errore6;
	// )

	if (liv == LIV_UTENTE) {
		// ( inizializziamo la pila sistema.
		natq* pl = reinterpret_cast<natq*>(pila_sistema);

		pl[-5] = reinterpret_cast<natq>(f); // RIP (codice utente)
		pl[-4] = SEL_CODICE_UTENTE;	    // CS (codice utente)
		pl[-3] = IF ? BIT_IF : 0;	    // RFLAGS
		pl[-2] = fin_utn_p - sizeof(natq);  // RSP
		pl[-1] = SEL_DATI_UTENTE;	    // SS (pila utente)
		//   eseguendo una IRET da questa situazione, il processo
		//   passera' ad eseguire la prima istruzione della funzione f,
		//   usando come pila la pila utente (al suo indirizzo virtuale)
		// )

		// ( creazione della pila utente
		if (!crea_pila(p->id, fin_utn_p, DIM_USR_STACK, LIV_UTENTE)) {
			flog(LOG_WARN, "creazione pila utente fallita");
			goto errore6;
		}
		// )

		// ( infine, inizializziamo il descrittore di processo
		//   indirizzo del bottom della pila sistema, che verra' usato
		//   dal meccanismo delle interruzioni
		pdes_proc->punt_nucleo = fin_sis_p;

		//   inizialmente, il processo si trova a livello sistema, come
		//   se avesse eseguito una istruzione INT, con la pila sistema
		//   che contiene le 5 parole lunghe preparate precedentemente
		pdes_proc->contesto[I_RSP] = fin_sis_p - 5 * sizeof(natq);

		//   il registro RDI deve contenere il parametro da passare
		//   alla funzione f
		pdes_proc->contesto[I_RDI] = a;
		//pdes_proc->contesto[I_FPU_CR] = 0x037f;
		//pdes_proc->contesto[I_FPU_TR] = 0xffff;
		pdes_proc->cpl = LIV_UTENTE;
	
		//   il campo iomap_base contiene l'offset (nel TSS) dell'inizio 
		//   della "I/O bitmap". Questa bitmap contiene un bit per ogni
		//   possibile indirizzo di I/O. Le istruzioni in e out eseguite
		//   da livello utente verranno permesse se il bit corrispondente
		//   all'indirizzo di I/O a cui si riferiscono vale 1.
		//   Per disattivare questo meccanismo dobbiamo inizializzare
		//   il campo iomap_base con un offset maggiore o uguale
		//   della dimensione del segmento TSS (come scritta nel
		//   descrittore di segmento TSS nella GDT, vedere 'set_entry_tss'
		//   in sistema.S)
		pdes_proc->iomap_base = DIM_DESP;

		//   tutti gli altri campi valgono 0
		// )
	} else {
		// ( inizializzazione della pila sistema
		natq* pl = reinterpret_cast<natq*>(pila_sistema);
		pl[-6] = reinterpret_cast<natq>(f);  	// RIP (codice sistema)
		pl[-5] = SEL_CODICE_SISTEMA;            // CS (codice sistema)
		pl[-4] = IF ? BIT_IF : 0;  	        // RFLAGS
		pl[-3] = fin_sis_p - sizeof(natq);	// RSP
		pl[-2] = 0;			        // SS
		pl[-1] = 0;			        // ind. rit.
							//(non significativo)
		//   i processi esterni lavorano esclusivamente a livello
		//   sistema. Per questo motivo, prepariamo una sola pila (la
		//   pila sistema)
		// )

		// ( inizializziamo il descrittore di processo
		pdes_proc->contesto[I_RSP] = fin_sis_p - 6 * sizeof(natq);
		pdes_proc->contesto[I_RDI] = a;

		//pdes_proc->contesto[I_FPU_CR] = 0x037f;
		//pdes_proc->contesto[I_FPU_TR] = 0xffff;
		pdes_proc->cpl = LIV_SISTEMA;

		//   tutti gli altri campi valgono 0
		// )
	}

	return p;

errore6:	rilascia_tutto(indirizzo_frame(dpf_tab4), I_SIS_P, N_SIS_P);
errore5:	rilascia_frame(dpf_tab4);
errore4:	dealloca(p);
errore3:	rilascia_tss(tss_off);
errore2:	dealloca(pdes_proc);
errore1:	return 0;
}

// parte "C++" della activate_p, descritta in
extern "C" void
c_activate_p(void f(int), int a, natl prio, natl liv)
{
	proc_elem *p;			// proc_elem per il nuovo processo
	natl id = 0xFFFFFFFF;		// id da restituire in caso di fallimento

	// (* non possiamo accettare una priorita' minore di quella di dummy
	//    o maggiore di quella del processo chiamante
	if (prio < MIN_PRIORITY || prio > esecuzione->precedenza) {
		flog(LOG_WARN, "priorita' non valida: %d", prio);
		c_abort_p();
		return;
	}
	// *)

	// (* controlliamo che 'liv' contenga un valore ammesso
	//    [segnalazione di E. D'Urso]
	if (liv != LIV_UTENTE && liv != LIV_SISTEMA) {
		flog(LOG_WARN, "livello non valido: %d", liv);
		c_abort_p();
		return;
	}
	// *)

	if (liv == LIV_SISTEMA && des_p(esecuzione->id)->cpl == LIV_UTENTE) {
		flog(LOG_WARN, "errore di protezione");
		c_abort_p();
		return;
	}

	// (* accorpiamo le parti comuni tra c_activate_p e c_activate_pe
	// nella funzione ausiliare crea_processo
	// (questa svolge, tra l'altro, i punti 1-3 in)
	p = crea_processo(f, a, prio, liv, (liv == LIV_UTENTE));
	// *)

	if (p != 0) {
		inserimento_lista(pronti, p);
		processi++;
		id = p->id;			// id del processo creato
						// (allocato da crea_processo)
		flog(LOG_INFO, "proc=%d entry=%p(%d) prio=%d liv=%d", id, f, a, prio, liv);
	}

	des_proc *self = des_p(esecuzione->id);
	self->contesto[I_RAX] = id;
}

void rilascia_tutto(addr tab4, natl i, natl n);
void riassegna_tutto(natl proc, faddr tab4, natl i, natl n);
void dealloca_blocco(natl blocco);
// rilascia tutte le strutture dati private associate al processo puntato da
// "p" (tranne il proc_elem puntato da "p" stesso)
faddr ultimo_terminato;
extern "C" void distruggi_pila_precedente() {
	rilascia_tutto(ultimo_terminato, I_SIS_P, N_SIS_P);
	rilascia_frame(descrittore_frame(ultimo_terminato));
	ultimo_terminato = 0;
}
void distruggi_processo(proc_elem* p)
{
	des_proc* pdes_proc = des_p(p->id);

	faddr tab4 = pdes_proc->cr3;
	riassegna_tutto(p->id, tab4, I_MIO_C, N_MIO_C);
	riassegna_tutto(p->id, tab4, I_UTN_C, N_UTN_C);
	rilascia_tutto(tab4, I_UTN_P, N_UTN_P);
	ultimo_terminato = tab4;
	if (p != esecuzione) {
		distruggi_pila_precedente();
	}
	rilascia_tss(id_to_tss(p->id));
	dealloca(pdes_proc);
}

// rilascia ntab tabelle (con tutte le pagine da esse puntate) a partire da
// quella puntata dal descrittore i-esimo di tab4.
void rilascia_ric(faddr tab, int liv, natl i, natl n)
{
	for (natl j = i; j < i + n && j < 512; j++) {
		tab_entry& dt = get_entry(tab, j);
		natl blocco;
		if (extr_P(dt)) {
			faddr sub = extr_IND_FISICO(dt);
			if (liv > 1)
				rilascia_ric(sub, liv - 1, 0, 512);
			des_frame *df = descrittore_frame(sub);
			blocco = df->ind_massa;
			rilascia_frame(df);
		} else {
			blocco = extr_IND_MASSA(dt);
		}
		dealloca_blocco(blocco);
		dt = 0;
	}
}

void rilascia_tutto(faddr tab4, natl i, natl n)
{
	rilascia_ric(tab4, 4, i, n);
}

void riassegna_ric(natl proc, faddr tab, int liv, natl i, natl n)
{
	for (natl j = i; j < i + n && j < 512; j++) {
		tab_entry& dt = get_entry(tab, j);
		if (extr_P(dt)) {
			faddr sub = extr_IND_FISICO(dt);
			if (liv > 1)
				riassegna_ric(proc, sub, liv - 1, 0, 512);
			des_frame *df = descrittore_frame(sub);
			if (df->processo == proc)
				df->processo = dummy_proc;
		}
	}
}

void riassegna_tutto(natl proc, faddr tab4, natl i, natl n)
{
	riassegna_ric(proc, tab4, 4, i, n);
}

void term_cur_proc(log_sev sev, const char *mode)
{
	proc_elem *p = esecuzione;
	distruggi_processo(p);
	processi--;
	flog(sev, "Processo %d %s", p->id, mode);
	dealloca(p);
	schedulatore();
}

// parte "C++" della terminate_p
extern "C" void c_terminate_p()
{
	term_cur_proc(LOG_INFO, "terminato");
}

// come la terminate_p, ma invia anche un warning al log (da invocare quando si
// vuole terminare un processo segnalando che c'e' stato un errore)
extern "C" void c_abort_p()
{
	term_cur_proc(LOG_WARN, "abortito");
}
// )

// driver del timer
extern "C" void c_driver_td(void)
{
	richiesta *p;

	inspronti();

	if (p_sospesi != 0) {
		p_sospesi->d_attesa--;
	}

	while (p_sospesi != 0 && p_sospesi->d_attesa == 0) {
		inserimento_lista(pronti, p_sospesi->pp);
		p = p_sospesi;
		p_sospesi = p_sospesi->p_rich;
		dealloca(p);
	}

	schedulatore();
}

void scrivi_swap(addr src, natl blocco);
void leggi_swap(addr dest, natl blocco);


void carica(des_frame* df) //
{
	tab_entry& e = get_des(df->processo, df->livello + 1, df->ind_virtuale);
	if (extr_ZERO(e)) {
		memset((addr)indirizzo_frame(df), 0, DIM_PAGINA);
	} else {
		leggi_swap((addr)indirizzo_frame(df), df->ind_massa);
	}
}

void scarica(des_frame* df) //
{
	scrivi_swap((addr)indirizzo_frame(df), df->ind_massa);
	tab_entry& e = get_des(df->processo, df->livello + 1, df->ind_virtuale);
	set_D(e, false);
}

void collega(des_frame *df)	//
{
	tab_entry& e = get_des(df->processo, df->livello + 1, df->ind_virtuale);
	set_IND_FISICO(e, indirizzo_frame(df));
	set_P(e, true);
	set_D(e, false);
	set_A(e, false);
}

extern "C" void invalida_entrata_TLB(vaddr ind_virtuale); //
bool scollega(des_frame* df)	//
{
	bool bitD;
	tab_entry& e = get_des(df->processo, df->livello + 1, df->ind_virtuale);
	bitD = extr_D(e);
	bool occorre_salvare = bitD || df->livello > 0;
	set_IND_MASSA(e, df->ind_massa);
	set_P(e, false);
	if (occorre_salvare)
		set_ZERO(e, false);
	if (df->processo == esecuzione->id)
		invalida_entrata_TLB(df->ind_virtuale);
	return occorre_salvare;	//
}

// alloca una pagina fisica destinata a contenere l'entita' del
// livello specificato, relativa all'indirizzo virtuale ind_virt
// nello spazio di indirizzamento di proc
des_frame* alloca_frame(natl proc, int livello, vaddr ind_virt)
{
	des_frame *df = alloca_frame_libero();
	if (df == 0) {
		df = scegli_vittima(proc, livello, ind_virt);
		if (df == 0)
			return 0;
		bool occorre_salvare = scollega(df);
		if (occorre_salvare)
			scarica(df);
	}
	return df;
}

// carica l'entita' del livello specificato, relativa all'indirizzo virtuale
// ind_virt nello spazio di indirizzamento di proc
des_frame* swap(natl proc, int livello, vaddr ind_virt)
{
	tab_entry& e = get_des(proc, livello + 1, ind_virt);
	natq m = extr_IND_MASSA(e);
	if (!m) {
		flog(LOG_WARN,
		     "indirizzo %p fuori dallo spazio virtuale allocato",
		     ind_virt);
		return 0;
	}
	des_frame* df = alloca_frame(proc, livello, ind_virt);
	if (!df) {
		flog(LOG_WARN, "memoria esaurita");
		return 0;
	}
	df->livello = livello;
	df->residente = 0;
	df->processo = proc;
	df->ind_virtuale = ind_virt;
	df->ind_massa = m;
	df->contatore = 0;
	carica(df);
	collega(df);
	return df;
}

void stat();
bool c_routine_pf()
{
	vaddr ind_virt = readCR2();
	natl proc = esecuzione->id;

	stat();

	for (int i = 3; i >= 0; i--) {
		tab_entry d = get_des(proc, i + 1, ind_virt);
		bool bitP = extr_P(d);
		if (!bitP) {
			des_frame *df = swap(proc, i, ind_virt);
			if (!df)
				return false;
		}
	}
	return true;
}

bool vietato(des_frame* df, natl proc, int liv, vaddr ind_virt)
{
	if (df->livello > liv && df->processo == proc &&
	    base(df->ind_virtuale, df->livello) == base(ind_virt, df->livello))
		return true;
	return false;
}

des_frame* scegli_vittima(natl proc, int liv, vaddr ind_virt)
{
	des_frame *df, *df_vittima;
	df = &vdf[0];
	while ( df < &vdf[N_DF] &&
		(df->residente ||
		 vietato(df, proc, liv, ind_virt)))
		df++;
	if (df == &vdf[N_DF]) return 0;
	df_vittima = df;
	for (df++; df < &vdf[N_DF]; df++) {
		if (df->residente ||
		    vietato(df, proc, liv, ind_virt))
			continue;
		if (df->contatore < df_vittima->contatore ||
		    (df->contatore == df_vittima->contatore &&
		     df_vittima->livello > df->livello))
			df_vittima = df;
	}
	return df_vittima;
}

void stat()
{
	des_frame *df1, *df2;
	faddr f1, f2;
	bool bitA;

	for (natq i = 0; i < N_DF; i++) {
		df1 = &vdf[i];
		if (df1->livello < 1)
			continue;
		f1 = indirizzo_frame(df1);
		for (int j = 0; j < 512; j++) {
			tab_entry& des = get_entry(f1, j);
			if (!extr_P(des))
				continue;
			bitA = extr_A(des);
			set_A(des, false);
			f2 = extr_IND_FISICO(des);
			df2 = descrittore_frame(f2);
			if (!df2 || df2->residente)
				continue;
			df2->contatore >>= 1;
			if (bitA)
				df2->contatore |= 0x80000000;
		}
	}
	invalida_TLB();
}


// funzione di supporto per carica_tutto()
bool carica_ric(natl proc, faddr tab, int liv, vaddr ind, natl n)
{
	natq dp = dim_pag(liv + 1);

	natl i = i_tab(ind, liv + 1);
	for (natl j = i; j < i + n; j++, ind += dp) {
		tab_entry e = get_entry(tab, j);
		if (!extr_IND_MASSA(e))
			continue;
		des_frame *df = swap(proc, liv, ind);
		if (!df) {
			flog(LOG_ERR, "impossibile caricare pagina virtuale %p", ind);
			return false;
		}
		df->residente = true;
		if (liv > PRELOAD_LEVEL &&
				!carica_ric(proc, indirizzo_frame(df), liv - 1, ind, 512))
			return false;
	}
	return true;
}

// carica e rende residenti tutte le pagine e tabelle allocate nello swap e
// relative alle entrate della tab4 del processo proc che vanno da i (inclusa)
// a i+n (esclusa)
bool carica_tutto(natl proc, natl i, natl n)
{
	des_proc *p = des_p(proc);

	return carica_ric(proc, p->cr3, 3, norm(i * dim_pag(4)), n);
}



// super blocco (vedi e [P_SWAP] avanti)
struct superblock_t {
	char	magic[8];
	natq	bm_start;
	natq	blocks;
	natq	directory;
	void	(*user_entry)(int);
	natq	user_end;
	void	(*io_entry)(int);
	natq	io_end;
	int	checksum;
};

// descrittore di swap (vedi [P_SWAP] avanti)
struct des_swap {
	natl *free;		// bitmap dei blocchi liberi
	superblock_t sb;	// contenuto del superblocco
} swap_dev; 	// c'e' un unico oggetto swap
bool swap_init();

// chiamata in fase di inizializzazione, carica in memoria fisica
// tutte le parti condivise di livello IO e utente.
bool crea_spazio_condiviso()
{

	// ( lettura del direttorio principale dallo swap
	flog(LOG_INFO, "lettura del direttorio principale...");
	addr tmp = alloca(DIM_PAGINA);
	if (tmp == 0) {
		flog(LOG_ERR, "memoria insufficiente");
		return false;
	}
	leggi_swap(tmp, swap_dev.sb.directory);
	// )

	// (  carichiamo le parti condivise nello spazio di indirizzamento del processo
	//    dummy
	faddr dummy_dir = des_p(dummy_proc)->cr3;
	copy_des((faddr)tmp, dummy_dir, I_MIO_C, N_MIO_C);
	copy_des((faddr)tmp, dummy_dir, I_UTN_C, N_UTN_C);
	dealloca(tmp);

	if (!carica_tutto(dummy_proc, I_MIO_C, N_MIO_C))
		return false;
	if (!carica_tutto(dummy_proc, I_UTN_C, N_UTN_C))
		return false;
	// )

	// ( copiamo i descrittori relativi allo spazio condiviso anche nel direttorio
	//   corrente, in modo che vengano ereditati dai processi che creeremo in seguito
	faddr my_dir = des_p(esecuzione->id)->cr3;
	copy_des(dummy_dir, my_dir, I_MIO_C, N_MIO_C);
	copy_des(dummy_dir, my_dir, I_UTN_C, N_UTN_C);
	// )

	invalida_TLB();
	return true;
}

///////////////////////////////////////////////////////////////////////////////////
//                   INIZIALIZZAZIONE                                            //
///////////////////////////////////////////////////////////////////////////////////

const natq HEAP_START = 1*MiB;
extern "C" natq start;
const natq HEAP_SIZE = (natq)&start - HEAP_START;


proc_elem init;

// creazione del processo dummy iniziale (usata in fase di inizializzazione del sistema)
extern "C" void end_program();	//
// corpo del processo dummy	//
void dd(int i)
{
	while (processi != 1)
		;
	end_program();
}

natl crea_dummy()
{
	proc_elem* di = crea_processo(dd, 0, DUMMY_PRIORITY, LIV_SISTEMA, true);
	if (di == 0) {
		flog(LOG_ERR, "Impossibile creare il processo dummy");
		return 0xFFFFFFFF;
	}
	inserimento_lista(pronti, di);
	processi++;
	return di->id;
}
void main_sistema(int n);
natl crea_main_sistema()
{
	proc_elem* m = crea_processo(main_sistema, 0, MAX_PRIORITY, LIV_SISTEMA, false);
	if (m == 0) {
		flog(LOG_ERR, "Impossibile creare il processo main_sistema");
		return 0xFFFFFFFF;
	}
	inserimento_lista(pronti, m);
	processi++;
	return m->id;
}

// ( [P_EXTERN_PROC]
// Registrazione processi esterni
proc_elem* const ESTERN_BUSY = (proc_elem*)1;
// primitiva di nucleo usata dal nucleo stesso
extern "C" void wfi();

// associa il processo esterno puntato da "p" all'interrupt "irq".
// Fallisce se un processo esterno era gia' stato associato a
// quello stesso interrupt
bool aggiungi_pe(proc_elem *p, natb irq)
{
	if (irq >= MAX_IRQ) {
		flog(LOG_WARN, "irq %d non valido (max %d)", irq, MAX_IRQ);
		return false;
	}
	if (a_p[irq]) {
		flog(LOG_WARN, "irq %d occupato", irq);
		return false;
	}

	a_p[irq] = p;
	apic_set_MIRQ(irq, false);
	return true;
}

extern "C" void c_activate_pe(void f(int), int a, natl prio, natl liv, natb type)
{
	proc_elem	*p;			// proc_elem per il nuovo processo
	des_proc *self = des_p(esecuzione->id);

	if (prio < MIN_PRIORITY) {
		flog(LOG_WARN, "priorita' non valida: %d", prio);
		c_abort_p();
		return;
	}

	p = crea_processo(f, a, prio, liv, true);
	if (p == 0)
		goto error1;

	if (!aggiungi_pe(p, type) )
		goto error2;

	flog(LOG_INFO, "estern=%d entry=%p(%d) prio=%d liv=%d type=%d",
			p->id, f, a, prio, liv, type);

	self->contesto[I_RAX] = p->id;
	return;

error2:	distruggi_processo(p);
error1:
	self->contesto[I_RAX] = 0xFFFFFFFF;
	return;
}

bool is_accessible(vaddr a)
{
	for (int i = 4; i > 0; i--) {
		natq d = get_des(esecuzione->id, i, a);
		bool bitP = extr_P(d);
		if (!bitP)
			return false;
	}
	return true;
}

// indirizzo del primo byte che non contiene codice di sistema (vedi "sistema.s")
extern "C" addr fine_codice_sistema;
void process_dump(natl id, log_sev sev)
{
	des_proc *p = des_p(id);
	natq *pila = (natq*)p->contesto[I_RSP];

	flog(sev, "  RIP=%lx CPL=%s", pila[0], pila[1] == SEL_CODICE_UTENTE ? "LIV_UTENTE" : "LIV_SISTEMA");
	natq rflags = pila[2];
        flog(sev, "  RFLAGS=%lx [%s %s %s %s %s %s %s %s %s %s, IOPL=%s]",
		rflags,
		(rflags & 1U << 14) ? "NT" : "nt",
		(rflags & 1U << 11) ? "OF" : "of",
		(rflags & 1U << 10) ? "DF" : "df",
		(rflags & 1U << 9)  ? "IF" : "if",
		(rflags & 1U << 8)  ? "TF" : "tf",
		(rflags & 1U << 7)  ? "SF" : "sf",
		(rflags & 1U << 6)  ? "ZF" : "zf",
		(rflags & 1U << 4)  ? "AF" : "af",
		(rflags & 1U << 2)  ? "PF" : "pf",
		(rflags & 1U << 0)  ? "CF" : "cf",
		(rflags & 0x3000) == 0x3000 ? "UTENTE" : "SISTEMA");
	if (!id)
		return;
	if (!p) {
		flog(sev, "  processo %d non trovato", id);
		return;
	}
	flog(sev, "  RAX=%lx RBX=%lx RCX=%lx RDX=%lx",
			p->contesto[I_RAX],
			p->contesto[I_RBX],
			p->contesto[I_RCX],
			p->contesto[I_RDX]);
	flog(sev, "  RDI=%lx RSI=%lx RBP=%lx RSP=%lx",
			p->contesto[I_RDI],
			p->contesto[I_RSI],
			p->contesto[I_RBP],
			pila[3] + 8);
	flog(sev, "  R8 =%lx R9 =%lx R10=%lx R11=%lx",
			p->contesto[I_R8],
			p->contesto[I_R9],
			p->contesto[I_R10],
			p->contesto[I_R11]);
	flog(sev, "  R12=%lx R13=%lx R14=%lx R15=%lx",
			p->contesto[I_R12],
			p->contesto[I_R13],
			p->contesto[I_R14],
			p->contesto[I_R15]);
	flog(sev, "  backtrace:");
	natq rbp = p->contesto[I_RBP];
	for (;;) {
		natq* acsite = ((natq *)rbp + 1);
		if (((natq)acsite & 0x7) || !is_accessible((vaddr)acsite)) {
			flog(sev, "  ! %lx", rbp);
			break;
		}
		addr csite = (addr)(*acsite - 5);
		if (csite < &start || csite >= fine_codice_sistema)
			break;
		flog(sev, "  > %lx", *((natq *)rbp + 1) - 5);
		rbp = *(natq *)rbp;
	}
}

extern "C" void c_panic(const char *msg)
{
	static int in_panic = 0;

	if (in_panic) {
		flog(LOG_ERR, "panic ricorsivo. STOP");
		end_program();
	}
	in_panic = 1;

	flog(LOG_ERR, "PANIC: %s", msg);
	if (esecuzione->id)
		process_dump(esecuzione->id, LOG_ERR);
	flog(LOG_ERR, "  processi utente: %d", processi - 1);
	for (natl id = MIN_PROC_ID; id < MAX_PROC_ID; id += 16) {
		des_proc *p = des_p(id);
		if (p && p->cpl == LIV_UTENTE) {
			vaddr v_eip = fin_sis_p - 5 * sizeof(natq);
			natq dp = get_des(id, 1, v_eip);
			natq ind_fis_pag = (natq)extr_IND_FISICO(dp);
			addr f_eip = (addr)(ind_fis_pag | (v_eip & 0xFFF));
			flog(LOG_ERR, "    *) proc=%d RIP=%p", id, *(natq*)f_eip);
		}
	}
	end_program();
}

// se riceviamo un non-maskerable-interrupt, fermiamo il sistema
extern "C" void c_nmi()
{
	panic("INTERRUZIONE FORZATA");
}

// restituisce l'indirizzo fisico che corrisponde a ind_virt nello
// spazio di indirizzamento del processo corrente.
extern "C" faddr c_trasforma(vaddr ind_virt)
{
	natq d;
	for (int liv = 4; liv > 0; liv--) {
		d = get_des(esecuzione->id, liv, ind_virt);
		if (!extr_P(d)) {
			flog(LOG_WARN, "impossibile trasformare %lx: non presente a livello %d",
				ind_virt, liv);
			return 0;
		}
		if (extr_PS(d)) {
			// pagina di grandi dimensioni
			natq mask = (1UL << ((liv - 1) * 9 + 12)) - 1;
			return norm((d & ~mask) | (ind_virt & mask));
		}
	}
	return extr_IND_FISICO(d) | (ind_virt & 0xfff);
}

// ( [P_IOAPIC]

void apic_fill()
{
	apic_set_VECT(0, VETT_0);
	apic_set_VECT(1, VETT_1);
	apic_set_VECT(2, VETT_2);
	apic_set_VECT(3, VETT_3);
	apic_set_VECT(4, VETT_4);
	apic_set_VECT(5, VETT_5);
	apic_set_VECT(6, VETT_6);
	apic_set_VECT(7, VETT_7);
	apic_set_VECT(8, VETT_8);
	apic_set_VECT(9, VETT_9);
	apic_set_VECT(10, VETT_10);
	apic_set_VECT(11, VETT_11);
	apic_set_VECT(12, VETT_12);
	apic_set_VECT(13, VETT_13);
	apic_set_VECT(14, VETT_14);
	apic_set_VECT(15, VETT_15);
	apic_set_VECT(16, VETT_16);
	apic_set_VECT(17, VETT_17);
	apic_set_VECT(18, VETT_18);
	apic_set_VECT(19, VETT_19);
	apic_set_VECT(20, VETT_20);
	apic_set_VECT(21, VETT_21);
	apic_set_VECT(22, VETT_22);
	apic_set_VECT(23, VETT_23);
}

// )

// timer
extern "C" void attiva_timer(natl count);
const natl DELAY = 59659;
extern "C" void init_gdt();

extern "C" void salta_a_main();
extern "C" void cmain()
{
	natl mid;

	// (* anche se il primo processo non e' completamente inizializzato,
	//    gli diamo un identificatore, in modo che compaia nei log
	init.id = 0xFFFFFFFF;
	init.precedenza = MAX_PRIORITY;
	esecuzione = &init;
	// *)

	flog(LOG_INFO, "Nucleo di Calcolatori Elettronici, v5.12.1");
	init_gdt();
	flog(LOG_INFO, "gdt inizializzata");

	// (* Assegna allo heap di sistema HEAP_SIZE byte nel secondo MiB
	heap_init((addr)HEAP_START, HEAP_SIZE);
	flog(LOG_INFO, "Heap di sistema: %x B @%x", HEAP_SIZE, HEAP_START);
	// *)

	// ( il resto della memoria e' per le pagine fisiche (parte M2, vedi)
	init_des_frame();
	flog(LOG_INFO, "Pagine fisiche: %d", N_DF);
	// )

	flog(LOG_INFO, "sis/cond [%p, %p)", ini_sis_c, fin_sis_c);
	flog(LOG_INFO, "sis/priv [%p, %p)", ini_sis_p, fin_sis_p);
	flog(LOG_INFO, "io /cond [%p, %p)", ini_mio_c, fin_mio_c);
	flog(LOG_INFO, "usr/cond [%p, %p)", ini_utn_c, fin_utn_c);
	flog(LOG_INFO, "usr/priv [%p, %p)", ini_utn_p, fin_utn_p);

	faddr inittab4 = crea_tab4();

	if(!crea_finestra_FM(inittab4))
			goto error;
	loadCR3(inittab4);
	flog(LOG_INFO, "Caricato CR3");

	apic_init(); // in libce
	apic_reset(); // in libce
	apic_fill();
	flog(LOG_INFO, "APIC inizializzato");

	// ( inizializzazione dello swap, che comprende la lettura
	//   degli entry point di start_io e start_utente
	if (!swap_init())
			goto error;
	flog(LOG_INFO, "sb: blocks = %d", swap_dev.sb.blocks);
	flog(LOG_INFO, "sb: user   = %p/%p",
			swap_dev.sb.user_entry,
			swap_dev.sb.user_end);
	flog(LOG_INFO, "sb: io     = %p/%p",
			swap_dev.sb.io_entry,
			swap_dev.sb.io_end);
	// )
	//
	// ( creazione del processo main_sistema
	mid = crea_main_sistema();
	if (mid == 0xFFFFFFFF)
		goto error;
	flog(LOG_INFO, "Creato il processo main_sistema (id = %d)", mid);
	// )

	// ( creazione del processo dummy
	dummy_proc = crea_dummy();
	if (dummy_proc == 0xFFFFFFFF)
		goto error;
	flog(LOG_INFO, "Creato il processo dummy (id = %d)", dummy_proc);
	// )

	// (* selezioniamo main_sistema
	schedulatore();
	// *)
	// ( esegue CALL carica_stato; IRETQ (vedi "sistema.S")
	salta_a_main();
	// )

error:
	c_panic("Errore di inizializzazione");
}

void gdb_breakpoint() {}

extern "C" natl activate_p(void f(int), int a, natl prio, natl liv); //
extern "C" void terminate_p();	//
void main_sistema(int n)
{
	natl sync_io;


	// ( caricamento delle tabelle e pagine residenti degli spazi condivisi ()
	flog(LOG_INFO, "creazione o lettura delle tabelle e pagine residenti condivise...");
	if (!crea_spazio_condiviso())
		goto error;
 	// )

	gdb_breakpoint();

	// ( inizializzazione del modulo di io
	flog(LOG_INFO, "creazione del processo main I/O...");
	sync_io = sem_ini(0);
	if (sync_io == 0xFFFFFFFF) {
		flog(LOG_ERR, "Impossibile allocare il semaforo di sincr per l'IO");
		goto error;
	}
	// occupiamo l'entrata del timer
	aggiungi_pe(ESTERN_BUSY, 2);
	if (activate_p(swap_dev.sb.io_entry, sync_io, MAX_PRIORITY, LIV_SISTEMA) == 0xFFFFFFFF) {
		flog(LOG_ERR, "impossibile creare il processo main I/O");
		goto error;
	}
	flog(LOG_INFO, "attendo inizializzazione modulo I/O...");
	sem_wait(sync_io);
	// )

	// ( creazione del processo start_utente
	flog(LOG_INFO, "creazione del processo start_utente...");
	if (activate_p(swap_dev.sb.user_entry, 0, MAX_PRIORITY, LIV_UTENTE) == 0xFFFFFFFF) {
		flog(LOG_ERR, "impossibile creare il processo main utente");
		goto error;
	}
	// )
	// (* attiviamo il timer
	attiva_timer(DELAY);
	flog(LOG_INFO, "attivato timer (DELAY=%d)", DELAY);
	// *)
	// ( terminazione
	flog(LOG_INFO, "passo il controllo al processo utente...");
	terminate_p();
	// )
error:
	panic("Errore di inizializzazione");
}

// ( [P_SWAP]
// lo swap e' descritto dalla struttura des_swap, che specifica il canale
// (primario o secondario) il drive (primario o master) e il numero della
// partizione che lo contiene. Inoltre, la struttura contiene una mappa di bit,
// utilizzata per l'allocazione dei blocchi in cui lo swap e' suddiviso, e un
// "super blocco".  Il contenuto del super blocco e' copiato, durante
// l'inizializzazione del sistema, dal primo settore della partizione di swap,
// e deve contenere le seguenti informazioni:
// - magic (un valore speciale che serve a riconoscere la partizione, per
// evitare di usare come swap una partizione utilizzata per altri scopi)
// - bm_start: il primo blocco, nella partizione, che contiene la mappa di bit
// (lo swap, inizialmente, avra' dei blocchi gia' occupati, corrispondenti alla
// parte utente/condivisa dello spazio di indirizzamento dei processi da
// creare: e' necessario, quindi, che lo swap stesso memorizzi una mappa di
// bit, che servira' come punto di partenza per le allocazioni dei blocchi
// successive)
// - blocks: il numero di blocchi contenuti nella partizione di swap (esclusi
// quelli iniziali, contenenti il superblocco e la mappa di bit)
// - directory: l'indice del blocco che contiene la tabella di livello 4
// - l'indirizzo virtuale dell'entry point del programma contenuto nello swap
// (l'indirizzo di main)
// - l'indirizzo virtuale successivo all'ultima istruzione del programma
// contenuto nello swap
// - l'indirizzo virtuale dell'entry point del modulo io contenuto nello swap
// - l'indirizzo virtuale successivo all'ultimo byte occupato dal modulo io
// - checksum: somma dei valori precedenti (serve ad essere ragionevolmente
// sicuri che quello che abbiamo letto dall'hard disk e' effettivamente un
// superblocco di questo sistema, e che il superblocco stesso non e' stato
// corrotto)
//

// usa l'istruzione macchina BSF (Bit Scan Forward) per trovare in modo
// efficiente il primo bit a 1 in v
extern "C" int trova_bit(natl v);
void scrivi_swap(addr src, natl blocco);
void leggi_swap(addr dest, natl blocco);

natl ceild(natl v, natl q)
{
	return v / q + (v % q != 0 ? 1 : 0);
}

natq alloca_blocco()
{
	natl i = 0;
	natq risu = 0;
	natq vecsize = ceild(swap_dev.sb.blocks, sizeof(natl) * 8);

	// saltiamo le parole lunghe che contengono solo zeri (blocchi tutti occupati)
	while (i < vecsize && swap_dev.free[i] == 0) i++;
	if (i < vecsize) {
		natl pos = __builtin_ffs(swap_dev.free[i]) - 1;
		swap_dev.free[i] &= ~(1UL << pos);
		risu = pos + sizeof(natl) * 8 * i;
	}
	return risu;
}

void dealloca_blocco(natl blocco)
{
	if (blocco == 0)
		return;
	swap_dev.free[blocco / 32] |= (1UL << (blocco % 32));
}



// legge dallo swap il blocco il cui indice e' passato come primo parametro,
// copiandone il contenuto a partire dall'indirizzo "dest"
void leggi_swap(addr dest, natl blocco)
{
	natl sector = blocco * 8;

	leggisett(sector, 8, static_cast<natw*>(dest));
}

void scrivi_swap(addr src, natl blocco)
{
	natl sector = blocco * 8;

	scrivisett(sector, 8, static_cast<natw*>(src));
}

// inizializzazione del descrittore di swap
natw read_buf[256];
bool swap_init()
{
	// lettura del superblocco
	flog(LOG_DEBUG, "lettura del superblocco dall'area di swap...");
	leggisett(1, 1, read_buf);

	swap_dev.sb = *reinterpret_cast<superblock_t*>(read_buf);

	// controlliamo che il superblocco contenga la firma di riconoscimento
	for (int i = 0; i < 8; i++)
		if (swap_dev.sb.magic[i] != "CE64SWAP"[i]) {
			flog(LOG_ERR, "Firma errata nel superblocco");
			return false;
		}

	// controlliamo il checksum
	int *w = (int*)&swap_dev.sb, sum = 0;
	for (natl i = 0; i < sizeof(swap_dev.sb) / sizeof(int); i++)
		sum += w[i];

	if (sum != 0) {
		flog(LOG_ERR, "Checksum errato nel superblocco");
		return false;
	}

	flog(LOG_DEBUG, "lettura della bitmap dei blocchi...");

	// calcoliamo la dimensione della mappa di bit in pagine/blocchi
	natl pages = ceild(swap_dev.sb.blocks, DIM_PAGINA * 8);

	// quindi allochiamo in memoria un buffer che possa contenerla
	swap_dev.free = static_cast<natl*>(alloca((pages * DIM_PAGINA)));
	if (swap_dev.free == 0) {
		flog(LOG_ERR, "Impossibile allocare la bitmap dei blocchi");
		return false;
	}
	// infine, leggiamo la mappa di bit dalla partizione di swap
	leggisett(swap_dev.sb.bm_start * 8, pages * 8, reinterpret_cast<natw*>(swap_dev.free));
	return true;
}
// )
// ( [P_SEM_ALLOC]
// I semafori non vengono mai deallocati, quindi e' possibile allocarli
// sequenzialmente. Per far questo, e' sufficiente ricordare quanti ne
// abbiamo allocati
natl sem_allocati = 0;
natl alloca_sem()
{
	natl i;

	if (sem_allocati >= MAX_SEM)
		return 0xFFFFFFFF;

	i = sem_allocati;
	sem_allocati++;
	return i;
}

// dal momento che i semafori non vengono mai deallocati,
// un semaforo e' valido se e solo se il suo indice e' inferiore
// al numero dei semafori allocati
bool sem_valido(natl sem)
{
	return sem < sem_allocati;
}

// parte "C++" della primitiva sem_ini
extern "C" natl c_sem_ini(int val)
{
	natl i = alloca_sem();

	if (i != 0xFFFFFFFF)
		array_dess[i].counter = val;

	return i;
}
// )
#ifdef AUTOCORR
int MAX_LOG = 4;
#else
int MAX_LOG = 5;
#endif

extern "C" void c_log(log_sev sev, const char* buf, natl quanti)
{
	do_log(sev, buf, quanti);
}


