#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

// Функція для виведення результату bind()
void checkBindResult(SOCKET sock, sockaddr_in& addr, const string& title) {
    cout << title;

    if (bind(sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        cout << "   [ПОМИЛКА] Код: " << WSAGetLastError() << endl;
    }
    else {
        cout << "   [OK] Прив'язка успішна\n";
    }
}

int main() {

    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    const int lastTwoDigits = 17;
    const int lastThreeDigits = 117;

    cout << "========== ІНІЦІАЛІЗАЦІЯ WinSock ==========\n";

    WSADATA wsData;

    int startResult = WSAStartup(MAKEWORD(2, 2), &wsData);

    if (startResult != 0) {
        cout << "Не вдалося ініціалізувати WinSock. Код: "
             << startResult << endl;
        return 1;
    }

    cout << "Версія WinSock: "
         << (int)HIBYTE(wsData.wVersion)
         << "."
         << (int)LOBYTE(wsData.wVersion)
         << endl;

    cout << "Опис: " << wsData.szDescription << endl;
    cout << "Статус: " << wsData.szSystemStatus << endl;

    cout << "\n========== СТВОРЕННЯ СОКЕТІВ ==========\n";

    SOCKET udpSocket1 = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    SOCKET udpSocket2 = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    SOCKET tcpSocket1 = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    SOCKET tcpSocket2 = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (udpSocket1 == INVALID_SOCKET ||
        udpSocket2 == INVALID_SOCKET ||
        tcpSocket1 == INVALID_SOCKET ||
        tcpSocket2 == INVALID_SOCKET) {

        cout << "Помилка створення сокетів! Код: "
             << WSAGetLastError() << endl;

        WSACleanup();
        return 1;
    }

    // Розмір буфера
    setsockopt(
        udpSocket1,
        SOL_SOCKET,
        SO_SNDBUF,
        (char*)&lastThreeDigits,
        sizeof(lastThreeDigits)
    );

    setsockopt(
        udpSocket2,
        SOL_SOCKET,
        SO_SNDBUF,
        (char*)&lastThreeDigits,
        sizeof(lastThreeDigits)
    );

    // Дозвіл Broadcast
    BOOL allowBroadcast = TRUE;

    setsockopt(
        udpSocket1,
        SOL_SOCKET,
        SO_BROADCAST,
        (char*)&allowBroadcast,
        sizeof(allowBroadcast)
    );

    cout << "UDP Socket 1: " << udpSocket1 << endl;
    cout << "UDP Socket 2: " << udpSocket2 << endl;
    cout << "TCP Socket 1: " << tcpSocket1 << endl;
    cout << "TCP Socket 2: " << tcpSocket2 << endl;

    cout << "\n========== НАЛАШТУВАННЯ АДРЕС ==========\n";

    // --- UDP Broadcast socket ---
    sockaddr_in address1{};
    address1.sin_family = AF_INET;
    address1.sin_port = htons(5001);
    address1.sin_addr.s_addr = INADDR_ANY;

    checkBindResult(
        udpSocket1,
        address1,
        "1) udpSocket1 -> INADDR_ANY :5001\n"
    );

    // --- UDP fixed IP ---
    char ipAddress2[20];
    sprintf_s(ipAddress2, "10.1.2.1%d", lastTwoDigits);

    sockaddr_in address2{};
    address2.sin_family = AF_INET;
    address2.sin_port = htons(5002);

    inet_pton(AF_INET, ipAddress2, &address2.sin_addr);

    cout << "2) udpSocket2 -> " << ipAddress2 << ":5002\n";

    if (bind(udpSocket2, (sockaddr*)&address2, sizeof(address2)) == SOCKET_ERROR) {

        cout << "   [ПОМИЛКА] Така адреса може бути відсутня на ПК. Код: "
             << WSAGetLastError() << endl;
    }
    else {
        cout << "   [OK] Прив'язка успішна\n";
    }

    // --- TCP fixed IP ---
    char ipAddress3[20];
    sprintf_s(ipAddress3, "10.1.2.1%d", lastTwoDigits + 1);

    sockaddr_in address3{};
    address3.sin_family = AF_INET;
    address3.sin_port = htons(5003);

    inet_pton(AF_INET, ipAddress3, &address3.sin_addr);

    checkBindResult(
        tcpSocket1,
        address3,
        string("3) tcpSocket1 -> ") + ipAddress3 + ":5003\n"
    );

    // --- TCP any address ---
    sockaddr_in address4{};
    address4.sin_family = AF_INET;
    address4.sin_port = htons(5004);
    address4.sin_addr.s_addr = INADDR_ANY;

    checkBindResult(
        tcpSocket2,
        address4,
        "4) tcpSocket2 -> INADDR_ANY :5004\n"
    );

    cout << "\n========== ГЕНЕРАЦІЯ ПОМИЛОК bind() ==========\n";

    // 1
    if (bind((SOCKET)123456, (sockaddr*)&address4, sizeof(address4)) == SOCKET_ERROR) {

        cout << "1. WSAENOTSOCK (10038)\n";
        cout << "   Неправильний дескриптор сокета.\n";
        cout << "   Код: " << WSAGetLastError() << endl;
    }

    SOCKET tempSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // 2
    if (bind(tempSocket, (sockaddr*)&address4, 1) == SOCKET_ERROR) {

        cout << "\n2. WSAEFAULT (10014)\n";
        cout << "   Некоректний розмір структури sockaddr.\n";
        cout << "   Код: " << WSAGetLastError() << endl;
    }

    // 3
    if (bind(tcpSocket2, (sockaddr*)&address4, sizeof(address4)) == SOCKET_ERROR) {

        cout << "\n3. WSAEINVAL (10022)\n";
        cout << "   Повторна прив'язка сокета.\n";
        cout << "   Код: " << WSAGetLastError() << endl;
    }

    // 4
    if (bind(tempSocket, (sockaddr*)&address4, sizeof(address4)) == SOCKET_ERROR) {

        cout << "\n4. WSAEADDRINUSE (10048)\n";
        cout << "   Порт вже використовується.\n";
        cout << "   Код: " << WSAGetLastError() << endl;
    }

    // 5
    sockaddr_in wrongAddress = address4;
    wrongAddress.sin_family = AF_IPX;

    if (bind(tempSocket, (sockaddr*)&wrongAddress, sizeof(wrongAddress)) == SOCKET_ERROR) {

        cout << "\n5. WSAEAFNOSUPPORT (10047)\n";
        cout << "   Несумісне сімейство адрес.\n";
        cout << "   Код: " << WSAGetLastError() << endl;
    }

    closesocket(tempSocket);

    cout << "\n========== ЗАВЕРШЕННЯ РОБОТИ ==========\n";

    closesocket(udpSocket1);
    closesocket(udpSocket2);

    closesocket(tcpSocket1);
    closesocket(tcpSocket2);

    cout << "Усі сокети успішно закриті.\n";

    WSACleanup();

    cout << "WinSock очищено.\n";
    cout << "Програму завершено.\n";

    return 0;
}