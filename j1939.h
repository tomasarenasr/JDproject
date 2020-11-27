#ifndef J1939_H
#define J1939_H

#include <QtGlobal>
#include <QByteArray>
#include <QCanBusFrame>
#include <QCanBusDevice>
#include <QCanBus>
#include <QObject>
#include <QDebug>
#include <QTimer>
#include <QColor>
#include <QMetaType>
#include "j1939_config.h"

/******************************************************************************
 *
 * Class: J1939
 *
 * This class is used to model the J1939 protocol software layer, on top of
 * CAN hardware. it includes basic functionalities like automatic reception and
 * decoding of CAN frames, a frame constructor, and a J1939 interpretator that
 * automatically updates data and DTCs for easy access.
 *
 * note: uncomment qDebug() lines to output debug data on console.
 *
******************************************************************************/

class j1939 : public QObject {
    Q_OBJECT
    Q_PROPERTY(int xpos READ readXPos NOTIFY xPosChanged)
    Q_PROPERTY(int ypos READ readYPos NOTIFY yPosChanged)
    Q_PROPERTY(int tempSP READ readTempSP
               NOTIFY tempSPChanged)
    Q_PROPERTY(int linearSP READ readLinearSP
               NOTIFY linearSPChanged)
    Q_PROPERTY(double FuelLevel READ readFuelLevel
               NOTIFY fuelLevelChanged)
    Q_PROPERTY(int Temperature READ readTemperature
               NOTIFY temperatureChanged)
    Q_PROPERTY(double LinearDisplacement READ readLinear
               NOTIFY linearChanged)
    Q_PROPERTY(double OrientationDegrees READ readOrientation
               NOTIFY orientationChanged)
    Q_PROPERTY(quint8 TachometerNewFaults READ readTachometerNewFaults
               NOTIFY tachometerNewFaultsChanged)
    Q_PROPERTY(quint8 FuelGaugeNewFaults READ readFuelGaugeNewFaults
               NOTIFY fuelGaugeNewFaultsChanged)
    Q_PROPERTY(quint8 ThermometerNewFaults READ readThermometerNewFaults
               NOTIFY thermometerNewFaultsChanged)

    Q_PROPERTY(quint8 LinearNewFaults READ readLinearNewFaults
               NOTIFY linearNewFaultsChanged)
    Q_PROPERTY(quint8 TemperatureNewFaults READ readTemperatureNewFaults
               NOTIFY temperatureNewFaultsChanged)
    Q_PROPERTY(quint8 PositionNewFaults READ readPositionNewFaults
               NOTIFY positionNewFaultsChanged)
public:
    /**************************************************************************
   *
   * Enum: Device_E
   *
   * This enumeration is used to identify the target device as an argument
   * for slot functions.
   *
   **************************************************************************/
    enum Device_E {
        D_THERMOMETER,
        D_FUEL_GAUGE,
        D_TACHOMETER
    };
    Q_ENUMS(Device_E)

    explicit j1939(QObject *parent = nullptr);
    quint32 getPGN(quint32 canId);
    quint8 getAddr(quint32 canId);
    QCanBusFrame prepareCANFrame(quint16 PGN, quint8 addr, QByteArray payload);
    QCanBusFrame sendTestFrame(quint16 PGN, QByteArray payload);
    ~j1939();

public slots:
    void connectDevice();
    void processFrames();
    void sendStatusReset(QString n);
    void sendData(QString n);
    //Functions to set data for sending
    void setTempSP(QString n);
    void setLinearSP(QString n);

signals:
    void canBusConnected();
    void rpmChanged();
    void xPosChanged();
    void yPosChanged();
    void linearChanged();
    void tempSPChanged();
    void linearSPChanged();
    void fuelLevelChanged();
    void orientationChanged();
    void temperatureChanged();
    void linearNewFaultsChanged();
    void positionNewFaultsChanged();
    void tachometerNewFaultsChanged();
    void fuelGaugeNewFaultsChanged();
    void temperatureNewFaultsChanged();
    void thermometerNewFaultsChanged();

private:
    // constants used in conversion between byte array values and actual values
    const double RPM_CONVERSION_CONSTANT =        0.125;
    const double FUEL_LEVEL_CONVERSION_CONSTANT = 0.4;
    const double LINEAR_DISPLACEMENT_CONSTANT =   10;
    const quint8 ENGINE_TEMPERATURE_OFFSET =      40;
    const double ORIENTATION_DEGREES_CONSTANT =   128;

    //variables used for interpretation of CAN Frames
    quint8 Priority;
    quint8 ExtendedData;
    quint8 DataPage;
    quint8 PDUFormat;
    quint8 PDUSpecific;
    quint8 SourceAddress;
    QByteArray Payload;

    //variables used to store DTC and data values
    quint8 TachometerFaultStates;
    quint8 TachometerNewFaults;
    quint8 FuelGaugeFaultStates;
    quint8 FuelGaugeNewFaults;
    quint8 ThermometerFaultStates;
    quint8 ThermometerNewFaults;
    quint8 LinearFaultStates;
    quint8 LinearNewFaults;
    quint8 TemperatureFaultStates;
    quint8 TemperatureNewFaults;
    quint8 PositionFaultStates;
    quint8 PositionNewFaults;

    double RPM = 0;
    double FuelLevel = 0;
    int Temperature = 0;
    double LinearDisplacement = 0;
    double OrientationDegrees = 0;
    int xpos = 0;
    int ypos = 0;
    uint8_t tempSP = 25;
    uint8_t linearSP = 25;
    int boton = 0;

    //additional variables for instances of classes required for operation
    QCanBusDevice *m_canDevice = nullptr;

    //Functions to access the data from qml throught Q_PROPERTY
    double readRPM() const;
    double readFuelLevel() const;
    int readTemperature() const;
    uint8_t readTempSP() const;
    uint8_t readLinearSP() const;
    int readXPos() const;
    int readYPos() const;
    double readLinear() const;
    double readOrientation() const;
    quint8 readTachometerNewFaults() const;
    quint8 readFuelGaugeNewFaults() const;
    quint8 readThermometerNewFaults() const;
    quint8 readLinearNewFaults() const;
    quint8 readTemperatureNewFaults() const;
    quint8 readPositionNewFaults() const;
};

#endif // CAN_H
