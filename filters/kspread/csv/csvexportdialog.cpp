/* This file is part of the KDE project
   Copyright (C) 1999 David Faure <faure@kde.org>
   Copyright (C) 2004 Nicolas GOUTTE <goutte@kde.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <csvexportdialog.h>
#include <exportdialogui.h>

#include <kspread_map.h>
#include <kspread_sheet.h>

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qcursor.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qptrlist.h>
#include <qradiobutton.h>
#include <qtextstream.h>
#include <qtabwidget.h>
#include <qtextcodec.h>

#include <kapplication.h>
#include <klocale.h>
#include <kdebug.h>
#include <kcombobox.h>
#include <kmessagebox.h>
#include <kcharsets.h>

CSVExportDialog::CSVExportDialog( QWidget * parent )
  : KDialogBase( parent, 0, true, QString::null, Ok | Cancel, No, true ),
    m_dialog( new ExportDialogUI( this ) ),
    m_delimiter( "," ),
    m_textquote('"')
{
  kapp->restoreOverrideCursor();

  QStringList encodings;
  encodings << i18n( "Descriptive encoding name", "Recommended ( %1 )" ).arg( "UTF-8" );
  encodings << i18n( "Descriptive encoding name", "Locale ( %1 )" ).arg( QTextCodec::codecForLocale()->name() );
  encodings += KGlobal::charsets()->descriptiveEncodingNames();
  // Add a few non-standard encodings, which might be useful for text files
  const QString description(i18n("Descriptive encoding name","Other ( %1 )"));
  encodings << description.arg("Apple Roman"); // Apple
  encodings << description.arg("IBM 850") << description.arg("IBM 866"); // MS DOS
  encodings << description.arg("CP 1258"); // Windows

  m_dialog->comboBoxEncoding->insertStringList(encodings);

  setButtonBoxOrientation ( Vertical );

  setMainWidget(m_dialog);

  connect( m_dialog->m_delimiterBox, SIGNAL( clicked(int) ),
           this, SLOT( delimiterClicked( int ) ) );
  connect( m_dialog->m_delimiterEdit, SIGNAL( returnPressed() ),
           this, SLOT( returnPressed() ) );
  connect( m_dialog->m_delimiterEdit, SIGNAL( textChanged ( const QString & ) ),
           this, SLOT(textChanged ( const QString & ) ) );
  connect( m_dialog->m_comboQuote, SIGNAL( activated( const QString & ) ),
           this, SLOT( textquoteSelected( const QString & ) ) );
  connect( m_dialog->m_selectionOnly, SIGNAL( toggled( bool ) ),
           this, SLOT( selectionOnlyChanged( bool ) ) );
}

CSVExportDialog::~CSVExportDialog()
{
  kapp->setOverrideCursor(Qt::waitCursor);
}

void CSVExportDialog::fillTable( KSpreadMap * map )
{
  m_dialog->m_tableList->clear();
  QCheckListItem * item;

  QPtrListIterator<KSpreadTable> it( map->tableList() );
  for( ; it.current(); ++it )
  {
    item = new QCheckListItem( m_dialog->m_tableList,
                               it.current()->tableName(),
                               QCheckListItem::CheckBox );
    item->setOn(true);
    m_dialog->m_tableList->insertItem( item );
  }

  m_dialog->m_tableList->setSorting(1, true);
  m_dialog->m_tableList->sort();
  m_dialog->m_tableList->setSorting( -1 );
}

QChar CSVExportDialog::getDelimiter() const
{
  return m_delimiter[0];
}

QChar CSVExportDialog::getTextQuote() const
{
  return m_textquote;
}

bool CSVExportDialog::printAlwaysTableDelimiter() const
{
  return m_dialog->m_delimiterAboveAll->isChecked();
}

QString CSVExportDialog::getTableDelimiter() const
{
  return m_dialog->m_tableDelimiter->text();
}

bool CSVExportDialog::exportTable(QString const & tableName) const
{
  for (QListViewItem * item = m_dialog->m_tableList->firstChild(); item; item = item->nextSibling())
  {
    if (((QCheckListItem * ) item)->isOn())
    {
      if ( ((QCheckListItem * ) item)->text() == tableName )
        return true;
    }
  }
  return false;
}

void CSVExportDialog::slotOk()
{
  accept();
}

void CSVExportDialog::slotCancel()
{
  reject();
}

void CSVExportDialog::returnPressed()
{
  if ( m_dialog->m_delimiterBox->id( m_dialog->m_delimiterBox->selected() ) != 4 )
    return;

  m_delimiter = m_dialog->m_delimiterEdit->text();
}

void CSVExportDialog::textChanged ( const QString & )
{
  m_dialog->m_radioOther->setChecked ( true );
  delimiterClicked(4); // other
}

void CSVExportDialog::delimiterClicked( int id )
{
  switch (id)
  {
    case 0: // comma
     m_delimiter = ",";
     break;
   case 1: // semicolon
    m_delimiter = ";";
    break;
   case 2: // tab
    m_delimiter = "\t";
    break;
   case 3: // space
    m_delimiter = " ";
    break;
   case 4: // other
    m_delimiter = m_dialog->m_delimiterEdit->text();
    break;
  }
}

void CSVExportDialog::textquoteSelected( const QString & mark )
{
  m_textquote = mark[0];
}

void CSVExportDialog::selectionOnlyChanged( bool on )
{
  m_dialog->m_tableList->setEnabled( !on );
  m_dialog->m_delimiterLineBox->setEnabled( !on );

  if ( on )
    m_dialog->m_tabWidget->setCurrentPage( 1 );
}

bool CSVExportDialog::exportSelectionOnly() const
{
  return m_dialog->m_selectionOnly->isChecked();
}

QTextCodec* CSVExportDialog::getCodec(void) const
{
    const QString strCodec( KGlobal::charsets()->encodingForName( m_dialog->comboBoxEncoding->currentText() ) );
    kdDebug(30502) << "Encoding: " << strCodec << endl;

    bool ok = false;
    QTextCodec* codec = QTextCodec::codecForName( strCodec.utf8() );

    // If QTextCodec has not found a valid encoding, so try with KCharsets.
    if ( codec )
    {
        ok = true;
    }
    else
    {
        codec = KGlobal::charsets()->codecForName( strCodec, ok );
    }

    // Still nothing?
    if ( !codec || !ok )
    {
        // Default: UTF-8
        kdWarning(30502) << "Cannot find encoding:" << strCodec << endl;
        // ### TODO: what parent to use?
        KMessageBox::error( 0, i18n("Cannot find encoding: %1").arg( strCodec ) );
        return 0;
    }

    return codec;
}

QString CSVExportDialog::getEndOfLine(void) const
{
    QString strReturn;
    if (m_dialog->radioEndOfLineLF==m_dialog->buttonGroupEndOfLine->selected())
        strReturn="\n";
    else if (m_dialog->radioEndOfLineCRLF==m_dialog->buttonGroupEndOfLine->selected())
        strReturn="\r\n";
    else if (m_dialog->radioEndOfLineCR==m_dialog->buttonGroupEndOfLine->selected())
        strReturn="\r";
    else
        strReturn="\n";

    return strReturn;
}

#include "csvexportdialog.moc"

