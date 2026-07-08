import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

Item {
    id: dialogs
    property var controller
    property var hostWindow
    property url pendingOpenUrl

    function newProject() {
        if (controller.modified)
            confirmNewDialog.open()
        else
            controller.newProject()
    }

    function openProject() { openDialog.open() }

    function saveProject() {
        if (controller.currentFile.length > 0)
            controller.saveProject()
        else
            saveDialog.open()
    }

    function saveProjectAs() { saveDialog.open() }
    function exportAnalysis() { exportDialog.open() }
    function editSystemSettings() {
        titleEditor.text = controller.systemTitle
        temperatureEditor.text = controller.temperatureC.toString()
        wavelengthEditor.text = controller.wavelengthText
        settingsDialog.open()
    }

    FileDialog {
        id: openDialog
        title: "Open Modalith project"
        fileMode: FileDialog.OpenFile
        nameFilters: [
            "Modalith projects (*.modalith)",
            "Legacy Photon projects (*.photon)",
            "Zemax projects (*.zmx)",
            "CODE V sequence files (*.seq)",
            "CAD files (*.step *.iges *.stl)",
            "All files (*)"
        ]
        onAccepted: {
            if (controller.modified) {
                dialogs.pendingOpenUrl = selectedFile
                confirmOpenDialog.open()
            } else {
                controller.openProject(selectedFile)
            }
        }
    }

    FileDialog {
        id: saveDialog
        title: "Save Modalith project"
        fileMode: FileDialog.SaveFile
        nameFilters: [
            "Modalith projects (*.modalith)",
            "Zemax projects (*.zmx)"
        ]
        onAccepted: controller.saveProject(selectedFile)
    }

    FileDialog {
        id: exportDialog
        title: "Export analysis data"
        fileMode: FileDialog.SaveFile
        defaultSuffix: "csv"
        nameFilters: ["CSV data (*.csv)"]
        onAccepted: controller.exportAnalysisCsv(selectedFile)
    }

    FileDialog {
        id: importCatalogDialog
        title: "Import Glass Catalog (.agf)"
        fileMode: FileDialog.OpenFile
        nameFilters: ["Zemax Glass Catalogs (*.agf)", "All files (*)"]
        onAccepted: controller.importGlassCatalog(selectedFile)
    }

    function importGlassCatalog() { importCatalogDialog.open() }

    Dialog {
        id: confirmNewDialog
        parent: hostWindow ? hostWindow.contentItem : null
        anchors.centerIn: parent
        modal: true
        title: "Unsaved changes"
        standardButtons: Dialog.Discard | Dialog.Cancel
        Label {
            text: "Create a new project and discard the current unsaved changes?"
            color: "#e6e8eb"
        }
        onDiscarded: controller.newProject()
    }

    Dialog {
        id: confirmOpenDialog
        parent: hostWindow ? hostWindow.contentItem : null
        anchors.centerIn: parent
        modal: true
        title: "Unsaved changes"
        standardButtons: Dialog.Discard | Dialog.Cancel
        Label {
            text: "Open another project and discard the current unsaved changes?"
            color: "#e6e8eb"
        }
        onDiscarded: controller.openProject(dialogs.pendingOpenUrl)
    }

    Dialog {
        id: settingsDialog
        parent: hostWindow ? hostWindow.contentItem : null
        anchors.centerIn: parent
        width: 470
        modal: true
        title: "System settings"
        standardButtons: Dialog.Ok | Dialog.Cancel
        contentItem: GridLayout {
            columns: 2
            columnSpacing: 14
            rowSpacing: 10
            Label { text: "System title" }
            TextField { id: titleEditor; Layout.fillWidth: true }
            Label { text: "Temperature" }
            RowLayout {
                TextField {
                    id: temperatureEditor
                    Layout.preferredWidth: 100
                    validator: DoubleValidator { bottom: -273.15; top: 1000.0; decimals: 3 }
                }
                Label { text: "°C" }
            }
            Label { text: "Wavelengths" }
            TextField {
                id: wavelengthEditor
                Layout.fillWidth: true
                placeholderText: "486.1327, 587.5618, 656.2725"
            }
            Label { text: "Format"; color: "#9da3ac" }
            Label { text: "Nanometres, separated by commas"; color: "#9da3ac" }
        }
        onAccepted: {
            controller.systemTitle = titleEditor.text
            controller.temperatureC = Number(temperatureEditor.text)
            controller.wavelengthText = wavelengthEditor.text
        }
    }
}
