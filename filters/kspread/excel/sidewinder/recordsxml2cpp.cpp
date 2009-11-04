#include <QCoreApplication>
#include <QDebug>
#include <QDomDocument>
#include <QFile>
#include <QTextStream>

static inline QString ucFirst(QString s) {
    return s[0].toUpper() + s.mid(1);
}

struct Field {
    QString name;
    QString type;
    bool isArray;
    bool isArrayLength;
    bool isStringLength;
    bool isEnum;
    QList<Field> arrayFields;
    Field(QString name = QString(), QString type = QString()) : name(name), type(type), isArray(false), isArrayLength(false), isStringLength(false), isEnum(false) {}

    QString getterName() const {
        if (type == "bool") {
            return "is" + ucFirst(name);
        } else {
            return name;
        }
    }

    QString setterName() const {
        return "set" + ucFirst(name);
    }
};

static QString getFieldType(QString xmlType, unsigned bits, QString otherType = QString())
{
    if (xmlType == "unsigned") return "unsigned";
    else if (xmlType == "signed") return "int";
    else if (xmlType == "float") return "double";
    else if (xmlType == "bool") return "bool";
    else if (xmlType == "bytestring" || xmlType == "unicodestring") return "UString";
    return "ERROR";
}

static QMap<QString, Field> getFields(QDomElement record, bool* foundStrings = 0)
{
    QDomNodeList fields = record.elementsByTagName("field");
    QMap<QString, Field> map;
    for (int i = 0; i < fields.size(); i++) {
        QDomElement e = fields.at(i).toElement();
        QString name = e.attribute("name");
        if (!name.startsWith("reserved")) {
            map[name] = Field(name, getFieldType(e.attribute("type"), e.attribute("size").toUInt(), map[name].type));
            if (foundStrings && map[name].type == "UString") *foundStrings = true;
            if (e.parentNode().nodeName() == "array") {
                map[name].isArray = true;
            }
            if (e.elementsByTagName("enum").size() > 0) {
                map[name].isEnum = true;
                map[name].type = ucFirst(name);
            }
        }
    }
    for (int i = 0; i < fields.size(); i++) {
        QDomElement e = fields.at(i).toElement();
        if (e.hasAttribute("length")) {
            QString name = e.attribute("length");
            if (map.contains(name))
                map[name].isStringLength = true;
        }
    }
    QDomNodeList arrays = record.elementsByTagName("array");
    for (int i = 0; i < arrays.size(); i++) {
        QDomElement e = arrays.at(i).toElement();
        QString name = e.attribute("length");
        if (map.contains(name)) {
            Field& field = map[name];
            field.isArrayLength = true;
            QDomNodeList afields = e.elementsByTagName("field");
            for (int j = 0; j < afields.size(); j++) {
                QDomElement af = afields.at(j).toElement();
                QString fname = af.attribute("name");
                if (!fname.startsWith("reserved"))
                    field.arrayFields.append(map[fname]);
            }
        }
    }
    return map;
}

void processEnumsForHeader(QDomNodeList fieldList, QTextStream& out)
{
    for (int i = 0; i < fieldList.size(); i++) {
        QDomElement f = fieldList.at(i).toElement();
        QDomNodeList enumNodes = f.elementsByTagName("enum");
        if (enumNodes.size()) {
            QString name = ucFirst(f.attribute("name"));
            out << "    enum " << name << " {\n";
            for (int j = 0; j < enumNodes.size(); j++) {
                QDomElement en = enumNodes.at(j).toElement();
                out << "        " << en.attribute("name");
                if (en.hasAttribute("value"))
                    out << " = " << en.attribute("value");
                if (j != enumNodes.size()-1) out << ",";
                out << "\n";
            }
            out << "    };\n\n"
                << "    static UString " << f.attribute("name") << "ToString(" << name << " " << f.attribute("name") << ");\n\n";
        }
    }
}

