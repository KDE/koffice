/* This file is part of the KDE project
   Copyright (C) 2003 Alexander Dymo <cloudtemple@mksat.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this library; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#include <qapplication.h>

#include "kugarmain.h"

int main(int argc, char **argv)
{
    QApplication kugarApp(argc, argv);

    if (argv[1] == 0)
    {
        qWarning("No data file specified");
        return 1;
    }
    bool print = false;
    if (argv[2] != 0)
        if (QString(argv[2]) == QString("--print"))
            print = true;
    QString dataFile(argv[1]);

    KugarMain kugarMain(dataFile);
    if (!print)
    {
        kugarMain.show();
        kugarApp.setMainWidget(&kugarMain);
    }
    else
    {
        kugarMain.fileQuickPrint();
        QObject::connect(&kugarApp, SIGNAL(lastWindowClosed()), &kugarApp, SLOT(quit()));
        kugarApp.quit();
        return 0;
    }

    return kugarApp.exec();
}
