/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>
                 2001, 2002 Sven Leiber         <s.leiber@web.de>

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

#include "koAutoFormatDia.h"
#include "koAutoFormatDia.moc"
#include "koAutoFormat.h"
#include "koCharSelectDia.h"

#include <klocale.h>
#include <kmessagebox.h>
#include <klistview.h>
#include <kstandarddirs.h>

#include <qlayout.h>
#include <qwhatsthis.h>
#include <qvbox.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <kdebug.h>
#include <knuminput.h>
#include <kcompletion.h>
#include <kconfig.h>
#include <klineeditdlg.h>
#include <qcombobox.h>
#include <qdir.h>
#include <koSearchDia.h>

KoAutoFormatLineEdit::KoAutoFormatLineEdit ( QWidget * parent, const char * name )
    : QLineEdit(parent,name)
{
}

void KoAutoFormatLineEdit::keyPressEvent ( QKeyEvent *ke )
{
    if( ke->key()  == QKeyEvent::Key_Return ||
        ke->key()  == QKeyEvent::Key_Enter )
    {
        emit keyReturnPressed();
        return;
    }
    QLineEdit::keyPressEvent (ke);
}


/******************************************************************/
/* Class: KoAutoFormatExceptionWidget                             */
/******************************************************************/

KoAutoFormatExceptionWidget::KoAutoFormatExceptionWidget(QWidget *parent, const QString &name,const QStringList &_list, bool _autoInclude, bool _abreviation)
    :QWidget( parent )
{
    m_bAbbreviation=_abreviation;
    m_listException=_list;
    QGridLayout *grid = new QGridLayout(this, 4, 2, 0, KDialog::spacingHint());

    QLabel *lab=new QLabel(name,this);
    grid->addMultiCellWidget(lab,0,0,0,1);

    exceptionLine = new KoAutoFormatLineEdit( this );
    grid->addWidget(exceptionLine,1,0);

    connect(exceptionLine,SIGNAL(keyReturnPressed()),SLOT(slotAddException()));
    connect(exceptionLine ,SIGNAL(textChanged ( const QString & )),
            SLOT(textChanged ( const QString & )));

    pbAddException=new QPushButton(i18n("Add"),this);
    connect(pbAddException, SIGNAL(clicked()),SLOT(slotAddException()));
    grid->addWidget(pbAddException,1,1);

    pbAddException->setEnabled(false);

    pbRemoveException=new QPushButton(i18n("Remove"),this);
    connect(pbRemoveException, SIGNAL(clicked()),SLOT(slotRemoveException()));
    grid->addWidget(pbRemoveException,2,1,Qt::AlignTop);

    exceptionList=new QListBox(this);
    exceptionList->insertStringList(m_listException);
    grid->addWidget(exceptionList,2,0);

    grid->setRowStretch( 2, 1 );

    connect( exceptionList , SIGNAL(selectionChanged () ),
            this,SLOT(slotExceptionListSelected()) );

    pbRemoveException->setEnabled( exceptionList->currentItem()!=-1);

    cbAutoInclude = new QCheckBox( i18n("Autoinclude"), this );
    grid->addWidget(cbAutoInclude,3,0);
    cbAutoInclude->setChecked( _autoInclude );
}


void KoAutoFormatExceptionWidget::textChanged ( const QString &_text )
{
    pbAddException->setEnabled(!_text.isEmpty());
}

void KoAutoFormatExceptionWidget::slotAddException()
{
    QString text=exceptionLine->text().stripWhiteSpace();
    if(!text.isEmpty())
    {
        if(text.at(text.length()-1)!='.' && m_bAbbreviation)
            text=text+".";
        if( m_listException.findIndex( text )==-1)
        {
            m_listException<<text;

            exceptionList->clear();
            exceptionList->insertStringList(m_listException);
            pbRemoveException->setEnabled( exceptionList->currentItem()!=-1);
            pbAddException->setEnabled(false);
        }
        exceptionLine->clear();
    }
}

void KoAutoFormatExceptionWidget::slotRemoveException()
{
    if(!exceptionList->currentText().isEmpty())
    {
        m_listException.remove(exceptionList->currentText());
        exceptionList->clear();
        pbAddException->setEnabled(false);
        pbRemoveException->setEnabled( exceptionList->currentItem()!=-1);
        exceptionList->insertStringList(m_listException);
        exceptionLine->clear();
    }
}

bool KoAutoFormatExceptionWidget::autoInclude()
{
    return cbAutoInclude->isChecked();
}

void KoAutoFormatExceptionWidget::setListException( const QStringList &list)
{
    exceptionList->clear();
    exceptionList->insertStringList(list);
}

void KoAutoFormatExceptionWidget::setAutoInclude(bool b)
{
    cbAutoInclude->setChecked( b );
}

void KoAutoFormatExceptionWidget::slotExceptionListSelected()
{
    pbRemoveException->setEnabled( exceptionList->currentItem()!=-1 );
}

/******************************************************************/
/* Class: KoAutoFormatDia                                         */
/******************************************************************/

