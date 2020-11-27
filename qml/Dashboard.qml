import QtQuick 2.9
import QtQuick.Window 2.3
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3
import QtQuick.Controls.Styles 1.4
import QtQuick.Extras 1.4
import io.qt.j1939 1.0


Window {
    id: root
    visible: true

    width: 1280 //Raspberry Pi screen width
    height: 720 //Raspberry Pi screen height

    color: "#A2A2A2"
    title: "Interfaz"

    Dialog {
        //this dialog will show if reset button is pressed and no new DTC are
        //received after 1 second.
        id: noFaultsDialog
        title: "Faults Reset Report"
        modal: true
        standardButtons: DialogButtonBox.Ok
        width: 512
        height: 220
        anchors.centerIn: parent
        contentItem: Text {
            id: faultsText
            textFormat: Text.StyledText
            text: "Faults have been successfully reset"
        }
    }

    Dialog {
        //this dialog will show upon receival of a new fault signal
        id: newFaultsDialog
        modal: true
        standardButtons: DialogButtonBox.Ok
        width: 512
        height: 220
        anchors.centerIn: parent
        contentItem: Text {
            textFormat: Text.StyledText
            id: newFaultsText
        }
    }



    // idleTimers actualize the 'no signal' state, activating it after 1 second
    // has passed since last received value.

    Timer {
        id:linearIdleTimer
        running: true
        interval: 3000
        onTriggered: {
            buttonLinear.checkable = false
            buttonLinear.flat = true
            bUpLineSP.checkable = false
            bUpLineSP.flat = true
            bDownLineSP.checkable = false
            bDownLineSP.flat = true
            if (buttonLinear.checked) {
                buttonLinear.checked = false
                noSignaltext.visible = true
                statusIndicator.active = false
                clearDTCButton.flat = true
                selButtons.lastButton = buttonLinear
            }
        }
    }

    Timer {
        id:thermometerIdleTimer
        running: true
        interval: 3000
        onTriggered: {
            buttonTemperature.checkable = false
            buttonTemperature.flat = true
            bUpTempSP.checkable = false
            bUpTempSP.flat = true
            bDownTempSP.checkable = false
            bDownTempSP.flat = true
            if (buttonTemperature.checked) {
                buttonTemperature.checked = false
                noSignaltext.visible = true
                statusIndicator.active = false
                clearDTCButton.flat = true
                selButtons.lastButton = buttonTemperature
            }
        }
    }

    Timer {
        id:positionIdleTimer
        running: true
        interval: 3000
        onTriggered: {
            buttonPosition.checkable = false
            buttonPosition.flat = true
            if (buttonPosition.checked) {
                buttonPosition.checked = false
                noSignaltext.visible = true
                statusIndicator.active = false
                clearDTCButton.flat = true
                selButtons.lastButton = buttonPosition
            }
        }
    }
    // This timer performs the 1s 'wait' after reset button is pressed
    Timer {
        id: faultResetTimer
        onTriggered: {
            noFaultsDialog.open()
            statusIndicator.updateColor()
        }
    }

    Timer {
        id:linearSPTimer
        running: true
        interval: 1000
        onTriggered: {
            j1939.sendData(1)
            linearSPTimer.restart()
        }
    }

    Timer {
        id:tempSPTimer
        running: true
        interval: 1000
        onTriggered: {
            j1939.sendData(2)
            tempSPTimer.restart()
        }
    }

    Timer {
        id:upLinearTimer
        running: bUpLineSP.pressed
        interval: 200
        onTriggered: {
            if (!buttonLinear.flat){
                j1939.setLinearSP(1)
                upLinearTimer.restart()
            }
        }
    }

    Timer {
        id:downLinearTimer
        running: bDownLineSP.pressed
        interval: 200
        onTriggered: {
            if (!buttonLinear.flat){
                j1939.setLinearSP(-1)
                downLinearTimer.restart()
            }
        }
    }

    Timer {
        id:upTempTimer
        running: bUpTempSP.pressed
        interval: 200
        onTriggered: {
            if (!buttonTemperature.flat){
                j1939.setTempSP(1)
                upTempTimer.restart()
            }
        }
    }

    Timer {
        id:downTempTimer
        running: bDownTempSP.pressed
        interval: 200
        onTriggered: {
            if (!buttonTemperature.flat){
                j1939.setTempSP(-1)
                downTempTimer.restart()
            }
        }
    }

    J1939 {
        id: j1939

        // to reduce code complexity, a copy of the DTCs is kept at display
        // level, reducing data interchange between the C++ class and this QML
        // object.
        property int temperatureDTC: 0
        property int linearDTC: 0
        property int positionDTC: 0

        // All three instruments have the same value and DTC change handlers
        onTemperatureChanged: {
            //sets to 'connected' state
            buttonTemperature.checkable = true
            buttonTemperature.flat = false
            bUpTempSP.checkable = false
            bUpTempSP.flat = false
            bDownTempSP.checkable = false
            bDownTempSP.flat = false
            thermometerIdleTimer.restart()
            // Reactivates last sensor if deactivated by 'no conection' state
            if (selButtons.lastButton == buttonTemperature) {
                buttonTemperature.clicked()
                buttonTemperature.checked = true
            }
        }

        onLinearChanged: {
            buttonLinear.checkable = true
            buttonLinear.flat = false
            bUpLineSP.checkable = false
            bUpLineSP.flat = false
            bDownLineSP.checkable = false
            bDownLineSP.flat = false
            linearIdleTimer.restart()
            if (selButtons.lastButton == buttonLinear) {
                buttonLinear.clicked()
                buttonLinear.checked = true
            }
        }

        onXPosChanged: {
            buttonPosition.checkable = true
            buttonPosition.flat = false
            positionIdleTimer.restart()
            if (selButtons.lastButton == buttonPosition) {
                buttonPosition.clicked()
                buttonPosition.checked = true
            }
        }

        onYPosChanged: {
            buttonPosition.checkable = true
            buttonPosition.flat = false
            positionIdleTimer.restart()
            if (selButtons.lastButton == buttonPosition) {
                buttonPosition.clicked()
                buttonPosition.checked = true
            }
        }

        onOrientationChanged: {
            buttonPosition.checkable = true
            buttonPosition.flat = false
            positionIdleTimer.restart()
            if (selButtons.lastButton == buttonPosition) {
                buttonPosition.clicked()
                buttonPosition.checked = true
            }
        }

        //////////////////////////////////

        onThermometerNewFaultsChanged: {
            //combines existing faults with the new reported ones
            temperatureDTC |= j1939.ThermometerNewFaults
            //informs of new faults if corresponding instrument is active
            if (buttonTemperature.checked) {
                statusIndicator.updateColor()
                //if no reset, informs of a new fault
                if (!faultResetTimer.running) {
                    newFaultsDialog.title = "New Thermometer Fault Report"
                    newFaultsText.text = ("Error code: 0b" +
                                          j1939.
                                          ThermometerNewFaults.toString(2))
                }
                //if reset, informs of the persisting faults
                else {
                    newFaultsDialog.title = "Thermometer Faults Report"
                    newFaultsText.text = ("Persistent Error code: 0b" +
                                          temperatureDTC.toString(2))
                    newFaultsDialog.open()
                }
                faultResetTimer.stop()
                newFaultsDialog.open()
            }
        }

        onTachometerNewFaultsChanged: {
            positionDTC |= j1939.TachometerNewFaults
            if (buttonPosition.checked) {
                statusIndicator.updateColor()
                if (!faultResetTimer.running) {
                    newFaultsDialog.title = "New Tachometer Fault Report"
                    newFaultsText.text = ("Error code: 0b" +
                                          j1939.TachometerNewFaults.toString(2))
                }
                else {
                    newFaultsDialog.title = "Tachometer Faults Report"
                    newFaultsText.text = ("Persistent Error code: 0b" +
                                          positionDTC.toString(2))
                    newFaultsDialog.open()
                }
                faultResetTimer.stop()
                newFaultsDialog.open()
            }
        }

        onFuelGaugeNewFaultsChanged: {
            linearDTC |= j1939.FuelGaugeNewFaults
            if (buttonLinear.checked) {
                statusIndicator.updateColor()
                if (!faultResetTimer.running) {
                    newFaultsDialog.title = "New Fuel Gauge Fault Report"
                    newFaultsText.text = ("Error code: 0b" +
                                          j1939.FuelGaugeNewFaults.toString(2))
                }
                else {
                    newFaultsDialog.title = "Tachometer Faults Report"
                    newFaultsText.text = ("Persistent Error code: 0b" +
                                          linearDTC.toString(2))
                    newFaultsDialog.open()
                }
                faultResetTimer.stop()
                newFaultsDialog.open()
            }
        }

        ////////////////////////////////////

        onLinearNewFaultsChanged:{
            linearDTC |= j1939.LinearNewFaults
            if (buttonLinear.checked) {
                statusIndicator.updateColor()
                if (!faultResetTimer.running) {
                    newFaultsDialog.title = "New Actuator Fault Report"
                    if(LinearNewFaults === 3)
                        newFaultsText.text = ("Supply voltage above normal or<br>shorted to high source")
                    else if(LinearNewFaults === 4)
                        newFaultsText.text = ("Supply voltage below normal or<br>shorted to low source")
                    else if(LinearNewFaults === 5)
                        newFaultsText.text = ("Motor current below normal or<br>open circuit")
                    else if(LinearNewFaults === 6)
                        newFaultsText.text = ("Motor current above normal or<br>grounded circuit")
                }
                else {
                    newFaultsDialog.title = "Persistent Actuator Fault Report"
                    if(LinearNewFaults === 3)
                        newFaultsText.text = ("Persistent: Supply voltage above<br>normal or shorted to high source")
                    else if(LinearNewFaults === 4)
                        newFaultsText.text = ("Persistent: Supply voltage below<br>normal or shorted to low source")
                    else if(LinearNewFaults === 5)
                        newFaultsText.text = ("Persistent: Motor current below<br>normal or open circuit")
                    else if(LinearNewFaults === 6)
                        newFaultsText.text = ("Persistent: Motor current Above<br>normal or grounded circuit")
                    newFaultsDialog.open()
                }
                faultResetTimer.stop()
                newFaultsDialog.open()
            }
        }

        onTemperatureNewFaultsChanged:{
            temperatureDTC |= j1939.TemperatureNewFaults
            if (buttonTemperature.checked) {
                statusIndicator.updateColor()
                if (!faultResetTimer.running) {
                    newFaultsDialog.title = "New temperature fault report"
                    if(TemperatureNewFaults === 0)
                        newFaultsText.text = ("Data valid but above normal<br>operating rate")
                    else if(TemperatureNewFaults === 1)
                        newFaultsText.text = ("Data valid but below normal<br>operating rate")
                    else if(TemperatureNewFaults === 2)
                        newFaultsText.text = ("Data erratic, intermitent or<br>incorrect")
                    else if(TemperatureNewFaults === 3)
                        newFaultsText.text = ("Supply voltage above normal or<br>shorted to high source")
                    else if(TemperatureNewFaults === 4)
                        newFaultsText.text = ("Supply voltage below normal or<br>shorted to low source")
                    else if(TemperatureNewFaults === 5)
                        newFaultsText.text = ("Motor current below normal or<br>open circuit")
                    else if(TemperatureNewFaults === 6)
                        newFaultsText.text = ("Overload or open circuit")
                }
                else {
                    newFaultsDialog.title = "Persistent temperature fault report"
                    if(TemperatureNewFaults === 0)
                        newFaultsText.text = ("Persistent: Data valid but above normal<br>operating rate")
                    else if(TemperatureNewFaults === 1)
                        newFaultsText.text = ("Persistent: Data valid but below normal<br>operating rate")
                    else if(TemperatureNewFaults === 2)
                        newFaultsText.text = ("Persistent: Data erratic, intermitent or<br>incorrect")
                    else if(TemperatureNewFaults === 3)
                        newFaultsText.text = ("Persistent: Supply voltage above normal or<br>shorted to high source")
                    else if(TemperatureNewFaults === 4)
                        newFaultsText.text = ("Persistent: Supply voltage below normal or<br>shorted to low source")
                    else if(TemperatureNewFaults === 5)
                        newFaultsText.text = ("Persistent: Motor current below normal or<br>open circuit")
                    else if(TemperatureNewFaults === 6)
                        newFaultsText.text = ("Persistent: Overload or open circuit")
                    newFaultsDialog.open()
                }
                faultResetTimer.stop()
                newFaultsDialog.open()
            }
        }

        onPositionNewFaultsChanged:{
            positionDTC |= j1939.PositionNewFaults
            if (buttonPosition.checked) {
                statusIndicator.updateColor()
                if (!faultResetTimer.running) {
                    newFaultsDialog.title = "New Position Fault Report"
                    if(PositionNewFaults === 2)
                        newFaultsText.text = ("Data erratic, intermitent or<br>incorrect")
                }
                else {
                    newFaultsDialog.title = "Persistent Position Fault Report"
                    if(PositionNewFaults === 2)
                        newFaultsText.text = ("Data erratic, intermitent or<br>incorrect")
                    newFaultsDialog.open()
                }
                faultResetTimer.stop()
                newFaultsDialog.open()
            }
        }
    }

    Item {
        id: container
        width: root.width
        height: root.height
        anchors.verticalCenterOffset: -2
        anchors.horizontalCenterOffset: 0
        anchors.centerIn: parent

        //text to be displayed during 'no signal' state
        Text {
            id: noSignaltext
            visible: false

            x: 34
            width: 850
            height: 438
            anchors.verticalCenter: parent.verticalCenter

            color: "#FF0000"
            text: "No Signal"
            z: 2
            anchors.verticalCenterOffset: -21
            font.pointSize: 100
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }

        Rectangle {
            id: background
            x: 40
            y: 125
            width: 815
            height: 433
            color: "#1b1b1b"
            z: -1
        }

        Text {
            id: thermometer
            visible: buttonTemperature.checked

            x: 29
            width: 855
            height: 427
            anchors.verticalCenter: parent.verticalCenter

            text: j1939.Temperature + " °C"
            font.pointSize: 150
            fontSizeMode: Text.VerticalFit
            font.bold: true
            font.weight: Font.Bold
            font.capitalization: Font.AllUppercase
            z: 0
            anchors.verticalCenterOffset: -21
            color: "#E00000"
            font.family: "Arial"
            font.italic: false

            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            style: Text.Raised
            styleColor: "#400000"
        }

        ButtonGroup {
            id: selButtons
            buttons: buttonCol.children
            property Button lastButton: buttonTemperature
        }

        Column {
            id:buttonCol
            x: 866
            y: 125
            width: 249
            height: 557
            spacing: 0

            RoundButton {
                id: buttonLinear
                x: 36
                width: 250
                height: 186
                radius: 15
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Linear\nDisplacement"
                z: 1
                font.pointSize: 25
                checkable: true
                flat: false
                onClicked: {
                    //if not hidden, activates the instrument
                    if(!buttonLinear.flat) {
                        statusIndicator.updateColor()
                        noSignaltext.visible = false
                        statusIndicator.active = true
                        clearDTCButton.flat = false
                        selButtons.lastButton = null
                    }
                }
            }

            RoundButton {
                id: buttonTemperature
                width: 250
                height: 186
                radius: 15
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Engine Coolant\nTemperature"
                spacing: 6
                font.pointSize: 25
                checkable: true
                checked: true
                onClicked: {
                    //if not hidden, activates the instrument
                    if(!buttonTemperature.flat) {
                        statusIndicator.updateColor()
                        noSignaltext.visible = false
                        statusIndicator.active = true
                        clearDTCButton.flat = false
                        selButtons.lastButton = null
                    }
                }
            }

            RoundButton {
                id: buttonPosition
                width: 250
                height: 186
                radius: 15
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Position/\nOrientation"
                font.pointSize: 25
                checkable: true
                onClicked: {
                    //if not hidden, activates the instrument
                    if(!buttonPosition.flat) {
                        statusIndicator.updateColor()
                        noSignaltext.visible = false
                        statusIndicator.active = true
                        clearDTCButton.flat = false
                        selButtons.lastButton = null
                    }
                }
            }
        }

        //Gets updated according to current selected instrument and the color
        //of the warning, using the 'updateColor()' function.
        StatusIndicator {
            id: statusIndicator
            x: 29
            y: 564
            width: 246
            height: 118
            color: "green"
            function updateColor() {
                var error
                switch (selButtons.checkedButton) {
                case buttonLinear:
                    error  = j1939.linearDTC
                    break
                case buttonPosition:
                    error = j1939.positionDTC
                    break
                case buttonTemperature:
                    error = j1939.temperatureDTC
                    break
                default:
                    statusIndicator.active = false
                    break
                }
                if (error > 7) {
                    statusIndicator.color = "red"
                }
                else if (error > 0) {
                    statusIndicator.color = "yellow"
                }
                else {
                    statusIndicator.color = "green"
                }
            }
        }

        RoundButton {
            id: clearDTCButton
            text: "Fault reset"
            font.pointSize: 25
            font.family: "Tahoma"
            x: 572
            y: 564
            width: 254
            height: 118
            radius: 15
            checkable: false
            onClicked: {
                /*
 * If the button is not hidden, it resets the current instrument and invokes
 * the sendStatusReset() method of the J1939 class to request a fault reset on
 * the corresponding instrument.
*/
                if(!clearDTCButton.flat) {
                    faultResetTimer.restart()
                    switch (selButtons.checkedButton) {
                    case buttonLinear:
                        j1939.linearDTC = 0
                        j1939.sendStatusReset(1)
                        break
                    case buttonPosition:
                        j1939.positionDTC = 0
                        j1939.sendStatusReset(2)
                        break
                    case buttonTemperature:
                        j1939.temperatureDTC = 0
                        j1939.sendStatusReset(3)
                        break
                    default:
                        clearDTCButton.flat = true
                        return
                    }
                }
            }
        }

        Image {
            id: image
            x: 274
            y: 8
            width: 371
            height: 100
            fillMode: Image.PreserveAspectFit
            source: "../images/tec-logo-bg.png"
        }

        Text {
            id: intelectualText
            x: 0
            y: 688
            width: 1280
            height: 32
            text: qsTr("Intelectual Property Information")
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: 20
        }

        Column {
            id: linearCol
            x: 52
            y: 210
            width: 792
            height: 336
            spacing: 100
            visible: buttonLinear.checked

            Text {
                id: linearText
                width: 764
                height: 176
                color: "#008000"
                text: j1939.LinearDisplacement.toFixed(1) + " mm"
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                font.pixelSize: 150
            }

            ProgressBar {
                id: linearBar
                x: 0
                y: 250
                width: 769
                height: 60
                value: j1939.LinearDisplacement * 10
                to: 64255
                font.pointSize: 17
            }
        }

        Column {
            id: posOrCol
            x: 59
            y: 125
            width: 774
            height: 433
            spacing: 77
            visible: buttonPosition.checked

            Text {
                id: posXText
                width: 733
                height: 93
                color: "#ffffff"
                text: "X: " + j1939.xpos
                styleColor: "#ffffff"
                font.pixelSize: 80
            }

            Text {
                id: posYText
                width: 733
                height: 93
                color: "#ffffff"
                text: "Y: " + j1939.ypos
                styleColor: "#ffffff"
                font.pixelSize: 80
            }

            Text {
                id: orientationText
                width: 733
                height: 93
                color: "#ffffff"
                text: "Orientation: " + j1939.OrientationDegrees.toFixed(2) + "°"
                font.pixelSize: 80
            }
        }

        Rectangle {
            id: bgLinearSP
            x: 1121
            y: 165
            width: 151
            height: 106
            color: "#1b1b1b"
            border.color: "#1b1b1b"
            z: -1

            Column {
                id: lineSPCol
                x: 0
                y: -40
                width: 151
                height: 186
                spacing: 106

                Button {
                    id: bUpLineSP
                    width: 151
                    height: 40
                    text: qsTr("▲")
                    onClicked:{
                        if (!buttonLinear.flat)
                            j1939.setLinearSP(1)
                    }
                }

                Button {
                    id: bDownLineSP
                    width: 151
                    height: 40
                    text: qsTr("▼")
                    onClicked:{
                        if (!buttonLinear.flat)
                            j1939.setLinearSP(-1)
                    }
                }
            }

            Text {
                id: textoLinearSP
                x: 0
                y: 0
                width: 151
                height: 106
                color: "#ffffff"
                text: j1939.linearSP + ""
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                font.pixelSize: 40
            }
        }

        Rectangle {
            id: bgTempSP
            x: 1121
            y: 351
            width: 151
            height: 106
            color: "#1b1b1b"
            z: -1
            border.color: "#1b1b1b"

            Column {
                id: tempSPCol
                x: 0
                y: -40
                width: 151
                height: 186
                spacing: 106

                Button {
                    id: bUpTempSP
                    width: 151
                    height: 40
                    text: qsTr("▲")
                    onClicked:{
                        if (!buttonTemperature.flat)
                            j1939.setTempSP(1)
                    }
                }

                Button {
                    id: bDownTempSP
                    width: 151
                    height: 40
                    text: qsTr("▼")
                    onClicked:{
                        if (!buttonTemperature.flat)
                            j1939.setTempSP(-1)
                    }
                }
            }

            Text {
                id: textoTempSP
                x: 0
                y: 0
                width: 151
                height: 106
                color: "#ffffff"
                text: j1939.tempSP + ""
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                font.pixelSize: 40
            }
        }
    }
}

/*##^## Designer {
    D{i:11;invisible:true}D{i:13;invisible:true}D{i:15;invisible:true}
}
 ##^##*/
