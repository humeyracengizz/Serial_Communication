#include <windows.h>
#include <iostream>
#include <string>

using namespace std;

string GetErrorFromUART(HANDLE hSerial)
{
    string error;
    char buffer;
    DWORD bytesRead;

    while (true)
    {
        if (!ReadFile(hSerial, &buffer, sizeof(buffer), &bytesRead, NULL))
        {
            cerr << "Hata mesaji alinamadi" << endl;
            return "";
        }

        if (bytesRead > 0)
        {
            error += buffer;
        }
        if (buffer == '\r')
        {
            break;
        }
    }
    return error;
}

int main()
{
    HANDLE hSerial;
    DCB dcbSerialParams;

    hSerial = CreateFileA("COM4", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hSerial == INVALID_HANDLE_VALUE)
    {
        cerr << "Seri port acma hatasi!" << endl;
        return 1;
    }

    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

    if (!GetCommState(hSerial, &dcbSerialParams))
    {
        cerr << "iletisim gerceklesmedi" << endl;
        CloseHandle(hSerial);
        return 1;
    }

    dcbSerialParams.BaudRate = CBR_9600;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.Parity = NOPARITY;
    dcbSerialParams.StopBits = ONESTOPBIT;

    if (!SetCommState(hSerial, &dcbSerialParams))
    {
        cerr << "İletisim Parametreleri Dogru Degil" << endl;
        CloseHandle(hSerial);
        return 1;
    }

    string userInput;
    cout << "Gonderilecek sayiyi girin: ";
    getline(cin, userInput);

    string data = userInput + "\r";
    DWORD bytesWritten;
    if (!WriteFile(hSerial, data.c_str(), data.length(), &bytesWritten, NULL))
    {
        cerr << "Mesaj gönderilemedi" << endl;
        CloseHandle(hSerial);
        return 1;
    }

    string uartError = GetErrorFromUART(hSerial);

    if (!uartError.empty())
    {
        cout << "Hata mesaji: " << uartError << endl;
    }
    else
    {
        cout << "Hata mesaji alinamadi" << endl;
    }

    CloseHandle(hSerial);
    return 0;
}

---------------------------------------------------------------UBUNTU------------------------------------------------------

#include <iostream>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <cstring>

int main() {
    const char* portPath = "/dev/ttyUSB2";
    struct termios serialParams;
    int serialPort = open(portPath, O_RDWR | O_NOCTTY);

    if (serialPort == -1) {
        std::cerr << "Seri port açilamadi!" << std::endl;
        return 1;
    }

    struct termios tty;
    if (tcgetattr(serialPort, &tty) != 0) {
        std::cerr << "Seri port ayarlari alinamadi!" << std::endl;
        return 1;
    }

    cfsetispeed(&tty, B9600);
    cfsetospeed(&tty, B9600);
    tty.c_cflag |= CS8;
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);

    if (tcsetattr(serialPort, TCSANOW, &tty) != 0) {
        std::cerr << "Seri port ayarlari uygulanamadi!" << std::endl;
        return 1;
    }

    std::string command;
    std::cout << "Cihaza gönderilecek komutu girin: ";
    std::getline(std::cin, command);

    command += '\r';

    int bytesWritten = write(serialPort, command.c_str(), command.length());
    if (bytesWritten < 0) {
        std::cerr << "Seri porta yazma hatasi!" << std::endl;
        return 1;
    } else {
        std::cout << "Komut seri porta gönderildi." << std::endl;
    }

    tcflush(serialPort, TCIOFLUSH);
    usleep(100000);
    tcdrain(serialPort);

    char readBuffer[1024];
    memset(readBuffer, 0, sizeof(readBuffer));
    int totalBytesRead = 0;
    int bytesRead = read(serialPort, readBuffer + totalBytesRead, sizeof(readBuffer) - 1 - totalBytesRead);
    
    while (totalBytesRead < sizeof(readBuffer) - 1) {

        if (bytesRead <= 0) {
            std::cerr << "Seri porttan okuma hatasi!" << std::endl;
            return 1;
        }
        totalBytesRead += bytesRead;

        if (readBuffer[totalBytesRead - 1] == '\r') {
            break;
        }
    }

    readBuffer[totalBytesRead] = '\0';

    std::cout << "Seri porttan gelen yanit: " << readBuffer << std::endl;

    close(serialPort);
    return 0;
}