void processRecordForHeader(QDomElement e, QTextStream& out)
{
    QString className = e.attribute("name") + "Record";
    QList<Field> fields = getFields(e).values();

    out << "class " << className << " : public Record\n{\npublic:\n";

    // add id field and rtti method
    out << "    static const unsigned id;\n\n"
        << "    virtual unsigned rtti() const { return this->id; }\n\n";

    // constructor and destructor
    out << "    " << className << "();\n    virtual ~" << className << "();\n\n";

    // copy and assignment
    out << "    " << className << "( const " << className << "& record );\n"
        << "    " << className << "& operator=( const " << className << "& record );\n\n";

    // enums
    QDomNodeList fieldNodes = e.elementsByTagName("field");
    processEnumsForHeader(fieldNodes, out);
    QDomNodeList cfieldNodes = e.elementsByTagName("computedField");
    processEnumsForHeader(cfieldNodes, out);

    // getters and setters
    foreach (const Field& f, fields) {
        out << "    " << f.type << " " << f.getterName() << "(";
        if (f.isArray) out << " unsigned index ";
        out << ") const;\n"
            << "    void " << f.setterName() << "(";
        if (f.isArray) out << " unsigned index,";
        out << " " << f.type << " " << f.name << " );\n\n";
    }

    // computed fields
    for (int i = 0; i < cfieldNodes.size(); i++) {
        QDomElement f = cfieldNodes.at(i).toElement();
        QString type = f.attribute("ctype");
        if (type.isEmpty()) type = ucFirst(f.attribute("name"));
        out << "    " << type << " " << f.attribute("name") << "() const;\n\n";
    }

    // setData method
    out << "    virtual void setData( unsigned size, const unsigned char* data, const unsigned* continuePositions );\n\n";

    // name method
    out << "    virtual const char* name() const { return \"" << e.attribute("name") << "\"; }\n\n";

    // dump method
    out << "    virtual void dump( std::ostream& out ) const;\n\n";

    // private stuff
    out << "private:\n    class Private;\n    Private * const d;\n};\n\n";
}

static void sizeCheck(QString indent, QTextStream& out, QDomElement firstField, unsigned offset, bool dynamicOffset)
{
    // find size of all fields until first unsized field or first if/array
    unsigned size = 0;
    for (QDomElement e = firstField; !e.isNull(); e = e.nextSiblingElement()) {
        if (e.tagName() != "field") break;
        unsigned bits = e.attribute("size", "0").toUInt();
        if (bits == 0) break;
        size += bits;
    }
    if (size % 8 != 0)
        qFatal("Invalid non-byte-sized chunk of fields found");
    if (offset % 8 != 0)
        qFatal("Invalid non-byte-aligned chunk of fields found");
    if (size != 0) {
        out << indent << "if (size < ";
        if (dynamicOffset) out << "curOffset + ";
        if (offset) out << (offset/8) << " + ";
        out << (size / 8) << ") {\n"
            << indent << "    setIsValid(false);\n"
            << indent << "    return;\n"
            << indent << "}\n";
    }
}

