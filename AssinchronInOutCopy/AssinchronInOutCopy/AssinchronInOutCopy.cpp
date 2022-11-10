#pragma comment ( lib, "Winmm.Lib")
#include <iostream>
#include <stdio.h>
#include <windows.h>
#include <string>
#include <fstream>
#include <timeapi.h>

using namespace std;
/*Блок функций и переменных для более точного таймера и логов*/
/*Использовались для логирования операций, результат с точностью до 0.1 мкс*/
void StartCounter();
double GetCounter();
double PCFreq;
LARGE_INTEGER CounterStart;
ofstream logger;
time_t now;
char time_as_string[30];
/*Вспомогательная функция на проверку корректности читаемых чисел*/
int InputAsNumber();
/*Счётчик возвратов функций завершения и необходимое количество операций в целом*/
int callback, neededCallback;
/*Объявим handle файлов в общем пространстве видимости*/
HANDLE ExistedFile;
HANDLE NewCopyFile;
/*Зададим "каретку" для перемещение по файлам*/
LARGE_INTEGER carriage;
/*Зададим независимо два вида завершающих функций*/
//Запускается после завершения чтения
VOID CALLBACK ReadStartWrite(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped); 
//Зпускается после завершения записи
VOID CALLBACK WriteStartRead(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped);
/*Обявление переменных кол-ва операций и размеров блока*/
int operationNum, sizeMultiplier;
int copySize;
/*Объявления структур данных OVERLAPPED для обоих файлов*/
LPOVERLAPPED overlapExist;
LPOVERLAPPED overlapCopy;
/*Обявление буфера для чтения/записи*/
CHAR** buffer;
/*Переменная содержащая размер файла*/
LARGE_INTEGER fileSize;

