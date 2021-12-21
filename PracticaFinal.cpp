#include <iostream>
#include <cstdlib>
#include <fstream>
#include <string>
#include <windows.h>

using namespace std;

/**********************************************************
 * 					CONFIGURACIÓ
***********************************************************/
// Configuarció del tauler per defecte
const int CONF_X_DEF_FILES = 6;
const int CONF_X_DEF_COLUMN = 7;
const int CONF_X_DEF_DIF = 2;

// Estructura de configuració on guardem el tamany del tauler i la dificultat del joc.
struct Configuracio
{
    int files, column, dificultat;
	// FILES: 4 - 10
	// COLUMNES: 4 - 10
	// DIFICULTAT: 1 / 2 / 3
};

// Prototipus de les funcions relatives a la configuració.
void visualitzarConfiguracio(Configuracio);
void configuracio2fitxer(string, Configuracio);
Configuracio fitxer2configuracio(string);
void canviarConfiguracio(string, Configuracio &);
Configuracio generarConfiguracio(string);

/**********************************************************
 * 						JUGADORS
***********************************************************/
const int N_MAX_JUG = 50; // Nombre màxim de jugadors

// Estructura per a guardar la informació relativa a un jugador
struct Jugador
{
    string nom;
    int victories;
    int empats;
    int derrotes;
    float mitjana_movs;
    int u_p_movs;
    float puntuacio;
};

// Definim Jugadors com una estructura que fa la funció de Vector de Jugadors
typedef Jugador V_jugadors[N_MAX_JUG];
struct Jugadors
{
	int n;
	V_jugadors valors;
};

// Prototipus de les funcions relatives a la administració de jugadors
void mostrarJugador(Jugador);
void jugador2fitxer(Jugador);
Jugador inicialitzarJugador(string);
void actualitzarEstadistiques(Jugador &, int, int);
bool inserirJugadorNou(Jugador, Jugadors &);
void mostrarJugadors(Jugadors);
void jugadors2fitxer (ofstream &, const Jugadors &);
void fitxer2jugadors (ifstream &, Jugadors &);
bool inserirJugadorOrdenat(Jugador, Jugadors &);
bool trobarJugador(string nick, const Jugadors &, int &);
void ordenarVector(Jugadors &);
void mostrarTop10(Jugadors &);
void escriureRanking(Jugadors &);

/**********************************************************
 * 					UI I DISSENY
***********************************************************/
// Prototipus de les funcions relatives a menu, animacions...
void mostrarMenu();
void pensant();
void perdre();
void guanyar();
void empat();

/**********************************************************
 * 					4 EN RATLLA
***********************************************************/
// Definim Matriu com una matriu de caràcters que farà de tauler.
// Per a simplificar, aquesta matriu sempre serà 10x10 i guardarem
// en la configuració el tamany real del tauler.
typedef char Matriu [10][10];

// Prototipus de funcions relatives al tauler.
void inicialitzarMatriu(Matriu);
void mostrarMatriu(const Matriu, const Configuracio &, int, int);
void copiarMatriu(const Matriu, Matriu);

// Prototipus de funcions relatives a la mecànica del joc.
int jugar(const Configuracio &, int &, ofstream &);
bool tirar_fitxa(Matriu, const int, int &, char, const Configuracio &, int &, int &);

// Prototipus de funcions relatives a la reproducció de la partida.
void reproduirUltimaPartida(const Jugador &);

// Prototipus de les funcions de la 'IA' (UTILITZANT L'ALGORITME MINIMAX)
int eleccio_IA(const Matriu m, const int depth, const Configuracio & config);
int torn_jugador(const Matriu m, const int depth, const Configuracio & config);
int torn_IA(const Matriu m, const int depth, const Configuracio & config);
int punts(const Matriu m, const Configuracio & config);
bool comprovar(const Matriu m, const Configuracio & config);


/**********************************************************
 * 					FUNCIÓ PRINCIPAL
***********************************************************/
int main (void)
{
	// Funcions per a la correcta codificació d'entrada i eixida.
	SetConsoleCP(CP_UTF8);
	SetConsoleOutputCP(CP_UTF8);
	
	string nick;
	short opcio;
	char entrada_opcio;
	
	ifstream jugs_input; 		// FITXER DE LECTURA DELS JUGADORS
	ofstream jugs_output;		// FITXER D'ESCRITURA DELS JUGADORS
	
	ofstream partida;			// Fitxer on es guarda l'ultima partida
	string nom_arxiu;			// Nom d'aquest fitxer (depén del nick)

	bool execucio = true;		// Determina quan acaba l'execució del
								// programa (false)
	
	cout << "Dona'm el teu nick per a començar el joc: ";
	getline(cin, nick);
	
	Configuracio config;						// Configuració del jugador
	config = fitxer2configuracio(nick);			// Llig (si es possible) la
												// configuració
	
	Jugadors jugs;								// Vector de jugadors
	jugs.n = 0;

	int index;									// Índex del nostre jugador en
												// el Vector de Jugadors
	
	/* Busca el fitxer 'jugadors.txt' i copia la informació en la estructura
	** jugs.
	** Si no existeix el fitxer, inicialitza jugs i crea un fitxer nou amb la
	** estructura ja creada.	
	*/
	jugs_input.open("jugadors.txt");
	if(!jugs_input)
	{
		Jugador quim = inicialitzarJugador("Quim");
		Jugador lucas = inicialitzarJugador("Lucas");
		inserirJugadorOrdenat(quim, jugs);
		inserirJugadorOrdenat(lucas, jugs);
		jugs_output.open("jugadors.txt");
		if(!jugs_output)
			cout << "ERROR CREANT/OBRINT EL FITXER DE JUGADORS!!!" << endl;
		else
			jugadors2fitxer(jugs_output, jugs);
		jugs_output.close();
	}
	else
		fitxer2jugadors(jugs_input, jugs);
	jugs_input.close();
	
	/* Busca al nostre jugador dins del vector de jugadors i assigna el valor
	** del índex corresponent.
	** Si no el troba, l'afegeix ordenadament (segons la puntuació, 0).
	*/
	while(!trobarJugador(nick, jugs, index) && execucio)
		if(!inserirJugadorOrdenat(inicialitzarJugador(nick), jugs))
		{
			system("cls");

			cout << "S'ha aplegat al límit de jugadors. Límit actual: "
				 << N_MAX_JUG << " jugadors." << endl;

			cout << "\nFi de l'execució." << endl;
			execucio = false;
			system("pause");
		}
	
	int res, movs;			// RES: resultat de la partida (1, -1, 0).
							// MOVS: nombre de moviments de la partida.
	
	while (execucio)		// Comença la magia
	{
		system("cls");

		/*
		** ===== MENÚ D'OPCIONS =====
		*/
		mostrarMenu();
		
		cout << "Dona'm una opció: ";
		cin >> entrada_opcio;
		opcio = int(entrada_opcio) - 48;		// Evita errors si l'entrada
												// no és un enter
		
		system("cls");
		switch (opcio)
		{
			case 1:
				// VEURE CONFIGURACIÓ
				visualitzarConfiguracio(config);
				break;
			
			case 2:
				// CANVIAR CONFIGURACIÓ
				canviarConfiguracio(nick, config);
				break;
			
			case 3:
				// VEURE ESTADÍSTIQUES
				mostrarJugador(jugs.valors[index]);
				break;
			
			case 4:
				// REPRODUIR ÚLTIMA PARTIDA
				reproduirUltimaPartida(jugs.valors[index]);
				break;
			
			case 5:
				// MOSTRAR TOP 10
				mostrarTop10(jugs);
				break;

			case 6:
				// JUGAR PARTIDA

				// Obri el fitxer per a guardar els moviments (binari)
				nom_arxiu = jugs.valors[index].nom + "_ultimapartida.dat";
				partida.open(nom_arxiu.c_str(), ios::binary);
				
				if(!partida)
					cout << "No s'ha pogut crear el fitxer " << nom_arxiu
						 << endl;
				
				else		// ===== COMENÇA LA PARTIDA =====
				{
					res = jugar(config, movs, partida);		// Es juga la
															// partida i es
															// guarda en res
															// el resultat.
					
					/* S'actualitzen les estadístiques del jugador segons el
					** resultat
					*/
					actualitzarEstadistiques(jugs.valors[index], res, movs);
					
					ordenarVector(jugs);				// Ordena el Vector
					trobarJugador(nick, jugs, index);	// Troba el nou índex
				}

				partida.close();
				break;
			
			case 0:
				// EIXIR

				/* Es guarda el Vector de Jugadors actualizat en el fitxer
				** corresponent.
				*/
				jugs_output.open("jugadors.txt");
				if(!jugs_output)
					cout << "ERROR CREANT/OBRINT EL FITXER DE JUGADORS!!!"
						 << endl;
				else
					jugadors2fitxer(jugs_output, jugs);
				jugs_output.close();

				/* Escriu jugador_resultat.txt
				*/
				jugador2fitxer(jugs.valors[index]);
				
				/* Escriu ranking.txt
				*/
				escriureRanking(jugs);
				
				execucio = false;				// S'acaba l'execució
				break;
			
			default:
				// JIBIRI JIBIRI
				cout << "No ha seleccionant ninguna opció vàlida" << endl;
				break;
		}
		cout << endl;
		system("pause");
	}
	
	return 0;
}

