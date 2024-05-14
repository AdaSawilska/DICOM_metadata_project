#include <iostream>                 // Dołączenie biblioteki strumieni wejścia-wyjścia
#include <fstream>                  // Dołączenie biblioteki strumieni plików
#include "tinyxml2.h"               // Dołączenie biblioteki tinyxml2 do obsługi XML
#include <sql.h>                    // Dołączenie deklaracji API SQL
#include <sqlext.h>                 // Dołączenie rozszerzeń SQL
#include <string.h>                 // Dołączenie funkcji manipulacji stringami
#include <dirent.h>                 // Dołączenie funkcji przeglądania katalogów
#include <dicomhero6/dicomhero.h>   // Dołączenie biblioteki dicomhero do obsługi plików DICOM
#include <filesystem>               // Dołączenie biblioteki systemu plików C++17
#include <cstdio>                   // Dołączenie biblioteki standardowego wejścia-wyjścia C

using namespace tinyxml2;           // Użycie przestrzeni nazw tinyxml2
using namespace std;                // Użycie standardowej przestrzeni nazw
namespace fs = std::filesystem;     // Utworzenie aliasu 'fs' dla przestrzeni nazw std::filesystem

// Definicja makra obsługi błędów
#define CHECK_ERROR(code, str, h, env) if (code!=SQL_SUCCESS) {extract_error(str, h, env); clearHandle(hstmt, hdbc, henv); return 0;}

// Funkcja do ekstrakcji i drukowania informacji o błędach SQL
void extract_error(string fn, SQLHANDLE handle, SQLSMALLINT type){
    SQLINTEGER i = 0;              // Zainicjalizuj licznik rekordów diagnostycznych
    SQLINTEGER native;             // Zmienna do przechowywania natywnego kodu błędu
    SQLCHAR state[7];              // Tablica do przechowywania 5-znakowego kodu stanu SQL + terminator null
    SQLCHAR text[256];             // Tablica do przechowywania tekstu komunikatu błędu
    SQLSMALLINT len;               // Zmienna do przechowywania długości pobranego tekstu
    SQLRETURN ret;                 // Zmienna przechwytująca wartość zwracaną przez SQLGetDiagRec
    printf("\nThe driver reported the following diagnostics whilst running %s\n\n", fn.c_str());
    do {
        // Funkcja SQLGetDiagRec jest wywoływana z odpowiednimi parametrami, aby pobrać kolejne rekordy diagnostyczne
        ret = SQLGetDiagRec(type, handle, ++i, state, &native, text, sizeof(text), &len);
        printf("\nSQLGetDiagRec returned %d\n\r", ret);
        if (SQL_SUCCEEDED(ret))
            printf("%s:%ld:%ld:%s\n", state, (long)i, (long)native, text);
    }
    while(ret == SQL_SUCCESS);
}

// Funkcja do czyszczenia uchwytów SQL i rozłączenia
// SQLHSTMT hstmt (uchwyt instrukcji SQL), SQLHDBC hdbc (uchwyt połączenia z bazą danych), oraz SQLHENV henv (uchwyt środowiska SQL).
void clearHandle(SQLHSTMT hstmt, SQLHDBC hdbc, SQLHENV henv){
    // sprawdza czy uchwyt został wcześniej przydzielony i nie został jeszcze zwolniony.
    if (hstmt != SQL_NULL_HSTMT)
    // Zwolnienie uchwytu instrukcji:
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    if (hdbc != SQL_NULL_HDBC) {
        // Rozłączenie z bazą danych
        SQLDisconnect(hdbc);
        // Zwolnienie uchwytu połączenia
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
    }
    if (henv != SQL_NULL_HENV)
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
}

