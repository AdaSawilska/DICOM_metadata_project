#include <iostream>           // Włącza standardową bibliotekę wejścia/wyjścia C++.
#include <fstream>            // Włącza bibliotekę umożliwiającą operacje na plikach.
#include <tinyxml2.h>         // Włącza bibliotekę TinyXML-2 do analizy i generowania dokumentów XML.
#include <sql.h>              // Włącza nagłówki do obsługi funkcji związanych z SQL.
#include <sqlext.h>           // Dodatkowe nagłówki dla obsługi SQL.
#include <string.h>           // Włącza bibliotekę dla operacji na łańcuchach znaków.
#include <dirent.h>           // Włącza nagłówek umożliwiający operacje na katalogach i plikach w systemie plików.
#include <filesystem>         // Włącza bibliotekę C++17 do zarządzania systemem plików i operacji na plikach.
#include <cstdio>             // Włącza standardową bibliotekę wejścia/wyjścia C.

using namespace tinyxml2;    // Używa przestrzeni nazw tinyxml2 (z biblioteki TinyXML-2).
using namespace std;         // Używa przestrzeni nazw standardowej (std) dla ułatwienia dostępu do elementów tych bibliotek w kodzie.

// Definicja makra CHECK_ERROR z czterema parametrami:
// - code: Kod błędu do sprawdzenia.
// - str: Opis błędu do wyświetlenia w przypadku niepowodzenia.
// - h: Uchwyt do zasobu (np. hstmt - uchwyt do instrukcji SQL).
// - env: Uchwyt do środowiska (np. henv - uchwyt do środowiska SQL).

// Warunek if sprawdza, czy wartość code nie jest równa SQL_SUCCESS (co oznacza niepowodzenie operacji SQL).

// W przypadku niepowodzenia (code != SQL_SUCCESS):
// - Wywołuje funkcję extract_error(str, h, env), która zajmuje się pobraniem informacji o błędzie na podstawie podanego opisu str i uchwytów h i env.
// - Wywołuje funkcję clearHandle(hstmt, hdbc, henv), która służy do zwolnienia zasobów reprezentowanych przez uchwyty hstmt, hdbc i henv.
// - Natychmiast zwraca wartość 0, kończąc bieżącą funkcję i sygnalizując niepowodzenie lub wystąpienie błędu.
#define CHECK_ERROR(code, str, h, env) if (code!=SQL_SUCCESS) {extract_error(str, h, env); clearHandle(hstmt, hdbc, henv); return 0;}

void displayMenu();
// Funkcja do wyświetlania menu głównego.

void viewPatients(SQLHDBC hdbc);
// Funkcja do wyświetlania listy pacjentów z bazy danych.
// Parametr `hdbc` to uchwyt do połączenia z bazą danych.

void viewStudies(SQLHDBC hdbc);
// Funkcja do wyświetlania listy badań pacjentów z bazy danych.
// Parametr `hdbc` to uchwyt do połączenia z bazą danych.

void viewSeries(SQLHDBC hdbc);
// Funkcja do wyświetlania listy serii obrazów z bazy danych.
// Parametr `hdbc` to uchwyt do połączenia z bazą danych.

void viewImages(SQLHDBC hdbc);
// Funkcja do wyświetlania listy obrazów z bazy danych.
// Parametr `hdbc` to uchwyt do połączenia z bazą danych.

void viewPatientDetails(SQLHDBC hdbc, int patientID);
// Funkcja do wyświetlania szczegółowych informacji o wybranym pacjencie.
// Parametr `hdbc` to uchwyt do połączenia z bazą danych.
// Parametr `patientID` to identyfikator pacjenta.

void viewStudyDetails(SQLHDBC hdbc, int studyID);
// Funkcja do wyświetlania szczegółowych informacji o wybranym badaniu.
// Parametr `hdbc` to uchwyt do połączenia z bazą danych.
// Parametr `studyID` to identyfikator badania.

void viewSeriesDetails(SQLHDBC hdbc, int seriesID);
// Funkcja do wyświetlania szczegółowych informacji o wybranej serii obrazów.
// Parametr `hdbc` to uchwyt do połączenia z bazą danych.
// Parametr `seriesID` to identyfikator serii obrazów.