KoAutoFormatDia::KoAutoFormatDia( QWidget *parent, const char *name,
      KoAutoFormat * autoFormat )
    : KDialogBase( Tabbed, i18n("Autocorrection"), Ok | Cancel | User1, Ok,
      parent, name, true, true, KGuiItem( i18n( "&Reset" ), "undo" )),
      oSimpleBegin( autoFormat->getConfigTypographicSimpleQuotes().begin ),
      oSimpleEnd( autoFormat->getConfigTypographicSimpleQuotes().end ),
      oDoubleBegin( autoFormat->getConfigTypographicDoubleQuotes().begin ),
      oDoubleEnd( autoFormat->getConfigTypographicDoubleQuotes().end ),
      bulletStyle( autoFormat->getConfigBulletStyle()),
      m_autoFormat( *autoFormat ),
      m_docAutoFormat( autoFormat )
{
    noSignal=true;
    setupTab1();
    setupTab2();
    setupTab3();
    setupTab4();
    setInitialSize( QSize(500, 300) );
    connect( this, SIGNAL( user1Clicked() ), this, SLOT(slotResetConf()));
    noSignal=false;
    changeLanguage = false;
}

void KoAutoFormatDia::slotResetConf()
{
    switch( activePageIndex() ) {
    case 0:
        initTab1();
        break;
    case 1:
        initTab2();
        break;
    case 2:
        initTab3();
        break;
    case 3:
        initTab4();
        break;
    default:
        break;
    }
}

void KoAutoFormatDia::setupTab1()
{
    tab1 = addPage( i18n( "Simple Autocorrection" ) );
    QVBoxLayout *vbox = new QVBoxLayout(tab1, KDialog::marginHint(),
            KDialog::spacingHint());

    cbUpperCase = new QCheckBox( tab1 );
    cbUpperCase->setText( i18n(
            "Convert &first letter of a sentence automatically to uppercase\n"
            "(e.g. \"my house. in this town\" to \"my house. In this town\")"
            ) );
    QWhatsThis::add( cbUpperCase, i18n(
            "Detect when a new sentence is started and always ensure that"
            " the first character is an uppercase character."));

    vbox->addWidget(cbUpperCase);


    cbUpperUpper = new QCheckBox( tab1 );
    cbUpperUpper->setText( i18n(
            "Convert &two uppercase characters to one uppercase and one"
            " lowercase character\n (e.g. PErfect to Perfect)" ) );
    QWhatsThis::add( cbUpperUpper, i18n(
            "All words are checked for the common mistake of holding the "
            "shift key down a bit too long. If some words must have two "
            "uppercase characters, then those exceptions should be added in "
            "the Exceptions tab."));

    vbox->addWidget(cbUpperUpper);

    cbDetectUrl=new QCheckBox( tab1 );
    cbDetectUrl->setText( i18n( "Autoformat &URLs" ) );
    QWhatsThis::add( cbDetectUrl, i18n(
            "Detect when a URL (Universal Remote Location) is typed and "
            "provide formatting that matches the way an Internet browser "
            "would show a URL."));

    vbox->addWidget(cbDetectUrl);

    cbIgnoreDoubleSpace=new QCheckBox( tab1 );
    cbIgnoreDoubleSpace->setText( i18n( "&Suppress double spaces" ) );
    QWhatsThis::add( cbIgnoreDoubleSpace, i18n(
            "Make sure that more than one space cannot be typed, as this is a "
            "common mistake which is quite hard to find in formatted text."));

    vbox->addWidget(cbIgnoreDoubleSpace);

    cbRemoveSpaceBeginEndLine=new QCheckBox( tab1 );
    cbRemoveSpaceBeginEndLine->setText( i18n(
            "R&emove spaces at the beginning and end of paragraphs" ) );
    QWhatsThis::add( cbRemoveSpaceBeginEndLine, i18n(
            "Keep correct formatting and indenting of sentences by "
            "automatically removing spaces typed at the beginning and end of "
            "a paragraph."));

    vbox->addWidget(cbRemoveSpaceBeginEndLine);

    cbAutoChangeFormat=new QCheckBox( tab1 );
    cbAutoChangeFormat->setText( i18n(
            "Automatically do &bold and underline formatting") );
    QWhatsThis::add( cbAutoChangeFormat, i18n(
            "When you use _underline_ or *bold*, the text between the "
            "underscores or asterisks will be converted to underlined or "
            "bold text.") );

    vbox->addWidget(cbAutoChangeFormat);

    cbAutoReplaceNumber=new QCheckBox( tab1 );
    cbAutoReplaceNumber->setText( i18n(
            "We add the 1/2 char at the %1", "Re&place 1/2... with %1..." )
            .arg( QString( "�" ) ) );
    QWhatsThis::add( cbAutoReplaceNumber, i18n(
            "Most standard fraction notations will be converted when available"
            ) );

    vbox->addWidget(cbAutoReplaceNumber);

    cbUseNumberStyle=new QCheckBox( tab1 );
    cbUseNumberStyle->setText( i18n(
            "Use &autonumbering for numbered paragraphs" ) );
    QWhatsThis::add( cbUseNumberStyle, i18n(
            "When typing '1)' or similar in front of a paragraph, "
            "automatically convert the paragraph to use that numbering style. "
            "This has the advantage that further paragraphs will also be "
            "numbered and the spacing is done correctly.") );

    vbox->addWidget(cbUseNumberStyle);

    cbAutoSuperScript = new QCheckBox( tab1 );
    cbAutoSuperScript->setText( i18n("Rep&lace 1st... with 1^st..."));
    cbAutoSuperScript->setEnabled( m_autoFormat.nbSuperScriptEntry()>0 );

    vbox->addWidget(cbAutoSuperScript);

    cbUseBulletStyle=new QCheckBox( tab1 );
    cbUseBulletStyle->setText( i18n(
            "Use l&ist-formatting for bulleted paragraphs" ) );
    QWhatsThis::add( cbUseBulletStyle, i18n(
            "When typing '*' or '-' in front of a paragraph, automatically "
            "convert the paragraph to use that list-style. Using a list-style "
            "formatting means that a correct bullet is used to draw the list."
            ) );

    connect( cbUseBulletStyle, SIGNAL( toggled( bool ) ),
            SLOT( slotBulletStyleToggled( bool ) ) );

    vbox->addWidget(cbUseBulletStyle);

    QHBoxLayout *hbox = new QHBoxLayout( tab1 );

    hbox->addSpacing( 20 );

    pbBulletStyle = new QPushButton( tab1 );
    pbBulletStyle->setFixedSize( pbBulletStyle->sizeHint() );

    hbox->addWidget( pbBulletStyle );

    pbDefaultBulletStyle = new QPushButton( tab1 );
    pbDefaultBulletStyle->setText(i18n("Default"));
    pbDefaultBulletStyle->setFixedSize( pbDefaultBulletStyle->sizeHint() );

    hbox->addWidget( pbDefaultBulletStyle );

    hbox->addStretch( 1 );
    vbox->addItem(hbox);
    vbox->addStretch( 1 );

    initTab1();

    connect( pbBulletStyle, SIGNAL( clicked() ), SLOT( chooseBulletStyle() ) );
    connect( pbDefaultBulletStyle, SIGNAL( clicked()),
            SLOT( defaultBulletStyle() ) );
}

