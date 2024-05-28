<?xml version="1.0" encoding="UTF-8"?>
<!-- Deklaracja arkusza stylów XSLT -->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
    <!-- Określenie sposobu wyjścia danych: HTML z wcięciami -->
    <xsl:output method="html" indent="yes"/>

    <!-- Szablon główny, który dopasowuje się do korzenia dokumentu XML -->
    <xsl:template match="/">
        <html>
            <head>
                <title>Exported Data</title> <!-- Tytuł strony HTML -->
            </head>
            <body>
                <!-- Sekcja z informacjami o pacjencie -->
                <h2>Patient Information</h2>
                <table border="1">
                    <tr>
                        <!-- Nagłówki tabeli z informacjami o pacjencie -->
                        <th>ID</th>
                        <th>Birthdate</th>
                        <th>Sex</th>
                        <th>Name</th>
                        <th>DicomID</th>
                    </tr>
                    <!-- Iteracja po każdym elemencie <patient> w elemencie <data> -->
                    <xsl:for-each select="data/patient">
                        <tr>
                            <!-- Wypełnienie komórek tabeli wartościami atrybutów -->
                            <td><xsl:value-of select="@id"/></td>
                            <td><xsl:value-of select="@birthDate"/></td>
                            <td><xsl:value-of select="@sex"/></td>
                            <td><xsl:value-of select="@name"/></td>
                            <td><xsl:value-of select="@dicomID"/></td>
                        </tr>
                    </xsl:for-each>
                </table>

                <!-- Sekcja z informacjami o badaniu -->
                <h2>Study Information</h2>
                <table border="1">
                    <tr>
                        <!-- Nagłówki tabeli z informacjami o badaniu -->
                        <th>ID</th>
                        <th>Study UID</th>
                        <th>Study Date</th>
                        <th>Study Time</th>
                        <th>Study Description</th>
                    </tr>
                    <!-- Iteracja po każdym elemencie <study> w elemencie <data> -->
                    <xsl:for-each select="data/study">
                        <tr>
                            <!-- Wypełnienie komórek tabeli wartościami atrybutów -->
                            <td><xsl:value-of select="@id"/></td>
                            <td><xsl:value-of select="@studyUID"/></td>
                            <td><xsl:value-of select="@studyDate"/></td>
                            <td><xsl:value-of select="@studyTime"/></td>
                            <td><xsl:value-of select="@studyDescription"/></td>
                        </tr>
                    </xsl:for-each>
                </table>

                <!-- Sekcja z informacjami o serii -->
                <h2>Series Information</h2>
                <table border="1">
                    <tr>
                        <!-- Nagłówki tabeli z informacjami o serii -->
                        <th>ID</th>
                        <th>Series UID</th>
                        <th>Modality</th>
                        <th>Body Part</th>
                        <th>Series Description</th>
                    </tr>
                    <!-- Iteracja po każdym elemencie <series> w elemencie <data> -->
                    <xsl:for-each select="data/series">
                        <tr>
                            <!-- Wypełnienie komórek tabeli wartościami atrybutów -->
                            <td><xsl:value-of select="@id"/></td>
                            <td><xsl:value-of select="@seriesUID"/></td>
                            <td><xsl:value-of select="@modality"/></td>
                            <td><xsl:value-of select="@bodyPart"/></td>
                            <td><xsl:value-of select="@seriesDescription"/></td>
                        </tr>
                    </xsl:for-each>
                </table>

                <!-- Sekcja z informacjami o obrazach -->
                <h2>Image Information</h2>
                <table border="1">
                    <tr>
                        <!-- Nagłówki tabeli z informacjami o obrazach -->
                        <th>ID</th>
                        <th>Image Path</th>
                        <th>Image</th>
                    </tr>
                    <!-- Iteracja po każdym elemencie <image> w elemencie <data> -->
                    <xsl:for-each select="data/image">
                        <tr>
                            <!-- Wypełnienie komórek tabeli wartościami atrybutów -->
                            <td><xsl:value-of select="@id"/></td>
                            <td><xsl:value-of select="@imagePath"/></td>
                            <td>
                                <!-- Wstawienie obrazka za pomocą ścieżki z atrybutu imagePath -->
                                <img>
                                    <xsl:attribute name="src">
                                        <xsl:value-of select="@imagePath"/>
                                    </xsl:attribute>
                                </img>
                            </td>
                        </tr>
                    </xsl:for-each>
                </table>
            </body>
        </html>
    </xsl:template>
</xsl:stylesheet>
