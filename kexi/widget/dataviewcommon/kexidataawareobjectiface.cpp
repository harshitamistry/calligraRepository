/* This file is part of the KDE project
   Copyright (C) 2005-2013 Jarosław Staniek <staniek@kde.org>

   Based on KexiTableView code.
   Copyright (C) 2002 Till Busch <till@bux.at>
   Copyright (C) 2003 Lucijan Busch <lucijan@gmx.at>
   Copyright (C) 2003 Daniel Molkentin <molkentin@kde.org>
   Copyright (C) 2003 Joseph Wenninger <jowenn@kde.org>

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

#include "kexidataawareobjectiface.h"

#include <QScrollArea>
#include <QScrollBar>
#include <QLabel>
#include <QToolTip>
#include <QStyle>
#include <QStyleOptionComplex>

#include <kmessagebox.h>

#include <KoIcon.h>

#include <kexi.h>
#include <kexi_global.h>
#include <kexiutils/utils.h>
#include <widget/utils/kexirecordnavigator.h>
#include <widget/utils/kexirecordmarker.h>
#include <db/roweditbuffer.h>
#include <db/validator.h>

#include <limits.h>

using namespace KexiUtils;

KexiDataAwareObjectInterface::KexiDataAwareObjectInterface()
{
    m_data = 0;
    m_owner = false;
    m_readOnly = -1; //don't know
    m_insertingEnabled = -1; //don't know
    m_isSortingEnabled = true;
    m_isFilteringEnabled = true;
    m_deletionPolicy = AskDelete;
    m_inside_acceptEditor = false;
    m_inside_acceptRowEdit = false;
    m_acceptsRowEditAfterCellAccepting = false;
    m_internal_acceptsRowEditAfterCellAccepting = false;
    m_contentsMousePressEvent_dblClick = false;
    m_navPanel = 0;
    m_initDataContentsOnShow = false;
    m_cursorPositionSetExplicityBeforeShow = false;
    m_verticalHeader = 0;
    m_insertItem = 0;
    m_spreadSheetMode = false;
    m_dropsAtRowEnabled = false;
    m_updateEntireRowWhenMovingToOtherRow = false;
    m_dragIndicatorLine = -1;
    m_emptyRowInsertingEnabled = false;
    m_contextMenu = 0;
    m_contextMenuEnabled = true;
    m_rowWillBeDeleted = -1;
    m_alsoUpdateNextRow = false;
    m_verticalHeaderAlreadyAdded = false;
    m_vScrollBarValueChanged_enabled = true;
    m_scrollbarToolTipsEnabled = true;
    m_recentSearchDirection = KexiSearchAndReplaceViewInterface::Options::DefaultSearchDirection;

    m_lengthExceededMessageVisible = false;
    clearVariables();
}

KexiDataAwareObjectInterface::~KexiDataAwareObjectInterface()
{
    delete m_insertItem;
}

void KexiDataAwareObjectInterface::clearVariables()
{
    m_editor = 0;
    m_rowEditing = false;
    m_newRowEditing = false;
    m_curRow = -1;
    m_curCol = -1;
    m_currentItem = 0;
}

void KexiDataAwareObjectInterface::setData(KexiDB::TableViewData *data, bool owner)
{
    const bool theSameData = m_data && m_data == data;
    if (m_owner && m_data && m_data != data/*don't destroy if it's the same*/) {
        kDebug() << "destroying old data (owned)";
        delete m_data; //destroy old data
        m_data = 0;
        m_itemIterator = KexiDB::TableViewData::Iterator();
    }
    m_owner = owner;
    m_data = data;
    if (m_data)
        m_itemIterator = m_data->constBegin();

    //kDebug() << "using shared data";
    //add columns
    clearColumnsInternal(false);
    if (m_data) {
        int i = -1;
        foreach(KexiDB::TableViewColumn *col, *m_data->columns()) {
            i++;
            KexiDB::Field *f = col->field();
            if (col->isVisible()) {
                int wid = col->width();
                if (wid == 0)
                    wid = KEXI_DEFAULT_DATA_COLUMN_WIDTH;//default col width in pixels
//! @todo add col width configuration and storage
                addHeaderColumn(col->isHeaderTextVisible()
                                ? col->captionAliasOrName() : QString(),
                                f->description(), col->icon(), wid);
            }
        }
    }
    
    if (m_verticalHeader) {
        m_verticalHeader->clear();
        if (m_data)
            m_verticalHeader->addLabels(m_data->count());
    }
//!Change the following:
    if (m_data && m_data->count() == 0 && m_navPanel)
        m_navPanel->setCurrentRecordNumber(0 + 1);
    
    if (m_data && !theSameData) {
//! @todo: store sorting settings?
        setSorting(-1);
        connectToReloadDataSlot(m_data, SIGNAL(reloadRequested()));
        QObject* thisObject = dynamic_cast<QObject*>(this);
        if (thisObject) {
            QObject::connect(m_data, SIGNAL(destroying()), thisObject, SLOT(slotDataDestroying()));
            QObject::connect(m_data, SIGNAL(rowsDeleted(QList<int>)),
                             thisObject, SLOT(slotRowsDeleted(QList<int>)));
            QObject::connect(m_data, SIGNAL(aboutToDeleteRow(KexiDB::RecordData&,KexiDB::ResultInfo*,bool)),
                             thisObject, SLOT(slotAboutToDeleteRow(KexiDB::RecordData&,KexiDB::ResultInfo*,bool)));
            QObject::connect(m_data, SIGNAL(rowDeleted()), thisObject, SLOT(slotRowDeleted()));
            QObject::connect(m_data, SIGNAL(rowInserted(KexiDB::RecordData*,bool)),
                             thisObject, SLOT(slotRowInserted(KexiDB::RecordData*,bool)));
            QObject::connect(m_data, SIGNAL(rowInserted(KexiDB::RecordData*,uint,bool)),
                             thisObject, SLOT(slotRowInserted(KexiDB::RecordData*,uint,bool))); //not db-aware
            QObject::connect(m_data, SIGNAL(rowRepaintRequested(KexiDB::RecordData&)),
                             thisObject, SLOT(slotRowRepaintRequested(KexiDB::RecordData&)));
            QObject::connect(verticalScrollBar(), SIGNAL(valueChanged(int)),
                             thisObject, SLOT(vScrollBarValueChanged(int)));
        }
    }

    if (!m_data) {
        cancelRowEdit();
        clearVariables();
    } else {
        if (!m_insertItem) {//first setData() call - add 'insert' item
            m_insertItem = m_data->createItem();
        } else {//just reinit
            m_insertItem->init(m_data->columnsCount());
        }
    }

    //update gui mode
    if (m_navPanel)
        m_navPanel->setInsertingEnabled(m_data && isInsertingEnabled());
    if (m_verticalHeader)
        m_verticalHeader->showInsertRow(m_data && isInsertingEnabled());

    initDataContents();
    updateIndicesForVisibleValues();

    if (m_data)
        /*emit*/ dataSet(m_data);
}

void KexiDataAwareObjectInterface::initDataContents()
{
    m_editor = 0;
    if (m_navPanel)
        m_navPanel->setRecordCount(rows());

    if (m_data && !m_cursorPositionSetExplicityBeforeShow) {
        //set current row:
        m_currentItem = 0;
        int curRow = -1, curCol = -1;
        if (m_data->columnsCount() > 0) {
            if (rows() > 0) {
                m_itemIterator = m_data->constBegin();
                m_currentItem = *m_itemIterator;
                curRow = 0;
                curCol = 0;
            } else {//no data
                if (isInsertingEnabled()) {
                    m_currentItem = m_insertItem;
                    curRow = 0;
                    curCol = 0;
                }
            }
        }
        setCursorPosition(curRow, curCol, true/*force*/);
    }
    ensureCellVisible(m_curRow, m_curCol);

//OK?
    updateWidgetContents();

    m_cursorPositionSetExplicityBeforeShow = false;

    /*emit*/ dataRefreshed();
}

void KexiDataAwareObjectInterface::setSortingEnabled(bool set)
{
    if (m_isSortingEnabled && !set)
        setSorting(-1);
    m_isSortingEnabled = set;
    /*emit*/ reloadActions();
}