void KoAutoFormatDia::initTab1()
{
    cbUpperCase->setChecked( m_autoFormat.getConfigUpperCase() );
    cbUpperUpper->setChecked( m_autoFormat.getConfigUpperUpper() );
    cbDetectUrl->setChecked( m_autoFormat.getConfigAutoDetectUrl());
    cbIgnoreDoubleSpace->setChecked( m_autoFormat.getConfigIgnoreDoubleSpace());
    cbRemoveSpaceBeginEndLine->setChecked( m_autoFormat.getConfigRemoveSpaceBeginEndLine());
    cbAutoChangeFormat->setChecked( m_autoFormat.getConfigAutoChangeFormat());
    cbAutoReplaceNumber->setChecked( m_autoFormat.getConfigAutoReplaceNumber());
    cbUseNumberStyle->setChecked( m_autoFormat.getConfigAutoNumberStyle());
    cbUseBulletStyle->setChecked( m_autoFormat.getConfigUseBulletSyle());
    cbAutoSuperScript->setChecked( m_autoFormat.getConfigAutoSuperScript());
    pbBulletStyle->setText( bulletStyle );

    slotBulletStyleToggled( cbUseBulletStyle->isChecked() );
}

void KoAutoFormatDia::slotBulletStyleToggled( bool b )
{
    pbBulletStyle->setEnabled( b );
    pbDefaultBulletStyle->setEnabled( b );
}

void KoAutoFormatDia::setupTab2()
{
    tab2 = addPage( i18n( "Custom Quotes" ) );

    QVBoxLayout *vbox = new QVBoxLayout(tab2, KDialog::marginHint(),
            KDialog::spacingHint());

    cbTypographicDoubleQuotes = new QCheckBox( tab2 );
    cbTypographicDoubleQuotes->setText( i18n(
            "Replace &double quotes with typographical quotes" ) );

    connect( cbTypographicDoubleQuotes,SIGNAL(toggled ( bool)),
            SLOT(slotChangeStateDouble(bool)));

    vbox->addWidget( cbTypographicDoubleQuotes );

    QHBoxLayout *hbox = new QHBoxLayout( tab2 );
    hbox->addSpacing( 20 );

    pbDoubleQuote1 = new QPushButton( tab2 );
    pbDoubleQuote1->setFixedSize( pbDoubleQuote1->sizeHint() );
    hbox->addWidget( pbDoubleQuote1 );

    pbDoubleQuote2 = new QPushButton( tab2 );
    pbDoubleQuote2->setFixedSize( pbDoubleQuote2->sizeHint() );
    hbox->addWidget( pbDoubleQuote2 );

    hbox->addSpacing( 20 );

    pbDoubleDefault = new QPushButton( tab2 );
    pbDoubleDefault->setText(i18n("Default"));
    pbDoubleDefault->setFixedSize( pbDoubleDefault->sizeHint() );
    hbox->addWidget( pbDoubleDefault );

    hbox->addStretch( 1 );

    connect(pbDoubleQuote1, SIGNAL( clicked() ), SLOT( chooseDoubleQuote1() ));
    connect(pbDoubleQuote2, SIGNAL( clicked() ), SLOT( chooseDoubleQuote2() ));
    connect(pbDoubleDefault, SIGNAL( clicked()), SLOT( defaultDoubleQuote() ));

    vbox->addItem( hbox );

    cbTypographicSimpleQuotes = new QCheckBox( tab2 );
    cbTypographicSimpleQuotes->setText( i18n(
            "Replace &single quotes with typographical quotes" ) );

    connect( cbTypographicSimpleQuotes,SIGNAL(toggled ( bool)),
            SLOT(slotChangeStateSimple(bool)));

    vbox->addWidget( cbTypographicSimpleQuotes );

    hbox = new QHBoxLayout( tab2 );
    hbox->addSpacing( 20 );

    pbSimpleQuote1 = new QPushButton( tab2 );
    pbSimpleQuote1->setFixedSize( pbSimpleQuote1->sizeHint() );
    hbox->addWidget( pbSimpleQuote1 );

    pbSimpleQuote2 = new QPushButton( tab2 );
    pbSimpleQuote2->setFixedSize( pbSimpleQuote2->sizeHint() );
    hbox->addWidget( pbSimpleQuote2 );

    hbox->addSpacing( 20 );

    pbSimpleDefault = new QPushButton( tab2 );
    pbSimpleDefault->setText(i18n("Default"));
    pbSimpleDefault->setFixedSize( pbSimpleDefault->sizeHint() );
    hbox->addWidget( pbSimpleDefault );

    hbox->addStretch( 1 );

    connect(pbSimpleQuote1, SIGNAL( clicked() ), SLOT( chooseSimpleQuote1() ));
    connect(pbSimpleQuote2, SIGNAL( clicked() ), SLOT( chooseSimpleQuote2() ));
    connect(pbSimpleDefault, SIGNAL( clicked()), SLOT( defaultSimpleQuote() ));

    vbox->addItem( hbox );
    vbox->addStretch( 1 );

    initTab2();
}

