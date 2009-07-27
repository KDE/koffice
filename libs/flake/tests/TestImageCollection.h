#ifndef TESTIMAGECOLLECTION_H
#define TESTIMAGECOLLECTION_H

#include <QtTest/QtTest>

class TestImageCollection : public QObject
{
    Q_OBJECT
private slots:
    // tests
    void testGetImageImage();
    void testGetExternalImage();
    void testGetImageStore();

    // imageData tests
    void testImageDataAsSharedData();
    void testPreload1();
    void testPreload2();
    void testPreload3();
};

#endif /* TESTIMAGECOLLECTION_H */