// Funkcja sprawdzająca istnienie określonego rekordu w bazie danych
SQLRETURN exist_check(string table, string column, string id_to_check, SQLHDBC hdbc) {
    SQLHSTMT hstmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
    string command = "SELECT " + column + " FROM " + table + " WHERE " + column + " = '" + id_to_check + "';";
    SQLExecDirect(hstmt, (SQLCHAR*) command.c_str(), SQL_NTS); // Wykonanie polecenia SELECT
    SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, 0, 0, (SQLPOINTER)id_to_check.c_str(), 0, NULL);
    SQLRETURN retcode = SQLFetch(hstmt); // Pobranie zestawu wyników
    SQLFreeStmt(hstmt, SQL_DROP);
    return retcode;
}

// Funkcja do pobierania określonej wartości z bazy danych
string get_value(string wanted_column, string table, string given_column, string value, SQLHDBC hdbc) {
    SQLHSTMT hstmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt); // Przydzielenie uchwytu instrukcji
    SQLINTEGER id;
    SQLLEN indicator;
    // komenda do znajdywania szukanej wartosci
    string query = "SELECT " + wanted_column + " FROM " + table + " WHERE " + given_column + " = '" + value + "';";
    if (SQLExecDirect(hstmt, (SQLCHAR *)query.c_str(), SQL_NTS) != SQL_SUCCESS) {
        cerr << "Error executing SQL statement" << endl;
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        return ""; // Zwróć pusty string w przypadku błędu
    }

    // Przetworzenie wyniku
    while (SQLFetch(hstmt) == SQL_SUCCESS) {
        SQLGetData(hstmt, 1, SQL_C_SLONG, &id, sizeof(id), &indicator);
        if (indicator != SQL_NULL_DATA) {
            return to_string(id); //zwraca string z wynikiem funkcji SELECT z komendy query
        } else {
            cerr << "No data found" << endl;
            return ""; // Zwróć pusty string, jeśli nie znaleziono danych
        }
    }
    return "";
}

// funkcja tworzaca sciezke do pliku .jpg na podstawie sciezki do .dcm
string createJPG_directory(const string& path) {
    // Get the parent directory of the given path
    path parentPath = path(path).parent_path();
    // Stworzenie sciezki folderu JPEG na pliki .jpg, zeby potem go stworzyc
    path jpgfolderPath = path(parentPath) / "JPEG";

    // Check if the parent directory exists
    if (!fs::exists(jpgfolderPath)) {
        // Create the parent directory if it doesn't exist
        if (!create_directories(jpgfolderPath)) {
            cerr << "Error: Failed to create directory: " << jpgfolderPath << endl;
            return "";
        }
        cout << "Created directory: " << jpgfolderPath << endl;
    }

    return jpgfolderPath;
}

