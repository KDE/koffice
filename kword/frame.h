/******************************************************************/ 
/* KWord - (c) by Reginald Stadlbauer and Torben Weis 1997-1998   */
/* Version: 0.0.1                                                 */
/* Author: Reginald Stadlbauer, Torben Weis                       */
/* E-Mail: reggie@kde.org, weis@kde.org                           */
/* Homepage: http://boch35.kfunigraz.ac.at/~rs                    */
/* needs c++ library Qt (http://www.troll.no)                     */
/* written for KDE (http://www.kde.org)                           */
/* needs mico (http://diamant.vsb.cs.uni-frankfurt.de/~mico/)     */
/* needs OpenParts and Kom (weis@kde.org)                         */
/* License: GNU GPL                                               */
/******************************************************************/
/* Module: Frame (header)                                         */
/******************************************************************/

#ifndef frame_h
#define frame_h

#include <qrect.h>
#include <qpoint.h>
#include <qlist.h>

#include "parag.h"
#include "paraglayout.h"

#include <iostream>
#include <koStream.h>

class KWordDocument_impl;

enum FrameType {FT_BASE = 0,FT_TEXT = 1,FT_PICTURE = 2};
enum RunAround {RA_NO = 0,RA_BOUNDUNGRECT = 1,RA_CONTUR = 2};

/******************************************************************/
/* Class: KWFrame                                                 */
/******************************************************************/

class KWFrame : public QRect
{
public:
  KWFrame() : QRect() { runAround = RA_NO; }
  KWFrame(const QPoint &topleft,const QPoint &bottomright) : QRect(topleft,bottomright) { runAround = RA_NO; }     
  KWFrame(const QPoint &topleft,const QSize &size) : QRect(topleft,size) { runAround = RA_NO; }    
  KWFrame(int left,int top,int width,int height) : QRect(left,top,width,height) { runAround = RA_NO; }
  KWFrame(int left,int top,int width,int height,RunAround _ra) : QRect(left,top,width,height) { runAround = _ra; }

  void setRunAround(RunAround _ra) { runAround = _ra; }
  RunAround getRunAround() { return runAround; }

protected:
  RunAround runAround;

};

/******************************************************************/
/* Class: KWFrameSet                                              */
/******************************************************************/

class KWFrameSet
{
public:
  KWFrameSet(KWordDocument_impl *_doc);
  virtual ~KWFrameSet() 
    {;}

  virtual FrameType getFrameType()
    { return FT_BASE; }

  virtual void addFrame(KWFrame _rect);
  virtual void delFrame(unsigned int _num);

  virtual int getFrame(int _x,int _y);
  virtual KWFrame getFrame(unsigned int _num);
  virtual KWFrame *getFramePtr(unsigned int _num);
  virtual unsigned int getNumFrames()
    { return frames.count(); }

  virtual bool isPTYInFrame(unsigned int _frame,unsigned int _ypos)
    { return true; }

  virtual void update()
    {;}

  virtual bool contains(unsigned int mx,unsigned int my);

  virtual void save(ostream &out);

protected:
  virtual void init()
    {;}

  // document
  KWordDocument_impl *doc;

  // frames
  QList<KWFrame> frames;

};

/******************************************************************/
/* Class: KWFrameSet                                              */
/******************************************************************/

class KWTextFrameSet : public KWFrameSet
{
public:
  KWTextFrameSet(KWordDocument_impl *_doc)
    : KWFrameSet(_doc) 
    {;}
  virtual ~KWTextFrameSet() 
    {;}

  virtual FrameType getFrameType()
    { return FT_TEXT; }

  virtual void update();
  
  virtual void clear()
    { frames.clear(); }

  /**
   * If another parag becomes the first one it uses this function
   * to tell the document about it.
   */
  void setFirstParag(KWParag *_parag)
    { parags = _parag; }

  KWParag* getFirstParag() { return parags; }

  virtual bool isPTYInFrame(unsigned int _frame,unsigned int _ypos);

  void deleteParag(KWParag *_parag);
  void joinParag(KWParag *_parag1,KWParag *_parag2);
  void insertParag(KWParag *_parag,InsertPos _pos);
  void splitParag(KWParag *_parag,unsigned int _pos);

  virtual void save(ostream &out);
  virtual void load(KOMLParser&,vector<KOMLAttrib>&);

protected:
  virtual void init();

  // pointer to the first parag of the list of parags
  KWParag *parags;

  KWParagLayout* defaultParagLayout;

};


#endif