void KoAutoFormatDia::initTab2()
{
    bool state=m_autoFormat.getConfigTypographicDoubleQuotes().replace;
    cbTypographicDoubleQuotes->setChecked( state );
    pbDoubleQuote1->setText( oDoubleBegin );
    pbDoubleQuote2->setText(oDoubleEnd );
    slotChangeStateDouble(state);

    state=m_autoFormat.getConfigTypographicSimpleQuotes().replace;
    cbTypographicSimpleQuotes->setChecked( state );
    pbSimpleQuote1->setText( oSimpleBegin );
    pbSimpleQuote2->setText(oSimpleEnd );
    slotChangeStateSimple(state);

}

void KoAutoFormatDia::setupTab3()
{
    tab3 = addPage( i18n( "Advanced Autocorrection" ) );

    QLabel *lblFind, *lblReplace;

    QGridLayout *grid = new QGridLayout( tab3, 3, 7, KDialog::marginHint(),
            KDialog::spacingHint() );

    autoFormatLanguage = new QComboBox(tab3);

    QStringList lst;
    lst<<i18n("Default");
    KStandardDirs *standard = new KStandardDirs();
    QStringList tmp = standard->findDirs("data", "koffice/autocorrect/");
    QString path = *(tmp.end());
    for ( QStringList::Iterator it = tmp.begin(); it != tmp.end(); ++it )
    {
        path =*it;
    }
    delete standard;
    QDir dir( path);
    tmp =dir.entryList (QDir::Files);
    for ( QStringList::Iterator it = tmp.begin(); it != tmp.end(); ++it )
    {
        if ( !(*it).contains("autocorrect"))
            lst<<(*it).left((*it).length()-4);
    }
    autoFormatLanguage->insertStringList(lst);

    connect(autoFormatLanguage, SIGNAL(highlighted ( const QString & )), this, SLOT(changeAutoformatLanguage(const QString & )));

    grid->addMultiCellWidget( autoFormatLanguage, 0, 0, 4, 6 );
    QLabel *lblAutoFormatLanguage = new QLabel( i18n("Remplacement and exeption for language"), tab3);
    grid->addMultiCellWidget( lblAutoFormatLanguage, 0, 0, 0, 3 );

    cbAdvancedAutoCorrection = new QCheckBox( tab3 );
    cbAdvancedAutoCorrection->setText( i18n("Enable autocorrection") );

    grid->addMultiCellWidget( cbAdvancedAutoCorrection, 1, 1, 0, 6 );

    cbAutoCorrectionWithFormat = new QCheckBox( tab3 );
    cbAutoCorrectionWithFormat->setText( i18n("Replace text with format") );
    grid->addMultiCellWidget( cbAutoCorrectionWithFormat, 2, 2, 0, 6 );

    lblFind = new QLabel( i18n( "&Find:" ), tab3 );
    grid->addWidget( lblFind, 3, 0 );

    m_find = new KoAutoFormatLineEdit( tab3 );
    grid->addWidget( m_find, 3, 1 );

    lblFind->setBuddy( m_find );

    connect( m_find, SIGNAL( textChanged( const QString & ) ),
	     SLOT( slotfind( const QString & ) ) );
    connect( m_find, SIGNAL( keyReturnPressed() ),
             SLOT( slotAddEntry()));

    pbSpecialChar1 = new QPushButton( "...", tab3 );
    pbSpecialChar1->setFixedWidth( 40 );
    grid->addWidget( pbSpecialChar1, 3, 2 );

    connect(pbSpecialChar1,SIGNAL(clicked()), SLOT(chooseSpecialChar1()));

    lblReplace = new QLabel( i18n( "&Replace:" ), tab3 );
    grid->addWidget( lblReplace, 3, 3 );

    m_replace = new KoAutoFormatLineEdit( tab3 );
    grid->addWidget( m_replace, 3, 4 );

    lblReplace->setBuddy( m_replace );

    connect( m_replace, SIGNAL( textChanged( const QString & ) ),
	        SLOT( slotfind2( const QString & ) ) );
    connect( m_replace, SIGNAL( keyReturnPressed() ),
            SLOT( slotAddEntry()));

    pbSpecialChar2 = new QPushButton( "...", tab3 );
    pbSpecialChar2->setFixedWidth( 40 );
    grid->addWidget( pbSpecialChar2, 3, 5 );

    connect(pbSpecialChar2,SIGNAL(clicked()), SLOT(chooseSpecialChar2()));

    pbAdd = new QPushButton( i18n( "&Add"), tab3  );
    grid->addWidget( pbAdd, 3, 6 );

    connect(pbAdd,SIGNAL(clicked()),this, SLOT(slotAddEntry()));

    m_pListView = new KListView( tab3 );
    m_pListView->addColumn( i18n( "Find" ) );
    m_pListView->addColumn( i18n( "Replace" ) );
    m_pListView->setAllColumnsShowFocus( true );
    grid->addMultiCellWidget( m_pListView, 5, 5, 0, 5 );

    connect(m_pListView, SIGNAL(doubleClicked ( QListViewItem * )),
             SLOT(slotEditEntry()) );
    connect(m_pListView, SIGNAL(clicked ( QListViewItem * ) ),
             SLOT(slotEditEntry()) );

    pbRemove = new QPushButton( i18n( "Remove" ), tab3 );
    grid->addWidget( pbRemove, 4, 6, Qt::AlignTop );

    connect(pbRemove,SIGNAL(clicked()), SLOT(slotRemoveEntry()));

    pbChangeFormat= new QPushButton( i18n( "Change Format" ), tab3 );
    grid->addWidget( pbChangeFormat, 5, 6, Qt::AlignTop );

    connect( pbChangeFormat, SIGNAL(clicked()), SLOT(slotChangeTextFormatEntry()));
    grid->setRowStretch( 2, 1 );

    pbRemove->setEnabled(false);
    pbChangeFormat->setEnabled( false );
    pbAdd->setEnabled(false);

    initTab3();
}