void extract_error(SQLHANDLE handle, SQLSMALLINT type);
// Funkcja do obsługi błędów związanych z bazą danych.
// Parametr `handle` to uchwyt, którego błąd ma być wydobyty.
// Parametr `type` to typ błędu, który ma zostać obsłużony.

int main() {
    // Initialize ODBC environment and handles
    SQLHENV henv = SQL_NULL_HENV;
    SQLHDBC hdbc = SQL_NULL_HDBC;
    SQLHSTMT hstmt = SQL_NULL_HSTMT;
    SQLRETURN retcode;

    // Allocate environment handle
    retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
    if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
        cout << "Error allocating environment handle" << endl;
        return 1;
    }

    // Set ODBC version
    retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
    if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
        cout << "Error setting ODBC version" << endl;
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return 1;
    }

    // Allocate connection handle
    retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);
    if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
        cout << "Error allocating connection handle" << endl;
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return 1;
    }

    // Connect to PostgreSQL database
    retcode = SQLConnect(hdbc, (SQLCHAR*)"testODBC", SQL_NTS, (SQLCHAR*)NULL, SQL_NTS, NULL, SQL_NTS);
    if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
        cout << "Error connecting to database" << endl;
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return 1;
    }

    cout << "Connected to database!" << endl;

    // Main loop for interactive menu
    while (true) {
        displayMenu();

        int choice;
        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice) {
            case 1:
                viewPatients(hdbc);
                break;
            case 2:
                viewStudies(hdbc);
                break;
            case 3:
                viewSeries(hdbc);
                break;
            case 4:
                viewImages(hdbc);
                break;
            case 5:
                cout << "Exiting..." << endl;
                break;
            default:
                cout << "Invalid choice. Please try again." << endl;
                break;
        }

        if (choice == 5)
            break;
    }

    // Disconnect and free resources
    SQLDisconnect(hdbc);
    SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
    SQLFreeHandle(SQL_HANDLE_ENV, henv);

    return 0;
}

void displayMenu() {
    cout << "\nDICOM Database Menu" << endl;
    cout << "1. View Patients" << endl;
    cout << "2. View Studies" << endl;
    cout << "3. View Series" << endl;
    cout << "4. View Images" << endl;
    cout << "5. Exit" << endl;
}

void viewPatients(SQLHDBC hdbc) {
    SQLHSTMT hstmt;
    SQLRETURN retcode;

    retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
    if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
        cout << "Error allocating statement handle" << endl;
        return;
    }

    retcode = SQLExecDirect(hstmt, (SQLCHAR*)"SELECT * FROM Patient", SQL_NTS);
    if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
        cout << "Error executing query" << endl;
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        return;
    }

    cout << "\nPatients:" << endl;
    SQLINTEGER id;
    SQLCHAR name[101];
    while (SQLFetch(hstmt) == SQL_SUCCESS) {
        SQLGetData(hstmt, 1, SQL_C_SLONG, &id, 0, NULL);
        SQLGetData(hstmt, 4, SQL_C_CHAR, name, sizeof(name), NULL);
        cout << "ID: " << id << ", Name: " << name << endl;
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

    // Prompt user to view details of a specific patient
    int patientID;
    cout << "Enter the ID of the patient you want to view details for: ";
    cin >> patientID;
    viewPatientDetails(hdbc, patientID);
}

void viewStudies(SQLHDBC hdbc) {
    SQLHSTMT hstmt;
    SQLRETURN retcode;

    retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
    if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
        cout << "Error allocating statement handle" << endl;
        return;
    }

    retcode = SQLExecDirect(hstmt, (SQLCHAR*)"SELECT * FROM Study", SQL_NTS);
    if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
        cout << "Error executing query" << endl;
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        return;
    }

    cout << "\nStudies:" << endl;
    SQLINTEGER id;
    SQLCHAR studyUID[101];
    while (SQLFetch(hstmt) == SQL_SUCCESS) {
        SQLGetData(hstmt, 1, SQL_C_SLONG, &id, 0, NULL);
        SQLGetData(hstmt, 2, SQL_C_CHAR, studyUID, sizeof(studyUID), NULL);
        cout << "ID: " << id << ", StudyUID: " << studyUID << endl;
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

    // Prompt user to view details of a specific patient
    int studyID;
    cout << "Enter the Study ID you want to view details for: ";
    cin >> studyID;
    viewStudyDetails(hdbc, studyID);
}

