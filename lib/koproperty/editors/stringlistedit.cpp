/* This file is part of the KDE project
   Copyright (C) 2004 Cedric Pasteur <cedric.pasteur@free.fr>
   Copyright (C) 2004  Alexander Dymo <cloudtemple@mskat.net>

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

#include "stringlistedit.h"

#include <qlineedit.h>
#include <qlayout.h>
#include <qdialog.h>
#include <qpainter.h>
#include <qvariant.h>
#include <qpushbutton.h>

#ifndef QT_ONLY
#include <keditlistbox.h>
#include <kdialogbase.h>
#include <kstdguiitem.h>
#include <klocale.h>
#include <kdebug.h>
#else
#include "qeditlistbox.h"
#include <compat_tools.h>
#endif

#include "property.h"

namespace KoProperty {

StringListEdit::StringListEdit(Property *property, QWidget *parent, const char *name)
 : Widget(property, parent, name)
{
	setHasBorders(false);
	QHBoxLayout *l = new QHBoxLayout(this, 0, 0);

	m_edit = new QLineEdit(this);
	m_edit->setLineWidth(0);
	m_edit->setReadOnly(true);
	m_edit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_edit->setMinimumHeight(5);
	l->addWidget(m_edit);

	QPushButton *selectButton = new QPushButton("...", this);
	selectButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
	l->addWidget(selectButton);
	setFocusWidget(selectButton);

	connect(selectButton, SIGNAL(clicked()), this, SLOT(showEditor()));
}

StringListEdit::~StringListEdit()
{}

QVariant
StringListEdit::value() const
{
	return m_list;
}

void
StringListEdit::setValue(const QVariant &value, bool emitChange)
{
	m_list = value.toStringList();
	m_edit->setText(value.toStringList().join(", "));
	if(emitChange)
		emit valueChanged(this);
}

void
StringListEdit::drawViewer(QPainter *p, const QColorGroup &cg, const QRect &r, const QVariant &value)
{
//	p->eraseRect(r);
//	p->drawText(r, Qt::AlignLeft | Qt::AlignVCenter | Qt::SingleLine, value.toStringList().join(", "));
	Widget::drawViewer(p, cg, r, value.toStringList().join(", "));
}

void
StringListEdit::showEditor()
{
#ifdef QT_ONLY
	QDialog* dia = new QDialog(this, "stringlist_dialog", true);
	QVBoxLayout *dv = new QVBoxLayout(dia, 2);
	QEditListBox *select = new QEditListBox(dia, "select_char");
	dv->addWidget(select);

	QHBoxLayout *dh = new QHBoxLayout(dv, 6);
	QPushButton *pbOk = new QPushButton(i18n("Ok"), dia);
	QPushButton *pbCancel = new QPushButton(i18n("Cancel"), dia);

	QSpacerItem *si = new QSpacerItem(30, 0, QSizePolicy::Expanding, QSizePolicy::Expanding);

	connect(pbOk, SIGNAL(clicked()), dia, SLOT(accept()));
	connect(pbCancel, SIGNAL(clicked()), dia, SLOT(reject()));

	dh->addItem(si);
	dh->addWidget(pbOk);
	dh->addWidget(pbCancel);

	select->insertStringList(m_list);

	if (dia->exec() == QDialog::Accepted) {
		m_list = select->items();
		m_edit->setText(select->items().join(", "));
		emit valueChanged(this);
	}
	delete dia;

#else

	KDialogBase dialog(this->topLevelWidget(), "stringlist_dialog", true, i18n("Edit List of Items"),
	    KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, false);

	KEditListBox *edit = new KEditListBox(i18n("Contents of %1").arg(property()->caption()), &dialog, "editlist");
	dialog.setMainWidget(edit);
	edit->insertStringList(m_list);

	if(dialog.exec() == QDialog::Accepted)
	{
		m_list = edit->items();
		m_edit->setText(m_list.join(", "));
		emit valueChanged(this);
	}

#endif

}

}

#include "stringlistedit.moc"

