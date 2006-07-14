/*
 *  Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2006 Isaac Clerencia <isaac@warp.es>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KSPREAD_KROSS_KRS_DOC_H_
#define KSPREAD_KROSS_KRS_DOC_H_

#include <kspread_doc.h>

#include <api/class.h>

namespace Kross { namespace KSpreadCore {

class Doc : public Kross::Api::Class<Doc>
{
    public:
        explicit Doc(KSpread::Doc* doc);
        virtual ~Doc();
        virtual const QString getClassName() const;
    private:
	/**
	 * This function return the Sheet currently active in this Doc.
	 *
	 * Example (in Ruby) :
	 * @code
	 * doc = krosskspreadcore::get("KSpreadDocument")
	 * sheet = doc.currentSheet()
	 * @endcode
	 */
        Kross::Api::Object::Ptr currentSheet(Kross::Api::List::Ptr);
	/**
	 * This function return a Sheet by name.
	 *
	 * Example (in Ruby) :
	 * @code
	 * doc = krosskspreadcore::get("KSpreadDocument")
	 * sheet = doc.sheetByName("foosheet")
	 * @endcode
	 */
        Kross::Api::Object::Ptr sheetByName(Kross::Api::List::Ptr);
	/**
	 * This function returns an array with the sheet names
	 * Example (in Ruby) : TODO
	 */
        Kross::Api::Object::Ptr sheetNames(Kross::Api::List::Ptr);
    private:
	KSpread::Doc* m_doc;
};
}
}


#endif
