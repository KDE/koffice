/* This file is part of the KDE project
   Copyright (C) 2002 Lars Siebold <khandha5@gmx.net>
   Copyright (C) 2002 Werner Trobin <trobin@kde.org>
   Copyright (C) 2002 Lennart Kudling <kudling@kde.org>
   Copyright (C) 2002-2003,2005 Rob Buis <buis@kde.org>
   Copyright (C) 2005 Boudewijn Rempt <boud@valdyas.org>
   Copyright (C) 2005 Raphael Langerhorst <raphael.langerhorst@kdemail.net>
   Copyright (C) 2005 Thomas Zander <zander@kde.org>
   Copyright (C) 2005,2008 Jan Hambrecht <jaham@gmx.net>
   Copyright (C) 2006 Inge Wallin <inge@lysator.liu.se>
   Copyright (C) 2006 Laurent Montel <montel@kde.org>

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
 * Boston, MA 02110-1301, USA.
*/

#ifndef SVGEXPORT_H
#define SVGEXPORT_H

#include <KoFilter.h>
#include <QtGui/QGradient>

class KarbonDocument;
class KoShapeLayer;
class KoShapeContainer;
class KoShape;
class KoPathShape;
class KoShapeBorderModel;
class ArtisticTextShape;
class PictureShape;
class KoEllipseShape;
class KoRectangleShape;
class KoPatternBackground;
class QTextStream;
class QPixmap;
class QImage;
class QColor;
class QBrush;

class SvgExport : public KoFilter
{
    Q_OBJECT

public:
    SvgExport( QObject* parent, const QStringList& );
    virtual ~SvgExport() {}

    virtual KoFilter::ConversionStatus convert( const QByteArray& from, const QByteArray& to );

private:
    void saveDocument( KarbonDocument& document );
    void saveLayer( KoShapeLayer * layer );
    void saveGroup( KoShapeContainer * group );
    void saveShape( KoShape * shape );
    void savePath( KoPathShape * path );
    void saveEllipse( KoEllipseShape * ellipse );
    void saveRectangle( KoRectangleShape * rectangle );

    void saveImage( PictureShape * picture );
    void saveText( ArtisticTextShape * text );

    void getStyle( KoShape * shape, QTextStream * stream );
    void getFill( KoShape * shape, QTextStream *stream );
    void getStroke( KoShape * shape, QTextStream *stream );
    void getColorStops( const QGradientStops & colorStops );
    void getGradient( const QGradient * gradient, const QMatrix &gradientTransform );
    void getPattern( KoPatternBackground * pattern, KoShape * shape );
    void getHexColor( QTextStream *, const QColor & color  );
    QString getTransform( const QMatrix &matrix, const QString &attributeName );

    QString getID( const KoShape *obj );
    QString createID( const KoShape * obj );

    /// Checks if the matrix only has translation set
    bool isTranslation( const QMatrix & );

    QTextStream* m_stream;
    QTextStream* m_defs;
    QTextStream* m_body;

    unsigned int m_indent;
    unsigned int m_indent2;

    QMap<const KoShape*, QString> m_shapeIds;

    QMatrix m_userSpaceMatrix;

};

#endif

