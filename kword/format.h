#ifndef format_h
#define format_h

class KWFormat;
class KWordDocument_impl;

#include "defs.h"
#include "font.h"

/**
 * This class is used to change the texts format within a paragraph.
 *
 * @see KWParagraph
 */
class KWFormat
{
public:
    /**
     * Creates a new KWFormat instance. 
     *
     * @param _color The color to use for the text. If this color is not not valid
     *               ( find out with QColor::isValid ) then the color does not change.
     *               Pass <TT>QColor()</TT> if you dont want to change the color at all.
     * @param _font A pointer to the font family. if this pointer is 0L that means
     *              that the text does not change its current font family.
     * @param _font_size The size of the font to use or -1 if this value does not change.
     * @param _weight The fonts weight or -1 if the value should not change
     * @param _italic 1 to enable italic font, 0 to disable and -1 for no change.
     * @param _math 1 to enable math mode, 0 to disable and -1 for no change.
     * @param _direct 1 to enable direct mode, 0 to disable and -1 for no change.
     *
     * @see KWUserFont
     */
    KWFormat( QColor& _color, KWUserFont *_font = 0L, int _font_size = -1, int _weight = -1,
	      char _italic = -1, char _math = -1, char _direct = -1 );
    
    /**
     * Creates a new KWFormat instance. This instance has set all values to
     * 'dont change'.
     */
    KWFormat();
    
    /**
     * Creates a new KWFormat instance.
     *
     * @param _format It copies its state from this KWFormat instance.
     */
    KWFormat( KWFormat& _format );
    
    /**
     * @return The color to use. The color may be not valid ( test with <TT>QColor::isValid()</TT>.
     *         In this case you shoud not use the color returned.
     */
    QColor& getColor() { return color; }
    
    /**
     * @return The font family to use. The return value may be 0L if the font should not change.
     */
    KWUserFont* getUserFont() { return userFont; }

    KWDisplayFont* loadFont( KWordDocument_impl *_doc, QPainter &_painter );
    
    void apply( KWFormat &_format );
    
    /**
     * @return The font size to use in points. The return value may be -1 if the size should not change.
     */
    int getPTFontSize() { return ptFontSize; }

    /**
     * @return The fonts weight to use. The return value may be -1 if the weight should not change.
     */
    int getWeight() { return weight; }

    /**
     * @return The italic mode. The return value may be -1 if the italic mode should not change.
     */
    char getItalic() { return italic; }

    /**
     * Fills all values with defaults. No value will remain in the 'dont change' state.
     * You may want to use this function to get a default font & color.
     *
     * @param _doc is the document this format belongs to. The document provides information
     *             on the default font for example.
     *
     * @see KWordDocument_impl
     */
    void setDefaults( KWordDocument_impl *_doc );
    
    /**
     * Set the color to use.
     * @param _color The color to use for the text. If this color is not not valid
     *               ( find out with QColor::isValid ) then the color does not change.
     *               Pass <TT>QColor()</TT> if you dont want to change the color at all.
     *
     * @see $QColor
     */
    void setColor( QColor& _c ) { color = _c; }

    /**
     * Sets the font to use.
     *
     * @param _font A pointer to the font family. if this pointer is 0L that means
     *              that the text does not change its current font family.
     *
     * @see KWUserFont
     */
    void setUserFont( KWUserFont* _font ) { userFont = _font; }
    
    /**
     * Sets the unzoomed font size to use.
     *
     * @param _font_size The size of the font to use or -1 if this value does not change.    
     */
    void setPTFontSize( int _size ) { ptFontSize = _size; }

    /**
     * Sets the fonts weight to use.
     *
     * @param _weight The fonts weight or -1 if the value should not change
     */
    void setWeight( int _weight ) { weight = _weight; }

    /**
     * Sets the fonts italic mode.
     *
     * @param _italic 1 to enable italic font, 0 to disable and -1 for no change.
     */
    void setItalic( char _italic ) { italic = _italic; }

protected:
    /**
     * Pointer to the font we have to use. If this value is 0L we are told
     * not to change the current font family.
     * @see KWUserFont
     */
    KWUserFont *userFont;    
    /**
     * The size of the font in points. If this is -1 then we are told not
     * to change the fonts size.
     */
    int ptFontSize;
    /**
     * The weight of the font. A value of -1 tells us not to change the fonts weight.
     */
    int weight;
    /**
     * Indicates wether to use italic or not. A value of 0 tells us 'no',
     * a value of 1 tells 'yes' and a value of -1 tells us 'dont change
     * the mode'.
     */
    char italic;
    /**
     * The color to use for the text. If this color is not not valid
     * ( find out with QColor::isValid ) then the color does not change.
     */
    QColor color;
    /**
     * 1 to enable math mode, 0 to disable and -1 for no change.
     */
    char math;
    /**
     * 1 to enable direct mode, 0 to disable and -1 for no change.
     */
    char direct;
};

#endif


