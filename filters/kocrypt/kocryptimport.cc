/* This file is part of the KDE project
   Copyright (C) 2001 George Staikos <staikos@kde.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <config.h>

#include "cbc.h"
#include "blowfish.h"
#include "sha1.h"

#include <kocryptdefs.h>

#include <qfile.h>
#include <qtextstream.h>
#include <kocryptimport.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <koFilterChain.h>
#include <kgenericfactory.h>
#include <qapplication.h>

#include "pwdprompt.h"

typedef KGenericFactory<KoCryptImport, KoFilter> KoCryptImportFactory;
K_EXPORT_COMPONENT_FACTORY( libkocryptimport, KoCryptImportFactory( "kocryptfilter" ) )


#define READ_ERROR_CHECK(XX)  do {                                         \
      if (rc != XX) {                                                      \
        QApplication::setOverrideCursor(Qt::arrowCursor);                  \
        KMessageBox::error(NULL,                                           \
              i18n("The password is invalid or the file is corrupted."),   \
              i18n("Encrypted Document Import"));                          \
         QApplication::restoreOverrideCursor();                            \
         goto start;                                                       \
      }                                                                    \
      } while(0)


#define WRITE_ERROR_CHECK(XX)  do {                                        \
      if (rc != XX) {                                                      \
         QApplication::setOverrideCursor(Qt::arrowCursor);                 \
         KMessageBox::error(NULL,                                          \
              i18n("Disk write error - out of space?"),                    \
              i18n("Encrypted Document Import"));                          \
         QApplication::restoreOverrideCursor();                            \
         outf.remove();                                                    \
         return KoFilter::StupidError;                                     \
      }                                                                    \
      } while(0)


#define CRYPT_ERROR_CHECK()  do {                                          \
      if (rc != blocksize) {                                               \
         QApplication::setOverrideCursor(Qt::arrowCursor);                 \
         KMessageBox::error(NULL,                                          \
              i18n("There was an internal error while decrypting the file."), \
              i18n("Encrypted Document Import"));                          \
         QApplication::restoreOverrideCursor();                            \
         outf.remove();                                                    \
         return KoFilter::StupidError;                                     \
      }                                                                    \
      } while(0)


#define SHA_ERROR()  do {                                                  \
         QApplication::setOverrideCursor(Qt::arrowCursor);                 \
         KMessageBox::error(NULL,                                          \
              i18n("There was an internal error verifying the file."),     \
              i18n("Encrypted Document Import"));                          \
         QApplication::restoreOverrideCursor();                            \
         return KoFilter::StupidError;                                     \
      } while(0)



KoCryptImport::KoCryptImport(KoFilter *, const char *, const QStringList&) :
                             KoFilter() {
}


KoFilter::ConversionStatus KoCryptImport::convert( const QCString& from, const QCString& to )
{
int ftype = -1;
int blocksize = 64;
int rc;

    if (to == "application/x-kword" && from == "application/x-kword-crypt")
    {
       ftype = APPID_KWORD;
    } else
    if (to == "application/x-kspread" && from == "application/x-kspread-crypt")
    {
       ftype = APPID_KSPREAD;
    } else {
        return KoFilter::NotImplemented;
    }

    //kdDebug() << "Crypto Filter Parameters: " << config << endl;

    // OK.  Get this.  I'm not going to add 4 lines of code to this thing and
    // nest it in another [infinite] loop just so someone can feel warm and
    // fuzzy because they found a complicated way to avoid using a perfectly
    // fine goto.  This is my code and I like the goto just the way it is.
    // Deal with it.
start:
QFile inf(m_chain->inputFile());
QFile outf(m_chain->outputFile());

    PasswordPrompt *pp = new PasswordPrompt(true);
    connect(pp, SIGNAL(setPassword(QString)), this, SLOT(setPassword(QString)));
    int dlgrc = pp->exec();
    delete pp;
    if (dlgrc == QDialog::Rejected) {
       outf.remove();  // incase it exists, lets not crash
       return KoFilter::UserCancelled;
    }

    BlowFish cipher;
    CipherBlockChain cbc(&cipher);
    SHA1 sha1;
    char thekey[512];

    strncpy(thekey, pass.latin1(), 56);
    thekey[56] = 0;

    // propagates to the cipher
    if (!cbc.setKey((void *)thekey, strlen(thekey)*8)) {
       QApplication::setOverrideCursor(Qt::arrowCursor);
       KMessageBox::error(NULL,
                  i18n("There was an internal error preparing the passphrase."),
                  i18n("Encrypted Document Import"));
       QApplication::restoreOverrideCursor();
       return KoFilter::StupidError;
    }

    if (cbc.blockSize() > 0) blocksize = cbc.blockSize();

    // This is bad.  We don't have a buffer big enough for this anyways.
    if (blocksize > 2048 || !sha1.readyToGo()) {
       QApplication::setOverrideCursor(Qt::arrowCursor);
       KMessageBox::error(NULL,
                  i18n("There was an internal error in the cipher code."),
                  i18n("Encrypted Document Import"));
       QApplication::restoreOverrideCursor();
       return KoFilter::StupidError;
    }

    inf.open(IO_ReadOnly);
    outf.open(IO_WriteOnly);

    char p[8192];

    // check the header
    rc = inf.readBlock(p, 9);
    READ_ERROR_CHECK(9);
    if (p[0] != FID_FIRST || p[1] != FID_SECOND || p[2] != FID_THIRD ||
        p[3] != ftype) {
       QApplication::setOverrideCursor(Qt::arrowCursor);
       KMessageBox::error(NULL,
                  i18n("This is the wrong document type or the document is corrupt."),
                  i18n("Encrypted Document Import"));
       QApplication::restoreOverrideCursor();
       outf.remove();
       return KoFilter::BadMimeType;   // wrong file type!
    }

    /*
     *   File format version determination.  Here we should set flags to be
     *   able to read older file formats.
     */
    if (p[4] != FID_FVER) {
       QApplication::setOverrideCursor(Qt::arrowCursor);
       KMessageBox::error(NULL,
                  i18n("I don't understand this version of the file format."),
                  i18n("Encrypted Document Import"));
       QApplication::restoreOverrideCursor();
       outf.remove();
       return KoFilter::StupidError;   // right now there is only one fileversion to understand!
    }

    /*
     *   Crypto algorithm determination - here we set flags to take old
     *   algorithms into account if we're more recent.
     */
    if (p[5] != 0 || p[6] != 0 || p[7] != 0 || p[8] != 0) {
       QApplication::setOverrideCursor(Qt::arrowCursor);
       KMessageBox::error(NULL,
                  i18n("I don't understand this version of the file format."),
                  i18n("Encrypted Document Import"));
       QApplication::restoreOverrideCursor();
       outf.remove();
       return KoFilter::StupidError;   // we only know one crypto algorithm too.
    }

    /*
     *   Decrypt it.  don't forget to toss the extra data at the beginning and
     *   end of the file!
     */
    rc = inf.readBlock(p, blocksize);
    READ_ERROR_CHECK(blocksize);
    rc = cbc.decrypt(p, blocksize);
    CRYPT_ERROR_CHECK();

    unsigned int previous_rand = ((unsigned char)p[0] + ((unsigned char)p[1] << 8)) % 5120;
    unsigned int fsize;

    // We skip the rest of this block since previous_rand%5120 has to be >=
    // blocksize.
    //kdDebug() << "             previous_rand = " << previous_rand << endl;
    previous_rand -= (blocksize-2);

    unsigned int remaining = 0;
    while (previous_rand > 0) {
      rc = inf.readBlock(p, blocksize);
      READ_ERROR_CHECK(blocksize);
      rc = cbc.decrypt(p, blocksize);
      CRYPT_ERROR_CHECK();
      if (previous_rand >= (unsigned int)blocksize) {
         previous_rand -= blocksize;
         continue;
      } else {
         remaining = (blocksize - previous_rand);
         previous_rand = 0;
      }
    }

    // read in the file size
    fsize = 0;
    for (int i = 0; i < 4; i++) {
       if (remaining == 0) {
          rc = inf.readBlock(p, blocksize);
          READ_ERROR_CHECK(blocksize);
          rc = cbc.decrypt(p, blocksize);
          CRYPT_ERROR_CHECK();
          remaining = blocksize;
       }
       fsize += (unsigned char)p[blocksize-remaining] << i*8;
       remaining--;
    }

    //kdDebug() << "             fsize = " << fsize << endl;
    // Empty out this remaining block that we read in
    if (remaining > 0) {
      if (remaining > fsize) {
        if (sha1.process(&(p[blocksize-remaining]), fsize) != (int)fsize)
           SHA_ERROR();
        rc = outf.writeBlock(&(p[blocksize-remaining]), fsize);
        WRITE_ERROR_CHECK((int)fsize);
        remaining -= fsize;
        fsize = 0;
      } else {
        if (sha1.process(&(p[blocksize-remaining]), remaining) != (int)remaining)
           SHA_ERROR();
        rc = outf.writeBlock(&(p[blocksize-remaining]), remaining);
        WRITE_ERROR_CHECK((int)remaining);
        fsize -= remaining;
        remaining = 0;
      }
    }

    // read in the rest of the file and decode
    while (fsize > 0) {
      rc = inf.readBlock(p, blocksize);
      READ_ERROR_CHECK(blocksize);
      rc = cbc.decrypt(p, blocksize);
      CRYPT_ERROR_CHECK();

      if (fsize >= (unsigned int)blocksize) {
         if (sha1.process(p, blocksize) != blocksize)
            SHA_ERROR();
         rc = outf.writeBlock(p, blocksize);
         WRITE_ERROR_CHECK(blocksize);
         fsize -= blocksize;
         remaining = 0;
         continue;
      } else {
         if (sha1.process(p, fsize) != (int)fsize)
            SHA_ERROR();
         rc = outf.writeBlock(p, fsize);
         WRITE_ERROR_CHECK((int)fsize);
         remaining = blocksize - fsize;
         fsize = 0;
      }
    }

    const unsigned char *res = sha1.getHash();
    int cnt = 0;

    /**************************************************

    This is for debugging only.  It dumps out the apparent hash.

    if (res) {
       for (int i = 0; i < 20; i++) {
          printf("%.2X", res[i]);
          if (i>0 && (i-1)%2 == 0) printf(" ");
       }
       printf("\n");
    }
    ***************************************************/

    while (cnt < 20) {
       while (remaining > 0 && cnt < 20) {
          if (res[cnt] != (unsigned char)p[blocksize-remaining]) {
            // fprintf(stderr, "ERROR: byte %d was %.2X but we computed %.2X\n",
            // cnt, (unsigned char)p[blocksize-remaining], res[cnt]);
             QApplication::setOverrideCursor(Qt::arrowCursor);
             KMessageBox::error(NULL,
                  i18n("This document is either corrupt or has been tampered with."),
                  i18n("Encrypted Document Import"));
             QApplication::restoreOverrideCursor();
             return KoFilter::StupidError;
          }
          cnt++; remaining--;
       }
       if (cnt == 20) break;
       rc = inf.readBlock(p, blocksize);
       READ_ERROR_CHECK(blocksize);
       rc = cbc.decrypt(p, blocksize);
       CRYPT_ERROR_CHECK();
       remaining = rc;
    }

    return KoFilter::OK;
}



void KoCryptImport::setPassword(QString p) {
  pass = p;
}


#include "kocryptimport.moc"

