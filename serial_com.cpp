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
#include<openssl/evp.h>


const unsigned char key[] = { 0x64, 0xF5, 0xD9, 0x2F, 0x1C, 0x00, 0x3A, 0x00, 0x64, 0xF5, 0xD9, 0x2F, 0x1C, 0x00, 0x3A, 0x00 };


std::string EncryptAES(const std::string& plainText)  
{
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx)
    {
        std::cout << "Failed to create cipher context." << std::endl;
        return "";
    }

    if (EVP_EncryptInit_ex(ctx, EVP_aes_128_ecb(), nullptr, key, nullptr) != 1)
    {
        std::cout << "Failed to initialize cipher." << std::endl;
        EVP_CIPHER_CTX_free(ctx);
        return "";
    }

    int encryptedLength = 0;
    int finalLength = 0;

    // Şifreleme için gerekli belleği elde etme
    int maxOutputLength = plainText.size() + EVP_CIPHER_CTX_block_size(ctx);
    std::string encryptedText(maxOutputLength, '\0');

    if (EVP_EncryptUpdate(ctx, reinterpret_cast<unsigned char*>(&encryptedText[0]), &encryptedLength, reinterpret_cast<const unsigned char*>(plainText.c_str()), plainText.size()) != 1)
    {
        std::cout << "Failed to encrypt data." << std::endl;
        EVP_CIPHER_CTX_free(ctx);
        return "";
    }

    if (EVP_EncryptFinal_ex(ctx, reinterpret_cast<unsigned char*>(&encryptedText[encryptedLength]), &finalLength) != 1)
    {
        std::cout << "Failed to finalize encryption." << std::endl;
        EVP_CIPHER_CTX_free(ctx);
        return "";
    }

    encryptedText.resize(encryptedLength + finalLength);
    EVP_CIPHER_CTX_free(ctx);

    return encryptedText;
}

int main() {
    const char* portPath = "/dev/ttyUSB0";
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

    if (command[0] == 'S' || command[0] == 'C')
        {
            
            std::cout << "encrypted" << std::endl;
            std::string commandString = command.substr(1);
            std::string encryptedCommand = EncryptAES(commandString);

            // Enkripte edilen komutun başına komutun türünü (S veya C) ekleyin
            command = command[0] + encryptedCommand;
        }
        else
        {
            std::cout << "not encrypted" << std::endl;
            command = command;
        }
        
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
