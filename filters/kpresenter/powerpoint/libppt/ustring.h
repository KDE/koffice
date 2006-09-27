// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef LIBPPT_USTRING_H_
#define LIBPPT_USTRING_H_

namespace Libppt {

  /**
   * @return True if d is not a number (platform support required).
   */
  bool isNaN(double d);

  bool isPosInf(double d);
  bool isNegInf(double d);

  class UCharReference;
  class UString;
  class UConstString;

  /**
   * @short Unicode character.
   *
   * UChar represents a 16 bit Unicode character. It's internal data
   * representation is compatible to XChar2b and QChar. It's therefore
   * possible to exchange data with X and Qt with shallow copies.
   */
  struct UChar {
    /**
     * Construct a character with value 0.
     */
    UChar();
    /**
     * Construct a character with the value denoted by the arguments.
     * @param h higher byte
     * @param l lower byte
     */
    UChar(unsigned char h , unsigned char l);
    /**
     * Construct a character with the given value.
     * @param u 16 bit Unicode value
     */
    UChar(unsigned short u);
    UChar(const UCharReference &c);
    /**
     * @return The higher byte of the character.
     */
    unsigned char high() const { return uc >> 8; }
    /**
     * @return The lower byte of the character.
     */
    unsigned char low() const { return uc & 0xFF; }
    /**
     * @return the 16 bit Unicode value of the character
     */
    unsigned short unicode() const { return uc; }
  public:
    /**
     * @return The character converted to lower case.
     */
    UChar toLower() const;
    /**
     * @return The character converted to upper case.
     */
    UChar toUpper() const;
    /**
     * A static instance of UChar(0).
     */
    static UChar null;
  private:
    friend class UCharReference;
    friend class UString;
    friend bool operator==(const UChar &c1, const UChar &c2);
    friend bool operator==(const UString& s1, const char *s2);
    friend bool operator<(const UString& s1, const UString& s2);

    unsigned short uc;
  };

  inline UChar::UChar() : uc(0) { }
  inline UChar::UChar(unsigned char h , unsigned char l) : uc(h << 8 | l) { }
  inline UChar::UChar(unsigned short u) : uc(u) { }

  /**
   * @short Dynamic reference to a string character.
   *
   * UCharReference is the dynamic counterpart of @ref UChar. It's used when
   * characters retrieved via index from a @ref UString are used in an
   * assignment expression (and therefore can't be treated as being const):
   * <pre>
   * UString s("hello world");
   * s[0] = 'H';
   * </pre>
   *
   * If that sounds confusing your best bet is to simply forget about the
   * existence of this class and treat it as being identical to @ref UChar.
   */
  class UCharReference {
    friend class UString;
    UCharReference(UString *s, unsigned int off) : str(s), offset(off) { }
  public:
    /**
     * Set the referenced character to c.
     */
    UCharReference& operator=(UChar c);
    /**
     * Same operator as above except the argument that it takes.
     */
    UCharReference& operator=(char c) { return operator=(UChar(c)); }
    /**
     * @return Unicode value.
     */
    unsigned short unicode() const { return ref().unicode(); }
    /**
     * @return Lower byte.
     */
    unsigned char low() const { return ref().uc & 0xFF; }
    /**
     * @return Higher byte.
     */
    unsigned char high() const { return ref().uc >> 8; }
    /**
     * @return Character converted to lower case.
     */
    UChar toLower() const { return ref().toLower(); }
    /**
     * @return Character converted to upper case.
     */
    UChar toUpper() const  { return ref().toUpper(); }
  private:
    // not implemented, can only be constructed from UString
    UCharReference();

    UChar& ref() const;
    UString *str;
    int offset;
  };

  /**
   * @short 8 bit char based string class
   */
  class CString {
  public:
    CString() : data(0L) { }
    explicit CString(const char *c);
    CString(const CString &);

    ~CString();

    CString &append(const CString &);
    CString &operator=(const char *c);
    CString &operator=(const CString &);
    CString &operator+=(const CString &);

    int length() const;
    const char *c_str() const { return data; }
  private:
    char *data;
  };

  /**
   * @short Unicode string class
   */
  class UString {
    friend bool operator==(const UString&, const UString&);
    friend class UCharReference;
    friend class UConstString;
    /**
     * @internal
     */
    struct Rep {
      friend class UString;
      friend bool operator==(const UString&, const UString&);
      static Rep *create(UChar *d, int l);
      inline UChar *data() const { return dat; }
      inline int length() const { return len; }

      inline void ref() { rc++; }
      inline int deref() { return --rc; }

      UChar *dat;
      int len;
      int rc;
      static Rep null;
    };

