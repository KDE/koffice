#ifndef FILTERBASE_H
#define FILTERBASE_H

#include <qobject.h>

// Attention: The nameOUT Strings are allocated with new[] in the
// slots!!! Therefore you have to delete [] them!
class FilterBase : public QObject {

    Q_OBJECT

public:
    FilterBase() : QObject() {};
    virtual ~FilterBase() {};

    virtual const bool filter() { return false; }
    virtual const QString part();
    virtual const QString extension() { return ".kwd"; }

signals:
    virtual void signalSavePic(const char *data, const char *type, const unsigned int size,
                               char **nameOUT);
    virtual void signalPart(const char *nameIN, const char *type, char **nameOUT);

protected slots:
    virtual void slotSavePic(const char *data, const char *type, const unsigned int size,
                             char **nameOUT);
    virtual void slotPart(const char *nameIN, const char *type, char **nameOUT);

    virtual void slotFilterError();

protected:
    bool success;

private:
    FilterBase(const FilterBase &);
    const FilterBase &operator=(const FilterBase &);
};
#endif // FILTERBASE_H
