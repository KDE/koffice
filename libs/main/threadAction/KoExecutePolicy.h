/*
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KO_EXECUTEPOLICY_H
#define KO_EXECUTEPOLICY_H

#include "komain_export.h"

class QVariant;

class KoAction;
class KoJobsListPolicy;

/**
 * Each action that is called 'execute()' on will be executed according to policy.
 * Each action will have a policy set; that policy will be used on execution of that
 * action to determine what will happen next.
 */
class KOMAIN_EXPORT KoExecutePolicy
{
public:
    KoExecutePolicy() {}
    virtual ~KoExecutePolicy() {}
    /**
     * schedule an action according to policy.
     * @param action the action to be scheduled.
     * @param jobsList the list of jobs currently associated with the action.
     * @param params a parameters object that belongs with the current action.
     */
    virtual void schedule(KoAction *action, KoJobsListPolicy *jobsList, const QVariant &params) = 0;

    /**
     * This policy will enqueue the action; but will remove others that would call the
     * same target method. In the case of a delayed-initialisation action being executed
     * from one or even several actions it is unwanted to let the actions queue-up and be
     * executed serially; this policy will prevent that.
     * (Example)
     * Consider having a list of items that will be previed when you click it. The
     * creation of the preview is a slow process. In this example it is possible for the
     * user to select 5 items in a row which will then be previewed one after another;
     * making the update of the preview incredably slow.  If you use this policy only the
     * last action will be executed and the other actions will be discarded. Effect is
     * that only the last preview will be generated and the first 4 ignored.
     */
    static KoExecutePolicy *const onlyLastPolicy;
    /**
     * This policy will execute the action in the calling thread.
     */
    static KoExecutePolicy *const directPolicy;
    /**
     * This policy will queue each action to be executed serially, while disabling the
     * action when running. When an action comes in it will be disabled and queued; after
     * executing it will be enabled again. This way only one action can be executed at the
     * same time and additional executes will be ignored until the first is done.
     */
    static KoExecutePolicy *const queuedPolicy;
    /**
     * This policy will queue each action to be executed serially.
     */
    static KoExecutePolicy *const simpleQueuedPolicy;
    // TODO alter to staticDeleter when we depend on kdelibs.
};

#endif
