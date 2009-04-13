#!/usr/bin/env kross
# -*- coding: utf-8 -*-

import traceback, Kross, KWord

class ToolActions:

    def __init__(self, scriptaction):
        self.scriptaction = scriptaction
        #self.currentpath = self.scriptaction.currentPath()
        self.forms = Kross.module("forms")
        self.dialog = self.forms.createDialog("Tool Actions")
        self.dialog.minimumWidth = 500
        self.dialog.minimumHeight = 360
        self.dialog.setButtons("Ok|Cancel")
        self.dialog.setFaceType("Plain") #Auto Plain List Tree Tabbed

        self.actions = []
        tool = KWord.tool()
        for n in tool.actionNames():
            self.actions.append( "%s" % n )

        page = self.dialog.addPage("", "")
        widget = self.forms.createWidgetFromUI(page,
            '<ui version="4.0" >'
            ' <class>Form</class>'
            ' <widget class="QWidget" name="Form" >'
            '  <layout class="QHBoxLayout" >'
            '   <item>'
            '    <widget class="QListWidget" name="List">'
            '     <property name="currentRow"><number>%i</number></property>'
            '     %s'
            '    </widget>'
            '   </item>'
            '  </layout>'
            ' </widget>'
            '</ui>'
            % ( 0, ''.join( [ '<item><property name="text" ><string>%s</string></property></item>' % s for s in self.actions ] ) )
        )
        self.widgetlist = widget["List"]

        if self.dialog.exec_loop():
            if len(self.actions) > 0:
                self.execAction()

    def __del__(self):
        self.dialog.delayedDestruct()

    def execAction(self):
        try:
            name = self.actions[ self.widgetlist.currentRow ]
            KWord.tool().triggerAction( name.strip() )
        except:
            message = "".join( traceback.format_exception(sys.exc_info()[0],sys.exc_info()[1],sys.exc_info()[2]) )
            self.forms.showMessageBox("Error", "Error", "%s" % message)

ToolActions(self)
