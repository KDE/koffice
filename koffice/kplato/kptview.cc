/* This file is part of the KDE project
   Copyright (C) 1998, 1999, 2000 Torben Weis <weis@kde.org>
   Copyright (C) 2002 - 2005 Dag Andersen <danders@get2net.dk>

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

#include <kprinter.h>
#include <kmessagebox.h>

#include <KoMainWindow.h>

#include <qapplication.h>
#include <qpainter.h>
#include <qiconset.h>
#include <qlayout.h>
#include <qsplitter.h>
#include <qcanvas.h>
#include <qscrollview.h>
#include <qcolor.h>
#include <qlabel.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qvbox.h>
#include <qgrid.h>
#include <qsize.h>
#include <qheader.h>
#include <qtabwidget.h>
#include <qwidgetstack.h>
#include <qtimer.h>
#include <qpopupmenu.h>
#include <qpair.h>

#include <kiconloader.h>
#include <kaction.h>
#include <kstdaction.h>
#include <klocale.h>
#include <kdebug.h>
#include <klistview.h>
#include <kstdaccel.h>
#include <kaccelgen.h>
#include <kdeversion.h>
#include <kstatusbar.h>
#include <kxmlguifactory.h>

#include <kstandarddirs.h>
#include <kdesktopfile.h>
#include <kcommand.h>
#include <kfiledialog.h>

#include "kptview.h"
#include "kptaccountsview.h"
#include "kptfactory.h"
#include "kptmilestoneprogressdialog.h"
#include "kptnode.h"
#include "kptpart.h"
#include "kptproject.h"
#include "kptmainprojectdialog.h"
#include "kptprojectdialog.h"
#include "kpttask.h"
#include "kptsummarytaskdialog.h"
#include "kpttaskdialog.h"
#include "kpttaskprogressdialog.h"
#include "kptganttview.h"
#include "kptpertview.h"
//#include "kptreportview.h"
#include "kptdatetime.h"
#include "kptcommand.h"
#include "kptrelation.h"
#include "kptrelationdialog.h"
#include "kptresourceview.h"
#include "kptresourcedialog.h"
#include "kptresource.h"
#include "kptresourcesdialog.h"
#include "kptcalendarlistdialog.h"
#include "kptstandardworktimedialog.h"
#include "kptcanvasitem.h"
#include "kptconfigdialog.h"
#include "kptwbsdefinitiondialog.h"
#include "kptaccountsdialog.h"

#include "KDGanttView.h"
#include "KDGanttViewTaskItem.h"
#include "KPtViewIface.h"

namespace KPlato
{

View::View(Part* part, QWidget* parent, const char* /*name*/)
    : KoView(part, parent, "Main View"),
    m_ganttview(0),
    m_ganttlayout(0),
    m_pertview(0),
    m_pertlayout(0),
