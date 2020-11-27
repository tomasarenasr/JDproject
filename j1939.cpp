#include "j1939.h"

/******************************************************************************
* FUNCTION: j1939()
*
* DESCRIPTION: This is the constructor of the class, used to initialize some
*              values, call the connectDevice fuction and connect the signal
*              with its corresponding slot.
*
* PARAMETERS:  None
*
* Return:      None
******************************************************************************/
j1939::j1939(QObject *parent) : QObject(parent) {
    connectDevice();
    connect(m_canDevice, &QCanBusDevice::framesReceived,
            this, &j1939::processFrames);
    TachometerFaultStates  = DTC_NO_FAULTS;
    FuelGaugeFaultStates =   DTC_NO_FAULTS;
    ThermometerFaultStates = DTC_NO_FAULTS;
    LinearFaultStates = DTC_NO_FAULTS;
    TemperatureFaultStates = DTC_NO_FAULTS;
    PositionFaultStates = DTC_NO_FAULTS;
    emit canBusConnected();
}

/******************************************************************************
* FUNCTION: ~j1939()
*
* DESCRIPTION: This is the the destructor of the class, used to disconect the
*              canDevice
*
* PARAMETERS:  None
*
* Return:      None
******************************************************************************/
j1939::~j1939() {
    m_canDevice->disconnectDevice();
    delete m_canDevice;
}

/******************************************************************************
* FUNCTION: j1939::connectDevice()
*
* DESCRIPTION: This function create a connection with the can0 device using the
*              socketcan plugin.
*
* PARAMETERS:  none
*
* Return:      none
******************************************************************************/
void j1939::connectDevice() {
    QString errorString = "Error, no can device connected";
    m_canDevice = QCanBus::instance()->createDevice(
                QStringLiteral("socketcan"),
                QStringLiteral("can0"), &errorString);
    if (!m_canDevice) {

        // Error handling goes here

        qDebug() << errorString;
    } else {
        m_canDevice->connectDevice();
        qDebug() << "Device Connected";
    }
}


/******************************************************************************
* FUNCTION: j1939::prepareCANFrame
*
* DESCRIPTION: This Function builds a message frame according to the J1939
*              protocol.
*
* PARAMETERS:  PGN- The PGN that will be assigned to frame.
*              addr - The address to be included in the message.
*              payload- The payload to be included in the frame.
*
* Return:      A can frame compliant with the J1939 protocol.
******************************************************************************/
QCanBusFrame j1939::prepareCANFrame(quint16 PGN, quint8 addr, QByteArray payload) {
    QCanBusFrame frame;

    Priority =      ECU_PRIORITY_LEVEL;
    ExtendedData =  EXTENDED_DATA_BIT ;
    DataPage =      DATA_PAGE_BIT;
    SourceAddress = addr;

    quint32 FrameId = quint32(Priority << PRIORITY_SHIFT_POSITION |
                              ExtendedData << EXTENDED_DATA_SHIFT_POSITION |
                              DataPage << DATA_PAGE_SHIFT_POSITION |
                              PGN << PGN_SHIFT_POSITION | SourceAddress);

    frame.setFrameId(FrameId);
    frame.setPayload(payload);
    return frame;
}

/******************************************************************************
* FUNCTION: j1939::sendStatusReset()
*
* DESCRIPTION: This function is called when the status reset button is clicked.
*
* PARAMETERS:  device- device PGN to request fault reset
*
* Return:      None
******************************************************************************/
void j1939::sendStatusReset(QString n) {
    int opt = n.toInt();
    QCanBusFrame frame;
    QByteArray payload(BYTE_DATA_PER_PACKET, DTC_NO_FAULTS);
    quint16 PGN = 0x0000;
    quint8 addrsend = 0;
    //qDebug() << "Reset";
    switch (opt){
    case 1:
        //Linear
        PGN = DM1_PGN;
        LinearFaultStates = DTC_NO_FAULTS;
        addrsend = 0x48;
        //qDebug() << PGN;
        break;
    case 2:
        //Position
        PGN = DM1_PGN;
        TemperatureFaultStates = DTC_NO_FAULTS;
        addrsend = 0x63;
        //qDebug() << PGN;
        break;
    case 3:
        //Temperature
        PGN = DM1_PGN;
        PositionFaultStates = DTC_NO_FAULTS;
        addrsend = 0x50;
        //qDebug() << PGN;
        break;
    }
    frame = prepareCANFrame(PGN, addrsend, payload);
    m_canDevice->writeFrame(frame);
    //qDebug() << frame.frameId();
}