void viewSeries(SQLHDBC hdbc) {
    SQLHSTMT hstmt;
    SQLRETURN retcode;

    retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
    if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
        cout << "Error allocating statement handle" << endl;
        return;
    }

    retcode = SQLExecDirect(hstmt, (SQLCHAR*)"SELECT * FROM Series", SQL_NTS);
    if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
        cout << "Error executing query" << endl;
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        return;
    }

    cout << "\nSeries:" << endl;
    SQLINTEGER id;
    SQLCHAR modality[51];
    while (SQLFetch(hstmt) == SQL_SUCCESS) {
        SQLGetData(hstmt, 1, SQL_C_SLONG, &id, 0, NULL);
        SQLGetData(hstmt, 4, SQL_C_CHAR, modality, sizeof(modality), NULL);
        cout << "ID: " << id << ", Modality: " << modality << endl;
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

    // Prompt user to view details of a specific patient
    int seriesID;
    cout << "Enter the Series ID you want to view details for: ";
    cin >> seriesID;
    viewSeriesDetails(hdbc, seriesID);
}

void viewImages(SQLHDBC hdbc) {
    SQLHSTMT hstmt;
    SQLRETURN retcode;

    retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
    if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
        cout << "Error allocating statement handle" << endl;
        return;
    }

    retcode = SQLExecDirect(hstmt, (SQLCHAR*)"SELECT * FROM Image", SQL_NTS);
    if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
        cout << "Error executing query" << endl;
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        return;
    }

    cout << "\nImages:" << endl;
    SQLINTEGER id;
    SQLCHAR imagepath[256];
    while (SQLFetch(hstmt) == SQL_SUCCESS) {
        SQLGetData(hstmt, 1, SQL_C_SLONG, &id, 0, NULL);
        SQLGetData(hstmt, 4, SQL_C_CHAR, imagepath, sizeof(imagepath), NULL);
        cout << "ID: " << id << ", Path: " << imagepath << endl;
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
}

void viewPatientDetails(SQLHDBC hdbc, int patientID) {
    SQLHSTMT hstmt;
    SQLRETURN retcode;

    retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
    if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
        cout << "Error allocating statement handle" << endl;
        return;
    }

    string query = "SELECT * FROM Patient WHERE id = " + to_string(patientID);
    retcode = SQLExecDirect(hstmt, (SQLCHAR*)query.c_str(), SQL_NTS);
    if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
        cout << "Error executing query" << endl;
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        return;
    }

    cout << "\nPatient Details:" << endl;
    SQLINTEGER id;
    SQLCHAR birthdate[11];
    SQLCHAR sex[11];
    SQLCHAR name[101];
    SQLCHAR dicomID[101];
    retcode = SQLFetch(hstmt);
    if (retcode == SQL_SUCCESS) {
        SQLGetData(hstmt, 1, SQL_C_SLONG, &id, 0, NULL);
        SQLGetData(hstmt, 2, SQL_C_CHAR, birthdate, sizeof(birthdate), NULL);
        SQLGetData(hstmt, 3, SQL_C_CHAR, sex, sizeof(sex), NULL);
        SQLGetData(hstmt, 4, SQL_C_CHAR, name, sizeof(name), NULL);
        SQLGetData(hstmt, 5, SQL_C_CHAR, dicomID, sizeof(dicomID), NULL);

        cout << "ID: " << id << endl;
        cout << "Birthdate: " << birthdate << endl;
        cout << "Sex: " << sex << endl;
        cout << "Name: " << name << endl;
        cout << "Dicom ID: " << dicomID << endl;

        // Fetch and display associated studies
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
        if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
            cout << "Error allocating statement handle" << endl;
            return;
        }

        query = "SELECT * FROM Study WHERE PatientID = " + to_string(patientID);
        retcode = SQLExecDirect(hstmt, (SQLCHAR*)query.c_str(), SQL_NTS);
        if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
            cout << "Error executing query" << endl;
            SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
            return;
        }

        cout << "\nStudies for Patient ID " << patientID << ":" << endl;
        SQLINTEGER studyID;
        SQLCHAR studyUID[101];
        SQLCHAR studyDate[11];
        SQLCHAR studyTime[9];
        SQLCHAR studyDescription[256];
        while (SQLFetch(hstmt) == SQL_SUCCESS) {
            SQLGetData(hstmt, 1, SQL_C_SLONG, &studyID, 0, NULL);
            SQLGetData(hstmt, 2, SQL_C_CHAR, studyUID, sizeof(studyUID), NULL);
            SQLGetData(hstmt, 4, SQL_C_CHAR, studyDate, sizeof(studyDate), NULL);
            SQLGetData(hstmt, 5, SQL_C_CHAR, studyTime, sizeof(studyTime), NULL);
            SQLGetData(hstmt, 6, SQL_C_CHAR, studyDescription, sizeof(studyDescription), NULL);

            cout << "Study ID: " << studyID << endl;
            cout << "Study UID: " << studyUID << endl;
            cout << "Study Date: " << studyDate << endl;
            cout << "Study Time: " << studyTime << endl;
            cout << "Study Description: " << studyDescription << endl;
            cout << "---------------------" << endl;
        }
        
        // Prompt user to view details of a specific patient
        int studyIDtoview;
        cout << "\nEnter the Study ID you want to view details for: ";
        cin >> studyIDtoview;
        viewStudyDetails(hdbc, studyIDtoview);

        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    } else {
        cout << "Patient with ID " << patientID << " not found." << endl;
    }
}

