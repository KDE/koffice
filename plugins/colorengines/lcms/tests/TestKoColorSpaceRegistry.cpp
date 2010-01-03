
#include "TestKoColorSpaceRegistry.h"

#include <qtest_kde.h>

#include "KoColorSpaceRegistry.h"
#include "KoColorSpace.h"
#include "KoRgbU8ColorSpace.h"
#include "KoRgbU16ColorSpace.h"
#include "KoLabColorSpace.h"

void TestKoColorSpaceRegistry::testConstruction()
{
    KoColorSpaceRegistry* instance = KoColorSpaceRegistry::instance();
    Q_ASSERT(instance);
}

void TestKoColorSpaceRegistry::testRgbU8()
{
    QString colorSpaceId = KoColorSpaceRegistry::instance()->colorSpaceId(RGBAColorModelID,
                           Integer8BitsColorDepthID);
    KoColorSpaceFactory *colorSpaceFactory = KoColorSpaceRegistry::instance()->value(colorSpaceId);
    QVERIFY(colorSpaceFactory != 0);

    const KoColorSpace *colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    QVERIFY(colorSpace != 0);

    const KoColorProfile *profile = colorSpace->profile();
    QVERIFY(profile != 0);

    QCOMPARE(profile->name(), colorSpaceFactory->defaultProfile());

    cmsHPROFILE lcmsProfile = cmsCreate_sRGBProfile();
    QString testProfileName = "TestRGBU8ProfileName";

    cmsAddTag(lcmsProfile, icSigProfileDescriptionTag, testProfileName.toLatin1().data());
    cmsAddTag(lcmsProfile, icSigDeviceModelDescTag, testProfileName.toLatin1().data());
    QByteArray manufacturerName("");
    cmsAddTag(lcmsProfile, icSigDeviceMfgDescTag, manufacturerName.data());

}

void TestKoColorSpaceRegistry::testRgbU16()
{
    QString colorSpaceId = KoColorSpaceRegistry::instance()->colorSpaceId(RGBAColorModelID,
                           Integer16BitsColorDepthID);
    KoColorSpaceFactory *colorSpaceFactory = KoColorSpaceRegistry::instance()->value(colorSpaceId);
    QVERIFY(colorSpaceFactory != 0);

    const KoColorSpace *colorSpace = KoColorSpaceRegistry::instance()->rgb16();
    QVERIFY(colorSpace != 0);

    const KoColorProfile *profile = colorSpace->profile();
    QVERIFY(profile != 0);

    QCOMPARE(profile->name(), colorSpaceFactory->defaultProfile());

    cmsHPROFILE lcmsProfile = cmsCreate_sRGBProfile();
    QString testProfileName = "TestRGBU16ProfileName";

    cmsAddTag(lcmsProfile, icSigProfileDescriptionTag, testProfileName.toLatin1().data());
    cmsAddTag(lcmsProfile, icSigDeviceModelDescTag, testProfileName.toLatin1().data());
    QByteArray manufacturerName("");
    cmsAddTag(lcmsProfile, icSigDeviceMfgDescTag, manufacturerName.data());

}

void TestKoColorSpaceRegistry::testLab()
{
    QString colorSpaceId = KoColorSpaceRegistry::instance()->colorSpaceId(LABAColorModelID,
                           Integer16BitsColorDepthID);
    KoColorSpaceFactory *colorSpaceFactory = KoColorSpaceRegistry::instance()->value(colorSpaceId);
    QVERIFY(colorSpaceFactory != 0);

    const KoColorSpace *colorSpace = KoColorSpaceRegistry::instance()->lab16();
    QVERIFY(colorSpace != 0);

    const KoColorProfile *profile = colorSpace->profile();
    QVERIFY(profile != 0);

    QCOMPARE(profile->name(), colorSpaceFactory->defaultProfile());

    cmsCIExyY whitepoint;
    whitepoint.x = 0.33;
    whitepoint.y = 0.33;
    whitepoint.Y = 1.0;

    cmsHPROFILE lcmsProfile = cmsCreateLabProfile(&whitepoint);
    QString testProfileName = "TestLabProfileName";

    cmsAddTag(lcmsProfile, icSigProfileDescriptionTag, testProfileName.toLatin1().data());
    cmsAddTag(lcmsProfile, icSigDeviceModelDescTag, testProfileName.toLatin1().data());
    QByteArray manufacturerName("");
    cmsAddTag(lcmsProfile, icSigDeviceMfgDescTag, manufacturerName.data());

}

QTEST_KDEMAIN(TestKoColorSpaceRegistry, NoGUI)
#include <TestKoColorSpaceRegistry.moc>

