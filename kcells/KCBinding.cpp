/* This file is part of the KDE project
   Copyright 2007 Stefan Nikolaus <stefan.nikolaus@kdemail.net>
    Copyright (C) 2008 Thomas Zander <zander@kde.org>

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

#include "KCBinding.h"
#include "KCBindingModel.h"

#include <QRect>

#include <kdebug.h>

#include "KCCellStorage.h"
#include "KCMap.h"
#include "KCSheet.h"
#include "KCValue.h"

class KCBinding::Private : public QSharedData
{
public:
    KCBindingModel* model;
    Private(KCBinding *q) : model(new KCBindingModel(q)) {}
    ~Private() { delete model; }
};


KCBinding::KCBinding()
    : d(new Private(this))
{
}

KCBinding::KCBinding(const KCRegion& region)
    : d(new Private(this))
{
    Q_ASSERT(region.isValid());
    d->model->setRegion(region);
}

KCBinding::KCBinding(const KCBinding& other)
    : d(other.d)
{
}

KCBinding::~KCBinding()
{
}

bool KCBinding::isEmpty() const
{
    return d->model->region().isEmpty();
}

QAbstractItemModel* KCBinding::model() const
{
    return d->model;
}

const KCRegion& KCBinding::region() const
{
    return d->model->region();
}

void KCBinding::setRegion(const KCRegion& region)
{
    d->model->setRegion(region);
}

void KCBinding::update(const KCRegion& region)
{
    QRect rect;
    KCRegion changedRegion;
    const QPoint offset = d->model->region().firstRange().topLeft();
    const QRect range = d->model->region().firstRange();
    const KCSheet* sheet = d->model->region().firstSheet();
    KCRegion::ConstIterator end(region.constEnd());
    for (KCRegion::ConstIterator it = region.constBegin(); it != end; ++it) {
        if (sheet != (*it)->sheet())
            continue;
        rect = range & (*it)->rect();
        rect.translate(-offset.x(), -offset.y());
        if (rect.isValid()) {
            d->model->emitDataChanged(rect);
            changedRegion.add(rect, (*it)->sheet());
        }
    }
    d->model->emitChanged(changedRegion);
}

void KCBinding::operator=(const KCBinding & other)
{
    d = other.d;
}

bool KCBinding::operator==(const KCBinding& other) const
{
    return d == other.d;
}

bool KCBinding::operator<(const KCBinding& other) const
{
    return d < other.d;
}

QHash<QString, QVector<QRect> > KCBindingModel::cellRegion() const
{
    QHash<QString, QVector<QRect> > answer;
    KCRegion::ConstIterator end = m_region.constEnd();
    for (KCRegion::ConstIterator it = m_region.constBegin(); it != end; ++it) {
        if (!(*it)->isValid()) {
            continue;
        }
        answer[(*it)->name()].append((*it)->rect());
    }
    return answer;
}

bool KCBindingModel::setCellRegion(const QString& regionName)
{
    Q_ASSERT(m_region.isValid());
    Q_ASSERT(m_region.firstSheet());
    const KCMap* const map = m_region.firstSheet()->map();
    const KCRegion region = KCRegion(regionName, map);
    if (!region.isValid()) {
        kDebug() << qPrintable(regionName) << "is not a valid region.";
        return false;
    }
    // Clear the old binding.
    KCRegion::ConstIterator end = m_region.constEnd();
    for (KCRegion::ConstIterator it = m_region.constBegin(); it != end; ++it) {
        if (!(*it)->isValid()) {
            continue;
        }
        // FIXME Stefan: This may also clear other bindings!
        (*it)->sheet()->cellStorage()->setBinding(KCRegion((*it)->rect(), (*it)->sheet()), KCBinding());
    }
    // Set the new region
    m_region = region;
    end = m_region.constEnd();
    for (KCRegion::ConstIterator it = m_region.constBegin(); it != end; ++it) {
        if (!(*it)->isValid()) {
            continue;
        }
        (*it)->sheet()->cellStorage()->setBinding(KCRegion((*it)->rect(), (*it)->sheet()), *m_binding);
    }
    return true;
}


/////// KCBindingModel

KCBindingModel::KCBindingModel(KCBinding* binding, QObject *parent)
        : QAbstractTableModel(parent)
        , m_binding(binding)
{
}

bool KCBindingModel::isCellRegionValid(const QString& regionName) const
{
    Q_CHECK_PTR(m_region.firstSheet());
    Q_CHECK_PTR(m_region.firstSheet()->map());
    return KCRegion(regionName, m_region.firstSheet()->map()).isValid();
}

void KCBindingModel::emitChanged(const KCRegion& region)
{
    emit changed(region);
}

void KCBindingModel::emitDataChanged(const QRect& rect)
{
    const QPoint tl = rect.topLeft();
    const QPoint br = rect.bottomRight();
    //kDebug(36005) << "emit QAbstractItemModel::dataChanged" << QString("%1:%2").arg(tl).arg(br);
    emit dataChanged(index(tl.y(), tl.x()), index(br.y(), br.x()));
}

QVariant KCBindingModel::data(const QModelIndex& index, int role) const
{
    if ((m_region.isEmpty()) || (role != Qt::EditRole && role != Qt::DisplayRole))
        return QVariant();
    const QPoint offset = m_region.firstRange().topLeft();
    const KCSheet* sheet = m_region.firstSheet();
    const KCValue value = sheet->cellStorage()->value(offset.x() + index.column(),
                        offset.y() + index.row());
    // KChart::KCValue is either:
    //  - a double (interpreted as a value)
    //  - a QString (interpreted as a label)
    //  - a QDateTime (interpreted as a date/time value)
    //  - Invalid (interpreted as empty)
    QVariant variant;
    switch (value.type()) {
    case KCValue::Float:
    case KCValue::Integer:
        if (value.format() == KCValue::fmt_DateTime ||
                value.format() == KCValue::fmt_Date ||
                value.format() == KCValue::fmt_Time) {
            variant.setValue<QDateTime>(value.asDateTime(sheet->map()->calculationSettings()));
            break;
        }
    case KCValue::Boolean:
    case KCValue::Complex:
    case KCValue::Array:
        variant.setValue<double>(numToDouble(value.asFloat()));
        break;
    case KCValue::String:
    case KCValue::Error:
        variant.setValue<QString>(value.asString());
        break;
    case KCValue::Empty:
    case KCValue::CellRange:
    default:
        break;
    }
    //kDebug() << index.column() <<"," << index.row() <<"," << variant;
    return variant;
}

const KCRegion& KCBindingModel::region() const
{
    return m_region;
}

QVariant KCBindingModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ((m_region.isEmpty()) || (role != Qt::EditRole && role != Qt::DisplayRole))
        return QVariant();
    const QPoint offset = m_region.firstRange().topLeft();
    const int col = (orientation == Qt::Vertical) ? offset.x() : offset.x() + section;
    const int row = (orientation == Qt::Vertical) ? offset.y() + section : offset.y();
    const KCSheet* sheet = m_region.firstSheet();
    const KCValue value = sheet->cellStorage()->value(col, row);
    return value.asVariant();
}

int KCBindingModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return m_region.isEmpty() ? 0 : m_region.firstRange().height();
}

int KCBindingModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return m_region.isEmpty() ? 0 : m_region.firstRange().width();
}

void KCBindingModel::setRegion(const KCRegion& region)
{
    m_region = region;
}

#include "KCBindingModel.moc"
