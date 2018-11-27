#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <QObject>
#include <QString>
#include <QSerialPort>
#include <QTimer>
#include "logpane.h"
#include "structs.h"

class Communication : public QObject
{
	Q_OBJECT
public:
	explicit Communication(QObject *parent = 0);
	~Communication();

signals:
	void depth(double d);
	void posture(PostureData);
	void propSwitch(bool b);
	void camSwitch(bool b);
	void LED(QColor);
	void leakWater();
	void operateLog(QString s, LogPane::WarnLevel level);
	void recvedMsg(QString sender, QString msg);
	void sendedMsg(QString sender, QString msg);

public slots:
	void create();
	void setZeroDepth();
	void sendToPropeller(QString s);
	void sendToSwitchArduino(QString s);
	void sendToLEDArduino(QString s);
	void sendToDepthSensor(QString s);

private slots:
	void readPropellerPort();
	void readSwitchArduinoPort();
	void readLEDArduinoPort();
	void readDepthSensorPort();
	void readPostureSensorPort();

private:
	QSerialPort *propellerPort;
	QString propellerRecvStr;

	QSerialPort *switchPort;
	QString switchArduinoRecvStr;

	QSerialPort *LEDPort;
	QString LEDArduinoRecvStr;
	QColor LEDColor;

	QSerialPort *depthPort;
	QString depthSensorRecvStr;
	QTimer *depthTimer;
	bool isRecali = true;
	double zeroDepth = 0;

	QSerialPort *postureSensorPort;
	QString postureSensorRecvStr;

	bool openPropellerPort();
	bool openSwitchPort();
	bool openLEDPort();
	bool openDepthPort();
	bool openPostureSensorPort();

	void propellerDataProc(QString &data);//舵机控制板返回信息处理
	void switchArduinoDataProc(QString &data);//开关arduino返回数据处理
	void LEDArduinoDataProc(QString &data);//LED arduino返回数据处理
	void depthSensorDataProc(QString &data);//深度传感器返回数据处理
	void postureSensorDataProc(QString &data);//姿态传感器返回数据处理
};

#endif // COMMUNICATION_H
