/* This file is part of the KDE project
   Copyright 2007 Stefan Nikolaus <stefan.nikolaus@kdemail.net>

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

#include "Binding.h"

#include <QRect>

#include <kdebug.h>

#include "CellStorage.h"
#include "Region.h"
#include "Sheet.h"
#include "Value.h"

namespace KSpread
{

class BindingModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    BindingModel(const Region& region);

    virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    virtual int rowCount( const QModelIndex& parent = QModelIndex() ) const;
    virtual int columnCount( const QModelIndex& parent = QModelIndex() ) const;
    virtual QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;

    const Region& region() const;
    void setRegion(const Region& region);

    void emitDataChanged(const QRect& range);
    void emitChanged(const Region& region);

Q_SIGNALS:
    void changed(const Region& region);

private:
    Region m_region;
};

} // namespace KSpread;


using namespace KSpread;

class Binding::Private : public QSharedData
{
public:
    Private() : model(0) {}
    ~Private() { delete model; }

    BindingModel* model;
};


Binding::Binding()
    : d(new Private)
{
}

Binding::Binding(const Region& region)
    : d(new Private)
{
    Q_ASSERT(region.isValid());
    Q_ASSERT(region.isContiguous());
    d->model = new BindingModel(region);
}

Binding::Binding(const Binding& other)
    : d(other.d)
{
}

Binding::~Binding()
{
}

bool Binding::isEmpty() const
{
    return d->model->region().isEmpty();
}

QAbstractItemModel* Binding::model() const
{
    return d->model;
}

const KSpread::Region& Binding::region() const
{
    return d->model->region();
}

void Binding::setRegion(const Region& region)
{
    d->model->setRegion(region);
}

void Binding::update(const Region& region)
{
    QRect rect;
    Region changedRegion;
    const QRect range = d->model->region().firstRange();
    const Sheet* sheet = d->model->region().firstSheet();
    Region::ConstIterator end(region.constEnd());
    for (Region::ConstIterator it = region.constBegin(); it != end; ++it)
    {
        if (sheet != (*it)->sheet())
            continue;
        rect = range & (*it)->rect();
        if (rect.isValid())
        {
            d->model->emitDataChanged(rect);
            changedRegion.add(rect, (*it)->sheet());
        }
    }
    kDebug() << "Binding: " << changedRegion << " changed." << endl;
    d->model->emitChanged(changedRegion);
}

void Binding::operator=(const Binding& other)
{
    d = other.d;
}

bool Binding::operator==(const Binding& other) const
{
    return d == other.d;
}

bool Binding::operator<(const Binding& other) const
{
    return d < other.d;
}


BindingModel::BindingModel(const Region& region)
    : QAbstractTableModel()
    , m_region(region)
{
}

QVariant BindingModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(role);
    if (m_region.isEmpty())
        return QVariant();
    const QPoint offset = m_region.firstRange().topLeft();
    const int col = (orientation == Qt::Vertical) ? offset.x() : offset.x() + section;
    const int row = (orientation == Qt::Vertical) ? offset.y() + section : offset.y();
    const Sheet* sheet = m_region.firstSheet();
    const Value value = sheet->cellStorage()->value(col, row);
    return QVariant(value.asString());
}

int BindingModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return m_region.isEmpty() ? 0 : m_region.firstRange().height();
}

int BindingModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return m_region.isEmpty() ? 0 : m_region.firstRange().width();
}

QVariant BindingModel::data(const QModelIndex& index, int role) const
{
    Q_UNUSED(role);
    if (m_region.isEmpty())
        return QVariant();
    const QPoint offset = m_region.firstRange().topLeft();
    const Sheet* sheet = m_region.firstSheet();
    const Value value = sheet->cellStorage()->value(offset.x() + index.column(),
                                                     offset.y() + index.row());
    // KoChart::Value is either:
    //  - a double (interpreted as a value)
    //  - a QString (interpreted as a label)
    //  - a QDateTime (interpreted as a date/time value)
    //  - Invalid (interpreted as empty)
    QVariant variant;
    switch (value.type())
    {
        case Value::Float:
        case Value::Integer:
            if (value.format() == Value::fmt_DateTime ||
                 value.format() == Value::fmt_Date ||
                 value.format() == Value::fmt_Time)
            {
                variant.setValue<QDateTime>(value.asDateTime(sheet->doc()));
                break;
            }
        case Value::Boolean:
        case Value::Complex:
        case Value::Array:
            variant.setValue<double>(numToDouble (value.asFloat()));
            break;
        case Value::String:
        case Value::Error:
            variant.setValue<QString>(value.asString());
            break;
        case Value::Empty:
        case Value::CellRange:
        default:
            break;
    }
    kDebug() << k_funcinfo << index.column() << ", " << index.row() << ", " << variant << endl;
    return variant;
}

const KSpread::Region& BindingModel::region() const
{
    return m_region;
}

void BindingModel::setRegion(const Region& region)
{
    m_region = region;
}

void BindingModel::emitDataChanged(const QRect& rect)
{
    const QPoint tl = rect.topLeft();
    const QPoint br = rect.bottomRight();
    emit dataChanged(index(tl.y(), tl.x()), index(br.y(), br.x()));
}

void BindingModel::emitChanged(const Region& region)
{
    emit changed(region);
}

#include "Binding.moc"
