/* This file is part of the KDE project
 * Copyright (C) 2014 Arjen Hiemstra <ahiemstra@heimr.nl>
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

Page {
    id:base;

    signal finished(variant options);

    Rectangle {
        anchors.fill: parent;
        color: Settings.theme.color("pages/customImagePage/background");
    }

    Header {
        id: header;
        anchors {
            top: parent.top;
            left: parent.left;
            right: parent.right;
        }

        text: "Custom Image"

        leftArea: Button {
            width: Constants.GridWidth;
            height: Constants.GridHeight;
            image: Settings.theme.icon("back");
            onClicked: pageStack.pop();
        }

        rightArea: Button {
            width: Constants.GridWidth;
            height: Constants.GridHeight;
            image: Settings.theme.icon("apply");
            onClicked: {
                var options = {
                    name: nameField.text,
                    width: widthField.text,
                    height: heightField.text,
                    resolution: resolutionField.text,
                    colorModelId: colorModelModel.id(colorModel.currentIndex),
                    colorDepthId: colorDepthModel.id(colorDepth.currentIndex),
                    colorProfileId: colorProfileModel.id(colorProfile.currentIndex),
                    "backgroundColor": backgroundColor.value,
                    "backgroundOpacity": backgroundOpacity.value / 100,
                }
                base.finished(options);
            }
        }
    }

    Column {
        anchors {
            top: header.bottom;
            left: parent.left;
            right: parent.right;
            bottom: parent.bottom;
            topMargin: Constants.GridHeight * 0.35;
            leftMargin: Constants.GridWidth * 0.25;
            rightMargin: Constants.GridWidth * 0.25;
        }
        spacing: Constants.DefaultMargin;

        TextField {
            id: nameField;

            height: Constants.GridHeight * 0.75;
            width: parent.width + Constants.DefaultMargin * 2;
            x: -Constants.DefaultMargin;

            placeholder: "Name"
            nextFocus: widthField;
        }

        Rectangle {
            color: Settings.theme.color("pages/customImagePage/groupBox");

            height: childrenRect.height;
            width: parent.width;
            radius: Constants.DefaultMargin;

            Column {
                spacing: Constants.DefaultMargin;
                width: parent.width;

                Label {
                    height: Constants.GridHeight * 0.75;
                    width: parent.width - Constants.DefaultMargin * 2;
                    x: Constants.DefaultMargin;

                    text: "Image Size"
                }

                Row {
                    height: Constants.GridHeight * 0.75;
                    width: parent.width;

                    TextField {
                        id: widthField;

                        width: parent.width / 2;
                        height: parent.height;

                        placeholder: "Width";
                        validator: IntValidator{bottom: 0; top: 10000;}
                        numeric: true;
                        nextFocus: heightField;

                        background: Settings.theme.color("pages/customImagePage/controls/background");
                        border.color: Settings.theme.color("pages/customImagePage/controls/border");
                        border.width: 1;

                        Component.onCompleted: text = Settings.customImageSettings.readProperty("Width"); //Krita.Window.width;
                    }
                    TextField {
                        id: heightField;

                        width: parent.width / 2;
                        height: parent.height;

                        placeholder: "Height"
                        validator: IntValidator{bottom: 0; top: 10000;}
                        numeric: true;
                        nextFocus: resolutionField;

                        background: Settings.theme.color("pages/customImagePage/controls/background");
                        border.color: Settings.theme.color("pages/customImagePage/controls/border");
                        border.width: 1;

                        Component.onCompleted: text = Settings.customImageSettings.readProperty("Height"); //Krita.Window.height;
                    }
                }
                TextField {
                    id: resolutionField;

                    height: Constants.GridHeight * 0.75;

                    background: Settings.theme.color("pages/customImagePage/controls/background");
                    border.color: Settings.theme.color("pages/customImagePage/controls/border");
                    border.width: 1;

                    placeholder: "Resolution"
                    text: "72";
                    validator: IntValidator{bottom: 0; top: 600;}
                    numeric: true;
                    Component.onCompleted: text = Settings.customImageSettings.readProperty("Resolution");
                }
            }
        }
        Rectangle {
            color: Settings.theme.color("pages/customImagePage/groupBox");

            height: childrenRect.height;
            width: parent.width;
            radius: Constants.DefaultMargin;

            Column {
                x: Constants.DefaultMargin;
                width: parent.width - Constants.DefaultMargin * 2;

                Label {
                    height: Constants.GridHeight * 0.75;
                    text: "Color"
                }

                ExpandingListView {
                    id: colorModel;

                    height: Constants.GridHeight * 0.75;
                    width: parent.width;

                    expandedHeight: Constants.GridHeight * 3;

                    model: ColorModelModel { id: colorModelModel; }
                    Component.onCompleted: currentIndex = colorModelModel.indexOf(Settings.customImageSettings.readProperty("ColorModel"));
                }

                ExpandingListView {
                    id: colorDepth;

                    height: Constants.GridHeight * 0.75;
                    width: parent.width;

                    expandedHeight: Constants.GridHeight * 3;

                    model: ColorDepthModel { id: colorDepthModel; colorModelId: colorModelModel.id(colorModel.currentIndex); }
                    Component.onCompleted: currentIndex = colorDepthModel.indexOf(Settings.customImageSettings.readProperty("ColorDepth"));
                }

                ExpandingListView {
                    id: colorProfile;

                    height: Constants.GridHeight * 0.75;
                    width: parent.width;

                    expandedHeight: Constants.GridHeight * 3;

                    currentIndex: colorProfileModel.defaultProfile;

                    model: ColorProfileModel {
                        id: colorProfileModel;
                        colorModelId: colorModelModel.id(colorModel.currentIndex);
                        colorDepthId: colorDepthModel.id(colorDepth.currentIndex);
                    }
                }
            }
        }

        Rectangle {
            color: Settings.theme.color("pages/customImagePage/groupBox");

            height: childrenRect.height;
            width: parent.width;
            radius: Constants.DefaultMargin;

            Column {
                x: Constants.DefaultMargin;
                width: parent.width - Constants.DefaultMargin * 2;

                Label {
                    height: Constants.GridHeight * 0.75;
                    text: "Background"
                }

                ExpandingListView {
                    id: backgroundColor;

                    height: Constants.GridHeight * 0.75;
                    width: parent.width;

                    expandedHeight: Constants.GridHeight * 2;

                    model: ListModel {
                        ListElement { text: "White"; r: 1.0; g: 1.0; b: 1.0; }
                        ListElement { text: "Black"; r: 0.0; g: 0.0; b: 0.0; }
                        ListElement { text: "Gray"; r: 0.5; g: 0.5; b: 0.5; }
                        ListElement { text: "Red"; r: 1.0; g: 0.0; b: 0.0; }
                        ListElement { text: "Green"; r: 0.0; g: 1.0; b: 0.0; }
                        ListElement { text: "Blue"; r: 0.0; g: 0.0; b: 1.0; }
                        ListElement { text: "Cyan"; r: 0.0; g: 1.0; b: 1.0; }
                        ListElement { text: "Magenta"; r: 1.0; g: 0.0; b: 1.0; }
                        ListElement { text: "Yellow"; r: 1.0; g: 1.0; b: 0.0; }
                    }

                    property color value: Qt.rgba(0.0, 0.0, 0.0, 0.0);

                    onCurrentIndexChanged: {
                        var item = model.get(currentIndex);
                        value = Qt.rgba(item.r, item.g, item.b, 1.0);
                    }
                }

                RangeInput {
                    id: backgroundOpacity;

                    width: parent.width + 2 * Constants.DefaultMargin;
                    x: -Constants.DefaultMargin;
                    height: Constants.GridHeight * 1.5;

                    background: Settings.theme.color("pages/customImagePage/controls/background");
                    border.color: Settings.theme.color("pages/customImagePage/controls/border");
                    border.width: 1;

                    min: 0;
                    max: 100;
                    decimals: 0;
                    value: 100;
                    placeholder: "Opacity";
                }
            }
        }
    }
}