bool endsWith(const string& str, const string& suffix) {
    return str.size() >= suffix.size() && str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

// funkcja rekurencyjna przeszukujaca foldery w poszukiwaniu plikow .dcm
void tree(string p, SQLHDBC hdbc) {
    DIR * dirp = opendir(p.c_str()); //otwarcie katalogu ze sciezki
    dirent * dp; //uchwyt do struktury danych
    int dir_id = 0; //indeks katalogu
	int file_id = 0; //indeks pliku

    while ((dp = readdir(dirp)) != NULL) { //przechodzenie w petli po wszystkich strukturach w katalogu
        if (dp->d_type == DT_DIR && strcmp(dp->d_name, "..") != 0 && strcmp(dp->d_name, ".") != 0) { //jezeli struktura jest katalogiem
            dir_id++; //inkrementacja indeksu katalogu
            string pp = p + "/" + dp->d_name; //zlozenie nowej sciezki poprzez dolozenie nazwy struktury            
            tree(pp, hdbc); //wywolanie funkcji tree dla nowej sciezki <- REKURENCJA
        } else if (dp->d_type == DT_REG) { //jezeli struktura jest plikiem
            file_id++; //inkrementacja indeksu plikow
            string pp = p + "/" + dp->d_name; //zlozenie nowej sciezki poprzez dolozenie nazwy struktury
            // jesli plik .dcm to wykona sie ten if
            if (endsWith(dp->d_name, ".dcm")) {
                SQLHSTMT hstmt;
                SQLRETURN retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
                if (retcode != SQL_SUCCESS) {
                    cerr << "Error allocating statement handle" << endl;
                    continue; // Skip current file and proceed to the next one
                }
                // ladowanie danych pliku .dcm ze sciezki pp, 2048 okresla rozmiar bufora - oznacza, że dane będą czytane blokami po 2048 bajtów. Jest to wartość arbitralna, która może być dostosowana 
                // w zależności od specyficznych potrzeb wydajnościowych i wymagań systemowych. 
                dicomhero::DataSet loadedDataSet(dicomhero::CodecFactory::load(pp, 2048));
                // Metoda służy do pobierania obrazu z zestawu danych DICOM
                dicomhero::Image image(loadedDataSet.getImageApplyModalityTransform(0));
                string jpgdir = createJPG_directory(p);
                string jpgname = dp->d_name;
                jpgname = jpgdir+ "/" + jpgname + ".jpg";
                // save .dcm as .jpeg
                dicomhero::CodecFactory::save(loadedDataSet, jpgname, dicomhero::codecType_t::jpeg);
                // Patient ID
                string patientID = loadedDataSet.getString(dicomhero::TagId(dicomhero::tagId_t::PatientID_0010_0020), 0);
                cout << patientID << endl;
                // Patient BirthDate
                string patientBirthDate = loadedDataSet.getString(dicomhero::TagId(dicomhero::tagId_t::PatientBirthDate_0010_0030), 0);
                string formattedBirthDate = patientBirthDate.substr(0, 4) + "-" + patientBirthDate.substr(4, 2) + "-" + patientBirthDate.substr(6, 2);
                // cout << "Patient's Birth Date: " << formattedBirthDate  << endl;
                // Patient Sex
                string patientSex = loadedDataSet.getString(dicomhero::TagId(dicomhero::tagId_t::PatientSex_0010_0040), 0);
                // cout << patientSex << endl;
                // Patient Name
                dicomhero::UnicodePatientName patientName = loadedDataSet.getUnicodePatientName(dicomhero::TagId(dicomhero::tagId_t::PatientName_0010_0010), 0);
                wstring patientNameCharacter = patientName.getAlphabeticRepresentation();
                string patientNameCharacter_S(patientNameCharacter.begin(), patientNameCharacter.end());
                // cout << patientNameCharacter_S << endl;

                // Study ID
                string studyID = loadedDataSet.getString(dicomhero::TagId(dicomhero::tagId_t::StudyID_0020_0010), 0);
                // cout << "Study ID: " << studyID << endl;
                // Study Instance UID
                string studyInstanceUID = loadedDataSet.getString(dicomhero::TagId(dicomhero::tagId_t::StudyInstanceUID_0020_000D), 0);
                // cout << "Study Instance UID: " << studyInstanceUID << endl;
                // Study Date
                string studyDate = loadedDataSet.getString(dicomhero::TagId(dicomhero::tagId_t::StudyDate_0008_0020), 0);
                string formattedStudyDate = studyDate.substr(0, 4) + "-" + studyDate.substr(4, 2) + "-" + studyDate.substr(6, 2);
                // cout << "Study Date: " << formattedStudyDate << endl;
                // Study Time
                string studyTime = loadedDataSet.getString(dicomhero::TagId(dicomhero::tagId_t::StudyTime_0008_0030), 0);
                std::string formattedStudyTime = studyTime.substr(0, 2) + ":" + studyTime.substr(2, 2) + ":" + studyTime.substr(4, 2);
                // cout << "Study Time: " << formattedStudyTime << endl;
                // Study Description
                string studyDescription = loadedDataSet.getString(dicomhero::TagId(dicomhero::tagId_t::StudyDescription_0008_1030), 0);
                // cout << "Study Description: " << studyDescription << endl;


                // Series Instance UID
                string seriesInstanceUID = loadedDataSet.getString(dicomhero::TagId(dicomhero::tagId_t::SeriesInstanceUID_0020_000E), 0);
                // cout << "Series Instance UID: " << seriesInstanceUID << endl;
                // Modality
                string modality = loadedDataSet.getString(dicomhero::TagId(dicomhero::tagId_t::Modality_0008_0060), 0);
                // cout << "Modality: " << modality << endl;
                // Body Part Examined
                string bodyPartExamined = loadedDataSet.getString(dicomhero::TagId(dicomhero::tagId_t::BodyPartExamined_0018_0015), 0);
                // cout << "Body Part Examined: " << bodyPartExamined << endl;
                // Series Description
                string seriesDescription = loadedDataSet.getString(dicomhero::TagId(dicomhero::tagId_t::SeriesDescription_0008_103E), 0);
                // cout << "Series Description: " << seriesDescription << endl;

                //sprawdzenie czy w bazie jest juz taki Patient z takim samym dicomid:
                retcode = exist_check("Patient", "dicomid", patientID, hdbc);
                if(retcode != SQL_NO_DATA) {
                    cout<<"Pacjent o takim id juz istnieje w bazie"<<endl;
                }
                //jesli nie ma takiego pacjenta to wprowadzenie go do bazy:
                else {
                    //dodanie nowego pacjenta
                    cout << "Wprowadzam nowego pacjenta do bazy..." << endl;
                    retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);                
                    string komenda1 = "INSERT INTO Patient(birthdate, sex, name, dicomid) VALUES ('"+formattedBirthDate+"','"+patientSex+"','"+patientNameCharacter_S+"', '"+patientID+"')";
                    cout << komenda1 << endl;
                    retcode = SQLExecDirect(hstmt,(SQLCHAR*) komenda1.c_str(), SQL_NTS);
                    retcode = SQLFreeStmt(hstmt, SQL_DROP);
                }

                //sprawdzenie czy w bazie jest juz takie Study z takim samym StudyUID:
                retcode = exist_check("Study", "studyuid", studyInstanceUID, hdbc);
                if(retcode != SQL_NO_DATA) {
                    cout<<"Study o takim uid juz istnieje w bazie"<<endl;
                }
                //jesli nie ma takiego Study to wprowadzenie go do bazy:
                else {
                    //dodanie nowego Study
                    cout << "Wprowadzam nowe study do bazy..." << endl;
                    retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
                    // sprawdzenie ID pacjenta w odbc dla danego study
                    string patient_database_id = get_value("id", "Patient", "dicomid", patientID, hdbc);
                    cout << "ID pacjenta w ODBC: " << patient_database_id << endl;
                    string komenda1 = "INSERT INTO Study(studyuid, patientid, studydate, studytime, studydescription) VALUES ('"+studyInstanceUID+"','"+patient_database_id+"','"+formattedStudyDate+"', '"+formattedStudyTime+"', '"+studyDescription+"')";
                    cout << komenda1 << endl;
                    retcode = SQLExecDirect(hstmt,(SQLCHAR*) komenda1.c_str(), SQL_NTS);
                    retcode = SQLFreeStmt(hstmt, SQL_DROP);
                }

                //sprawdzenie czy w bazie jest juz takie Series z takim samym SeriesUID:
                retcode = exist_check("Series", "seriesuid", seriesInstanceUID, hdbc);
                if(retcode != SQL_NO_DATA) {
                    cout<<"Series o takim uid juz istnieje w bazie"<<endl;
                }
                //jesli nie ma takiego Series to wprowadzenie go do bazy:
                else {
                    //dodanie nowego Series
                    cout << "Wprowadzam nowe study do bazy..." << endl;
                    retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
                    // sprawdzenie StudyUID w odbc dla danego study
                    string study_database_id = get_value("id", "Study", "studyuid", studyInstanceUID, hdbc);
                    cout << "StudyID w ODBC: " << study_database_id << endl;
                    string komenda1 = "INSERT INTO Series(seriesuid, studyid, modality, bodypart, seriesdescription) VALUES ('"+seriesInstanceUID+"','"+study_database_id+"','"+modality+"', '"+bodyPartExamined+"', '"+seriesDescription+"')";
                    cout << komenda1 << endl;
                    retcode = SQLExecDirect(hstmt,(SQLCHAR*) komenda1.c_str(), SQL_NTS);
                    retcode = SQLFreeStmt(hstmt, SQL_DROP);
                }

                //sprawdzenie czy w bazie jest juz taki Image z taka sciezka:
                retcode = exist_check("Image", "imagepath", pp, hdbc);
                if(retcode != SQL_NO_DATA) {
                    cout<<"Image juz istnieje w bazie"<<endl;
                }
                //jesli nie ma takiego Series to wprowadzenie go do bazy:
                else {
                    //dodanie nowego Series
                    cout << "Wprowadzam nowe image do bazy..." << endl;
                    retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
                    // sprawdzenie SeriesUID w odbc dla danego image
                    string series_database_id = get_value("id", "Series", "seriesuid", seriesInstanceUID, hdbc);
                    cout << "SeriesID w ODBC: " << series_database_id << endl;
                    string komenda1 = "INSERT INTO Image(imagepath, seriesid) VALUES ('"+jpgname+"','"+series_database_id+"')";
                    cout << komenda1 << endl;
                    retcode = SQLExecDirect(hstmt,(SQLCHAR*) komenda1.c_str(), SQL_NTS);
                    retcode = SQLFreeStmt(hstmt, SQL_DROP);
                }

                SQLFreeHandle(SQL_HANDLE_STMT, hstmt); // Free statement handle
            } else {
                    // Skip processing if it's not a .dcm file
                continue;
            }
            
        }
    }
}