void KexiDataAwareObjectInterface::setSorting(int col, bool ascending)
{
    if (!m_data || !m_isSortingEnabled)
        return;
    setLocalSortingOrder(col, ascending ? 1 : -1);
    m_data->setSorting(col, ascending);
}

int KexiDataAwareObjectInterface::dataSortedColumn() const
{
    if (m_data && m_isSortingEnabled)
        return m_data->sortedColumn();
    return -1;
}

int KexiDataAwareObjectInterface::dataSortingOrder() const
{
    return m_data ? m_data->sortingOrder() : 0;
}

bool KexiDataAwareObjectInterface::sort()
{
    if (!m_data || !m_isSortingEnabled)
        return false;

    if (rows() < 2)
        return true;

    if (!acceptRowEdit())
        return false;

    const int oldRow = m_curRow;
    if (m_data->sortedColumn() != -1)
        m_data->sort();

    //locate current record
    if (!m_currentItem) {
        m_itemIterator = m_data->constBegin();
        m_currentItem = *m_itemIterator;
        m_curRow = 0;
        if (!m_currentItem)
            return true;
    }
    if (m_currentItem != m_insertItem) {
        m_curRow = m_data->indexOf(m_currentItem);
        int jump = m_curRow - oldRow;
        if (jump < 0)
            m_itemIterator -= -jump;
        else
            m_itemIterator += jump;
    }

    updateGUIAfterSorting();
    editorShowFocus(m_curRow, m_curCol);
    if (m_verticalHeader)
        m_verticalHeader->setCurrentRow(m_curRow);
    if (m_navPanel)
        m_navPanel->setCurrentRecordNumber(m_curRow + 1);
    return true;
}

void KexiDataAwareObjectInterface::sortAscending()
{
    if (currentColumn() < 0)
        return;
    sortColumnInternal(currentColumn(), 1);
}

void KexiDataAwareObjectInterface::sortDescending()
{
    if (currentColumn() < 0)
        return;
    sortColumnInternal(currentColumn(), -1);
}

void KexiDataAwareObjectInterface::sortColumnInternal(int col, int order)
{
    //-select sorting
    bool asc;
    if (order == 0) {// invert
        if (col == dataSortedColumn() && dataSortingOrder() == 1)
            asc = dataSortingOrder() == -1; //inverse sorting for this column -> descending order
        else
            asc = true;
    } else
        asc = (order == 1);

    int prevSortOrder = currentLocalSortingOrder();
    const int prevSortColumn = currentLocalSortingOrder();
    setSorting(col, asc);
    //-perform sorting
    if (!sort())
        setLocalSortingOrder(prevSortColumn, prevSortOrder); //this will also remove indicator
    //if prevSortColumn==-1
    if (col != prevSortColumn)
        /*emit*/ sortedColumnChanged(col);
}

bool KexiDataAwareObjectInterface::isInsertingEnabled() const
{
    if (isReadOnly())
        return false;
    if (m_insertingEnabled == 1 || m_insertingEnabled == 0)
        return (bool)m_insertingEnabled;
    if (!hasData())
        return true;
    return m_data->isInsertingEnabled();
}

void KexiDataAwareObjectInterface::setFilteringEnabled(bool set)
{
    m_isFilteringEnabled = set;
}

bool KexiDataAwareObjectInterface::isDeleteEnabled() const
{
    return (m_deletionPolicy != NoDelete) && !isReadOnly();
}

void KexiDataAwareObjectInterface::setDeletionPolicy(DeletionPolicy policy)
{
    m_deletionPolicy = policy;
}

void KexiDataAwareObjectInterface::setReadOnly(bool set)
{
    if (isReadOnly() == set || (m_data && m_data->isReadOnly() && !set))
        return; //not allowed!
    m_readOnly = (set ? 1 : 0);
    if (set)
        setInsertingEnabled(false);
    updateWidgetContents();
    /*emit*/ reloadActions();
}

bool KexiDataAwareObjectInterface::isReadOnly() const
{
    if (!hasData())
        return true;
    if (m_readOnly == 1 || m_readOnly == 0)
        return (bool)m_readOnly;
    if (!hasData())
        return true;
    return m_data->isReadOnly();
}

void KexiDataAwareObjectInterface::setInsertingEnabled(bool set)
{
    if (isInsertingEnabled() == set || (m_data && !m_data->isInsertingEnabled() && set))
        return; //not allowed!
    m_insertingEnabled = (set ? 1 : 0);
    if (m_navPanel)
        m_navPanel->setInsertingEnabled(set);
    if (m_verticalHeader)
        m_verticalHeader->showInsertRow(set);
    if (set)
        setReadOnly(false);
    updateWidgetContents();
    /*emit*/ reloadActions();
}

void KexiDataAwareObjectInterface::setSpreadSheetMode()
{
    m_spreadSheetMode = true;
    setSortingEnabled(false);
    setInsertingEnabled(false);
    setAcceptsRowEditAfterCellAccepting(true);
    setFilteringEnabled(false);
    setEmptyRowInsertingEnabled(true);
    m_navPanelEnabled = false;
}

void KexiDataAwareObjectInterface::selectNextRow()
{
    selectRow(qMin(rows() - 1 + (isInsertingEnabled() ? 1 : 0), m_curRow + 1));
}

void KexiDataAwareObjectInterface::selectPrevPage()
{
    selectRow(
        qMax(0, m_curRow - rowsPerPage())
    );
}

void KexiDataAwareObjectInterface::selectNextPage()
{
    selectRow(
        qMin(
            rows() - 1 + (isInsertingEnabled() ? 1 : 0),
            m_curRow + rowsPerPage()
        )
    );
}

void KexiDataAwareObjectInterface::selectFirstRow()
{
    selectRow(0);
}

void KexiDataAwareObjectInterface::selectLastRow()
{
    selectRow(rows() - 1);
}

void KexiDataAwareObjectInterface::selectRow(int row)
{
    m_vScrollBarValueChanged_enabled = false; //disable tooltip
    setCursorPosition(row, -1);
    m_vScrollBarValueChanged_enabled = true;
}

void KexiDataAwareObjectInterface::selectPrevRow()
{
    selectRow(qMax(0, m_curRow - 1));
}

void KexiDataAwareObjectInterface::clearSelection()
{
    int oldRow = m_curRow;
    m_curRow = -1;
    m_curCol = -1;
    m_currentItem = 0;
    updateRow(oldRow);
    if (m_navPanel)
        m_navPanel->setCurrentRecordNumber(0);
}

