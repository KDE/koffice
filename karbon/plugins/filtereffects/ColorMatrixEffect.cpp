/* This file is part of the KDE project
 * Copyright (c) 2009 Jan Hambrecht <jaham@gmx.net>
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "ColorMatrixEffect.h"
#include "KoFilterEffectRenderContext.h"
#include "KoXmlWriter.h"
#include "KoXmlReader.h"
#include <KLocale>
#include <QtCore/QRect>
#include <math.h>

const int MatrixRows  = 4;
const int MatrixCols  = 5;

ColorMatrixEffect::ColorMatrixEffect()
: KoFilterEffect(ColorMatrixEffectId, i18n( "Color Matrix" ))
, m_type(Matrix)
{
    setIdentity();
}

void ColorMatrixEffect::setIdentity()
{
    // set identity matrix
    for(int r = 0; r < MatrixRows; ++r) {
        for(int c = 0; c < MatrixCols; ++c) {
            m_matrix[r*MatrixCols+c] = r == c ? 1.0 : 0.0;
        }
    }
}

ColorMatrixEffect::Type ColorMatrixEffect::type() const
{
    return m_type;
}

const qreal * ColorMatrixEffect::colorMatrix() const
{
    return m_matrix;
}

void ColorMatrixEffect::setColorMatrix(qreal *colorMatrix)
{
    memcpy(m_matrix, colorMatrix, ColorMatrixElements*sizeof(qreal));
    m_type = Matrix;
}

void ColorMatrixEffect::setSaturate(qreal value)
{
    m_type = Saturate;
    m_value = qBound(0.0, value, 1.0);
    
    setIdentity();
    
    m_matrix[0] = 0.213+0.787*value;
    m_matrix[1] = 0.715-0.715*value;
    m_matrix[2] = 0.072-0.072*value;
    
    m_matrix[5] = 0.213-0.213*value;
    m_matrix[6] = 0.715+0.285*value;
    m_matrix[7] = 0.072-0.072*value;
    
    m_matrix[10] = 0.213-0.213*value;
    m_matrix[11] = 0.715-0.715*value;
    m_matrix[12] = 0.072+0.928*value;
}

qreal ColorMatrixEffect::saturate() const
{
    if (m_type == Saturate)
        return m_value;
    else
        return 1.0;
}

void ColorMatrixEffect::setHueRotate(qreal value)
{
    m_type = HueRotate;
    m_value = value;
    
    const qreal rad = m_value * M_PI / 180.0;
    const qreal c = cos(rad);
    const qreal s = sin(rad);

    setIdentity();
    
    m_matrix[0] = 0.213 + 0.787*c - 0.213*s;
    m_matrix[1] = 0.715 - 0.715*c - 0.715*s;
    m_matrix[2] = 0.072 - 0.072*c + 0.928*s;
    
    m_matrix[5] = 0.213 - 0.213*c + 0.143*s;
    m_matrix[6] = 0.715 + 0.285*c + 0.140*s;
    m_matrix[7] = 0.072 - 0.072*c - 0.283*s;
    
    m_matrix[10] = 0.213 - 0.213*c - 0.787*s;
    m_matrix[11] = 0.715 - 0.715*c + 0.715*s;
    m_matrix[12] = 0.072 + 0.928*c + 0.072*s;
}

qreal ColorMatrixEffect::hueRotate() const
{
    if (m_type == HueRotate)
        return m_value;
    else
        return 0.0;
}

void ColorMatrixEffect::setLuminanceAlpha()
{
    m_type = LuminanceAlpha;

    memset(m_matrix, 0, ColorMatrixElements*sizeof(qreal));
    
    m_matrix[15] = 0.2125;
    m_matrix[16] = 0.7154;
    m_matrix[17] = 0.0721;
    m_matrix[18] = 0.0;
}

QImage ColorMatrixEffect::processImage(const QImage &image, const KoFilterEffectRenderContext &context) const
{
    QImage result = image;
    
    QRgb *src = (QRgb*)image.bits();
    QRgb *dst = (QRgb*)result.bits();
    int w = result.width();
    
    const qreal * m = m_matrix;
    qreal sa, sr, sg, sb;
    qreal da, dr, dg, db;
    
    QRect roi = context.filterRegion().toRect();
    for( int row = roi.top(); row < roi.bottom(); ++row) {
        for(int col = roi.left(); col < roi.right(); ++col) {
            const QRgb &s = src[row*w+col];
            sa = qAlpha(s)/255.0;
            sr = qRed(s)/255.0;
            sb = qBlue(s)/255.0;
            sg = qGreen(s)/255.0;
            // the matrix is applied to non-premultiplied color values
            // so we have to convert colors by dividing by alpha value
            if (sa == 0.0) {
                // alpha is zero (fully transparent)
                //sr = sb = sg = 0.0;
            } else if (sa != 1.0) {
                sr /= sa;
                sb /= sa;
                sg /= sa;
            }
            
            // apply matrix to color values
            dr = m[ 0]*sr + m[ 1]*sg + m[ 2]*sb + m[ 3]*sa + m[ 4];
            dg = m[ 5]*sr + m[ 6]*sg + m[ 7]*sb + m[ 8]*sa + m[ 9];
            db = m[10]*sr + m[11]*sg + m[12]*sb + m[13]*sa + m[14];
            da = m[15]*sr + m[16]*sg + m[17]*sb + m[18]*sa + m[19];
            
            // the new alpha value
            da *= 255.0;
            
            // set pre-multiplied color values on destination image
            dst[row*w+col] = qRgba(static_cast<quint8>(qBound(0.0, dr*da, 255.0)),
                                   static_cast<quint8>(qBound(0.0, dg*da, 255.0)),
                                   static_cast<quint8>(qBound(0.0, db*da, 255.0)),
                                   static_cast<quint8>(qBound(0.0, da, 255.0)));
        }
    }
    
    return result;
}

bool ColorMatrixEffect::load(const KoXmlElement &element, const QMatrix &matrix)
{
    Q_UNUSED(matrix);
    
    if (element.tagName() != id())
        return false;

    QString typeStr = element.attribute("type");
    if (typeStr.isEmpty())
        return false;
    
    QString valueStr = element.attribute("values");
    
    setIdentity();
    m_type = Matrix;
    
    if (typeStr == "matrix") {
        // values are separated by whitespace and/or comma
        QStringList values = valueStr.trimmed().split( QRegExp("(\\s+|,)"), QString::SkipEmptyParts );
        if (values.count() == ColorMatrixElements) {
            for (int i = 0; i < ColorMatrixElements; ++i) {
                m_matrix[i] = values[i].toDouble();
            }
        }
    } else if (typeStr == "saturate") {
        if (!valueStr.isEmpty()) {
            setSaturate(valueStr.toDouble());
        }
    } else if (typeStr == "hueRotate") {
        if (!valueStr.isEmpty()) {
            setHueRotate(valueStr.toDouble());
        }
    } else if (typeStr == "luminanceToAlpha") {
        setLuminanceAlpha();
    } else {
        return false;
    }
    
    return true;
}

void ColorMatrixEffect::save(KoXmlWriter &writer)
{
    writer.startElement(ColorMatrixEffectId);
    
    saveCommonAttributes(writer);
    
    switch(m_type) {
    case Matrix:
        {
            writer.addAttribute("type", "matrix");
            QString matrix;
            for (int r = 0; r < MatrixRows; ++r) {
                for (int c = 0; c < MatrixCols; ++c) {
                    matrix += QString("%1 ").arg(m_matrix[r*MatrixCols+c]);
                }
            }
            writer.addAttribute("values", matrix);
        }
        break;
    case Saturate:
        writer.addAttribute("type", "saturate");
        writer.addAttribute("values", QString("%1").arg(m_value));
        break;
    case HueRotate:
        writer.addAttribute("type", "hueRotate");
        writer.addAttribute("values", QString("%1").arg(m_value));
        break;
    case LuminanceAlpha:
        writer.addAttribute("type", "luminanceToAlpha");
        break;
    }

    writer.endElement();
}
