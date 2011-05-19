/*
 * This file is part of the KDE project
 *
 * Copyright (C) 2011 Shantanu Tushar <jhahoneyk@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

import QtQuick 1.0
import CalligraActive 1.0

Rectangle {
    id: homeScreen

    function openDocument(path) {
        doc.openDocument(path);
    }

    gradient: Gradient {
         GradientStop { position: 0.0; color: "#808080" }
         GradientStop { position: 1.0; color: "#303030" }
    }

    DocumentTypeSelector {
        id: docTypeSelector

        buttonWidth: homeScreen.width/2.1; buttonHeight: 100;
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.right: parent.horizontalCenter
        anchors.bottom: progressBar.top
        anchors.margins: 10
    }

    RecentFiles {
        id: recentFiles

        buttonWidth: homeScreen.width/2.1; buttonHeight: 100;
        anchors.left: parent.horizontalCenter
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.bottom: progressBar.top
        anchors.margins: 10
    }

    Rectangle {
        id: progressBar

        color: "blue"
        width: parent.width/100*doc.loadProgress; height: 32;
        anchors.bottom: parent.bottom
    }

    Doc {
        id: doc

        width: parent.width; height: parent.height;
        anchors.left: parent.right
        anchors.verticalCenter: parent.verticalCenter

        onDocumentLoaded: {
            homeScreen.state = "doc"
        }
    }

    transitions: Transition {
         // smoothly reanchor myRect and move into new position
         AnchorAnimation { duration: 500 }
     }

    states : [
        State {
            name: "doc"
            AnchorChanges {
                target: docTypeSelector
                anchors.left: undefined
                anchors.right: parent.left
            }
            AnchorChanges {
                target: recentFiles
                anchors.left: undefined
                anchors.right: parent.left
            }
            AnchorChanges {
                target: doc
                anchors.left: parent.left
            }
        },
        State {
            name: "showTextDocs"
            PropertyChanges { target: recentFiles; model: recentTextDocsModel }
        },
        State {
            name: "showSpreadsheets"
            PropertyChanges { target: recentFiles; model: recentSpreadsheetsModel }
        },
        State {
            name: "showPresentations"
            PropertyChanges { target: recentFiles; model: recentPresentationsModel }
        }
    ]
}