void KexiDataAwareObjectInterface::setCursorPosition(int row, int col/*=-1*/, bool forceSet)
{
    int newrow = row;
    int newcol = col;

    if (rows() <= 0) {
        if (m_verticalHeader)
            m_verticalHeader->setCurrentRow(-1);
        if (isInsertingEnabled()) {
            m_currentItem = m_insertItem;
            newrow = 0;
            if (col >= 0)
                newcol = col;
            else
                newcol = 0;
        } else {
            m_currentItem = 0;
            m_curRow = -1;
            m_curCol = -1;
            return;
        }
    }

    if (col >= 0) {
        newcol = qMax(0, col);
        newcol = qMin(columns() - 1, newcol);
    } else {
        newcol = m_curCol; //no changes
        newcol = qMax(0, newcol); //may not be < 0 !
    }
    newrow = qMax(0, row);
    newrow = qMin(rows() - 1 + (isInsertingEnabled() ? 1 : 0), newrow);

// kDebug() << "setCursorPosition(): d->curRow=" << d->curRow << " oldRow=" << oldRow << " d->curCol=" << d->curCol << " oldCol=" << oldCol;

    if (forceSet || m_curRow != newrow || m_curCol != newcol) {
        kDebug() << QString("old:%1,%2 new:%3,%4").arg(m_curCol)
            .arg(m_curRow).arg(newcol).arg(newrow);

        // cursor moved: get rid of editor
        if (m_editor) {
            if (!m_contentsMousePressEvent_dblClick) {
                if (!acceptEditor()) {
                    return;
                }
                //update row num. again
                newrow = qMin(rows() - 1 + (isInsertingEnabled() ? 1 : 0), newrow);
            }
        }
        if (m_errorMessagePopup) {
            m_errorMessagePopup->animatedHide();
        }

        if ((m_curRow != newrow || forceSet) && m_navPanel)  {//update current row info
            m_navPanel->setCurrentRecordNumber(newrow + 1);
        }

        // cursor moved to other row: end of row editing
        bool newRowInserted = false;
        if (m_rowEditing && m_curRow != newrow) {
            newRowInserted = m_newRowEditing;
            if (!acceptRowEdit()) {
                //accepting failed: cancel setting the cursor
                return;
            }
            //update row number, because number of rows changed
            newrow = qMin(rows() - 1 + (isInsertingEnabled() ? 1 : 0), newrow);

            if (m_navPanel)
                m_navPanel->setCurrentRecordNumber(newrow + 1); //refresh
        }

        //change position
        int oldRow = m_curRow;
        int oldCol = m_curCol;
        m_curRow = newrow;
        m_curCol = newcol;

        //show editor-dependent focus, if we're changing the current column
        if (oldCol >= 0 && oldCol < columns() && m_curCol != oldCol) {
            //find the editor for this column
            KexiDataItemInterface *edit = editor(oldCol);
            if (edit) {
                edit->hideFocus();
            }
        }

        // position changed, so subsequent searching should be started from scratch
        // (e.g. from the current cell or the top-left cell)
        m_positionOfRecentlyFoundValue.exists = false;

        //show editor-dependent focus, if needed
        editorShowFocus(m_curRow, m_curCol);

        if (m_updateEntireRowWhenMovingToOtherRow)
            updateRow(oldRow);
        else
            updateCell(oldRow, oldCol);

        if (m_verticalHeader && (oldRow != m_curRow || forceSet))
            m_verticalHeader->setCurrentRow(m_curRow);

        if (m_updateEntireRowWhenMovingToOtherRow)
            updateRow(m_curRow);
        else
            updateCell(m_curRow, m_curCol);

        if (m_curCol != oldCol || m_curRow != oldRow || forceSet) {//ensure this is also refreshed
            if (!m_updateEntireRowWhenMovingToOtherRow) //only if entire row has not been updated
                updateCell(oldRow, m_curCol);
        }
        //update row
        if (forceSet || m_curRow != oldRow) {
            if (isInsertingEnabled() && m_curRow == rows()) {
                kDebug() << "NOW insert item is current";
                m_currentItem = m_insertItem;
                m_itemIterator = KexiDB::TableViewData::Iterator();
            } else {
                kDebug() << QString("NOW item at %1 (%2) is current")
                    .arg(m_curRow).arg((ulong)itemAt(m_curRow));
// #define setCursorPosition_DEBUG
#ifdef setCursorPosition_DEBUG
                int _i = 0;
                kDebug() << "m_curRow:" << m_curRow;
                for (KexiDB::TableViewData::Iterator ii = m_data->constBegin();
                     ii != m_data->constEnd(); ++ii)
                {
                    kDebug() << _i << (ulong)(*ii)
                             << (ii == m_itemIterator ? "CURRENT" : "")
                             << (*ii)->debugString();
                    _i++;
                }
                kDebug() << "~" << m_curRow << (ulong)(*m_itemIterator)
                         << (*m_itemIterator)->debugString();
#endif
                if (   !newRowInserted && isInsertingEnabled() && m_currentItem == m_insertItem
                    && m_curRow == (rows() - 1))
                {
                    //moving from the 'insert item' to the last item
                    m_itemIterator = m_data->constBegin();
                    m_itemIterator += (m_data->count() - 1);
                }
                else if (!newRowInserted && !forceSet && m_currentItem != m_insertItem && 0 == m_curRow) {
                    m_itemIterator = m_data->constBegin();
                }
                else if (   !newRowInserted && !forceSet && m_currentItem != m_insertItem
                         && oldRow >= 0 && (oldRow + 1) == m_curRow)
                {
                    ++m_itemIterator; // just move next
                }
                else if (   !newRowInserted && !forceSet && m_currentItem != m_insertItem
                         && oldRow >= 0 && (oldRow - 1) == m_curRow)
                {
                    --m_itemIterator; // just move back
                }
                else { //move at:
                    m_itemIterator = m_data->constBegin();
                    m_itemIterator += m_curRow;
                }
                if (!*m_itemIterator) { //sanity
                    m_itemIterator = m_data->constBegin();
                    m_itemIterator += m_curRow;
                }
                m_currentItem = *m_itemIterator;
#ifdef setCursorPosition_DEBUG
                kDebug() << "new~" << m_curRow
                         << (ulong)(*m_itemIterator) << (*m_itemIterator)->debugString();
#endif
            }
        }

        //quite clever: ensure the cell is visible:
        ensureCellVisible(m_curRow, m_curCol);

        /*emit*/ itemSelected(m_currentItem);
        /*emit*/ cellSelected(m_curCol, m_curRow);
        /* only needed for forms */
        selectCellInternal();
    } else {
        kDebug() << "NO CHANGE";
    }

    if (m_initDataContentsOnShow) {
        m_cursorPositionSetExplicityBeforeShow = true;
    }
}

bool KexiDataAwareObjectInterface::acceptRowEdit()
{
    if (!m_rowEditing || /*sanity*/!m_data->rowEditBuffer() || m_inside_acceptRowEdit)
        return true;
    if (m_inside_acceptEditor) {
        m_internal_acceptsRowEditAfterCellAccepting = true;
        return true;
    }
    m_inside_acceptRowEdit = true; // avoid recursion
    KexiUtils::Setter<bool> acceptRowEditSetter(&m_inside_acceptRowEdit, false);

    m_internal_acceptsRowEditAfterCellAccepting = false;

    const int columnEditedBeforeAccepting = m_editor ? currentColumn() : -1;
    if (!acceptEditor()) {
        return false;
    }
    kDebug() << "EDIT RECORD ACCEPTING...";

    bool success = true;
    const bool inserting = m_newRowEditing;

    if (m_data->rowEditBuffer()->isEmpty() && !m_newRowEditing) {
        kDebug() << "-- NOTHING TO ACCEPT!!!";
    } else {//not empty edit buffer or new row to insert:
        if (m_newRowEditing) {
            kDebug() << "-- INSERTING: ";
            m_data->rowEditBuffer()->debug();
            success = m_data->saveNewRow(*m_currentItem);
        }
        else {
            if (success) {
                //accept changes for this row:
                kDebug() << "-- UPDATING: ";
                m_data->rowEditBuffer()->debug();
                kDebug() << "-- BEFORE: ";
                m_currentItem->debug();
                success = m_data->saveRowChanges(*m_currentItem);
                kDebug() << "-- AFTER: ";
                m_currentItem->debug();
            }
        }
    }

    if (success) {
        //editing is finished:
        if (m_newRowEditing) {
            //update current-item-iterator
            setCursorPosition(-1, -1, true /*forceSet*/);
        }
        m_rowEditing = false;
        m_newRowEditing = false;
        //indicate on the vheader that we are not editing
        if (m_verticalHeader)
            m_verticalHeader->setEditRow(-1);

        updateAfterAcceptRowEdit();
        kDebug() << "EDIT RECORD ACCEPTED:";

        if (inserting) {
            //update navigator's data
            if (m_navPanel)
                m_navPanel->setRecordCount(rows());
        }
        /*emit*/ rowEditTerminated(m_curRow);
    } else {
        int faultyColumn = -1;
        if (m_data->result().column >= 0 && m_data->result().column < columns())
            faultyColumn = m_data->result().column;
        else if (columnEditedBeforeAccepting >= 0)
            faultyColumn = columnEditedBeforeAccepting;
        if (faultyColumn >= 0) {
            setCursorPosition(m_curRow, faultyColumn);
        }

        const int button = showErrorMessageForResult(m_data->result());
        if (KMessageBox::No == button) {
            //discard changes
            cancelRowEdit();
        } else {
            if (faultyColumn >= 0) {
                //edit this cell
                startEditCurrentCell();
            }
        }
    }
    return success;
}

