/*
 *  kis_tool_colorchanger.cc - part of Krayon
 *
 *  Copyright (c) 2000 John Califf <jcaliff@compuzone.net>
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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <kdebug.h>

#include "kis_doc.h"
#include "kis_view.h"
#include "kis_cursor.h"
#include "kis_tool_colorchanger.h"
#include "opts_fill_dlg.h"


ColorChangerTool::ColorChangerTool(KisDoc *doc, KisView *view)
  : KisTool(doc, view)
{
    m_Cursor = KisCursor::pickerCursor();
    
    fillOpacity = 255;

    toleranceRed = 0;
    toleranceGreen = 0;
    toleranceBlue = 0;

    layerAlpha = true;
}

ColorChangerTool::~ColorChangerTool() {}


bool ColorChangerTool::changeColors(int startX, int startY)
{
    int startx = startX;
    int starty = startY;
    
    KisImage *img = m_pDoc->current();
    if (!img) return false;    

    KisLayer *lay = img->getCurrentLayer();
    if (!lay) return false;

    if (!img->colorMode() == cm_RGB && !img->colorMode() == cm_RGBA)
	    return false;
    
    layerAlpha = (img->colorMode() == cm_RGBA);
    fLayer = lay;
    
    // source color values of selected pixed
    sRed    = lay->pixel(0, startx, starty);
    sGreen  = lay->pixel(1, startx, starty);
    sBlue   = lay->pixel(2, startx, starty);

    // new color values from color selector 

    nRed     = m_pView->fgColor().R();
    nGreen   = m_pView->fgColor().G();
    nBlue    = m_pView->fgColor().B();
    
    int left    = lay->imageExtents().left(); 
    int top     = lay->imageExtents().top();    
    int width   = lay->imageExtents().width();    
    int height  = lay->imageExtents().height();    

    QRect ur(left, top, width, height);
    
    kdDebug() << "ur.left() " << ur.left() 
              << "ur.top() "  << ur.top() << endl;

    if(!m_pDoc->frameBuffer()->changeColors(qRgba(sRed, sGreen, sBlue, 255), 
        qRgba(nRed, nGreen, nBlue, 255), ur))
    {     
        kdDebug() << "error changing colors" << endl;
        return false;    
    } 
    
    /* refresh canvas so changes show up */
    img->markDirty(ur);
  
    return true;
}


void ColorChangerTool::mousePress(QMouseEvent *e)
{
    KisImage * img = m_pDoc->current();
    if (!img) return;

    if (e->button() != QMouseEvent::LeftButton
    && e->button() != QMouseEvent::RightButton)
        return;

    QPoint pos = e->pos();
    pos = zoomed(pos);
        
    if( !img->getCurrentLayer()->visible() )
        return;
  
    if( !img->getCurrentLayer()->imageExtents().contains(pos))
        return;
  
    /*  need to fill with foreground color on left click,
    transparent on middle click, and background color on right click,
    need another paramater or to set color here and pass in */
    
    if (e->button() == QMouseEvent::LeftButton)
        changeColors(pos.x(), pos.y());
    else if (e->button() == QMouseEvent::RightButton)
        changeColors(pos.x(), pos.y());
}

void ColorChangerTool::optionsDialog()
{
    FillOptionsDialog *pOptsDialog = new FillOptionsDialog(fillOpacity, 
       toleranceRed, toleranceGreen, toleranceBlue);
    pOptsDialog->exec();
    if(!pOptsDialog->result() == QDialog::Accepted)
        return;

    fillOpacity = pOptsDialog->opacity();
    
    toleranceRed = pOptsDialog->ToleranceRed();
    toleranceGreen = pOptsDialog->ToleranceGreen();    
    toleranceBlue = pOptsDialog->ToleranceBlue();    
}
