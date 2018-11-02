/*
 ============================================================================
 Name        : citizmdat.c
 Author      : Vasilije Blaženi
 Version     :
 Copyright   : 
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#define VERZIJA	"V0.2  01.11.2018."

#define _QNX 0	// 1 omogucuje kompajliranje naredaba specificnih za QNX

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>	// getopt(), optarg, optind, opterr, optopt
#include <err.h>	// err(), errx()
#include <string.h>	// strcmp(), strdup()

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
	" -u   Prikazuje ovo uputstvo.													\n"
	"      Bez opcija i argumenata se takodje prikazuje ovo uputstvo.				\n"
	"																				\n"
	" Program cita svoj program sa 'stdin', medjutim, mogu se koristiti i sledeca	\n"
	" preusmeravanja:																\n"
	"./citupdat DATOTEKA... < PROGRAM			- cita iz datoteke PROGRAM			\n"
	" cat PROGRAM | ./citupdat DATOTEKA...		- datoteka PROGRAM se prosledjuje preko cevi na 'stdin'					\n"
	" echo 'PROGRAM' | ./citupdat DATOTEKA...	- string PROGRAM se prosledjuje preko cevi na 'stdin'	\n"
	" ./citupdat <<KRAJ_PROGRAMA													\n"
	"    ...																		\n"
	"    ...																		\n"
	" KRAJ_PROGRAMA        - cita PROGRAM iz OVDE-dokumenta							\n"
	"																				\n"
	" Ovakva sintaksa je bila najzgodnija za omogucavanje zamene imena datoteka koja\n"
	" sadrze dzoker-znake, uz izvrsenje sopstvenog PROGRAMa za sve datoteke, radi	\n"
	" cega se PROGRAM najpre mora zapamtiti u ovom programu, kako bi se mogao		\n"
	" ponavljati.																	\n"
	"																				\n"
	" Sopstveni program moze sadrzati sledece 'naredbe':								\n"
	" [// KOMENTAR][[+|-]s][-bcuU] [(+|-)i] [-o] [-p [POMERAJ] [[-]%%[*]FORMAT] ] [?|VREDNOST]...\n"
	"																				\n"
	" 'Naredbe':																	\n"
	"																				\n"
	" //  Kao prvi ne-beli znaci na pocetku reda oznacavaju komentar do kraja reda.	\n"
	" ?   Cita se vrednost poslednjeg zadatog tipa (FORMAT-om), pri cemu poslednja.	\n"
	"     zadata opcija od -[cuU] mora biti -c.										\n"
	" -b  Brisanje zadatog tipa podatka (uz skracenje datoteke).					\n"
	" -c  Citanje podataka iz datoteke.												\n"
	" (+|-)   Ukljucenje/iskljucenje prikaza izvestaja na 'stderr' korisnih za trazenje\n"
	"     gresaka u programu.												\n"
	" -o  Opsiran prikaz podataka: polozaja u datoteci u obliku 10-cifrenog			\n"
	" dekadnog broja sa vodecim nulama, iza kojeg slede 2 tacke i razmak, a			\n"
	" zatim podatak iz/za datoteku i u novom redu polozaj nakon pristupa datoteci	\n"
	" -p  Pomeraj za zadatu vrednost, pri cemu oznacena vrednost predstavlja		\n"
	"     relativni, neoznacena vrednost apsolutni pomeraj, format duzinu odgovarajuceg a 'k' skok na kraj datoteke.	\n"
	" [+|-]s   Smer citanja/upisa/brisanja/umetanja podatka (+/- - ka kraju/pocetku	\n"
	"          datoteke). Podrazumevano je ka kraju datoteke.						\n"
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
	" -c %c 5x -u %f 1 3.2 4.56 %d 2 (cita podatak tipa 'char' 5 puta, upisuje 3	\n"
	"podatka tipa float, a zatim 1 podatak tipa int)								\n"
	" -b %t 2x %hd 1x %f 3x %Lf5x %15s %c												\n"
	" -u %hhd 10 67 %d 12345567 %ld 123456789 %f 23.45								\n"
	" %f=3.4e11 %lf=2341234234.234234 %s=Ovo\\ je\\ primer %15s=primer				\n"
	"  (float, string, desno poravnanje u polju sirine 15 karaktera, uz popunjavanje razmacima)\n"
	" %10s=proba (levo poravnanje u polju sirine 10 karaktera, uz popunjavanje razmacima)\n"
	" %*c %*f %*d (preskocen 1 char, 1 float, 1 int)\n"
	" %*f (1 broj tipa 'float')\n"
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
	static int	re_nepreveden = 1;

	char	*p = format + 1,	// preskakanje '%' na pocetku specifikatora formata
		obrazac[] = "%(((h{0,2})[duoxX])|(l|L)?f|[[:digit:]]c)";
	regex_t	re_obj;
char buffer[BUFFER_SIZE];

	if( re_nepreveden )
	{
		if( ( re_nepreveden = regcomp( &re_obj, obrazac, REG_NOSUB ) ) != 0 )
		{
			regerror( re_nepreveden, &re_obj, buffer, sizeof buffer );
			fprintf( stderr,"grep: %s (%s)\n", buffer, obrazac );
			errx( EXIT_FAILURE, "regcomp( %s ): GRESKA: %s\n", obrazac, buffer );
		}
	}

	if( strncmp( p, "hh", 2 ) == 0 )	/* %hhd, %hhu, %hho, %hhx, %hhX */
		return sizeof( char );

	if( *p == 'h' )	// %hd, %hu, %ho, %hx, %hX
		return sizeof( short );

	if( strchr( "duoxX", *p ) == 0 )	// %d, %u, %o, %x, %X
		return sizeof( int );

	if( strncmp( p + 1, "ld", 2 ) == 0 )
		return sizeof( long );

	errx( EXIT_FAILURE, "GRESKA: Nepostojeći specifikator formata %s\n", format );
}

