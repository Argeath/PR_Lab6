#include <winsock2.h>
#include <iostream>

int mapa[10][10];

void umiescStatek(int size) {
	char buf[2];
	printf("Umiesc statek o rozmiarze %d: ", size);
	std::cin >> buf;

	if (buf[0] >= 'A' && buf[0] <= 'J') {
		int rzad = buf[0] - 'A';
		if (buf[1] >= '0' && buf[1] <= '9') {
			int kolumna = buf[1] - '0';
		} else {
			printf("Bledna kolumna\n");
			umiescStatek(size); // ponownie
		}
	}
	else {
		printf("Bledny rzad\n");
		umiescStatek(size); // ponownie
	}
}

void umiescStatki() {
	umiescStatek(3);
	umiescStatek(2);
	umiescStatek(1);
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
	localAddress.sin_port = htons(10000);
	localAddress.sin_addr.s_addr = INADDR_ANY;

	sockaddr_in sa;
	sa.sin_family = AF_INET;
	sa.sin_port = htons(10000);
	sa.sin_addr.s_addr = inet_addr("127.0.0.1");

	int result = connect(s, (struct sockaddr FAR *) &sa, sizeof(sa));
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

		SOCKET sa;
		int lenc;

		for (;;) {
			lenc = sizeof(sa);
			printf("Oczekiwanie na klienta...\n");
			sa = accept(s, (struct sockaddr FAR *) &sa, &lenc);
			printf("Klient sie polaczyl.\n");

			char buf[80];

			while (recv(sa, buf, 80, 0) > 0) {
				if (strcmp(buf, "CLR") == 0) {
					system("cls");
					send(sa, "Cleared.", 9, 0);
				} else if (strcmp(buf, "KONIEC") == 0) {
					closesocket(sa);
					closesocket(s);
					WSACleanup();
					return 0;
				} else {
					printf("\n%s", buf);
					send(sa, "s", 2, 0);
				}
			}
		}
		return 0;
	}

	// klient
	int dlug;
	char buf[80];

	printf("Polaczono na serwer.\n");

	for (;;) {
		fgets(buf, 80, stdin);
		dlug = strlen(buf); buf[dlug - 1] = '\0';

		send(s, buf, dlug, 0);

		if (strcmp(buf, "KONIEC") == 0) break;

		if (recv(s, buf, 80, 0) > 0) {
			printf("\n%s", buf);
		}
	}

	closesocket(s);
	WSACleanup();
	return 0;
}


/*
Server: umiesc 1 statek
Klient: umiesc 1 statek
Server: umie


*/
