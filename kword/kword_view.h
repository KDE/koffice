/******************************************************************/
/* KWord - (c) by Reginald Stadlbauer 1997-1998                   */
/* based on Torben Weis' KWord                                    */
/* Version: 0.0.1                                                 */
/* Author: Reginald Stadlbauer                                    */
/* E-Mail: reggie@kde.org                                         */
/* Homepage: http://boch35.kfunigraz.ac.at/~rs                    */
/* needs c++ library Qt (http://www.troll.no)                     */
/* written for KDE (http://www.kde.org)                           */
/* needs mico (http://diamant.vsb.cs.uni-frankfurt.de/~mico/)     */
/* needs OpenParts and Kom (weis@kde.org)                         */
/* License: GNU GPL                                               */
/******************************************************************/
/* Module: View (header)                                          */
/******************************************************************/

#ifndef kword_view_h
#define kword_view_h

class KWordView_impl;
class KWordDocument_impl;
class KWordChild;
class KWordGUI;
class KWPaintWindow;

#include <view_impl.h>
#include <document_impl.h>
#include <part_frame_impl.h>
#include <menu_impl.h>
#include <toolbar_impl.h>

#include <qpixmap.h>
#include <qwidget.h>
#include <qrect.h>
#include <qlist.h>
#include <qevent.h>
#include <qscrollbar.h>
#include <qcolor.h>
#include <qfont.h>
#include <qmessagebox.h>

#include <kruler.h>
#include <kcolordlg.h>

#include "kword.h"
#include "kword_page.h"
#include "format.h"

#include <koPageLayoutDia.h>

/******************************************************************/
/* Class: KWordFrame                                              */
/******************************************************************/

class KWordFrame : public PartFrame_impl
{
  Q_OBJECT

public:
  KWordFrame(KWordView_impl*,KWordChild*);
  
  KWordChild* child() 
    { return m_pKWordChild; }
  KWordView_impl* view() 
    { return m_pKWordView; }
  
protected:
  KWordChild *m_pKWordChild;
  KWordView_impl *m_pKWordView;

};

/******************************************************************/
/* Class: KWordView_impl                                          */
/******************************************************************/

