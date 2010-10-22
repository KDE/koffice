/* This file is part of the KDE project
   Copyright (C) 2010 Marijn Kruisselbrink <m.kruisselbrink@student.tue.nl>

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
#include <CFBWriter.h>

#include <QBuffer>
#include <QDataStream>
#include <QFile>
#include <QIODevice>
#include <QStringList>

#include <QDebug>

static const quint32 ENDOFCHAIN = 0xFFFFFFFE;
static const quint32 FATSECT = 0xFFFFFFFD;
static const quint32 FREESECT = 0xFFFFFFFF;
static const quint32 NOSTREAM = 0xFFFFFFFF;

CFBWriter::CFBWriter(bool largeSectors)
    : m_device(0), m_ownsDevice(false), m_sectorSize(largeSectors ? 4096 : 512)
{
}

CFBWriter::~CFBWriter()
{
    if (m_device)
        close();
}

bool CFBWriter::open(QIODevice *device)
{
    if (!device->isOpen()) {
        if (!device->open(QIODevice::WriteOnly)) {
            return false;
        }
        if (device->isSequential()) {
            return false;
        }
        m_device = device;
        m_ownsDevice = false;
    }
    init();
    return true;
}

bool CFBWriter::open(const QString &fileName)
{
    m_device = new QFile(fileName);
    if (!m_device->open(QIODevice::WriteOnly)) {
        delete m_device; m_device = 0;
        return false;
    }
    if (m_device->isSequential()) {
        delete m_device; m_device = 0;
        return false;
    }
    m_ownsDevice = true;
    init();
    return true;
}

class CFBWriter::StreamIODevice : public QIODevice
{
public:
    StreamIODevice(CFBWriter& writer, DirectoryEntry& entry);
    virtual ~StreamIODevice();
    virtual void close();
    virtual bool open(OpenMode mode);
    virtual qint64 size() const;
protected:
    virtual qint64 writeData(const char *data, qint64 len);
    virtual qint64 readData(char *data, qint64 maxlen);
    void appendData(const char *data, qint64 len);
    qint64 internalWriteData(const char* data, qint64 len, qint64 pos);
private:
    DirectoryEntry& m_entry;
    CFBWriter& m_writer;
    QByteArray m_buffer;
    unsigned m_lastSector;
};

CFBWriter::StreamIODevice::StreamIODevice(CFBWriter& writer, DirectoryEntry& entry)
    : m_entry(entry), m_writer(writer), m_lastSector(-1)
{
    m_writer.m_openStreams.append(this);
    QIODevice::open(QIODevice::WriteOnly);
}

CFBWriter::StreamIODevice::~StreamIODevice()
{
    close();
}

void CFBWriter::StreamIODevice::close()
{
    QIODevice::close();
    if (m_buffer.size() > 0) {
        if (m_entry.streamSize == 0 && m_entry.id != 0) {
            // smaller than cutoff size, so use minifat
            m_entry.streamSize = m_buffer.size();
            int sector = -1;
            for (int i = 0; i < m_buffer.size(); i += 64) {
                QByteArray b = m_buffer.mid(i, 64);
                if (b.size() < 64) b.append(QByteArray(64 - b.size(), '\0'));
                sector = m_writer.writeMiniSector(b, sector);
                if (i == 0) m_entry.firstSector = sector;
            }
        } else {
            // write full sectors first
            while (unsigned(m_buffer.size()) > m_writer.m_sectorSize) {
                QByteArray sector = m_buffer.left(m_writer.m_sectorSize);
                m_buffer = m_buffer.mid(m_writer.m_sectorSize);
                m_lastSector = m_writer.writeSector(sector, m_lastSector);
                if (m_entry.firstSector == quint32(-1)) {
                    m_entry.firstSector = m_lastSector;
                }
                m_entry.streamSize += m_writer.m_sectorSize;
            }
            m_entry.streamSize += m_buffer.size();
            m_buffer.append(QByteArray(m_writer.m_sectorSize - m_buffer.size(), '\0'));
            m_lastSector = m_writer.writeSector(m_buffer, m_lastSector);
            if (m_entry.firstSector == quint32(-1)) {
                m_entry.firstSector = m_lastSector;
            }
        }
    }
    m_writer.m_openStreams.removeAll(this);
}

bool CFBWriter::StreamIODevice::open(OpenMode)
{
    return false;
}

qint64 CFBWriter::StreamIODevice::size() const
{
    return m_entry.streamSize + m_buffer.size();
}

void CFBWriter::StreamIODevice::appendData(const char *data, qint64 len)
{
    m_buffer.append(data, len);

    // cutoff for mini-fat
    if (m_entry.streamSize == 0 && m_buffer.size() <= 4096) return;

    while (unsigned(m_buffer.size()) > m_writer.m_sectorSize) {
        QByteArray sector = m_buffer.left(m_writer.m_sectorSize);
        m_buffer = m_buffer.mid(m_writer.m_sectorSize);
        m_lastSector = m_writer.writeSector(sector, m_lastSector);
        if (m_entry.firstSector == quint32(-1)) {
            m_entry.firstSector = m_lastSector;
        }
        m_entry.streamSize += m_writer.m_sectorSize;
    }
}

qint64 CFBWriter::StreamIODevice::writeData(const char *data, qint64 len)
{
    qint64 bytesWritten = 0;
    qint64 curPos = pos();
    while (bytesWritten < len) {
        bytesWritten += internalWriteData(data + bytesWritten, len - bytesWritten, curPos + bytesWritten);
    }
    return len;
}

qint64 CFBWriter::StreamIODevice::internalWriteData(const char *data, qint64 len, qint64 pos)
{
    Q_ASSERT(pos >= 0 && pos <= size());
    if (pos == size()) {
        appendData(data, len);
        return len;
    } else {
        if (pos + len > size()) {
            len = size() - pos;
        }
        qint64 bufferStart = m_entry.streamSize;
        if (pos < bufferStart) {
            unsigned sectorIndex = pos / m_writer.m_sectorSize;
            unsigned sector = m_entry.firstSector;
            while (sectorIndex--) {
                sector = m_writer.m_fat[sector];
            }
            unsigned sectorOffset = pos % m_writer.m_sectorSize;
            if (sectorOffset + len > m_writer.m_sectorSize) {
                len = m_writer.m_sectorSize - sectorOffset;
            }
            m_writer.writeData(sector, sectorOffset, QByteArray::fromRawData(data, len));
        } else {
            // len has already been adjusted to be fully inside the buffer, so just overwrite what is in the buffer with the subset of data we have
            m_buffer.replace(pos - bufferStart, len, QByteArray::fromRawData(data, len));
        }
        return len;
    }
}

qint64 CFBWriter::StreamIODevice::readData(char *data, qint64 maxlen)
{
    Q_UNUSED(data);
    Q_UNUSED(maxlen);
    return -1;
}

QIODevice* CFBWriter::openSubStream(const QString &streamName)
{
    DirectoryEntry* parent = &m_entries[0];
    QStringList parts = streamName.split('/');
    for (int i = 0; i < parts.size()-1; i++) {
        if (parent->children.contains(parts[i])) {
            parent = parent->children[parts[i]];
            Q_ASSERT(parent->type == DirectoryEntry::Storage);
        } else {
            m_entries.append(DirectoryEntry(m_entries.size(), parts[i], DirectoryEntry::Storage));
            parent->children[parts[i]] = &m_entries.last();
            parent = &m_entries.last();
        }
    }
    Q_ASSERT(!parent->children.contains(parts.last()));
    m_entries.append(DirectoryEntry(m_entries.size(), parts.last(), DirectoryEntry::Stream));
    parent->children[parts.last()] = &m_entries.last();
    return new StreamIODevice(*this, m_entries.last());
}

void CFBWriter::setRootClassId(const QUuid& classId)
{
    m_entries[0].uuid = classId;
}

static bool compareNames(const QString& a, const QString& b)
{
    if (a.length() < b.length()) return true;
    if (a.length() > b.length()) return false;
    return a.toUpper() < b.toUpper();
}

void CFBWriter::DirectoryEntry::buildChildrenTree()
{
    if (children.size() == 0) {
        firstChild = 0;
        return;
    }
    QList<QString> names = children.keys();
    qSort(names.begin(), names.end(), compareNames);
    QList<DirectoryEntry*> entries;
    foreach (const QString& name, names) {
        entries.append(children[name]);
    }
    int h = entries.size() / 2;
    firstChild = entries[h];
    firstChild->buildSiblingTree(entries, 0, h, entries.size() - 1);
}

void CFBWriter::DirectoryEntry::buildSiblingTree(const QList<DirectoryEntry *> &nodes, int first, int middle, int last)
{
    if (middle > first) {
        int h = middle / 2;
        leftSibling = nodes[h];
        leftSibling->buildSiblingTree(nodes, first, h, middle-1);
    } else
        leftSibling = 0;
    if (last > middle) {
        int h = ((last - middle + 1) / 2) + middle;
        rightSibling = nodes[h];
        rightSibling->buildSiblingTree(nodes, middle+1, h, last);
    } else
        rightSibling = 0;
}

void CFBWriter::close()
{
    static const char zeroes[16] = {0};

    Q_ASSERT(m_device);
    Q_ASSERT(m_device->isOpen());
    Q_ASSERT(m_device->isWritable());
    Q_ASSERT(!m_device->isSequential());

    // close streams
    foreach (StreamIODevice* stream, m_openStreams)
        stream->close();

    // write directory entries
    // - update left/right sibling/first child fields
    for (int i = 0; i < m_entries.size(); i++) {
        m_entries[i].buildChildrenTree();
    }
    // - write actual entries
    unsigned dirSector = -1;
    m_directorySectorCount = 0;
    m_firstDirectorySector = -1;
    for (int i = 0; i < m_entries.size(); i += m_sectorSize/128) {
        QByteArray sector(m_sectorSize, '\0');
        QBuffer b(&sector);
        b.open(QIODevice::WriteOnly);
        QDataStream ds(&b);
        ds.setByteOrder(QDataStream::LittleEndian);
        for (int j = 0; j < int(m_sectorSize/128) && i+j < m_entries.size(); j++) {
            const DirectoryEntry& e = m_entries[i+j];
            const ushort* name = e.name.utf16();
            int n = 0;
            while (*name) {
                ds << quint16(*name);
                n++; name++;
            }
            for (int i = 0; i < 32-n; i++) {
                ds << quint16(0);
            }
            ds << quint16(n*2+2);
            ds << quint8(e.type);
            ds << quint8(0); // color flag: black
            ds << quint32(e.leftSibling ? e.leftSibling->id : NOSTREAM);
            ds << quint32(e.rightSibling ? e.rightSibling->id : NOSTREAM);
            ds << quint32(e.firstChild ? e.firstChild->id : NOSTREAM);
            ds << e.uuid; // CLSID
            ds << quint32(0); // state bits
            ds.writeRawData(zeroes, 16); // creation&modified time
            ds << quint32(e.firstSector);
            ds << quint64(e.streamSize);
        }
        dirSector = writeSector(sector, dirSector);
        m_directorySectorCount++;
        if (m_firstDirectorySector == unsigned(-1))
            m_firstDirectorySector = dirSector;
    }

    // write mini-fat
    unsigned miniFatSector = -1;
    for (int i = 0; i < m_miniFat.size(); i += m_sectorSize/4) {
        QByteArray sector(m_sectorSize, '\0');
        QBuffer b(&sector);
        b.open(QIODevice::WriteOnly);
        QDataStream ds(&b);
        ds.setByteOrder(QDataStream::LittleEndian);
        for (int j = 0; j < int(m_sectorSize/4) && i+j < m_miniFat.size(); j++) {
            ds << quint32(m_miniFat[i+j]);
        }
        miniFatSector = writeSector(sector, miniFatSector);
        if (m_miniFatSectorCount == 0) {
            m_firstMiniFatSector = miniFatSector;
        }
        m_miniFatSectorCount++;
    }

    // write fat
    unsigned sectorCount = (m_fat.size() + (m_sectorSize/4)-1) / (m_sectorSize/4);
    unsigned fatSize = m_fat.size() + sectorCount;
    while ((fatSize + (m_sectorSize/4)-1) / (m_sectorSize/4) > sectorCount) {
        sectorCount = (fatSize + (m_sectorSize/4)-1) / (m_sectorSize/4);
        fatSize = m_fat.size() + sectorCount;
    }
    for (unsigned i = 0; i < sectorCount; i++) {
        m_difat.append(m_fat.size());
        m_fat.append(FATSECT);
    }
    for (unsigned i = 0; i < sectorCount; i++) {
        unsigned sector = m_difat[i];
        m_device->seek((sector + 1) * m_sectorSize);
        QDataStream ds(m_device);
        ds.setByteOrder(QDataStream::LittleEndian);
        for (unsigned j = 0, idx = i*(m_sectorSize/4); j < m_sectorSize/4; j++, idx++) {
            ds << quint32(idx < unsigned(m_fat.size()) ? m_fat[idx] : FREESECT);
        }
    }

    // write difat
    // TODO: support difat of more than 109 entries
    Q_ASSERT(m_difat.size() <= 109);

    // update header
    writeHeader();

    if (m_ownsDevice)
        delete m_device;
    m_device = 0;
}

void CFBWriter::init()
{
    m_difat.clear();
    m_firstDifatSector = ENDOFCHAIN;
    m_difatSectorCount = 0;
    m_fat.clear();
    m_miniFat.clear();
    m_firstMiniFatSector = ENDOFCHAIN;
    m_miniFatSectorCount = 0;
    m_entries.clear();
    m_entries.append(DirectoryEntry(0, "Root Entry", DirectoryEntry::RootStorage));
    m_miniFatDataStream = new StreamIODevice(*this, m_entries[0]);
}

void CFBWriter::writeData(unsigned sector, unsigned sectorOffset, const QByteArray &data)
{
    Q_ASSERT(m_device);
    Q_ASSERT(m_device->isOpen());
    Q_ASSERT(m_device->isWritable());
    Q_ASSERT(!m_device->isSequential());
    Q_ASSERT(sector < unsigned(m_fat.size()));
    Q_ASSERT(sectorOffset + data.length() <= m_sectorSize);

    m_device->seek((sector + 1) * m_sectorSize + sectorOffset);
    m_device->write(data);
}

unsigned CFBWriter::writeSector(const QByteArray &data, unsigned previousSector)
{
    Q_ASSERT(m_device);
    Q_ASSERT(m_device->isOpen());
    Q_ASSERT(m_device->isWritable());
    Q_ASSERT(!m_device->isSequential());
    Q_ASSERT(data.size() == m_sectorSize);
    Q_ASSERT(previousSector == unsigned(-1) || previousSector < unsigned(m_fat.size()));

    qDebug() << "writeSector: previousSector=" << previousSector << ", fat-size =" << m_fat.size();
    unsigned sector = m_fat.size();
    m_fat.append(ENDOFCHAIN);
    if (previousSector != unsigned(-1))
        m_fat[previousSector] = sector;

    m_device->seek((sector + 1) * m_sectorSize);
    m_device->write(data);

    return sector;
}

unsigned CFBWriter::writeMiniSector(const QByteArray &data, unsigned previousSector)
{
    Q_ASSERT(m_miniFatDataStream);
    Q_ASSERT(m_miniFatDataStream->isOpen());
    Q_ASSERT(m_miniFatDataStream->isWritable());
    Q_ASSERT(data.size() == 64);
    Q_ASSERT(previousSector == unsigned(-1) || previousSector < unsigned(m_miniFat.size()));

    qDebug() << "writeMiniSector: previousSector=" << previousSector << ", fat-size =" << m_miniFat.size();
    unsigned sector = m_miniFat.size();
    m_miniFat.append(ENDOFCHAIN);
    if (previousSector != unsigned(-1))
        m_miniFat[previousSector] = sector;

    m_miniFatDataStream->write(data);

    return sector;
}

unsigned CFBWriter::fatSectorCount() const
{
    return (m_fat.size() + (m_sectorSize/4)-1) / (m_sectorSize/4);
}

void CFBWriter::writeHeader()
{
    static const char signature[] = {0xD0, 0xCF, 0x11, 0xE0, 0xA1, 0xB1, 0x1A, 0xE1};
    static const char zeroes[16] = {0};
    Q_ASSERT(m_device);
    Q_ASSERT(m_device->isOpen());
    Q_ASSERT(m_device->isWritable());
    Q_ASSERT(!m_device->isSequential());

    m_device->seek(0);
    QDataStream ds(m_device);
    ds.setByteOrder(QDataStream::LittleEndian);

    ds.writeRawData(signature, 8); // Header Signature
    ds.writeRawData(zeroes, 16);   // Header CLSID
    ds << quint16(0x003E);         // Minor version
    ds << quint16(m_sectorSize == 4096 ? 0x0004 : 0x0003);         // Major version
    ds << quint16(0xFFFE);         // Byte order
    ds << quint16(m_sectorSize == 4096 ? 0x000C : 0x0009);         // Sector shift
    ds << quint16(0x0006);         // Mini Sector shift
    ds.writeRawData(zeroes, 6);    // Reserved
    ds << quint32(m_sectorSize == 4096 ? m_directorySectorCount : 0);
    ds << quint32(fatSectorCount());
    ds << quint32(m_firstDirectorySector);
    ds << quint32(0);              // Transaction Signature Number
    ds << quint32(0x1000);         // Mini Stream Cutoff size
    ds << quint32(m_firstMiniFatSector);
    ds << quint32(m_miniFatSectorCount);
    ds << quint32(m_firstDifatSector);
    ds << quint32(m_difatSectorCount);
    for (int i = 0; i < 109; i++) {
        ds << quint32(i < m_difat.size() ? m_difat[i] : NOSTREAM);
    }
}