void KoAutoFormatDia::initTab3()
{
    cbAdvancedAutoCorrection->setChecked(m_autoFormat.getConfigAdvancedAutoCorrect());
    cbAutoCorrectionWithFormat->setChecked( m_autoFormat.getConfigCorrectionWithFormat());
    m_pListView->clear();

    QDictIterator<KoAutoFormatEntry> it( m_autoFormat.getAutoFormatEntries());
    for( ; it.current(); ++it )
    {
        ( void )new QListViewItem( m_pListView, it.currentKey(), it.current()->replace() );
    }
    if ( !changeLanguage || noSignal)
    {
        if ( m_autoFormat.getConfigAutoFormatLanguage( ).isEmpty() )
            autoFormatLanguage->setCurrentItem(0);
        else
            autoFormatLanguage->setCurrentText(m_autoFormat.getConfigAutoFormatLanguage( ));
    }
}

void KoAutoFormatDia::changeAutoformatLanguage(const QString & text)
{
    if ( text==i18n("Default"))
        m_autoFormat.configAutoFormatLanguage( QString::null);
    else
        m_autoFormat.configAutoFormatLanguage( text);

    if ( !noSignal )
    {
        changeLanguage=true;
        m_autoFormat.readConfig( true );
        initTab3();
        initTab4();
        changeLanguage=false;
    }
}

void KoAutoFormatDia::setupTab4()
{
    tab4 = addPage( i18n( "Exceptions" ) );
    QVBoxLayout *vbox = new QVBoxLayout(tab4, KDialog::marginHint(),
            KDialog::spacingHint());

    abbreviation=new KoAutoFormatExceptionWidget(tab4,
            i18n("Do not treat as the end of a sentence:"),
            m_autoFormat.listException(),
            m_autoFormat.getConfigIncludeAbbreviation() , true);

    vbox->addWidget( abbreviation );

    twoUpperLetter=new KoAutoFormatExceptionWidget(tab4,
            i18n("Accept two uppercase letters in:"),
            m_autoFormat.listTwoUpperLetterException(),
            m_autoFormat.getConfigIncludeTwoUpperUpperLetterException());

    vbox->addWidget( twoUpperLetter );

    initTab4();
}

void KoAutoFormatDia::initTab4()
{
    abbreviation->setListException( changeLanguage ? m_autoFormat.listException(): m_docAutoFormat->listException() );
    if ( !changeLanguage )
    {
        abbreviation->setAutoInclude( m_docAutoFormat->getConfigIncludeAbbreviation() );
        twoUpperLetter->setAutoInclude( m_docAutoFormat->getConfigIncludeTwoUpperUpperLetterException() );
    }
    twoUpperLetter->setListException( changeLanguage ? m_autoFormat.listException():m_docAutoFormat->listTwoUpperLetterException() );
}

void KoAutoFormatDia::slotChangeTextFormatEntry()
{
    if ( m_pListView->currentItem() )
    {
        KoAutoFormatEntry *entry = m_autoFormat.findFormatEntry(m_pListView->currentItem()->text(0));
        KoSearchContext *tmpFormat = entry->formatEntryContext();
        bool createNewFormat = false;

        if ( !tmpFormat )
        {
            tmpFormat = new KoSearchContext();
            createNewFormat = true;
        }

        KoFormatDia *dia = new KoFormatDia( this, i18n("Change Text Format"), tmpFormat ,  0L);
        if ( dia->exec())
        {
            dia->ctxOptions( );
            entry->setFormatEntryContext( tmpFormat );

        }
        else
        {
            if ( createNewFormat )
                delete tmpFormat;
        }
        delete dia;
    }
}

void KoAutoFormatDia::slotRemoveEntry()
{
    //find entry in listbox
   if(m_pListView->currentItem())
    {
        m_autoFormat.removeAutoFormatEntry(m_pListView->currentItem()->text(0));
        pbAdd->setText(i18n("&Add"));
        refreshEntryList();
    }
}