//    m_reportview(0),
    m_baselineMode(false),
    m_currentEstimateType(Effort::Use_Expected)
{
    //kdDebug()<<k_funcinfo<<endl;
    getProject().setCurrentSchedule(Schedule::Expected);
    
    setInstance(Factory::global());
    if ( !part->isReadWrite() )
        setXMLFile("kplato_readonly.rc");
    else
        setXMLFile("kplato.rc");
    m_dcop = 0L;
    // build the DCOP object
    dcopObject();

    m_tab = new QWidgetStack(this);
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->add(m_tab);

    m_ganttview = new GanttView(m_tab, part->isReadWrite());
    m_tab->addWidget(m_ganttview);
    m_updateGanttview = false;
    m_ganttview->draw(getPart()->getProject());

    m_pertview = new PertView( this, m_tab, layout );
    m_tab->addWidget(m_pertview);

    m_resourceview = new ResourceView( this, m_tab );
    m_updateResourceview = true;
    m_tab->addWidget(m_resourceview);
    
    m_accountsview = new AccountsView( getProject(), this, m_tab );
    m_updateAccountsview = true;
    m_tab->addWidget(m_accountsview);
    
    //m_reportview = new ReportView(this, m_tab);
    //m_tab->addWidget(m_reportview);

    connect(m_tab, SIGNAL(aboutToShow(QWidget *)), this, SLOT(slotAboutToShow(QWidget *)));
    
    connect(m_pertview, SIGNAL(addRelation(Node*, Node*)), SLOT(slotAddRelation(Node*, Node*)));
    connect(m_pertview, SIGNAL(modifyRelation(Relation*)), SLOT(slotModifyRelation(Relation*)));

	connect(m_ganttview, SIGNAL(enableActions(bool)), SLOT(setTaskActionsEnabled(bool)));
    connect(m_ganttview, SIGNAL(addRelation(Node*, Node*, int)), SLOT(slotAddRelation(Node*, Node*, int)));
    connect(m_ganttview, SIGNAL(modifyRelation(Relation*, int)), SLOT(slotModifyRelation(Relation*, int)));
    connect(m_ganttview, SIGNAL(modifyRelation(Relation*)), SLOT(slotModifyRelation(Relation*)));
    connect(m_ganttview, SIGNAL(itemDoubleClicked()), SLOT(slotOpenNode()));
    connect(m_ganttview, SIGNAL(itemRenamed(Node*, const QString&)),this,SLOT(slotRenameNode(Node*, const QString&)));
    connect(m_ganttview, SIGNAL(requestPopupMenu(const QString&, const QPoint &)),this,SLOT(slotPopupMenu(const QString&, const QPoint&)));
    connect(m_resourceview, SIGNAL(itemDoubleClicked()), SLOT(slotEditResource()));

    // The menu items
    // ------ Edit
    actionCut = KStdAction::cut( this, SLOT( slotEditCut() ), actionCollection(), "edit_cut" );
    actionCopy = KStdAction::copy( this, SLOT( slotEditCopy() ), actionCollection(), "edit_copy" );
    actionPaste = KStdAction::paste( this, SLOT( slotEditPaste() ), actionCollection(), "edit_paste" );

    actionIndentTask = new KAction(i18n("Indent Task"), "indent_task", 0, this,
        SLOT(slotIndentTask()), actionCollection(), "indent_task");
    actionUnindentTask = new KAction(i18n("Unindent Task"), "unindent_task", 0, this,
        SLOT(slotUnindentTask()), actionCollection(), "unindent_task");
    actionMoveTaskUp = new KAction(i18n("Move Up"), "move_task_up", 0, this,
        SLOT(slotMoveTaskUp()), actionCollection(), "move_task_up");
    actionMoveTaskDown = new KAction(i18n("Move Down"), "move_task_down", 0, this,
        SLOT(slotMoveTaskDown()), actionCollection(), "move_task_down");

    // ------ View
    actionViewGantt = new KAction(i18n("Gantt"), "gantt_chart", 0, this, SLOT(slotViewGantt()), actionCollection(), "view_gantt");
    
    QString group = "EstimationType";
    actionViewExpected = new KRadioAction(i18n("Expected"), 0, 0, this, SLOT(slotViewExpected()), actionCollection(), "view_expected");
    actionViewExpected->setExclusiveGroup(group);
    actionViewOptimistic = new KRadioAction(i18n("Optimistic"), 0, 0, this, SLOT(slotViewOptimistic()), actionCollection(), "view_optimistic");
    actionViewOptimistic->setExclusiveGroup(group);
    actionViewPessimistic = new KRadioAction(i18n("Pessimistic"), 0, 0, this, SLOT(slotViewPessimistic()), actionCollection(), "view_pessimistic");
    actionViewPessimistic->setExclusiveGroup(group);

    actionViewGanttResources = new KToggleAction(i18n("Resources"), 0, 0, this, SLOT(slotViewGanttResources()), actionCollection(), "view_gantt_showResources");
    actionViewGanttTaskName = new KToggleAction(i18n("Task Name"), 0, 0, this, SLOT(slotViewGanttTaskName()), actionCollection(), "view_gantt_showTaskName");
    actionViewGanttTaskLinks = new KToggleAction(i18n("Task Links"), 0, 0, this, SLOT(slotViewGanttTaskLinks()), actionCollection(), "view_gantt_showTaskLinks");
    actionViewGanttProgress = new KToggleAction(i18n("Progress"), 0, 0, this, SLOT(slotViewGanttProgress()), actionCollection(), "view_gantt_showProgress");
    actionViewGanttFloat = new KToggleAction(i18n("Float"), 0, 0, this, SLOT(slotViewGanttFloat()), actionCollection(), "view_gantt_showFloat");
    actionViewGanttCriticalTasks = new KToggleAction(i18n("Critical Tasks"), 0, 0, this, SLOT(slotViewGanttCriticalTasks()), actionCollection(), "view_gantt_showCriticalTasks");
    actionViewGanttCriticalPath = new KToggleAction(i18n("Critical Path"), 0, 0, this, SLOT(slotViewGanttCriticalPath()), actionCollection(), "view_gantt_showCriticalPath");
    
//    actionViewGanttNotScheduled = new KToggleAction(i18n("Not Scheduled"), 0, 0, this, SLOT(slotViewGanttNotScheduled()), actionCollection(), "view_gantt_showNotScheduled");
    
    actionViewTaskAppointments = new KToggleAction(i18n("Show allocations"), 0, 0, this, SLOT(slotViewTaskAppointments()), actionCollection(), "view_task_appointments");

    actionViewPert = new KAction(i18n("Network"), "pert_chart", 0, this, SLOT(slotViewPert()), actionCollection(), "view_pert");
    
    actionViewResources = new KAction(i18n("Resources"), "resources", 0, this, SLOT(slotViewResources()), actionCollection(), "view_resources");
    
    actionViewResourceAppointments = new KToggleAction(i18n("Show allocations"), 0, 0, this, SLOT(slotViewResourceAppointments()), actionCollection(), "view_resource_appointments");

    actionViewAccounts = new KAction(i18n("Accounts"), "accounts", 0, this, SLOT(slotViewAccounts()), actionCollection(), "view_accounts");

    //actionViewReports = new KAction(i18n("Reports"), "reports", 0, this, SLOT(slotViewReports()), actionCollection(), "view_reports");

    // ------ Insert
    actionAddTask = new KAction(i18n("Task..."), "add_task", 0, this,
        SLOT(slotAddTask()), actionCollection(), "add_task");
    actionAddSubtask = new KAction(i18n("Sub-Task..."), "add_sub_task", 0, this,
        SLOT(slotAddSubTask()), actionCollection(), "add_sub_task");
    actionAddMilestone = new KAction(i18n("Milestone..."), "add_milestone", 0, this,
        SLOT(slotAddMilestone()), actionCollection(), "add_milestone");

    // ------ Project
    actionEditMainProject = new KAction(i18n("Edit Main Project..."), "edit", 0, this, SLOT(slotProjectEdit()), actionCollection(), "project_edit");
    actionEditStandardWorktime = new KAction(i18n("Edit Standard Worktime..."), "edit", 0, this, SLOT(slotProjectWorktime()), actionCollection(), "project_worktime");
    actionEditCalendar = new KAction(i18n("Edit Calendar..."), "edit", 0, this, SLOT(slotProjectCalendar()), actionCollection(), "project_calendar");
    actionEditAccounts = new KAction(i18n("Edit Accounts..."), "edit", 0, this, SLOT(slotProjectAccounts()), actionCollection(), "project_accounts");
    actionEditResources = new KAction(i18n("Edit Resources..."), "edit", 0, this, SLOT(slotProjectResources()), actionCollection(), "project_resources");
    
    actionCalculate = new KActionMenu(i18n("Calculate"), "project_calculate",  actionCollection(), "project_calculate");
    connect(actionCalculate, SIGNAL(activated()), SLOT(slotProjectCalculate()));
    
    actionCalculateExpected = new KAction(i18n("Expected"), 0, 0, this, SLOT(slotProjectCalculateExpected()), actionCollection(), "project_calculate_expected");
    actionCalculate->insert(actionCalculateExpected);
    
    actionCalculateOptimistic = new KAction(i18n("Optimistic"), 0, 0, this, SLOT(slotProjectCalculateOptimistic()), actionCollection(), "project_calculate_optimistic");
    actionCalculate->insert(actionCalculateOptimistic);
    
    actionCalculatePessimistic = new KAction(i18n("Pessimistic"), 0, 0, this, SLOT(slotProjectCalculatePessimistic()), actionCollection(), "project_calculate_pessimistic");
    actionCalculate->insert(actionCalculatePessimistic);
    
/*    // ------ Reports
    actionFirstpage = KStdAction::firstPage(m_reportview,SLOT(slotPrevPage()),actionCollection(),"go_firstpage");
    connect(m_reportview, SIGNAL(setFirstPageActionEnabled(bool)), actionFirstpage, SLOT(setEnabled(bool)));
    actionPriorpage = KStdAction::prior(m_reportview,SLOT(slotPrevPage()),actionCollection(),"go_prevpage");
    connect(m_reportview, SIGNAL(setPriorPageActionEnabled(bool)), actionPriorpage, SLOT(setEnabled(bool)));
    actionNextpage = KStdAction::next(m_reportview,SLOT(slotNextPage()),actionCollection(), "go_nextpage");
    connect(m_reportview, SIGNAL(setNextPageActionEnabled(bool)), actionNextpage, SLOT(setEnabled(bool)));
    actionLastpage = KStdAction::lastPage(m_reportview,SLOT(slotLastPage()),actionCollection(), "go_lastpage");
    connect(m_reportview, SIGNAL(setLastPageActionEnabled(bool)), actionLastpage, SLOT(setEnabled(bool)));
    m_reportview->enableNavigationBtn();*/
    mainWindow()->toolBar("report")->hide();
    
//     new KAction(i18n("Design..."), "report_design", 0, this,
//         SLOT(slotReportDesign()), actionCollection(), "report_design");


    // ------ Tools
    actionDefineWBS = 
        new KAction(i18n("Define WBS Pattern..."), "tools_define_wbs", 0, this,
        SLOT(slotDefineWBS()), actionCollection(), "tools_generate_wbs");
    actionGenerateWBS = 
        new KAction(i18n("Generate WBS Code"), "tools_generate_wbs", 0, this,
        SLOT(slotGenerateWBS()), actionCollection(), "tools_define_wbs");
    
    // ------ Export (testing)
    //actionExportGantt = new KAction(i18n("Export Ganttview"), "export_gantt", 0, this,
    //    SLOT(slotExportGantt()), actionCollection(), "export_gantt");
    
    // ------ Settings
    actionConfigure = new KAction(i18n("Configure KPlato..."), "configure", 0, this,
        SLOT(slotConfigure()), actionCollection(), "configure");
    
    // ------ Popup
    actionOpenNode = new KAction(i18n("Edit..."), "edit", 0, this,
        SLOT(slotOpenNode()), actionCollection(), "node_properties");
    actionTaskProgress = new KAction(i18n("Progress..."), "edit", 0, this,
        SLOT(slotTaskProgress()), actionCollection(), "task_progress");
    actionDeleteTask = new KAction(i18n("Delete Task"), "editdelete", 0, this,
        SLOT(slotDeleteTask()), actionCollection(), "delete_task");

    actionEditResource = new KAction(i18n("Edit Resource..."), "edit", 0, this,
        SLOT(slotEditResource()), actionCollection(), "edit_resource");

    // ------------------- Actions with a key binding and no GUI item
    // Temporary, till we get a menu entry
    actNoInformation = new KAction("Toggle no information", CTRL+SHIFT+Key_T, this, SLOT(slotViewGanttNoInformation()), actionCollection(), "show_noinformation");
    
#ifndef NDEBUG
    //new KAction("Print Debug", CTRL+SHIFT+Key_P, this, SLOT( slotPrintDebug()), actionCollection(), "print_debug");
    new KAction("Print Debug", CTRL+SHIFT+Key_P, this, SLOT(slotPrintSelectedDebug()), actionCollection(), "print_debug");
    new KAction("Print Calendar Debug", CTRL+SHIFT+Key_C, this, SLOT(slotPrintCalendarDebug()), actionCollection(), "print_calendar_debug");
//     new KAction("Print Test Debug", CTRL+SHIFT+Key_T, this, SLOT(slotPrintTestDebug()), actionCollection(), "print_test_debug");

    KAction* actExportGantt = new KAction( i18n( "Export Gantt" ), CTRL+SHIFT+Key_G,
                        this, SLOT( slotExportGantt() ), actionCollection(), "export_gantt" );

#endif
    // Stupid compilers ;)
#ifndef NDEBUG
/*  Q_UNUSED( actPrintSelectedDebug );
    Q_UNUSED( actPrintCalendarDebug );*/
    Q_UNUSED( actExportGantt );
#endif

    m_estlabel = new KStatusBarLabel("", 0);
    addStatusBarItem(m_estlabel, 0, true);
    actionViewExpected->setChecked(true); //TODO: context
    setScheduleActionsEnabled();
    slotViewExpected();
    
    setTaskActionsEnabled(false);
}

