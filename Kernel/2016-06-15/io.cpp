// io.cpp
//
#include <costanti.h>
#include <libce.h>
#include <sys.h>
#include <sysio.h>
#include <io.h>

const natl LIV = LIV_SISTEMA;

extern "C" void panic(const char *msg)
{
	flog(LOG_ERR, "modulo I/O: %s", msg);
	io_panic();
}

////////////////////////////////////////////////////////////////////////////////
//                        MEMORIA DINAMICA                                    //
////////////////////////////////////////////////////////////////////////////////

natl ioheap_mutex;

void* operator new(size_t s)
{
	void *p;

	sem_wait(ioheap_mutex);
	p = alloca(s);
	sem_signal(ioheap_mutex);

	return p;
}

void operator delete(void* p)
{
	sem_wait(ioheap_mutex);
	dealloca(p);
	sem_signal(ioheap_mutex);
}

extern "C" natq c_getiomeminfo()
{
	natq rv;
	sem_wait(ioheap_mutex);
	rv = disponibile();
	sem_signal(ioheap_mutex);
	return rv;
}

void* operator new(size_t s, align_val_t a)
{
	void *p;

	sem_wait(ioheap_mutex);
	p = alloc_aligned(s, a);
	sem_signal(ioheap_mutex);
	return p;
}

////////////////////////////////////////////////////////////////////////////////
//                         GESTIONE DELLA CONSOLE                             //
////////////////////////////////////////////////////////////////////////////////

struct des_console {
	natl mutex;
	natl sincr;
	char* punt;
	natq cont;
	natq dim;
} console;

extern "C" void c_writeconsole(const char* buff, natq quanti) //
{
	des_console *p_des = &console;

	if (!access(buff, quanti, false, false)) {
		flog(LOG_WARN, "writeconsole: parametri non validi: %p, %d:", buff, quanti);
		abort_p();
	}

	sem_wait(p_des->mutex);
#ifndef AUTOCORR
	for (natq i = 0; i < quanti; i++)
		char_write(buff[i]);
#else /* AUTOCORR */
	if (buff[quanti - 2] == '\n')
		quanti -= 2;
	flog(LOG_USR, "%.*s", quanti, buff);
#endif /* AUTOCORR */
	sem_signal(p_des->mutex);
}

void startkbd_in(des_console* d, char* buff, natq dim)
{
	d->punt = buff;
	d->cont = dim;
	d->dim  = dim;
	enable_intr_kbd();
}

extern "C" natq c_readconsole(char* buff, natq quanti) //
{
	des_console *d = &console;
	natq rv;

	if (!access(buff, quanti, true)) {
		flog(LOG_WARN, "readconsole: parametri non validi: %p, %d:", buff, quanti);
		abort_p();
	}

#ifdef AUTOCORR
	return 0;
#endif

	if (!quanti)
		return 0;

	sem_wait(d->mutex);
	startkbd_in(d, buff, quanti);
	sem_wait(d->sincr);
	rv = d->dim - d->cont;
	sem_signal(d->mutex);
	return rv;
}

void estern_kbd(int h) //
{
	des_console *d = &console;
	char a;
	bool fine;

	for(;;) {
		disable_intr_kbd();

		a = char_read_int();

		fine = false;
		switch (a) {
		case 0:
			break;
		case '\b':
			if (d->cont < d->dim) {
				d->punt--;
				d->cont++;
				str_write("\b \b");
			}
			break;
		case '\r':
		case '\n':
			fine = true;
			*d->punt = '\0';
			str_write("\r\n");
			break;
		default:
			*d->punt = a;
			d->punt++;
			d->cont--;
			char_write(a);
			if (d->cont == 0) {
				fine = true;
			}
			break;
		}
		if (fine)
			sem_signal(d->sincr);
		else
			enable_intr_kbd();
		wfi();
	}
}

extern "C" void c_iniconsole(natb cc)
{
	clear_screen(cc);
}

// Interruzione hardware della tastiera
const int KBD_IRQ = 1;
const int KBD_PRIO = MIN_EXT_PRIO + 0x50;