void KoAutoFormatDia::slotfind( const QString & )
{
    KoAutoFormatEntry *entry = m_autoFormat.findFormatEntry(m_find->text());
    if ( entry )
    {
        m_replace->setText(entry->replace().latin1());
        pbAdd->setText(i18n("&Modify"));
        m_pListView->setCurrentItem(m_pListView->findItem(m_find->text(),0));

    } else {
        m_replace->clear();
        pbAdd->setText(i18n("&Add"));
        m_pListView->setCurrentItem(0L);
    }
    slotfind2("");
}


void KoAutoFormatDia::slotfind2( const QString & )
{
    bool state = !m_replace->text().isEmpty() && !m_find->text().isEmpty();
    KoAutoFormatEntry * entry=m_autoFormat.findFormatEntry(m_find->text());
    pbRemove->setEnabled(state && entry);
    pbChangeFormat->setEnabled(state && entry);
    pbAdd->setEnabled(state);
}


void KoAutoFormatDia::refreshEntryList()
{
    m_pListView->clear();
    QDictIterator<KoAutoFormatEntry> it( m_autoFormat.getAutoFormatEntries());
    for( ; it.current(); ++it )
    {
        ( void )new QListViewItem( m_pListView, it.currentKey(), it.current()->replace() );
    }
    m_pListView->setCurrentItem(m_pListView->firstChild ());
    bool state = !(m_replace->text().isEmpty()) && !(m_find->text().isEmpty());
    //we can delete item, as we search now in listbox and not in m_find lineedit
    pbRemove->setEnabled(m_pListView->currentItem() && m_pListView->selectedItem()!=0 );
    pbChangeFormat->setEnabled(m_pListView->currentItem() && m_pListView->selectedItem()!=0 );
    pbAdd->setEnabled(state);
}


void KoAutoFormatDia::addEntryList(const QString &key, KoAutoFormatEntry *_autoEntry)
{
    m_autoFormat.addAutoFormatEntry( key, _autoEntry );
}



void KoAutoFormatDia::editEntryList(const QString &key,const QString &newFindString, KoAutoFormatEntry *_autoEntry)
{
    m_autoFormat.removeAutoFormatEntry( key );
    m_autoFormat.addAutoFormatEntry( newFindString, _autoEntry );
}


void KoAutoFormatDia::slotAddEntry()
{
    if(!pbAdd->isEnabled())
        return;
    QString repl = m_replace->text();
    QString find = m_find->text();
    if(repl.isEmpty() || find.isEmpty())
    {
        KMessageBox::sorry( 0L, i18n( "An area is empty" ) );
        return;
    }
    if(repl==find)
    {
        KMessageBox::sorry( 0L, i18n( "Find string is the same as replace string!" ) );
	return;
    }
    KoAutoFormatEntry *tmp = new KoAutoFormatEntry( repl );

    if(pbAdd->text() == i18n( "&Add" ))
        addEntryList(find, tmp);
    else
        editEntryList(find, find, tmp);

    m_find->clear();
    m_replace->clear();

    refreshEntryList();
}


void KoAutoFormatDia::chooseSpecialChar1()
{
    QString f = font().family();
    QChar c = ' ';
    if ( KoCharSelectDia::selectChar( f, c, false ) )
        m_find->setText( c );
}


void KoAutoFormatDia::chooseSpecialChar2()
{
    QString f = font().family();
    QChar c = ' ';
    if ( KoCharSelectDia::selectChar( f, c, false ) )
        m_replace->setText( c );
}


void KoAutoFormatDia::slotItemRenamed(QListViewItem *, const QString & , int )
{
    // Wow. This need a redesign (we don't have the old key anymore at this point !)
    // -> inherit QListViewItem and store the KoAutoFormatEntry pointer in it.
}


void KoAutoFormatDia::slotEditEntry()
{
    if(m_pListView->currentItem()==0)
        return;
    m_find->setText(m_pListView->currentItem()->text(0));
    m_replace->setText(m_pListView->currentItem()->text(1));
    bool state = !m_replace->text().isEmpty() && !m_find->text().isEmpty();
    pbRemove->setEnabled(state);
    pbChangeFormat->setEnabled( state );
    pbAdd->setEnabled(state);
}


