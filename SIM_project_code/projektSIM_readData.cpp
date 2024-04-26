#include <iostream>
#include "tinyxml2.h"
#include <sql.h>
#include <sqlext.h> 
#include <string.h>
#include <dirent.h> 
#include <dicomhero6/dicomhero.h> 

using namespace tinyxml2;
using namespace std;

#define CHECK_ERROR(code, str, h, env) if (code!=SQL_SUCCESS) {extract_error(str, h, env); clearHandle(hstmt, hdbc, henv); return 0;}

void extract_error(string fn, SQLHANDLE handle, SQLSMALLINT type){
	SQLINTEGER i = 0;
	SQLINTEGER native;
	SQLCHAR state[ 7 ];
	SQLCHAR text[256];
	SQLSMALLINT len;
	SQLRETURN ret;
	printf("\nThe driver reported the following diagnostics whilst running %s\n\n", fn.c_str());
	do
	{
		ret = SQLGetDiagRec(type, handle, ++i, state, &native, text, sizeof(text), &len );
		printf("\nSQLGetDiagRec returned %d\n\r", ret);
		if (SQL_SUCCEEDED(ret))
			printf("%s:%ld:%ld:%s\n", state, (long)i, (long)native, text);
	}
	while( ret == SQL_SUCCESS );
}

void clearHandle(SQLHSTMT hstmt, SQLHDBC hdbc, SQLHENV henv){
	if (hstmt != SQL_NULL_HSTMT)
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	if (hdbc != SQL_NULL_HDBC) {
		SQLDisconnect(hdbc);
		SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
	}
	if (henv != SQL_NULL_HENV)
		SQLFreeHandle(SQL_HANDLE_ENV, henv);
}


void tree(string p, SQLHDBC hdbc) {
    DIR * dirp = opendir(p.c_str()); //otwarcie katalogu ze sciezki
    dirent * dp; //uchwyt do struktury danych
    int dir_id = 0; //indeks katalogu
	int file_id = 0; //indeks pliku

    while ((dp = readdir(dirp)) != NULL) { //przechodzenie w petli po wszystkich strukturach w katalogu
        if (dp->d_type == DT_DIR && strcmp(dp->d_name, "..") != 0 && strcmp(dp->d_name, ".") != 0) { //jezeli struktura jest katalogiem
            dir_id++; //inkrementacja indeksu katalogu
            string pp = p + "/" + dp->d_name; //zlozenie nowej sciezki poprzez dolozenie nazwy struktury            
            tree(pp, hdbc); //wywolanie funkcji tree dla nowej sciezki
        } else if (dp->d_type == DT_REG) { //jezeli struktura jest plikiem
            file_id++; //inkrementacja indeksu plikow
            string pp = p + "/" + dp->d_name; //zlozenie nowej sciezki poprzez dolozenie nazwy struktury
            cout << pp << endl;
            SQLHSTMT hstmt;
            SQLRETURN retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
            if (retcode != SQL_SUCCESS) {
                cerr << "Error allocating statement handle" << endl;
                continue; // Skip current file and proceed to the next one
            }
            dicomhero::DataSet loadedDataSet(dicomhero::CodecFactory::load(pp, 2048));
    
            dicomhero::UnicodePatientName patientName = loadedDataSet.getUnicodePatientName(dicomhero::TagId(dicomhero::tagId_t::PatientName_0010_0010), 0);
            wstring patientNameCharacter = patientName.getAlphabeticRepresentation();
            string patientNameCharacter_S(patientNameCharacter.begin(), patientNameCharacter.end());
            cout << patientNameCharacter_S << endl;
            string fileName = dp->d_name;
            string query = "INSERT INTO Image(ImagePath) VALUES ('" + pp + "')";
            cout << query << endl;
            retcode = SQLExecDirect(hstmt, (SQLCHAR*)query.c_str(), SQL_NTS);
            if (retcode != SQL_SUCCESS) {
                cerr << "Error executing SQL statement" << endl;
            }
            SQLFreeHandle(SQL_HANDLE_STMT, hstmt); // Free statement handle
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

    string path = "/home/sim24/Documents/projekt/database_sample"; //sciezka do katalogu z series
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