bool KexiDataAwareObjectInterface::cancelRowEdit()
{
    if (!hasData())
        return true;
    if (!m_rowEditing)
        return true;
    cancelEditor();
    m_rowEditing = false;
    //indicate on the vheader that we are not editing
    if (m_verticalHeader)
        m_verticalHeader->setEditRow(-1);
    m_alsoUpdateNextRow = m_newRowEditing;
    if (m_newRowEditing) {
        m_newRowEditing = false;
        //remove current edited row (it is @ the end of list)
        m_data->removeLast();
        //current item is now empty, last row
        m_currentItem = m_insertItem;
        //update visibility
        if (m_verticalHeader)
            m_verticalHeader->removeLabel(false); //-1 label
        updateWidgetContents();
        updateWidgetContentsSize();
        //--no cancel action is needed for datasource,
        //  because the row was not yet stored.
    }

    m_data->clearRowEditBuffer();
    updateAfterCancelRowEdit();

//! \todo (js): cancel changes for this row!
    kDebug() << "EDIT RECORD CANCELLED.";

    /*emit*/ rowEditTerminated(m_curRow);
    return true;
}

void KexiDataAwareObjectInterface::updateAfterCancelRowEdit()
{
    updateRow(m_curRow);
    if (m_alsoUpdateNextRow)
        updateRow(m_curRow + 1);
    m_alsoUpdateNextRow = false;
}

void KexiDataAwareObjectInterface::updateAfterAcceptRowEdit()
{
    updateRow(m_curRow);
}

void KexiDataAwareObjectInterface::removeEditor()
{
    if (!m_editor)
        return;
    m_editor->hideWidget();
    m_editor = 0;
}

bool KexiDataAwareObjectInterface::cancelEditor()
{
    if (m_errorMessagePopup) {
        m_errorMessagePopup->animatedHide();
    }
    if (!m_editor)
        return true;
    removeEditor();
    return true;
}

bool KexiDataAwareObjectInterface::acceptEditor()
{
    if (!hasData())
        return true;
    if (!m_editor || m_inside_acceptEditor)
        return true;

    m_inside_acceptEditor = true; // avoid recursion
    KexiUtils::Setter<bool> acceptRowEditSetter(&m_inside_acceptEditor, false);

    QVariant newval;
    KexiDB::Validator::Result res = KexiDB::Validator::Ok;
    QString msg, desc;
    bool setNull = false;

    //autoincremented field can be omitted (left as null or empty) if we're inserting a new row
    const bool autoIncColumnCanBeOmitted = m_newRowEditing && m_editor->field()->isAutoIncrement();

    bool valueChanged = m_editor->valueChanged();
    if (valueChanged) {
        if (!m_editor->valueIsValid()) {
            //used e.g. for date or time values - the value can be null but not necessary invalid
            res = KexiDB::Validator::Error;
            //! @todo allow displaying user-defined warning
            showEditorContextMessage(
                        m_editor,
                        i18nc("Question", "Error: %1?", m_editor->columnInfo()->field->typeName()),
                        KMessageWidget::Error,
                        KMessageWidget::Up);
        }
        else if (m_editor->valueIsNull()) {//null value entered
            if (m_editor->field()->isNotNull() && !autoIncColumnCanBeOmitted) {
                kDebug() << "NULL NOT ALLOWED!";
                res = KexiDB::Validator::Error;
                msg = KexiDB::Validator::msgColumnNotEmpty().arg(m_editor->field()->captionOrName())
                      + "\n\n" + KexiDB::msgYouCanImproveData();
                desc = i18n("The column's constraint is declared as NOT NULL.");
            } else {
                kDebug() << "NULL VALUE WILL BE SET";
                //ok, just leave newval as NULL
                setNull = true;
            }
        } else if (m_editor->valueIsEmpty()) {//empty value entered
            if (m_editor->field()->hasEmptyProperty()) {
                if (m_editor->field()->isNotEmpty() && !autoIncColumnCanBeOmitted) {
                    kDebug() << "EMPTY NOT ALLOWED!";
                    res = KexiDB::Validator::Error;
                    msg = KexiDB::Validator::msgColumnNotEmpty().arg(m_editor->field()->captionOrName())
                          + "\n\n" + KexiDB::msgYouCanImproveData();
                    desc = i18n("The column's constraint is declared as NOT EMPTY.");
                } else {
                    kDebug() << "EMPTY VALUE WILL BE SET";
                }
            } else {
                if (m_editor->field()->isNotNull() && !autoIncColumnCanBeOmitted) {
                    kDebug() << "NEITHER NULL NOR EMPTY VALUE CAN BE SET!";
                    res = KexiDB::Validator::Error;
                    msg = KexiDB::Validator::msgColumnNotEmpty().arg(m_editor->field()->captionOrName())
                          + "\n\n" + KexiDB::msgYouCanImproveData();
                    desc = i18n("The column's constraint is declared as NOT EMPTY and NOT NULL.");
                } else {
                    kDebug() << "NULL VALUE WILL BE SET BECAUSE EMPTY IS NOT ALLOWED";
                    //ok, just leave newval as NULL
                    setNull = true;
                }
            }
        }
        else {
            // try to fixup the value before accepting, e.g. trim the text
            if (!m_editor->fixup()) {
                res = KexiDB::Validator::Error;
            }
            if (m_errorMessagePopup) {
                m_errorMessagePopup->animatedHide();
            }
            if (res != KexiDB::Validator::Ok) {
                //! @todo display message related to failed fixup if needed
            }
            //! @todo after fixup we may want to apply validation rules again
        }
    }//changed

    const int realFieldNumber = fieldNumberForColumn(m_curCol);
    if (realFieldNumber < 0) {
        kWarning() << "fieldNumberForColumn(m_curCol) < 0";
        return false;
    }

    KexiDB::TableViewColumn *currentTVColumn = column(m_curCol);

    //try to get the value entered:
    if (res == KexiDB::Validator::Ok) {
        if (   (!setNull && !valueChanged)
            || (m_editor->field()->type() != KexiDB::Field::Boolean && setNull && m_currentItem->at(realFieldNumber).isNull()))
        {
            kDebug() << "VALUE NOT CHANGED.";
            removeEditor();
            if (m_acceptsRowEditAfterCellAccepting || m_internal_acceptsRowEditAfterCellAccepting)
                acceptRowEdit();
            return true;
        }
        if (!setNull) {//get the new value
            newval = m_editor->value();
//! @todo validation rules for this value?
        }

        //Check other validation rules:
        //1. check using validator
        KexiDB::Validator *validator = currentTVColumn->validator();
        if (validator) {
            res = validator->check(currentTVColumn->field()->captionOrName(),
                                   newval, msg, desc);
        }
    }

    //show the validation result if not OK:
    if (res == KexiDB::Validator::Error) {
        if (!msg.isEmpty()) {
            if (desc.isEmpty())
                KMessageBox::sorry(dynamic_cast<QWidget*>(this), msg);
            else
                KMessageBox::detailedSorry(dynamic_cast<QWidget*>(this), msg, desc);
        }
    } else if (res == KexiDB::Validator::Warning) {
        //! @todo: message
        KMessageBox::messageBox(dynamic_cast<QWidget*>(this), KMessageBox::Sorry, msg + "\n" + desc);
    }

    if (res == KexiDB::Validator::Ok) {
        //2. check using signal
        //send changes to the backend
        QVariant visibleValue;
        if (   !newval.isNull()/* visible value should be null if value is null */
            && currentTVColumn->visibleLookupColumnInfo())
        {
            visibleValue = m_editor->visibleValue(); //visible value for lookup field
        }
        //should be also added to the buffer
        if (m_data->updateRowEditBufferRef(m_currentItem, m_curCol, currentTVColumn,
                                           newval, /*allowSignals*/true, 
                                           currentTVColumn->visibleLookupColumnInfo() ? &visibleValue : 0))
        {
            kDebug() << "------ EDIT BUFFER CHANGED TO:";
            m_data->rowEditBuffer()->debug();
        } else {
            kDebug() << "------ CHANGE FAILED";
            res = KexiDB::Validator::Error;

            //now: there might be called cancelEditor() in updateRowEditBuffer() handler,
            //if this is true, d->pEditor is NULL.

            if (m_editor && m_data->result().column >= 0 && m_data->result().column < columns()) {
                //move to faulty column (if m_editor is not cleared)
                setCursorPosition(m_curRow, m_data->result().column);
            }
            if (!m_data->result().msg.isEmpty()) {
                const int button = showErrorMessageForResult(m_data->result());
                if (KMessageBox::No == button) {
                    //discard changes
                    cancelEditor();
                    if (m_acceptsRowEditAfterCellAccepting)
                        cancelRowEdit();
                    return false;
                }
            }
        }
    }

    if (res == KexiDB::Validator::Ok) {
        removeEditor();
        /*emit*/ itemChanged(m_currentItem, m_curRow, m_curCol,
                             m_currentItem->at(realFieldNumber));
        /*emit*/ itemChanged(m_currentItem, m_curRow, m_curCol);
    }
    if (res == KexiDB::Validator::Ok) {
        if (m_acceptsRowEditAfterCellAccepting || m_internal_acceptsRowEditAfterCellAccepting) {
            m_inside_acceptEditor = false;
            acceptRowEdit();
            m_inside_acceptEditor = true;
        }
        return true;
    }
    if (m_editor) {
        //allow to edit the cell again, (if m_pEditor is not cleared)

        if (m_editor->hasFocusableWidget()) {
            m_editor->showWidget();
            m_editor->setFocus();
        }
    }
    return false;
}

