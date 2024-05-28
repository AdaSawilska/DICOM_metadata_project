#include <iostream>
#include <stdexcept>
#include <string>

#include <iostream>
#include <stdexcept>
#include <string>

// Assuming dicomhero namespace and relevant classes are already defined elsewhere
// #include "dicomhero.h"

int main() {
    try {
        // Patient Name
        dicomhero::UnicodePatientName patientName;
        wstring patientNameCharacter;
        string patientNameCharacter_S;
        
        try {
            patientName = loadedDataSet.getUnicodePatientName(dicomhero::TagId(dicomhero::tagId_t::PatientName_0010_0010), 0);
            patientNameCharacter = patientName.getAlphabeticRepresentation();
            patientNameCharacter_S = string(patientNameCharacter.begin(), patientNameCharacter.end());
        } catch (const std::exception &e) {
            patientNameCharacter_S = "";
        }
        
        // cout << "Patient Name: " << patientNameCharacter_S << endl;
    } catch (const std::exception &e) {
        std::cerr << "An unexpected error occurred: " << e.what() << std::endl;
    }
    return 0;
}


int main() {
    try {
        // Study ID
        string studyID;
        try {
            studyID = loadedDataSet.getString(dicomhero::TagId(dicomhero::tagId_t::StudyID_0020_0010), 0);
        } catch (const std::exception &e) {
            studyID = "";
        }
        // cout << "Study ID: " << studyID << endl;

        // Study Instance UID
        string studyInstanceUID;
        try {
            studyInstanceUID = loadedDataSet.getString(dicomhero::TagId(dicomhero::tagId_t::StudyInstanceUID_0020_000D), 0);
        } catch (const std::exception &e) {
            studyInstanceUID = "";
        }
        // cout << "Study Instance UID: " << studyInstanceUID << endl;

        // Study Date
        string studyDate;
        try {
            studyDate = loadedDataSet.getString(dicomhero::TagId(dicomhero::tagId_t::StudyDate_0008_0020), 0);
        } catch (const std::exception &e) {
            studyDate = "";
        }
        string formattedStudyDate;
        if (!studyDate.empty()) {
            formattedStudyDate = studyDate.substr(0, 4) + "-" + studyDate.substr(4, 2) + "-" + studyDate.substr(6, 2);
        } else {
            formattedStudyDate = "";
        }
        // cout << "Study Date: " << formattedStudyDate << endl;

        // Study Time
        string studyTime;
        try {
            studyTime = loadedDataSet.getString(dicomhero::TagId(dicomhero::tagId_t::StudyTime_0008_0030), 0);
        } catch (const std::exception &e) {
            studyTime = "";
        }
        std::string formattedStudyTime;
        if (!studyTime.empty()) {
            formattedStudyTime = studyTime.substr(0, 2) + ":" + studyTime.substr(2, 2) + ":" + studyTime.substr(4, 2);
        } else {
            formattedStudyTime = "";
        }
        // cout << "Study Time: " << formattedStudyTime << endl;

        // Study Description
        string studyDescription;
        try {
            studyDescription = loadedDataSet.getString(dicomhero::TagId(dicomhero::tagId_t::StudyDescription_0008_1030), 0);
        } catch (const std::exception &e) {
            studyDescription = "";
        }
        // cout << "Study Description: " << studyDescription << endl;

        // Series Instance UID
        string seriesInstanceUID;
        try {
            seriesInstanceUID = loadedDataSet.getString(dicomhero::TagId(dicomhero::tagId_t::SeriesInstanceUID_0020_000E), 0);
        } catch (const std::exception &e) {
            seriesInstanceUID = "";
        }
        // cout << "Series Instance UID: " << seriesInstanceUID << endl;

        // Modality
        string modality;
        try {
            modality = loadedDataSet.getString(dicomhero::TagId(dicomhero::tagId_t::Modality_0008_0060), 0);
        } catch (const std::exception &e) {
            modality = "";
        }
        // cout << "Modality: " << modality << endl;

        // Body Part Examined
        string bodyPartExamined;
        try {
            bodyPartExamined = loadedDataSet.getString(dicomhero::TagId(dicomhero::tagId_t::BodyPartExamined_0018_0015), 0);
        } catch (const std::exception &e) {
            bodyPartExamined = "";
        }
        // cout << "Body Part Examined: " << bodyPartExamined << endl;

        // Series Description
        string seriesDescription;
        try {
            seriesDescription = loadedDataSet.getString(dicomhero::TagId(dicomhero::tagId_t::SeriesDescription_0008_103E), 0);
        } catch (const std::exception &e) {
            seriesDescription = "";
        }
        // cout << "Series Description: " << seriesDescription << endl;
    } catch (const std::exception &e) {
        std::cerr << "An unexpected error occurred: " << e.what() << std::endl;
    }
    return 0;
}
