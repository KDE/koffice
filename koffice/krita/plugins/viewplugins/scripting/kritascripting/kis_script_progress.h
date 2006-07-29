/*
 *  Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _KIS_SCRIPT_PROGRESS_H_
#define _KIS_SCRIPT_PROGRESS_H_

#include <kis_progress_subject.h>

class KisView;

/**
 * TODO: clarify the situation, while, in the future, multiple scripts could be running at a same time,
 * some of the functions are global to all script and some aren't.
 */
class KisScriptProgress : public KisProgressSubject
{
    public:
        KisScriptProgress(KisView* view) : m_view(view) {};
    public:
        /**
         * This function will set this class as the KisProgressSubject in view
         */
        void activateAsSubject();
        virtual void cancel() {};
    public:
        void setProgressTotalSteps(Q_INT32 totalSteps);
        void setProgress(Q_INT32 progress);
        void incProgress();
        void setProgressStage(const QString& stage, Q_INT32 progress);
        void progressDone();
        inline void setPackagePath(QString path) { m_packagePath = path; };
        inline QString packagePath() { return m_packagePath; }
    private:
        Q_INT32 m_progressSteps, m_progressTotalSteps, m_lastProgressPerCent;
        KisView * m_view;
        QString m_packagePath;
};

#endif