View::~View()
{
    delete m_dcop;
    removeStatusBarItem(m_estlabel);
    delete m_estlabel;
}

DCOPObject * View::dcopObject()
{
  if ( !m_dcop )
    m_dcop = new ViewIface( this );

  return m_dcop;
}


Project& View::getProject() const
{
	return getPart()->getProject();
}

void View::setZoom(double zoom) {
    m_ganttview->setZoom(zoom);
	m_pertview->setZoom(zoom);
}

void View::setupPrinter(KPrinter &/*printer*/) {
    //kdDebug()<<k_funcinfo<<endl;
}

void View::print(KPrinter &printer) {
    //kdDebug()<<k_funcinfo<<endl;
    if (printer.previewOnly()) {
        //HACK: KoMainWindow shows setup on print, but not on print preview!
        if (!printer.setup()) {
            return;
        }
    }
	if (m_tab->visibleWidget() == m_ganttview)
	{
        m_ganttview->print(printer);
    }
	else if (m_tab->visibleWidget() == m_pertview)
	{
        m_pertview->print(printer);
	}
	else if (m_tab->visibleWidget() == m_resourceview)
	{
        m_resourceview->print(printer);
	}
	else if (m_tab->visibleWidget() == m_accountsview)
    {
        m_accountsview->print(printer);
    }
// 	else if (m_tab->visibleWidget() == m_reportview)
// 	{
//         m_reportview->print(printer);
// 	}

}

void View::slotEditCut() {
    //kdDebug()<<k_funcinfo<<endl;
}

void View::slotEditCopy() {
    //kdDebug()<<k_funcinfo<<endl;
}

void View::slotEditPaste() {
    //kdDebug()<<k_funcinfo<<endl;
}

void View::slotViewExpected() {
    //kdDebug()<<k_funcinfo<<endl;
    m_currentEstimateType = Effort::Use_Expected;
    getProject().setCurrentSchedulePtr(getProject().findSchedule(Schedule::Expected));
    slotUpdate(false);
}

void View::slotViewOptimistic() {
    //kdDebug()<<k_funcinfo<<endl;
    m_currentEstimateType = Effort::Use_Optimistic;
    getProject().setCurrentSchedulePtr(getProject().findSchedule(Schedule::Optimistic));
    slotUpdate(false);
}

void View::slotViewPessimistic() {
    //kdDebug()<<k_funcinfo<<endl;
    m_currentEstimateType = Effort::Use_Pessimistic;
    getProject().setCurrentSchedulePtr(getProject().findSchedule(Schedule::Pessimistic));
    slotUpdate(false);
}

void View::slotViewGanttResources() {
    //kdDebug()<<k_funcinfo<<endl;
    m_ganttview->setShowResources(actionViewGanttResources->isChecked());
    if (m_tab->visibleWidget() == m_ganttview)
        slotUpdate(false);
}

void View::slotViewGanttTaskName() {
    //kdDebug()<<k_funcinfo<<endl;
    m_ganttview->setShowTaskName(actionViewGanttTaskName->isChecked());
    if (m_tab->visibleWidget() == m_ganttview)
        slotUpdate(false);
}

void View::slotViewGanttTaskLinks() {
    //kdDebug()<<k_funcinfo<<endl;
    m_ganttview->setShowTaskLinks(actionViewGanttTaskLinks->isChecked());
    if (m_tab->visibleWidget() == m_ganttview)
        slotUpdate(false);
}

void View::slotViewGanttProgress() {
    //kdDebug()<<k_funcinfo<<endl;
    m_ganttview->setShowProgress(actionViewGanttProgress->isChecked());
    if (m_tab->visibleWidget() == m_ganttview)
        slotUpdate(false);
}

void View::slotViewGanttFloat() {
    //kdDebug()<<k_funcinfo<<endl;
    m_ganttview->setShowPositiveFloat(actionViewGanttFloat->isChecked());
    if (m_tab->visibleWidget() == m_ganttview)
        slotUpdate(false);
}

void View::slotViewGanttCriticalTasks() {
    //kdDebug()<<k_funcinfo<<endl;
    m_ganttview->setShowCriticalTasks(actionViewGanttCriticalTasks->isChecked());
    if (m_tab->visibleWidget() == m_ganttview)
        slotUpdate(false);
}

void View::slotViewGanttCriticalPath() {
    //kdDebug()<<k_funcinfo<<endl;
    m_ganttview->setShowCriticalPath(actionViewGanttCriticalPath->isChecked());
    if (m_tab->visibleWidget() == m_ganttview)
        slotUpdate(false);
}

void View::slotViewGanttNoInformation() {
    kdDebug()<<k_funcinfo<<m_ganttview->showNoInformation()<<endl;
    m_ganttview->setShowNoInformation(!m_ganttview->showNoInformation()); //Toggle
    if (m_tab->visibleWidget() == m_ganttview)
        slotUpdate(false);
}

void View::slotViewTaskAppointments() {
    //kdDebug()<<k_funcinfo<<endl;
    m_ganttview->setShowAppointments(actionViewTaskAppointments->isChecked());
    m_updateGanttview = true;
    if (m_tab->visibleWidget() == m_ganttview)
        slotUpdate(false);
}

void View::slotViewGantt() {
    //kdDebug()<<k_funcinfo<<endl;
    m_tab->raiseWidget(m_ganttview);
}

void View::slotViewPert() {
    //kdDebug()<<k_funcinfo<<endl;
    m_tab->raiseWidget(m_pertview);
}

void View::slotViewResources() {
    //kdDebug()<<k_funcinfo<<endl;
    m_tab->raiseWidget(m_resourceview);
}

void View::slotViewResourceAppointments() {
    //kdDebug()<<k_funcinfo<<endl;
    m_resourceview->setShowAppointments(actionViewResourceAppointments->isChecked());
    m_updateResourceview = true;
    if (m_tab->visibleWidget() == m_resourceview)
        slotUpdate(false);
}

void View::slotViewAccounts() {
    //kdDebug()<<k_funcinfo<<endl;
    m_tab->raiseWidget(m_accountsview);
}

void View::slotProjectEdit() {
    MainProjectDialog *dia = new MainProjectDialog(getProject());
    if (dia->exec()) {
        KCommand *cmd = dia->buildCommand(getPart());
        if (cmd) {
            getPart()->addCommand(cmd);
        }
    }
    delete dia;
}

void View::slotProjectCalendar() {
    CalendarListDialog *dia = new CalendarListDialog(getProject());
    if (dia->exec()) {
        KCommand *cmd = dia->buildCommand(getPart());
        if (cmd) {
            //kdDebug()<<k_funcinfo<<"Modifying calendar(s)"<<endl;
            getPart()->addCommand(cmd); //also executes
        }
    }
    delete dia;
}

