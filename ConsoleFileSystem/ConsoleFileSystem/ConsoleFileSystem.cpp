#include <iostream>
#include <limits>
#include <cmath>
#include <stdio.h>
#include <windows.h>
#include <string>

using namespace std;
/*Функция очистки потока ввода*/
void cleanInStream();
/*Доп. функция печати атрибутов*/
void printAttributes(DWORD Attributes);
/*Функция печати главного меню*/
void printMainMenu();
/*Опция 1 - Вывод имеющихся дисков*/
void getDisks(LPTSTR Drivers) {
    cout << "Founded Drives on this PC:" << endl;
    for (int i = 0; Drivers[i] || Drivers[i + 1]; i++) {
        if (Drivers[i]) wprintf(L"%c", Drivers[i]);
        if (!Drivers[i+1]) cout << endl;
    }
    cout << endl;
} 
/*Опция 2 - Вывод информации о дисках*/
void getInformation(LPTSTR Drivers) {
    cout << "Please, select one of the Drives:" << endl;
    int k = 0;
    for (int i = 0; Drivers[i] || Drivers[i + 1]; i++) {
        if (Drivers[i]) wprintf(L"%c", Drivers[i]);
        if (!Drivers[i + 1]) cout << " - " << ++k << endl;
    }
    cout << "User>>";
    cleanInStream();
    cin >> k;
    k--;
    LPTSTR strBuff = Drivers + k * sizeof(LPTSTR);
    cout << "Selected Drive: ";
    wprintf(L"%s", strBuff);
    cout << "\nType of selected Drive : ";
    switch (GetDriveType(strBuff)) {
        case 0: 
            cout << "Unnamed error";
            break;
        case DRIVE_REMOVABLE:
            cout << "Removable drive";
            break;
        case DRIVE_FIXED:
            cout << "Fixed drive";
            break;
        case DRIVE_REMOTE:
            cout << "Remote drive";
            break;
        case DRIVE_CDROM:
            cout << "CD-disk";
            break;
    }
    cout << endl;
    LPDWORD nClaster = new DWORD(), nSector = new DWORD(), nByte = new DWORD(), nTotalClaster = new DWORD();
    DWORD VSNumber, Length, FileSF;
    LPWSTR Name = new wchar_t[MAX_PATH], SystemName = new wchar_t[MAX_PATH];
    GetVolumeInformation(strBuff, Name, MAX_PATH, &VSNumber, &Length, &FileSF, SystemName, MAX_PATH);
    cout << "System information" << endl;
    cout << "Tom name: "; wprintf(L"%s", Name);
    cout << "\nFileSystem name: "; wprintf(L"%s", SystemName);
    cout << "\nSerial number: " << VSNumber;
    cout << "\nMax length filename: " << Length;
    cout << "\nSpecific system flags: " << FileSF << endl;

    GetDiskFreeSpace(strBuff, nSector, nByte, nClaster, nTotalClaster);
    cout << "Free " << *nClaster << "/" << *nTotalClaster << " clasters.\nSectors per claster: ";
    cout << *nSector << ".\nBytes per sector: " << *nByte << ".\n\n";

    cout << "Free memory: " << ((((*nClaster) / 1024 * (*nSector)) / 1024) * (*nByte)) / 1024 << " GB" << endl;
    delete nClaster;
    delete nSector;
    delete nByte;
    delete nTotalClaster;
    delete[]Name;
    delete[]SystemName;
}
/*Опция 3 - Создание директории*/
void CreateNewDirectory() {
    LPTSTR user_input = new wchar_t[MAX_PATH];
    LPTSTR Directory = new wchar_t[MAX_PATH];

    cout << "Enter new directory path: \nUser>>";
    cleanInStream();
    wcin.getline(user_input, MAX_PATH);
    lstrcpyW(Directory, user_input);
    if (CreateDirectory(Directory, NULL)) {
        cout << "Directory ";
        wprintf(L"%s", Directory);
        cout << " sucsessfull created\n";
    }
    else {
        if (GetLastError() == ERROR_ALREADY_EXISTS) cout << "Error. This directory already exit.\n";
        if (GetLastError() == ERROR_PATH_NOT_FOUND) cout << "Some intermediat directories don't exit\nAttention! This function create only final directory!\n";
    }
    delete[] user_input;
    delete[] Directory;
}
/*Опция 4 - удаление директории*/
void RemoveCurrentDirectory() {
    LPTSTR user_input = new wchar_t[MAX_PATH];

    cout << "Enter directory path to delete: \nUser>>";
    cleanInStream();
    wcin.getline(user_input, MAX_PATH);
    if (RemoveDirectory(user_input)) {
        cout << "Directory ";
        wprintf(L"%s", user_input);
        cout << " sucsessfull deleted\n";
    }
    else {
        cout << "Delete directory Error!";
        DWORD err = GetLastError();
        if (GetLastError() == ERROR_DIR_NOT_EMPTY) cout << " Directory isn't empty!\n";
        if (GetLastError() == ERROR_PATH_NOT_FOUND) cout << " This directory doesn't exist!\n";
    }
    delete[] user_input;

}
/*Опция 5 - создание файла*/
void CreateNewFile() {
    LPTSTR user_input = new wchar_t[MAX_PATH];
    LPTSTR Directory = new wchar_t[MAX_PATH];
    string user_number;
    int access;
    DWORD readOrWrite;
    HANDLE File;

    cout << "Enter full path to create file: \nUser>>";
    cleanInStream();
    wcin.getline(user_input, MAX_PATH);
    cout << "Choose type of File Access: \n1 - Only Read\n2 - Only Write\n3 - Read and Write\nUser>>";
    cin >> access;
    switch (access) {
        case 1:
            readOrWrite = GENERIC_READ;
            break;
        case 2:
            readOrWrite = GENERIC_WRITE;
            break;
        case 3:
            readOrWrite = GENERIC_READ || GENERIC_WRITE;
            break;
    }
    if ((File = CreateFile(user_input, readOrWrite, 0, 0, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, 0)) != INVALID_HANDLE_VALUE) {
        cout << "File has been created! You can find it in ";
        wprintf(L"%s", user_input);
        cout << endl;
        CloseHandle(File);
    }
    else {
        switch (GetLastError()) {
        case ERROR_PATH_NOT_FOUND:
            cout << "Error. This directory doesn't exist. Firstly, check path and create non-exist directory\n";
            break;
        case ERROR_INVALID_NAME:
            cout << "Incorrect filename. Please, choose this menu item and try again.\n";
            break;
        }
    };

    delete[] user_input;
    delete[] Directory;
}
/*Опция 6 - копирование файла*/
void CopyCurrentFile();
/*Опция 7 - перемещение файла*/
void MoveCurrentFile();
/*Опция 8 - просмотр имеющихся атрибутов файла*/
void AnalyseFile();
/*Опция 9 - настройка атрибутов файла*/
void ChangeAttributesFile();
/*Функция проверки корректности команды*/
int InputAsNumber();