/**********************************************************
Entrada: Configuració
Eixida: Resultat de la partida (1 / 0 / -1), nombre de
		moviments i arxiu dels moviments de la última partida.
Objectiu: Jugar partida al 4 en ratlla contra Mr. Roboto
***********************************************************/
int jugar(const Configuracio & config, int & movs, ofstream & arxiu)
{
	movs = 0;							// Nº de moviments
	int f, c;							// Fila i columna
	char entrada;						// Entrada del jugador
	int fila = -1, columna = -1;		// Fila i columna on s'ha tirat la
										// fitxa
	int total;							// Nº total de tirades: f*c
	
	Matriu m;							// Tauler
	inicialitzarMatriu(m);
	
	/* Les dues primeres linies del fitxer on guarda l'última partida seran:
	** el nombre de files i de columnes del tauler.
	*/
	arxiu.write((char *)(& config.files), sizeof(config.files));
	arxiu.write((char *)(& config.column), sizeof(config.column));
	
	total = config.files * config.column;	// Es defineix la variable total

	while(movs < total)						// (Mentre es puguen tirar fitxer)
	{

		/* ===== TORN DEL JUGADOR =====
		*/
		if(movs%2 == 0)
		{		
			system("cls");

			// Es mostra el tauler actual.
			mostrarMatriu(m, config, fila, columna);

			// Demana una columna.
			cout << "És el teu torn. En quina columna vols introduir la fitxa? ";
			cin >> entrada;
			
			c = int(entrada) - 48;		// 48 és el codi del 0 en ASCII
			
			fila = -1;					// Encara no s'ha col·locat la
			columna = -1;				// fitxa

			
			// Mentre no es puga tirar la fitxa...
			while(!tirar_fitxa(m, c-1, f, 'X', config, fila, columna))
			{
				cout << "Posició no vàlida. Torna a intentar-ho: ";
				cin >> entrada;
			
				c = int(entrada) - 48;
			}
			
			// Escriu al fitxer en quina columna s'ha tirat la fitxa
			arxiu.write((char *)(& columna), sizeof(columna));

			// Incrementa el nombre de moviments
			movs++;
		}

		/* ===== TORN DE LA IA =====
		*/
		else
		{
			system("cls");

			// Mostra el tauler actual
			mostrarMatriu(m, config, fila, columna);
			cout << "Torn de Mr. Roboto...\n" << endl;

			// Animació mentre que la IA decideix on tirar
			pensant();
			
			// Pot tardar en funció del tamany i la dificultat
			c = eleccio_IA(m, 4 + config.dificultat, config);
			
			system("cls");
			
			if(c != 100)		// Si no és un empat.
			{
				tirar_fitxa(m, c, f, 'O', config, fila, columna);
				// Escriu on s'ha tirat la fitxa
				arxiu.write((char *)(& columna), sizeof(columna));
				// Incrementa el nombre de moviments
				movs++;
			}
			else				// Empat
			{
				empat();
				return 0; 		// Torna un empat
			}
		}
		
		if(comprovar(m, config))		// Si algú ha guanyat
		{
			system("cls");
			if(movs%2 == 0)				// Guanya la IA
			{
				mostrarMatriu(m, config, fila, columna);
				perdre();
				return -1; 				// Torna una derrota
			}
			else						// Guanya el jugador
			{
				mostrarMatriu(m, config, fila, columna);
				guanyar();
				return 1; 				// Torna una victòria
			}
		}
	}
	
	empat();
	return 0; // Torna un empat
}

/**********************************************************
Entrada: Matriu m, nivell de profunditat de la IA, Configuracio
Eixida: Tirada triada per la IA (la columna).
Objectiu: Funció que determina quina és la millor tirada que pot fer la IA.
***********************************************************/
int eleccio_IA(const Matriu m, const int depth, const Configuracio & config)
{
	int i, f;							// Fila i columna
	int fila = -1, columna = -1;		// Fila i columna on ha tirat

	Matriu temp;						// Matriu temporal per a guardar
										// l'original

	int maxim = -100000000;			// Inicia max: idealment -infinit
	int eleccio = 100;					// Columna eligida per la IA

	for(i = 0; i < config.column; ++i)		// Recorre totes les possibles
		if(m[0][i] == ' ') 					// columnes on tirar
		{

			/* Fa una copia en temp i simula una tirada en la columna 'i'
			*/
			copiarMatriu(m, temp);
			tirar_fitxa(temp, i, f, 'O', config, fila, columna);

			/* Simula ara el torn del rival amb un nivell menys de profunditat
			** (recursivitat)
			*/
			if(torn_jugador(temp, depth - 1, config) + punts(temp, config) >= maxim)
			{

				// Si la puntuació és igual que la màxima tria la jugada
				// aleatòriament.
				if(torn_jugador(temp, depth - 1, config) + punts(temp, config) == maxim)
				{
					if(rand()%2 == 1)
						eleccio = i;
				}

				// Si la puntuació és major que la màxima fins al moment,
				// aquesta és la millor jugada de moment.
				else
				{
					eleccio = i;
					maxim = torn_jugador(temp, depth - 1, config) + punts(temp, config);
				}
			}

		}

	// Una volta examinades totes les jugades, tornem la millor segons la IA.
	return eleccio;
}

