#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
#include <cstdlib>

namespace fs = std::filesystem;

// Function to list XML files in the current directory
void listXMLFiles(std::vector<std::string> &files) {
    for (const auto &entry : fs::directory_iterator(fs::current_path())) {
        if (entry.path().extension() == ".xml" && entry.path().stem().string().find("exported_data") != std::string::npos) {
            files.push_back(entry.path().string());
        }
    }
}

// Function to transform XML to HTML using XSLT
void transformXMLtoHTML(const std::string &xmlFile, const std::string &xsltFile, const std::string &htmlFile) {
    // Initialize XML parser and XSLT engine
    xmlSubstituteEntitiesDefault(1);
    xmlLoadExtDtdDefaultValue = 1;
    
    // Load XSLT stylesheet
    const char *params[] = { NULL };
    xsltStylesheetPtr stylesheet = xsltParseStylesheetFile((const xmlChar *)xsltFile.c_str());

    // Parse XML document
    xmlDocPtr document = xmlParseFile(xmlFile.c_str());

    // Check if XML parsing was successful
    if (document == NULL) {
        std::cerr << "Error parsing XML document: " << xmlFile << std::endl;
        return;
    }

    // Apply XSLT transformation to XML document
    xmlDocPtr result = xsltApplyStylesheet(stylesheet, document, params);

    // Save transformed result to HTML file
    xsltSaveResultToFilename(htmlFile.c_str(), result, stylesheet, 0);

    // Free resources
    xsltFreeStylesheet(stylesheet);
    xmlFreeDoc(document);
    xmlFreeDoc(result);
    xsltCleanupGlobals();
    xmlCleanupParser();
}

// Function to open HTML file in default browser
void openHTMLFile(const std::string &htmlFile) {
    std::string command = "xdg-open " + htmlFile;
    system(command.c_str());
}

int main() {
    // List XML files in the current directory
    std::vector<std::string> xmlFiles;
    listXMLFiles(xmlFiles);

    // Check if XML files are found
    if (xmlFiles.empty()) {
        std::cout << "No XML files found." << std::endl;
        return 1;
    }

    // Display list of XML files for selection
    std::cout << "Select an XML file to transform:" << std::endl;
    for (size_t i = 0; i < xmlFiles.size(); ++i) {
        std::cout << i + 1 << ": " << xmlFiles[i] << std::endl;
    }

    // Prompt user to choose an XML file
    int choice;
    std::cout << "Enter your choice (number): ";
    std::cin >> choice;

    // Validate user choice
    if (choice < 1 || choice > xmlFiles.size()) {
        std::cerr << "Invalid choice." << std::endl;
        return 1;
    }

    // Transform selected XML file to HTML
    std::string selectedXML = xmlFiles[choice - 1];
    std::string xsltFile = "transform.xsl"; // Ensure this file is in the same directory or provide the correct path
    std::string htmlFile = "output.html";
    transformXMLtoHTML(selectedXML, xsltFile, htmlFile);
    std::cout << "HTML file generated: " << htmlFile << std::endl;

    // Open HTML file in default browser
    openHTMLFile(htmlFile);

    return 0;
}