void viewStudyDetails(SQLHDBC hdbc, int studyID) {
    SQLHSTMT hstmt;
    SQLRETURN retcode;

    retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
    if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
        cout << "Error allocating statement handle" << endl;
        return;
    }

    string query = "SELECT * FROM Study WHERE id = " + to_string(studyID);
    retcode = SQLExecDirect(hstmt, (SQLCHAR*)query.c_str(), SQL_NTS);
    if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
        cout << "Error executing query" << endl;
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        return;
    }

    cout << "\nStudy Details:" << endl;
    SQLINTEGER id;
    SQLCHAR studyUID[101];
    SQLCHAR patientID[101];
    SQLCHAR studyDate[11];
    SQLCHAR studyTime[9];
    SQLCHAR studyDescription[256];
    retcode = SQLFetch(hstmt);
    if (retcode == SQL_SUCCESS) {
        SQLGetData(hstmt, 1, SQL_C_SLONG, &id, 0, NULL);
        SQLGetData(hstmt, 2, SQL_C_CHAR, studyUID, sizeof(studyUID), NULL);
        SQLGetData(hstmt, 3, SQL_C_CHAR, patientID, sizeof(patientID), NULL);
        SQLGetData(hstmt, 4, SQL_C_CHAR, studyDate, sizeof(studyDate), NULL);
        SQLGetData(hstmt, 5, SQL_C_CHAR, studyTime, sizeof(studyTime), NULL);
        SQLGetData(hstmt, 6, SQL_C_CHAR, studyDescription, sizeof(studyDescription), NULL);

        cout << "ID: " << id << endl;
        cout << "Study UID: " << studyUID << endl;
        cout << "Patient ID: " << patientID << endl;
        cout << "Study Date: " << studyDate << endl;
        cout << "Study Time: " << studyTime << endl;
        cout << "Study Description: " << studyDescription << endl;

        // Fetch and display associated series
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
        if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
            cout << "Error allocating statement handle" << endl;
            return;
        }

        query = "SELECT * FROM Series WHERE StudyID = " + to_string(studyID);
        retcode = SQLExecDirect(hstmt, (SQLCHAR*)query.c_str(), SQL_NTS);
        if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
            cout << "Error executing query" << endl;
            SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
            return;
        }

        cout << "\nSeries for Study ID " << studyID << ":" << endl;
        SQLINTEGER seriesID;
        SQLCHAR seriesUID[101];
        SQLCHAR modality[51];
        SQLCHAR bodypart[101];
        SQLCHAR seriesDescription[256];
        while (SQLFetch(hstmt) == SQL_SUCCESS) {
            SQLGetData(hstmt, 1, SQL_C_SLONG, &seriesID, 0, NULL);
            SQLGetData(hstmt, 2, SQL_C_CHAR, seriesUID, sizeof(studyUID), NULL);
            SQLGetData(hstmt, 4, SQL_C_CHAR, modality, sizeof(studyDate), NULL);
            SQLGetData(hstmt, 5, SQL_C_CHAR, bodypart, sizeof(studyTime), NULL);
            SQLGetData(hstmt, 6, SQL_C_CHAR, seriesDescription, sizeof(studyDescription), NULL);

            cout << "Series ID: " << seriesID << endl;
            cout << "Series UID: " << seriesUID << endl;
            cout << "Modality: " << modality << endl;
            cout << "Body Part: " << bodypart << endl;
            cout << "Series Description: " << seriesDescription << endl;
            cout << "---------------------" << endl;
        }

        // Prompt user to view details of a specific patient
        int seriesIDtoview;
        cout << "\nEnter the Series ID you want to view details for: ";
        cin >> seriesIDtoview;
        viewSeriesDetails(hdbc, seriesIDtoview);
        
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    } else {
        cout << "Study with ID " << studyID << " not found." << endl;
    }
}