/**********************************************************
Entrada: Matriu m, nivell de profunditat de la IA, Configuracio
Eixida: Puntuació
Objectiu: Determina la puntuació de la tirada amb major puntuació per al jugador.
***********************************************************/
int torn_jugador(const Matriu m, const int depth, const Configuracio & config)
{
	int i, f;							// Fila i columna
	int fila = -1, columna = -1;		// Fila i columna on ha tirat
	
	Matriu temp;						// Matriu temporal per a guardar
										// l'original

	int minim = 1000000;				// Inicia min: idealment +infinit

	/* Si s'ha aplegat a la última jugada a examinar, torna la puntuació de la
	** situació actual del tauler.
	*/
	if(depth == 0)
		return punts(m, config);

	/* Si la partida resulta guanyada, torna la puntuació del tauler
	** + 250 per la profundiata actual. Així, assegura prioritzar guanyar
	** abans.
	*/
	if(comprovar(m, config))
		return punts(m, config) + depth*250;

	for(i = 0; i < config.column; i++)		// Recorre totes les possibles
		if(m[0][i] == ' ') 					// columnes on tirar
		{

			/* Fa una copia en temp i simula una tirada en la columna 'i'
			*/
			copiarMatriu(m, temp);
			tirar_fitxa(temp, i, f, 'X', config, fila, columna);

			/* Simula ara el torn de la IA amb un nivell menys de profunditat
			** (recursivitat)
			*/
			if(torn_IA(temp, depth - 1, config) < minim)
				// Tria la jugada amb menor puntuació (la millor per al rival)
				minim = torn_IA(temp, depth - 1, config);

		}
	
	/* Torna la puntuació de la millor jugada per a al jugador (el rival),
	** és a dir, la pijor per a la IA.
	*/
	return minim;		// En teoria MINIMAX, el jugador és MIN.
}

/**********************************************************
Entrada: Matriu m, nivell de profunditat de la IA, Configuracio
Eixida: Puntuació
Objectiu: Determina la puntuació de la columna amb major puntuació per a la màquina.
***********************************************************/
int torn_IA(const Matriu m, const int depth, const Configuracio & config)
{
	int i, f;							// Fila i columna
	int fila = -1, columna = -1;		// Fila i columna on ha tirat

	Matriu temp;						// Matriu temporal per a guardar
										// l'original

	int maxim = -100000;				// Inicia max: idealment -infinit
	
	/* Si s'ha aplegat a la última jugada a examinar, torna la puntuació de la
	** situació actual del tauler.
	*/
	if(depth == 0)
		return punts(m, config);
	
	/* Si la partida resulta guanyada, torna la puntuació del tauler
	** - 250 per la profundiata actual. Així, assegura prioritzar guanyar
	** abans.
	*/
	if(comprovar(m, config))
		return punts(m, config) - depth*250;
	
	for(i = 0; i < config.column; i++)		// Recorre totes les possibles
		if(m[0][i] == ' ') 					// columnes on tirar
		{

			/* Fa una copia en temp i simula una tirada en la columna 'i'
			*/
			copiarMatriu(m, temp);
			tirar_fitxa(temp, i, f, 'O', config, fila, columna);
			
			/* Simula ara el torn del jugador amb un nivell menys de profunditat
			** (recursivitat)
			*/
			if(torn_jugador(temp, depth - 1, config) > maxim)
			{
				// Tria la jugada amb major puntuació (la pijor per al rival)
				maxim = torn_jugador(temp, depth - 1, config);
			}
		}

	/* Torna la puntuació de la millor jugada per a la IA,
	** és a dir, la pijor per al jugador (el rival).
	*/
	return maxim;		// En teoria MINIMAX, la IA és MAX.
}

