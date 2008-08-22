/* This file is part of the KDE project
 * Copyright (C) 2008 Fredy Yanardi <fyanardi@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KPRCONFIGURESLIDESHOWDIALOG_H
#define KPRCONFIGURESLIDESHOWDIALOG_H

#include <KDialog>

#include "ui_KPrConfigureSlideShow.h"

class KPrDocument;

class KPrConfigureSlideShowDialog : public KDialog
{
    Q_OBJECT
public:
    explicit KPrConfigureSlideShowDialog( KPrDocument *document, QWidget *parent=0 );

    QString activeCustomSlideShow() const;

private slots:
    void editCustomSlideShow();

private:
    Ui::KPrConfigureSlideShow ui;
    KPrDocument *m_document;
};

#endif // KPRCONFIGUREPRESENTERVIEWDIALOG_H

