/*
 *  Copyright (c) 2003 Boudewijn Rempt (boud@valdyas.org)
 *
 *  This program is free software; you can distribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef GRAY_U16_PLUGIN_H_
#define GRAY_U16_PLUGIN_H_

#include <QObject>

/**
 * A plugin wrapper around the GRAY U16 color space strategy.
 */
class GRAYU16Plugin : public QObject
{
    Q_OBJECT
public:
    GRAYU16Plugin(QObject *parent, const QStringList &);
    virtual ~GRAYU16Plugin();

};


#endif // GRAY_U16_PLUGIN_H_
