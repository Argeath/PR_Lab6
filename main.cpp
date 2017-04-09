#include <winsock2.h>
#include <iostream>

int mapa[10][10];

void umiescStatek(int size) {
	char buf[3];
	printf("Umiesc statek o rozmiarze %d (A0-J9): ", size);
	std::cin >> buf;

	if (buf[0] >= 'A' && buf[0] <= 'J') {
		int kolumna = buf[0] - 'A';

		if (buf[1] >= '0' && buf[1] <= '9' - (size - 1)) {
			int rzad = buf[1] - '0';

			for (int s = 0; s < size; s++) {
				if (rzad + s < 9)
					if(mapa[rzad + s][kolumna] == 0)
						mapa[rzad + s][kolumna] = 1;
					else {
						printf("Na tym polu juz jest inny statek\n");
						umiescStatek(size); // ponownie
						return;
					}
				else {
					printf("Statek wychodzi poza mape\n");
					umiescStatek(size); // ponownie
					return;
				}
			}
		}
		else {
			printf("Bledny rzad\n");
			umiescStatek(size); // ponownie
			return;
		}
	}
	else {
		printf("Bledna kolumna\n");
		umiescStatek(size); // ponownie
		return;
	}
}

void umiescStatki() {
	umiescStatek(3);
	umiescStatek(2);
	umiescStatek(1);
}

void atak(SOCKET s)
{
	char buf[3];
	printf("Jakie pole chcesz zaatakowac? ");
	std::cin >> buf;
	if (buf[0] >= 'A' && buf[0] <= 'J') {
		int kolumna = buf[0] - 'A';

		if (buf[1] >= '0' && buf[1] <= '9') {
			int rzad = buf[1] - '0';

			char msg[7] = "ATK ";
			strcat_s(msg, buf);

			send(s, msg, 7, 0);
		}
		else {
			printf("Bledny rzad\n");
			atak(s); // ponownie
		}
	}
	else {
		printf("Bledna kolumna\n");
		atak(s); // ponownie
	}
}

bool sprawdzKoniec()
{
	bool koniec = true;

	for (int y = 0; y < 10; y++)
		for (int x = 0; x < 10; x++)
			if (mapa[y][x] == 1) {
				koniec = false;
				break;
			}

	return koniec;
}

void obrona(SOCKET s, char* a)
{
	// a = ATK A6
	int kolumna = a[0] - 'A';
	int rzad = a[1] - '0';

	bool trafiony = mapa[rzad][kolumna] == 1;

	if(trafiony)
	{
		printf("Nasz statek zostal trafiony na polu %s.\n", a);
		mapa[rzad][kolumna] = 0;

		if(sprawdzKoniec())
		{
			send(s, "KONIEC", 7, 0);
			closesocket(s);
			WSACleanup();
			exit(0);
			return;
		}
	} else
	{
		printf("Przeciwnik spudlowal na polu %s.\n", a);
	}
	char msg[6] = "RES ";
	strcat_s(msg, trafiony ? "1" : "0");
	send(s, msg, 6, 0);
}

bool checkPrefix(char* a, char* b, int n)
{
	bool same = true;
	for(int i = 0; i < n; i++)
	{
		if (a[i] != b[i]) {
			same = false;
			break;
		}
	}
	return same;
}

int petla(SOCKET s)
{
	char buf[80];
	while (recv(s, buf, 80, 0) > 0) {
		if(checkPrefix(buf, "START", 5)) {
			atak(s);
		}
		else if (checkPrefix(buf, "ATK", 3)) {
			obrona(s, &buf[4]); // pomiń ATK
			atak(s);
		}
		else if (checkPrefix(buf, "RES", 3)) {
			if (buf[4] == '1')
				printf("TRAFIONY!\n");
			else
				printf("PUDLO!\n");

			printf("Tura przeciwnika... ");
		}
		else if (strcmp(buf, "KONIEC") == 0) {
			closesocket(s);
			WSACleanup();
			return 0;
		}
	}
}


int main(int argc, char* argv) {
	// Must be done at the beginning of every WinSock program
	WSADATA w;	// used to store information about WinSock version
	int error = WSAStartup(0x0202, &w);   // Fill in w

	if (error)
	{ // there was an error
		return 0;
	}
	if (w.wVersion != 0x0202)
	{ // wrong WinSock version!
		WSACleanup(); // unload ws2_32.dll
		return 0;
	}

	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddr_in localAddress;
	localAddress.sin_family = AF_INET;
	localAddress.sin_port = htons(10050);
	localAddress.sin_addr.s_addr = INADDR_ANY;

	sockaddr_in sa;
	sa.sin_family = AF_INET;
	sa.sin_port = htons(10050);
	sa.sin_addr.s_addr = inet_addr("127.0.0.1");

	int result = connect(s, reinterpret_cast<struct sockaddr FAR *>(&sa), sizeof(sa));
	if (result == SOCKET_ERROR)
	{
		printf("\nBlad polaczenia! Tworzenie serwera...\n");

		// SERVER
		s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (bind(s, (LPSOCKADDR)&localAddress, sizeof(localAddress)) == SOCKET_ERROR) {
			printf("Unable to bind.\n");
			WSACleanup();
			return 0;
		}

		if (listen(s, 1) == SOCKET_ERROR) {
			printf("Unable to listen.\n");
			WSACleanup();
			return 0;
		}

		int lenc = sizeof(sa);
		printf("Oczekiwanie na klienta...\n");
		SOCKET socket = accept(s, reinterpret_cast<struct sockaddr FAR *>(&sa), &lenc);
		printf("Klient sie polaczyl.\n");

		umiescStatki();

		printf("Twoja mapa:\n");
		for (int x = 0; x < 10; x++)
		{
			for (int y = 0; y < 10; y++)
				printf("%d ", mapa[x][y]);
			printf("\n");
		}

		send(socket, "START", 6, 0);

		printf("Tura przeciwnika... ");

		return petla(socket);
	}

	// klient
	printf("Polaczono na serwer.\n");

	umiescStatki();

	printf("Twoja mapa:\n");
	for(int x = 0; x < 10; x++)
	{
		for (int y = 0; y < 10; y++)
			printf("%d ", mapa[x][y]);
		printf("\n");
	}
	printf("Poczekaj az gracz ustawi statki...\n");

	return petla(s);
}


/*
Server: umiesc 1 statek
Klient: umiesc 1 statek
Server: umie
*/
