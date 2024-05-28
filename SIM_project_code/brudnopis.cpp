#include <dirent.h> // For directory handling
#include <cstring>  // For string manipulation
#include <iostream> // For standard I/O operations
#include <string>   // For using std::string
#include "dicomhero/dicomhero.hpp" // Assuming this is the DICOM library you are using

using namespace std;

// Helper function to check if a file is a DICOM file
bool isDICOMFile(const string &filePath) {
    try {
        // Attempt to load the file as a DICOM file with a buffer size of 2048 bytes
        dicomhero::DataSet loadedDataSet(dicomhero::CodecFactory::load(filePath, 2048));
        return true; // If loading is successful, return true
    } catch (...) {
        return false; // If any exception is caught, return false
    }
}

// Recursive function to search directories for files and process DICOM files
void tree(string p, SQLHDBC hdbc) {
    // Open the directory specified by path 'p'
    DIR *dirp = opendir(p.c_str());
    if (dirp == nullptr) {
        cerr << "Error opening directory: " << p << endl;
        return; // If directory cannot be opened, print an error and return
    }

    dirent *dp; // Pointer to directory entry structure
    int dir_id = 0; // Index for directories
    int file_id = 0; // Index for files

    // Iterate over all entries in the directory
    while ((dp = readdir(dirp)) != NULL) {
        // Check if the entry is a directory (and not the special entries '.' or '..')
        if (dp->d_type == DT_DIR && strcmp(dp->d_name, "..") != 0 && strcmp(dp->d_name, ".") != 0) {
            dir_id++; // Increment directory index
            string pp = p + "/" + dp->d_name; // Construct new path for the subdirectory
            tree(pp, hdbc); // Recursively call tree for the subdirectory
        } else if (dp->d_type == DT_REG) { // Check if the entry is a regular file
            file_id++; // Increment file index
            string pp = p + "/" + dp->d_name; // Construct full path for the file

            // Try to open the file as a DICOM file
            if (isDICOMFile(pp)) {
                // Allocate a statement handle for SQL operations
                SQLHSTMT hstmt;
                SQLRETURN retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
                if (retcode != SQL_SUCCESS) {
                    cerr << "Error allocating statement handle" << endl;
                    continue; // Skip the current file and proceed to the next one if allocation fails
                }

                // Load and process the DICOM file
                dicomhero::DataSet loadedDataSet(dicomhero::CodecFactory::load(pp, 2048)); // Load DICOM file
                dicomhero::Image image(loadedDataSet.getImageApplyModalityTransform(0)); // Get the image from the dataset
                string jpgdir = createJPG_directory(p); // Create directory for saving JPEG files
                string jpgname = dp->d_name; // Use the original file name
                jpgname = jpgdir + "/" + jpgname + ".jpg"; // Construct full path for the JPEG file

                // Save the image as a JPEG file
                dicomhero::CodecFactory::save(loadedDataSet, jpgname, dicomhero::codecType_t::jpeg);

                // Free the allocated statement handle
                SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
            }
        }
    }
    // Close the directory
    closedir(dirp);
}