static void processFieldElement(QString indent, QTextStream& out, QDomElement field, unsigned& offset, bool& dynamicOffset, QMap<QString, Field>& fieldsMap, QString setterArgs = QString())
{
    if (field.tagName() == "field") {
        unsigned bits = field.attribute("size").toUInt();
        QString name = field.attribute("name");
        if (!name.startsWith("reserved")) {
            if (bits >= 8 && offset%8 != 0)
                qFatal("Unaligned byte-or-larger field");
            if (bits >= 16 && bits%8 != 0)
                qFatal("Fields of 16 bits and larger must always be an exact number of bytes");

            Field& f = fieldsMap[name];
            if (f.type == "UString") {
                if (offset % 8 != 0)
                    qFatal("Unaligned string");

                if (!dynamicOffset)
                    out << indent << "curOffset = " << (offset/8) << ";\n";
                else
                    if (offset) out << indent << "curOffset += " << (offset/8) << ";\n";
                out << indent << f.setterName() << "(" << setterArgs;
                dynamicOffset = true; offset = 0;

                if (field.attribute("type") == "unicodestring")
                    out << "readUnicodeString(";
                else
                    out << "readByteString(";

                out << "data + curOffset, " << field.attribute("length") << ", size - curOffset"
                    << ", &stringLengthError, &stringSize));\n";
                out << indent << "if (stringLengthError) {\n"
                    << indent << "    setIsValid(false);\n"
                    << indent << "    return;\n"
                    << indent << "}\n";
                out << indent << "curOffset += stringSize;\n";
                sizeCheck(indent, out, field.nextSiblingElement(), offset, dynamicOffset);
            } else if (bits % 8 == 0) {
                if (f.isStringLength)
                    out << indent << "unsigned " << name << " = ";
                else
                    out << indent << f.setterName() << "(" << setterArgs;

                if (f.isEnum)
                    out << "static_cast<" << f.type << ">(";

                if (field.attribute("type") == "unsigned" || field.attribute("type") == "bool")
                    out << "readU" << bits;
                else if (field.attribute("type") == "signed")
                    out << "readS" << bits;
                else if (field.attribute("type") == "float")
                    out << "readFloat" << bits;
                else
                    qFatal("Unsupported type %s", qPrintable(field.attribute("type")));

                out << "(data";
                if (dynamicOffset) out << " + curOffset";
                if (offset) out << " + " << (offset/8);
                out << ")";
                if (field.attribute("type") == "bool") out << " != 0";
                if (f.isEnum) out << ")";
                if (!f.isStringLength) out << ")";
                out << ";\n";
            } else {
                out << indent << f.setterName() << "(" << setterArgs;
                unsigned firstByte = offset/8;
                unsigned lastByte = (offset+bits-1)/8;
                unsigned bitOffset = offset%8;
                unsigned mask = (1 << bits)-1;
                if (firstByte == lastByte) {
                    out << "(readU8(data";
                } else {
                    out << "(readU16(data";
                }
                if (dynamicOffset) out << " + curOffset";
                if (firstByte) out << " + " << firstByte;
                out << ")";
                if (bitOffset) out << " >> " << bitOffset;
                out << ") & 0x" << hex << mask << dec;
                if (field.attribute("type") == "bool") out << " != 0";
                out << ");\n";
            }
        }
        offset += bits;
    } else if (field.tagName() == "if") {
        if (offset % 8 != 0)
            qFatal("Ifs should always be byte-aligned");
        if (!dynamicOffset)
            out << indent << "curOffset = " << (offset/8) << ";\n";
        else
            if (offset) out << indent << "curOffset += " << (offset/8) << ";\n";

        out << indent << "if (" << field.attribute("predicate") << ") {\n";

        offset = 0; dynamicOffset = true;
        sizeCheck(indent + "    ", out, field.firstChildElement(), offset, dynamicOffset);

        for (QDomElement child = field.firstChildElement(); !child.isNull(); child = child.nextSiblingElement()) {
            processFieldElement(indent + "    ", out, child, offset, dynamicOffset, fieldsMap);
        }

        if (offset % 8 != 0)
            qFatal("Ifs should contain an integer number of bytes");

        if (offset) out << indent << "    curOffset += " << (offset/8) << ";\n";
        out << indent << "}\n";
        offset = 0;
        sizeCheck(indent, out, field.nextSiblingElement(), offset, dynamicOffset);
    } else if (field.tagName() == "array") {
        if (offset % 8 != 0)
            qFatal("Arrays should always be byte-aligned");

        if (!dynamicOffset)
            out << indent << "curOffset = " << (offset/8) << ";\n";
        else
            if (offset) out << indent << "curOffset += " << (offset/8) << ";\n";

        QString length = field.attribute("length");
        if (fieldsMap.contains(length))
            length = fieldsMap[length].getterName() + "()";
        else {
            QDomNodeList fields = field.elementsByTagName("field");
            for (int i = 0; i < fields.size(); i++) {
                QDomElement e = fields.at(i).toElement();
                QString name = e.attribute("name");
                if (!name.startsWith("reserved")) {
                    out << indent << "d->" << name << ".resize(" << length << ");\n";
                }
            }
        }

        out << indent << "for (unsigned i = 0, endi = " << length << "; i < endi; ++i) {\n";
        offset = 0; dynamicOffset = true;
        sizeCheck(indent + "    ", out, field.firstChildElement(), offset, dynamicOffset);

        for (QDomElement child = field.firstChildElement(); !child.isNull(); child = child.nextSiblingElement()) {
            processFieldElement(indent + "    ", out, child, offset, dynamicOffset, fieldsMap, setterArgs + "i, ");
        }

        if (offset % 8 != 0)
            qFatal("Arrays should contain an integer number of bytes");

        if (offset) out << indent << "    curOffset += " << (offset/8) << ";\n";
        out << indent << "}\n";
        offset = 0;
        sizeCheck(indent, out, field.nextSiblingElement(), offset, dynamicOffset);
    }
}

