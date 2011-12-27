/* This file is part of the KDE project
   Copyright (C) 2007 Martin Pfeiffer <hubipete@gmx.net>

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
   Boston, MA 02110-1301, USA.
*/

#ifndef SCPAGEEFFECTDOCKER_H
#define SCPAGEEFFECTDOCKER_H

#include <QWidget>

class KComboBox;
class QDoubleSpinBox;
class SCPageEffect;
class SCPageEffectFactory;
class KoPAView;
class SCViewModePreviewPageEffect;

/**
 * This is the page effect docker widget that let's you choose a page animation.
 */
class SCPageEffectDocker : public QWidget
{

    Q_OBJECT
public:
    explicit SCPageEffectDocker(QWidget* parent = 0, Qt::WindowFlags flags = 0);

    void setView(KoPAView* view);

public slots:
    void slotActivePageChanged();

    void slotEffectChanged(int index);

protected:
    void updateSubTypes(const SCPageEffectFactory * factory);
    SCPageEffect * createPageEffect(const SCPageEffectFactory * factory, int subType, double time);

protected slots:
    void slotSubTypeChanged(int index);
    void slotDurationChanged(double duration);

    void cleanup(QObject* object);

    void setEffectPreview();

private:
    KoPAView* m_view;
    KComboBox* m_effectCombo;
    KComboBox* m_subTypeCombo;
    QDoubleSpinBox* m_durationSpinBox;
    SCViewModePreviewPageEffect *m_previewMode;
};

#endif // SCPAGEEFFECTDOCKER_H
