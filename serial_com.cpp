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
