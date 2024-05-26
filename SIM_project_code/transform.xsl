<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
    <xsl:output method="html" indent="yes"/>

    <xsl:template match="/">
        <html>
            <head>
                <title>Exported Data</title>
            </head>
            <body>
                <h2>Patient Information</h2>
                <table border="1">
                    <tr>
                        <th>ID</th>
                        <th>Birthdate</th>
                        <th>Sex</th>
                        <th>Name</th>
                        <th>DicomID</th>
                    </tr>
                    <xsl:for-each select="data/patient">
                        <tr>
                            <td><xsl:value-of select="@id"/></td>
                            <td><xsl:value-of select="@birthDate"/></td>
                            <td><xsl:value-of select="@sex"/></td>
                            <td><xsl:value-of select="@name"/></td>
                            <td><xsl:value-of select="@dicomID"/></td>
                        </tr>
                    </xsl:for-each>
                </table>

                <h2>Study Information</h2>
                <table border="1">
                    <tr>
                        <th>ID</th>
                        <th>Study UID</th>
                        <th>Study Date</th>
                        <th>Study Time</th>
                        <th>Study Description</th>
                    </tr>
                    <xsl:for-each select="data/study">
                        <tr>
                            <td><xsl:value-of select="@id"/></td>
                            <td><xsl:value-of select="@studyUID"/></td>
                            <td><xsl:value-of select="@studyDate"/></td>
                            <td><xsl:value-of select="@studyTime"/></td>
                            <td><xsl:value-of select="@studyDescription"/></td>
                        </tr>
                    </xsl:for-each>
                </table>

                <h2>Series Information</h2>
                <table border="1">
                    <tr>
                        <th>ID</th>
                        <th>Series UID</th>
                        <th>Modality</th>
                        <th>Body Part</th>
                        <th>Series Description</th>
                    </tr>
                    <xsl:for-each select="data/series">
                        <tr>
                            <td><xsl:value-of select="@id"/></td>
                            <td><xsl:value-of select="@seriesUID"/></td>
                            <td><xsl:value-of select="@modality"/></td>
                            <td><xsl:value-of select="@bodyPart"/></td>
                            <td><xsl:value-of select="@seriesDescription"/></td>
                        </tr>
                    </xsl:for-each>
                </table>

                <h2>Image Information</h2>
                <table border="1">
                    <tr>
                        <th>ID</th>
                        <th>Image Path</th>
                        <th>Image</th>
                    </tr>
                    <xsl:for-each select="data/image">
                        <tr>
                            <td><xsl:value-of select="@id"/></td>
                            <td><xsl:value-of select="@imagePath"/></td>
                            <td>
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