class KWordView_impl : public QWidget,
		       virtual public View_impl,
		       virtual public KWord::KWordView_skel
{
  Q_OBJECT

public:
  // C++
  KWordView_impl(QWidget *_parent = 0L,const char *_name = 0L);
  virtual ~KWordView_impl();

  // IDL  
  virtual void newView();

  virtual void textSizeSelected(const char *size);
  virtual void textFontSelected(const char *font);
  virtual void textBold();
  virtual void textItalic();
  virtual void textUnderline();
  virtual void textColor();
  virtual void textAlignLeft();
  virtual void textAlignCenter();
  virtual void textAlignRight();
  virtual void textEnumList();
  virtual void textUnsortList();

  virtual void extraLayout();

  virtual void helpAbout();

  virtual void setMode(OPParts::Part::Mode _mode);
  virtual void setFocus(CORBA::Boolean mode);

  // C++
  virtual void setDocument(KWordDocument_impl *_doc);

  virtual void createGUI();
  virtual void construct();
  virtual void setFormat(KWFormat &_format,bool _check = true);

  KWordGUI *getGUI() { return gui; }

public slots:
  void slotInsertObject(KWordChild *_child);
  void slotUpdateChildGeometry(KWordChild *_child);
  void slotGeometryEnd(PartFrame_impl*);
  void slotMoveEnd(PartFrame_impl*);
  
protected:
  virtual void cleanUp();
  
  void resizeEvent(QResizeEvent *e);
  void keyPressEvent(QKeyEvent *e);
  void mousePressEvent(QMouseEvent *e);
  void mouseReleaseEvent(QMouseEvent *e);
  void mouseMoveEvent(QMouseEvent *e);

  void setupMenu();
  void setupEditToolbar();
  void setupInsertToolbar();
  void setupTextToolbar();

  char* colorToPixString(QColor);
  void getFonts();


  KWordDocument_impl *m_pKWordDoc;

  bool m_bUnderConstruction;
  
  OPParts::MenuBarFactory_var m_vMenuBarFactory;
  MenuBar_ref m_rMenuBar;
  
  // edit menu
  CORBA::Long m_idMenuEdit;
  CORBA::Long m_idMenuEdit_Undo;
  CORBA::Long m_idMenuEdit_Redo;
  CORBA::Long m_idMenuEdit_Cut;
  CORBA::Long m_idMenuEdit_Copy;
  CORBA::Long m_idMenuEdit_Paste;
  CORBA::Long m_idMenuEdit_SelectAll;
  CORBA::Long m_idMenuEdit_Find;
  CORBA::Long m_idMenuEdit_FindReplace;

  // view menu
  CORBA::Long m_idMenuView;
  CORBA::Long m_idMenuView_NewView;
 
  // insert menu
  CORBA::Long m_idMenuInsert;
  CORBA::Long m_idMenuInsert_Page;
  CORBA::Long m_idMenuInsert_Picture;
  CORBA::Long m_idMenuInsert_Clipart;
  CORBA::Long m_idMenuInsert_Table;
  CORBA::Long m_idMenuInsert_Part;
 
  // extra menu
  CORBA::Long m_idMenuExtra;
  CORBA::Long m_idMenuExtra_Font;
  CORBA::Long m_idMenuExtra_Color;
  CORBA::Long m_idMenuExtra_Format;
  CORBA::Long m_idMenuExtra_Layout;
  CORBA::Long m_idMenuExtra_Options;

  // help menu
  CORBA::Long m_idMenuHelp;
  CORBA::Long m_idMenuHelp_Contents;
  CORBA::Long m_idMenuHelp_About;
  CORBA::Long m_idMenuHelp_AboutKOffice;
  CORBA::Long m_idMenuHelp_AboutKDE;

  OPParts::ToolBarFactory_var m_vToolBarFactory;
  ToolBar_ref m_rToolBarFile;
  QList<KWordFrame> m_lstFrames;

  // edit toolbar
  ToolBar_ref m_rToolBarEdit;
  CORBA::Long m_idButtonEdit_Undo;
  CORBA::Long m_idButtonEdit_Redo;
  CORBA::Long m_idButtonEdit_Cut;
  CORBA::Long m_idButtonEdit_Copy;
  CORBA::Long m_idButtonEdit_Paste;

  // insert toolbar
  ToolBar_ref m_rToolBarInsert;
  CORBA::Long m_idButtonInsert_Picture;
  CORBA::Long m_idButtonInsert_Clipart;
  CORBA::Long m_idButtonInsert_Table;
  CORBA::Long m_idButtonInsert_Part;

  // text toolbar
  ToolBar_ref m_rToolBarText;
  CORBA::Long m_idComboText_FontSize;
  CORBA::Long m_idComboText_FontList;
  CORBA::Long m_idButtonText_Bold;
  CORBA::Long m_idButtonText_Italic;
  CORBA::Long m_idButtonText_Underline;
  CORBA::Long m_idButtonText_Color;
  CORBA::Long m_idButtonText_ARight;
  CORBA::Long m_idButtonText_ACenter;
  CORBA::Long m_idButtonText_ALeft;
  CORBA::Long m_idButtonText_ABlock;
  CORBA::Long m_idButtonText_EnumList;
  CORBA::Long m_idButtonText_UnsortList;

  // text toolbar values
  QFont tbFont;
  QColor tbColor;
  QStrList fontList;

  KWordGUI *gui;
  bool m_bShowGUI;

  KWFormat format;

};

/******************************************************************/
/* Class: KWordGUI                                                */
/******************************************************************/

class KWordGUI : public QWidget
{
  Q_OBJECT

public:
  KWordGUI(QWidget *parent,bool __show,KWordDocument_impl *_doc,KWordView_impl *_view);
  
  void showGUI(bool __show);
  KWordDocument_impl *getDocument()
    { return doc; }
  void setDocument(KWordDocument_impl *_doc)
    { doc = _doc; paperWidget->setDocument(doc); }

  QScrollBar *getVertScrollBar()
    { return s_vert; }
  QScrollBar *getHorzScrollBar()
    { return s_horz; }
  KWordView_impl *getView()
    { return view; }
  KWPage *getPaperWidget()
    { return paperWidget; }
  
  void setOffset(int _x,int _y)
    { xOffset = _x; yOffset = _y; }

  void keyEvent(QKeyEvent *e)
    { keyPressEvent(e); }

  void setRanges();

  void scrollTo(int _x,int _y)
    { if (_x != xOffset) scrollH(_x); if (_y != yOffset) scrollV(_y); }

protected slots:
  void scrollH(int);
  void scrollV(int);

protected:
  void resizeEvent(QResizeEvent *e);
  void keyPressEvent(QKeyEvent *e);
  void reorganize();

  int xOffset,yOffset;
  bool _show;
  QScrollBar *s_vert,*s_horz;
  KRuler *r_vert,*r_horz; 
  KWPage *paperWidget;
  KWordDocument_impl *doc;
  KWordView_impl *view;

};

#endif