/**********************************************************
Entrada: Matriu m.
Eixida: Puntuació
Objectiu: Assigna una puntuació segons la situació en la que ens trobem.

Funcionament: Divideix el tauler en quadrats 4x4 i busca un 4 en ratlla en
			  algun d'aquests quadrats.
			  Fa el mateix per a trobar 3 en ratlla i 2 en ratlla.
***********************************************************/
int punts(const Matriu m, const Configuracio & config)
{
	int x, y;	// Fila i columna del primer element de la subdivisió.
	int i, j;	// Fila i columna de cada element
	int f, c;	// Variables auxiliars per a la comprovació de files i
				// columnes.

	/**********************************************************
	 * 		INTENTA GUANYAR PARTIDA
	***********************************************************/
	for(x = 0; x < config.files-3; x++)
		for(y = 0; y < config.column-3; y++)
		{
			// Mirar 4 en files
			for(f = 0; f < 4; f++)
			{
				i = f + x;
				j = y;
				if(((m[i][j]==m[i][j+1] && m[i][j+1]==m[i][j+2]) && m[i][j+2]==m[i][j+3]) && m[i][j]=='O')
					return 5000;
			}

			// Mirar 4 en les columnes
			for(c = 0; c < 4; c++)
			{
				i = x;
				j = c + y;
				if(((m[i][j]==m[i+1][j] && m[i+1][j]==m[i+2][j]) && m[i+2][j]==m[i+3][j]) && m[i][j]=='O')
					return 5000;
			}

			// Mirar 4 en diag princ
			i = x;
			j = y;
			if(((m[i][j]==m[i+1][j+1] && m[i+1][j+1]==m[i+2][j+2]) && m[i+2][j+2]==m[i+3][j+3]) && m[i][j]=='O')
				return 5000;

			// Mirar 4 en diag inv
			i = x;
			j = y + 3;
			if(((m[i][j]==m[i+1][j-1] && m[i+1][j-1]==m[i+2][j-2]) && m[i+2][j-2]==m[i+3][j-3]) && m[i][j]=='O')
				return 5000;
		}

	/**********************************************************
	 * 		EVITA PERDRE LA PARTIDA
	***********************************************************/
	for(x = 0; x < config.files-3; x++)
		for(y = 0; y < config.column-3; y++)
		{
			// Mirar 4 en files
			for(f = 0; f < 4; f++)
			{
				i = f + x;
				j = y;
				if(((m[i][j]==m[i][j+1] && m[i][j+1]==m[i][j+2])
					&& m[i][j+2]==m[i][j+3]) && m[i][j]=='X')
					return -5000;
			}

			// Mirar 4 en les columnes
			for(c = 0; c < 4; c++)
			{
				i = x;
				j = c + y;
				if(((m[i][j]==m[i+1][j] && m[i+1][j]==m[i+2][j])
					&& m[i+2][j]==m[i+3][j]) && m[i][j]=='X')
					return -5000;
			}

			// Mirar 4 en diag princ
			i = x;
			j = y;
			if(((m[i][j]==m[i+1][j+1] && m[i+1][j+1]==m[i+2][j+2])
				&& m[i+2][j+2]==m[i+3][j+3]) && m[i][j]=='X')
				return -5000;

			// Mirar 4 en diag inv
			i = x;
			j = y + 3;
			if(((m[i][j]==m[i+1][j-1] && m[i+1][j-1]==m[i+2][j-2])
				&& m[i+2][j-2]==m[i+3][j-3]) && m[i][j]=='X')
				return -5000;
		}

	/**********************************************************
	 * 		INTENTA FER 3 EN RATLLA (Amb possibilitat de 4, millor)
	***********************************************************/
	// Fer 3 en ratlla. El funcionament és el mateix que per a 4 en ratlla
	//però dividint en quadrats 3x3
	for(x = 0; x < config.files-2; x++)
		for(y = 0; y < config.column-2; y++)
		{
			// Mirar 3 en files
			for(f = 0; f < 3; f++)
			{
				i = f + x;
				j = y;
				if((m[i][j]==m[i][j+1] && m[i][j+1]==m[i][j+2]) && m[i][j]=='O')
					if(m[i][j-1]==' ' || m[i][j+3]==' ')
						return 1200;
					else
						return 1000;
			}

			// Mirar 3 en les columnes
			for(c = 0; c < 3; c++)
			{
				i = x;
				j = c + y;
				if((m[i][j]==m[i+1][j] && m[i+1][j]==m[i+2][j]) && m[i][j]=='O')
					if(m[i-1][j]==' ' || m[i+3][j]==' ')
						return 1200;
					else
						return 1000;
			}

			// Mirar 3 en diag princ
			i = x;
			j = y;
			if((m[i][j]==m[i+1][j+1] && m[i+1][j+1]==m[i+2][j+2]) && m[i][j]=='O')
				if(m[i-1][j-1]==' ' || m[i+3][j+3]==' ')
					return 1200;
				else
					return 1000;

			// Mirar 3 en diag inv
			i = x;
			j = y + 2;
			if((m[i][j]==m[i+1][j-1] && m[i+1][j-1]==m[i+2][j-2]) && m[i][j]=='O')
				if(m[i-1][j+1]==' ' || m[i+3][j-3]==' ')
					return 1200;
				else
					return 1000;
		}
	
	/**********************************************************
	 * 		EVITA 3 EN RATLLA DEL RIVAL
	***********************************************************/
	// Bloquejar 3 en ratlla
	for(x = 0; x < config.files-2; x++)
		for(y = 0; y < config.column-2; y++)
		{
			// Mirar 3 en files
			for(f = 0; f < 3; f++)
			{
				i = f + x;
				j = y;
				if((m[i][j]==m[i][j+1] && m[i][j+1]==m[i][j+2]) && m[i][j]=='X')
					if(m[i][j-1]==' ' || m[i][j+3]==' ')
						return -1200;
					else
						return -1000;
			}

			// Mirar 3 en les columnes
			for(c = 0; c < 3; c++)
			{
				i = x;
				j = c + y;
				if((m[i][j]==m[i+1][j] && m[i+1][j]==m[i+2][j]) && m[i][j]=='X')
					if(m[i-1][j]==' ' || m[i+3][j]==' ')
						return -1200;
					else
						return -1000;
			}

			// Mirar 3 en diag princ
			i = x;
			j = y;
			if((m[i][j]==m[i+1][j+1] && m[i+1][j+1]==m[i+2][j+2]) && m[i][j]=='X')
				if(m[i-1][j-1]==' ' || m[i+3][j+3]==' ')
					return -1200;
				else
					return -1000;

			// Mirar 3 en diag inv
			i = x;
			j = y + 2;
			if((m[i][j]==m[i+1][j-1] && m[i+1][j-1]==m[i+2][j-2]) && m[i][j]=='X')
				if(m[i-1][j+1]==' ' || m[i+3][j-3]==' ')
					return -1200;
				else
					return -1000;
		}

	/**********************************************************
	 * 		INTENTA FER 2 EN RATLLA
	***********************************************************/
	// Fer 2 en ratlla
	for(x = 0; x < config.files-1; x++)
		for(y = 0; y < config.column-1; y++)
		{
			// Mirar 2 en files
			for(f = 0; f < 2; f++)
			{
				i = f + x;
				j = y;
				if(m[i][j]==m[i][j+1] && m[i][j]=='O')
					return 250;
			}

			// Mirar 2 en les columnes
			for(c = 0; c < 2; c++)
			{
				i = x;
				j = c + y;
				if(m[i][j]==m[i+1][j] && m[i][j]=='O')
					return 250;
			}

			// Mirar 2 en diag princ
			i = x;
			j = y;
			if(m[i][j]==m[i+1][j+1] && m[i][j]=='O')
				return 250;

			// Mirar 2 en diag inv
			i = x;
			j = y + 1;
			if(m[i][j]==m[i+1][j-1] && m[i][j]=='O')
				return 250;
		}
		
	/**********************************************************
	 * 		EVITA 2 EN RATLLA DEL RIVAL
	***********************************************************/
	// Bloquetjar 2 en ratlla
	for(x = 0; x < config.files-1; x++)
		for(y = 0; y < config.column-1; y++)
		{
			// Mirar 2 en files
			for(f = 0; f < 2; f++)
			{
				i = f + x;
				j = y;
				if(m[i][j]==m[i][j+1] && m[i][j]=='X')
					return -250;
			}

			// Mirar 2 en les columnes
			for(c = 0; c < 2; c++)
			{
				i = x;
				j = c + y;
				if(m[i][j]==m[i+1][j] && m[i][j]=='X')
					return -250;
			}

			// Mirar 2 en diag princ
			i = x;
			j = y;
			if(m[i][j]==m[i+1][j+1] && m[i][j]=='X')
				return -250;

			// Mirar 2 en diag inv
			i = x;
			j = y + 1;
			if(m[i][j]==m[i+1][j-1] && m[i][j]=='X')
				return -250;
		}

	return 0;
}

/**********************************************************
Entrada: Matriu m.
Eixida: false/true
Objectiu: Determina si algú ha guanyat la partida.

Funcionament: Divideix el tauler en quadrats 4x4 i busca un 4 en ratlla en
			  algun d'aquests quadrats.
***********************************************************/
bool comprovar(const Matriu m, const Configuracio & config)
{
	int x, y;	// Fila i columna del primer element de la subdivisió.
	int i, j;	// Fila i columna de cada element
	int f, c;	// Variables auxiliars per a la comprovació de files i
				// columnes.
	
	/**********************************************************
	 * 			BUSCA 4 EN RATLLA
	***********************************************************/
	for(x = 0; x < config.files - 3; x++)
		for(y = 0; y < config.column - 3; y++)
		{
			// Mirar 4 en files
			for(f = 0; f < 4; f++)
			{
				i = f + x;
				j = y;
				if(((m[i][j]==m[i][j+1] && m[i][j+1]==m[i][j+2]) && m[i][j+2]==m[i][j+3]) && m[i][j]!=' ')
					return true;
			}

			// Mirar 4 en les columnes
			for(c = 0; c < 4; c++)
			{
				i = x;
				j = c + y;
				if(((m[i][j]==m[i+1][j] && m[i+1][j]==m[i+2][j]) && m[i+2][j]==m[i+3][j]) && m[i][j]!=' ')
					return true;
			}

			// Mirar 4 en diag princ
			i = x;
			j = y;
			if(((m[i][j]==m[i+1][j+1] && m[i+1][j+1]==m[i+2][j+2]) && m[i+2][j+2]==m[i+3][j+3]) && m[i][j]!=' ')
				return true;

			// Mirar 4 en diag inv
			i = x;
			j = y + 3;
			if(((m[i][j]==m[i+1][j-1] && m[i+1][j-1]==m[i+2][j-2]) && m[i+2][j-2]==m[i+3][j-3]) && m[i][j]!=' ')
				return true;
		}

	return false;
}

