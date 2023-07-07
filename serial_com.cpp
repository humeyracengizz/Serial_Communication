#include<windows.h>
#include<iostream>

std::string GetErrorFromUART(HANDLE hSerial)
{
    std::string error;
    char buffer;
    DWORD bytesRead;

    while (true)
    {
        if (!ReadFile(hSerial, &buffer, sizeof(buffer), &bytesRead, NULL))
        {
            std::cerr << "Hata mesaji alinamadi" << std::endl;
            return 0;
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
        std::cerr << "Seri port acma hatasi!" << std::endl;
        return 1;
    }

    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (!GetCommState(hSerial, &dcbSerialParams))
    {
        std::cerr << "iletisim gerceklesmedi" << std::endl;
        CloseHandle(hSerial);
        return 1;
    }

    dcbSerialParams.BaudRate = CBR_9600;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.Parity = NOPARITY;
    dcbSerialParams.StopBits = ONESTOPBIT;
    
    if (!SetCommState(hSerial, &dcbSerialParams))
    {
        std::cerr << "İletisim durumu ayarlanamadi!" << std::endl;
        CloseHandle(hSerial);
        return 1;
    }
    
    char data[] = "M600000000\r"; // gönderilecek argümanlar
    char database;
    std::cin>> database;
    DWORD bytesWritten;
    if (!WriteFile(hSerial, data, sizeof(data) - 1, &bytesWritten, NULL))
    {
        std::cerr << "Mesaj gönderilemedi!" << std::endl;
        CloseHandle(hSerial);
        return 1;
    }

    std::string uartError = GetErrorFromUART(hSerial);

    if (!uartError.empty())
    {
        std::cout << "Hata mesaji: " << uartError << std::endl;
    }
    else
    {
        std::cout << "Hata mesaji alinamadi" << std::endl;
    }
    
    CloseHandle(hSerial);
    return 0;
}
