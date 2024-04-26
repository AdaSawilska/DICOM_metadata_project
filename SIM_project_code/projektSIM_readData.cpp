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

SQLRETURN exist_check(string table, string column, string id_to_check, SQLHDBC hdbc) {
    SQLHSTMT hstmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
    string command = "SELECT " + column +" FROM "+ table +" WHERE "+ column +" = '"+id_to_check+"';";
    SQLExecDirect(hstmt,(SQLCHAR*) command.c_str(), SQL_NTS); //w tym celu wywolywany jest select z zadanymi wartosciami
    //SQLBindCol(hstmt, 1, SQL_C_USHORT, &cIDB, 2,  &lenIDB); //pobieramy tylko jedna kolumne bo wiecej nie potrzeba w do weryfikacji
    SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, 0, 0, (SQLPOINTER)id_to_check.c_str(), 0, NULL);
    SQLRETURN retcode = SQLFetch(hstmt); //pobranie pojedynczego wiersza zwracanego po select
    SQLFreeStmt(hstmt, SQL_DROP);
    return retcode;
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

            // odczytanie danych z pliku .dcm
            dicomhero::DataSet loadedDataSet(dicomhero::CodecFactory::load(pp, 2048));
            // Patient ID
            string patientID = loadedDataSet.getString(dicomhero::TagId(dicomhero::tagId_t::PatientID_0010_0020), 0);
            cout << patientID << endl;
            // Patient BirthDate
            string patientBirthDate = loadedDataSet.getString(dicomhero::TagId(dicomhero::tagId_t::PatientBirthDate_0010_0030), 0);
            string formattedBirthDate = patientBirthDate.substr(0, 4) + "-" + patientBirthDate.substr(4, 2) + "-" + patientBirthDate.substr(6, 2);
            cout << "Patient's Birth Date: " << formattedBirthDate  << endl;
            // Patient Sex
            string patientSex = loadedDataSet.getString(dicomhero::TagId(dicomhero::tagId_t::PatientSex_0010_0040), 0);
            cout << patientSex << endl;
            // Patient Name
            dicomhero::UnicodePatientName patientName = loadedDataSet.getUnicodePatientName(dicomhero::TagId(dicomhero::tagId_t::PatientName_0010_0010), 0);
            wstring patientNameCharacter = patientName.getAlphabeticRepresentation();
            string patientNameCharacter_S(patientNameCharacter.begin(), patientNameCharacter.end());
            cout << patientNameCharacter_S << endl;

            // Study ID
            string studyID = loadedDataSet.getString(dicomhero::TagId(dicomhero::tagId_t::StudyID_0020_0010), 0);
            cout << "Study ID: " << studyID << endl;
            // Study Instance UID
            string studyInstanceUID = loadedDataSet.getString(dicomhero::TagId(dicomhero::tagId_t::StudyInstanceUID_0020_000D), 0);
            cout << "Study Instance UID: " << studyInstanceUID << endl;
            // Study Date
            string studyDate = loadedDataSet.getString(dicomhero::TagId(dicomhero::tagId_t::StudyDate_0008_0020), 0);
            string formattedStudyDate = studyDate.substr(0, 4) + "-" + studyDate.substr(4, 2) + "-" + studyDate.substr(6, 2);
            cout << "Study Date: " << formattedStudyDate << endl;
            // Study Time
            string studyTime = loadedDataSet.getString(dicomhero::TagId(dicomhero::tagId_t::StudyTime_0008_0030), 0);
            std::string formattedStudyTime = studyTime.substr(0, 2) + ":" + studyTime.substr(2, 2) + ":" + studyTime.substr(4, 2);
            cout << "Study Time: " << formattedStudyTime << endl;
            // Study Description
            string studyDescription = loadedDataSet.getString(dicomhero::TagId(dicomhero::tagId_t::StudyDescription_0008_1030), 0);
            cout << "Study Description: " << studyDescription << endl;


            // Series Instance UID
            string seriesInstanceUID = loadedDataSet.getString(dicomhero::TagId(dicomhero::tagId_t::SeriesInstanceUID_0020_000E), 0);
            cout << "Series Instance UID: " << seriesInstanceUID << endl;
            // Modality
            string modality = loadedDataSet.getString(dicomhero::TagId(dicomhero::tagId_t::Modality_0008_0060), 0);
            cout << "Modality: " << modality << endl;
            // Body Part Examined
            string bodyPartExamined = loadedDataSet.getString(dicomhero::TagId(dicomhero::tagId_t::BodyPartExamined_0018_0015), 0);
            cout << "Body Part Examined: " << bodyPartExamined << endl;
            // Series Description
            string seriesDescription = loadedDataSet.getString(dicomhero::TagId(dicomhero::tagId_t::SeriesDescription_0008_103E), 0);
            cout << "Series Description: " << seriesDescription << endl;

            //sprawdzenie czy w bazie jest juz taki Patient z takim samym dicomid:
            retcode = exist_check("Patient", "dicomid", patientID, hdbc);


            if(retcode != SQL_NO_DATA) {
                cout<<"Pacjent o takim id juz istnieje w bazie"<<endl;
            }
            //jesli nie ma takiego badania to wprowadzenie go do bazy:
            else {
                //dodanie nowego pacjenta
                cout << "Wprowadzam nowego pacjenta do bazy..." << endl;
                retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
                // CHECK_ERROR(retcode, "SQLAllocHandle(SQL_HANDLE_STMT)", hstmt, SQL_HANDLE_STMT);
                
                string komenda1 = "INSERT INTO Patient(birthdate, sex, name, dicomid) VALUES ('"+formattedBirthDate+"','"+patientSex+"','"+patientNameCharacter_S+"', '"+patientID+"')";
                cout << komenda1 << endl;
                retcode = SQLExecDirect(hstmt,(SQLCHAR*) komenda1.c_str(), SQL_NTS);
                // CHECK_ERROR(retcode, "SQLExecDirect() INSERT", hstmt, SQL_HANDLE_STMT);
                retcode = SQLFreeStmt(hstmt, SQL_DROP);
                
                // retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
                // CHECK_ERROR(retcode, "SQLAllocHandle(SQL_HANDLE_STMT)", hstmt, SQL_HANDLE_STMT);
                // string findID = "SELECT max(id) FROM pacjent";
                // retcode = SQLExecDirect(hstmt,(SQLCHAR*) findID.c_str(), SQL_NTS);
                // CHECK_ERROR(retcode, "SQLExecDirect() SELECT", hstmt, SQL_HANDLE_STMT);
                // retcode = SQLFetch(hstmt);
                // CHECK_ERROR(retcode, "SQLFetch()", hstmt, SQL_HANDLE_STMT);
                // retcode = SQLGetData(hstmt, 1, SQL_C_DEFAULT, &cMAXID, 0,  &lenMAXID);
                // CHECK_ERROR(retcode, "SQLGetData()", hstmt, SQL_HANDLE_STMT);
                // SQLFreeStmt(hstmt, SQL_DROP);
                // int pacjentIDint = cMAXID;
                // cout << pacjentIDint<< endl;
            }



            string fileName = dp->d_name;
            // string query = "INSERT INTO Image(ImagePath) VALUES ('" + pp + "')";
            // cout << query << endl;
            // retcode = SQLExecDirect(hstmt, (SQLCHAR*)query.c_str(), SQL_NTS);
            // if (retcode != SQL_SUCCESS) {
            //     cerr << "Error executing SQL statement" << endl;
            // }
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


