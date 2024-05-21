import os
import pydicom
from datetime import datetime


def change_meta(dcm_pathseries_directory):
    # Iterate through all DICOM files in the directory
    for root, dirs, files in os.walk(series_directory):
        for filename in files:
            # if filename.endswith(".dcm"):
                dicom_file_path = os.path.join(root, filename)
                dicom_data = pydicom.dcmread(dicom_file_path)

                # Modify the metadata as needed
                dicom_data.PatientName = ""
                dicom_data.PatientID = ""
                dicom_data.PatientSex = ""
                birthdate = datetime.strptime("0000-00-00", "%Y-%m-%d")
                dicom_data.PatientBirthDate = birthdate.strftime("%Y%m%d")
                # dicom_data.SeriesDescription = ""
                #study_description = "RTG klatki piersiowej - PA"
                # dicom_data.StudyDescription = study_description
                # print(dicom_data.StudyID)

                dicom_data.save_as(dicom_file_path)


if __name__ == '__main__':
    # Directory containing DICOM files of the series
    series_directory = "..."
    change_meta(series_directory)


