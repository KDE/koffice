/*
   This file is part of the KDE project
   Copyright (C) 2000 Werner Trobin <wtrobin@mandrakesoft.com>

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

#ifndef koTemplates_h
#define koTemplates_h

#include <qlist.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qpixmap.h>

class KInstance;


class KoTemplate {

public:
    KoTemplate(const QString &name,
	       const QString &file=QString::null,
	       const QString &picture=QString::null,
	       const bool &hidden=false, const bool &touched=false);
    ~KoTemplate() {}

    QString name() const { return m_name; }
    QString file() const { return m_file; }
    QString picture() const { return m_picture; }
    const QPixmap &loadPicture();

    const bool &isHidden() const { return m_hidden; }
    void setHidden(const bool hidden=true) { m_hidden=hidden; m_touched=true; }

    const bool &touched() const { return m_touched; }

private:
    QString m_name, m_file, m_picture;
    bool m_hidden;
    mutable bool m_touched;
    QPixmap m_pixmap;
    bool m_cached;
};


class KoTemplateGroup {

public:
    KoTemplateGroup(const QString &name,
		    const QString &dir=QString::null,
		    const bool &touched=false);
    ~KoTemplateGroup() {}

    QString name() const { return m_name; }
    QStringList dirs() const { return m_dirs; }
    void addDir(const QString &dir) { m_dirs.append(dir); m_touched=true; }

    // If all children are hidden, we are hidden too
    const bool isHidden() const;
    // if we should hide, we hide all the children
    void setHidden(const bool &hidden=true) const;

    KoTemplate *first() { return m_templates.first(); }
    KoTemplate *next() { return m_templates.next(); }
    KoTemplate *last() { return m_templates.last(); }
    KoTemplate *prev() { return m_templates.prev(); }
    KoTemplate *current() { return m_templates.current(); }

    void add(KoTemplate *t, bool touch=true);
    KoTemplate *find(const QString &name) const;

    const bool &touched() const { return m_touched; }

private:
    QString m_name;
    QStringList m_dirs;
    QList<KoTemplate> m_templates;
    mutable bool m_touched;
};


class KoTemplateTree {

public:
    KoTemplateTree(const QString &templateType, KInstance *instance,
		   const bool &readTree=false);
    ~KoTemplateTree() {}

    QString templateType() const { return m_templateType; }
    KInstance *instance() const { return m_instance; }
    void readTemplateTree();
    void writeTemplateTree();

    KoTemplateGroup *first() { return m_groups.first(); }
    KoTemplateGroup *next() { return m_groups.next(); }
    KoTemplateGroup *last() { return m_groups.last(); }
    KoTemplateGroup *prev() { return m_groups.prev(); }
    KoTemplateGroup *current() { return m_groups.current(); }

    void add(KoTemplateGroup *g);
    KoTemplateGroup *find(const QString &name) const;

private:
    void readGroups();
    void readTemplates();
    void createLocalTemplateList(QStringList &list);
    void writeOutLocalTemplates(const QStringList &list);
    void createGroupDir(const QString &localDir, KoTemplateGroup *group,
			QStringList &localTemplates);
    void writeTemplate(KoTemplate *t, KoTemplateGroup *group,
		       const QString &localDir);

    QString m_templateType;
    KInstance *m_instance;
    QList<KoTemplateGroup> m_groups;
};


namespace KoTemplates {
QString stripWhiteSpace(const QString &string);
};

#endif