/**********************************************************
Entrada: Matriu m, columna on es tira fitxa, carcàcter del jugador, configuració
Eixida: Si s'ha pogut tirar (false/true) i la fila i columna on s'ha tirat
Objectiu: Tirar la fitxa en el tauler.
***********************************************************/
bool tirar_fitxa(Matriu m, const int c, int &f, char car, const Configuracio & config, int & fila, int & columna)
{
    bool fitxa_colocada;				// TRUE SI ACONSEGUEIX
    fitxa_colocada = false;             // COL·LOCAR-LA


	// Si no existeix la columna -> false
	if(c < 0 || c > config.column)
		return fitxa_colocada;

	// Si la columna no està buida -> false
    else if (m[0][c] != ' ')
        return fitxa_colocada;

	// Tira la fitxa -> true
    else
    {
        f = config.files - 1;
        while(!fitxa_colocada)
        {
            if(m[f][c] == ' ')
            {
                m[f][c] = car;
                fila = f;						// Es retorna per referència
                columna = c;					// la fila i columna on s'ha
                fitxa_colocada = true;			// tirat la fitxa
            }									
												// (ÚTIL PER A MOSTRAR LA
												// FITXA EN UN ALTRE COLOR
												// AL MOSTRAR LA MATRIU)
            f--;
        }

        return fitxa_colocada;
    }
}

/**********************************************************
Entrada: Matriu m, configuració, ultima fitxa tirada (fila i columna)
Eixida: El tauler (per pantalla)
Objectiu: Mostrar per pantalla el tauler de 4 en ratlla
***********************************************************/
void mostrarMatriu(const Matriu m, const Configuracio & c, int fila, int columna)
{
	int i, j;			// Iteradors per als bucles
						// (fila i columna, respectivament)
	
	cout << "\n\n";

	// Truquets i mangalaxes per a mostrar correctament la matriu
	
	/* Es recorre la matriu 
	*/
	for(i = 0; i < c.files; i++)
	{
		cout << "\t|\t";

		for(j = 0; j < c.column; j++)
			
			/* ES PINTA DE VERD L'ÚLTIMA FITXA AFEGIDA AL TAULER
			*/
			if(i == fila && j == columna)
			{
				HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
				SetConsoleTextAttribute(hConsole, 10);
				cout << m[i][j] << "\t";
				SetConsoleTextAttribute(hConsole, 15);
			}

			else
				cout << m[i][j] << "\t";
		
		cout << "|\n\t|";

		for(j = 0; j <= c.column; j++)
			cout << "\t";
		cout << "|\n";

	}

	// Linies que separen els nombres de les columnes del tauler
	cout << "\t";
	for(i = 0; i < c.column + 1; i++)
		for(j = 0; j < 8; j++)
			cout << "-";
	cout << "-" << endl << "\t\t";

	// Nombre de les columnes
	for(i = 1; i <= c.column; i++)
		cout << i << "\t";
	cout << "\n\n";

}

/**********************************************************
Entrada: Matriu m (buida)
Eixida: Matriu m
Objectiu: Plena la matriu amb espais
***********************************************************/
void inicialitzarMatriu(Matriu m)
{
	int i, j;
	for(i = 0; i < 10; i++)
		for(j = 0; j < 10; j++)
			m[i][j] = ' ';
	
	return;
}

/**********************************************************
Entrada: Matriu m.
Eixida: Matriu temp
Objectiu: Copia el contingut de m en temp
***********************************************************/
void copiarMatriu(const Matriu m, Matriu temp)
{
	int i, j;
	for(i = 0; i < 10; i++)
		for(j = 0; j < 10; j++)
			temp[i][j] = m[i][j];
	
	return;
}

/**********************************************************
Entrada: nick
Eixida: Configuracio, fitxer de configuració
Objectiu: Genera una configuració per defecte i la guarda en el disc
***********************************************************/
Configuracio generarConfiguracio(string nick)
{
    Configuracio config;
    ofstream f;					// Fitxer de configuració
	string fitxer;				// Nom d'aquest fitxer

	/* Es genera la nova configuració amb els valors per defecte.
	*/
	config.files = CONF_X_DEF_FILES;
	config.column = CONF_X_DEF_COLUMN;
	config.dificultat = CONF_X_DEF_DIF;
	
	/* S'obri el fitxer d'eixida
	*/
	fitxer = nick + "_configuracio.txt";
	f.open(fitxer.c_str());
	
	if(!f)
		cout << "ERROR CREANT/OBRINT EL FITXER DE CONFIGURACIÓ!!!" << endl;
	else
		/* Escriu la nova configuració
		*/
		configuracio2fitxer(nick, config);
    
	// Retorna la configuració creada
	return config;
}

/**********************************************************
Entrada: nick
Eixida: nova configuració, fitxer de configuració
Objectiu: Canvia la configuració per defecte i la guarda en el disc
***********************************************************/
void canviarConfiguracio(string nick, Configuracio & config)
{
	ofstream f;
	string fitxer;

	char entrada;
	
	/* Canvia la configuració.
	*/
	do {
		cout << "\n\t~ Configuració del tauler (màx.: 10 x 10): ~" << endl;
		cout << "\n\tPer defecte i recomanada 6x7 a nivell mitjà" << endl;
		cout << endl;

    	cout << "Introdueix el nombre de files: ";
    	cin >> entrada;
		config.files = int(entrada) - 48;

    	cout << endl;

    	cout << "Introdueix el nombre de columnes: ";
    	cin >> entrada;
		config.column = int(entrada) - 48;

    	cout << endl;
    	
    	cout << "Introdueix el nivell de dificultat: " << endl;
    	cout << "\n\t1. Fàcil\n\t2. Mitjà\n\t3. Difícil" << endl;
    	cout << endl << "\t(Aquest determinarà el nivell de profunditat de la IA\n"
    				 << "\ti en taulers molt grans pot ralentitzar molt la resposta\n"
    				 << "\tde Mr. Roboto degut a que el nombre de casos a analitzar\n"
    				 << "\tcreix exponencialment). Es recomana aquesta relació:\n\n";
    	cout << "\tNivell 3: Taulers de 4x4 fins a 6x6." << endl;
    	cout << "\tNivell 2: Taulers de 7x7 fins a 10x10." << endl;
    	cout << "\tNivell 1: En cas de voler prioritzar la velocitat en la resposta." << endl;
    	cout << endl;

    	cout << "Nivell: ";
    	cin >> entrada;
		config.dificultat = int(entrada) - 48;
	}
	while((config.files < 4 || config.column < 4) || (config.files > 10 || config.column > 10) || (config.dificultat < 1 || config.dificultat > 3));
	
	/* Escriu la nova configuració
	*/
	fitxer = nick + "_configuracio.txt";
	f.open(fitxer.c_str());
	if(!f)
		cout << "ERROR CREANT/OBRINT EL FITXER DE CONFIGURACIÓ!!!" << endl;
	else
		configuracio2fitxer(nick, config);
	
	return;
}

/**********************************************************
Entrada: config
Eixida: config (per pantalla)
Objectiu: Mostra la configuració per pantalla
***********************************************************/
void visualitzarConfiguracio(Configuracio config)
{
    cout << "\n\tLa configuració actual del tauler és de "
         << config.files << " x " << config.column << endl;
         
	cout << endl << "\tNivell de dificultat: ";
	switch(config.dificultat)
	{
		case 1:
			cout << "Fàcil" << endl;
			break;
		case 2:
			cout << "Mitjà" << endl;
			break;
		case 3:
			cout << "Difícil" << endl;
			break;
	}
	
	cout << "\n\n\n";

    return;
}

/**********************************************************
Entrada: nick, configuració
Eixida: fitxer amb la configuració
Objectiu: Guarda la configuració en el disc
***********************************************************/
void configuracio2fitxer(string nick, Configuracio c)
{
    ofstream eixida;
    string fitxer;
    
    fitxer = nick + "_configuracio.txt";
    eixida.open(fitxer.c_str());

    if(!eixida)
        cout << "ERROR CREANT/OBRINT EL FITXER DE CONFIGURACIÓ!!!" << endl;
    else
    {
        eixida << c.files << endl;
        eixida << c.column << endl;
        eixida << c.dificultat;
    }
    eixida.close();

    return;
}