void KexiDataAwareObjectInterface::startEditCurrentCell(const QString &setText)
{
    kDebug() << "setText:" << setText;
    if (isReadOnly() || !columnEditable(m_curCol))
        return;
    if (m_editor) {
        if (m_editor->hasFocusableWidget()) {
            m_editor->showWidget();
            m_editor->setFocus();
        }
    }
    else {
        createEditor(m_curRow, m_curCol, setText, !setText.isEmpty());
    }
}

void KexiDataAwareObjectInterface::deleteAndStartEditCurrentCell()
{
    if (isReadOnly() || !columnEditable(m_curCol))
        return;
    if (m_editor) {//if we've editor - just clear it
        m_editor->clear();
        return;
    }
    ensureCellVisible(m_curRow + 1, m_curCol);
    createEditor(m_curRow, m_curCol, QString(), false/*removeOld*/);
    if (!m_editor)
        return;
    m_editor->clear();
    if (m_editor->acceptEditorAfterDeleteContents())
        acceptEditor();
    if (!m_editor || !m_editor->hasFocusableWidget())
        updateCell(m_curRow, m_curCol);
}

void KexiDataAwareObjectInterface::deleteCurrentRow()
{
    if (m_newRowEditing) {//we're editing fresh new row: just cancel this!
        cancelRowEdit();
        return;
    }

    if (!acceptRowEdit())
        return;

    if (!isDeleteEnabled() || !m_currentItem || m_currentItem == m_insertItem) {
        return;
    }
    switch (m_deletionPolicy) {
    case NoDelete:
        return;
    case ImmediateDelete:
        break;
    case AskDelete:
        if (KMessageBox::Cancel == KMessageBox::warningContinueCancel(
                    dynamic_cast<QWidget*>(this),
                    i18n("Do you want to delete selected record?"), QString(),
                    KGuiItem(i18n("&Delete Record"), koIconName("edit-delete")), KStandardGuiItem::cancel(),
                    "dontAskBeforeDeleteRow"/*config entry*/,
                    KMessageBox::Notify | KMessageBox::Dangerous))
        {
            return;
        }
        break;
    case SignalDelete:
        /*emit*/ itemDeleteRequest(m_currentItem, m_curRow, m_curCol);
        /*emit*/ currentItemDeleteRequest();
        return;
    default:
        return;
    }

    if (!deleteItem(m_currentItem)) {//nothing
    }
}

KexiDB::RecordData* KexiDataAwareObjectInterface::insertEmptyRow(int pos)
{
    if (!acceptRowEdit() || !isEmptyRowInsertingEnabled()
            || (pos != -1 && pos >= ((int)rows() + (isInsertingEnabled() ? 1 : 0))))
        return 0;

    KexiDB::RecordData *newRecord = m_data->createItem();
    insertItem(newRecord, pos);
    return newRecord;
}

void KexiDataAwareObjectInterface::insertItem(KexiDB::RecordData *newRecord, int pos)
{
    const bool changeCurrentRecord = pos == -1 || pos == m_curRow;
    if (changeCurrentRecord) {
        //change current record
        pos = (m_curRow >= 0 ? m_curRow : 0);
        m_currentItem = newRecord;
        m_curRow = pos;
    } else if (m_curRow >= pos) {
        m_curRow++;
    }
    m_data->insertRow(*newRecord, pos, true /*repaint*/);

    // always update iterator since the list was modified...
    m_itemIterator = m_data->constBegin();
    m_itemIterator += m_curRow;
}

void KexiDataAwareObjectInterface::slotRowInserted(KexiDB::RecordData* record, bool repaint)
{
    slotRowInserted(record, m_data->indexOf(record), repaint);
}

void KexiDataAwareObjectInterface::slotRowInserted(KexiDB::RecordData * /*record*/, uint pos, bool repaint)
{
    if (repaint && (int)pos < rows()) {
        updateWidgetContentsSize();
        updateAllVisibleRowsBelow(pos);
        if (!m_verticalHeaderAlreadyAdded) {
            if (m_verticalHeader)
                m_verticalHeader->addLabel();
        }
        else { //it was added because this inserting was interactive
            m_verticalHeaderAlreadyAdded = false;
        }

        //update navigator's data
        if (m_navPanel)
            m_navPanel->setRecordCount(rows());

        if (m_curRow >= (int)pos) {
            //update
            editorShowFocus(m_curRow, m_curCol);
        }
    }
}

tristate KexiDataAwareObjectInterface::deleteAllRows(bool ask, bool repaint)
{
    if (!hasData())
        return true;
    if (m_data->count() < 1)
        return true;

    if (ask) {
        QString tableName = m_data->dbTableName();
        if (!tableName.isEmpty()) {
            tableName.prepend(" \"");
            tableName.append("\"");
        }
        if (KMessageBox::Cancel == KMessageBox::warningContinueCancel(dynamic_cast<QWidget*>(this),
                i18n("Do you want to clear the contents of table %1?", tableName),
                0, KGuiItem(i18n("&Clear Contents"))))
        {
            return cancelled;
        }
    }

    cancelRowEdit();
    const bool repaintLater = repaint && m_spreadSheetMode;
    const int oldRows = rows();

    bool res = m_data->deleteAllRows(repaint && !repaintLater);

    if (res) {
        if (m_spreadSheetMode) {
            for (int i = 0; i < oldRows; i++) {
                m_data->append(m_data->createItem());
            }
        }
    }
    if (repaintLater)
        m_data->reload();

    return res;
}

void KexiDataAwareObjectInterface::clearColumns(bool repaint)
{
    cancelRowEdit();
    m_data->clearInternal();

    clearColumnsInternal(repaint);
    updateIndicesForVisibleValues();

    if (repaint)
        updateWidgetContents();
}

void KexiDataAwareObjectInterface::reloadData()
{
    acceptRowEdit();
    if (m_verticalHeader)
        m_verticalHeader->clear();

    if (m_curCol >= 0 && m_curCol < columns()) {
        //find the editor for this column
        KexiDataItemInterface *edit = editor(m_curCol);
        if (edit) {
            edit->hideFocus();
        }
    }
    clearVariables();
    if (m_verticalHeader)
        m_verticalHeader->setCurrentRow(-1);

    if (dynamic_cast<QWidget*>(this) && dynamic_cast<QWidget*>(this)->isVisible())
        initDataContents();
    else
        m_initDataContentsOnShow = true;

    if (m_verticalHeader)
        m_verticalHeader->addLabels(m_data->count());

    updateWidgetScrollBars();
}

int KexiDataAwareObjectInterface::columnType(int col)
{
    KexiDB::TableViewColumn* c = m_data ? column(col) : 0;
    return c ? c->field()->type() : KexiDB::Field::InvalidType;
}

bool KexiDataAwareObjectInterface::columnEditable(int col)
{
    KexiDB::TableViewColumn* c = m_data ? column(col) : 0;
    return c ? (! c->isReadOnly()) : false;
}

int KexiDataAwareObjectInterface::rows() const
{
    if (!hasData())
        return 0;
    return m_data->count();
}

int KexiDataAwareObjectInterface::dataColumns() const
{
    if (!hasData())
        return 0;
    return m_data->columnsCount();
}

QVariant KexiDataAwareObjectInterface::columnDefaultValue(int /*col*/) const
{
    return QVariant(0);
//! @todo return m_data->columns[col].defaultValue;
}

