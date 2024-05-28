#include <iostream>          // Zawiera standardowe strumienie wejścia i wyjścia
#include <filesystem>        // Zawiera funkcje do pracy z systemem plików
#include <vector>            // Zawiera kontener wektorowy
#include <string>            // Zawiera funkcje i klasy do pracy z ciągami znaków
#include <libxml/parser.h>   // Zawiera funkcje do parsowania plików XML
#include <libxml/xpath.h>    // Zawiera funkcje do obsługi XPath w XML
#include <libxslt/xslt.h>    // Zawiera funkcje do transformacji XSLT
#include <libxslt/xsltInternals.h> // Zawiera wewnętrzne funkcje XSLT
#include <libxslt/transform.h>     // Zawiera funkcje do transformacji dokumentów XML za pomocą XSLT
#include <libxslt/xsltutils.h>     // Zawiera dodatkowe narzędzia XSLT
#include <cstdlib>           // Zawiera funkcje standardowe, takie jak system()

namespace fs = std::filesystem; // Używamy krótszego aliasu dla std::filesystem

// Funkcja do listowania plików XML w bieżącym katalogu
void listXMLFiles(std::vector<std::string> &files) {
    // Iterujemy po wszystkich plikach w bieżącym katalogu
    for (const auto &entry : fs::directory_iterator(fs::current_path())) {
        // Sprawdzamy, czy plik ma rozszerzenie .xml i zawiera w nazwie "exported_data"
        if (entry.path().extension() == ".xml" && entry.path().stem().string().find("exported_data") != std::string::npos) {
            // Dodajemy ścieżkę pliku do wektora files
            files.push_back(entry.path().string());
        }
    }
}

// Funkcja do transformacji pliku XML na HTML za pomocą XSLT
void transformXMLtoHTML(const std::string &xmlFile, const std::string &xsltFile, const std::string &htmlFile) {
    // Inicjalizujemy parser XML i silnik XSLT
    xmlSubstituteEntitiesDefault(1);
    xmlLoadExtDtdDefaultValue = 1;
    
    // Ładujemy arkusz stylów XSLT
    const char *params[] = { NULL };
    xsltStylesheetPtr stylesheet = xsltParseStylesheetFile((const xmlChar *)xsltFile.c_str());

    // Parsujemy dokument XML
    xmlDocPtr document = xmlParseFile(xmlFile.c_str());

    // Sprawdzamy, czy parsowanie XML zakończyło się sukcesem
    if (document == NULL) {
        std::cerr << "Error parsing XML document: " << xmlFile << std::endl;
        return;
    }

    // Stosujemy transformację XSLT na dokumencie XML
    xmlDocPtr result = xsltApplyStylesheet(stylesheet, document, params);

    // Zapisujemy wynik transformacji do pliku HTML
    xsltSaveResultToFilename(htmlFile.c_str(), result, stylesheet, 0);

    // Zwalniamy zasoby
    xsltFreeStylesheet(stylesheet);
    xmlFreeDoc(document);
    xmlFreeDoc(result);
    xsltCleanupGlobals();
    xmlCleanupParser();
}

// Funkcja do otwierania pliku HTML w domyślnej przeglądarce
void openHTMLFile(const std::string &htmlFile) {
    // Tworzymy polecenie do otwarcia pliku HTML
    std::string command = "xdg-open " + htmlFile;
    system(command.c_str());
}

int main() {
    // Listujemy pliki XML w bieżącym katalogu
    std::vector<std::string> xmlFiles;
    listXMLFiles(xmlFiles);

    // Sprawdzamy, czy znaleziono pliki XML
    if (xmlFiles.empty()) {
        std::cout << "No XML files found." << std::endl;
        return 1;
    }

    // Wyświetlamy listę znalezionych plików XML do wyboru
    std::cout << "Select an XML file to transform:" << std::endl;
    for (size_t i = 0; i < xmlFiles.size(); ++i) {
        std::cout << i + 1 << ": " << xmlFiles[i] << std::endl;
    }

    // Pobieramy wybór użytkownika
    int choice;
    std::cout << "Enter your choice (number): ";
    std::cin >> choice;

    // Walidujemy wybór użytkownika
    if (choice < 1 || choice > xmlFiles.size()) {
        std::cerr << "Invalid choice." << std::endl;
        return 1;
    }

    // Transformujemy wybrany plik XML na HTML
    std::string selectedXML = xmlFiles[choice - 1];
    std::string xsltFile = "transform.xsl"; // Upewnij się, że ten plik znajduje się w tym samym katalogu lub podaj poprawną ścieżkę
    std::string htmlFile = "output.html";
    transformXMLtoHTML(selectedXML, xsltFile, htmlFile);
    std::cout << "HTML file generated: " << htmlFile << std::endl;

    // Otwieramy plik HTML w domyślnej przeglądarce
    openHTMLFile(htmlFile);

    return 0;
}