int main()
{
    //setlocale(LC_ALL, "Russian");
   
    DWORD buff = GetLogicalDrives();
    LPTSTR Drivers = new wchar_t[20]; //Получение списка дисков заранее
    int user; //Опции пользвателя

    buff = GetLogicalDriveStringsW(0, Drivers);
    GetLogicalDriveStringsW(buff, Drivers);

    cout << "Welcom to LabWork_1!";
    do {
        cout << "\n-----------------------------------------------\n\n";
        printMainMenu(); 
        cout << "Made by Evgeny Butov 0305" << endl;
        user = InputAsNumber();
        system("cls");
        switch (user) {
            case 1:
                getDisks(Drivers);
                break;
            case 2:
                getInformation(Drivers);
                break;
            case 3:
                CreateNewDirectory();
                break;
            case 4:
                RemoveCurrentDirectory();
                break;
            case 5:
                CreateNewFile();
                break;
            case 6:
                CopyCurrentFile();
                break;
            case 7:
                MoveCurrentFile();
                break;
            case 8:
                AnalyseFile();
                break;
            case 9:
                ChangeAttributesFile();
                break;
            case 0:
                cout << "Thank you for cheking my programm! See you later!";
                break;
        }
    } while (user != 0);
    delete[] Drivers;
}


void cleanInStream() {
    cin.clear();
    if (char(cin.peek()) == '\n') cin.ignore();
}

