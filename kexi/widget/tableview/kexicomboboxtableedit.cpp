/* This file is part of the KDE project
   Copyright (C) 2002   Peter Simonsson <psn@linux.se>
   Copyright (C) 2003-2014 Jarosław Staniek <staniek@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <QLayout>
#include <QStyle>
#include <QWindowsStyle>
#include <QPainter>
#include <QKeyEvent>
#include <QEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QClipboard>

#include "kexicomboboxtableedit.h"
#include <widget/utils/kexicomboboxdropdownbutton.h>
#include <kexiutils/utils.h>
#include "kexicomboboxpopup.h"
#include "kexitableview.h"
#include "kexi.h"

#include <klineedit.h>

// the right margin is too large when the editor is show, reduce it
const int RIGHT_MARGIN_DELTA = 6;

//! @internal
class KexiComboBoxTableEdit::Private
{
public:
    Private()
            : popup(0)
            , currentEditorWidth(0)
            , visibleTableViewColumn(0)
            , internalEditor(0) {
    }
    ~Private() {
        delete internalEditor;
        delete visibleTableViewColumn;
    }

    KexiComboBoxDropDownButton *button;
    KexiComboBoxPopup *popup;
    int currentEditorWidth;
    QSize totalSize;
    KexiDB::TableViewColumn* visibleTableViewColumn;
    KexiTableEdit* internalEditor;
};

//======================================================

KexiComboBoxTableEdit::KexiComboBoxTableEdit(KexiDB::TableViewColumn &column, QWidget *parent)
        : KexiComboBoxBase()
        , KexiInputTableEdit(column, parent)
        , d(new Private())
{
    m_setVisibleValueOnSetValueInternal = true;
    m_reinstantiatePopupOnShow = true; // needed because re-opening of the popup fails for unknown reason
    d->button = new KexiComboBoxDropDownButton(parentWidget() /*usually a viewport*/);
    d->button->hide();
    d->button->setFocusPolicy(Qt::NoFocus);
    connect(d->button, SIGNAL(clicked()), this, SLOT(slotButtonClicked()));

    connect(m_lineedit, SIGNAL(textChanged(QString)), this, SLOT(slotLineEditTextChanged(QString)));

    m_rightMarginWhenFocused = this->column()->isReadOnly() ? 0 : d->button->width();
    m_rightMarginWhenFocused -= RIGHT_MARGIN_DELTA;
    updateLineEditStyleSheet();
    m_rightMarginWhenFocused += RIGHT_MARGIN_DELTA;
}

KexiComboBoxTableEdit::~KexiComboBoxTableEdit()
{
    delete d;
}

void KexiComboBoxTableEdit::createInternalEditor(KexiDB::QuerySchema& schema)
{
    if (!m_column->visibleLookupColumnInfo() || d->visibleTableViewColumn/*sanity*/)
        return;
    const KexiDB::Field::Type t = m_column->visibleLookupColumnInfo()->field->type();
//! @todo subtype?
    KexiCellEditorFactoryItem *item = KexiCellEditorFactory::item(t);
    if (!item || item->className() == "KexiInputTableEdit")
        return; //unsupported type or there is no need to use subeditor for KexiInputTableEdit
    //special cases: BLOB, Bool datatypes
//! @todo
    //find real type to display
    KexiDB::QueryColumnInfo *ci = m_column->visibleLookupColumnInfo();
    KexiDB::QueryColumnInfo *visibleLookupColumnInfo = 0;
    if (ci->indexForVisibleLookupValue() != -1) {
        //Lookup field is defined
        visibleLookupColumnInfo = schema.expandedOrInternalField(ci->indexForVisibleLookupValue());
    }
    d->visibleTableViewColumn = new KexiDB::TableViewColumn(schema, *ci, visibleLookupColumnInfo);
//! todo set d->internalEditor visible and use it to enable data entering by hand
    d->internalEditor = KexiCellEditorFactory::createEditor(*d->visibleTableViewColumn, 0);
    m_lineedit->hide();
}

KexiComboBoxPopup *KexiComboBoxTableEdit::popup() const
{
    return d->popup;
}

void KexiComboBoxTableEdit::setPopup(KexiComboBoxPopup *popup)
{
    d->popup = popup;
}

void KexiComboBoxTableEdit::showFocus(const QRect& r, bool readOnly)
{
    updateFocus(r);
    d->button->setEnabled(!readOnly);
    d->button->setVisible(!readOnly);
}