void View::slotProjectAccounts() {
    AccountsDialog *dia = new AccountsDialog(getProject().accounts());
    if (dia->exec()) {
        KCommand *cmd = dia->buildCommand(getPart());
        if (cmd) {
            //kdDebug()<<k_funcinfo<<"Modifying account(s)"<<endl;
            getPart()->addCommand(cmd); //also executes
        }
    }
    delete dia;
}

void View::slotProjectWorktime() {
    StandardWorktimeDialog *dia = new StandardWorktimeDialog(getProject());
    if (dia->exec()) {
        KCommand *cmd = dia->buildCommand(getPart());
        if (cmd) {
            //kdDebug()<<k_funcinfo<<"Modifying calendar(s)"<<endl;
            getPart()->addCommand(cmd); //also executes
        }
    }
    delete dia;
}

void View::slotProjectResources() {
    ResourcesDialog *dia = new ResourcesDialog(getProject());
    if (dia->exec()) {
        KCommand *cmd = dia->buildCommand(getPart());
        if (cmd) {
            //kdDebug()<<k_funcinfo<<"Modifying resources"<<endl;
            getPart()->addCommand(cmd); //also executes
        }
    }
    delete dia;
}

void View::slotProjectCalculate() {
    //kdDebug()<<k_funcinfo<<endl;
    slotUpdate(true);
}

void View::slotProjectCalculateExpected() {
    m_currentEstimateType = Effort::Use_Expected;
    m_updateGanttview = true;
    m_updateResourceview = true;
    m_updateAccountsview = true;
    slotUpdate(true);
}

void View::slotProjectCalculateOptimistic() {
    m_currentEstimateType = Effort::Use_Optimistic;
    m_updateGanttview = true;
    m_updateResourceview = true;
    m_updateAccountsview = true;
    slotUpdate(true);
}

void View::slotProjectCalculatePessimistic() {
    m_currentEstimateType = Effort::Use_Pessimistic;
    m_updateGanttview = true;
    m_updateResourceview = true;
    m_updateAccountsview = true;
    slotUpdate(true);
}

void View::projectCalculate() {
    if (false /*getProject().actualEffort() > 0*/) {
        // NOTE: This can be removed when proper baselining etc is implemented
        if (KMessageBox::warningContinueCancel(this, i18n("Progress information will be deleted if the project is recalculated."), i18n("Calculate"), i18n("Calculate")) == KMessageBox::Cancel) {
            return;
        }
    }
    QApplication::setOverrideCursor(Qt::waitCursor);
    Schedule *ns = getProject().findSchedule((Schedule::Type)m_currentEstimateType);
    KCommand *cmd;
    if (ns) {
        cmd = new RecalculateProjectCmd(getPart(), getProject(), *ns, i18n("Calculate"));
    } else  {
        cmd = new CalculateProjectCmd(getPart(), getProject(), i18n("Standard"), (Effort::Use)m_currentEstimateType, i18n("Calculate"));
    }
    getPart()->addCommand(cmd);
    QApplication::restoreOverrideCursor();
}

void View::slotViewReportDesign() {
    //kdDebug()<<k_funcinfo<<endl;
}

void View::slotViewReports() {
    //kdDebug()<<k_funcinfo<<endl;
    //m_tab->raiseWidget(m_reportview);
}

void View::slotAddSubTask() {
	// If we are positionend on the root project, then what we really want to
	// do is to add a first project. We will silently accept the challenge
	// and will not complain.
    Task* node = getProject().createTask(getPart()->config().taskDefaults(), currentTask());
    TaskDialog *dia = new TaskDialog(*node, getProject().accounts(), getProject().standardWorktime(), getProject().isBaselined());
    if (dia->exec()) {
		Node *currNode = currentTask();
		if (currNode)
        {
            KCommand *m = dia->buildCommand(getPart());
            m->execute(); // do changes to task
            delete m;
            SubtaskAddCmd *cmd = new SubtaskAddCmd(getPart(), &(getProject()), node, currNode, i18n("Add Subtask"));
            getPart()->addCommand(cmd); // add task to project
			return;
	    }
		else
		    kdDebug()<<k_funcinfo<<"Cannot insert new project. Hmm, no current node!?"<<endl;
	}
    delete node;
    delete dia;
}


void View::slotAddTask() {
    Task* node = getProject().createTask(getPart()->config().taskDefaults(), currentTask());
    TaskDialog *dia = new TaskDialog(*node, getProject().accounts(), getProject().standardWorktime(), getProject().isBaselined());
    if (dia->exec()) {
		Node* currNode = currentTask();
		if (currNode)
        {
            KCommand *m = dia->buildCommand(getPart());
            m->execute(); // do changes to task
            delete m;
            TaskAddCmd *cmd = new TaskAddCmd(getPart(), &(getProject()), node, currNode, i18n("Add Task"));
            getPart()->addCommand(cmd); // add task to project
			return;
	    }
		else
		    kdDebug()<<k_funcinfo<<"Cannot insert new task. Hmm, no current node!?"<<endl;
	}
    delete node;
    delete dia;
}

void View::slotAddMilestone() {
    Task* node = getProject().createTask(currentTask());
    node->effort()->set(Duration::zeroDuration);

    TaskDialog *dia = new TaskDialog(*node, getProject().accounts(), getProject().standardWorktime(), getProject().isBaselined());
    if (dia->exec()) {
		Node *currNode = currentTask();
		if (currNode)
        {
            KCommand *m = dia->buildCommand(getPart());
            m->execute(); // do changes to task
            delete m;
            TaskAddCmd *cmd = new TaskAddCmd(getPart(), &(getProject()), node, currNode, i18n("Add Milestone"));
            getPart()->addCommand(cmd); // add task to project
			return;
	    }
		else
		    kdDebug()<<k_funcinfo<<"Cannot insert new milestone. Hmm, no current node!?"<<endl;
	}
    delete node;
    delete dia;
}

void View::slotDefineWBS() {
    //kdDebug()<<k_funcinfo<<endl;
    WBSDefinitionDialog *dia = new WBSDefinitionDialog(getPart()->wbsDefinition());
    dia->exec();
    
    delete dia;
}

void View::slotGenerateWBS() {
    //kdDebug()<<k_funcinfo<<endl;
    getPart()->generateWBS();
    slotUpdate(false);
}

void View::slotConfigure() {
    //kdDebug()<<k_funcinfo<<endl;
    ConfigDialog *dia = new ConfigDialog(getPart()->config(), getProject());
    dia->exec();
    delete dia;
}

Node *View::currentTask()
{
	Node* task = 0;
	if (m_tab->visibleWidget() == m_ganttview) {
	    task = m_ganttview->currentNode();
	}
	else if (m_tab->visibleWidget() == m_pertview) {
		task = m_pertview->currentNode();
	}
	else if (m_tab->visibleWidget() == m_resourceview) {
		task = m_resourceview->currentNode();
	}
	if ( 0 != task ) {
		return task;
	}
	return &(getProject());
}

void View::slotOpenNode() {
    //kdDebug()<<k_funcinfo<<endl;
    Node *node = currentTask();
    if (!node)
        return;

    switch (node->type()) {
        case Node::Type_Project: {
            Project *project = dynamic_cast<Project *>(node);
            MainProjectDialog *dia = new MainProjectDialog(*project);
            if (dia->exec()){
                KCommand *m = dia->buildCommand(getPart());
                if (m) {
                    getPart()->addCommand(m);
                }
            }
            delete dia;
            break;
        }
        case Node::Type_Subproject:
            //TODO
            break;
        case Node::Type_Task: {
            Task *task = dynamic_cast<Task *>(node);
            TaskDialog *dia = new TaskDialog(*task, getProject().accounts(), getProject().standardWorktime(), getProject().isBaselined());
            if (dia->exec()) {
                KCommand *m = dia->buildCommand(getPart());
                if (m) {
                    getPart()->addCommand(m);
                }
            }
            delete dia;
            break;
        }
        case Node::Type_Milestone: {
            // Use the normal task dialog for now.
            // Maybe milestone should have it's own dialog, but we need to be able to
            // enter a duration in case we accidentally set a tasks duration to zero
            // and hence, create a milestone
            Task *task = dynamic_cast<Task *>(node);
            TaskDialog *dia = new TaskDialog(*task, getProject().accounts(), getProject().standardWorktime(), getProject().isBaselined());
            if (dia->exec()) {
                KCommand *m = dia->buildCommand(getPart());
                if (m) {
                    getPart()->addCommand(m);
                }
            }
            delete dia;
            break;
        }
        case Node::Type_Summarytask: {
            Task *task = dynamic_cast<Task *>(node);
            SummaryTaskDialog *dia = new SummaryTaskDialog(*task);
            if (dia->exec()) {
                KCommand *m = dia->buildCommand(getPart());
                if (m) {
                    getPart()->addCommand(m);
                }
            }
            delete dia;
            break;
        }
        default:
            break; // avoid warnings
    }
}