/**********************************************************
Entrada: nick
Eixida: configuració
Objectiu: llig la configuració des d'un fitxer
***********************************************************/
Configuracio fitxer2configuracio(string nick)
{
    Configuracio config;
    ifstream f;
	string fitxer;
	
	fitxer = nick + "_configuracio.txt";
    f.open(fitxer.c_str());

    if(!f)
    {
        config = generarConfiguracio(nick);
	}
	else
    {
        f >> config.files;
        f >> config.column;
        f >> config.dificultat;
	}

    f.close();

    return config;
}

/**********************************************************
Entrada: jugador i vector de jugadors
Eixida: true si s'ha pogut, false en cas contrari, vector ordenat
Objectiu: Inserta el jugador i ordena el vector
***********************************************************/
bool inserirJugadorOrdenat(Jugador j, Jugadors & V_j)
{
	/* Afegeix (si es pot) el jugador al vector de jugadors existent
	*/
	if(!inserirJugadorNou(j, V_j))
		return false;					// NO és possible		-> false
	
	/* Si s'ha pogut afegir, ordena el vector segons la puntuació i nº mitjà
	** de moviments en la última partida.
	*/
	else
	{
		ordenarVector(V_j);
		return true;					// Afegit correctament 	-> true
	}
}

/**********************************************************
Entrada: vector de jugadors
Eixida: vector de jugadors ordenat
Objectiu: Ordena el vector de jugadors
***********************************************************/
void ordenarVector(Jugadors & V_j)
{
	int i;
	Jugador aux;

	bool ordenat;		// Si està desordenat	-> false
						// Si està ordenat 		-> true
	
	do					// Mentre que considerem el vector desordenat...
	{

		/* Suposem inicialment que està ordenat
		*/
		ordenat = true;
		
		for(i = 1; i < V_j.n; i++)
		{

			/* Si trobem un jugador mal ordenat, tenim que el vector està
			** desordenat (ordenat -> false) i intercanviem al jugador per
			** l'anterior
			*/
			if(V_j.valors[i].puntuacio > V_j.valors[i-1].puntuacio)
			{
				aux = V_j.valors[i];
				V_j.valors[i] = V_j.valors[i-1];
				V_j.valors[i-1] = aux;
				ordenat = false;
			}

			/* En cas d'empate es determina l'ordre segons el nombre mitjà de
			** moviments
			*/
			else if(V_j.valors[i].puntuacio == V_j.valors[i-1].puntuacio &&
					V_j.valors[i].mitjana_movs < V_j.valors[i-1].mitjana_movs)
			{
				aux = V_j.valors[i];
				V_j.valors[i] = V_j.valors[i-1];
				V_j.valors[i-1] = aux;
				ordenat = false;
			}

		}
		
	} while (!ordenat);

	// Una volta finalitzat aquest algorisme, ens trobarem amb el vector
	// correctament ordenat.

	return;
}

/**********************************************************
Entrada: vector de jugadors
Eixida: arxiu
Objectiu: Escriu la informació de tots els jugadors en un fitxer.
***********************************************************/
void jugadors2fitxer (ofstream & f, const Jugadors & V_j)
{
	int i;
	
	f << V_j.n << endl;
	for(i = 0; i < V_j.n; i++)
	{
		f << V_j.valors[i].nom << " " 
		  << V_j.valors[i].victories << " "
		  << V_j.valors[i].empats << " "
		  << V_j.valors[i].derrotes << " "
		  << V_j.valors[i].mitjana_movs << " "
		  << V_j.valors[i].u_p_movs << " "
		  << V_j.valors[i].puntuacio << endl;
	}
	
	return;
}

/**********************************************************
Entrada: fitxer
Eixida: true si s'ha pogut, false en cas contrari, vector ordenat
Objectiu: Inserta el jugador i ordena el vector
***********************************************************/
void fitxer2jugadors (ifstream & f, Jugadors & V_j)
{
	int i;
	
	f >> V_j.n;
	for(i = 0; i < V_j.n; i++)
	{
		f >> V_j.valors[i].nom;
		f >> V_j.valors[i].victories;
		f >> V_j.valors[i].empats;
		f >> V_j.valors[i].derrotes;
		f >> V_j.valors[i].mitjana_movs;
		f >> V_j.valors[i].u_p_movs;
		f >> V_j.valors[i].puntuacio;
	}
	
	return;
}

/**********************************************************
Entrada: vector de jugadors
Eixida: vector de jugadors (per pantalla)
Objectiu: Mostra els jugadors per pantalla
***********************************************************/
void mostrarJugadors(Jugadors V_j)
{
	int i;

	for(i = 0; i < V_j.n; i++)
	{
		mostrarJugador(V_j.valors[i]);
		cout << endl;
	}

	return;
}

/**********************************************************
Entrada: Jugador
Eixida: vector de jugadors
Objectiu: Afegeix un jugador nou al vector de jugadors
***********************************************************/
bool inserirJugadorNou(Jugador j, Jugadors & V_j)
{
	int i;
	bool es_pot_inserir;	// Si es pot 		-> true
							// si no es pot		-> false
	
	/* S'ha superat el nombre màxim de jugadors -> false
	*/
	if(V_j.n >= N_MAX_JUG)
		return false;
	
	/* Existeix un jugador amb el mateix nom -> false
	*/
	es_pot_inserir = true;
	for(i = 0; i < V_j.n; i++)
		if(V_j.valors[i].nom == j.nom)
			es_pot_inserir = false;
	
	/* Altre cas -> true
	*/
	if(es_pot_inserir)
	{
		V_j.valors[V_j.n] = j;			// S'afegeix al jugador al final
		V_j.n++;						// del vector.
		return true;
	}

	else
		return false;
}

/**********************************************************
Entrada: Jugador
Eixida: Jugador (per pantalla)
Objectiu: Mostra un jugador per pantalla
***********************************************************/
void mostrarJugador(Jugador j)
{
	cout << "\n\t-----------------------------------"  << endl;
    cout << "\t\tJugador: " << j.nom << endl;
    cout << "\t-----------------------------------"  << endl;
    cout << "Victories:\t\t\t\t\t" << j.victories << endl;
    cout << "Empats:\t\t\t\t\t\t" << j.empats << endl;
    cout << "Derrotes:\t\t\t\t\t" << j.derrotes << endl;
    cout << "Nombre mitjà de moviments per partida:\t\t" << j.mitjana_movs << endl;
    
	/* Per qüestions d'estil hem decidit multiplicar la puntuació per 1000 i
	** mostrar-la per pantalla com un enter.
	*/
	cout << "Puntuació:\t\t\t\t\t" << int(j.puntuacio * 1000) << endl;

    return;
}

