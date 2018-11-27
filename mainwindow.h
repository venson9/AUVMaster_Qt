#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include <QTimer>
#include <QThread>
#include <QPointer>
#include "controlpane.h"
#include "logpane.h"
#include "opencvpane.h"
#include "chartpane.h"
#include "communication.h"
#include "propeller.h"
#include "autocontrol.h"
#include "switchled.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

signals:
	void test(double d);
	void creatComm();
	void creatAutoCtrl();
	void startAutoCtrl();

protected:
	void resizeEvent(QResizeEvent *event);
	void keyPressEvent(QKeyEvent *e);
	void closeEvent(QCloseEvent *event);

private:
	Ui::MainWindow *ui;

	QTimer *timer;

	ControlPane *pControlPane = nullptr;
	LogPane *pLogPane = nullptr;
//	ChartPane *pChartPane = nullptr;
	ChartPane *pChartPane = nullptr;
	OpenCVPane *pOpenCVPane = nullptr;

	QSplitter *verSplter;	//水平分隔条123
	QSplitter *hozSplter1;	//竖直分隔条
	QSplitter *hozSplter2;	//竖直分隔条

	QPointer<Communication> comm = nullptr;
	QPointer<QThread> commThread = nullptr;

	QPointer<AutoControl> autoControl = nullptr;
	QPointer<QThread> autoDepthThread = nullptr;

	Propeller *thePropeller;
	SwitchLED *theSwitchLED;

	unsigned int chartDispCnt = 9;//每收到chartDispCnt+1个数据，就在pChartPane上添加一个点

	void splitWindow();
	void constructConnect();

private slots:
	void depthDispSlot(double d);
	void waterLeakWarnSlot();

	void on_actChartAniSwitch_triggered(bool checked);
	void on_act10Points_triggered(bool checked);
	void on_act20Points_triggered(bool checked);
	void on_act50Points_triggered(bool checked);
	void on_act100Points_triggered(bool checked);
	void on_actLogFolder_triggered();
	void on_actRecordData_triggered(bool checked);
};

#endif // MAINWINDOW_H