void View::slotTaskProgress() {
    //kdDebug()<<k_funcinfo<<endl;
    Node *node = currentTask();
    if (!node)
        return;

    switch (node->type()) {
        case Node::Type_Project: {
            break;
        }
        case Node::Type_Subproject:
            //TODO
            break;
        case Node::Type_Task: {
            Task *task = dynamic_cast<Task *>(node);
            TaskProgressDialog *dia = new TaskProgressDialog(*task, getProject().standardWorktime());
            if (dia->exec()) {
                KCommand *m = dia->buildCommand(getPart());
                if (m) {
                    getPart()->addCommand(m);
                }
            }
            delete dia;
            break;
        }
        case Node::Type_Milestone: {
            Task *task = dynamic_cast<Task *>(node);
            MilestoneProgressDialog *dia = new MilestoneProgressDialog(*task);
            if (dia->exec()) {
                KCommand *m = dia->buildCommand(getPart());
                if (m) {
                    getPart()->addCommand(m);
                }
            }
            delete dia;
            break;
        }
        case Node::Type_Summarytask: {
            // TODO
            break;
        }
        default:
            break; // avoid warnings
    }
}

void View::slotDeleteTask()
{
    //kdDebug()<<k_funcinfo<<endl;
    Node *node = currentTask();
    if (node == 0 || node->getParent() == 0) {
        kdDebug()<<k_funcinfo<<(node ? "Task is main project" : "No current task")<<endl;
        return;
    }
    KMacroCommand *cmd = new KMacroCommand(i18n("Delete Task"));
    cmd->addCommand(new NodeDeleteCmd(getPart(), node));
    QPtrListIterator<Relation> it = node->dependChildNodes();
    for (; it.current(); ++it) {
        cmd->addCommand(new DeleteRelationCmd(getPart(), it.current()));
    }
    it = node->dependParentNodes();
    for (; it.current(); ++it) {
        cmd->addCommand(new DeleteRelationCmd(getPart(),it.current()));
    }
    getPart()->addCommand(cmd);
}

void View::slotIndentTask()
{
    //kdDebug()<<k_funcinfo<<endl;
    Node *node = currentTask();
    if (node == 0 || node->getParent() == 0) {
        kdDebug()<<k_funcinfo<<(node ? "Task is main project" : "No current task")<<endl;
        return;
    }
    if (getProject().canIndentTask(node)) {
        NodeIndentCmd *cmd = new NodeIndentCmd(getPart(), *node, i18n("Indent Task"));
        getPart()->addCommand(cmd);
    }
}

void View::slotUnindentTask()
{
    //kdDebug()<<k_funcinfo<<endl;
    Node *node = currentTask();
    if (node == 0 || node->getParent() == 0) {
        kdDebug()<<k_funcinfo<<(node ? "Task is main project" : "No current task")<<endl;
        return;
    }
    if (getProject().canUnindentTask(node)) {
        NodeUnindentCmd *cmd = new NodeUnindentCmd(getPart(), *node, i18n("Unindent Task"));
        getPart()->addCommand(cmd);
    }
}

void View::slotMoveTaskUp()
{
    //kdDebug()<<k_funcinfo<<endl;

	Node* task = currentTask();
	if ( 0 == task ) {
		// is always != 0. At least we would get the Project, but you never know who might change that
		// so better be careful
		kdError()<<k_funcinfo<<"No current task"<<endl;
		return;
	}

	if ( Node::Type_Project == task->type() ) {
		kdDebug()<<k_funcinfo<<"The root node cannot be moved up"<<endl;
		return;
	}
    if (getProject().canMoveTaskUp(task)) {
        NodeMoveUpCmd *cmd = new NodeMoveUpCmd(getPart(), *task, i18n("Move Task Up"));
        getPart()->addCommand(cmd);
    }
}

void View::slotMoveTaskDown()
{
    //kdDebug()<<k_funcinfo<<endl;

	Node* task = currentTask();
	if ( 0 == task ) {
		// is always != 0. At least we would get the Project, but you never know who might change that
		// so better be careful
		return;
	}

	if ( Node::Type_Project == task->type() ) {
		kdDebug()<<k_funcinfo<<"The root node cannot be moved down"<<endl;
		return;
	}
    if (getProject().canMoveTaskDown(task)) {
        NodeMoveDownCmd *cmd = new NodeMoveDownCmd(getPart(), *task, i18n("Move Task Down"));
        getPart()->addCommand(cmd);
    }
}

void View::slotAddRelation(Node *par, Node *child) {
    //kdDebug()<<k_funcinfo<<endl;
    Relation *rel = new Relation(par, child);
    AddRelationDialog *dia = new AddRelationDialog(rel, this);
    if (dia->exec()) {
        KCommand *cmd = dia->buildCommand(getPart());
        if (cmd)
            getPart()->addCommand(cmd);
    } else {
        delete rel;
    }
    delete dia;
}

void View::slotAddRelation(Node *par, Node *child, int linkType) {
    //kdDebug()<<k_funcinfo<<endl;
    if (linkType == Relation::FinishStart ||
        linkType == Relation::StartStart ||
        linkType == Relation::FinishFinish) 
    {
        Relation *rel = new Relation(par, child,  static_cast<Relation::Type>(linkType));
        getPart()->addCommand(new AddRelationCmd(getPart(), rel, i18n("Add Relation")));
    } else {
        slotAddRelation(par, child);
    }
}

void View::slotModifyRelation(Relation *rel) {
    //kdDebug()<<k_funcinfo<<endl;
    ModifyRelationDialog *dia = new ModifyRelationDialog(rel, this);
    if (dia->exec()) {
        if (dia->relationIsDeleted()) {
            getPart()->addCommand(new DeleteRelationCmd(getPart(), rel, i18n("Delete Relation")));
        } else {
            KCommand *cmd = dia->buildCommand(getPart());
            if (cmd) {
                getPart()->addCommand(cmd);
            }
        }
    }
    delete dia;
}

void View::slotModifyRelation(Relation *rel, int linkType) {
    //kdDebug()<<k_funcinfo<<endl;
    if (linkType == Relation::FinishStart ||
        linkType == Relation::StartStart ||
        linkType == Relation::FinishFinish) 
    {
        getPart()->addCommand(new ModifyRelationTypeCmd(getPart(), rel, static_cast<Relation::Type>(linkType)));
    } else {
        slotModifyRelation(rel);
    }
}

// testing
void View::slotExportGantt() {
    //kdDebug()<<k_funcinfo<<endl;
    if (!m_ganttview) {
        return;
    }
    QString fn = KFileDialog::getSaveFileName( QString::null, QString::null, this );
    if (!fn.isEmpty()) {
        QFile f(fn);
        m_ganttview->exportGantt(&f);
    }
}

void View::slotEditResource() {
    //kdDebug()<<k_funcinfo<<endl;
    Resource *r = m_resourceview->currentResource();
    if (!r)
        return;
    ResourceDialog *dia = new ResourceDialog(getProject(), r);
    if (dia->exec()) {
        KCommand *cmd = dia->buildCommand(getPart());
        if (cmd)
            getPart()->addCommand(cmd);
    }
    delete dia;
}