bool KoAutoFormatDia::applyConfig()
{
    // First tab
    KoAutoFormat::TypographicQuotes tq = m_autoFormat.getConfigTypographicSimpleQuotes();
    tq.replace = cbTypographicSimpleQuotes->isChecked();
    tq.begin = pbSimpleQuote1->text()[ 0 ];
    tq.end = pbSimpleQuote2->text()[ 0 ];
    m_docAutoFormat->configTypographicSimpleQuotes( tq );

    tq = m_autoFormat.getConfigTypographicDoubleQuotes();
    tq.replace = cbTypographicDoubleQuotes->isChecked();
    tq.begin = pbDoubleQuote1->text()[ 0 ];
    tq.end = pbDoubleQuote2->text()[ 0 ];
    m_docAutoFormat->configTypographicDoubleQuotes( tq );


    m_docAutoFormat->configUpperCase( cbUpperCase->isChecked() );
    m_docAutoFormat->configUpperUpper( cbUpperUpper->isChecked() );
    m_docAutoFormat->configAutoDetectUrl( cbDetectUrl->isChecked() );

    m_docAutoFormat->configIgnoreDoubleSpace( cbIgnoreDoubleSpace->isChecked());
    m_docAutoFormat->configRemoveSpaceBeginEndLine( cbRemoveSpaceBeginEndLine->isChecked());
    m_docAutoFormat->configUseBulletStyle(cbUseBulletStyle->isChecked());

    m_docAutoFormat->configBulletStyle(pbBulletStyle->text()[ 0 ]);

    m_docAutoFormat->configAutoChangeFormat( cbAutoChangeFormat->isChecked());

    m_docAutoFormat->configAutoReplaceNumber( cbAutoReplaceNumber->isChecked());
    m_docAutoFormat->configAutoNumberStyle(cbUseNumberStyle->isChecked());

    m_docAutoFormat->configAutoSuperScript ( cbAutoSuperScript->isChecked() );

    // Second tab
    m_docAutoFormat->copyAutoFormatEntries( m_autoFormat );
    m_docAutoFormat->copyListException(abbreviation->getListException());
    m_docAutoFormat->copyListTwoUpperCaseException(twoUpperLetter->getListException());
    m_docAutoFormat->configAdvancedAutocorrect( cbAdvancedAutoCorrection->isChecked() );
    m_docAutoFormat->configCorrectionWithFormat( cbAutoCorrectionWithFormat->isChecked());

    m_docAutoFormat->configIncludeTwoUpperUpperLetterException( twoUpperLetter->autoInclude());
    m_docAutoFormat->configIncludeAbbreviation( abbreviation->autoInclude());

    QString lang = autoFormatLanguage->currentText();
    if ( lang == i18n("Default") )
        m_docAutoFormat->configAutoFormatLanguage(QString::null);
    else
        m_docAutoFormat->configAutoFormatLanguage(lang);

    // Save to config file
    m_docAutoFormat->saveConfig();

    return true;
}

void KoAutoFormatDia::slotOk()
{
    if (applyConfig())
    {
       KDialogBase::slotOk();
    }
}

void KoAutoFormatDia::chooseDoubleQuote1()
{
    QString f = font().family();
    QChar c = oDoubleBegin;
    if ( KoCharSelectDia::selectChar( f, c, false ) )
    {
        pbDoubleQuote1->setText( c );
    }
}

void KoAutoFormatDia::chooseDoubleQuote2()
{
    QString f = font().family();
    QChar c = oDoubleEnd;
    if ( KoCharSelectDia::selectChar( f, c, false ) )
    {
        pbDoubleQuote2->setText( c );
    }
}


void KoAutoFormatDia::defaultDoubleQuote()
{
    pbDoubleQuote1->setText(m_docAutoFormat->getDefaultTypographicDoubleQuotes().begin);
    pbDoubleQuote2->setText(m_docAutoFormat->getDefaultTypographicDoubleQuotes().end);
}

void KoAutoFormatDia::chooseSimpleQuote1()
{
    QString f = font().family();
    QChar c = oSimpleBegin;
    if ( KoCharSelectDia::selectChar( f, c, false ) )
    {
        pbSimpleQuote1->setText( c );
    }
}

void KoAutoFormatDia::chooseSimpleQuote2()
{
    QString f = font().family();
    QChar c = oSimpleEnd;
    if ( KoCharSelectDia::selectChar( f, c, false ) )
    {
        pbSimpleQuote2->setText( c );
    }
}

void KoAutoFormatDia::defaultSimpleQuote()
{

    pbSimpleQuote1->setText(m_docAutoFormat->getDefaultTypographicSimpleQuotes().begin);
    pbSimpleQuote2->setText(m_docAutoFormat->getDefaultTypographicSimpleQuotes().end);
}


void KoAutoFormatDia::chooseBulletStyle()
{
    QString f = font().family();
    QChar c = bulletStyle;
    if ( KoCharSelectDia::selectChar( f, c, false ) )
    {
        pbBulletStyle->setText( c );
    }
}

void KoAutoFormatDia::defaultBulletStyle()
{
    pbBulletStyle->setText( "" );
}

void KoAutoFormatDia::slotChangeStateSimple(bool b)
{
    pbSimpleQuote1->setEnabled(b);
    pbSimpleQuote2->setEnabled(b);
    pbSimpleDefault->setEnabled(b);
}

void KoAutoFormatDia::slotChangeStateDouble(bool b)
{
    pbDoubleQuote1->setEnabled(b);
    pbDoubleQuote2->setEnabled(b);
    pbDoubleDefault->setEnabled(b);
}


/******************************************************************/
/* Class: KoCompletionDia                                         */
/******************************************************************/

KoCompletionDia::KoCompletionDia( QWidget *parent, const char *name,
      KoAutoFormat * autoFormat )
    : KDialogBase( parent, name , true, i18n( "Completion" ), Ok|Cancel|User1,
      Ok, true, KGuiItem( i18n( "&Reset" ), "undo" ) ),
      m_autoFormat( *autoFormat ),
      m_docAutoFormat( autoFormat )
{
    setup();
    slotResetConf();
    setInitialSize( QSize( 500, 500 ) );
    connect( this, SIGNAL( user1Clicked() ), this, SLOT(slotResetConf()));
    changeButtonStatus();
}

void KoCompletionDia::changeButtonStatus()
{
    bool state = cbAllowCompletion->isChecked();
    cbAppendSpace->setEnabled( state );
    cbAddCompletionWord->setEnabled( state );
    pbRemoveCompletionEntry->setEnabled( state );
    pbSaveCompletionEntry->setEnabled( state );
    pbAddCompletionEntry->setEnabled( state );
    m_lbListCompletion->setEnabled( state );
    m_minWordLength->setEnabled( state );
    m_maxNbWordCompletion->setEnabled( state );

    state = state && (m_lbListCompletion->count()!=0 && !m_lbListCompletion->currentText().isEmpty());
    pbRemoveCompletionEntry->setEnabled( state );
}