void KexiDataAwareObjectInterface::setAcceptsRowEditAfterCellAccepting(bool set)
{
    m_acceptsRowEditAfterCellAccepting = set;
}

void KexiDataAwareObjectInterface::setDropsAtRowEnabled(bool set)
{
    if (!set)
        m_dragIndicatorLine = -1;
    if (m_dropsAtRowEnabled && !set) {
        m_dropsAtRowEnabled = false;
        updateWidgetContents();
    } else {
        m_dropsAtRowEnabled = set;
    }
}

void KexiDataAwareObjectInterface::setEmptyRowInsertingEnabled(bool set)
{
    m_emptyRowInsertingEnabled = set;
    /*emit*/ reloadActions();
}

void KexiDataAwareObjectInterface::slotAboutToDeleteRow(KexiDB::RecordData& record,
        KexiDB::ResultInfo* /*result*/, bool repaint)
{
    if (repaint) {
        m_rowWillBeDeleted = m_data->indexOf(&record);
    }
}

void KexiDataAwareObjectInterface::slotRowDeleted()
{
    if (m_rowWillBeDeleted >= 0) {
        if (m_rowWillBeDeleted > 0 && m_rowWillBeDeleted >= (rows() - 1) && !m_spreadSheetMode)
            m_rowWillBeDeleted = rows() - 1; //move up if it's the last row
        updateWidgetContentsSize();

        if (!(m_spreadSheetMode && m_rowWillBeDeleted >= (rows() - 1)))
            setCursorPosition(m_rowWillBeDeleted, m_curCol, true/*forceSet*/);
        if (m_verticalHeader)
            m_verticalHeader->removeLabel();

        updateAllVisibleRowsBelow(m_curRow); //needed for KexiTableView

        //update navigator's data
        if (m_navPanel)
            m_navPanel->setRecordCount(rows());

        m_rowWillBeDeleted = -1;
    }
}

bool KexiDataAwareObjectInterface::beforeDeleteItem(KexiDB::RecordData*)
{
    //always return
    return true;
}

bool KexiDataAwareObjectInterface::deleteItem(KexiDB::RecordData* record)
{
    if (!record || !beforeDeleteItem(record))
        return false;

    QString msg, desc;
    //we need to know this so we
    //can return to the last row
    //after reinserting it
    const bool lastRowDeleted = m_spreadSheetMode && m_data->last() == record; 
    if (!m_data->deleteRow(*record, true /*repaint*/)) {
        /*const int button =*/
        showErrorMessageForResult(m_data->result());
        return false;
    }

    if (m_spreadSheetMode) { //append empty row for spreadsheet mode
        m_data->append(m_data->createItem());
        if (m_verticalHeader)
            m_verticalHeader->addLabels(1);
        if (lastRowDeleted) //back to the last row
            setCursorPosition(rows() - 1, m_curCol, true/*forceSet*/);
        /*emit*/ newItemAppendedForAfterDeletingInSpreadSheetMode();
    }
    return true;
}

KexiDB::TableViewColumn* KexiDataAwareObjectInterface::column(int col)
{
    return m_data->column(col);
}

bool KexiDataAwareObjectInterface::hasDefaultValueAt(const KexiDB::TableViewColumn& tvcol)
{
    if (m_rowEditing && m_data->rowEditBuffer() && m_data->rowEditBuffer()->isDBAware()) {
        return m_data->rowEditBuffer()->hasDefaultValueAt(*tvcol.columnInfo());
    }
    return false;
}

const QVariant* KexiDataAwareObjectInterface::bufferedValueAt(int col, bool useDefaultValueIfPossible)
{
    if (m_rowEditing && m_data->rowEditBuffer()) {
        KexiDB::TableViewColumn* tvcol = column(col);
        if (tvcol->isDBAware()) {
            //get the stored value
            const int realFieldNumber = fieldNumberForColumn(col);
            if (realFieldNumber < 0) {
                kWarning() << "fieldNumberForColumn(m_curCol) < 0";
                return 0;
            }
            const QVariant *storedValue = &m_currentItem->at(realFieldNumber);

            //db-aware data: now, try to find a buffered value (or default one)
            const QVariant *cv = m_data->rowEditBuffer()->at(*tvcol->columnInfo(),
                                 storedValue->isNull() && useDefaultValueIfPossible);
            if (cv)
                return cv;
            return storedValue;
        }
        //not db-aware data:
        const QVariant *cv = m_data->rowEditBuffer()->at(tvcol->field()->name());
        if (cv)
            return cv;
    }
    //not db-aware data:
    const int realFieldNumber = fieldNumberForColumn(col);
    if (realFieldNumber < 0) {
        kWarning() << "fieldNumberForColumn(m_curCol) < 0";
        return 0;
    }
    return &m_currentItem->at(realFieldNumber);
}

void KexiDataAwareObjectInterface::startEditOrToggleValue()
{
    if (!isReadOnly() && columnEditable(m_curCol)) {
        if (columnType(m_curCol) == KexiDB::Field::Boolean) {
            boolToggled();
        } else {
            startEditCurrentCell();
            return;
        }
    }
}

void KexiDataAwareObjectInterface::boolToggled()
{
    startEditCurrentCell();
    if (m_editor) {
        m_editor->clickedOnContents();
    }
    acceptEditor();
    updateCell(m_curRow, m_curCol);
}

void KexiDataAwareObjectInterface::slotDataDestroying()
{
    m_data = 0;
    m_itemIterator = KexiDB::TableViewData::Iterator();
}

void KexiDataAwareObjectInterface::addNewRecordRequested()
{
    if (!isInsertingEnabled())
        return;
    if (m_rowEditing) {
        if (!acceptRowEdit())
            return;
    }
    selectRow(rows());
    startEditCurrentCell();
    if (m_editor)
        m_editor->setFocus();
}

bool KexiDataAwareObjectInterface::handleKeyPress(QKeyEvent *e, int &curRow, int &curCol,
        bool fullRecordSelection, bool *moveToFirstField, bool *moveToLastField)
{
    if (moveToFirstField)
        *moveToFirstField = false;
    if (moveToLastField)
        *moveToLastField = false;

    const bool nobtn = e->modifiers() == Qt::NoModifier;
    const int k = e->key();

    if (   (k == Qt::Key_Up && nobtn)
        || (k == Qt::Key_PageUp && e->modifiers() == Qt::ControlModifier))
    {
        selectPrevRow();
        e->accept();
    }
    else if (   (k == Qt::Key_Down && nobtn)
               || (k == Qt::Key_PageDown && e->modifiers() == Qt::ControlModifier))
    {
        selectNextRow();
        e->accept();
    } else if (k == Qt::Key_PageUp && nobtn) {
        selectPrevPage();
        e->accept();
    } else if (k == Qt::Key_PageDown && nobtn) {
        selectNextPage();
        e->accept();
    } else if (k == Qt::Key_Home) {
        if (fullRecordSelection) {
            //we're in row-selection mode: home key always moves to 1st row
            curRow = 0;//to 1st row
        } else {//cell selection mode: different actions depending on ctrl and shift keys mods
            if (nobtn) {
                curCol = 0;//to 1st col
            } else if (e->modifiers() == Qt::ControlModifier) {
                curRow = 0;//to 1st row and col
                curCol = 0;
            } else
                return false;
        }
        if (moveToFirstField)
            *moveToFirstField = true;
        //do not accept yet
        e->ignore();
    } else if (k == Qt::Key_End) {
        if (fullRecordSelection) {
            //we're in row-selection mode: home key always moves to last row
            curRow = m_data->count() - 1 + (isInsertingEnabled() ? 1 : 0);//to last row
        } else {//cell selection mode: different actions depending on ctrl and shift keys mods
            if (nobtn) {
                curCol = columns() - 1;//to last col
            } else if (e->modifiers() == Qt::ControlModifier) {
                curRow = m_data->count() - 1 /*+(isInsertingEnabled()?1:0)*/; //to last row and col
                curCol = columns() - 1;//to last col
            } else
                return false;
        }
        if (moveToLastField)
            *moveToLastField = true;
        //do not accept yet
        e->ignore();
    }
    else if (isInsertingEnabled()
               && (   (e->modifiers() == Qt::ControlModifier && k == Qt::Key_Equal)
                   || (e->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier) && k == Qt::Key_Equal)
                  )
            )
    {
        curRow = m_data->count(); //to the new row
        curCol = 0;//to first col
        if (moveToFirstField)
            *moveToFirstField = true;
        //do not accept yet
        e->ignore();
    } else {
        return false;
    }

    return true;
}

