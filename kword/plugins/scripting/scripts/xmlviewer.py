#!/usr/bin/env kross

import os, sys, re, types, string, datetime, tempfile

import Kross
try:
    import KWord
except ImportError:
    KWord = Kross.module("kword")

class Dialog:
    def __init__(self, action):
        self.forms = Kross.module("forms")
        self.dialog = self.forms.createDialog("XML Viewer")
        self.dialog.setButtons("Ok")
        self.dialog.setFaceType("List") #Auto Plain List Tree Tabbed
        self.dialog.minimumWidth = 620
        self.dialog.minimumHeight = 400

        doc = KWord.document()
        #doc.openUrl("/home/kde4/odf/_works/Lists_bulletedList/testDoc/testDoc.odt")
        self.store = KWord.store()
        self.pages = {}

        reader = self.store.open("META-INF/manifest.xml")
        if not reader:
            raise "Failed to read the mainfest"
        for i in range( reader.count() ):
            typeName = reader.type(i)
            path = reader.path(i)
            widgets = []
            if typeName == "text/xml":
                page = self.dialog.addPage(path, "")

                browser = self.forms.createWidget(page, "QTextBrowser", "Editor")
                widgets.append(browser)
                ##self.part = self.forms.loadPart(partpage, "libkmultipart","file:///home/kde4/kword.py")
                #self.part = self.forms.loadPart(partpage, "libnotepadpart","file:///home/kde4/kword.py")
                #if not self.part:
                    #raise "Failed to load the KPart"

                w = self.forms.createWidget(page, "QWidget")
                self.forms.createLayout(w,"QHBoxLayout")

                openBtn = self.forms.createWidget(w, "QPushButton")
                openBtn.text = "Open with..."
                openBtn.connect("clicked()", self.openClicked)
                widgets.append(openBtn)

                kxmleditorBtn = self.forms.createWidget(w, "QPushButton")
                kxmleditorBtn.text = "Open with KXMLEditor"
                kxmleditorBtn.connect("clicked(bool)", self.kxmleditorClicked)
                widgets.append(kxmleditorBtn)

                #saveBtn = self.forms.createWidget(w, "QPushButton")
                #saveBtn.text = "Save to..."
                #saveBtn.connect("clicked(bool)", self.saveClicked)

            self.pages[path] = [typeName, None, widgets]

        #doc.setDefaultStyleSheet(
            #(
                #"li { text-color:#ff0000; color:#ff0000; background:#ff0000; background-color:#ff0000; margin:5em; }"
            #)
        #)

        self.dialog.connect("currentPageChanged(KPageWidgetItem*,KPageWidgetItem*)",self.currentPageChanged)
        self.currentPageChanged()

        if self.dialog.exec_loop():
            pass

    def __del__(self):
        self.dialog.delayedDestruct()

    def currentPageChanged(self, *args):
        path = self.dialog.currentPage()
        (text,widgets) = self.pages[path][1:3]
        self._text = text
        if self._text:
            return
        self._text = ""
        self._prevLevel = -1
        reader = self.store.open(path)
        if not reader:
            raise "failed to open %s" % path
        def onElement():
            #print "############### name=%s namespaceURI=%s level=%s" % (reader.name(),reader.namespaceURI(),reader.level())
            level = reader.level()
            if level > self._prevLevel:
                self._text += "<li><ul>"
                self._prevLevel = reader.level()
            if level < self._prevLevel:
                self._text += "</ul></li>"
            self._text += "<li>%s" % reader.name()
            #print "  attributeNames=%s" % reader.attributeNames()
            #print "  isElement=%s isText=%s" % (reader.isElement(),reader.isText())
            #if reader.isText():
            #if not reader.hasChildren() and reader.text():
            if reader.name().startswith("text:") and reader.text():
                self._text += "<blockquote>%s</blockquote>" % reader.text()
            self._text += "</li>"
        reader.connect("onElement()", onElement)
        reader.start()
        widgets[0].html = "%s" % self._text
        self.pages[path][1] = self._text

        #self.store.close()
        #widget.plainText = "%s" % self.store.extract(path)

    def doOpen(self, program):
        path = self.dialog.currentPage()
        typeName = self.pages[path][0]
        print "START doOpen program=\"%s\" path=\"%s\" typeName=\"%s\"" % (program,path,typeName)

        toFile = tempfile.mktemp()
        if typeName == "text/xml":
            toFile += ".xml"
        if not self.store.extractToFile(path,toFile):
            raise "Failed to extract \"%s\" to \"%s\"" % (path,tempfile)
        os.system( "\"%s\" \"%s\"" % (program,toFile) )
        if os.path.isfile(toFile):
            os.remove(toFile)

        print "DONE doOpen program=\"%s\" path=\"%s\" typeName=\"%s\"" % (program,path,typeName)

    def openClicked(self, *args):
        dialog = self.forms.createDialog("Open with...")
        dialog.setButtons("Ok|Cancel")
        dialog.setFaceType("Plain") #Auto Plain List Tree Tabbed
        dialog.minimumWidth = 360
        page = dialog.addPage("", "")
        edit = self.forms.createWidget(page, "QLineEdit", "Filter")
        edit.text = 'kwrite'
        edit.setFocus()
        if dialog.exec_loop():
            program = edit.text.strip()
            dialog.delayedDestruct()
            if not program:
                raise "No program defined."
            self.doOpen(program)

    def kxmleditorClicked(self, *args):
        self.doOpen('kxmleditor')

Dialog(self)