int
main( int argc, char *argv[] )
{
	char 			bafer[ 300 ], format[ 20 ],*p, pod[ 100 ], **naredbe = NULL, *naredba,
		*ime_dat, *odziv_charp, smer = 1, opsirno = 0, izvestaj = 0;
	unsigned char	indeks_arg, obrada = 'c';
	short			br_naredaba = 0, indeks_naredbe, brojac, br_citanja;
	int				od_polozaja, odziv_int;
	off_t 			pomeraj;
	FILE			*program = NULL, *dat = NULL;

/* Izbaciti main()-argumente, jer ce se samo prikazivati uputstvo kad se program pozove
 * bez argumenata, a sve ostalo (osim imena datoteka) ce se zadavati u programu. */
	if( argc == 1 )
	{
		printf( "%s%s", POMOC, argv[ 0 ] );
		return EXIT_FAILURE;
	}

	while( ( odziv_int = getopt( argc, argv, "iuv" ) ) != -1 )
	{
		if( odziv_int == 'o' )
		{
//			fputs( "Opsiran ispis\n", stderr );
			opsirno = 1;
			optind = 1;	/* da bi se omogucila ponovna analiza argumenata
			  * ovog programa dole */
			break;
		}
	}

	while( ( odziv_int = getopt( argc, argv, "iuv" ) ) != -1 )
	{
		switch( odziv_int )
		{	/* U donjim slucajevima se koristi "stderr" da bi se izlaz (stdout)
			 * ovog programa mogao nesmetano preusmeriti (pomocu operatora ">"
			 * ili "|". */
			case 'i':
				izvestaj = 1;
				break;
#if 0
			case 'p':
				// Otvaranje programske datoteke za ovaj program:
				if( ( program = fopen( optarg, "r" ) ) != NULL )
				{
					if( opsirno )
						fprintf( stderr, "\nProgramska datoteka \"%s\"\n", optarg );

					break;
				}

				err( EXIT_FAILURE, "GRESKA pri otvaranju programske datoteke "
					"\"%s\"", optarg );
#endif
			case 'u':	case -1:
				if( odziv_int == -1 & argc != 1 )
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

	// ucitavanje naredaba iz programa u niz
	while( scanf( "%s", bafer ) == 1 )
	{
		if( strncmp( bafer, "//", 2 ) == 0 )
		{	// red s komentarom - preskace se
			fgets( bafer, sizeof bafer, stdin );
			continue;
		}

		if( bafer == '%')
		{	// naredba za specifikaciju formata - vrsi se provera ispravnosti iste

#include <regex.h>

//#if _QNX == 0	// GNU/Linux
//	#include <sys/types.h>
//#endif

#define BUFFER_SIZE 512



	if( regexec( &re_obj, buffer, 0, NULL, 0 ) == 0 )
	{
		fputs( buffer, stdout );
	}

	regfree( &re_obj );


		naredbe = ( char ** )realloc( naredbe, ++br_naredaba * sizeof( char * ) );
		naredbe[ br_naredaba - 1 ] = strdup( bafer );
	}

	for( indeks_arg = 1; indeks_arg < argc; indeks_arg++ )
	{	// zadati program se vrsi za sve zadate datoteke
		ime_dat = argv[ indeks_arg ];
//		odziv_int = fscanf( program, "%s", bafer );
		if( izvestaj )
			fprintf( stderr, "\nObrada datoteke \"%s\":\n\n", ime_dat );

		for( indeks_naredbe = 0; indeks_naredbe < br_naredaba; indeks_naredbe++ )
		{
			naredba = naredbe[ indeks_naredbe ];

			if( izvestaj )
				fprintf( stderr, "Naredba \"%s\"\n", naredba );

			if( strcmp( bafer, "-b" ) == 0 || strcmp( bafer, "-c" ) == 0 ||
				strcmp( bafer, "-u" ) == 0 || strcmp( bafer, "-U" ) == 0 ||
				strcmp( bafer, "-p" ) == 0)
			{	/* brisanje/citanje/upisivanje/umetanje podataka ili polozaj/
				 * pomeraj */
				obrada = bafer[ 1 ];
			}
			else if( bafer[ 0 ] == '%' )
			{	// specifikacija formata podatka za citanje/upis/umetanje
				strcpy( format, bafer );
			}
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

				break;
			}
			else if( obrada == 'u' )
			{	// upisivanje

			}
			else if( strcmp( bafer, "?" ) == 0 )
			{
				if( smer > 0 )
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
			else if( sscanf( bafer, format, ( double * )pod ) == 1 )
			{
	printf( "ucitan double '%lf'\n", *( double * )pod );
				if( smer > 0 )
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
		else if( sscanf( bafer, "%d", &pomeraj ) == 1 || bafer[ 0 ] == 'k' )
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

			if( ftello( dat ) == -1L )
				err( EXIT_FAILURE, "ftello( %s )", ime_dat );

			if( fseeko( dat, pomeraj, od_polozaja ) == -1 )
				err( EXIT_FAILURE, "fseek( %s, %d, %s )", optarg, pomeraj,
					od_polozaja == SEEK_SET ? "SEEK_SET" :
					od_polozaja == SEEK_CUR ? "SEEK_CUR" : "SEEK_END" );
		}
//		else
//			errx( EXIT_FAILURE, "fopen( %s ): pogresan argument", arg );

	}

	fclose( program );
	fclose( dat );
	return EXIT_SUCCESS;
}
