/*
 ============================================================================
 Name        : citizmdat.c
 Author      : Vasilije Blaženi
 Version     :
 Copyright   :
 Description : Program za čitanje/izmenu datoteke
 ============================================================================
 */

#define VERZIJA	"V0.7   06.11.2018."

#define _QNX 0	// 1 omogucuje kompajliranje naredaba specificnih za QNX

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>	// getopt(), optarg, optind, opterr, optopt
#include <err.h>	// err(), errx()
#include <errno.h>	// errno
#include <string.h>	// strcmp(), strdup()
#include <regex.h>

//#if _QNX == 0	// GNU/Linux
//	#include <sys/types.h>
//#endif

const char	POMOC[] =	/* Koristimo ovaj nacin da bi se mogao primeniti
 * i u QNX-u i u GNU/LINUX-u. */
	"																				\n"
	"   " VERZIJA "																	\n"
	"																				\n"
	" Program za citanje i/ili izmenu podataka u datoteci.							\n"
	" Autor: Aca Krinulovic															\n"
	"																				\n"
	" Poziv:																		\n"
	"																				\n"
	" ./citupdat -[uv]																\n"
	" ./citupdat [-i] DATOTEKA...													\n"
	"																				\n"
	" Opcije:																		\n"
	" -i   Prikazuje se izvestaj o toku programa.									\n"
	" -u   Prikazuje ovo uputstvo i prekida izvrsenje programa.						\n"
	"      Bez opcija i argumenata se takodje prikazuje ovo uputstvo.				\n"
	" -v   Prikazuje verziju programa i prekida izvrsenje istog.					\n"
	"																				\n"
	" Program cita svoj program sa 'stdin', medjutim, mogu se koristiti i sledeca	\n"
	" preusmeravanja:																\n"
	"./citupdat DATOTEKA... < PROGRAM			- cita iz datoteke PROGRAM			\n"
	" cat PROGRAM | ./citupdat DATOTEKA...		- datoteka PROGRAM se 				\n"
	"	prosledjuje preko cevi na 'stdin'											\n"
	" echo 'PROGRAM' | ./citupdat DATOTEKA...	- string PROGRAM se					\n"
	"	prosledjuje preko cevi na 'stdin'											\n"
	" ./citupdat <<KRAJ_PROGRAMA													\n"
	"    ...																		\n"
	"    ...																		\n"
	" KRAJ_PROGRAMA                             - cita program iz OVDE-				\n"
	"	-dokumenta																	\n"
	"																				\n"
	" Ovakva sintaksa je bila najpogodnija za omogucavanje zamene imena				\n"
	" datoteka koja sadrze dzoker-znake, uz izvrsenje sopstvenog PROGRAMa za		\n"
	" sve datoteke, radi cega se PROGRAM najpre mora zapamtiti u ovom programu,		\n"
	" kako bi se mogao ponavljati.													\n"
	"																				\n"
	"																				\n"
	" Sintaksa sopstvenog programa:													\n"
	" [// KOMENTAR][[+|-]s][-bcuU] [(+|-)i] [(+|-)o] [-p [POMERAJ]	\\				\n"
	"    [[-]%%[*]FORMAT] ] [VREDNOST]...											\n"
	"																				\n"
	"																				\n"
	" 'Naredbe' sopstvenog programa:												\n"
	"																				\n"
	" //  Kao prvi ne-beli znaci na pocetku reda oznacavaju komentar do kraja reda.	\n"
	" VREDNOST   Pri citanju/brisanju je oblika [N]x, gde je N broj ponavljanja		\n"
	"            citanja/brisanja polja tipa zadatog poslednjim specifikatorom		\n"
	"            FORMAT-a, dok pri upisivanju/umetanju predstavlja novu vrednost	\n"
	"            polja tipa zadatog poslednjim specifikatorom FORMAT-a.				\n"
	" -b  Brisanje zadatog tipa podatka (uz skracenje datoteke).					\n"
	" -c  Citanje podataka iz datoteke (podrazumevano ako se ne zada drugacija		\n"
	"     obrada ako n).															\n"
	" zatim podatak iz/za datoteku i u novom redu polozaj nakon pristupa datoteci	\n"
	" (+|-)o   Ukljucenje/iskljucenje opsirnog prikaza podataka: polozaja u			\n"
	"          datoteci u obliku 10-cifrenogd ekadnog broja sa vodecim nulama, iza	\n"
	"          kojeg slede 2 tacke i razmak, a zatim podatak iz/za datoteku i u		\n"
	"          novom redu polozaj nakon pristupa datoteci.							\n"
	" -p  Pomeraj za zadatu vrednost, pri cemu oznacena vrednost predstavlja		\n"
	"     relativni, neoznacena vrednost apsolutni pomeraj, format duzinu odgovarajuceg a 'k' skok na kraj datoteke.	\n"
	" [+|-]s   Smer citanja/upisa/brisanja/umetanja podatka (+/- - ka kraju/pocetku	\n"
	"          datoteke). Podrazumevan je ka kraju datoteke.						\n"
	" -u  Upis podataka u datoteku, uz prepisivanje postojecih.						\n"
	" -U  Umetanje zadatog podatka (uz produzenje datoteke).						\n"
	"																				\n"
	" FORMAT - kombinacija specifikacija formata konverzije biblioteckih			\n"
	" funkcija printf() i scanf(), uz neke dodatne specifikacije:					\n"
	"																				\n"
	" %[*][hh|h|l|ll][douXx], gde je: hh - short short, h - short, nista -			\n"
	" - osnovni tip, l - long, ll - long long, d - int, o - oktalno, u -			\n"
	" - unsigned int i X/x - heksadecimalno (velika/mala slova pri upisu u			\n"
	" 'stdout')																		\n"
	"																				\n"
	" %[l|L]f, gde je: nista - osnovni tip, l - double, L - long double, f -		\n"
	" float																			\n"
	"																				\n"
	" %[n]c, gde je: n - broj karaktera koji se ucitava, c - char (ne dodaje		\n"
	" se '\0' na kraj)																\n"
	"																				\n"
	" %[[-]n]s, gde je: n - sirina polja pri ucitavanju stringa do 1. ne-			\n"
	" -belog karaktera, a najmanja sirina polja pri ispisu, bez/sa '-' -			\n"
	" - desno/levo poravnanje pri ispisu, s - string (pri ucitavanju se				\n"
	" dodaje '\0' na kraj)															\n"
	"																				\n"
	" %[l]t, gde je: nista/l - UTC/lokalno vreme tipa time_t u datoteci, a			\n"
	" formata 'DD.MM.GGGG. cc:mm:ss' pri citanju sa 'stdin' zadate vrednosti za upis\n"
	" ili slanju ocitanje vrenosti na 'stdout'.										\n"
	"																				\n"
	" Primeri formata konverzija pri citanju/upisu/preskakanju/brisanju/			\n"
	" umetanju podataka:															\n"
	"																				\n"
	" -c %c 5x -u %lf 1 3.2 4.56 %d 2 (cita podatak tipa 'char' 5 puta, upisuje 3	\n"
	"podatka tipa double, a zatim 1 podatak tipa int)								\n"
	" -b %t 2x %hd 1x %f 3x %Lf 5x %15s x %c 15x									\n"
	" -u %hhd 10 67 %d 12345567 %ld 123456789 %f 23.45								\n"
	" -U %f 3.4e11 %lf 2341234234.234234 %15s \"Ovo je primer.\" %-10s \"proba\"	\n"
	"  (umece podatak tipa float, dobule, string uz desno poravnanje u polju		\n"
	"  sirine 15 karaktera, uz popunjavanje razmacima, a zatim string uz levo		\n"
	"  poravnanje u polju sirine 10 karaktera, uz popunjavanje razmacima)			\n"
	" -p %c x %f 2x %d 3x (preskocen 1 char, 2 float, 3 int)						\n"
	"																				\n";