void CopyCurrentFile() {
    LPTSTR user_input = new wchar_t[MAX_PATH];
    LPTSTR FileName = new wchar_t[MAX_PATH];
    int access;
    cout << "Enter filepath of existing file to copy: \nUser>>";
    cleanInStream();
    wcin.getline(FileName, MAX_PATH);
    cout << "Enter the path of new file: \nUser>>";
    wcin.getline(user_input, MAX_PATH);
    if (CopyFile(FileName, user_input, true)) {
        cout << "The file was successfully copied! You can find new file in ";
        wprintf(L"%s", user_input);
        cout << endl;
    }
    else {
        if (GetLastError() == ERROR_FILE_EXISTS) {
            cout << "File with this name already exists, do you want to overwrite it?\n1 - Yes\n2 - No\n";
            cleanInStream();
            cin >> access;
            if (access == 1) {
                if (CopyFile(FileName, user_input, false)) {
                    cout << "The file was successfully copied! You can find new file in ";
                    wprintf(L"%s", user_input);
                    cout << endl;
                }
                else cout << "Huston, we have a problem number: " << GetLastError() << "!!";
            }
        }
        else {
            if (GetLastError() == ERROR_FILE_NOT_FOUND) cout << "The file beeng copyed doesn't exist!\n";
            if (GetLastError() == ERROR_PATH_NOT_FOUND) cout << "Directory which must contain new file doesn't exist!\n";
        }
    }
    delete[] user_input;
    delete[] FileName;
}

void MoveCurrentFile() {
    LPTSTR user_input = new wchar_t[MAX_PATH];
    LPTSTR FileName = new wchar_t[MAX_PATH];
    int access;

    cout << "Enter path of existing file to move: \nUser>>";
    cleanInStream();
    wcin.getline(FileName, MAX_PATH);
    cout << "Enter the path of new file: \nUser>>";
    wcin.getline(user_input, MAX_PATH);

    if (MoveFile(FileName, user_input)) {
        cout << "File successfully moved! You can find it in ";
        wprintf(L"%s", user_input);
        cout << endl;
    }
    else {
        if (GetLastError() == ERROR_ALREADY_EXISTS) {
            cout << "File with this name already exists, do you want to overwrite it?\n1 - Yes\n2 - No\n";
            cleanInStream();
            cout << "User>>";
            cin >> access;
            if (access == 1) {
                if (MoveFileEx(FileName, user_input, MOVEFILE_REPLACE_EXISTING)) { 
                    cout << "The file was successfully moved! You can find it in "; 
                    wprintf(L"%s", user_input);
                    cout << endl;
                }
                else cout << "Huston, we have a problem number: " << GetLastError() << "!!";
            }
        }
        else cout << "Huston, we have a problem number: " << GetLastError() << "!!";
    }
    delete[] user_input;
    delete[] FileName;
}

void printAttributes(DWORD Attributes) {
    if (Attributes & FILE_ATTRIBUTE_ARCHIVE) {
        cout << "Archive ";
    }
    if (Attributes & FILE_ATTRIBUTE_DEVICE) {
        cout << "Device ";
    }
    if (Attributes & FILE_ATTRIBUTE_DIRECTORY) {
        cout << "Directory ";
    }
    if (Attributes & FILE_ATTRIBUTE_ENCRYPTED) {
        cout << "Encrypted ";
    }
    if (Attributes & FILE_ATTRIBUTE_HIDDEN) {
        cout << "Hidden ";
    }
    if (Attributes & FILE_ATTRIBUTE_NORMAL) {
        cout << "Normal ";
    }
    if (Attributes & FILE_ATTRIBUTE_READONLY) {
        cout << "Read-only ";
    }
    if (Attributes & FILE_ATTRIBUTE_SYSTEM) {
        cout << "System ";
    }
    if (Attributes & FILE_ATTRIBUTE_TEMPORARY) {
        cout << "Temporary ";
    }
    if (Attributes & FILE_ATTRIBUTE_VIRTUAL) {
        cout << "Virtual ";
    }
}