void viewSeriesDetails(SQLHDBC hdbc, int seriesID) {
    SQLHSTMT hstmt;
    SQLRETURN retcode;

    retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
    if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
        cout << "Error allocating statement handle" << endl;
        return;
    }

    string query = "SELECT * FROM Series WHERE id = " + to_string(seriesID);
    retcode = SQLExecDirect(hstmt, (SQLCHAR*)query.c_str(), SQL_NTS);
    if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
        cout << "Error executing query" << endl;
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        return;
    }

    cout << "\nSeries Details:" << endl;
    SQLINTEGER id;
    SQLCHAR seriesUID[101];
    SQLCHAR studyID[101];
    SQLCHAR modality[51];
    SQLCHAR bodypart[101];
    SQLCHAR seriesDescription[256];
    retcode = SQLFetch(hstmt);
    if (retcode == SQL_SUCCESS) {
        SQLGetData(hstmt, 1, SQL_C_SLONG, &id, 0, NULL);
        SQLGetData(hstmt, 2, SQL_C_CHAR, seriesUID, sizeof(seriesUID), NULL);
        SQLGetData(hstmt, 3, SQL_C_CHAR, studyID, sizeof(studyID), NULL);
        SQLGetData(hstmt, 4, SQL_C_CHAR, modality, sizeof(modality), NULL);
        SQLGetData(hstmt, 5, SQL_C_CHAR, bodypart, sizeof(bodypart), NULL);
        SQLGetData(hstmt, 6, SQL_C_CHAR, seriesDescription, sizeof(seriesDescription), NULL);

        cout << "ID: " << id << endl;
        cout << "Series UID: " << seriesUID << endl;
        cout << "Study ID: " << studyID << endl;
        cout << "Modality: " << modality << endl;
        cout << "Body Part: " << bodypart << endl;
        cout << "Series Description: " << seriesDescription << endl;

        // Fetch and display associated series
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
        if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
            cout << "Error allocating statement handle" << endl;
            return;
        }

        query = "SELECT * FROM Image WHERE SeriesID = " + to_string(seriesID);
        retcode = SQLExecDirect(hstmt, (SQLCHAR*)query.c_str(), SQL_NTS);
        if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
            cout << "Error executing query" << endl;
            SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
            return;
        }

        cout << "\nImages for Series ID " << seriesID << ":" << endl;
        SQLINTEGER imageid;
        SQLCHAR ImagePath[256];
        while (SQLFetch(hstmt) == SQL_SUCCESS) {
            SQLGetData(hstmt, 1, SQL_C_SLONG, &imageid, 0, NULL);
            SQLGetData(hstmt, 2, SQL_C_CHAR, ImagePath, sizeof(ImagePath), NULL);

            cout << "Image ID: " << imageid << endl;
            cout << "Image Path: " << ImagePath << endl;
            cout << "---------------------" << endl;
        }

        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    } else {
        cout << "Series with ID " << seriesID << " not found." << endl;
    }
}

void extract_error(SQLHANDLE handle, SQLSMALLINT type) {
    SQLINTEGER i = 0;
    SQLINTEGER native;
    SQLCHAR state[SQL_SQLSTATE_SIZE + 1];
    SQLCHAR text[SQL_MAX_MESSAGE_LENGTH];
    SQLSMALLINT len;
    SQLRETURN ret;

    do {
        ret = SQLGetDiagRec(type, handle, ++i, state, &native, text, sizeof(text), &len);
        if (SQL_SUCCEEDED(ret)) {
            std::cout << "SQL Error: " << state << " - " << text << std::endl;
        }
    } while (ret == SQL_SUCCESS);
}