/******************************************************************************
* FUNCTION: j1939::sendData()
*
* DESCRIPTION: This function is called periodically to send data devices.
*
* PARAMETERS:  device- device PGN to send data.
*
* Return:      None
******************************************************************************/

void j1939::sendData(QString n){
    int device = n.toInt();
    QCanBusFrame frame;
    QByteArray payload(BYTE_DATA_PER_PACKET, 0xFF);
    quint16 PGN = 0x0000;
    quint8 addrsend = 0;
    //qDebug() << "Reset";
    switch (device){
    case 1:{
        // TREAD_POS_PGN = 0xFFF8
        PGN = TREAD_POS_PGN;
        payload[3] = linearSP;
        addrsend = LINEAR_ADR;
        break;
    }

    case 2:{
        // HEATER_SP_PGN = 0xF037
        PGN = HEATER_SP_PGN;
        payload[0] = tempSP;
        addrsend = TEMP_ADR;
        break;
    }
    }
    frame = prepareCANFrame(PGN, addrsend, payload);
    m_canDevice->writeFrame(frame);
}

/******************************************************************************
* FUNCTION: j1939::processFrames()
*
* DESCRIPTION: This fuction is executed in the reception of a can frame. If
*              there are frames availables the specificed PGN is checking to
*              execute some tasks.
*
* PARAMETERS:  None
*
* Return:      None
******************************************************************************/
void j1939::processFrames() {
    if (!m_canDevice) {
        //qDebug() << "No Device";
        return;
    }
    while (m_canDevice->framesAvailable()){
        QCanBusFrame frame = m_canDevice->readFrame();
        quint32 canId = frame.frameId();

        Priority =      (canId & ID_PRIORITY_MASK) >> PRIORITY_SHIFT_POSITION;
        ExtendedData =  (canId & EXTENDED_DATA_MASK) >>
                                                        EXTENDED_DATA_SHIFT_POSITION;
        DataPage =      (canId & DATA_PAGE_MASK) >> DATA_PAGE_SHIFT_POSITION;
        PDUFormat =     (canId & PDU_FORMAT_MASK) >> PDU_FORMAT_SHIFT_POSITION;
        PDUSpecific =   (canId & PDU_SPECIFIC_MASK) >> PGN_SHIFT_POSITION;
        SourceAddress = (canId & SOURCE_ADRESS_MASK);

        //qDebug() << "Can Id: " << canId;
        //qDebug() << "P: " << Priority;
        //qDebug() << "ED: " << ExtendedData;
        //qDebug() << "D: " << DataPage;
        //qDebug() << "PDUF " << PDUFormat;
        //qDebug() << "PDUS: " << PDUSpecific;
        //qDebug() << "SA: " << SourceAddress;

        QByteArray payload;
        payload = frame.payload();
        uint data, data2;
        quint8 PreviousStates;
        quint8 address;

        qDebug() << "MSG RECEIVED" << getPGN(canId);

        /*
       * this switch takes the PGN of the CAN frame, and if it matches the
       * PGN of the relevant data for the system, enters the respective switch
       * case and executes the required data extraction and processing, end
       * emits a signal to inform there is new data available.
      */

        switch (getPGN(canId)) {
        case TEMPERATURE_DTC:{
            PreviousStates = ThermometerFaultStates;
            //checks all of the payload bytes for content.
            for (int i = BIT_CHECK_BEGINNING; i < BIT_CHECK_END; i++) {
                //if it has content, a DTC is detected. Same for all DTC cases.
                if (payload.at(i) != EMPTY_PAYLOAD) {
                    ThermometerFaultStates |= (DTC_BITMASK << i) ;
                }
            }
            ThermometerNewFaults = PreviousStates ^
                    (ThermometerFaultStates | PreviousStates);
            if (ThermometerNewFaults != DTC_NO_NEW_FAULTS)
                emit thermometerNewFaultsChanged();
            //qDebug() << "Thermo DTC: " << ThermometerFaultStates;
            //qDebug() << "New Faults: " << ThermometerNewFaults;
            //qDebug() << "PGN: " << getPGN(canId);
            break;
        }

        case TACHOMETER_DTC:{
            PreviousStates = TachometerFaultStates;
            for (int i = BIT_CHECK_BEGINNING; i < BIT_CHECK_END; i++) {
                if (payload.at(i) != EMPTY_PAYLOAD) {
                    TachometerFaultStates |= (DTC_BITMASK << i) ;
                }
            }
            TachometerNewFaults = PreviousStates ^
                    (TachometerFaultStates | PreviousStates);
            if (TachometerNewFaults != DTC_NO_NEW_FAULTS)
                emit tachometerNewFaultsChanged();
            //qDebug() << "Tacho DTC: " << TachometerFaultStates;
            //qDebug() << "New Faults: " << TachometerNewFaults;
            //qDebug() << "PGN: " << getPGN(canId);
            break;
        }

        case FUEL_GAUGE_DTC:{
            PreviousStates = FuelGaugeFaultStates;
            qDebug() << PreviousStates;
            for (int i = BIT_CHECK_BEGINNING; i < BIT_CHECK_END; i++) {
                if (payload.at(i) != EMPTY_PAYLOAD) {
                    FuelGaugeFaultStates |= (DTC_BITMASK << i) ;
                }
            }
            qDebug() << FuelGaugeFaultStates;
            FuelGaugeNewFaults = PreviousStates ^
                    (FuelGaugeFaultStates | PreviousStates);
            if (FuelGaugeNewFaults != DTC_NO_NEW_FAULTS)
                emit fuelGaugeNewFaultsChanged();
            qDebug() << FuelGaugeFaultStates;
            //qDebug() << "Fuel Gauge DTC: " << FuelGaugeFaultStates;
            //qDebug() << "New Faults: " << FuelGaugeNewFaults;
            //qDebug() << "PGN: " << gtPGN(canId);
            break;
        }

        case DM1_PGN:{
            qDebug() << "DM1 Active Diagnostic Detected";
            address = getAddr(canId);

            switch(address){
            case LINEAR_ADR:{
                qDebug() << "Linear";
                PreviousStates = LinearNewFaults;
                LinearNewFaults = (payload.at(FMI_POS) & FMI_MASK);
                emit linearNewFaultsChanged();
                qDebug() << LinearNewFaults;
                break;
            }
            case TEMP_ADR:{
                qDebug() << "Temperature";
                PreviousStates = TemperatureNewFaults;
                TemperatureNewFaults = (payload.at(FMI_POS) & FMI_MASK);
                emit temperatureNewFaultsChanged();
                qDebug() << TemperatureNewFaults;
                break;
            }
            case POS_ADR:{
                qDebug() << "Position";
                PreviousStates = PositionNewFaults;
                PositionNewFaults = (payload.at(FMI_POS) & FMI_MASK);
                emit positionNewFaultsChanged();
                qDebug() << PositionNewFaults;
                break;
            }
            }
            break;
        }

        case LINEAR_DISPLACEMENT_PGN:{
            qDebug() << "Linear Displacement Data Detected";
            data = (uint(payload.at(LINEAR_DISPLACEMENT_MSB)) << MSB_SHIFT_POSITION) |
                    uint(payload.at(LINEAR_DISPLACEMENT_LSB));
            LinearDisplacement = static_cast<int>(data) / LINEAR_DISPLACEMENT_CONSTANT;
            qDebug() << LinearDisplacement;
            emit linearChanged();
            break;
        }

        case ENGINE_TEMPERATURE_PGN:{
            qDebug() << "Engine Temperature Data Detected";
            data = (uint(payload.at(ENGINE_TEMPERATURE_B)));

            Temperature = static_cast<int>(data) - ENGINE_TEMPERATURE_OFFSET;
            qDebug() << data;
            emit temperatureChanged();
            break;
        }

        case VEHICLE_POSITION_PGN:{
            qDebug() << "Vehicle Position Data Detected";
            data = (uint(payload.at(VEHICLE_POSITION_X_MSB)) << MSB_SHIFT_POSITION) |
                    uint(payload.at(VEHICLE_POSITION_X_LSB));
            data2 = (uint(payload.at(VEHICLE_POSITION_Y_MSB)) << MSB_SHIFT_POSITION) |
                    uint(payload.at(VEHICLE_POSITION_Y_LSB));
            xpos = static_cast<int>(data);
            ypos = static_cast<int>(data2);
            qDebug() << data;
            qDebug() << data2;
            emit xPosChanged();
            emit yPosChanged();
            break;
        }

        case VEHICLE_ORIENTATION_PGN:{
            qDebug() << "Vehicle Orientation Data Detected";
            data = (uint(payload.at(VEHICLE_ORIENTATION_X_MSB)) << MSB_SHIFT_POSITION) |
                    uint(payload.at(VEHICLE_ORIENTATION_X_LSB));
            OrientationDegrees = static_cast<int>(data) / ORIENTATION_DEGREES_CONSTANT;
            qDebug() << OrientationDegrees;
            emit orientationChanged();
            break;
        }

        case TEST_PGN:{
            qDebug() << "Test PGN detected";
            quint16 PGN = DM4_TEST_PGN;
            QByteArray payload(BYTE_DATA_PER_PACKET, 0xFF);
            payload[3] = 0x00;
            qDebug() << Payload;
            frame = prepareCANFrame(PGN, 0x00, payload);
            m_canDevice->writeFrame(frame);
            break;
        }
        }
    }
}