int main() {
    SQLHENV henv = SQL_NULL_HENV;
    SQLHDBC hdbc = SQL_NULL_HDBC;
    SQLHSTMT hstmt = SQL_NULL_HSTMT; //uchwyty do laczenia sie z odbc
    SQLRETURN retcode;

    //funkcje:
    retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
    CHECK_ERROR(retcode, "SQLAllocHandle(SQL_HANDLE_ENV)", henv, SQL_HANDLE_ENV);
    retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0);
    CHECK_ERROR(retcode, "SQLSetEnvAttr(SQL_ATTR_ODBC_VERSION)", henv, SQL_HANDLE_ENV);
    retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);
    CHECK_ERROR(retcode, "SQLAllocHandle(SQL_HANDLE_DBC)", hdbc, SQL_HANDLE_DBC);
    retcode = SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);
    CHECK_ERROR(retcode, "SQLSetConnectAttr(SQL_LOGIN_TIMEOUT)", hdbc, SQL_HANDLE_DBC);

    retcode = SQLConnect(hdbc, (SQLCHAR*)"testODBC", SQL_NTS, (SQLCHAR*)NULL, SQL_NTS, NULL, SQL_NTS);
    CHECK_ERROR(retcode, "SQLConnect(DATASOURCE)", hdbc, SQL_HANDLE_DBC);

    cout << "Udalo sie polaczyc z baza!" << endl;

    // Pobranie danych z bazy danych
    retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
    CHECK_ERROR(retcode, "SQLAllocHandle(SQL_HANDLE_STMT)", hstmt, SQL_HANDLE_STMT);

    string query = "SELECT * FROM Image"; // Zmodyfikuj nazwę tabeli, z której chcesz pobrać dane
    retcode = SQLExecDirect(hstmt, (SQLCHAR*)query.c_str(), SQL_NTS);
    CHECK_ERROR(retcode, "SQLExecDirect()", hstmt, SQL_HANDLE_STMT);

    cout << "Dane z bazy pobrane!" << endl;

    string path = "/home/sim24/Documents/projekt/database_full/database"; //sciezka do katalogu do bazy danych
    tree(path, hdbc);

    // Zwalnianie zasobów
    if (hstmt != SQL_NULL_HSTMT)
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    if (hdbc != SQL_NULL_HDBC) {
        SQLDisconnect(hdbc);
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
    }
    if (henv != SQL_NULL_HENV)
        SQLFreeHandle(SQL_HANDLE_ENV, henv);

    return 0;
}


