#ifndef KWFRAMELAYOUT_H
#define KWFRAMELAYOUT_H

#include <qptrlist.h>
#include <kdebug.h>
#include <koRect.h>

class KWDocument;
class KWFrameSet;
class KWTextFrameSet;
class KoRect;

class KWFrameLayout
{
public:
    // Maybe all that data should go into a KWHeaderFooterFrameSet
    // (or rather a base class shared by KWFootNoteFrameSet....)
    struct HeaderFooterFrameset {
        enum OddEvenAll { Odd, Even, All };

        HeaderFooterFrameset( KWTextFrameSet* fs, int start, int end,
                              double spacing, OddEvenAll oea = All );

        // Frameset. Also gives the type (header, footer, footnote).
        KWTextFrameSet* m_frameset;

        // Future features - but already used for "first page" stuff
        int m_startAtPage;
        int m_endAtPage; // (-1 for no end)

        // Odd/even/all
        OddEvenAll m_oddEvenAll;

        // Height in pt
        double m_height;

        // Space between this frame and the next one
        // (the one at bottom for headers, the one on top for footers/footnotes).
        // e.g. ptHeaderBodySpacing for headers/footers
        double m_spacing;

        // Minimum Y value - for footnotes
        double m_minY;

        // True once the footnote has been correctly positionned and
        // shouldn't be moved by checkFootNotes anymore.
        bool m_positioned;

        // frame number for the given page.... -1 if no frame on that page
        int frameNumberForPage( int page ) const
            {
                if ( page < m_startAtPage || ( m_endAtPage != -1 && page > m_endAtPage ) )
                    return -1;
                int pg = page - m_startAtPage; // always >=0
                switch (m_oddEvenAll) {
                case Even:
                    if ( page % 2 == 0 ) // even/odd is for the absolute page number, too confusing otherwise
                        return pg / 2; // page 0[+start] -> frame 0, page 2[+start] -> frame 1
                    else
                        return -1;
                case Odd:
                    if ( page % 2 )
                        return pg / 2; // page 1 -> 0, page 3 -> 1
                    else
                        return -1;
                case All:
                    return pg; // page 0[+start] -> frame 0, etc.
                default:
                    return -1;
                }
            }

        // the last frame we need, layout() will delete any frame after that
        int lastFrameNumber( int lastPage ) const
            {
                if ( lastPage < m_startAtPage )
                    return -1; // we need none
                int pg = lastPage;
                if ( m_endAtPage > -1 )
                    pg = QMIN( m_endAtPage, pg );
                pg -= m_startAtPage; // always >=0
                Q_ASSERT( pg >= 0 );
                switch (m_oddEvenAll) {
                case Even:
                case Odd:
                    return pg / 2; // page 0 and 1 -> 0. page 2 and 3 -> 1.
                case All:
                    return pg; // page 0 -> 0 etc. ;)
                default:
                    return -1;
                }
            }

        void debug();
        void deleteFramesAfterLast( int lastPage );
    };

    /**
     * Constructor
     */
    KWFrameLayout( KWDocument* doc, QPtrList<HeaderFooterFrameset>& headersFooters, QPtrList<HeaderFooterFrameset>& footnotes )
        : m_headersFooters( headersFooters ), m_footnotes( footnotes ), m_doc( doc )
        {}

    /**
     * The main method of this file. Do the frame layout.
     * @param mainTextFrameSet if set, its frames will be resized. Usually: set in WP mode, not set in DTP mode.
     * @param numColumns number of columns to create for the main textframeset. Only relevant if mainTextFrameSet!=0.
     */
    void layout( KWFrameSet* mainTextFrameSet, int numColumns,
                 int fromPage, int toPage );

protected:
    void resizeOrCreateHeaderFooter( KWTextFrameSet* headerFooter, uint frameNumber, const KoRect& rect );
    KoRect firstColumnRect( KWFrameSet* mainTextFrameSet, int pageNum, int numColumns ) const;
    bool resizeMainTextFrame( KWFrameSet* mainTextFrameSet, int pageNum, int numColumns, double ptColumnWidth, double ptColumnSpacing, double left, double top, double bottom, bool hasFootNotes );
    void checkFootNotes();

private:
    // A _ref_ to a list. Must remain alive as long as this object.
    QPtrList<HeaderFooterFrameset>& m_headersFooters;
    QPtrList<HeaderFooterFrameset>& m_footnotes;
    KWDocument* m_doc;
};

#endif
