/******************************************************************/
/* KCharSelectDia - (c) by Reginald Stadlbauer 1998               */
/* Version: 0.0.4                                                 */
/* Author: Reginald Stadlbauer                                    */
/* E-Mail: reggie@kde.org                                         */
/* needs c++ library Qt (http://www.troll.no)                     */
/* written for KDE (http://www.kde.org)                           */
/* KCharSelectDia is under GNU GPL                                */
/******************************************************************/
/* Module: Character Selector Dialog                              */
/******************************************************************/

#include "kcharselectdia.h"
#include "kcharselectdia.moc"

/******************************************************************/
/* class KCharSelectDia                                           */
/******************************************************************/

/*==================== constructor ===============================*/
KCharSelectDia::KCharSelectDia(QWidget* parent,const char* name,QList<QFont> *__font,QList<QColor> *__color,QList<int> *__c,QStrList _fontList)
  : QDialog(parent,name,true)
{
  unsigned int i = 0;

  _font = __font;
  _color = __color;
  _c = __c;

  _depth = 0;

  setCaption("Select a character");

  grid = new QGridLayout(this,6,2,15,7);

  fontList = _fontList;

  lFont = new QLabel("Font:",this);
  lFont->resize(lFont->sizeHint());
  grid->addWidget(lFont,0,0);

  fontCombo = new QComboBox(true,this);
  fontCombo->insertStrList(&fontList);
  fontCombo->resize(fontCombo->sizeHint());
  fontList.find(_font->at(0)->family());
  fontCombo->setCurrentItem(fontList.at());
  grid->addWidget(fontCombo,1,0);
  connect(fontCombo,SIGNAL(activated(const QString &)),this,SLOT(fontSelected(const QString &)));

  charSelect = new KCharSelect(this,"",*_font->at(0),*_color->at(0),*_c->at(0));
  charSelect->resize(charSelect->sizeHint());
  grid->addWidget(charSelect,2,0);
  connect(charSelect,SIGNAL(activated(int)),this,SLOT(charChanged(int)));

  wid = new QWidget(this);

  grid2 = new QGridLayout(wid,2,7,0,7);

  lSize = new QLabel("Size:",wid);
  lSize->resize(lSize->sizeHint());
  grid2->addWidget(lSize,0,0);

  lColor = new QLabel("Color:",wid);
  lColor->resize(lColor->sizeHint());
  grid2->addWidget(lColor,0,1);

  sizeCombo = new QComboBox(false,wid);
  char chr[5];
  for (i = 4;i <= 100;i++)
    {
      sprintf(chr,"%d",i);
      sizeCombo->insertItem(chr,-1);
    }
  sizeCombo->resize(sizeCombo->sizeHint());
  grid2->addWidget(sizeCombo,1,0);
  sizeCombo->setCurrentItem(_font->at(0)->pointSize() - 4);
  connect(sizeCombo,SIGNAL(activated(int)),this,SLOT(sizeSelected(int)));

  colorButton = new KColorButton(*_color->at(0),wid);
  colorButton->resize(colorButton->sizeHint());
  grid2->addWidget(colorButton,1,1);
  connect(colorButton,SIGNAL(changed(const QColor&)),this,SLOT(colorChanged(const QColor&)));

  lAttrib = new QLabel("Attributes:",wid);
  lAttrib->resize(lAttrib->sizeHint());
  grid2->addMultiCellWidget(lAttrib,0,0,2,4);

  bold = new QCheckBox("Bold",wid);
  bold->resize(bold->sizeHint());
  grid2->addWidget(bold,1,2);
  bold->setChecked(_font->at(0)->bold());
  connect(bold,SIGNAL(clicked()),this,SLOT(boldChanged()));

  italic= new QCheckBox("Italic",wid);
  italic->resize(italic->sizeHint());
  grid2->addWidget(italic,1,3);
  italic->setChecked(_font->at(0)->italic());
  connect(italic,SIGNAL(clicked()),this,SLOT(italicChanged()));

  underl = new QCheckBox("Underlined",wid);
  underl->resize(underl->sizeHint());
  grid2->addWidget(underl,1,4);
  underl->setChecked(_font->at(0)->underline());
  connect(underl,SIGNAL(clicked()),this,SLOT(underlChanged()));

  lDepth = new QLabel("Depth",wid);
  lDepth->resize(lDepth->sizeHint());
  grid2->addWidget(lDepth,0,5);

  depthCombo = new QComboBox(false,wid);
  for (i = 1;i <= 16;i++)
    {
      sprintf(chr,"%d",i);
      depthCombo->insertItem(chr,-1);
    }
  depthCombo->resize(depthCombo->sizeHint());
  grid2->addWidget(depthCombo,1,5);
  connect(depthCombo,SIGNAL(activated(int)),this,SLOT(depthSelected(int)));

  lPreview = new QLabel(wid);
  lPreview->setBackgroundColor(white);
  lPreview->resize(7+colorButton->height()+lColor->height(),
		   7+colorButton->height()+lColor->height());
  lPreview->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  sprintf(chr,"%c",*_c->at(0));
  lPreview->setText(chr);
  lPreview->setAlignment(AlignHCenter | AlignVCenter);
  lPreview->setFont(*_font->at(0));
  colorChanged(*_color->at(0));
  grid2->addMultiCellWidget(lPreview,0,1,6,6);

  grid2->addColSpacing(0,lSize->width());
  grid2->addColSpacing(0,sizeCombo->width());
  grid2->addColSpacing(1,lColor->width());
  grid2->addColSpacing(1,colorButton->width());
  grid2->addColSpacing(2,bold->width());
  grid2->addColSpacing(3,italic->width());
  grid2->addColSpacing(4,underl->width());
  grid2->addColSpacing(5,lDepth->width());
  grid2->addColSpacing(5,depthCombo->width());
  grid2->addColSpacing(6,lPreview->width());

  grid2->addRowSpacing(0,lSize->height());
  grid2->addRowSpacing(0,lColor->height());
  grid2->addRowSpacing(0,lAttrib->height());
  grid2->addRowSpacing(0,lSize->height());
  grid2->addRowSpacing(1,sizeCombo->height());
  grid2->addRowSpacing(1,colorButton->height());
  grid2->addRowSpacing(1,bold->height());
  grid2->addRowSpacing(1,italic->height());
  grid2->addRowSpacing(1,underl->height());
  grid2->addRowSpacing(1,depthCombo->height());

  grid2->activate();
  grid->addWidget(wid,3,0);

  bbox = new KButtonBox(this,KButtonBox::HORIZONTAL,7);
  bbox->addStretch(20);
  bOk = bbox->addButton("OK");
  bOk->setAutoRepeat(false);
  bOk->setAutoResize(false);
  bOk->setAutoDefault(true);
  bOk->setDefault(true);
  connect(bOk,SIGNAL(clicked()),SLOT(accept()));
  bCancel = bbox->addButton("Cancel");
  connect(bCancel,SIGNAL(clicked()),SLOT(reject()));
  bbox->layout();
  grid->addWidget(bbox,4,0);

  grid->addColSpacing(0,lFont->width());
  grid->addColSpacing(0,fontCombo->width());
  grid->addColSpacing(0,charSelect->width());
  grid->addColSpacing(0,wid->width());
  grid->addColSpacing(1,0);
  grid->setColStretch(1,1);

  grid->addRowSpacing(0,lFont->height());
  grid->addRowSpacing(1,fontCombo->height());
  grid->addRowSpacing(2,charSelect->height());
  grid->addRowSpacing(3,wid->height());
  grid->addRowSpacing(4,bOk->height());
  grid->addRowSpacing(4,bCancel->height());
  grid->addRowSpacing(5,0);
  grid->setRowStretch(5,1);
  grid->activate();
}