/******************************************************************************
* FUNCTION: j1939::getPGN(quint32 canId)
*
* DESCRIPTION: This fuction obtain the PGN from the id segment of a can
*              message.
*
* PARAMETERS:  canId - is the id segment of the can frame.
*
* Return:      The PGN from the id segment.
******************************************************************************/
quint32 j1939::getPGN(quint32 canId) {
    quint32 PGN = (canId & PGN_MASK) >> PGN_SHIFT_POSITION;
    //qDebug() << "PGN: " << PGN;
    return PGN;
}

quint8 j1939::getAddr(quint32 canId){
    quint8 addr = (canId & ADR_MASK);
    return addr;
}

/******************************************************************************
* FUNCTION: j1939::readRPM()
*
* DESCRIPTION: This fuction represent the data of RPM with a resolution of
*              0.125rpm per bit. It is used to read the data from qml.
*
* PARAMETERS:  None
*
* Return:      The j1939Data
******************************************************************************/

double j1939::readRPM() const {
    return RPM;
}

/******************************************************************************
* FUNCTION: j1939::readfuelLevel()
*
* DESCRIPTION: This fuction represent the fuel level data with a resolution of
*              0.4% per bit. It is used to read the data from qml.
*
* PARAMETERS:  None
*
* Return:      The j1939Data
******************************************************************************/

