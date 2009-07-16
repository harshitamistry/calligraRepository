/* This file is part of the KDE project
   Copyright (C) 2009 Jeremias Epperlein <jeeree@web.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "FormulaElement.h"
#include "FormulaCursor.h"
#include "FormulaData.h"
#include "FormulaCommand.h"
#include "KoFormulaShape.h"


FormulaData::FormulaData(FormulaElement* element)
           : QObject()
{
    m_element=element;
}

FormulaData::~FormulaData() 
{
    if (m_element) {
        delete m_element;
    }
}

void FormulaData::notifyDataChange(FormulaCommand* command, bool undo)
{
    emit dataChanged(command,undo);
}

void FormulaData::setFormulaElement ( FormulaElement* element )
{
    if (m_element) {
        delete m_element;
    }
    m_element=element;
    notifyDataChange(0,false);
}

FormulaElement* FormulaData::formulaElement() const
{
    return m_element;
}