/*===================== destructor ===============================*/
KCharSelectDia::~KCharSelectDia()
{
}

/*======================= about application ======================*/
bool KCharSelectDia::selectChar(QList<QFont> *__font,QList<QColor> *__color,QList<int> *__c,QStrList _fontList)
{
  bool res = false;

    KCharSelectDia *dlg = new KCharSelectDia(0,"Select Character",__font,__color,__c,_fontList);
    if (dlg->exec() == QDialog::Accepted)
      {
	__font = dlg->font();
	__color = dlg->color();
	__c = dlg->c();
	res = true;
      }

    delete dlg;

    return res;
}

/*=========================== Font selected =====================*/
void KCharSelectDia::fontSelected(const QString &_family)
{
  _font->at(_depth)->setFamily(_family);
  charSelect->setFont(QFont(_family));
  lPreview->setFont(*_font->at(_depth));
}

/*===================== size selected ===========================*/
void KCharSelectDia::sizeSelected(int i)
{
  _font->at(_depth)->setPointSize(i + 4);
  lPreview->setFont(*_font->at(_depth));
}

/*===============================================================*/
void KCharSelectDia::depthSelected(int d)
{
  _depth = d;

  char chr[5];
  sprintf(chr,"%c",*_c->at(d));
  lPreview->setText(chr);
  charSelect->setChar(*_c->at(d));

  fontList.find(_font->at(d)->family());
  fontCombo->setCurrentItem(fontList.at());

  colorButton->setColor(*_color->at(d));
  colorChanged(*_color->at(d));

  bold->setChecked(_font->at(d)->bold());
  italic->setChecked(_font->at(d)->italic());
  underl->setChecked(_font->at(d)->underline());

  sizeCombo->setCurrentItem(_font->at(d)->pointSize() - 4);
}

/*===================== color selected ==========================*/
void KCharSelectDia::colorChanged(const QColor& __color)
{
 *_color->at(_depth) = __color;
 QColorGroup cgrp(__color,lPreview->colorGroup().background(),
		  lPreview->colorGroup().light(),lPreview->colorGroup().dark(),
		  lPreview->colorGroup().mid(),__color,lPreview->colorGroup().base());
 QPalette p(cgrp,cgrp,cgrp);

 lPreview->setPalette(p);
}

/*====================== bold clicked ===========================*/
void KCharSelectDia::boldChanged()
{
  _font->at(_depth)->setBold(bold->isChecked());
  lPreview->setFont(*_font->at(_depth));
}

/*====================== italic clicked =========================*/
void KCharSelectDia::italicChanged()
{
  _font->at(_depth)->setItalic(italic->isChecked());
  lPreview->setFont(*_font->at(_depth));
}

/*====================== underline clicked ======================*/
void KCharSelectDia::underlChanged()
{
  _font->at(_depth)->setUnderline(underl->isChecked());
  lPreview->setFont(*_font->at(_depth));
}

/*================== character changed ==========================*/
void KCharSelectDia::charChanged(int __c)
{
  char chr[5];

  sprintf(chr,"%c",__c);

  lPreview->setText(chr);
  *_c->at(_depth) = __c;
}
