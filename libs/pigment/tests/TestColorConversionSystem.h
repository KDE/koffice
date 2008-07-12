/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef TestColorConversionSystem_H
#define TestColorConversionSystem_H

#include <QtTest/QtTest>

struct ModelDepthProfile {
  ModelDepthProfile( const QString& _model, const QString& _depth, const QString& _profile )
        : model(_model), depth(_depth), profile(_profile)
  {
  }
  QString model;
  QString depth;
  QString profile;
};

class TestColorConversionSystem : public QObject
{
        Q_OBJECT
    public:
        TestColorConversionSystem();
    private slots:
        void testConnections();
        void testGoodConnections();
    private:
        QList< ModelDepthProfile > listModels;
};

#endif
