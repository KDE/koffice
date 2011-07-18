#ifndef TESTCHANGESDATABASE_H
#define TESTCHANGESDATABASE_H

#include <QtCore/QObject>
#include <QtTest/QtTest>

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