void KexiComboBoxTableEdit::resize(int w, int h)
{
    d->totalSize = QSize(w, h);
    if (!column()->isReadOnly()) {
        d->button->resize(h, h);
        QWidget::resize(w, h);
    }
    m_rightMarginWhenFocused = column()->isReadOnly() ? 0 : d->button->width();
    m_rightMarginWhenFocused -= RIGHT_MARGIN_DELTA;
    updateLineEditStyleSheet();
    m_rightMarginWhenFocused += RIGHT_MARGIN_DELTA;
    QRect r(pos().x(), pos().y(), w + 1, h + 1);
    if (m_scrollView)
        r.translate(m_scrollView->contentsX(), m_scrollView->contentsY());
    updateFocus(r);
    if (popup()) {
        popup()->updateSize();
    }
}

// internal
void KexiComboBoxTableEdit::updateFocus(const QRect& r)
{
    if (!column()->isReadOnly()) {
        if (d->button->width() > r.width())
            moveChild(d->button, r.right() + 1, r.top());
        else
            moveChild(d->button, r.right() - d->button->width(), r.top());
    }
}

void KexiComboBoxTableEdit::hideFocus()
{
    d->button->hide();
}

QVariant KexiComboBoxTableEdit::visibleValue()
{
    return KexiComboBoxBase::visibleValue();
}

void KexiComboBoxTableEdit::clear()
{
    m_lineedit->clear();
    KexiComboBoxBase::clear();
}

bool KexiComboBoxTableEdit::valueChanged()
{
    const tristate res = valueChangedInternal();
    if (~res) //no result: just compare values
        return KexiInputTableEdit::valueChanged();
    return res == true;
}

void KexiComboBoxTableEdit::paintFocusBorders(QPainter *p, QVariant &, int x, int y, int w, int h)
{
    p->drawRect(x, y, w, h);
}

void KexiComboBoxTableEdit::setupContents(QPainter *p, bool focused, const QVariant& val,
        QString &txt, int &align, int &x, int &y_offset, int &w, int &h)
{
    if (d->internalEditor) {
        d->internalEditor->setupContents(p, focused, val, txt, align, x, y_offset, w, h);
    } else {
        KexiInputTableEdit::setupContents(p, focused, val, txt, align, x, y_offset, w, h);
    }
    if (!val.isNull()) {
        KexiDB::TableViewData *relData = column()->relatedData();
        KexiDB::LookupFieldSchema *lookupFieldSchema = 0;
        if (relData) {
            int rowToHighlight;
            txt = valueForString(val.toString(), &rowToHighlight, 0, 1);
        }
        else if ((lookupFieldSchema = this->lookupFieldSchema())) {
        }
        else {
            //use 'enum hints' model
            txt = field()->enumHint(val.toInt());
        }
    }
}

void KexiComboBoxTableEdit::slotButtonClicked()
{
    // this method is sometimes called by hand:
    // do not allow to simulate clicks when the button is disabled
    if (column()->isReadOnly() || !d->button->isEnabled())
        return;

    if (m_mouseBtnPressedWhenPopupVisible) {
        m_mouseBtnPressedWhenPopupVisible = false;
        return;
    }
    kDebug();
    if (!popup() || !popup()->isVisible()) {
        kDebug() << "SHOW POPUP";
        showPopup();
    }
}

void KexiComboBoxTableEdit::slotPopupHidden()
{
}

void KexiComboBoxTableEdit::updateButton()
{
}

void KexiComboBoxTableEdit::hide()
{
    KexiInputTableEdit::hide();
    KexiComboBoxBase::hide();
}

void KexiComboBoxTableEdit::show()
{
    KexiInputTableEdit::show();
    if (!column()->isReadOnly()) {
        d->button->show();
    }
}

bool KexiComboBoxTableEdit::handleKeyPress(QKeyEvent *ke, bool editorActive)
{
    //kDebug() << ke;
    const int k = ke->key();
    if ((ke->modifiers() == Qt::NoButton && k == Qt::Key_F4)
            || (ke->modifiers() == Qt::AltButton && k == Qt::Key_Down)) {
        //show popup
        slotButtonClicked();
        return true;
    } else if (editorActive) {
        const bool enterPressed = k == Qt::Key_Enter || k == Qt::Key_Return;
        if (enterPressed && m_internalEditorValueChanged) {
            createPopup(false);
            selectItemForEnteredValueInLookupTable(m_userEnteredValue);
            return true;
        }

        return handleKeyPressForPopup(ke);
    }

    return false;
}

void KexiComboBoxTableEdit::slotLineEditTextChanged(const QString& s)
{
    slotInternalEditorValueChanged(s);
}