void KoCompletionDia::setup()
{
    QVBox *page = makeVBoxMainWidget();
    cbAllowCompletion = new QCheckBox( page );
    cbAllowCompletion->setText( i18n( "E&nable completion" ) );
    connect(cbAllowCompletion, SIGNAL(toggled ( bool )), this, SLOT( changeButtonStatus()));
    // TODO whatsthis or text, to tell about the key to use for autocompletion....
    cbAddCompletionWord = new QCheckBox( page );
    cbAddCompletionWord->setText( i18n( "&Automatically add new words to completion list" ) );
    QWhatsThis::add( cbAddCompletionWord, i18n("If this is option is enabled, any word typed in this document will automatically be added to the list of words used by the completion." ) );

    m_lbListCompletion = new QListBox( page );
    connect( m_lbListCompletion, SIGNAL( selected ( const QString & ) ), this, SLOT( slotCompletionWordSelected( const QString & )));
    connect( m_lbListCompletion, SIGNAL( highlighted ( const QString & ) ), this, SLOT( slotCompletionWordSelected( const QString & )));

    pbAddCompletionEntry = new QPushButton( i18n("Add Completion Entry.."), page);
    connect( pbAddCompletionEntry, SIGNAL( clicked() ), this, SLOT( slotAddCompletionEntry()));

    pbRemoveCompletionEntry = new QPushButton(i18n( "R&emove Completion Entry"), page  );
    connect( pbRemoveCompletionEntry, SIGNAL( clicked() ), this, SLOT( slotRemoveCompletionEntry()));

    pbSaveCompletionEntry= new QPushButton(i18n( "&Save Completion List"), page );
    connect( pbSaveCompletionEntry, SIGNAL( clicked() ), this, SLOT( slotSaveCompletionEntry()));


    m_minWordLength = new KIntNumInput( page );
    m_minWordLength->setRange ( 5, 100,1,true );
    m_minWordLength->setLabel( i18n( "&Minimum word length:" ) );

    m_maxNbWordCompletion = new KIntNumInput( page );
    m_maxNbWordCompletion->setRange( 1, 500, 1, true);
    m_maxNbWordCompletion->setLabel( i18n( "Ma&ximum number of completion words:" ) );

    cbAppendSpace = new QCheckBox( page );
    cbAppendSpace->setText( i18n( "A&ppend space" ) );

    m_listCompletion = m_docAutoFormat->listCompletion();
}

void KoCompletionDia::slotResetConf()
{
   cbAllowCompletion->setChecked( m_autoFormat.getConfigCompletion());
    cbAddCompletionWord->setChecked( m_autoFormat.getConfigAddCompletionWord());
    m_lbListCompletion->clear();
    QStringList lst = m_docAutoFormat->listCompletion();
    m_lbListCompletion->insertStringList( lst );
    if( lst.isEmpty() || m_lbListCompletion->currentText().isEmpty())
        pbRemoveCompletionEntry->setEnabled( false );
    m_minWordLength->setValue ( m_docAutoFormat->getConfigMinWordLength() );
    m_maxNbWordCompletion->setValue ( m_docAutoFormat->getConfigNbMaxCompletionWord() );
    cbAppendSpace->setChecked( m_autoFormat.getConfigAppendSpace() );
    changeButtonStatus();
}

void KoCompletionDia::slotSaveCompletionEntry()
{

    KConfig config("kofficerc");
    KConfigGroupSaver cgs( &config, "Completion Word" );
    config.writeEntry( "list", m_listCompletion );
    config.sync();
    KMessageBox::information( this, i18n(
            "Completion list saved.\nIt will be used for all documents "
            "from now on."), i18n("Completion List Saved") );
}

void KoCompletionDia::slotAddCompletionEntry()
{
    bool ok;
    QString newWord = KLineEditDlg::getText( i18n("Add Completion Entry"),i18n("Enter entry:"),QString::null, &ok, this );
    if ( ok )
    {
        m_listCompletion.append( newWord );
        m_lbListCompletion->insertItem( newWord );
        pbRemoveCompletionEntry->setEnabled( true );

    }
}

void KoCompletionDia::slotOk()
{
    if (applyConfig())
    {
       KDialogBase::slotOk();
    }
}

bool KoCompletionDia::applyConfig()
{

    m_docAutoFormat->configCompletion( cbAllowCompletion->isChecked());
    m_docAutoFormat->configAppendSpace( cbAppendSpace->isChecked() );
    m_docAutoFormat->configMinWordLength( m_minWordLength->value() );
    m_docAutoFormat->configNbMaxCompletionWord( m_maxNbWordCompletion->value () );
    m_docAutoFormat->configAddCompletionWord( cbAddCompletionWord->isChecked());

    m_docAutoFormat->getCompletion()->setItems( m_listCompletion );
    // Save to config file
    m_docAutoFormat->saveConfig();
    return true;
}

void KoCompletionDia::slotRemoveCompletionEntry()
{
    QString text = m_lbListCompletion->currentText();
    if( !text.isEmpty() )
    {
        m_listCompletion.remove( text );
        m_lbListCompletion->removeItem( m_lbListCompletion->currentItem () );
        if( m_lbListCompletion->count()==0 )
            pbRemoveCompletionEntry->setEnabled( false );
    }
}

void KoCompletionDia::slotCompletionWordSelected( const QString & word)
{
    pbRemoveCompletionEntry->setEnabled( !word.isEmpty() );
}