bool kbd_init()
{
	// blocchiamo subito le interruzioni generabili dalla tastiera
	disable_intr_kbd();

	// svuotiamo il buffer interno della tastiera
	drain_kbd();

	if (activate_pe(estern_kbd, 0, KBD_PRIO, LIV, KBD_IRQ) == 0xFFFFFFFF) {
		flog(LOG_ERR, "kbd: impossibile creare estern_kbd");
		return false;
	}
	flog(LOG_INFO, "kbd: tastiera inizializzata");
	return true;
}

bool vid_init()
{
	clear_screen(0x07);
	flog(LOG_INFO, "vid: video inizializzato");
	return true;
}

bool console_init()
{
	des_console *d = &console;

	if ( (d->mutex = sem_ini(1)) == 0xFFFFFFFF) {
		flog(LOG_ERR, "console: impossibile creare mutex");
		return false;
	}
	if ( (d->sincr = sem_ini(0)) == 0xFFFFFFFF) {
		flog(LOG_ERR, "console: impossibile creare sincr");
		return false;
	}
	return kbd_init() && vid_init();
}

////////////////////////////////////////////////////////////////////////////////
//                         INTERFACCE ATA                                     //
////////////////////////////////////////////////////////////////////////////////

struct des_ata {
	natb comando;
	natl mutex;
	natl sincr;
	natb cont;
	natb* punt;
} hd;

extern "C" natl hd_prd[];

const natb HD_IRQ = 14;
const natl HD_PRIO = MIN_EXT_PRIO + 0x60;

void hd_prepare_prd(natb* vett, natb quanti)
{
	natq n = quanti * DIM_BLOCK;
	int i = 0;

	while (n && i < MAX_PRD) {
		paddr p = trasforma(vett);
		natq  r = DIM_PAGINA - (p % DIM_PAGINA);
		if (r < n)
			r = n;
		hd_prd[i] = p;
		hd_prd[i + 1] = r;

		n -= r;
		vett += r;
		i += 2;
	}
	hd_prd[i - 1] |= 0x80000000;
}

void starthd_in(des_ata *d, natb vetti[], natl primo, natb quanti)
{
	d->cont = quanti;
	d->punt = vetti;
	d->comando = READ_SECT;
	hd_start_cmd(primo, quanti, READ_SECT);
}

extern "C" void c_readhd_n(natb vetti[], natl primo, natb quanti)
{
	des_ata *d = &hd;

	if (!access(vetti, quanti * DIM_BLOCK, true)) {
		flog(LOG_WARN, "readhd_n: parametri non validi: %p, %d", vetti, quanti);
		abort_p();
	}

	sem_wait(d->mutex);
	starthd_in(d, vetti, primo, quanti);
	sem_wait(d->sincr);
	sem_signal(d->mutex);
}

void starthd_out(des_ata *d, natb vetto[], natl primo, natb quanti)
{
	d->cont = quanti;
	d->punt = vetto + DIM_BLOCK;
	d->comando = WRITE_SECT;
	hd_start_cmd(primo, quanti, WRITE_SECT);
	hd_output_sect(vetto);
}

extern "C" void c_writehd_n(natb vetto[], natl primo, natb quanti)
{
	des_ata *d = &hd;

	if (!access(vetto, quanti * DIM_BLOCK, false)) {
		flog(LOG_WARN, "writehd_n: parametri non validi: %p, %d", vetto, quanti);
		abort_p();
	}

	sem_wait(d->mutex);
	starthd_out(d, vetto, primo, quanti);
	sem_wait(d->sincr);
	sem_signal(d->mutex);
}

void dmastarthd_in(des_ata *d, natb vetti[], natl primo, natb quanti)
{
	// la scrittura in iBMDTPR di &hd_prd[0] avviene in fase di inizializzazione
	d->comando = READ_DMA;
	d->cont = 1;					// informazione per il driver
	hd_prepare_prd(vetti, quanti);
	paddr prd = trasforma(hd_prd);
	bm_prepare(prd, false);
	hd_start_cmd(primo, quanti, READ_DMA);
	bm_start();
}

extern "C" void c_dmareadhd_n(natb vetti[], natl primo, natb quanti,
		natb &errore)
{
	des_ata *d = &hd;

	if (!access(vetti, quanti * DIM_BLOCK, true)) {
		flog(LOG_WARN, "dmareadhd_n: parametri non validi: %p, %d", vetti, quanti);
		abort_p();
	}

	sem_wait(d->mutex);
	dmastarthd_in(d, vetti, primo, quanti);
	sem_wait(d->sincr);
	sem_signal(d->mutex);
}