  public:
    /**
     * Constructs a null string.
     */
    UString();
    /**
     * Constructs a string from the single character c.
     */
    explicit UString(char c);
    /**
     * Constructs a string from the single character c.
     */
    explicit UString(UChar c);
    /**
     * Constructs a string from a classical zero determined char string.
     */
    explicit UString(const char *c);
    /**
     * Constructs a string from an array of Unicode characters of the specified
     * length.
     */
    UString(const UChar *c, int length);
    /**
     * If copy is false a shallow copy of the string will be created. That
     * means that the data will NOT be copied and you'll have to guarantee that
     * it doesn't get deleted during the lifetime of the UString object.
     */
    UString(UChar *c, int length, bool copy);
    /**
     * Copy constructor. Makes a shallow copy only.
     */
    UString(const UString &);
    /**
     * Destructor. If this handle was the only one holding a reference to the
     * string the data will be freed.
     */
    ~UString();

    /**
     * Constructs a string from an int.
     */
    static UString from(int i);
    /**
     * Constructs a string from an unsigned int.
     */
    static UString from(unsigned int u);
    /**
     * Constructs a string from a double.
     */
    static UString from(double d);

    /**
     * Append another string.
     */
    UString &append(const UString &);

    /**
     * @return The string converted to the 8-bit string type @ref CString().
     */
    CString cstring() const;
    /**
     * Convert the Unicode string to plain ASCII chars chopping of any higher
     * bytes. This method should only be used for *debugging* purposes as it
     * is neither Unicode safe nor free from side effects. In order not to
     * waste any memory the char buffer is static and *shared* by all UString
     * instances.
     */
    char *ascii() const;

    /**
     * Assignment operator.
     */
    UString &operator=(const char *c);
    /**
     * Assignment operator.
     */
    UString &operator=(const UString &);
    /**
     * Appends the specified string.
     */
    UString &operator+=(const UString &s);

    /**
     * @return A pointer to the internal Unicode data.
     */
    const UChar* data() const { return rep->data(); }
    /**
     * @return True if null.
     */
    bool isNull() const { return (rep == &Rep::null); }
    /**
     * @return True if null or zero length.
     */
    bool isEmpty() const { return (!rep->len); }
    /**
     * Use this if you want to make sure that this string is a plain ASCII
     * string. For example, if you don't want to lose any information when
     * using @ref cstring() or @ref ascii().
     *
     * @return True if the string doesn't contain any non-ASCII characters.
     */
    bool is8Bit() const;
    /**
     * @return The length of the string.
     */
    int length() const { return rep->length(); }
    /**
     * Const character at specified position.
     */
    UChar operator[](int pos) const;
    /**
     * Writable reference to character at specified position.
     */
    UCharReference operator[](int pos);

    /**
     * Attempts an conversion to a number. Apart from floating point numbers,
     * the algorithm will recognize hexadecimal representations (as
     * indicated by a 0x or 0X prefix) and +/- Infinity.
     * Returns NaN if the conversion failed.
     * @param tolerant if true, toDouble can tolerate garbage after the number.
     */
    double toDouble(bool tolerant=false) const;
    /**
     * Attempts an conversion to an unsigned long integer. ok will be set
     * according to the success.
     */
    unsigned long toULong(bool *ok = 0L) const;
    /**
     * @return Position of first occurrence of f starting at position pos.
     * -1 if the search was not successful.
     */
    int find(const UString &f, int pos = 0) const;
    /**
     * @return Position of first occurrence of f searching backwards from
     * position pos.
     * -1 if the search was not successful.
     */
    int rfind(const UString &f, int pos) const;
    /**
     * @return The sub string starting at position pos and length len.
     */
    UString substr(int pos = 0, int len = -1) const;
    /**
     * Static instance of a null string.
     */
    static UString null;

  private:
    void attach(Rep *r);
    void detach();
    void release();
    Rep *rep;
  };

  inline bool operator==(const UChar &c1, const UChar &c2) {
    return (c1.uc == c2.uc);
  }
  inline bool operator!=(const UChar &c1, const UChar &c2) {
    return !(c1 == c2);
  }
  bool operator==(const UString& s1, const UString& s2);
  inline bool operator!=(const UString& s1, const UString& s2) {
    return !Libppt::operator==(s1, s2);
  }
  bool operator<(const UString& s1, const UString& s2);
  bool operator==(const UString& s1, const char *s2);
  inline bool operator!=(const UString& s1, const char *s2) {
    return !Libppt::operator==(s1, s2);
  }
  inline bool operator==(const char *s1, const UString& s2) {
    return operator==(s2, s1);
  }
  inline bool operator!=(const char *s1, const UString& s2) {
    return !Libppt::operator==(s1, s2);
  }
  bool operator==(const CString& s1, const CString& s2);
  UString operator+(const UString& s1, const UString& s2);


  class UConstString : private UString {
    public:
      UConstString( UChar* data, unsigned int length );
      ~UConstString();

      const UString& string() const { return *this; }
  };

} 

#endif /* LIBPPT_USTRING_H_ */
