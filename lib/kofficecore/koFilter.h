/* This file is part of the KOffice libraries
   Copyright (C) 2000 Werner Trobin <trobin@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef __koffice_filter_h__
#define __koffice_filter_h__

#include <qobject.h>

class QDomDocument;
class QCString;
class KoStore;
class KoDocument;

/**
 * This is an abstract base class for filters. Please reimplement the
 * method you "like best" (i.e. best matching your needs for the filter).
 * See http://koffice.kde.org/filters/index.html for further information
 * on filter development!
 * @ref KoFilterManager
 * @ref KoFilterDialog
 *
 * @author Werner Trobin
 * @short Abstract base class for filters
 */

// Note: This class has to be derived from QObject, because we open the
// filter libs dynamically (KLibLoader)!
class KoFilter : public QObject {

    Q_OBJECT

public:
    virtual ~KoFilter() {}

    /**
     * This method takes a file as input and outputs a file. (This is the
     * "traditional way" of filtering). Your filter should open and read
     * the input file, convert the contents, and write the resulting
     * (filtered) file to the disk. It's not very fast (because of all
     * the tmp files, but it's easy and your filter doesn't rely on internals
     * of the app.
     * @param fileIn    The name of the file to filter (input file)
     * @param fileOut   Save the converted stuff to that file (output file)
     * @param from      Mimetype of the input
     * @param to        Mimetype of the output
     * @param config    A String which can be used to pass configure information (see HOWTO)
     * @return          If the method returns true the filtering was successful
     */
    virtual const bool filter(const QString &fileIn, const QString &fileOut,
                              const QString &from, const QString &to,
                              const QString &config=QString::null);

    /**
     * Takes a file as input and writes the converted information in a
     * QDomDocument. It's slightly faster than the first one, but you need
     * a part which uses QDom (e.g. KSpread, KIS,...- but NOT KWord, KPresenter!).
     * This one can only be used as import filter!!! (hence the I_ :)
     * @param file      The filename of the input file
     * @param from      Mimetype of the input
     * @param to        Mimetype of the output
     * @param doc       An empty QDomDocument, write the converted information in there
     * @param config    A String which can be used to pass configure information
     * @return          If the method returns true the filtering was successful
     */
    virtual const bool I_filter(const QString &file, const QString &from,
                                QDomDocument &doc, const QString &to,
                                const QString &config=QString::null);

    /**
     * This is the most hacky method(tm) available. Here you have direct
     * access to the (empty) KoDocument. Reimplement this method if you want
     * to IMPORT data directly into a KoDocument. Note: This one is needed
     * only for very special filters which have to transfer huge amounts of
     * data (e.g. image import). Normally you shouldn't use this one!!!
     * @param file      The filename of the input file
     * @param document  The (empty) KoDocument of the part
     * @param from      Mimetype of the input
     * @param to        Mimetype of the output
     * @param config    A String which can be used to pass configure information
     * @return          If the method returns true the filtering was successful
     */
    virtual const bool I_filter(const QString &file, KoDocument *document,
                                const QString &from, const QString &to,
                                const QString &config=QString::null);

    /**
     * This is another very nasty method. Here you have direct access to the
     * "full" KoDocument (read only, if you don't cast away the constness :)
     * Reimplement this method if you want to EXPORT data. Note: Normally you
     * won't need direct access to the document! Use this method only in very
     * special clases where everything else is impossible! Beware of corrupting
     * the document!!!
     * @param file      The filename of the input file
     * @param document  The KoDocument of the part
     * @param from      Mimetype of the input
     * @param to        Mimetype of the output
     * @param config    A String which can be used to pass configure information
     * @return          If the method returns true the filtering was successful
     */
    virtual const bool E_filter(const QString &file, const KoDocument * const document,
                                const QString &from, const QString &to,
                                const QString &config=QString::null);

signals:
    void sigProgress(int value);

protected:
    KoFilter(KoFilter *parent, const char *name);   // only for classes inheriting this one

private:
    KoFilter();                                 // Meyers says this is nice :)
    KoFilter &operator=(const KoFilter &);
};
#endif