static void processFieldElementForDump(QString indent, QTextStream& out, QDomElement field, const QMap<QString, Field>& fieldsMap, QString getterArgs = QString())
{
    if (field.tagName() == "field") {
        QString name = field.attribute("name");
        if (!name.startsWith("reserved")) {
            const Field& f = fieldsMap[name];
            if (!f.isStringLength) {
                out << indent << "out << \"";
                if (getterArgs.length() == 0) {
                    out << QString(19-name.length(), ' ');
                    out << ucFirst(name) << " : \" << ";
                } else {
                    out << QString(15-name.length(), ' ');
                    out << ucFirst(name);
                    out << " \" << std::setw(3) << " << getterArgs << " <<\" : \" << ";
                }
                if (f.isEnum)
                    out << name << "ToString(" << f.getterName() << "(" << getterArgs << "))";
                else
                    out << f.getterName() << "(" << getterArgs << ")";
                out << " << std::endl;\n";
            }
        }
    } else if (field.tagName() == "if") {
        out << indent << "if (" << field.attribute("predicate") << ") {\n";
        for (QDomElement e = field.firstChildElement(); !e.isNull(); e = e.nextSiblingElement())
            processFieldElementForDump(indent + "    ", out, e, fieldsMap);
        out << indent << "}\n";
    } else if (field.tagName() == "array") {
        QString length = field.attribute("length");
        if (fieldsMap.contains(length))
            length = fieldsMap[length].getterName() + "()";
        else
            length = "d->" + field.firstChildElement().attribute("name") + ".size()";

        out << indent << "for (unsigned i = 0, endi = " << length << "; i < endi; ++i) {\n";
        for (QDomElement e = field.firstChildElement(); !e.isNull(); e = e.nextSiblingElement())
            processFieldElementForDump(indent + "    ", out, e, fieldsMap, "i");
        out << indent << "}\n";
    }
}

void processEnumsForImplementation(QDomNodeList fieldList, QString className, QTextStream& out)
{
    for (int i = 0; i < fieldList.size(); i++) {
        QDomElement f = fieldList.at(i).toElement();
        QDomNodeList enumNodes = f.elementsByTagName("enum");
        if (enumNodes.size()) {
            QString name = ucFirst(f.attribute("name"));
            out << "UString " << className << "::" << f.attribute("name") << "ToString(" << name << " " << f.attribute("name") << ")\n{\n"
                << "    switch (" << f.attribute("name") << ") {\n";
            for (int j = 0; j < enumNodes.size(); j++) {
                QDomElement en = enumNodes.at(j).toElement();
                out << "        case " << en.attribute("name") << ": return UString(\"" << en.attribute("name") << "\");\n";
            }
            out << "        default: return UString(\"Unknown: \") + UString::from(" << f.attribute("name") << ");\n    }\n}\n\n";
        }
    }
}