int main()
{
    LPTSTR user_inputFile = new wchar_t[MAX_PATH]; //Путь до существующего файла
    LPTSTR user_copyFile = new wchar_t[MAX_PATH]; //Путь до нового файла куда будет осуществлятся копирование
    DWORD start_time, end_time; //счётчики замера времени timeGetTime()
    ofstream result; //файловый вывод результатов в CSV файл для обработки
    int ex = 1; //флаг выхода
    //Файл логирования (Для вывода информации об откликах операции)
    logger.open("logger.txt"); //Открытие файла лог. Закомментировать
    do {
        system("cls");
        callback = 0;
        /*-------------------------------------*/
        /*Процедуры считывания исходных данных*/
        /*-------------------------------------*/
        SetLastError(0);
        cout << "This is a program that implements copying using overlapping I/O operations.\n";
        //Считывание существующего файла для копирования
        cout << "Please, enter filepath to existing file to copy:\n";
        do {
            if (GetLastError()) cout << "File with this path and name doesn't exist!\n"; //Если ввод получился неверным
            cout << "> ";
            wcin.getline(user_inputFile, MAX_PATH);
        } while ((ExistedFile = CreateFile( //Открытие файла с указаными ниже параметрами...
            user_inputFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED, NULL))
            == INVALID_HANDLE_VALUE); //...пытаться до тех пор пока не будет обнаружен корректный файл.
        SetLastError(0); // Ошибки ввода обработаны, переход к следующему блоку считывания
        cout << "Please, enter filepath for new copyed file:\n";
        do {
            if (GetLastError()) cout << "Can't create file with this name or filepath!\n";
            cout << "> ";
            wcin.getline(user_copyFile, MAX_PATH);
        } while ((NewCopyFile = CreateFile( //Попытка сздать файл для копирования с параметрами ниже...
            user_copyFile, GENERIC_WRITE | GENERIC_READ, 0, NULL, CREATE_ALWAYS, FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED, NULL))
            == INVALID_HANDLE_VALUE); //...до тех пор пока попытка не будет удовлетворена.
        SetLastError(0); // Ошибки ввода обработаны.
        cout << "Enter count of overlapping operation:\n"; //Выберем количество перекрывающих операций в/в
        operationNum = InputAsNumber();
        cout << "Enter multiplier for memory quantity (n*4096 Byte):\n"; //Выберем область памяти кратную 4 Кб.
        sizeMultiplier = InputAsNumber();
        copySize = 4096 * sizeMultiplier; //Определение объёма блока копируемой за раз памяти
        /*-----------------------------------------------------------------------------------*/
        /*Все необхдимые данные инициализованы. Можно выделять память для буферов и структур*/
        /*----------------------------------------------------------------------------------*/
        buffer = new CHAR * [operationNum];        //  ---
        for (int i = 0; i < operationNum; i++) { // - Выделение памяти под буфер для каждой операции
            buffer[i] = new CHAR[copySize];      //  ---
        }
        overlapExist = new OVERLAPPED[operationNum]; // Инициализация для каждой операции
        overlapCopy = new OVERLAPPED[operationNum];  // Структур OVERLAPPED читаемого и записываемого файла

        carriage.QuadPart = 0; // Установка "каретки" в начало для чтения файла

        GetFileSizeEx(ExistedFile, &fileSize); // Получение размера файла, является одним из критериев верноости
        neededCallback = (fileSize.QuadPart + copySize - 1) / copySize; // Целочисленное деление с округлением вверх (количество операций ввод-вывод)
        /*----------------------------------------------------------------------------------*/
        /*Запуск операций чтения в цикле, подготовка объектов OVERLAPPED. Старт копирования.*/
        /*----------------------------------------------------------------------------------*/
        
        start_time = timeGetTime();
        StartCounter(); //начало отсчёта для логирования
        now = time(0); //Для логов
        ctime_s(time_as_string, 26, &now); //Для логов
        logger << time_as_string << GetCounter() << " ms: Start copy\n"; //Вывод лога
        for (int i = 0; i < operationNum; i++) {
            overlapExist[i].hEvent = (HANDLE)i; //При использовании <>Ex операций этот параметр игнорируется функцией поэтому мы можем
            overlapCopy[i].hEvent = (HANDLE)i;  //Использовать это поле в своих целях, как поле означающее номер операции (возможно обр. преобр в DWORD)
            overlapExist[i].Offset = carriage.LowPart;      // Обозначение границ откуда начинается чтение
            overlapExist[i].OffsetHigh = carriage.HighPart; // файла. Обновляется кареткой.
            /*OVERLAPPED конструкция настроена, запуск операции чтения*/
            if (carriage.QuadPart < fileSize.QuadPart) { //Если мы всё ещё находимся в пределах файла...
                ReadFileEx(ExistedFile, buffer[i], copySize, &overlapExist[i], ReadStartWrite); //...То читаем кусочек файла.
                /*По окончании запускается функция выхода ReadStartWrite(),
                инициирующая этой операцией запись в копируемый файл (см. комм. к ф-ции)*/
            }
            carriage.QuadPart += (LONGLONG)copySize; //Сдвигаем каретку на следующий блок файла.
        }
        /*---------------------------------------------------------------------------------------*/
        /*Теперь операторы чтения и записи будут с помощью косвенной рекурсии вызывать друг друга*/
        /*В функциях завершения соответствующей операции. При этом каждая операция не зависит    */
        /*От выполнения друг друга, т.е. если одина из завершила чтение раньше, она не ожидая    */
        /*Сможет начать запись со своей стороны в свою часть памяти*/
        /*---------------------------------------------------------------------------------------*/
        while (callback < 2 * neededCallback) { //Мы заведомо подсчитали, сколько пердполагается операций. коэфф. "2" на две(!) операции: in и out
            SleepEx(INFINITE, true); //Приостанавливаем поток до тех пор, пока какая-либо операция не завершится.
        }

        /*В конце концов копирование будет завершено. Необходимо установить конец нового файла*/
        SetFilePointerEx(NewCopyFile, fileSize, NULL, FILE_BEGIN); //Установка указателя на предполагаемый конец файла...
        SetEndOfFile(NewCopyFile); //... И установка по этому указателю eof.
        now = time(0); //...Логи...
        ctime_s(time_as_string, 26, &now); //...Логи...
        logger << time_as_string << GetCounter() << " ms: End copy\n"; //...Логи...
        cout << "Copying completed successfully!" << endl; 
        //Закрываем открытые файлы
        CloseHandle(ExistedFile);
        CloseHandle(NewCopyFile);

        end_time = timeGetTime();
        cout << "Copy execution time is " << end_time - start_time << " ms.\n"; //With tick counter: " << GetCounter();
        /*Запись результатов в CSV файл для дальнейшей обработки*/
        result.open("results_operation_big.txt", ios::app); 
        result << operationNum << ";" << sizeMultiplier << ";" << end_time - start_time << endl;
        result.close();

        /*-----------------------------*/
        /*Очищаем всю выделенную память*/
        /*-----------------------------*/
        for (int i = 0; i < operationNum; i++) {
            delete buffer[i];
        }
        delete[] buffer;
        delete[] overlapCopy;
        delete[] overlapExist;
        cout << "Do you want try again? 1 - YES, 0 - EXIT\n>";
        ex = InputAsNumber();
    } while (ex != 0);
    
    delete[] user_inputFile;
    delete[] user_copyFile;
    logger.close();
    return 0;
}