void View::updateReadWrite(bool /*readwrite*/) {
}

Part *View::getPart()const {
    return (Part *)koDocument();
}

void View::slotConnectNode() {
    //kdDebug()<<k_funcinfo<<endl;
/*    NodeItem *curr = m_ganttview->currentItem();
    if (curr) {
        kdDebug()<<k_funcinfo<<"node="<<curr->getNode().name()<<endl;
    }*/
}

QPopupMenu * View::popupMenu( const QString& name )
{
    //kdDebug()<<k_funcinfo<<endl;
    Q_ASSERT(factory());
    if ( factory() )
        return ((QPopupMenu*)factory()->container( name, this ));
    return 0L;
}

void View::slotChanged(QWidget *)
{
    //kdDebug()<<k_funcinfo<<endl;
    slotUpdate(false);
}

void View::slotChanged()
{
    //kdDebug()<<k_funcinfo<<endl;
    slotUpdate(false);
}

void View::slotUpdate(bool calculate)
{
    //kdDebug()<<k_funcinfo<<"calculate="<<calculate<<endl;
    if (calculate)
        projectCalculate();
        
    m_updateGanttview = true;
    m_updateResourceview = true;
    m_updateAccountsview = true;
    
    updateView(m_tab->visibleWidget());
}

void View::slotAboutToShow(QWidget *widget) {
    updateView(widget);
}

void View::updateView(QWidget *widget)
{
    QApplication::setOverrideCursor(Qt::waitCursor);
    setScheduleActionsEnabled();
    setTaskActionsEnabled(false);
    mainWindow()->toolBar("report")->hide();
    if (widget == m_ganttview)
    {
        //kdDebug()<<k_funcinfo<<"draw gantt"<<endl;
        m_ganttview->setShowExpected(actionViewExpected->isChecked());
        m_ganttview->setShowOptimistic(actionViewOptimistic->isChecked());
        m_ganttview->setShowPessimistic(actionViewPessimistic->isChecked());
        if (m_updateGanttview)
            m_ganttview->drawChanges(getProject());
        setTaskActionsEnabled(widget, true);
        m_updateGanttview = false;
    }
    else if (widget == m_pertview)
    {
        //kdDebug()<<k_funcinfo<<"draw pertview"<<endl;
        m_pertview->draw();
    }
    else if (widget == m_resourceview)
    {
        //kdDebug()<<k_funcinfo<<"draw resourceview"<<endl;
        if (m_updateResourceview)
            m_resourceview->draw(getPart()->getProject());
        m_updateResourceview = false;
    }
    else if (widget == m_accountsview)
    {
        //kdDebug()<<k_funcinfo<<"draw accountsview"<<endl;
        if (m_updateAccountsview)
            m_accountsview->draw();
        m_updateAccountsview = false;
    }
/*    else if (widget == m_reportview)
    {
        mainWindow()->toolBar("report")->show();
        m_reportview->enableNavigationBtn();
    }*/
    QApplication::restoreOverrideCursor();
}

void View::slotRenameNode(Node *node, const QString& name) {
    //kdDebug()<<k_funcinfo<<name<<endl;
    if (node) {
        NodeModifyNameCmd *cmd = new NodeModifyNameCmd(getPart(), *node, name, i18n("Modify Name"));
        getPart()->addCommand(cmd);
    }
}

void View::slotPopupMenu(const QString& menuname, const QPoint & pos)
{
    QPopupMenu* menu = this->popupMenu(menuname);
    if (menu)
      menu->exec(pos);
}

bool View::setContext(Context &context) {
    //kdDebug()<<k_funcinfo<<endl;
    m_currentEstimateType = context.currentEstimateType;
    getProject().setCurrentSchedule(context.currentSchedule);
    actionViewExpected->setChecked(context.actionViewExpected);
    actionViewOptimistic->setChecked(context.actionViewOptimistic);
    actionViewPessimistic->setChecked(context.actionViewPessimistic);
    
    m_ganttview->setContext(context.ganttview, getProject());
    // hmmm, can't decide if these should be here or actions moved to ganttview
    actionViewGanttResources->setChecked(context.ganttview.showResources);
    actionViewGanttTaskName->setChecked(context.ganttview.showTaskName);
    actionViewGanttTaskLinks->setChecked(context.ganttview.showTaskLinks);
    actionViewGanttProgress->setChecked(context.ganttview.showProgress);
    actionViewGanttFloat->setChecked(context.ganttview.showPositiveFloat);
    actionViewGanttCriticalTasks->setChecked(context.ganttview.showCriticalTasks);
    actionViewGanttCriticalPath->setChecked(context.ganttview.showCriticalPath);

    m_pertview->setContext(context.pertview);
    m_resourceview->setContext(context.resourceview);
    m_accountsview->setContext(context.accountsview);
//    m_reportview->setContext(context.reportview);
    
    if (context.currentView == "ganttview") {
        m_ganttview->setShowExpected(actionViewExpected->isChecked());
        m_ganttview->setShowOptimistic(actionViewOptimistic->isChecked());
        m_ganttview->setShowPessimistic(actionViewPessimistic->isChecked());
        slotViewGantt();
    } else if (context.currentView == "pertview") {
        slotViewPert();
    } else if (context.currentView == "resourceview") {
        slotViewResources();
    } else if (context.currentView == "accountsview") {
        slotViewAccounts();
    } else if (context.currentView == "reportview") {
        //slotViewReport();
    } else {
        slotViewGantt();
    }
    slotUpdate(false);
    return true;
}

void View::getContext(Context &context) const {
    //kdDebug()<<k_funcinfo<<endl;
    context.currentEstimateType = m_currentEstimateType;
    if (getProject().currentSchedule())
        context.currentSchedule = getProject().currentSchedule()->id();
    context.actionViewExpected = actionViewExpected->isChecked();
    context.actionViewOptimistic = actionViewOptimistic->isChecked();
    context.actionViewPessimistic = actionViewPessimistic->isChecked();
    
    if (m_tab->visibleWidget() == m_ganttview) {
        context.currentView = "ganttview";
    } else if (m_tab->visibleWidget() == m_pertview) {
        context.currentView = "pertview";
    } else if (m_tab->visibleWidget() == m_resourceview) {
        context.currentView = "resourceview";
    } else if (m_tab->visibleWidget() == m_accountsview) {
        context.currentView = "accountsview";
/*    } else if (m_tab->visibleWidget() == m_reportview) {
        context.currentView = "reportview";*/
    }
    m_ganttview->getContext(context.ganttview);
    m_pertview->getContext(context.pertview);
    m_resourceview->getContext(context.resourceview);
    m_accountsview->getContext(context.accountsview);
//    m_reportview->getContext(context.reportview);
}

void View::setBaselineMode(bool /*on*/) {
    //kdDebug()<<k_funcinfo<<endl;
/*    m_baselineMode = on;
    
    m_ganttview->setReadWriteMode(!on);
    
    actionCut->setEnabled(!on);
    actionCopy->setEnabled(!on);
    actionPaste->setEnabled(!on);

    actionDeleteTask->setEnabled(!on);
    actionIndentTask->setEnabled(!on);
    actionUnindentTask->setEnabled(!on);
    actionMoveTaskUp->setEnabled(!on);
    actionMoveTaskDown->setEnabled(!on);

    actionAddTask->setEnabled(!on);
    actionAddSubtask->setEnabled(!on);
    actionAddMilestone->setEnabled(!on);

    actionEditStandardWorktime->setEnabled(!on);
    actionEditCalendar->setEnabled(!on);
    actionEditResources->setEnabled(!on);
    actionCalculate->setEnabled(!on);

    actionEditResource->setEnabled(!on);*/
}