void AnalyseFile() {
    LPTSTR FileName = new wchar_t[MAX_PATH];
    HANDLE File;
    DWORD Atributs = 0;
    LPBY_HANDLE_FILE_INFORMATION Info = new BY_HANDLE_FILE_INFORMATION();
    cout << "Enter fullpath of file to analyse it: ";
    cleanInStream();
    wcin.getline(FileName, MAX_PATH);
    if ((File = CreateFile(FileName, GENERIC_READ || GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0)) != INVALID_HANDLE_VALUE) {
        cout << "File successfully founded! Loading information...\n";
        Atributs = GetFileAttributes(FileName);
        GetFileInformationByHandle(File, Info);
        SYSTEMTIME st;
        cout << "File type: ";
        switch (GetFileType(File)) {
            case FILE_TYPE_CHAR:
                cout << "Character file/Console" << endl;
                break;
            case FILE_TYPE_DISK:
                cout << "file on disk" << endl;
                break;
            case FILE_TYPE_PIPE:
                cout << "Named or anonim chanel" << endl;
            case FILE_TYPE_UNKNOWN:
                cout << "Can't determinate file" << endl;
        }
        cout << "Serial number of the volume contains File: " << Info->dwVolumeSerialNumber << endl;
        cout << "Attributes: ";
        printAttributes(Info->dwFileAttributes);
        cout << endl;
        FileTimeToSystemTime(&Info->ftCreationTime, &st);
        cout << "File creation date: " << st.wDay << '.' << st.wMonth << '.' << st.wYear << endl;
        FileTimeToSystemTime(&Info->ftLastAccessTime, &st);
        cout << "File last access date: " << st.wDay << '.' << st.wMonth << '.' << st.wYear << endl;
        FileTimeToSystemTime(&Info->ftLastWriteTime, &st);
        cout << "File last write date: " << st.wDay << '.' << st.wMonth << '.' << st.wYear << endl;
        cout << "Size of file: " << Info->nFileSizeLow << endl;
        cout << "Number of links to this file: " << Info->nNumberOfLinks << endl;
        CloseHandle(File);
    }
    else {
        cout << "Huston, we have a problem number: " << GetLastError() << "!!";
    };
    
    delete Info;
    delete[] FileName;
}

void ChangeAttributesFile() {
    LPTSTR FileName = new wchar_t[MAX_PATH];
    HANDLE File;
    int user_input;
    DWORD attributes = 0;

    cout << "Enter fullpath of file to change attributes: ";
    cleanInStream();
    wcin.getline(FileName, MAX_PATH);
    if ((File = CreateFile(FileName, GENERIC_READ || GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0)) != INVALID_HANDLE_VALUE) {
        cout << "For any question press 1 to 'Yes' and 0 to 'No'" << endl;
        cout << "Set file to be archived?" << endl;
        cin >> user_input;
        if (user_input) attributes = attributes | FILE_ATTRIBUTE_ARCHIVE;
        cout << "Set file to be read-only?" << endl;
        cin >> user_input;
        if (user_input) attributes = attributes | FILE_ATTRIBUTE_READONLY;
        cout << "Set file to be hidden?" << endl;
        cin >> user_input;
        if (user_input) attributes = attributes | FILE_ATTRIBUTE_HIDDEN;
        if (attributes == 0)
            SetFileAttributes(FileName, FILE_ATTRIBUTE_NORMAL);
        else
            SetFileAttributes(FileName, attributes);
        cout << "Set file to be compressed?" << endl;
        cin >> user_input;
        if (user_input) DeviceIoControl(File, FSCTL_SET_COMPRESSION, 0, 0, 0, 0, 0, 0);
        CloseHandle(File);
    }
    delete[] FileName;
};

void printMainMenu() {
    cout << "Please, choose the option: " << endl;
    cout << "1 - Available drives on this PC" << endl;
    cout << "2 - Infromation about one drive" << endl;
    cout << "3 - Create a new directory" << endl;
    cout << "4 - Remove an existing directory" << endl;
    cout << "5 - Create a new file" << endl;
    cout << "6 - Copy an existing file" << endl;
    cout << "7 - Move an existing file" << endl;
    cout << "8 - Information about files" << endl;
    cout << "9 - Change attributes of files" << endl;
    cout << "0 - Exit " << endl;
}

int InputAsNumber() {
    string user;
    int in = -1;
    bool flag = true;
    do {
        cout << "User>>";
        cleanInStream();
        getline(cin, user);
        try {
            in = stoi(user);
            flag = (in < 10) && (in >= 0);
            if (!flag) cout << "Invalid command";
        }
        catch (const std::invalid_argument) {
            flag = false;
            cout << "Invalid format of command! Expected number in range [0; 9]" << endl;
        }
        
    } while (!flag);
    return in;
}