int InputAsNumber() {
    string user;
    int in = -1;
    bool flag = true;
    do {
        cout << "> ";
        getline(cin, user);
        try {
            in = stoi(user);
            flag = true;
        }
        catch (const std::invalid_argument) {
            flag = false;
            cout << "Invalid format of command! Expected natural number" << endl;
        }

    } while (!flag);
    return in;
}
/*Функция завершения оператора чтения. Обрабатывает считанные данные и инициирует операцию записи*/
/*Переменные файлов,а так же буферов доступны в этой функции в силу глобального пространства видимости*/
VOID CALLBACK ReadStartWrite(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped) {
    ++callback; //Конец оператора, увеличение счётчика

    LARGE_INTEGER carriageIn, carriageCopy; //Каретки чтения и записи
    /*Применение неиспользоваемого функциями <>Ex поля*/
    DWORD i = (DWORD)lpOverlapped->hEvent; //Восстановление номера операции

    now = time(0);
    ctime_s(time_as_string, 26, &now);
    logger << time_as_string << GetCounter() << " ms: Operation num [" << i << "] end read. Start write...\n";

    /*Восстанавливаем каретки на файлы*/
    carriageIn.LowPart = lpOverlapped->Offset;      //  ---
    carriageIn.HighPart = lpOverlapped->OffsetHigh; // - Получение каретки чтения и установка в соотв область каретки записи.
    carriageCopy.QuadPart = carriageIn.QuadPart;    //  ---
    overlapCopy[i].Offset = carriageCopy.LowPart;    
    overlapCopy[i].OffsetHigh = carriageCopy.HighPart; //Настройка объекта OVERLAPPED для записи
    /*Каретка и объект настроены, можно начинать операцию записи*/
    WriteFileEx(NewCopyFile, buffer[i], copySize, &overlapCopy[i], WriteStartRead); //Вызывает функцию завершения для новог цикла чтения-запись этой операции
    /*Настройка объекта OVERLAPPED на чтение следующего фрагмента файла*/
    carriageIn.QuadPart += copySize * (LONGLONG)operationNum;
    overlapExist[i].Offset = carriageIn.LowPart;
    overlapExist[i].OffsetHigh = carriageIn.HighPart;
    return;
};
/*Функция завершения оператора записи. Инициирует следующий цикл чтения для этого перекрытия*/
VOID CALLBACK WriteStartRead(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped) {
    ++callback; //Конец оператора и операции в целом. Увеличение счётчика.

    /*В функции завершения чтения уже предусмотрен перенос в следующую область памяти*/
    /*Достаточно восстановить эти данные*/
    LARGE_INTEGER carrietIn;
    DWORD i = (DWORD)lpOverlapped->hEvent; //Восстановление номера операции
    
    now = time(0);
    ctime_s(time_as_string, 26, &now);
    logger << time_as_string << GetCounter() << " ms: Operation num [" << i << "] end write. Start read...\n";

    carrietIn.LowPart = overlapExist[i].Offset;      // Настройка объекта OVERLAPPED
    carrietIn.HighPart = overlapExist[i].OffsetHigh; // для следующего чтения
    if (carrietIn.QuadPart < fileSize.QuadPart) { //Если мы не вышли за пределы читаемого файла
        ReadFileEx(ExistedFile, buffer[i], copySize, &overlapExist[i], ReadStartWrite); //Мы продолжаем записывать пока имеется возможность
    }
    return;
}

void StartCounter()
{
    LARGE_INTEGER li;
    if (!QueryPerformanceFrequency(&li))
        cout << "QueryPerformanceFrequency failed!\n";

    PCFreq = double(li.QuadPart) / 1000.0;

    QueryPerformanceCounter(&li);
    CounterStart.QuadPart = li.QuadPart;
}
double GetCounter()
{
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return double(li.QuadPart - CounterStart.QuadPart) / PCFreq;
}