/**********************************************************
Entrada: Jugador
Eixida: Fitxer amb jugador
Objectiu: Escriu la informació del jugador en un fitxer
***********************************************************/
void jugador2fitxer(Jugador j)
{
	ofstream f;
	string nom;
	nom = j.nom + "_resultats.txt";
	f.open(nom.c_str());
	
	if(!f)
		cout << "HA EXPLOTAT EL ORDENADOR. AWUITA" << endl;
	else
	{
		f << "\t-----------------------------------"  << endl;
	    f << "\t\tJugador: " << j.nom << endl;
	    f << "\t-----------------------------------"  << endl;
	    f << "Victories:\t\t\t\t\t" << j.victories << endl;
	    f << "Empats:\t\t\t\t\t\t" << j.empats << endl;
	    f << "Derrotes:\t\t\t\t\t" << j.derrotes << endl;
	    f << "Nombre mitjà de moviments per partida:\t\t" << j.mitjana_movs << endl;

		/* Per qüestions d'estil hem decidit multiplicar la puntuació per 1000 i
		** mostrar-lo al fitxer com un enter.
		*/
	    f << "Puntuació:\t\t\t\t\t" << int(j.puntuacio * 1000) << endl;
	}
	
	f.close();
	
	return;
}

/**********************************************************
Entrada: nick
Eixida: Jugador
Objectiu: Crea i posa les estadístiques del jugador a 0
***********************************************************/
Jugador inicialitzarJugador(string nombre)
{
    Jugador j;

    j.nom = nombre;
    j.victories = 0;
    j.empats = 0;
    j.derrotes = 0;
    j.mitjana_movs = 0;
    j.u_p_movs = 0;
    j.puntuacio = 0;

    return j;
}

/**********************************************************
Entrada: Jugador, resultat, nombre de moviments
Eixida: Jugador (amb noves estadístiques)
Objectiu: Actualitza les estadístiques del jugador al finalitzar la partida.
***********************************************************/
void actualitzarEstadistiques(Jugador & j, int res, int num_movs)
{
    int partides;			// Nº de partides jugades

	// Calcula el nº de partides jugades
    partides = j.derrotes + j.victories + j.empats;

	// Actualitza les estadístiques segons el resultat de la partida
    if (res == 1)
		j.victories++;
	
    else if (res == 0)
		j.empats++;

    else if (res == -1)
		j.derrotes++;
	
	else
		cout << "Resultat no vàlid" << endl;

	// Assigna el nº de moviments de la última partida
	j.u_p_movs = num_movs;

	// Calcula la mitjana de moviments
    j.mitjana_movs = float(j.mitjana_movs*partides + num_movs)/(partides+1);

	// Calcula la puntuació
    j.puntuacio = float(j.victories - j.derrotes)/(partides+1);

    return;
}

/**********************************************************
Entrada: nick, Vector de jugadors
Eixida: si s'ha trobat (true/false) i l'index del jugador al vector
Objectiu: Busca un jugador al vector de jugadors.
***********************************************************/
bool trobarJugador(string nick, const Jugadors & jugs, int & index)
{
	int i;
	bool trobat = false;		// Si l'ha trobat 		-> true
								// si no l'ha trobat 	-> false

	/* Recorre el vector i si troba un jugador amb el mateix nom, guarda en la
	** variable index el valor corresponent.
	*/
	for(i = 0; i < jugs.n; i++)
		if(jugs.valors[i].nom == nick)
		{
			index = i;
			trobat = true;		// -> l'ha trobat
		}

	/* Si no l'ha trobat, situarem l'índex just al final del vector.
	*/
	if(!trobat)
		index = jugs.n;

	return trobat;
}

/**********************************************************
Entrada: Vector de jugadors
Eixida: Top 10
Objectiu: Mostra el top 10 per pantalla
***********************************************************/
void mostrarTop10(Jugadors & V_j)
{
	int i;

	// Ordena el vector per evitar errors.
	ordenarVector(V_j);
	
	/* Si hi ha menys de 10 jugados, ens els mostra tots.
	*/
	if(V_j.n < 10)
		for(i = 0; i < V_j.n; i++)
			mostrarJugador(V_j.valors[i]);

	/* Si hi ha més de 10 jugadors, ens mostra sols els 10 amb al major
	** puntuació.
	*/
	else
		for(i = 0; i < 10; i++)
			mostrarJugador(V_j.valors[i]);

	return;
}

/**********************************************************
Entrada: Vector de jugadors
Eixida: Top 10 (en fitxer)
Objectiu: Mostra el top 10 per pantalla
***********************************************************/
void escriureRanking(Jugadors & V_j)
{
	ofstream f;
	f.open("ranking.txt");
	
	if(!f)
		cout << "Error creant el fitxer ranking.txt" << endl;

	/* Escriu els 10 jugadors amb la major puntuació.
	** Aquest algorisme segueix la mateixa lògica que la funció
	** 'mostrarTop10()' definida anteriorment.
	*/
	else
	{
		int i;

		ordenarVector(V_j);

		if(V_j.n < 10)
			for(i = 0; i < V_j.n; i++)
			{
				f << "\n\t-----------------------------------"  << endl;
			    f << "\t\tN. " << i+1 << ": " << V_j.valors[i].nom << endl;
			    f << "\t-----------------------------------"  << endl;
			    f << "Victories:\t\t\t\t\t" << V_j.valors[i].victories << endl;
			    f << "Empats:\t\t\t\t\t\t" << V_j.valors[i].empats << endl;
			    f << "Derrotes:\t\t\t\t\t" << V_j.valors[i].derrotes << endl;
			    f << "Nombre mitjà de moviments per partida:\t\t" << V_j.valors[i].mitjana_movs << endl;
			    
				/* Per qüestions d'estil hem decidit multiplicar la puntuació per 1000 i
				** mostrar-lo al fitxer com un enter.
				*/
				f << "Puntuació:\t\t\t\t\t" << int(V_j.valors[i].puntuacio * 1000) << endl;
				
				f << endl;
			}
		
		else
			for(i = 0; i < 10; i++)
			{
				f << "\n\t-----------------------------------"  << endl;
			    f << "\t\tN. " << i+1 << ": " << V_j.valors[i].nom << endl;
			    f << "\t-----------------------------------"  << endl;
			    f << "Victories:\t\t\t\t\t" << V_j.valors[i].victories << endl;
			    f << "Empats:\t\t\t\t\t\t" << V_j.valors[i].empats << endl;
			    f << "Derrotes:\t\t\t\t\t" << V_j.valors[i].derrotes << endl;
			    f << "Nombre mitjà de moviments per partida:\t\t" << V_j.valors[i].mitjana_movs << endl;
			    
				/* Per qüestions d'estil hem decidit multiplicar la puntuació per 1000 i
				** mostrar-lo al fitxer com un enter.
				*/
				f << "Puntuació:\t\t\t\t\t" << int(V_j.valors[i].puntuacio * 1000) << endl;
				
				f << endl;
			}
	}
	
	f.close();

	return;
}

