/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_document_undo_store.h"

#include "KoDocument.h"
#include <kundo2stack.h>


/*****************************************************************/
/*                KisDocumentUndoStore                           */
/*****************************************************************/

KisDocumentUndoStore::KisDocumentUndoStore(KoDocument *doc)
    : m_doc(doc)
{
}

const KUndo2Command* KisDocumentUndoStore::presentCommand()
{
    return m_doc->undoStack()->command(m_doc->undoStack()->index() - 1);
}

void KisDocumentUndoStore::undoLastCommand()
{
    /**
     * FIXME: Added as a workaround for being able to cancel creation
     * of the new adjustment mask (or any other mask whose
     * creation can be cancelled).
     *
     * Ideally, we should use "addToIndex-commit" technique like git does.
     * When a user presses Create button, we call command->redo()
     * and save this command in a cache. When the user confirms creation
     * of the layer with "OK" button, we "commit" the command to the undoStack.
     * If the user changes his mind and presses Cancel, we just call
     * command->undo() and remove the cache without committing it
     * to the undoStack
     */
    m_doc->undoStack()->undo();
}

void KisDocumentUndoStore::addCommand(KUndo2Command *command)
{
    if(!command) return;
    m_doc->addCommand(command);
}

void KisDocumentUndoStore::beginMacro(const KUndo2MagicString& macroName)
{
    m_doc->beginMacro(macroName);
}

void KisDocumentUndoStore::endMacro()
{
    m_doc->endMacro();
}
