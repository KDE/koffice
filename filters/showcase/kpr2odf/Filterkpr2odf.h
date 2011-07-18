/* This file is part of the KDE project
   Copyright (C) 2008 Carlos Licea <carlos.licea@kdemail.net>
   Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA  02110-1301  USA.
*/

#ifndef FILTERKPR2ODF_H
#define FILTERKPR2ODF_H

//Qt's includes
#include <QHash>
#include <QList>
#include <QSizeF>
#include <QPointF>

//KOffice includes
#include <KoFilter.h>
#include <KOdfGenericStyles.h>
#include <QVariantList>
#include <KXmlReader.h>

class KXmlWriter;
class KOdfGenericStyles;
class KXmlNode;
class KXmlElement;

class Filterkpr2odf : public KoFilter
{
    Q_OBJECT

public:
    Filterkpr2odf(QObject* parent, const QVariantList&);

    virtual ~Filterkpr2odf() {}

    //reimplemented from KoFilter
    virtual KoFilter::ConversionStatus convert(const QByteArray& from, const QByteArray& to);

private:
    //Helper functions
    void createImageList(KOdfStore* output, KOdfStore* input, KXmlWriter* manifest);
    void createSoundList(KOdfStore* output, KOdfStore* input, KXmlWriter* manifest);
    void convertContent(KXmlWriter* content);
    void convertObjects(KXmlWriter* content, const KXmlNode& objects);
    void saveAnimations(KXmlWriter* content);

    //Objects' functions
    void appendPicture(KXmlWriter* content, const KXmlElement& objectElement);
    void appendLine(KXmlWriter* content, const KXmlElement& objectElement);
    void appendRectangle(KXmlWriter* content, const KXmlElement& objectElement);
    void appendEllipse(KXmlWriter* content, const KXmlElement& objectElement);
    void appendTextBox(KXmlWriter* content, const    KXmlElement& objectElement);
    void appendParagraph(KXmlWriter* content, const    KXmlElement& objectElement);
    void appendText(KXmlWriter* content, const    KXmlElement& objectElement);
    void appendPie(KXmlWriter* content, const KXmlElement& objectElement);
    void appendGroupObject(KXmlWriter* content, const KXmlElement& objectElement);
    void appendPoly(KXmlWriter* content, const KXmlElement& objectElement, bool polygon);
    void appendPolygon(KXmlWriter* content, const KXmlElement& objectElement);
    void appendAutoform(KXmlWriter* content, const KXmlElement& objectElement);
    void appendArrow(KXmlWriter* content, const KXmlElement& objectElement);
    void appendFreehand(KXmlWriter* content, const KXmlElement& objectElement);
    void appendBezier(KXmlWriter* content, const KXmlElement& objectElement);

    const QString getPictureNameFromKey(const KXmlElement& key);
    void set2DGeometry(KXmlWriter* content, const KXmlElement& objectElement);
    QString rotateValue(double val);
    void exportAnimation(const KXmlElement& objectElement, int indentLevel);

    //The next functions are used to correctly export arcs, pies and chords
    //And they were obteined from the 1.6 sources
    void getRealSizeAndOrig(QSizeF& realSize, QPointF& realOrig, int startAngle, int endAngle, int angle, int pieType);
    void setEndPoints(QPointF points[], const QSizeF& size, int startAngle, int endAngle);
    void setMinMax(double &min_x, double &min_y, double &max_x, double &max_y, QPointF point);

    //Styles functions
    const QString createPageStyle(const KXmlElement& page);
    const QString createGradientStyle(const KXmlElement& page);
    const QString createGraphicStyle(const KXmlElement& page);
    const QString createPageLayout();//we use more than one tag, better load them from m_mainDoc
    const QString createMasterPageStyle(const KXmlNode & objects, const KXmlElement & masterBackground);  //same as above
    const QString createOpacityGradientStyle(int opacity);
    const QString createMarkerStyle(int markerType);
    const QString createStrokeDashStyle(int strokeStyle);
    const QString createHatchStyle(int brushStyle, QString fillColor);
    const QString createParagraphStyle(const KXmlElement& element);
    const QString createTextStyle(const KXmlElement& element);
    const QString createListStyle(const KXmlElement& element);

    QString convertBorder(const KXmlElement& border);

    KXmlDocument m_mainDoc;//from KPR
    KXmlDocument m_documentInfo;//from KPR

    QHash< int, QList<QString> > m_pageAnimations;//stores the animations, needed a hash to be able to sort them

    int m_pageHeight;//needed to find out where's every object
    int m_currentPage;
    int m_objectIndex;//the number of the next object
    QHash<QString, QString> m_pictures;//store the <fullFilename, name> pair of the keys
    QHash<QString, QString> m_sounds;//store the <fullFilename, name> pair of the keys
    bool m_sticky; // set to true when we want to read objects from the master page

    KOdfGenericStyles m_styles;//style collector
};

#endif //FILTERKPR2ODF_H