void KexiDataAwareObjectInterface::vScrollBarValueChanged(int v)
{
    Q_UNUSED(v);
    if (!m_vScrollBarValueChanged_enabled)
        return;

    if (m_scrollbarToolTipsEnabled && verticalScrollBar()->isSliderDown()) {
        QWidget* thisWidget = dynamic_cast<QWidget*>(this);
        const int row = lastVisibleRow() + 1;
        const QString toolTipText( i18n("Record: %1", row) );
        QToolTip::showText(
            QPoint(
                thisWidget->mapToGlobal(verticalScrollBar()->pos()).x()
                    - thisWidget->fontMetrics().width(toolTipText+"    "),
                QCursor::pos().y() ),
            toolTipText,
            0, 
            QRect()
        );
    }
}

void KexiDataAwareObjectInterface::setContextMenuTitle(const QIcon &icon, const QString &text)
{
    m_contextMenuTitleIcon = icon;
    m_contextMenuTitleText = text;
    /*emit*/ reloadActions();
}

bool KexiDataAwareObjectInterface::scrollbarToolTipsEnabled() const
{
    return m_scrollbarToolTipsEnabled;
}

void KexiDataAwareObjectInterface::setScrollbarToolTipsEnabled(bool set)
{
    m_scrollbarToolTipsEnabled = set;
}

void KexiDataAwareObjectInterface::focusOutEvent(QFocusEvent* e)
{
    Q_UNUSED(e);
    updateCell(m_curRow, m_curCol);
}

int KexiDataAwareObjectInterface::showErrorMessageForResult(const KexiDB::ResultInfo& resultInfo)
{
    QWidget *thisWidget = dynamic_cast<QWidget*>(this);
    if (resultInfo.allowToDiscardChanges) {
        return KMessageBox::questionYesNo(thisWidget, resultInfo.msg
                                          + (resultInfo.desc.isEmpty() ? QString() : ("\n" + resultInfo.desc)),
                                          QString(),
                                          KGuiItem(i18nc("Correct Changes", "Correct"),
                                                   QString(),
                                                   i18n("Correct changes")),
                                          KGuiItem(i18n("Discard Changes")));
    }

    if (resultInfo.desc.isEmpty())
        KMessageBox::sorry(thisWidget, resultInfo.msg);
    else
        KMessageBox::detailedSorry(thisWidget, resultInfo.msg, resultInfo.desc);

    return KMessageBox::Ok;
}

void KexiDataAwareObjectInterface::updateIndicesForVisibleValues()
{
    m_indicesForVisibleValues.resize(m_data ? m_data->columnsCount() : 0);
    if (!m_data)
        return;
    for (uint i = 0; i < m_data->columnsCount(); i++) {
        KexiDB::TableViewColumn* tvCol = m_data->column(i);
        if (tvCol->columnInfo() && tvCol->columnInfo()->indexForVisibleLookupValue() != -1)
            // retrieve visible value from lookup field
            m_indicesForVisibleValues[ i ] = tvCol->columnInfo()->indexForVisibleLookupValue();
        else
            m_indicesForVisibleValues[ i ] = i;
    }
}

/*! Performs searching \a stringValue in \a where string.
 \a matchAnyPartOfField, \a matchWholeField, \a wholeWordsOnly options are used to control how to search.

 If \a matchWholeField is true, \a wholeWordsOnly is not checked.
 \a firstCharacter is in/out parameter. If \a matchAnyPartOfField is true and \a matchWholeField is false,
 \a firstCharacter >= 0, the search will be performed after skipping first \a firstCharacter characters.

 If \a forward is false, we are searching backward from \a firstCharacter position. \a firstCharacter == -1
 means then the last character. \a firstCharacter == INT_MAX means "before first" place, so searching fails
 immediately.
 On success, true is returned and \a firstCharacter is set to position of the matched string. */
static inline bool findInString(const QString& stringValue, int stringLength,
                                const QString& where,
                                int& firstCharacter, bool matchAnyPartOfField, bool matchWholeField,
                                Qt::CaseSensitivity caseSensitivity, bool wholeWordsOnly, bool forward)
{
    if (where.isEmpty()) {
        firstCharacter = -1;
        return false;
    }

    if (matchAnyPartOfField) {
        if (forward) {
            int pos = firstCharacter == -1 ? 0 : firstCharacter;
            if (wholeWordsOnly) {
                const int whereLength = where.length();
                while (true) {
                    pos = where.indexOf(stringValue, pos, caseSensitivity);
                    if (pos == -1)
                        break;
                    if (   (pos > 0 && where.at(pos - 1).isLetterOrNumber())
                        || ((pos + stringLength - 1) < (whereLength - 1) && where.at(pos + stringLength - 1 + 1).isLetterOrNumber()))
                    {
                        pos++; // invalid match because before or after the string there is non-white space
                    } else
                        break;
                }//while
                firstCharacter = pos;
            } else {// !wholeWordsOnly
                firstCharacter = where.indexOf(stringValue, pos, caseSensitivity);
            }
            return firstCharacter != -1;
        } else { // !matchAnyPartOfField
            if (firstCharacter == INT_MAX) {
                firstCharacter = -1; //next time we'll be looking at different cell
                return false;
            }
            int pos = firstCharacter;
            if (wholeWordsOnly) {
                const int whereLength = where.length();
                while (true) {
                    pos = where.lastIndexOf(stringValue, pos, caseSensitivity);
                    if (pos == -1)
                        break;
                    if (   (pos > 0 && where.at(pos - 1).isLetterOrNumber())
                        || ((pos + stringLength - 1) < (whereLength - 1) && where.at(pos + stringLength - 1 + 1).isLetterOrNumber()))
                    {
                        // invalid match because before or after the string there is non-white space
                        pos--;
                        if (pos < 0) // it can make pos < 0
                            break;
                    } else
                        break;
                }//while
                firstCharacter = pos;
            } else {// !wholeWordsOnly
                firstCharacter = where.lastIndexOf(stringValue, pos, caseSensitivity);
            }
            return firstCharacter != -1;
        }
    } else if (matchWholeField) {
        if (firstCharacter != -1 && firstCharacter != 0) { //we're not at 0-th char
            firstCharacter = -1;
        } else if ((caseSensitivity == Qt::CaseSensitive ? where : where.toLower()) == stringValue) {
            firstCharacter = 0;
            return true;
        }
    } else {// matchStartOfField
        if (firstCharacter != -1 && firstCharacter != 0) { //we're not at 0-th char
            firstCharacter = -1;
        } else if (where.startsWith(stringValue, caseSensitivity)) {
            if (wholeWordsOnly) {
                // If where.length() < stringValue.length(), true will be returned too - fine.
                return !where.at(stringValue.length()).isLetterOrNumber();
            }
            firstCharacter = 0;
            return true;
        }
    }
    return false;
}