void dmastarthd_out(des_ata *d, natb vetto[], natl primo, natb quanti)
{
	d->comando = WRITE_DMA;
	d->cont = 1; 				// informazione per il driver
	hd_prepare_prd(vetto, quanti);
	paddr prd = trasforma(hd_prd);
	bm_prepare(prd, true);
	hd_start_cmd(primo, quanti, WRITE_DMA);
	bm_start();
}

extern "C" void c_dmawritehd_n(natb vetto[], natl primo, natb quanti)
{
	des_ata *d = &hd;

	if (!access(vetto, quanti * DIM_BLOCK, false)) {
		flog(LOG_WARN, "dmawritehd_n: parametri non validi: %p, %d", vetto, quanti);
		abort_p();
	}

	sem_wait(d->mutex);
	dmastarthd_out(d, vetto, primo, quanti);
	sem_wait(d->sincr);
	sem_signal(d->mutex);
}


void esternAta(int h)			// codice commune ai 2 processi esterni ATA
{
	des_ata* d = &hd;
	for(;;) {
		d->cont--;
		hd_ack_intr();
		switch (d->comando) {
		case READ_SECT:
			hd_input_sect(d->punt);
			d->punt += DIM_BLOCK;
			break;
		case WRITE_SECT:
			if (d->cont != 0) {
				hd_output_sect(d->punt);
				d->punt += DIM_BLOCK;
			}
			break;
		case READ_DMA:
		case WRITE_DMA:
			bm_ack();
			break;
		}
		if (d->cont == 0)
			sem_signal(d->sincr);
		wfi();
	}
}

bool hd_init()
{
	natl id;
	natb bus = 0, dev = 0, fun = 0;
	des_ata* d;

	d = &hd;

	if ( (d->mutex = sem_ini(1)) == 0xFFFFFFFF) {
		flog(LOG_ERR, "hd: impossibile creare mutex");
		return false;
	}
	if ( (d->sincr = sem_ini(0)) == 0xFFFFFFFF) {
		flog(LOG_ERR, "hd: impossibile creare sincr");
		return false;
	}

	if (!bm_find(bus, dev, fun)) {
		flog(LOG_WARN, "hd: bus master non trovato");
		return false;
	}

	flog(LOG_INFO, "bm: %2x:%2x:%2x", bus, dev, fun);
	bm_init(bus, dev, fun);

	id = activate_pe(esternAta, 0, HD_PRIO, LIV, HD_IRQ);
	if (id == 0xFFFFFFFF) {
		flog(LOG_ERR, "com: impossibile creare proc. esterno");
		return false;
	}

	hd_enable_intr();

	return true;
}

// ( ESAME 2016-06-15
static const int MAX_CE = 16;
struct des_ce {
	ioaddr iBMPTR, iBMLEN, iCMD, iSTS;
	natl sync;
	natl mutex;
	char *buf;
	natl quanti;
};
des_ce array_ce[MAX_CE];
natl next_ce;
//   ESAME 2016-06-15 )