// called when widget w is about to be shown
void View::setTaskActionsEnabled(QWidget *w, bool on) {
    Node *n = 0;
    if (w == m_ganttview) {
        n = m_ganttview->currentNode();
    }// else pert, etc when implemented
     
    actionAddTask->setEnabled(on);
    actionAddMilestone->setEnabled(on);
    // only enabled when we have a task selected
    bool o = (on && n);
    actionAddSubtask->setEnabled(o);
    actionDeleteTask->setEnabled(o);
    actionMoveTaskUp->setEnabled(o && getProject().canMoveTaskUp(n));
    actionMoveTaskDown->setEnabled(o && getProject().canMoveTaskDown(n));
    actionIndentTask->setEnabled(o && getProject().canIndentTask(n));
    actionUnindentTask->setEnabled(o && getProject().canUnindentTask(n));
}

void View::setTaskActionsEnabled(bool on) {
    setTaskActionsEnabled(m_ganttview, on); //FIXME if more than ganttview can do this
}

void View::setScheduleActionsEnabled() {
    actionViewExpected->setEnabled(getProject().findSchedule(Schedule::Expected));
    actionViewOptimistic->setEnabled(getProject().findSchedule(Schedule::Optimistic));
    actionViewPessimistic->setEnabled(getProject().findSchedule(Schedule::Pessimistic));
    if (getProject().notScheduled()) {
        m_estlabel->setText(i18n("Not scheduled"));
        return;
    }
    Schedule *ns = getProject().currentSchedule();
    if (ns->type() == Schedule::Expected) {
        actionViewExpected->setChecked(true);
        m_estlabel->setText(i18n("Expected"));
    } else if (ns->type() == Schedule::Optimistic) {
        actionViewOptimistic->setChecked(true);
        m_estlabel->setText(i18n("Optimistic"));
    } else if (ns->type() == Schedule::Pessimistic) {
        actionViewPessimistic->setChecked(true);
        m_estlabel->setText(i18n("Pessimistic"));
    }
}


