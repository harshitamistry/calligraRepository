/* This file is part of the KDE project
 * Copyright (C) 2012 Dan Leinir Turthra Jensen <admin@leinir.dk>
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

import QtQuick 1.1
import org.krita.sketch.components 1.0

Column {
    id: base
    property bool fullView: true;
    height: childrenRect.height;
    spacing: Constants.DefaultMargin;

    function apply() {
        toolManager.currentTool.crop();
    }

    ExpandingListView {
        id: cropTypeList;
        width: parent.width;
        visible: fullView;
        expandedHeight: Constants.GridHeight * 2;
        currentIndex: toolManager.currentTool.cropType !== undefined ? toolManager.currentTool.cropType : 0;
        onCurrentIndexChanged: if (toolManager.currentTool && toolManager.currentTool.cropType !== undefined) toolManager.currentTool.cropType = currentIndex;
        model: ListModel {
            ListElement {
                text: "Layer";
            }
            ListElement {
                text: "Image";
            }
        }
    }
    ExpandingListView {
        id: cropDecorationList;
        width: parent.width;
        visible: fullView;
        expandedHeight: Constants.GridHeight * 3;
        currentIndex: toolManager.currentTool.decoration !== undefined ? toolManager.currentTool.decoration : 0;
        onCurrentIndexChanged: if (toolManager.currentTool && toolManager.currentTool.decoration !== undefined && toolManager.currentTool.decoration !== currentIndex) toolManager.currentTool.decoration = currentIndex;
        model: ListModel {
            ListElement {
                text: "No decoration";
            }
            ListElement {
                text: "Thirds";
            }
            ListElement {
                text: "Fifths";
            }
            ListElement {
                text: "Passport photo";
            }
        }
    }

    RangeInput {
        id: xInput;
        width: parent.width;
        placeholder: "X";
        min: 0; max: sketchView.imageWidth; decimals: 0;
        value: toolManager.currentTool.cropX !== undefined ? toolManager.currentTool.cropX : 0;
        onValueChanged: if (toolManager.currentTool.cropX !== value) toolManager.currentTool.cropX = value;
    }

    RangeInput {
        id: yInput;
        width: parent.width;
        visible: fullView;
        placeholder: "Y";
        min: 0; max: sketchView.imageHeight; decimals: 0;
        value: toolManager.currentTool.cropY !== undefined ? toolManager.currentTool.cropY : 0;
        onValueChanged: if (toolManager.currentTool.cropY !== value) toolManager.currentTool.cropY = value;
    }

    RangeInput {
        id: widthInput;
        width: parent.width;
        placeholder: "Width";
        min: 0; max: sketchView.imageWidth; decimals: 0;
        value: toolManager.currentTool.cropWidth !== undefined ? toolManager.currentTool.cropWidth : 0;
        onValueChanged: if (toolManager.currentTool.cropWidth !== value) toolManager.currentTool.cropWidth = value;
    }

    RangeInput {
        id: heightInput;
        width: parent.width;
        placeholder: "Height";
        min: 0; max: sketchView.imageHeight; decimals: 0;
        value: toolManager.currentTool.cropHeight !== undefined ? toolManager.currentTool.cropHeight : 0;
        onValueChanged: if (toolManager.currentTool.cropHeight !== value) toolManager.currentTool.cropHeight = value;
    }

    Connections {
        target: toolManager.currentTool;
        onCropWidthChanged: if (widthInput.value !== toolManager.currentTool.cropWidth) widthInput.value = toolManager.currentTool.cropWidth;
        onCropHeightChanged: if (heightInput.value !== toolManager.currentTool.cropHeight) heightInput.value = toolManager.currentTool.cropHeight;
        onCropXChanged: if (xInput.value !== toolManager.currentTool.cropX) xInput.value = toolManager.currentTool.cropX;
        onCropYChanged: if (yInput.value !== toolManager.currentTool.cropY) yInput.value = toolManager.currentTool.cropY;
    }
}
