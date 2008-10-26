#ifndef TESTCHANGESDATABASE_H
#define TESTCHANGESDATABASE_H

#include <QObject>
#include <QtTest>

class TestChangesDatabase : public QObject
{
    Q_OBJECT
public:
    TestChangesDatabase() {}

private slots:
    void testInsert();
    void testRemove();
    void testSplit();
    void testMerge();
};

#endif
