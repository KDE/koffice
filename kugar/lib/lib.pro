SOURCES	+= mcalcobject.cpp \
	mfieldobject.cpp \
	mlabelobject.cpp \
	mlineobject.cpp \
	mpagecollection.cpp \
	mpagedisplay.cpp \
	mreportdetail.cpp \
	mreportengine.cpp \
	mreportobject.cpp \
	mreportsection.cpp \
	mreportviewer.cpp \
	mspecialobject.cpp \
	mutil.cpp \
	mdatabasereportengine.cpp \
	sqlcursor.cpp
HEADERS	+= kugarqt.h \
	mcalcobject.h \
	mfieldobject.h \
	mlabelobject.h \
	mlineobject.h \
	mpagecollection.h \
	mpagedisplay.h \
	mreportdetail.h \
	mreportengine.h \
	mreportobject.h \
	mreportsection.h \
	mreportviewer.h \
	mspecialobject.h \
	mutil.h \
	mdatabasereportengine.h \
	sqlcursor.h

# File generated by kdevelop's qmake manager. 
# ------------------------------------------- 
# Subdir relative project main directory: ./lib
# Target is a library:  libkugar

TARGET = kugarqt
MOC_DIR = moc
OBJECTS_DIR = obj
UI_DIR = ui
MAKEFILE=Makefile.qt
TEMPLATE	=lib 
CONFIG	+= release warn_on thread
DEFINES	+= PURE_QT
LANGUAGE	= C++