void processRecordForImplementation(QDomElement e, QTextStream& out)
{
    QString className = e.attribute("name") + "Record";
    bool containsStrings = false;
    QMap<QString, Field> fieldsMap = getFields(e, &containsStrings);
    QList<Field> fields = fieldsMap.values();

    out << "// ========== " << className << " ==========\n\n";

    // id field
    out << "const unsigned " << className << "::id = " << e.attribute("id") << ";\n\n";

    // private class
    out << "class " << className << "::Private\n{\n"
        << "public:\n";
    foreach (const Field& f, fields) {
        if (f.isArray)
            out << "    std::vector<" << f.type << "> " << f.name << ";\n";
        else if (!f.isStringLength)
            out << "    " << f.type << " " << f.name << ";\n";
    }
    out << "};\n\n";

    // constructor
    out << className << "::" << className << "()\n";
    out << "    : d(new Private)\n{\n";
    foreach (const Field& f, fields) {
        if (f.isArray || f.isStringLength) continue;
        QString val;
        if (f.type == "unsigned" || f.type == "int") {
            val = "0";
        } else if (f.type == "bool") {
            val = "false";
        }
        if (!val.isEmpty())
            out << "    " << f.setterName() << "(" << val << ");\n";
    }
    out << "}\n\n";

    // destructor
    out << className << "::~" << className << "()\n{\n    delete d;\n}\n\n";

    // copy constructor
    out << className << "::" << className << "( const " << className << "& record )\n"
        << "    : Record(record), d(new Private)\n{\n"
        << "    *this = record;\n}\n\n";

    // assignment operator
    out << className << "& " << className << "::operator=( const " << className << "& record )\n{\n"
        << "    *d = *record.d;\n"
        << "    return *this;\n}\n\n";

    // enums
    QDomNodeList fieldNodes = e.elementsByTagName("field");
    processEnumsForImplementation(fieldNodes, className, out);
    QDomNodeList cfieldNodes = e.elementsByTagName("computedField");
    processEnumsForImplementation(cfieldNodes, className, out);

    // getters and setters
    foreach (const Field& f, fields) {
        if (f.isStringLength) continue;

        if (f.isEnum) out << className << "::";
        out << f.type << " " << className << "::" << f.getterName() << "(";
        if (f.isArray) out << " unsigned index ";
        out << ") const\n{\n    return d->" << f.name;
        if (f.isArray) out << "[index]";
        out << ";\n}\n\n";

        out << "void " << className << "::" << f.setterName() << "(";
        if (f.isArray) out << " unsigned index, ";
        out << f.type << " " << f.name << " )\n{\n    d->" << f.name;
        if (f.isArray) out << "[index]";
        out << " = " << f.name << ";\n";

        if (f.isArrayLength) {
            foreach (const Field& af, f.arrayFields) {
                out << "    d->" << af.name << ".resize(" << f.name << ");\n";
            }
        }

        out << "}\n\n";
    }

    // computed fields
    for (int i = 0; i < cfieldNodes.size(); i++) {
        QDomElement f = cfieldNodes.at(i).toElement();
        QString type = f.attribute("ctype");
        if (type.isEmpty()) type = className + "::" + ucFirst(f.attribute("name"));
        out << type << " " << className << "::" << f.attribute("name") << "() const\n{\n"
            << "    return " << f.attribute("value") << ";\n}\n\n";
    }

    // setData method
    out << "void " << className << "::setData( unsigned size, const unsigned char* data, const unsigned int* )\n{\n";
    if (e.elementsByTagName("if").size() > 0 || e.elementsByTagName("array").size() > 0 || containsStrings)
        out << "    unsigned curOffset;\n";
    if (containsStrings) {
        out << "    bool stringLengthError = false;\n"
            << "    unsigned stringSize;\n";
    }
    unsigned offset = 0;
    bool dynamicOffset = false;
    sizeCheck("    ", out, e.firstChildElement(), offset, dynamicOffset);
    for (QDomElement child = e.firstChildElement(); !child.isNull(); child = child.nextSiblingElement())
        processFieldElement("    ", out, child, offset, dynamicOffset, fieldsMap);
    out << "}\n\n";

    // dump method
    out << "void " << className << "::dump( std::ostream& out ) const\n{\n"
        << "    out << \"" << e.attribute("name") << "\" << std::endl;\n";
    for (QDomElement child = e.firstChildElement(); !child.isNull(); child = child.nextSiblingElement())
        processFieldElementForDump("    ", out, child, fieldsMap);
    out << "}\n\n";

    // creator function
    out << "static Record* create" << className << "()\n{\n    return new " << className << "();\n}\n\n";
}


int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    QDomDocument doc("records");
    QFile f;
    if (argc <= 1) {
        f.setFileName("records.xml");
    } else {
        f.setFileName(argv[1]);
    }
    if (!f.open(QIODevice::ReadOnly))
        qFatal("Error opening file");
    if (!doc.setContent(&f)) {
        f.close();
        qFatal("Error parsing file");
    }
    f.close();

    QFile hFile("records.h");
    hFile.open(QIODevice::WriteOnly);
    QTextStream hOut(&hFile);

    QFile cppFile("records.cpp");
    cppFile.open(QIODevice::WriteOnly);
    QTextStream cppOut(&cppFile);

    hOut << "// This file was automatically generated from records.xml\n"
         << "#ifndef SWINDER_RECORDS_H\n"
         << "#define SWINDER_RECORDS_H\n\n"
         << "#include \"utils.h\"\n\n"
         << "namespace Swinder {\n\n"
         << "void registerRecordClasses();\n\n";

    cppOut << "// This file was automatically generated from records.xml\n"
           << "#include \"records.h\"\n"
           << "#include <vector>\n"
           << "#include <iomanip>\n\n"
           << "namespace Swinder {\n\n";

    QDomNodeList records = doc.elementsByTagName("record");
    for (int i = 0; i < records.size(); i++) {
        QDomElement e = records.at(i).toElement();
        processRecordForHeader(e, hOut);
        processRecordForImplementation(e, cppOut);
    }

    cppOut << "void registerRecordClasses()" << endl << "{" << endl;
    for (int i = 0; i < records.size(); i++) {
        QDomElement e = records.at(i).toElement();
        cppOut << "    RecordRegistry::registerRecordClass(" << e.attribute("name") << "Record::id, create" << e.attribute("name") << "Record);\n";
    }
    cppOut << "}\n\n";

    hOut << "} // namespace Swinder\n\n";
    hOut << "#endif // SWINDER_RECORDS_H\n";

    cppOut << "} // namespace Swinder\n";
}