double j1939::readFuelLevel() const {
    return FuelLevel;
}

double j1939::readLinear() const{
    return LinearDisplacement;
}

/******************************************************************************
* FUNCTION: j1939::readTemperature()
*
* DESCRIPTION: This fuction represent the data of Temperature with a resolution
*              of 1°C per bit. It is used to read the data from qml.
*
* PARAMETERS:  None
*
* Return:      the j1939Data
******************************************************************************/

int j1939::readTemperature() const {
    return Temperature;
}

uint8_t j1939::readTempSP() const{
    return tempSP;
}

uint8_t j1939::readLinearSP() const{
    return linearSP;
}

int j1939::readXPos() const{
    return xpos;
}

int j1939::readYPos() const{
    return ypos;
}

void j1939::setTempSP(QString n){
    tempSP += n.toInt();
    if (tempSP == 251)
        tempSP = 0;
    else if (tempSP == 255)
        tempSP = 250;
    qDebug() << "tempSP =" << tempSP;
    emit tempSPChanged();
}

void j1939::setLinearSP(QString n){
    linearSP += n.toInt();
    if (linearSP == 251)
        linearSP = 0;
    else if (linearSP == 255)
        linearSP = 250;
    qDebug() << "linearSP =" << linearSP;
    emit linearSPChanged();
}

double j1939::readOrientation() const{
    return OrientationDegrees;
}

/******************************************************************************
* FUNCTION: j1939::readTachometerNewFaults()
*
* DESCRIPTION: This fuction represent the DTC with a 8 bit variable with a
*              designated bit per FMI used to read the data from qml.
*
* PARAMETERS: None
*
* Return: the j1939Data
******************************************************************************/

quint8 j1939::readTachometerNewFaults() const {
    return TachometerNewFaults;
}

/******************************************************************************
* FUNCTION: j1939::readFuelGaugeNewFaults()
*
* DESCRIPTION: This fuction represent the DTC with a 8 bit variable with a
*              designated bit per FMI used to read the data from qml.
*
* PARAMETERS: None
*
* Return:     the j1939Data
******************************************************************************/

quint8 j1939::readFuelGaugeNewFaults() const {
    return FuelGaugeNewFaults;
}

/******************************************************************************
* FUNCTION: j1939::readThermometerNewFaults()
*
* DESCRIPTION: This fuction represent the DTC with a 8 bit variable with a
*              designated bit per FMI used to read the data from qml.
*
* PARAMETERS:  None
*
* Return:      the j1939Data
******************************************************************************/

quint8 j1939::readThermometerNewFaults() const {
    return ThermometerNewFaults;
}

quint8 j1939::readLinearNewFaults() const{
    return LinearNewFaults;
}

quint8 j1939::readTemperatureNewFaults() const{
    return TemperatureNewFaults;
}

quint8 j1939::readPositionNewFaults() const{
    return PositionNewFaults;
}