/**********************************************************
Entrada: Jugador
Eixida: Última partida
Objectiu: Mostrar per pantalla la última partida
***********************************************************/
void reproduirUltimaPartida(const Jugador & j)
{
	ifstream f;
	string nom_arxiu;
	
	Matriu m;					// Matriu sobre la que es reproduirà la
								// partida.

	Configuracio config;		// Configuració on es guardarà la
								// configuració del tauler de la partida.
	
	int fil, col;				// Fila i columna on es vol tirar la fitxa
	int fila, columna;			// Fila i columna on es tira la fitxa
	
	inicialitzarMatriu(m);
	
	nom_arxiu = j.nom + "_ultimapartida.dat";
	f.open(nom_arxiu.c_str(), ios::binary);
	
	if(f)
	{

		/* Es llig la configuració del tauler
		*/
		f.read((char *)(& config.files), sizeof(config.files));	  // FILES
		f.read((char *)(& config.column), sizeof(config.column)); // COLUMNES
		
		// ====== COMENÇA L'ESPECTACLE ======
		cout << "Reproduint partida..." << endl;
		
		/* Cada iteració són dues tirades
		** (una del jugador i una altra de la IA)
		*/
		for(int i = 0; i < j.u_p_movs; i+=2)
		{

			// TORN DEL JUGADOR
			cout << "Tira " << j.nom << ":" << endl;

			/* Llig en quin columna ha de tirar
			*/
			f.read((char *)(& col), sizeof(col));

			/* Tira la fitxa
			*/			
			tirar_fitxa(m, col, fil, 'X', config, fila, columna);
			
			mostrarMatriu(m, config, fila, columna);
			system("pause");
			

			if(i+1 < j.u_p_movs)		// Mentre que continue la partida....
			{

				// TORN DE LA IA
				cout << "Tira Mr. Roboto:" << endl;

				/* Llig en quin columna ha de tirar
				*/
				f.read((char *)(& col), sizeof(col));

				/* Tira la fitxa
				*/
				tirar_fitxa(m, col, fil, 'O', config, fila, columna);
				
				mostrarMatriu(m, config, fila, columna);
				system("pause");
			}
		}

		// Acaba la reproducció
		cout << "\n\tFI DE LA PARTIDA\n" << endl;
	}

	/* En cas de no trobar ningun arxiu 'jugador_ultimapartida.dat'...
	*/
	else
		cout << "El jugador " << j.nom << " no ha jugat ninguna partida." << endl;
	
	f.close();

	return;
}

/**********************************************************
Entrada: 
Eixida: Menú d'opcions
Objectiu: Mostra per pantalla al nostre heroi Mr. Roboto amb el menú d'opcions
***********************************************************/
void mostrarMenu()
{
	cout << endl;
	cout << "         ·      ·               ________________" 									<< endl
		 << "     _____"<<char(92)<<"____/_____          |                |      Enfronta't"	<< endl
		 << "    |                |         |   Connecta-4   |     contra la IA" 				<< endl
		 << " _  |    __    __    |  _      |________________|     'Mr. Roboto'" 				<< endl
		 << "| | |___|  |__|  |___| | |" 														<< endl
	     << "| |=|   |__|  |__|   |=| |              MENÚ D'OPCIONS" 							<< endl
	     << "|_| |     __/"<<char(92)<<"__     | |_|            ==================" 			<< endl
	     << "    |    (_|__|_)    |            1.  Veure configuració" 							<< endl
	     << "    |________________|            2.  Canviar configuració" 						<< endl
	     << " _________|____|_________         3.  Veure estadístiques" 						<< endl
	     << "|                        |        4.  Reproduir última partida" 					<< endl
         << "|  Joaquim Pascual Egea  |        5.  Mostrar top 10" 								<< endl
         << "|    Lucas Cerdà Insa    |        6.  Jugar partida" 								<< endl
		 << "|________________________|" 														<< endl
		 << "                                  0.  EIXIR\n\n" 									<< endl;

	return;
}

/**********************************************************
Entrada: 
Eixida: Animació
Objectiu: Mostra per pantalla a Mr. Roboto pensant
***********************************************************/
void pensant()
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, 14);
	cout << "\n                                           _____________________"				<< endl
	     << "         ·      ·                         |                     |" 				<< endl
		 << "     _____"<<char(92)<<"____/_____               __    |   Estic pensant...  |"	<< endl
		 << "    |                |             |  |   |_____________________|"					<< endl
		 << " _  |    __    __    |  _     _    |__|" 											<< endl
		 << "| | |___|  |__|  |___| | |   |_|" 													<< endl
	     << "| |=|   |__|  |__|   |=| |" 														<< endl
	     << "|_| |     __/"<<char(92)<<"__     | |_|" 											<< endl
	     << "    |    (_|__|_)    |" 															<< endl
	     << "    |________________|" 															<< endl
	     << " _________|____|_________ " 														<< endl
	     << "|                        |"														<< endl
         << "|       MR. ROBOTO       | " 														<< endl
		 << "|________________________|" 														<< endl;
	SetConsoleTextAttribute(hConsole, 15);

	return;
}

/**********************************************************
Entrada: 
Eixida: Animació
Objectiu: Mostra per pantalla el resultat de la partida
***********************************************************/
void perdre()
{
	cout << "\n                                           _____________________"				<< endl
	     << "         ·      ·                         |     He guanyat!     |" 				<< endl
		 << "     _____"<<char(92)<<"____/_____               __    |   Sóc un 'màquina'  |"	<< endl
		 << "    |                |             |  |   |______ jeje... ______|"					<< endl
		 << " _  |    __    __    |  _     _    |__|" 											<< endl
		 << "| | |___|  |__|  |___| | |   |_|" 													<< endl
	     << "| |=|   |__|  |__|   |=| |" 														<< endl
	     << "|_| |     __/"<<char(92)<<"__     | |_|" 											<< endl
	     << "    |    (_|__|_)    |" 															<< endl
	     << "    |________________|" 															<< endl
	     << " _________|____|_________ " 														<< endl
	     << "|                        |"														<< endl
         << "|       MR. ROBOTO       | " 														<< endl
		 << "|________________________|" 														<< endl
		 << "\n\n" 																				<< endl;

	return;
}

/**********************************************************
Entrada: 
Eixida: Animació
Objectiu: Mostra per pantalla el resultat de la partida
***********************************************************/
void guanyar()
{
	cout << "\n                                           _____________________"				<< endl
	     << "         ·      ·                         |     He perdut...    |" 				<< endl
		 << "     _____"<<char(92)<<"____/_____               __    |  Ets un bon rival!  |"	<< endl
		 << "    |                |             |  |   |____ENHORABONA ; )___|"					<< endl
		 << " _  |    __    __    |  _     _    |__|" 											<< endl
		 << "| | |___|  |__|  |___| | |   |_|" 													<< endl
	     << "| |=|   |__|  |__|   |=| |" 														<< endl
	     << "|_| |     __/"<<char(92)<<"__     | |_|" 											<< endl
	     << "    |    (_|__|_)    |" 															<< endl
	     << "    |________________|" 															<< endl
	     << " _________|____|_________ " 														<< endl
	     << "|                        |"														<< endl
         << "|       MR. ROBOTO       | " 														<< endl
		 << "|________________________|" 														<< endl
		 << "\n\n" 																				<< endl;

	return;
}

/**********************************************************
Entrada: 
Eixida: Animació
Objectiu: Mostra per pantalla el resultat de la partida
***********************************************************/
void empat()
{
	cout << "\n                                           _____________________"				<< endl
	     << "         ·      ·                         |    Hem empatat...   |" 				<< endl
		 << "     _____"<<char(92)<<"____/_____               __    |     Juguem una      |"	<< endl
		 << "    |                |             |  |   |___altra partida?____|"					<< endl
		 << " _  |    __    __    |  _     _    |__|" 											<< endl
		 << "| | |___|  |__|  |___| | |   |_|" 													<< endl
	     << "| |=|   |__|  |__|   |=| |" 														<< endl
	     << "|_| |     __/"<<char(92)<<"__     | |_|" 											<< endl
	     << "    |    (_|__|_)    |" 															<< endl
	     << "    |________________|" 															<< endl
	     << " _________|____|_________ " 														<< endl
	     << "|                        |"														<< endl
         << "|       MR. ROBOTO       | " 														<< endl
		 << "|________________________|" 														<< endl
		 << "\n\n" 																				<< endl;

	return;
}