// ( SOLUZIONE 2016-06-15
/*che permetta di leggere quanti byte dalla periferica numero id (tra quelle di tipo ce) copiandoli nel
buffer buf. Notare che buf è un indirizzo virtuale e il buffer potrebbe attraversare più pagine virtuali:
la primitiva dovrà funzionare in ogni caso, eventualmente eseguendo più trasferimenti*/
extern "C" void c_cedmaread(natl id, char *buf, natl quanti){

	// Controllo su esistenza periferica
	if(id >= next_ce){
		flog(LOG_WARN, "Periferica non esistente");
		abort_p();
	}

	// Controllo sul problema del cavallo di troia
	if(!access(buf, quanti, true)){
		flog(LOG_WARN, "Problema del cavallo di troia");
		abort_p();
	}

	des_ce *ce = &array_ce[id];

	// dato che le periferiche lavorano in DMA con indirizzi fisici,
	// converto da virtuale a fisico e scrivo nel registro della
	// periferica l'indirizzo fisico di destinazione
	paddr f = trasforma(buf);
	// dobbiamo controllare quanti byte ci sono da buffer fino alla fine della pagina
	// per sapere l'offset della pagina consideriamo gli ultimi 12 bit
	natl da_copiare = DIM_PAGINA - (f & 0xFFF);
	if(da_copiare > quanti){
		da_copiare = quanti;
	}

	// mutua esclusione della periferica, "presa del gettone"
	sem_wait(ce->mutex);
	ce->quanti = quanti - da_copiare;
	ce->buf = buf + da_copiare;

	outputl(da_copiare, ce->iBMLEN);
	outputl(f, ce->iBMPTR);
	outputl(1, ce->iCMD); // da questo comando in poi, la periferica inizia il suo lavoro

	// utilizzo la sync perche', se usassi il mutex, all'inizio ci sarebbe
	// un gettone, quindi la primitiva non si bloccherebbe, noi vogliamo
	// una primitiva bloccante, di conseguenza usiamo sync
	sem_wait(ce->sync);
	// "rilascio del gettone"
	sem_signal(ce->mutex);
}
extern "C" void estern_ce(int id){
	des_ce *c = &array_ce[id];

	for(;;){
		inputl(c->iSTS);	// funziona da risposta all'interruzione
		// dobbiamo controllare 'quanti' ed eventualmente decrementarlo
		if(c->quanti > 0){
			natl da_copiare;
			if(c->quanti >= DIM_PAGINA){
				da_copiare = DIM_PAGINA;
			} else{
				da_copiare = c->quanti;
			}

			c->quanti -= da_copiare;
			paddr f = trasforma(c->buf);
			c->buf += da_copiare;

			outputl(da_copiare, c->iBMLEN);
			outputl(f, c->iBMPTR);
			outputl(1, c->iCMD);
		} else{	// quanti == 0, abbiamo finito, risvegliamo il processo
			sem_signal(c->sync);
		}
		wfi();
	}
}
//   SOLUZIONE 2016-06-15 )

// ( ESAME 2016-06-15
bool ce_init()
{

	for (natb bus = 0, dev = 0, fun = 0;
	     pci_find_dev(bus, dev, fun, 0xedce, 0x1234);
	     pci_next(bus, dev, fun))
	{
		if (next_ce >= MAX_CE) {
			flog(LOG_WARN, "troppi dispositivi ce");
			break;
		}
		des_ce *ce = &array_ce[next_ce];
		ioaddr base = pci_read_confl(bus, dev, fun, 0x10);
		base &= ~0x1;
		ce->iBMPTR = base;
		ce->iBMLEN = base + 4;
		ce->iCMD = base + 8;
		ce->iSTS = base + 12;
		ce->sync = sem_ini(0);
		ce->mutex = sem_ini(1);
		natb irq = pci_read_confb(bus, dev, fun, 0x3c);
		activate_pe(estern_ce, next_ce, MIN_EXT_PRIO + 0x55, LIV, irq);
		flog(LOG_INFO, "ce%d %2x:%1x:%1x base=%4x IRQ=%d", next_ce, bus, dev, fun, base, irq);
		next_ce++;
	}
	return true;
}
//   ESAME 2016-06-15 )

////////////////////////////////////////////////////////////////////////////////
//                 INIZIALIZZAZIONE DEL SOTTOSISTEMA DI I/O                   //
////////////////////////////////////////////////////////////////////////////////

extern "C" natq end;
extern "C" void fill_io_gates();
// eseguita in fase di inizializzazione
extern "C" void main(int sem_io)
{

	fill_io_gates();
	ioheap_mutex = sem_ini(1);
	if (ioheap_mutex == 0xFFFFFFFF) {
		panic("impossible creare semaforo ioheap_mutex");
	}
	vaddr end_ = reinterpret_cast<vaddr>(&end);
	end_ = (end_ + DIM_PAGINA - 1) & ~(DIM_PAGINA - 1);
	heap_init(reinterpret_cast<void*>(end_), DIM_IO_HEAP);
	flog(LOG_INFO, "Heap del modulo I/O: %xB [%p, %p)", DIM_IO_HEAP,
			end_, end_ + DIM_IO_HEAP);
	if (!console_init())
		panic("inizializzazione console fallita");
	if (!hd_init())
		panic("inizializzazione hard disk fallita");
// ( ESAME 2016-06-15
	if (!ce_init())
		panic("inizializzazione CE fallita");
//   ESAME 2016-06-15 )
	sem_signal(sem_io);
	terminate_p();
}