int KexiComboBoxTableEdit::widthForValue(const QVariant &val, const QFontMetrics &fm)
{
    KexiDB::TableViewData *relData = column() ? column()->relatedData() : 0;
    if (lookupFieldSchema() || relData) {
        // in 'lookupFieldSchema' or  or 'related table data' model
        // we're assuming val is already the text, not the index
//! @todo ok?
        return qMax(KEXITV_MINIMUM_COLUMN_WIDTH, fm.width(val.toString()));
    }
    //use 'enum hints' model
    QVector<QString> hints = field()->enumHints();
    bool ok;
    int idx = val.toInt(&ok);
    if (!ok || idx < 0 || idx > int(hints.size() - 1))
        return KEXITV_MINIMUM_COLUMN_WIDTH;
    QString txt = hints.value(idx);
    if (txt.isEmpty())
        return KEXITV_MINIMUM_COLUMN_WIDTH;
    return fm.width(txt);
}

bool KexiComboBoxTableEdit::eventFilter(QObject *o, QEvent *e)
{
#if 0
    if (e->type() != QEvent::Paint
            && e->type() != QEvent::Leave
            && e->type() != QEvent::MouseMove
            && e->type() != QEvent::HoverMove
            && e->type() != QEvent::HoverEnter
            && e->type() != QEvent::HoverLeave)
    {
        kDebug() << e << o;
        kDebug() << "FOCUS WIDGET:" << focusWidget();
    }
#endif
    KexiTableView *tv = dynamic_cast<KexiTableView*>(m_scrollView);
    if (tv && e->type() == QEvent::KeyPress) {
        if (tv->eventFilter(o, e)) {
            return true;
        }
    }
    if (!column()->isReadOnly() && e->type() == QEvent::MouseButtonPress && m_scrollView) {
        QPoint gp = static_cast<QMouseEvent*>(e)->globalPos()
                    + QPoint(m_scrollView->childX(d->button), m_scrollView->childY(d->button));
        QRect r(d->button->mapToGlobal(d->button->geometry().topLeft()),
                d->button->mapToGlobal(d->button->geometry().bottomRight()));
        if (o == popup() && popup()->isVisible() && r.contains(gp)) {
            m_mouseBtnPressedWhenPopupVisible = true;
        }
    }
    return false;
}

QSize KexiComboBoxTableEdit::totalSize() const
{
    return d->totalSize;
}

QWidget *KexiComboBoxTableEdit::internalEditor() const
{
    return m_lineedit;
}

void KexiComboBoxTableEdit::moveCursorToEndInInternalEditor()
{
    moveCursorToEnd();
}

void KexiComboBoxTableEdit::selectAllInInternalEditor()
{
    selectAll();
}

void KexiComboBoxTableEdit::moveCursorToEnd()
{
    m_lineedit->end(false/*!mark*/);
}

void KexiComboBoxTableEdit::moveCursorToStart()
{
    m_lineedit->home(false/*!mark*/);
}

void KexiComboBoxTableEdit::selectAll()
{
    m_lineedit->selectAll();
}

void KexiComboBoxTableEdit::setValueInInternalEditor(const QVariant& value)
{
    KexiUtils::BoolBlocker guard(m_slotInternalEditorValueChanged_enabled, false);
    m_lineedit->setText(value.toString());
}

QVariant KexiComboBoxTableEdit::valueFromInternalEditor()
{
    return m_lineedit->text();
}

QPoint KexiComboBoxTableEdit::mapFromParentToGlobal(const QPoint& pos) const
{
    KexiTableView *tv = dynamic_cast<KexiTableView*>(m_scrollView);
    if (!tv)
        return QPoint(-1, -1);
    return tv->viewport()->mapToGlobal(pos);
}

int KexiComboBoxTableEdit::popupWidthHint() const
{
    return m_lineedit->width() + m_leftMargin + m_rightMarginWhenFocused;
}

void KexiComboBoxTableEdit::handleCopyAction(const QVariant& value, const QVariant& visibleValue)
{
    Q_UNUSED(value);
//! @todo does not work with BLOBs!
    qApp->clipboard()->setText(visibleValue.toString());
}

void KexiComboBoxTableEdit::handleAction(const QString& actionName)
{
    const bool alreadyVisible = m_lineedit->isVisible();

    if (actionName == "edit_paste") {
        if (!alreadyVisible) { //paste as the entire text if the cell was not in edit mode
            emit editRequested();
            m_lineedit->clear();
        }
//! @todo does not work with BLOBs!
        setValueInInternalEditor(qApp->clipboard()->text());
    } else
        KexiInputTableEdit::handleAction(actionName);
}

QVariant KexiComboBoxTableEdit::origValue() const
{
    return KexiDataItemInterface::originalValue();
}

KEXI_CELLEDITOR_FACTORY_ITEM_IMPL(KexiComboBoxEditorFactoryItem, KexiComboBoxTableEdit)

#include "kexicomboboxtableedit.moc"
