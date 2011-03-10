/* This file is part of the KDE project
   Copyright (C) 2010 KO GmbH <jos.van.den.oever@kogmbh.com>

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

#include "pptstyle.h"

using namespace MSO;

namespace {

    enum TextTypeEnum {
        Tx_TYPE_TITLE = 0,
        Tx_TYPE_BODY,
        Tx_TYPE_NOTES,
        Tx_TYPE_OTHER = 4,
        Tx_TYPE_CENTERBODY,
        Tx_TYPE_CENTERTITLE,
        Tx_TYPE_HALFBODY,
        Tx_TYPE_QUARTERBODY
    };

const TextMasterStyleAtom*
getTextMasterStyleAtom(const MasterOrSlideContainer* m, quint16 texttype)
{
    if (!m) return 0;
    const MainMasterContainer* mm = m->anon.get<MainMasterContainer>();
    if (!mm) return 0;
    const TextMasterStyleAtom* textstyle = 0;
    foreach (const TextMasterStyleAtom&ma, mm->rgTextMasterStyle) {
        if (ma.rh.recInstance == texttype) {
            textstyle = &ma;
        }
    }
    return textstyle;
}
const TextMasterStyle9Atom*
getTextMasterStyle9Atom(const PP9SlideBinaryTagExtension* m, quint16 texttype)
{
    if (!m) return 0;
    const TextMasterStyle9Atom* textstyle = 0;
    foreach (const TextMasterStyle9Atom&ma, m->rgTextMasterStyleAtom) {
        if (ma.rh.recInstance == texttype) {
            textstyle = &ma;
        }
    }
    return textstyle;
}
const TextMasterStyle9Atom*
getTextMasterStyle9Atom(const PP9DocBinaryTagExtension* m, quint16 texttype)
{
    if (!m) return 0;
    const TextMasterStyle9Atom* textstyle = 0;
    foreach (const TextMasterStyle9Atom&ma, m->rgTextMasterStyle9) {
        if (ma.rh.recInstance == texttype) {
            textstyle = &ma;
        }
    }
    return textstyle;
}
const TextMasterStyleLevel *
getTextMasterStyleLevel(const TextMasterStyleAtom* ms, quint16 level)
{
    if (!ms) return 0;
    const TextMasterStyleLevel *l = 0;
    if (ms->rh.recInstance < 5) {
        switch (level) {
        case 0: if (ms->lstLvl1) l = ms->lstLvl1.data();break;
        case 1: if (ms->lstLvl2) l = ms->lstLvl2.data();break;
        case 2: if (ms->lstLvl3) l = ms->lstLvl3.data();break;
        case 3: if (ms->lstLvl4) l = ms->lstLvl4.data();break;
        case 4: if (ms->lstLvl5) l = ms->lstLvl5.data();break;
        }
    } else {
        if (ms->cLevels > 0 && level == ms->lstLvl1level) {
            l =  ms->lstLvl1.data();
        } else if (ms->cLevels > 1 && level == ms->lstLvl2level) {
            l =  ms->lstLvl2.data();
        } else if (ms->cLevels > 2 && level == ms->lstLvl3level) {
            l =  ms->lstLvl3.data();
        } else if (ms->cLevels > 3 && level == ms->lstLvl4level) {
            l =  ms->lstLvl4.data();
        } else if (ms->cLevels > 4 && level == ms->lstLvl5level) {
            l =  ms->lstLvl5.data();
        }
    }
    return l;
}
template <class Run>
const Run* getRun(const QList<Run> &runs, quint32 start)
{
    int i = 0;
    quint32 end = 0;
    const Run* run = 0;
    while (i < runs.size()) {
        end += runs[i].count;
        if (end > start) {
            run = &runs[i];
            break;
        }
        i++;
    }
    return run;
}
const TextPFRun* getPFRun(const TextContainer* tc, quint32 start)
{
    if (tc && tc->style) {
        return getRun<TextPFRun>(tc->style->rgTextPFRun, start);
    }
    return 0;
}
const TextPFException* getLevelPF(const MasterOrSlideContainer* m,
                                  quint32 textType, quint16 level)
{
    const TextMasterStyleAtom* ms = getTextMasterStyleAtom(m, textType);
    const TextMasterStyleLevel* ml = getTextMasterStyleLevel(ms, level);
    return (ml) ?&ml->pf :0;
}
const TextPFException* getLevelPF(const MasterOrSlideContainer* m,
                                  const TextContainer* tc, quint16 level)
{
    return (tc) ?getLevelPF(m, tc->textHeaderAtom.textType, level) :0;
}
const TextCFException* getLevelCF(const MasterOrSlideContainer* m,
                                  const TextContainer& tc, quint16 level)
{
    quint32 textType = tc.textHeaderAtom.textType;
    const TextMasterStyleAtom* ms = getTextMasterStyleAtom(m, textType);
    const TextMasterStyleLevel* ml = getTextMasterStyleLevel(ms, level);
    return (ml) ?&ml->cf :0;
}
const TextMasterStyleLevel* getBaseLevel(const MasterOrSlideContainer* m,
                                         quint32 textType, quint16 level)
{
    const TextMasterStyleAtom* ms = 0;
    const TextMasterStyleLevel* ml = 0;

    if (textType == Tx_TYPE_CENTERTITLE) {
        ms = getTextMasterStyleAtom(m, Tx_TYPE_TITLE);
        ml = getTextMasterStyleLevel(ms, level); 
    }
    //those are placeholder shapes so let's inherit from Tx_TYPE_BODY
    //NOTE: testing required!
//     else if (textType == 5 || textType == 7 || textType == 8) {
//         ms = getTextMasterStyleAtom(m, 1);
//     }

    else if (textType == Tx_TYPE_OTHER) {
        //MS Office 2007 specific at the moment 
	if (level) {
            ms = getTextMasterStyleAtom(m, Tx_TYPE_OTHER);
            ml = getTextMasterStyleLevel(ms, 0);
        }
    }
    //NOTE: Tx_TYPE_NOTES do not inherit at the moment

    return ml;
}
const TextMasterStyleLevel* getBaseLevel(const MasterOrSlideContainer* m,
                                  const TextContainer* tc, quint16 level)
{
    return (tc) ?getBaseLevel(m, tc->textHeaderAtom.textType, level) :0;
}
const TextPFException* getBaseLevelPF(const MasterOrSlideContainer* m,
                                  const TextContainer* tc, quint16 level)
{
    const TextMasterStyleLevel* ml = getBaseLevel(m, tc, level);
    return (ml) ?&ml->pf :0;
}
const TextCFException* getBaseLevelCF(const MasterOrSlideContainer* m,
                                  const TextContainer* tc, quint16 level)
{
    const TextMasterStyleLevel* ml = getBaseLevel(m, tc, level);
    return (ml) ?&ml->cf :0;
}
const TextMasterStyleLevel* getDefaultLevel(const MSO::DocumentContainer* d,
                                            quint16 level)
{
    if (!d) return 0;
    const TextMasterStyleLevel* ml = getTextMasterStyleLevel(
            &d->documentTextInfo.textMasterStyleAtom, level);
    if (!ml) {
        ml = getTextMasterStyleLevel(
                d->documentTextInfo.textMasterStyleAtom2.data(), level);
    }
    return ml;
}
const TextPFException* getDefaultLevelPF(const MSO::DocumentContainer* d,
                                         quint16 level)
{
    const TextMasterStyleLevel* ml = getDefaultLevel(d, level);
    return (ml) ?&ml->pf :0;
}
const TextCFException* getDefaultLevelCF(const MSO::DocumentContainer* d,
                                         quint16 level)
{
    const TextMasterStyleLevel* ml = getDefaultLevel(d, level);
    return (ml) ?&ml->cf :0;
}
const TextPFException* getDefaultPF(const MSO::DocumentContainer* d)
{
    if (d && d->documentTextInfo.textPFDefaultsAtom) {
        return &d->documentTextInfo.textPFDefaultsAtom->pf;
    }
    return 0;
}
const TextCFException* getDefaultCF(const MSO::DocumentContainer* d)
{
    if (d && d->documentTextInfo.textCFDefaultsAtom) {
        return &d->documentTextInfo.textCFDefaultsAtom->cf;
    }
    return 0;
}
const MSO::StyleTextProp9*
getStyleTextProp9(const MSO::DocumentContainer* d, quint32 slideIdRef,
                  quint32 textType, quint8 pp9rt)
{
    const PP9DocBinaryTagExtension* pp9 = getPP<PP9DocBinaryTagExtension>(d);
    if (pp9 && pp9->outlineTextPropsContainer) {
        foreach (const OutlineTextProps9Entry& o,
                 pp9->outlineTextPropsContainer->rgOutlineTextProps9Entry) {
            if (o.outlineTextHeaderAtom.slideIdRef == slideIdRef
                    && o.outlineTextHeaderAtom.txType == textType) {
                // we assume that pp9rt is the index in this array
                if (o.styleTextProp9Atom.rgStyleTextProp9.size() > pp9rt) {
                    return &o.styleTextProp9Atom.rgStyleTextProp9[pp9rt];
                }
            }
        }
    }
    return 0;
}
const MSO::StyleTextProp9*
getStyleTextProp9(const PptOfficeArtClientData* pcd, quint8 pp9rt)
{
    const PP9ShapeBinaryTagExtension* p = 0;
    if (pcd) {
        p = getPP<PP9ShapeBinaryTagExtension>(*pcd);
    }
    if (p && p->styleTextProp9Atom.rgStyleTextProp9.size() > pp9rt) {
        return &p->styleTextProp9Atom.rgStyleTextProp9[pp9rt];
    }
    return 0;
}
const TextPFException9* getPF9(const MSO::DocumentContainer* d,
                                    const MSO::SlideListWithTextSubContainerOrAtom* texts,
                                    const PptOfficeArtClientData* pcd,
                                    const MSO::TextContainer* tc,
                                    const int start)
{
    // to find the pf9, the cf has to be obtained which contains a pp9rt
    quint8 pp9rt = 0;
    const TextCFException* cf = getTextCFException(tc, start);
    if (cf && cf->fontStyle) {
        pp9rt = cf->fontStyle->pp9rt;
    }
    const StyleTextProp9* stp9 = 0;
    if (tc) {
        quint32 textType = tc->textHeaderAtom.textType;
        stp9 = getStyleTextProp9(pcd, pp9rt);
        if (!stp9 && texts) {
            stp9 = getStyleTextProp9(d, texts->slidePersistAtom.slideId.slideId,
                                     textType, pp9rt);
        }
    }
    return (stp9) ?&stp9->pf9 :0;
}
const TextMasterStyle9Level* getMaster9Level(const MasterOrSlideContainer* m,
                                              quint32 textType,
                                              quint16 level)
{
    const PP9SlideBinaryTagExtension* pp9 = getPP<PP9SlideBinaryTagExtension>(m);
    if (!pp9) return 0;
    const TextMasterStyle9Atom* ms = getTextMasterStyle9Atom(pp9, textType);
    const TextMasterStyle9Level *l = 0;
    if (!ms) return l;
    switch (level) {
    case 0: if (ms->lstLvl1) l = ms->lstLvl1.data();break;
    case 1: if (ms->lstLvl2) l = ms->lstLvl2.data();break;
    case 2: if (ms->lstLvl3) l = ms->lstLvl3.data();break;
    case 3: if (ms->lstLvl4) l = ms->lstLvl4.data();break;
    case 4: if (ms->lstLvl5) l = ms->lstLvl5.data();break;
    }
    return l;
}
const TextPFException9* getLevelPF9(const MasterOrSlideContainer* m,
                                    quint32 textType,
                                    quint16 level)
{
    const TextMasterStyle9Level* ml = getMaster9Level(m, textType, level);
    return (ml) ?&ml->pf9 :0;
}
const TextPFException9* getLevelPF9(const MasterOrSlideContainer* m,
                                    const MSO::TextContainer* tc,
                                    quint16 level)
{
    return (tc) ?getLevelPF9(m, tc->textHeaderAtom.textType, level) :0;
}
const TextMasterStyle9Level* getDefault9Level(const MSO::DocumentContainer* d,
                                              quint32 textType,
                                              quint16 level)
{
    const PP9DocBinaryTagExtension* pp9 = getPP<PP9DocBinaryTagExtension>(d);
    if (!pp9) return 0;
    const TextMasterStyle9Atom* ms = getTextMasterStyle9Atom(pp9, textType);
    if (!ms) return 0;
    const TextMasterStyle9Level *l = 0;
    switch (level) {
    case 0: if (ms->lstLvl1) l = ms->lstLvl1.data();break;
    case 1: if (ms->lstLvl2) l = ms->lstLvl2.data();break;
    case 2: if (ms->lstLvl3) l = ms->lstLvl3.data();break;
    case 3: if (ms->lstLvl4) l = ms->lstLvl4.data();break;
    case 4: if (ms->lstLvl5) l = ms->lstLvl5.data();break;
    }
    return l;
}
const TextPFException9* getDefaultLevelPF9(const MSO::DocumentContainer* d,
                                           quint32 textType,
                                           quint16 level)
{
    const TextMasterStyle9Level* ml = getDefault9Level(d, textType, level);
    return (ml) ?&ml->pf9 :0;
}
const TextPFException9* getDefaultLevelPF9(const MSO::DocumentContainer* d,
                                           const MSO::TextContainer* tc,
                                           quint16 level)
{
    return (tc) ?getDefaultLevelPF9(d, tc->textHeaderAtom.textType, level) :0;
}
const TextPFException9* getDefaultPF9(const MSO::DocumentContainer* d)
{
    const PP9DocBinaryTagExtension* pp9 = getPP<PP9DocBinaryTagExtension>(d);
    return (pp9 && pp9->textDefaultsAtom) ?&pp9->textDefaultsAtom->pf9 :0;
}
template <class Style>
void addStyle(const Style** list, const Style* style)
{
    if (style) {
        while (*list) ++list;
        *list = style;
        *list++;
        *list = 0;
    }
}
} //namespace

const TextCFRun* getCFRun(const TextContainer* tc, const quint32 start)
{
    if (tc && tc->style) {
        return getRun<TextCFRun>(tc->style->rgTextCFRun, start);
    }
    return 0;
}

const TextCFException*
getTextCFException(const MSO::TextContainer* tc, const int start)
{
    if (!tc || !tc->style) return 0;
    const QList<TextCFRun> &cfs = tc->style->rgTextCFRun;
    int i = 0;
    int cfend = 0;
    while (i < cfs.size()) {
        cfend += cfs[i].count;
        if (cfend > start) {
            break;
        }
        i++;
    }
    if (i >= cfs.size()) {
        return 0;
    }
    return &cfs[i].cf;
}

PptTextPFRun::PptTextPFRun(const MSO::DocumentContainer* d,
             const MSO::MasterOrSlideContainer* m,
             quint32 textType)
{
    level_ = 0;
    *pfs = 0;
    addStyle(pfs, getLevelPF(m, textType, 0));
    addStyle(pfs, getDefaultLevelPF(d, 0));
    addStyle(pfs, getDefaultPF(d));

    *pf9s = 0;
    addStyle(pf9s, getLevelPF9(m, textType, 0));
    addStyle(pf9s, getDefaultLevelPF9(d, textType, 0));
    addStyle(pf9s, getDefaultPF9(d));
}

PptTextPFRun::PptTextPFRun(const DocumentContainer* d,
                           const SlideListWithTextSubContainerOrAtom* texts,
                           const MasterOrSlideContainer* m,
                           const PptOfficeArtClientData* pcd,
                           const TextContainer* tc,
                           quint32 start)
{
    const TextPFRun* pfrun = getPFRun(tc, start);
    quint16 level = 0;
    if (pfrun) {
        level = pfrun->indentLevel;
        // [MS-PPT].pdf says there are only 5 levels
        if (level > 4) level = 4;
    }

    *pfs = 0;
    addStyle(pfs, (pfrun) ?&pfrun->pf :0);
    //check the main master/title master slide's TextMasterStyleAtom
    addStyle(pfs, getLevelPF(m, tc, level));
    addStyle(pfs, getBaseLevelPF(m, tc, level));
    //check DocumentContainer/DocumentTextInfoContainer/textMasterStyleAtom
    addStyle(pfs, getDefaultLevelPF(d, level));
    //check DocumentContainer/DocumentTextInfoContainer/textPFDefaultsAtom
    addStyle(pfs, getDefaultPF(d));

    *pf9s = 0;
    addStyle(pf9s, getPF9(d, texts, pcd, tc, start));
    addStyle(pf9s, getLevelPF9(m, tc, level));
    addStyle(pf9s, getDefaultLevelPF9(d, tc, level));
    addStyle(pf9s, getDefaultPF9(d));

    // the level reported by PptPFRun is 0 when not bullets, i.e. no list is
    // active, 1 is lowest list level, 5 is the highest list level
//     level_ = (level || fHasBullet()) ?level + 1 :0;

    //NOTE: The previous assumption is not true, there are cases when
    //(indentLevel == 1 && fHasBullet == false).  We should interpret this text
    //as content of a paragraph with corresponding indentLevel, thus avoid
    //placing it into a text:list element.
    level_ = level;
}

PptTextCFRun::PptTextCFRun(const MSO::DocumentContainer* d,
                           QList<const MSO::MasterOrSlideContainer*> &mh,
                           const MSO::TextContainer& tc,
                           quint16 level)
{
    //check the main master/title master slide's TextMasterStyleAtom
    for (int i = 0; i < mh.size(); i++) {
        cfs.append(getLevelCF(mh[i], tc, level));
        cfs.append(getBaseLevelCF(mh[i], &tc, level));
    }

    //check DocumentContainer/DocumentTextInfoContainer/textMasterStyleAtom
    cfs.append(getDefaultLevelCF(d, level));
    //check DocumentContainer/DocumentTextInfoContainer/textCFDefaultsAtom
    cfs.append(getDefaultCF(d));

    level_ = level;
    cfrun_rm = false;
}

int PptTextCFRun::addCurrentCFRun(const MSO::TextContainer& tc, quint32 start)
{
    int n = 0;

    //remove pointer to TextCFException structure of the previous run of text
    if (cfrun_rm) {
        cfs.removeFirst();
        cfrun_rm = false;
    }

    const TextCFRun* cfrun = getCFRun(&tc, start);
    if (cfrun) {
        cfs.prepend(&cfrun->cf);
        n = cfrun->count;
        cfrun_rm = true;
    }
    return n;
}

#define GETTER(TYPE, PARENT, PRE, NAME, TEST, DEFAULT) \
TYPE PptTextPFRun::NAME() const \
{ \
    const MSO::TextPFException* const * p = pfs; \
    while (*p) { \
        if ((*p)->masks.TEST) { \
            return PRE (*p)->PARENT NAME; \
        } \
        ++p; \
    } \
    return DEFAULT; \
}
//     TYPE      PARENT       PRE NAME             TEST            DEFAULT
GETTER(bool,     bulletFlags->,,  fHasBullet,      hasBullet,      false)
GETTER(bool,     bulletFlags->,,  fBulletHasFont,  bulletHasFont,  false)
GETTER(bool,     bulletFlags->,,  fBulletHasColor, bulletHasColor, false)
GETTER(bool,     bulletFlags->,,  fBulletHasSize,  bulletHasSize,  false)
GETTER(qint16,   ,             ,  bulletChar,      bulletChar,     0)
GETTER(quint16,  ,             ,  textAlignment,   align,          0)
GETTER(qint16,   ,             ,  lineSpacing,     lineSpacing,    0)
GETTER(qint16,   ,             ,  spaceBefore,     spaceBefore,    0)
GETTER(qint16,   ,             ,  spaceAfter,      spaceAfter,     0)
GETTER(quint16,  ,             ,  leftMargin,      leftMargin,     0)
GETTER(quint16,  ,             ,  indent,          indent,         0)
GETTER(quint16,  ,             ,  defaultTabSize,  defaultTabSize, 0)
GETTER(TabStops, ,             *, tabStops,        tabStops,       0)
GETTER(quint16,  ,             ,  fontAlign,       fontAlign,      0)
GETTER(bool,     wrapFlags->,  ,  charWrap,        wordWrap,       false)
GETTER(bool,     wrapFlags->,  ,  wordWrap,        wordWrap,       false)
GETTER(bool,     wrapFlags->,  ,  overflow,        overflow,       false)
GETTER(quint16,  ,             ,  textDirection,   textDirection,  0)
#undef GETTER

#define GETTER(TYPE, PRE, NAME, TEST, VALID, DEFAULT) \
TYPE PptTextPFRun::NAME() const \
{ \
    const MSO::TextPFException* const * p = pfs; \
    while (*p) { \
        if ((*p)->masks.TEST) { \
 	    if (VALID()) { \
                return PRE (*p)->NAME; \
            } \
        } \
        ++p; \
    } \
    return DEFAULT; \
}
//     TYPE             PRE NAME             TEST         VALID            DEFAULT
GETTER(quint16,          ,  bulletFontRef,   bulletFont,  fBulletHasFont,  0)
GETTER(qint16,           ,  bulletSize,      bulletSize,  fBulletHasSize,  0)
GETTER(ColorIndexStruct, *, bulletColor,     bulletColor, fBulletHasColor, ColorIndexStruct())
#undef GETTER

qint32 PptTextPFRun::bulletBlipRef() const {
    const MSO::TextPFException9* const * p = pf9s;
    while (*p) {
        if ((*p)->masks.bulletBlip) {
            return (*p)->bulletBlipRef;
        }
        ++p;
    }
    return 65535;
}
qint16 PptTextPFRun::fBulletHasAutoNumber() const {
    const MSO::TextPFException9* const * p = pf9s;
    while (*p) {
        if ((*p)->masks.bulletHasScheme) {
            return (*p)->fBulletHasAutoNumber;
        }
        ++p;
    }
    return 0;
}

#define ANM_Default 3
quint16 PptTextPFRun::scheme() const {
    const MSO::TextPFException9* const * p = pf9s;
    while (*p) {
        if ((*p)->masks.bulletScheme) {
            return (*p)->bulletAutoNumberScheme->scheme;
        }
        ++p;
    }
    
    return ANM_Default;
}
qint16 PptTextPFRun::startNum() const {
    const MSO::TextPFException9* const * p = pf9s;
    while (*p) {
        if ((*p)->masks.bulletScheme) {
            return (*p)->bulletAutoNumberScheme->startNum;
        }
        ++p;
    }
    return 1;
}

//NOTE: Logic exploited from test documents saved in MS PowerPoint 2003/2007
bool PptTextPFRun::isList() const {
    bool ret = false;
    if (fHasBullet() ||
        fBulletHasAutoNumber() ||
        (bulletBlipRef() != 65535)
//         (bulletChar() > 0) ||
//         fBulletHasFont() ||
       ) {
        ret = true;
    }
    return ret;
}

#define GETTER(TYPE, PARENT, PRE, NAME, TEST, DEFAULT) \
TYPE PptTextCFRun::NAME() const \
{ \
    for (int i = 0; i < cfs.size(); i++) { \
        if (cfs[i]) { \
            if ((cfs[i])->TEST) { \
                return PRE (cfs[i])->PARENT NAME; \
            } \
        } \
    } \
    return DEFAULT; \
}

//     TYPE             PARENT     PRE NAME           TEST              DEFAULT
GETTER(bool,            fontStyle->,,  bold,          masks.bold,        false)
GETTER(bool,            fontStyle->,,  italic,        masks.italic,      false)
GETTER(bool,            fontStyle->,,  underline,     masks.underline,   false)
GETTER(bool,            fontStyle->,,  shadow,        masks.shadow,      false)
GETTER(bool,            fontStyle->,,  fehint,        masks.fehint,      false)
GETTER(bool,            fontStyle->,,  kumi,          masks.kumi,        false)
GETTER(bool,            fontStyle->,,  emboss,        masks.emboss,      false)
GETTER(quint8,          fontStyle->,,  pp9rt,         fontStyle,             0)
GETTER(quint16,         ,           ,  fontRef,       masks.typeface,        0)
GETTER(quint16,         ,           ,  oldEAFontRef,  masks.oldEATypeface,   0)
GETTER(quint16,         ,           ,  ansiFontRef,   masks.ansiTypeface,    0)
GETTER(quint16,         ,           ,  symbolFontRef, masks.symbolTypeface,  0)
GETTER(quint16,         ,           ,  fontSize,      masks.size,            0)
GETTER(ColorIndexStruct,,           *, color,   masks.color, ColorIndexStruct())
GETTER(qint16,          ,           ,  position,      masks.position,        0)
#undef GETTER
