/* This file is part of the KDE project
   Copyright 2004 Ariya Hidayat <ariya@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "tester.h"
#include "stylecluster_tester.h"
#include "stylecluster.h"

#include "kspread_doc.h"
#include "kspread_style_manager.h"
#include "kspread_style.h"
#include "kspread_sheet.h"

#include <kspread_value.h>



#define CHECK_STYLE(x,y)  check(__FILE__,__LINE__,#x,x,y)


using namespace KSpread;

StyleClusterTester::StyleClusterTester(KSpreadSheet *sheet): Tester()
{
  m_sheet = sheet;
}

QString StyleClusterTester::name()
{
  return QString("Test Cell Styles");
}

template<typename T>
void StyleClusterTester::check( const char *file, int line, const char* msg, const T &result, 
  const T &expected )
{
  testCount++;
  if( &result != &expected )
  {
    QString message;
    QTextStream ts( &message, IO_WriteOnly );
    ts << msg;
    ts << "  Result:";
    ts << &result;
    ts << ", ";
    ts << "Expected:";
    ts << &expected;
    fail( file, line, message );
  }
}

void StyleClusterTester::run()
{
  testCount = 0;
  errorList.clear();

  StyleCluster stylecluster(m_sheet);
  CHECK_STYLE(stylecluster.lookup(0,0), static_cast< const KSpreadStyle& > (*(m_sheet->doc()->styleManager()->defaultStyle())));

}