#ifndef NDEBUG
void View::slotPrintDebug() {
    kdDebug()<<"-------- Debug printout: Node list" <<endl;
/*    Node *curr = m_ganttview->currentNode();
    if (curr) {
        curr->printDebug(true,"");
    } else*/
        getPart()->getProject().printDebug(true, "");
}
void View::slotPrintSelectedDebug() {
    Node *curr = m_ganttview->currentNode();
    if (curr) {
        kdDebug()<<"-------- Debug printout: Selected node" <<endl;
        curr->printDebug(true,"");
    } else
        slotPrintDebug();
}
void View::slotPrintCalendarDebug() {
    kdDebug()<<"-------- Debug printout: Node list" <<endl;
/*    Node *curr = m_ganttview->currentNode();
    if (curr) {
        curr->printDebug(true,"");
    } else*/
        getPart()->getProject().printCalendarDebug("");
}
void View::slotPrintTestDebug() {
    const QStringList &lst = getPart()->xmlLoader().log();
    
    for ( QStringList::ConstIterator it = lst.constBegin(); it != lst.constEnd(); ++it ) {
        kdDebug()<<*it<<endl;
    }
//     kdDebug()<<"------------Test 1---------------------"<<endl;
//     {
//     DateTime d1(QDate(2006,1,2), QTime(8,0,0));
//     DateTime d2 = d1.addSecs(3600);
//     Duration d = d2 - d1;
//     bool b = d==Duration(0,0,0,3600);
//     kdDebug()<<"1: Success="<<b<<"    "<<d2.toString()<<"-"<<d1.toString()<<"="<<d.toString()<<endl;
//     d = d1 - d2;
//     b = d==Duration(0,0,0,3600);
//     kdDebug()<<"2: Success="<<b<<"    "<<d1.toString()<<"-"<<d2.toString()<<"="<<d.toString()<<endl;
//     d2 = d2.addDays(-2);
//     d = d1 - d2;
//     b = d==Duration(2,0,0)-Duration(0,0,0,3600);
//     kdDebug()<<"3: Success="<<b<<"    "<<d1.toString()<<"-"<<d2.toString()<<"="<<d.toString()<<endl;
//     d = d2 - d1;
//     b = d==Duration(2,0,0)-Duration(0,0,0,3600);
//     kdDebug()<<"4: Success="<<b<<"     "<<d2.toString()<<"-"<<d1.toString()<<"="<<d.toString()<<endl;
//     kdDebug()<<endl;
//     b = (d2 + d)==d1;
//     kdDebug()<<"5: Success="<<b<<"   "<<d2<<"+"<<d.toString()<<"="<<d1<<endl;
//     b = (d1 - d)==d2;
//     kdDebug()<<"6: Success="<<b<<"   "<<d1<<"-"<<d.toString()<<"="<<d2<<endl;
//     } // end test 1
//     kdDebug()<<endl;
//     kdDebug()<<"------------Test 2 Single calendar-----------------"<<endl;
//     {
//     Calendar *t = new Calendar("Test 2");
//     QDate wdate(2006,1,2);
//     DateTime before = DateTime(wdate.addDays(-1));
//     DateTime after = DateTime(wdate.addDays(1));
//     QTime t1(8,0,0);
//     QTime t2(10,0,0);
//     DateTime wdt1(wdate, t1);
//     DateTime wdt2(wdate, t2);
//     CalendarDay *day = new CalendarDay(QDate(2006,1,2), Map::Working);
//     day->addInterval(QPair<QTime, QTime>(t1, t2));
//     if (!t->addDay(day)) {
//         kdDebug()<<"Failed to add day"<<endl;
//         delete day;
//         delete t;
//         return;
//     }
//     kdDebug()<<"Added     date="<<day->date().toString()<<" "<<day->startOfDay().toString()<<" - "<<day->endOfDay()<<endl;
//     kdDebug()<<"Found     date="<<day->date().toString()<<" "<<day->startOfDay().toString()<<" - "<<day->endOfDay()<<endl;
//     
//     CalendarDay *d = t->findDay(wdate);
//     bool b = (day == d);
//     kdDebug()<<"1: Success="<<b<<"      Find same day"<<endl;
//     
//     DateTime dt = t->firstAvailableAfter(after, after.addDays(10));
//     b = !dt.isValid();
//     kdDebug()<<"2: Success="<<b<<"      firstAvailableAfter("<<after<<"): ="<<dt<<endl;
//     
//     dt = t->firstAvailableBefore(before, before.addDays(-10));
//     b = !dt.isValid();
//     kdDebug()<<"3: Success="<<b<<"       firstAvailableBefore("<<before.toString()<<"): ="<<dt<<endl;
//     
//     dt = t->firstAvailableAfter(before, after);
//     b = dt == wdt1;
//     kdDebug()<<"4: Success="<<b<<"      firstAvailableAfter("<<before<<"): ="<<dt<<endl;
//     
//     dt = t->firstAvailableBefore(after, before);
//     b = dt == wdt2;
//     kdDebug()<<"5: Success="<<b<<"      firstAvailableBefore("<<after<<"): ="<<dt<<endl;
//     
//     b = t->hasInterval(before, after);
//     kdDebug()<<"6: Success="<<b<<"      hasInterval("<<before<<", "<<after<<")"<<endl;
//     
//     b = !t->hasInterval(after, after.addDays(1));
//     kdDebug()<<"7: Success="<<b<<"      !hasInterval("<<after<<", "<<after.addDays(1)<<")"<<endl;
//     
//     b = !t->hasInterval(before, before.addDays(-1));
//     kdDebug()<<"8: Success="<<b<<"      !hasInterval("<<before<<", "<<before.addDays(-1)<<")"<<endl;
//     
//     Duration e1(0, 2, 0); // 2 hours
//     Duration e2 = t->effort(before, after);
//     b = e1==e2;
//     kdDebug()<<"9: Success="<<b<<"      effort"<<e1.toString()<<" = "<<e2.toString()<<endl;
//     
//     delete t;
//     }// end test 2
//     
//     kdDebug()<<endl;
//     kdDebug()<<"------------Test 3 Parent calendar-----------------"<<endl;
//     {
//     Calendar *t = new Calendar("Test 3");
//     Calendar *p = new Calendar("Test 3 parent");
//     t->setParent(p);
//     QDate wdate(2006,1,2);
//     DateTime before = DateTime(wdate.addDays(-1));
//     DateTime after = DateTime(wdate.addDays(1));
//     QTime t1(8,0,0);
//     QTime t2(10,0,0);
//     DateTime wdt1(wdate, t1);
//     DateTime wdt2(wdate, t2);
//     CalendarDay *day = new CalendarDay(QDate(2006,1,2), Map::Working);
//     day->addInterval(QPair<QTime, QTime>(t1, t2));
//     if (!p->addDay(day)) {
//         kdDebug()<<"Failed to add day"<<endl;
//         delete day;
//         delete t;
//         return;
//     }
//     kdDebug()<<"Added     date="<<day->date().toString()<<" "<<day->startOfDay().toString()<<" - "<<day->endOfDay().toString()<<endl;
//     kdDebug()<<"Found     date="<<day->date().toString()<<" "<<day->startOfDay().toString()<<" - "<<day->endOfDay().toString()<<endl;
//     
//     CalendarDay *d = p->findDay(wdate);
//     bool b = (day == d);
//     kdDebug()<<"1: Success="<<b<<"      Find same day"<<endl;
//     
//     DateTime dt = t->firstAvailableAfter(after, after.addDays(10));
//     b = !dt.isValid();
//     kdDebug()<<"2: Success="<<b<<"      firstAvailableAfter("<<after.toString()<<"): ="<<!b<<endl;
//     
//     dt = t->firstAvailableBefore(before, before.addDays(-10));
//     b = !dt.isValid();
//     kdDebug()<<"3: Success="<<b<<"       firstAvailableBefore("<<before.toString()<<"): ="<<!b<<endl;
//     
//     dt = t->firstAvailableAfter(before, after);
//     b = dt == wdt1;
//     kdDebug()<<"4: Success="<<b<<"      firstAvailableAfter("<<before.toString()<<"): ="<<dt.toString()<<endl;
//     
//     dt = t->firstAvailableBefore(after, before);
//     b = dt == wdt2;
//     kdDebug()<<"5: Success="<<b<<"      firstAvailableBefore("<<after.toString()<<"): ="<<dt.toString()<<endl;
//     
//     b = t->hasInterval(before, after);
//     kdDebug()<<"6: Success="<<b<<"      hasInterval("<<before.toString()<<", "<<after<<")"<<endl;
//     
//     b = !t->hasInterval(after, after.addDays(1));
//     kdDebug()<<"7: Success="<<b<<"      !hasInterval("<<after.toString()<<", "<<after.addDays(1)<<")"<<endl;
//     
//     b = !t->hasInterval(before, before.addDays(-1));
//     kdDebug()<<"8: Success="<<b<<"      !hasInterval("<<before.toString()<<", "<<before.addDays(-1)<<")"<<endl;
//     Duration e1(0, 2, 0); // 2 hours
//     Duration e2 = t->effort(before, after);
//     b = e1==e2;
//     kdDebug()<<"9: Success="<<b<<"      effort "<<e1.toString()<<"=="<<e2.toString()<<endl;
//     
//     delete t;
//     delete p;
//     }// end test 3
//     kdDebug()<<endl;
//     kdDebug()<<"------------Test 4 Parent calendar/weekdays-------------"<<endl;
//     {
//     QTime t1(8,0,0);
//     QTime t2(10,0,0);
//     Calendar *p = new Calendar("Test 4 parent");
//     CalendarDay *wd1 = p->weekday(0); // monday
//     if (wd1 == 0) {
//         kdDebug()<<"Failed to get weekday"<<endl;
//     }
//     wd1->setState(Map::NonWorking);
//     
//     CalendarDay *wd2 = p->weekday(2); // wednesday
//     if (wd2 == 0) {
//         kdDebug()<<"Failed to get weekday"<<endl;
//     }
//     wd2->addInterval(QPair<QTime, QTime>(t1, t2));
//     wd2->setState(Map::Working);
//      
//     Calendar *t = new Calendar("Test 4");
//     t->setParent(p);
//     QDate wdate(2006,1,2); // monday jan 2
//     DateTime before = DateTime(wdate.addDays(-4)); //Thursday dec 29
//     DateTime after = DateTime(wdate.addDays(4)); // Friday jan 6
//     DateTime wdt1(wdate, t1);
//     DateTime wdt2(QDate(2006, 1, 4), t2); // Wednesday
//     CalendarDay *day = new CalendarDay(QDate(2006,1,2), Map::Working);
//     day->addInterval(QPair<QTime, QTime>(t1, t2));
//     if (!p->addDay(day)) {
//         kdDebug()<<"Failed to add day"<<endl;
//         delete day;
//         delete t;
//         return;
//     }
//     kdDebug()<<"Added     date="<<day->date().toString()<<" "<<day->startOfDay().toString()<<" - "<<day->endOfDay().toString()<<endl;
//     kdDebug()<<"Found     date="<<day->date().toString()<<" "<<day->startOfDay().toString()<<" - "<<day->endOfDay().toString()<<endl;
//     
//     CalendarDay *d = p->findDay(wdate);
//     bool b = (day == d);
//     kdDebug()<<"1: Success="<<b<<"      Find same day"<<endl;
//     
//     DateTime dt = t->firstAvailableAfter(after, after.addDays(10));
//     b = (dt.isValid() && dt == DateTime(QDate(2006,1,11), t1));
//     kdDebug()<<"2: Success="<<b<<"      firstAvailableAfter("<<after<<"): ="<<dt<<endl;
//     
//     dt = t->firstAvailableBefore(before, before.addDays(-10));
//     b = (dt.isValid() && dt == DateTime(QDate(2005, 12, 28), t2));
//     kdDebug()<<"3: Success="<<b<<"       firstAvailableBefore("<<before.toString()<<"): ="<<dt<<endl;
//     
//     dt = t->firstAvailableAfter(before, after);
//     b = dt == wdt1; // We find the day jan 2
//     kdDebug()<<"4: Success="<<b<<"      firstAvailableAfter("<<before.toString()<<"): ="<<dt.toString()<<endl;
//     
//     dt = t->firstAvailableBefore(after, before);
//     b = dt == wdt2; // We find the weekday (wednesday)
//     kdDebug()<<"5: Success="<<b<<"      firstAvailableBefore("<<after.toString()<<"): ="<<dt.toString()<<endl;
//     
//     b = t->hasInterval(before, after);
//     kdDebug()<<"6: Success="<<b<<"      hasInterval("<<before.toString()<<", "<<after<<")"<<endl;
//     
//     b = !t->hasInterval(after, after.addDays(1));
//     kdDebug()<<"7: Success="<<b<<"      !hasInterval("<<after.toString()<<", "<<after.addDays(1)<<")"<<endl;
//     
//     b = !t->hasInterval(before, before.addDays(-1));
//     kdDebug()<<"8: Success="<<b<<"      !hasInterval("<<before.toString()<<", "<<before.addDays(-1)<<")"<<endl;
//     Duration e1(0, 4, 0); // 2 hours
//     Duration e2 = t->effort(before, after);
//     b = e1==e2;
//     kdDebug()<<"9: Success="<<b<<"      effort "<<e1.toString()<<"="<<e2.toString()<<endl;
//     
//     QPair<DateTime, DateTime> r = t->firstInterval(before, after);
//     b = r.first == wdt1; // We find the monday jan 2
//     kdDebug()<<"10: Success="<<b<<"      firstInterval("<<before<<"): ="<<r.first<<", "<<r.second<<endl;
//     r = t->firstInterval(r.second, after);
//     b = r.first == DateTime(QDate(2006, 1, 4),t1); // We find the wednesday jan 4
//     kdDebug()<<"11: Success="<<b<<"      firstInterval("<<r.second<<"): ="<<r.first<<", "<<r.second<<endl;
//     
//     delete t;
//     delete p;
//     }// end test 4
}
#endif

}  //KPlato namespace

#include "kptview.moc"