#define IF(TIP, FORMAT)											\
	if( sscanf( bafer, "-" #TIP "=" FORMAT, ( TIP * )pod ) == 1 )	\
	{															\
		do														\
			fwrite( pod, sizeof( TIP ), 1, dat );				\
		while( ( p = strchr( p, ',' ) ) != NULL					\
			&& sscanf( ++p, FORMAT, ( TIP * )pod ) == 1 );		\
	}


unsigned char
duz_tipa_pod( char format[] )
{	// Podrazumeva se da je pri dodeli vrednosti nizu naredaba za one koje predstavljaju format ("%...") izvrsena provera i da su svi formati ispravni.
	char	*p = format + 1;	// preskakanje '%' na pocetku specifikatora formata
	short	br_kar;


	if( sscanf( p, "%hdc", &br_kar ) == 1 )	// %nc
		return br_kar;

	if( *p == 'c' || strncmp( p, "hh", 2 ) == 0 )	// p, %hhd, %hhu, %hho, %hhx, %hhX
		return sizeof( char );

	if( *p == 'h' )	// %hd, %hu, %ho, %hx, %hX
		return sizeof( short );

	if( strchr( "duoxX", *p ) != NULL )	// %d, %u, %o, %x, %X
		return sizeof( int );

	if( strcmp( p, "ld" ) == 0 )
		return sizeof( long );

	if( *p == 'l' )	// %la, %lA, %le, %lE, %lf, %lF, %lg, %lG
		return sizeof( float );

	if( *p == 'l' )	// %lf
		return sizeof( float );

	if( strchr( "aAeEfFgG", *p ) != NULL )	// %a, %A, %e, %E, %f, %F, %g, %G

		return sizeof( int );

	if( *p == 'L' )	// %La, %LA, %Le, %LE, %Lf, %LF, %Lg, %LG
		return sizeof( long double );

	errx( EXIT_FAILURE, "%s(): GRESKA: Nepostojeci specifikator formata %s\n",
		__func__, format );
}


int
main( int argc, char *argv[] )
{
	char 			bafer[ 300 ], format[ 20 ],*p, pod[ 100 ], **naredbe = NULL, *naredba,
		*ime_dat, izvestaj = 0, obrazac[] =
    "%(((h{0,2})[duoxX])|(l|L)?f|[[:digit:]]c)";
	unsigned char	indeks_arg;
	short			br_naredaba = 0, indeks_naredbe, brojac, br_citanja;
	int				od_polozaja, odziv_int;
	off_t 			pomeraj;
	FILE			*dat = NULL;
	regex_t			re_obj;	/* NAPOMENA: greška koju prikazuje Eclipse IDE za
		* ovaj tip podatka i funkcije za regularne izraze ne postoji, već je
		* posledica greške u samom IDE-u. */


	if( argc == 1 )
	{
		printf( "%s%s", POMOC, argv[ 0 ] );
		return EXIT_FAILURE;
	}

//			optind = 1;	/* da bi se omogucila ponovna analiza argumenata
//			  * ovog programa dole */

	while( ( odziv_int = getopt( argc, argv, "iuv" ) ) != -1 )
	{
		switch( odziv_int )
		{	/* U donjim slucajevima se koristi "stderr" da bi se izlaz (stdout)
			 * ovog programa mogao nesmetano preusmeriti (pomocu operatora ">"
			 * ili "|". */
			case 'i':
				izvestaj = 1;
				fputs( "\nIzvrsava se: ", stderr );

				for( indeks_arg = 0; indeks_arg < argc; indeks_arg++ )
					fprintf( stderr, "%s ", argv[ indeks_arg ] );

				fputs( "\n\n", stderr );
				break;

			case 'u':	case -1:
				if( odziv_int == -1 && argc != 1 )
					break;

				printf( "%s%s", POMOC, argv[ 0 ] );
				return EXIT_SUCCESS;

			case 'v':
				fprintf( stderr, "\nVerzija programa: %s\n\n", VERZIJA );
				return EXIT_SUCCESS;

			case '?':	// automatski se u "stderr" prikazuje poruka o gresci
//				errx( EXIT_FAILURE, "GRESKA: Nepostojeca opcija -%c\n", optopt );
				return EXIT_FAILURE;
		}
	}

//	if( program == NULL )
//		errx( EXIT_FAILURE, "GRESKA: Nije zadata programska datoteka\n" );

	if( ( odziv_int = regcomp( &re_obj, obrazac, REG_NOSUB ) ) != 0 )
	{
		regerror( odziv_int, &re_obj, bafer, sizeof bafer );
		errx( EXIT_FAILURE, "regcomp( %s ): GRESKA: %s\n", obrazac, bafer );
	}

	if( izvestaj )
		fputs( "Naredbe:\n", stderr );

	/* Ucitavanje naredaba sopstvenog programa u niz, kako bi se mogao ponoviti
	 * za sve zadate datoteke za obradu. */
	while( scanf( "%s", bafer ) == 1 )
	{
		if( strncmp( bafer, "//", 2 ) == 0 )
		{	// red s komentarom - preskace se
			fgets( bafer, sizeof bafer, stdin );
			continue;
		}

		if( bafer[ 0 ] == '%' )
		{	// naredba za specifikaciju formata - vrsi se provera ispravnosti iste
	        if( !regexec( &re_obj, bafer, 0, NULL, 0 ) )
	    		errx( EXIT_FAILURE, "regexec( %s ): GRESKA u formatu %s\n", obrazac, bafer );
    	}

		naredbe = ( char ** )realloc( naredbe, ++br_naredaba * sizeof( char * ) );
   		naredbe[ br_naredaba - 1 ] = strdup( bafer );

   		if( izvestaj )
			fprintf( stderr, "%hd) %s\n", br_naredaba - 1, bafer );
	}

	for( indeks_arg = optind; indeks_arg < argc; indeks_arg++ )
	{	// zadati program se vrsi za sve zadate datoteke
		char  nacin_otvaranja[] = "r+", obrada = 'c', smer = '+', opsirno = 0;
			/* Za svaku novu datoteku 'obrada', 'smer' i 'opsirno' moraju imati
			 * podrazumevane pocetne vrednosti. */

		// Otvaranje datoteke za obradu:
		while( ( dat = fopen( ime_dat = argv[ indeks_arg ], nacin_otvaranja ) ) == NULL )
		{
			if( errno != ENOENT )
				warn( "GRESKA pri otvaranju (%s) datoteke \"%s\"",
					nacin_otvaranja, ime_dat );

			// ne postoji datoteka
			strcpy( nacin_otvaranja, "w+" );

			if( izvestaj )
				fprintf( stderr, "Obrazovana nova datoteka\n" );
		}

		if( izvestaj )
			fprintf( stderr, "\nDatoteka \"%s\":\n\n", ime_dat );

		for( indeks_naredbe = 0; indeks_naredbe < br_naredaba; indeks_naredbe++ )
		{
			naredba = naredbe[ indeks_naredbe ];

			if( izvestaj )
				fprintf( stderr, "   %hd) %s\n", indeks_naredbe, naredba );

			if( strcmp( naredba, "-b" ) == 0 || strcmp( naredba, "-c" ) == 0 ||
				strcmp( naredba, "-u" ) == 0 || strcmp( naredba, "-U" ) == 0 ||
				strcmp( naredba, "-p" ) == 0 )
			{	/* brisanje/citanje/upisivanje/umetanje podataka ili polozaj/
				 * pomeraj */
				if( obrada == 'c' && smer == '+' )
				{	/* Pri prelasku sa citanje na upisivanje ili obrnuto se mora
					 * (prema literaturi za C), kao jedno od resenja, pozvati
					 * donja funkcija. Ona se u ovom programu svuda poziva, osim
					 * za slucaj u ovom uslovu. */
					if( fseeko( dat, ( off_t )0, SEEK_CUR ) )
						err( EXIT_FAILURE, "fseeko( %s ): GRESKA:", ime_dat );
				}

				obrada = naredba[ 1 ];
			}
			else if( strcmp( naredba, "+s" ) == 0 )	// smer ka kraju datoteke
				smer = '+';
			else if( strcmp( naredba, "-s" ) == 0 )	// smer ka početku datoteke
				smer = '-';
			else if( naredba[ 0 ] == '%' )
			{	// specifikacija formata podatka za čitanje/upis/umetanje
				strcpy( format, naredba );
			}
			else if( strcmp( naredba, "+o" ) == 0 )
				opsirno = 1;
			else if( strcmp( naredba, "-o" ) == 0 )
				opsirno = 0;

			/* VAZNA NAPOMENA:
			 * Uslovi za detekciju opcija sopstvenog programa i specifikatora
			 * formata moraju biti gore, jer algoritam podrazumeva da se dole
			 * vrsi obrada datoteke. */

			else if( obrada == 'c' )
			{	// citanje
				br_citanja = atoi( naredba );

				if( izvestaj )
					fprintf( stderr, "\n-c %s %hd\n", format, br_citanja );

				for( brojac = 1; brojac <= br_citanja; brojac++ )
				{
					if( fscanf( dat, format, pod ) != 1 )
						errx( EXIT_FAILURE, "GRESKA pri ucitavanju %s\n", format );

					if( opsirno )
						;	// dodati prikaz adrese

					if( strcmp( format, "%d" ) )
						printf( format, *( int * )pod );
					else if( strcmp( format, "%lf" ) )
						printf( format, *( double * )pod );

					puts( "\n" );
				}

				if( smer == '+' )
					;

				break;
			}
			else if( obrada == 'u' )
			{	// upisivanje
				if( izvestaj )
					fprintf( stderr, "-%c %cs %s %s\n", obrada, smer, format, naredba );

				if( strcmp( format, "%c" ) == 0 )
				{
					sscanf( naredba, format, ( char * )pod );

					if( smer == '-' && !fseeko( dat, -sizeof( char ), SEEK_CUR ) )
						err( EXIT_FAILURE, "fseeko( %s -%c %cs %s %s ): GRESKA:",
							ime_dat, obrada, smer, format, naredba );
							// pomeraj ka pocetku datoteke

					if( fwrite( pod, duz_tipa_pod( format ), 1, dat ) != 1 )
						err( EXIT_FAILURE, "fseeko( %s -%c %cs %s %s ): GRESKA:",
							ime_dat, obrada, smer, format, naredba );

					if( smer == '-' && !fseeko( dat, -sizeof( char ), SEEK_CUR ) )
						err( EXIT_FAILURE, "fseeko( %s -%c %cs %s %s ): GRESKA:",
							ime_dat, obrada, smer, format, naredba );
							// pomeraj ka pocetku datoteke
				}
				else if( strcmp( format, "%hhu" ) == 0 )
				{
					sscanf( naredba, format, ( unsigned char * )pod );

					if( fwrite( pod, duz_tipa_pod( format ), 1, dat ) != 1 )
						err( EXIT_FAILURE, "fwrite( %s %s %s ): GRESKA:\n",
							ime_dat, format, naredba );
				}
				else if( strcmp( format, "%hd" ) == 0 )
				{
					sscanf( naredba, format, ( short * )pod );

					if( fwrite( pod, duz_tipa_pod( format ), 1, dat ) != 1 )
						err( EXIT_FAILURE, "fwrite( %s %s %s ): GRESKA:\n",
							ime_dat, format, naredba );
				}
				else if( strcmp( format, "%hu" ) == 0 )
				{
					sscanf( naredba, format, ( unsigned short * )pod );

					if( fwrite( pod, duz_tipa_pod( format ), 1, dat ) != 1 )
						err( EXIT_FAILURE, "fwrite( %s %s %s ): GRESKA:\n",
							ime_dat, format, naredba );
				}
				else if( strcmp( format, "%d" ) == 0 )
				{
					sscanf( naredba, format, ( int * )pod );

					if( fwrite( pod, duz_tipa_pod( format ), 1, dat ) != 1 )
						err( EXIT_FAILURE, "fwrite( %s %s %s ): GRESKA:\n",
							ime_dat, format, naredba );
				}
				else if( strcmp( format, "%u" ) == 0 )
				{
					sscanf( naredba, format, ( unsigned * )pod );

					if( fwrite( pod, duz_tipa_pod( format ), 1, dat ) != 1 )
						err( EXIT_FAILURE, "fwrite( %s %s %s ): GRESKA:\n",
							ime_dat, format, naredba );
				}
				else if( strcmp( format, "%ld" ) == 0 )
				{
					sscanf( naredba, format, ( long * )pod );

					if( fwrite( pod, duz_tipa_pod( format ), 1, dat ) != 1 )
						err( EXIT_FAILURE, "fwrite( %s %s %s ): GRESKA:\n",
							ime_dat, format, naredba );
				}
				else if( strcmp( format, "%f" ) == 0 )
				{
					sscanf( naredba, format, ( float * )pod );

					if( fwrite( pod, duz_tipa_pod( format ), 1, dat ) != 1 )
						err( EXIT_FAILURE, "fwrite( %s %s %s ): GRESKA:\n",
							ime_dat, format, naredba );
				}
				else if( strcmp( format, "%lf" ) == 0 )
				{
fprintf( stderr, "format %s, broj %lf\n", format, ( double * )pod );
					sscanf( naredba, format, ( double * )pod );

					if( fwrite( pod, duz_tipa_pod( format ), 1, dat ) != 1 )
						err( EXIT_FAILURE, "fwrite( %s %s %s ): GRESKA:\n",
							ime_dat, format, naredba );
				}
				else if( strcmp( format, "%Lf" ) == 0 )
				{
					sscanf( naredba, format, ( long double * )pod );

					if( fwrite( pod, duz_tipa_pod( format ), 1, dat ) != 1 )
						err( EXIT_FAILURE, "fwrite( %s %s %s ): GRESKA:\n",
							ime_dat, format, naredba );
				}
			}
			else if( strcmp( naredba, "?" ) == 0 )
			{
				if( smer == '+' )
					;	// fseek() za duzinu podatka ka pocetku datoteke

				switch( obrada )
				{
					case 'b':
						break;

					case 'u':
						fwrite( pod, sizeof( int ), 1, dat );
						break;

					case 'U':
						break;
				}
			}
			else if( sscanf( naredba, format, ( double * )pod ) == 1 )
			{
	printf( "ucitan double '%lf'\n", *( double * )pod );
				if( smer == '+' )
					;	// fseek() za duzinu podatka ka pocetku datoteke

				if( obrada == 'u' )
					fwrite( pod, sizeof( double ), 1, dat );
				else
						; 	// fread()
			}
	else
	printf( "Linija %d\n", __LINE__ );

		}
continue;	// privemeno, dok se ne izbaci sve ispod
		if( dat == NULL || odziv_int == EOF  )
		{	/* Nije otvorena datoteka, kraj programske datoteke ili greska pri
			 * citanju programske datoteke (za sada zanemarujemo poslednju
			 * mogucnost). */
// treba i premotati program na pocetak
			if( ( optarg = argv[ optind ] ) == NULL )
				// u "optarg" se pamti ime datoteke
			{
				if( opsirno )
					fputs( "Kraj obrade\n", stderr );

				return EXIT_SUCCESS;
			}

			if( ( dat = fopen( optarg, "r+" ) ) == NULL )
				err( EXIT_FAILURE, "fopen( %s )", optarg );


			optind++;	// kasnije omogucuje otvaranje sledece datoteke
		}

//		else if( sscanf( p, "-char=%c", pod ) == 1 )
//		{	// karakter ne moze biti ','
//			do
//				fwrite( pod, sizeof( char ), 1, dat );
//			while( ( p = strchr( p, ',' ) ) != NULL
//				&& sscanf( ++p, "%c", pod ) == 1 );
//		}
//		else if( sscanf( p, "-short=%hd", ( short * )pod ) == 1 )
//		{	// karakter ne moze biti ','
//			do
//				fwrite( pod, sizeof( short ), 1, dat );
//			while( ( p = strchr( p, ',' ) ) != NULL
//				&& sscanf( ++p, "%hd", ( short * )pod ) == 1 );
//		}
//		else if( sscanf( p, "-int=%d", ( int * )pod ) == 1 )
//		{	// karakter ne moze biti ','
//			do
//				fwrite( pod, sizeof( int ), 1, dat );
//			while( ( p = strchr( p, ',' ) ) != NULL
//				&& sscanf( ++p, "%d", ( int * )pod ) == 1 );
//		}
//		else if( sscanf( p, "-float=%f", ( float * )pod ) == 1 )
//		{	// karakter ne moze biti ','
//			do
//				fwrite( pod, sizeof( float ), 1, dat );
//			while( ( p = strchr( p, ',' ) ) != NULL
//				&& sscanf( ++p, "%f", ( float * )pod ) == 1 );
//		}
//		else if( sscanf( p, "-double=%lf", ( double * )pod ) == 1 )
//		{	// karakter ne moze biti ','
//			do
//				fwrite( pod, sizeof( double ), 1, dat );
//			while( ( p = strchr( p, ',' ) ) != NULL
//				&& sscanf( ++p, "%lf", ( double * )pod ) == 1 );
//		}


		// karakteri ',' i ' ' se moraju zadati kao '\,' i '\ '
		IF( char, "%c" )
//		else IF( unsigned char, "%hhu" )
		else IF( short, "%hd" )
//		else IF( unsigned short, "%hu" )
		else IF( int, "%d" )
		else IF( unsigned, "%u" )
		else IF( long, "%ld" )
		else IF( float, "%f" )
		else IF( double, "%lf" )
		else if( sscanf( bafer, "%ld", &pomeraj ) == 1 || bafer[ 0 ] == 'k' )
		{	// pomeraj u odnosu na pocetak datoteke
			if( pomeraj < 0 || bafer[ 0 ] == '+' )
			{
				od_polozaja = SEEK_CUR;
			}
			else if( bafer[ 0 ] == 'k' )
			{
				pomeraj = 0;
				od_polozaja = SEEK_END;
			}
			else
			{	// apsolutni polozaj
				pomeraj = 0;
				od_polozaja = SEEK_SET;
			}

			if( ftello( dat ) == ( off_t )-1 )
				err( EXIT_FAILURE, "ftello( %s )", ime_dat );

			if( fseeko( dat, pomeraj, od_polozaja ) == -1 )
				err( EXIT_FAILURE, "fseek( %s, %ld, %s )", optarg, pomeraj,
					od_polozaja == SEEK_SET ? "SEEK_SET" :
					od_polozaja == SEEK_CUR ? "SEEK_CUR" : "SEEK_END" );
		}
//		else
//			errx( EXIT_FAILURE, "fopen( %s ): pogresan argument", arg );

		fclose( dat );
	}

	regfree( &re_obj );
	return EXIT_SUCCESS;
}
