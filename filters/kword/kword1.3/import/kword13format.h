//

/*
   This file is part of the KDE project
   Copyright (C) 2004 Nicolas GOUTTE <goutte@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef KWORD13_FORMAT
#define KWORD13_FORMAT

class KWord13FormatOneData;

/**
 * \brief Basic classes for all format classes
 *
 * For \<FORMAT\>
 */
class KWord13Format
{
public:
    KWord13Format(void);
    virtual ~KWord13Format(void);
public:
    /// Get lenght of format (1 except for texts)
    virtual int length( void );
    /**
     * Get a pointer to the data correponding to \<FORMAT id="1"\>
     *
     * \note The returned pointer MUST NOT be deleted and also it can be NULL!
     */
    KWord13FormatOneData* getFormatOneData(void);
public:
    int m_id; ///< Type of format
    int m_pos; ///< Position of format
};


#endif // KWORD13_FORMAT
