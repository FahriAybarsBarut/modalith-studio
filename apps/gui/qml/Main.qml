import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import Modalith 1.0

ApplicationWindow {
    id: root
    width: 1500
    height: 900
    minimumWidth: 1180
    minimumHeight: 700
    visible: true
    title: "Modalith Studio — " + modalithController.systemTitle + (modalithController.modified ? " *" : "")
    color: "#15171a"
    font.family: "Segoe UI"
    font.pixelSize: 12
    Material.theme: Material.Dark
    Material.primary: "#20242a"
    Material.accent: "#5b8def"

    property color workspaceColor: "#15171a"
    property color panelColor: "#1b1e22"
    property color headerColor: "#20242a"
    property color fieldColor: "#181b1f"
    property color borderColor: "#343a43"
    property color textColor: "#e6e8eb"
    property color secondaryTextColor: "#9da3ac"
    property color accentColor: "#5b8def"
    property color selectionColor: "#263954"
    property color warningColor: "#d5a24b"
    property int selectedSurfaceRow: 0

    ProjectDialogs {
        id: projectDialogs
        controller: modalithController
        hostWindow: root
    }

    menuBar: MenuBar {
        Menu {
            title: qsTr("File")
            Action { text: qsTr("New"); shortcut: StandardKey.New; onTriggered: projectDialogs.newProject() }
            Action { text: qsTr("Open…"); shortcut: StandardKey.Open; onTriggered: projectDialogs.openProject() }
            Action { text: qsTr("Save"); shortcut: StandardKey.Save; onTriggered: projectDialogs.saveProject() }
            Action { text: qsTr("Save As…"); shortcut: "Ctrl+Shift+S"; onTriggered: projectDialogs.saveProjectAs() }
            MenuSeparator {}
            Action { text: qsTr("Export analysis data…"); onTriggered: projectDialogs.exportAnalysis() }
            MenuSeparator {}
            Action { text: qsTr("Exit"); shortcut: StandardKey.Quit; onTriggered: Qt.quit() }
        }
        Menu {
            title: qsTr("Edit")
            Action { text: qsTr("Undo"); shortcut: StandardKey.Undo; enabled: modalithController.canUndo; onTriggered: modalithController.undo() }
            Action { text: qsTr("Redo"); shortcut: StandardKey.Redo; enabled: modalithController.canRedo; onTriggered: modalithController.redo() }
            MenuSeparator {}
            Action { text: qsTr("Insert surface"); shortcut: "Insert"; onTriggered: surfaceModel.addSurface() }
            Action { text: qsTr("Duplicate surface"); shortcut: "Ctrl+D"; onTriggered: modalithController.duplicateSurface(root.selectedSurfaceRow) }
            Action { text: qsTr("Delete surface"); shortcut: "Delete"; onTriggered: surfaceModel.removeSurface(root.selectedSurfaceRow) }
        }
        Menu {
            title: qsTr("Analyze")
            Action { text: qsTr("Update all analyses"); shortcut: "F5"; onTriggered: modalithController.analyze() }
            MenuSeparator {}
            Action { text: qsTr("Spot diagram"); onTriggered: analysisTabs.currentIndex = 0 }
            Action { text: qsTr("Ray fan"); onTriggered: analysisTabs.currentIndex = 1 }
        }
        Menu {
            title: qsTr("Tools")
            Action { text: qsTr("System settings…"); onTriggered: projectDialogs.editSystemSettings() }
            Action { text: qsTr("Export analysis data…"); onTriggered: projectDialogs.exportAnalysis() }
            Action { text: qsTr("Glass catalog"); enabled: false }
        }
        Menu {
            title: qsTr("Help")
            Action { text: qsTr("About Modalith Studio") }
        }
    }

    header: ToolBar {
        height: 52
        background: Rectangle {
            color: root.headerColor
            Rectangle { anchors.left: parent.left; anchors.right: parent.right; anchors.bottom: parent.bottom; height: 1; color: root.borderColor }
        }
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 12
            anchors.rightMargin: 12
            spacing: 8

            Rectangle {
                Layout.preferredWidth: 28
                Layout.preferredHeight: 28
                radius: 3
                color: root.accentColor
                Label { anchors.centerIn: parent; text: "M"; color: "white"; font.bold: true; font.pixelSize: 15 }
            }
            ColumnLayout {
                spacing: -2
                Label { text: "Modalith Studio"; color: root.textColor; font.bold: true; font.pixelSize: 13 }
                Label { text: modalithController.systemTitle + (modalithController.modified ? " · Modified" : ""); color: root.secondaryTextColor; font.pixelSize: 10 }
            }
            ToolSeparator {}
            ToolButton { text: "New"; onClicked: projectDialogs.newProject() }
            ToolButton { text: "Open"; onClicked: projectDialogs.openProject() }
            ToolButton { text: "Save"; onClicked: projectDialogs.saveProject() }
            ToolSeparator {}
            ComboBox {
                Layout.preferredWidth: 158
                model: ["Sequential mode"]
                currentIndex: 0
            }
            Item { Layout.fillWidth: true }
            Label { text: "Field X"; color: root.secondaryTextColor }
            TextField {
                Layout.preferredWidth: 66
                Layout.preferredHeight: 34
                text: modalithController.fieldX.toFixed(2)
                horizontalAlignment: TextInput.AlignRight
                validator: DoubleValidator { bottom: -89; top: 89; decimals: 4 }
                onEditingFinished: modalithController.fieldX = Number(text)
            }
            Label { text: "deg"; color: root.secondaryTextColor; font.pixelSize: 10 }
            Label { text: "Field Y"; color: root.secondaryTextColor; leftPadding: 4 }
            TextField {
                Layout.preferredWidth: 66
                Layout.preferredHeight: 34
                text: modalithController.fieldY.toFixed(2)
                horizontalAlignment: TextInput.AlignRight
                validator: DoubleValidator { bottom: -89; top: 89; decimals: 4 }
                onEditingFinished: modalithController.fieldY = Number(text)
            }
            Label { text: "deg"; color: root.secondaryTextColor; font.pixelSize: 10 }
            ToolSeparator {}
            CheckBox {
                text: "Auto-update"
                checked: modalithController.autoUpdate
                onToggled: modalithController.autoUpdate = checked
            }
            Button {
                text: "Update analysis"
                highlighted: true
                onClicked: modalithController.analyze()
            }
        }
    }

    footer: Rectangle {
        height: 27
        color: "#191c20"
        Rectangle { anchors.left: parent.left; anchors.right: parent.right; anchors.top: parent.top; height: 1; color: root.borderColor }
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 10
            anchors.rightMargin: 10
            spacing: 10
            Rectangle {
                Layout.preferredWidth: 7
                Layout.preferredHeight: 7
                radius: 4
                color: modalithController.statusText.indexOf("error") >= 0 ? "#d26a6a" : "#68a67d"
            }
            Label { text: modalithController.statusText; color: root.secondaryTextColor; font.pixelSize: 10 }
            Item { Layout.fillWidth: true }
            Label { text: "Sequential · Double precision"; color: root.secondaryTextColor; font.pixelSize: 10 }
            Rectangle { Layout.preferredWidth: 1; Layout.preferredHeight: 14; color: root.borderColor }
            Label { text: modalithController.wavelengthCount + " wavelengths"; color: root.secondaryTextColor; font.pixelSize: 10 }
        }
    }

    SplitView {
        anchors.fill: parent
        orientation: Qt.Horizontal

        Pane {
            SplitView.preferredWidth: 220
            SplitView.minimumWidth: 185
            padding: 0
            background: Rectangle { color: root.panelColor; border.color: root.borderColor }
            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 35
                    color: root.headerColor
                    Label {
                        anchors.left: parent.left
                        anchors.leftMargin: 11
                        anchors.verticalCenter: parent.verticalCenter
                        text: "Project"
                        color: root.textColor
                        font.bold: true
                    }
                }
                ListView {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 248
                    clip: true
                    model: [
                        { title: "System", detail: "Sequential" },
                        { title: "Wavelengths", detail: modalithController.wavelengthCount + " wavelengths" },
                        { title: "Fields", detail: "1 field" },
                        { title: "Aperture", detail: "Surface 1" },
                        { title: "Lens data", detail: surfaceModel.rowCount() + " surfaces" },
                        { title: "Analyses", detail: "2 open" }
                    ]
                    delegate: ItemDelegate {
                        width: ListView.view.width
                        height: 41
                        highlighted: index === 4
                        background: Rectangle {
                            color: parent.highlighted ? root.selectionColor : "transparent"
                            Rectangle { visible: parent.parent.highlighted; width: 3; anchors.top: parent.top; anchors.bottom: parent.bottom; color: root.accentColor }
                        }
                        contentItem: RowLayout {
                            spacing: 8
                            Rectangle {
                                Layout.preferredWidth: 6
                                Layout.preferredHeight: 6
                                radius: 3
                                color: index === 4 ? root.accentColor : "#626a75"
                            }
                            ColumnLayout {
                                spacing: -2
                                Label { text: modelData.title; color: root.textColor; font.pixelSize: 11 }
                                Label { text: modelData.detail; color: root.secondaryTextColor; font.pixelSize: 9 }
                            }
                        }
                    }
                }
                Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: root.borderColor }
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 35
                    color: root.headerColor
                    Label { anchors.left: parent.left; anchors.leftMargin: 11; anchors.verticalCenter: parent.verticalCenter; text: "Analysis setup"; color: root.textColor; font.bold: true }
                }
                GridLayout {
                    Layout.fillWidth: true
                    Layout.margins: 11
                    columns: 2
                    columnSpacing: 8
                    rowSpacing: 7
                    Label { text: "Pupil rings"; color: root.secondaryTextColor }
                    SpinBox {
                        Layout.fillWidth: true
                        from: 1
                        to: 30
                        value: modalithController.pupilRings
                        editable: true
                        onValueModified: modalithController.pupilRings = value
                    }
                    Label { text: "Sampling"; color: root.secondaryTextColor }
                    Label { text: "Concentric"; color: root.textColor }
                    Label { text: "Reference"; color: root.secondaryTextColor }
                    Label { text: modalithController.referenceWavelength.toFixed(1) + " nm"; color: root.textColor }
                }
                Item { Layout.fillHeight: true }
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 58
                    Layout.margins: 10
                    color: root.fieldColor
                    border.color: root.borderColor
                    radius: 2
                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 9
                        spacing: 2
                        Label { text: "Calculation engine"; color: root.textColor; font.bold: true; font.pixelSize: 10 }
                        Label { text: "CPU reference · C++20 · Eigen3"; color: root.secondaryTextColor; font.pixelSize: 9 }
                    }
                }
            }
        }

        SplitView {
            orientation: Qt.Vertical
            SplitView.fillWidth: true
            SplitView.minimumWidth: 560

            Frame {
                SplitView.fillHeight: true
                SplitView.minimumHeight: 290
                padding: 0
                background: Rectangle { color: root.workspaceColor; border.color: root.borderColor }
                ColumnLayout {
                    anchors.fill: parent
                    spacing: 0
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 35
                        color: root.headerColor
                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 11
                            anchors.rightMargin: 11
                            Label { text: "Optical layout"; color: root.textColor; font.bold: true }
                            Item { Layout.fillWidth: true }
                            Label { text: "YZ section · Reference wavelength 587.6 nm"; color: root.secondaryTextColor; font.pixelSize: 10 }
                        }
                    }
                    OpticalLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        controller: modalithController
                        revision: modalithController.analysisRevision
                    }
                }
            }

            Frame {
                SplitView.preferredHeight: 330
                SplitView.minimumHeight: 220
                padding: 0
                background: Rectangle { color: root.panelColor; border.color: root.borderColor }
                ColumnLayout {
                    anchors.fill: parent
                    spacing: 0
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 36
                        color: root.headerColor
                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 10
                            anchors.rightMargin: 6
                            Label { text: "Lens Data Editor"; color: root.textColor; font.bold: true }
                            Label { text: "Sequential prescription"; color: root.secondaryTextColor; font.pixelSize: 10; leftPadding: 6 }
                            Item { Layout.fillWidth: true }
                            ToolButton { text: "Insert"; onClicked: surfaceModel.addSurface() }
                            ToolButton { text: "Duplicate"; onClicked: modalithController.duplicateSurface(root.selectedSurfaceRow) }
                            ToolButton { text: "Delete"; onClicked: surfaceModel.removeSurface(root.selectedSurfaceRow) }
                        }
                    }
                    HorizontalHeaderView {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 29
                        syncView: surfaceTable
                        clip: true
                        delegate: Rectangle {
                            required property var display
                            color: "#24282e"
                            border.color: root.borderColor
                            implicitHeight: 29
                            Label {
                                anchors.fill: parent
                                anchors.leftMargin: 7
                                verticalAlignment: Text.AlignVCenter
                                text: display
                                color: root.secondaryTextColor
                                font.bold: true
                                font.pixelSize: 10
                            }
                        }
                    }
                    TableView {
                        id: surfaceTable
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true
                        model: surfaceModel
                        columnSpacing: 1
                        rowSpacing: 1
                        property var widths: [120, 84, 86, 96, 104, 70, 110, 58, 88, 88, 74, 74]
                        columnWidthProvider: function(column) { return widths[column] || 82 }
                        delegate: Rectangle {
                            id: cell
                            required property int row
                            required property int column
                            required property var display
                            implicitWidth: surfaceTable.widths[column]
                            implicitHeight: 31
                            color: root.selectedSurfaceRow === row ? root.selectionColor : (row % 2 ? "#1d2025" : root.panelColor)
                            border.color: root.borderColor
                            CheckBox {
                                visible: cell.column === 7
                                anchors.centerIn: parent
                                checked: cell.display === "Yes"
                                onActiveFocusChanged: if (activeFocus) root.selectedSurfaceRow = cell.row
                                onToggled: surfaceModel.setCell(cell.row, cell.column, checked ? "Yes" : "No")
                            }
                            TextField {
                                visible: cell.column !== 7
                                anchors.fill: parent
                                anchors.margins: 1
                                leftPadding: 6
                                rightPadding: 6
                                text: cell.display
                                color: root.textColor
                                font.pixelSize: 10
                                selectByMouse: true
                                background: Item {}
                                onActiveFocusChanged: if (activeFocus) root.selectedSurfaceRow = cell.row
                                onEditingFinished: surfaceModel.setCell(cell.row, cell.column, text)
                            }
                        }
                        ScrollBar.vertical: ScrollBar {}
                        ScrollBar.horizontal: ScrollBar {}
                    }
                }
            }
        }

        Pane {
            SplitView.preferredWidth: 390
            SplitView.minimumWidth: 330
            padding: 0
            background: Rectangle { color: root.panelColor; border.color: root.borderColor }
            ColumnLayout {
                anchors.fill: parent
                spacing: 0
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 35
                    color: root.headerColor
                    Label { anchors.left: parent.left; anchors.leftMargin: 11; anchors.verticalCenter: parent.verticalCenter; text: "Analysis results"; color: root.textColor; font.bold: true }
                }
                TabBar {
                    id: analysisTabs
                    Layout.fillWidth: true
                    TabButton { text: "Spot diagram" }
                    TabButton { text: "Ray fan" }
                }
                StackLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    currentIndex: analysisTabs.currentIndex

                    Item {
                        ColumnLayout {
                            anchors.fill: parent
                            spacing: 0
                            SpotPlot {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                controller: modalithController
                                revision: modalithController.analysisRevision
                            }
                            Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: root.borderColor }
                            GridLayout {
                                Layout.fillWidth: true
                                Layout.margins: 12
                                columns: 2
                                columnSpacing: 18
                                rowSpacing: 7
                                Label { text: "RMS radius"; color: root.secondaryTextColor }
                                Label { text: modalithController.rmsSpotRadius.toFixed(6) + " mm"; color: root.textColor; horizontalAlignment: Text.AlignRight; Layout.fillWidth: true }
                                Label { text: "Geometric radius"; color: root.secondaryTextColor }
                                Label { text: modalithController.geometricSpotRadius.toFixed(6) + " mm"; color: root.textColor; horizontalAlignment: Text.AlignRight; Layout.fillWidth: true }
                                Label { text: "Traced rays"; color: root.secondaryTextColor }
                                Label { text: modalithController.tracedRays; color: root.textColor; horizontalAlignment: Text.AlignRight; Layout.fillWidth: true }
                                Label { text: "Vignetted rays"; color: root.secondaryTextColor }
                                Label { text: modalithController.vignettedRays; color: modalithController.vignettedRays > 0 ? root.warningColor : root.textColor; horizontalAlignment: Text.AlignRight; Layout.fillWidth: true }
                            }
                        }
                    }
                    Item {
                        ColumnLayout {
                            anchors.fill: parent
                            spacing: 0
                            RayFanPlot {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                controller: modalithController
                                revision: modalithController.analysisRevision
                            }
                            Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: root.borderColor }
                            GridLayout {
                                Layout.fillWidth: true
                                Layout.margins: 12
                                columns: 2
                                rowSpacing: 7
                                Label { text: "Reference wavelength"; color: root.secondaryTextColor }
                                Label { text: modalithController.referenceWavelength.toFixed(4) + " nm"; color: root.textColor; horizontalAlignment: Text.AlignRight; Layout.fillWidth: true }
                                Label { text: "Pupil samples"; color: root.secondaryTextColor }
                                Label { text: modalithController.fanSamples; color: root.textColor; horizontalAlignment: Text.AlignRight; Layout.fillWidth: true }
                            }
                        }
                    }
                }
            }
        }
    }
}
