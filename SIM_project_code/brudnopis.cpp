#include <dirent.h>
#include <cstring>
#include <iostream>
#include <string>
#include "dicomhero/dicomhero.hpp" // Assuming this is the library you are using

using namespace std;

bool isDICOMFile(const string &filePath) {
    try {
        dicomhero::DataSet loadedDataSet(dicomhero::CodecFactory::load(filePath, 2048));
        return true;
    } catch (...) {
        return false;
    }
}

void tree(string p, SQLHDBC hdbc) {
    DIR *dirp = opendir(p.c_str());
    dirent *dp;
    int dir_id = 0;
    int file_id = 0;

    while ((dp = readdir(dirp)) != NULL) {
        if (dp->d_type == DT_DIR && strcmp(dp->d_name, "..") != 0 && strcmp(dp->d_name, ".") != 0) {
            dir_id++;
            string pp = p + "/" + dp->d_name;
            tree(pp, hdbc);
        } else if (dp->d_type == DT_REG) {
            file_id++;
            string pp = p + "/" + dp->d_name;

            // Try to open the file as a DICOM file
            if (isDICOMFile(pp)) {
                SQLHSTMT hstmt;
                SQLRETURN retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
                if (retcode != SQL_SUCCESS) {
                    cerr << "Error allocating statement handle" << endl;
                    continue;
                }
                // Load and process the DICOM file
                dicomhero::DataSet loadedDataSet(dicomhero::CodecFactory::load(pp, 2048));
                dicomhero::Image image(loadedDataSet.getImageApplyModalityTransform(0));
                string jpgdir = createJPG_directory(p);
                string jpgname = dp->d_name;
                jpgname = jpgdir + "/" + jpgname + ".jpg";
                dicomhero::CodecFactory::save(loadedDataSet, jpgname, dicomhero::codecType_t::jpeg);

                SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
            }
        }
    }
    closedir(dirp);
}