tristate KexiDataAwareObjectInterface::find(const QVariant& valueToFind,
        const KexiSearchAndReplaceViewInterface::Options& options, bool next)
{
    if (!hasData())
        return cancelled;
    const QVariant prevSearchedValue(m_recentlySearchedValue);
    m_recentlySearchedValue = valueToFind;
    const KexiSearchAndReplaceViewInterface::Options::SearchDirection prevSearchDirection = m_recentSearchDirection;
    m_recentSearchDirection = options.searchDirection;
    if (valueToFind.isNull() || valueToFind.toString().isEmpty())
        return cancelled;

    const bool forward = (options.searchDirection == KexiSearchAndReplaceViewInterface::Options::SearchUp)
                         ? !next : next; //direction can be reversed

    if ((!prevSearchedValue.isNull() && prevSearchedValue != valueToFind)
            || (prevSearchDirection != options.searchDirection && options.searchDirection == KexiSearchAndReplaceViewInterface::Options::SearchAllRows))
    {
        // restart searching when value has been changed or new direction is SearchAllRows
        m_positionOfRecentlyFoundValue.exists = false;
    }

    const bool startFrom1stRowAndCol = !m_positionOfRecentlyFoundValue.exists && next
                                       && options.searchDirection == KexiSearchAndReplaceViewInterface::Options::SearchAllRows;
    const bool startFromLastRowAndCol =
           (   !m_positionOfRecentlyFoundValue.exists && !next
            && options.searchDirection == KexiSearchAndReplaceViewInterface::Options::SearchAllRows)
        || (m_curRow >= rows() && !forward); //we're at "insert" record, and searching backwards: move to the last cell

    if (!startFrom1stRowAndCol && !startFromLastRowAndCol && m_curRow >= rows()) {
        //we're at "insert" record, and searching forward: no chances to find something
        return false;
    }
    KexiDB::TableViewData::Iterator it((startFrom1stRowAndCol || startFromLastRowAndCol)
                                   ? m_data->constBegin() : m_itemIterator /*start from the current cell*/);
    if (startFromLastRowAndCol)
        it += (m_data->columnsCount() - 1);
    int firstCharacter;
    if (m_positionOfRecentlyFoundValue.exists) {// start after the next/prev char position
        if (forward)
            firstCharacter = m_positionOfRecentlyFoundValue.lastCharacter + 1;
        else {
            firstCharacter = (m_positionOfRecentlyFoundValue.firstCharacter > 0) ?
                             (m_positionOfRecentlyFoundValue.firstCharacter - 1) : INT_MAX /* this means 'before first'*/;
        }
    } else {
        firstCharacter = -1; //forward ? -1 : INT_MAX;
    }

    const int columnsCount = m_data->columnsCount();
    int row, col;
    if (startFrom1stRowAndCol) {
        row = 0;
        col = 0;
    } else if (startFromLastRowAndCol) {
        row = rows() - 1;
        col = columnsCount - 1;
    } else {
        row = m_curRow;
        col = m_curCol;
    }

    //sache some flags for efficiency
    const bool matchAnyPartOfField
        = options.textMatching == KexiSearchAndReplaceViewInterface::Options::MatchAnyPartOfField;
    const bool matchWholeField
        = options.textMatching == KexiSearchAndReplaceViewInterface::Options::MatchWholeField;
    const Qt::CaseSensitivity caseSensitivity
        = options.caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;
    const bool wholeWordsOnly = options.wholeWordsOnly;
    int columnNumber = (options.columnNumber == KexiSearchAndReplaceViewInterface::Options::CurrentColumn)
                       ? m_curCol : options.columnNumber;
    if (columnNumber >= 0)
        col = columnNumber;
    const bool lookInAllColumns = columnNumber == KexiSearchAndReplaceViewInterface::Options::AllColumns;
    int firstColumn; // real number of the first column, can be smaller than lastColumn if forward==true
    int lastColumn; // real number of the last column
    if (lookInAllColumns) {
        firstColumn = forward ? 0 : columnsCount - 1;
        lastColumn = forward ? columnsCount - 1 : 0;
    } else {
        firstColumn = columnNumber;
        lastColumn = columnNumber;
    }
    const QString stringValue(
        caseSensitivity == Qt::CaseSensitive
        ? valueToFind.toString() : valueToFind.toString().toLower());
    const int stringLength = stringValue.length();

    // search
    const int prevRow = m_curRow;
    KexiDB::RecordData *record = 0;
    while ((it != m_data->constEnd() && (record = *it))) {
        for (; forward ? col <= lastColumn : col >= lastColumn;
                col = forward ? (col + 1) : (col - 1)) {
            const QVariant cell(record->at(m_indicesForVisibleValues[ col ]));
            if (findInString(stringValue, stringLength, cell.toString(), firstCharacter,
                             matchAnyPartOfField, matchWholeField, caseSensitivity, 
                             wholeWordsOnly, forward))
            {
                setCursorPosition(row, col, true/*forceSet*/);
                if (prevRow != m_curRow)
                    updateRow(prevRow);
                // remember the exact position for the found value
                m_positionOfRecentlyFoundValue.exists = true;
                m_positionOfRecentlyFoundValue.firstCharacter = firstCharacter;
//! @todo for regexp lastCharacter should be computed
                m_positionOfRecentlyFoundValue.lastCharacter = firstCharacter + stringLength - 1;
                return true;
            }
        }//for
        if (forward) {
            ++it;
            ++row;
        } else {
            --it;
            --row;
        }
        col = firstColumn;
    }//while
    return false;
}

tristate KexiDataAwareObjectInterface::findNextAndReplace(
    const QVariant& valueToFind, const QVariant& replacement,
    const KexiSearchAndReplaceViewInterface::Options& options, bool replaceAll)
{
    Q_UNUSED(replacement);
    Q_UNUSED(options);
    Q_UNUSED(replaceAll);

    if (isReadOnly())
        return cancelled;
    if (valueToFind.isNull() || valueToFind.toString().isEmpty())
        return cancelled;
    //! @todo implement KexiDataAwareObjectInterface::findAndReplace()
    return false;
}

void KexiDataAwareObjectInterface::setRowEditing(bool set)
{
    if (set == m_rowEditing)
        return;
    m_rowEditing = set;
    if (m_rowEditing)
        emit rowEditStarted(m_curRow);
    else
        emit rowEditTerminated(m_curRow);
}

int KexiDataAwareObjectInterface::horizontalHeaderHeight() const
{
    return 0;
}

void KexiDataAwareObjectInterface::showEditorContextMessage(
        KexiDataItemInterface *item,
        const QString &message,
        KMessageWidget::MessageType type,
        KMessageWidget::CalloutPointerDirection direction)
{
    QWidget *par = dynamic_cast<QScrollArea*>(this)
                   ? dynamic_cast<QScrollArea*>(this)->widget() : dynamic_cast<QWidget*>(this);
    QWidget *edit = dynamic_cast<QWidget*>(item);
    if (par && edit) {
        delete m_errorMessagePopup;
        KexiContextMessage msg(message);
        m_errorMessagePopup = new KexiContextMessageWidget(dynamic_cast<QWidget*>(this), 0, 0, msg);
        QPoint arrowPos = par->mapToGlobal(edit->pos()) + QPoint(12, edit->height() + 6);
        if (m_verticalHeader) {
            arrowPos += QPoint(m_verticalHeader->width(), horizontalHeaderHeight());
        }
        m_errorMessagePopup->setMessageType(type);
        m_errorMessagePopup->setCalloutPointerDirection(direction);
        m_errorMessagePopup->setCalloutPointerPosition(arrowPos);
        m_errorMessagePopup->setWordWrap(false);
        m_errorMessagePopup->setClickClosesMessage(true);
        m_errorMessagePopup->resizeToContents();
        QObject::connect(m_errorMessagePopup, SIGNAL(animatedHideFinished()),
                         edit, SLOT(setFocus()));
        m_errorMessagePopup->animatedShow();

        edit->setFocus();
    }
}

static QString lengthExceededMessage(KexiDataItemInterface *item)
{
    return i18np(
        "Limit of %2 characters for <resource>%3</resource> field has been exceeded by %1 character.\n"
        "Fix the text or it will be truncated upon saving changes.",
        "Limit of %2 characters for <resource>%3</resource> field has been exceeded by %1 characters.\n"
        "Fix the text or it will be truncated upon saving changes.",
        item->value().toString().length() - item->columnInfo()->field->maxLength(),
        item->columnInfo()->field->maxLength(),
        item->columnInfo()->captionOrAliasOrName());
}

void KexiDataAwareObjectInterface::showLengthExceededMessage(KexiDataItemInterface *item, bool exceeded)
{
    if (exceeded) {
        if (item) {
            showEditorContextMessage(
                item,
                lengthExceededMessage(item),
                KMessageWidget::Warning,
                KMessageWidget::Up);
            m_lengthExceededMessageVisible = true;
        }
    }
    else {
         if (m_errorMessagePopup) {
             m_errorMessagePopup->animatedHide();
             m_lengthExceededMessageVisible = false;
         }
    }
}

void KexiDataAwareObjectInterface::showUpdateForLengthExceededMessage(KexiDataItemInterface *item)
{
    if (m_errorMessagePopup && m_lengthExceededMessageVisible) {
        m_errorMessagePopup->setText(lengthExceededMessage(item));
        m_errorMessagePopup->resizeToContents();
    }
}
