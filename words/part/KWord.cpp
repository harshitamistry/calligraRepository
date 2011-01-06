/* This file is part of the KDE project
 * Copyright (C) 2008 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KWord.h"
#include "frames/KWTextFrameSet.h"

namespace KWord
{
bool isAutoGenerated(KWFrameSet *fs)
{
    Q_ASSERT(fs);
    if (fs->type() == BackgroundFrameSet)
        return true;
    if (fs->type() == TextFrameSet) {
        KWTextFrameSet *tfs = static_cast<KWTextFrameSet*>(fs);
        return tfs
            && (tfs->textFrameSetType() == KWord::OddPagesHeaderTextFrameSet
                    || tfs->textFrameSetType() == KWord::EvenPagesHeaderTextFrameSet
                    || tfs->textFrameSetType() == KWord::OddPagesFooterTextFrameSet
                    || tfs->textFrameSetType() == KWord::EvenPagesFooterTextFrameSet
                    || tfs->textFrameSetType() == KWord::MainTextFrameSet);
    }
    return false;
}

bool isHeaderFooter(KWTextFrameSet *tfs)
{
    return tfs
        && (tfs->textFrameSetType() == KWord::OddPagesHeaderTextFrameSet
                || tfs->textFrameSetType() == KWord::EvenPagesHeaderTextFrameSet
                || tfs->textFrameSetType() == KWord::OddPagesFooterTextFrameSet
                || tfs->textFrameSetType() == KWord::EvenPagesFooterTextFrameSet);
}
}