#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "defines.h"
#include "time.h"
#include "task.hpp"
#include "vision.h"
#include <QMessageBox>
#include <QDateTime>
#include <qprocess>
#include <QDesktopServices>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	ui->menuBar->setStyleSheet("QMenuBar{background-color:#646464;} QMenuBar::item {background: #808080;color:#F0F0F0}");
	setMinimumSize(1032, 654);
	setWindowState(Qt::WindowMaximized);

	splitWindow();

	thePropeller = new Propeller(this);
	theSwitchLED = new SwitchLED(this);

	//创建通信线程 TO-DO:将其写到函数中
	comm = new Communication;
	commThread = new QThread(this);
	comm->moveToThread(commThread);
	connect(commThread, &QThread::finished, comm, &Communication::deleteLater);
	connect(commThread, &QThread::finished, commThread, &QThread::deleteLater);
	connect(this, &MainWindow::creatComm, comm, &Communication::create);
	commThread->start();
	emit creatComm();

	//创建自动控制线程
	autoControl = new AutoControl;
	autoDepthThread = new QThread(this);
	autoControl->moveToThread(autoDepthThread);
	connect(autoDepthThread, &QThread::finished, autoControl, &AutoControl::deleteLater);
	connect(autoDepthThread, &QThread::finished, autoDepthThread, &QThread::deleteLater);
	connect(this, &MainWindow::creatAutoCtrl, autoControl, &AutoControl::create);
	autoDepthThread->start();
	emit creatAutoCtrl();

	constructConnect();

	ui->actChartAniSwitch->setChecked(false);
	ui->act10Points->setChecked(true);
	pLogPane->addItem(QSL("系统编译时间：%1 - %2").arg(__DATE__).arg(__TIME__));
	pLogPane->addItem(QSL("系统启动于：%1").arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyy年MM月dd日 hh:mm:ss"))));
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
	Q_UNUSED(event);
	pChartPane->setFixedHeight(height()/5*2);
}

void MainWindow::keyPressEvent(QKeyEvent *e)
{
	qDebug()<<"MainWindow::keyPressEvent";
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	if(theSwitchLED->isSwitchOn)
	{
		switch(QMessageBox::warning(this, QSL("退出确认"), QSL("尚未手动关闭电源，关闭电源并退出？"), QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel))
		{
		case QMessageBox::Yes:
			theSwitchLED->propSwitchSlot(false);
			theSwitchLED->camSwitchSlot(false);
			event->accept();
			break;
		case QMessageBox::No:
			event->accept();
			break;
		case QMessageBox::Cancel:
		default:
			event->ignore();
			return;
		}
	}

	//退出自动控制线程
	autoDepthThread->quit();
	autoDepthThread->wait();
//	autoDepthThread->deleteLater();

	//退出通信线程
	commThread->quit();
	commThread->wait();
//	commThread->deleteLater();
}

void MainWindow::splitWindow()
{
	verSplter = new QSplitter(Qt::Horizontal, this);
	setCentralWidget(verSplter);
	verSplter->setStyleSheet("QSplitter::handle{background-color:#646464}");

	//第一列：水平分隔线将窗口分为竖直的两部分
	hozSplter1 = new QSplitter(Qt::Vertical, verSplter);

	//第一列第一行 controlpane
	pControlPane = new ControlPane(hozSplter1);
	QPalette ControlPal(pControlPane->palette());
	ControlPal.setColor(QPalette::Background, QColor(255,255,255));
	pControlPane->setAutoFillBackground(true);
	pControlPane->setPalette(ControlPal);
	pControlPane->setFixedSize(pControlPane->width(),pControlPane->height());

	//第一列第二行 logPane
	pLogPane = new LogPane(hozSplter1);
	pLogPane->setFixedWidth(pControlPane->width());

	//第二列
	hozSplter2 = new QSplitter(Qt::Vertical, verSplter);

	//第二列第一行 opencvPane
	pOpenCVPane = new OpenCVPane(hozSplter2);
	QPalette openCVPal(pOpenCVPane->palette());
	openCVPal.setColor(QPalette::Background, QColor(255,255,255));
	pOpenCVPane->setAutoFillBackground(true);
	pOpenCVPane->setPalette(openCVPal);

	//第二列第二行 chartPane

	pChartPane = new ChartPane(hozSplter2);
//	QPalette curvePal(pChartPane->palette());
//	curvePal.setColor(QPalette::Background, QColor(255,255,255));
//	pChartPane->setAutoFillBackground(true);
//	pChartPane->setPalette(curvePal);

	//设定拖动因数
	verSplter->setStretchFactor(0, 0);
	verSplter->setStretchFactor(1, 1);
	hozSplter1->setStretchFactor(0, 0);
	hozSplter1->setStretchFactor(1, 1);
	hozSplter2->setStretchFactor(0, 3);
	hozSplter2->setStretchFactor(1, 2);

	//禁用竖直分割线的拖动功能和第一个水平分隔线的拖动功能
	QSplitterHandle *pSplterHdl = verSplter->handle(1);
	if (pSplterHdl) pSplterHdl->setDisabled(true);
	pSplterHdl = verSplter->handle(2);
	if (pSplterHdl) pSplterHdl->setDisabled(true);
	pSplterHdl = hozSplter1->handle(1);
	if (pSplterHdl) pSplterHdl->setDisabled(true);
	pSplterHdl = hozSplter1->handle(2);
	if (pSplterHdl) pSplterHdl->setDisabled(true);
	pSplterHdl = hozSplter2->handle(1);
	if (pSplterHdl) pSplterHdl->setDisabled(true);
	pSplterHdl = hozSplter2->handle(2);
	if (pSplterHdl) pSplterHdl->setDisabled(true);
}

void MainWindow::constructConnect()
{
	//注册元数据类型
	qRegisterMetaType<LogPane::WarnLevel>("LogPane::WarnLevel");
	qRegisterMetaType<PostureData>("PostureData");
	qRegisterMetaType<LEDSetting>("LEDSetting");
	qRegisterMetaType<Task::TaskEndStatus>("Task::TaskEndStatus");
	qRegisterMetaType<cv::Mat>("cv::Mat");

	//控制面板发出的信号--------------------------------------------------------------------------------------------------------
	connect(pControlPane, &ControlPane::mannualGoSgl, thePropeller, &Propeller::mannualGoDelta);		//控制面板->推进器：手动前进
	connect(pControlPane, &ControlPane::mannualTurnSgl, thePropeller, &Propeller::mannualTurnDelta);	//控制面板->推进器：手动转向
	connect(pControlPane, &ControlPane::mannualOffsetSgl, thePropeller, &Propeller::mannualOffsetDelta);//控制面板->推进器：手动侧移
	connect(pControlPane, &ControlPane::taskGoSgl, thePropeller, &Propeller::taskGoDelta);				//控制面板->推进器：任务前进
	connect(pControlPane, &ControlPane::taskTurnSgl, thePropeller, &Propeller::taskTurnDelta);			//控制面板->推进器：任务转向
	connect(pControlPane, &ControlPane::taskOffsetSgl, thePropeller, &Propeller::taskOffsetDelta);		//控制面板->推进器：任务侧移
	connect(pControlPane, &ControlPane::taskDiveSgl, thePropeller, &Propeller::taskDiveDelta);			//控制面板->推进器：任务浮潜
	connect(pControlPane, &ControlPane::automaticSwitch, thePropeller, &Propeller::autoCtrlRunning);	//控制面板->推进器：自动控制开关

	connect(pControlPane, &ControlPane::propSwitch, theSwitchLED, &SwitchLED::propSwitchSlot);			//控制面板->开关LED：推进器开关
	connect(pControlPane, &ControlPane::camSwitch, theSwitchLED, &SwitchLED::camSwitchSlot);			//控制面板->开关LED：摄像头开关
	connect(pControlPane, &ControlPane::LEDFlash, theSwitchLED, &SwitchLED::setLED);					//控制面板->开关LED：LED颜色闪烁频率

	connect(pControlPane, &ControlPane::autoDepthSwitch, autoControl, &AutoControl::autoDepthSwitch);	//控制面板->自动控制：定深闭环开关
	connect(pControlPane, &ControlPane::setAutoDepth, autoControl, &AutoControl::setDepthTgt);			//控制面板->自动控制：设定定深值
	connect(pControlPane, &ControlPane::autoHeadingSwitch, autoControl, &AutoControl::autoHeadingSwitch);//控制面板->自动控制：定航闭环开关
	connect(pControlPane, &ControlPane::setAutoHeading, autoControl, &AutoControl::setHeadingTgt);		//控制面板->自动控制：设定定航值
	connect(pControlPane, &ControlPane::autoPitchSwitch, autoControl, &AutoControl::autoPitchSwitch);	//控制面板->自动控制：俯仰闭环开关

	connect(pControlPane, &ControlPane::propOrder, comm, &Communication::sendToPropeller);				//控制面板->通信：直接发送推进器指令

	connect(pControlPane, &ControlPane::videoImg, pOpenCVPane, &OpenCVPane::ImgDispSlot);				//控制面板->opencv：显示

	connect(pControlPane, &ControlPane::setAutoDepth, pChartPane, &ChartPane::setTarget);				//控制面板->图表面板：深度目标

	connect(pControlPane, &ControlPane::operateLog, pLogPane, &LogPane::addString);						//控制面板->日志面板：日志
	//--------------------------------------------------------------------------------------------------------

	//自动控制类发出的信号--------------------------------------------------------------------------------------------------------
	connect(autoControl, &AutoControl::autoDepthProp, thePropeller, &Propeller::autoDepthDelta);		//自动控制->推进器：自动深度
	connect(autoControl, &AutoControl::autoPitchProp, thePropeller, &Propeller::autoPitchDelta);		//自动控制->推进器：自动俯仰
	connect(autoControl, &AutoControl::autoHeadingProp, thePropeller, &Propeller::autoHeadingDelta);	//自动控制->推进器：自动航向

	connect(autoControl, &AutoControl::operateLog, pLogPane, &LogPane::addString);						//自动控制->日志面板：日志
	//--------------------------------------------------------------------------------------------------------

	//推进器类发出的信号--------------------------------------------------------------------------------------------------------
	connect(thePropeller, &Propeller::relativeThrust, pControlPane, &ControlPane::setThrusterDisp);		//推进器->控制面板：推进值显示

	connect(thePropeller, &Propeller::order, comm, &Communication::sendToPropeller);					//推进器->通信：推进器指令
	//--------------------------------------------------------------------------------------------------------

	//通信类发出的信号--------------------------------------------------------------------------------------------------------
	connect(comm, &Communication::LED, pControlPane, &ControlPane::setLEDColor);				//通信->控制面板：LED颜色
	connect(comm, &Communication::depth, pControlPane, &ControlPane::setDepth);					//通信->控制面板：深度数据
	connect(comm, &Communication::depth, pControlPane, &ControlPane::depthSensor);				//通信->控制面板：深度数据
	connect(comm, &Communication::posture, pControlPane, &ControlPane::setPosture);				//通信->控制面板：姿态数据
	connect(comm, &Communication::posture, pControlPane, &ControlPane::postureSensor);			//通信->控制面板：姿态数据

	connect(comm, &Communication::leakWater, pControlPane, &ControlPane::waterLeakWarnSlot);	//通信->控制面板：漏液检测
//	connect(comm, &Communication::recvedMsg, pControlPane, &ControlPane::setRecvMsg);			//通信->控制面板：接收到的信息
//	connect(comm, &Communication::sendedMsg, pControlPane, &ControlPane::setSendMsg);			//通信->控制面板：发送的信息

	connect(comm, &Communication::depth, autoControl, &AutoControl::depthData);					//通信->自动控制：深度数据
	connect(comm, &Communication::posture, autoControl, &AutoControl::postureData);				//通信->自动控制：姿态数据

	connect(comm, &Communication::depth, this, &MainWindow::depthDispSlot);						//通信->mainwindow：深度数据图表显示
	connect(comm, &Communication::leakWater, this, &MainWindow::waterLeakWarnSlot);				//通信->mainwindow：漏液检测

	connect(comm, &Communication::operateLog, pLogPane, &LogPane::addString);					//通信->日志面板：日志
	//--------------------------------------------------------------------------------------------------------

	//开关及LED类发出的信号--------------------------------------------------------------------------------------------------------
	connect(theSwitchLED, &SwitchLED::switchOrder, comm, &Communication::sendToSwitchArduino);			//开关LED->通信：开关指令
	connect(theSwitchLED, &SwitchLED::LEDOrder, comm, &Communication::sendToLEDArduino);				//开关LED->通信：LED指令
	//LED列表为空
	//--------------------------------------------------------------------------------------------------------
//	connect(this, &MainWindow::startAutoCtrl, autoControl, &autoControl::start);
}

void MainWindow::depthDispSlot(double d)
{
	static unsigned int cnt = 9;
	if(cnt >= chartDispCnt)
	{
		pChartPane->addData(d);
		cnt = 0;
	}
	else
		cnt++;
}

void MainWindow::waterLeakWarnSlot()
{
	LEDSetting s;
	s.changeTime = 200;
	s.flashTimes = 0;
	s.colorList<<QColor(255,0,0)<<QColor(0,0,0);
	theSwitchLED->setLED(s);
	static bool isWarned = false;
	if(!isWarned)
	{
		isWarned = true;
		pLogPane->addString(QSL("漏液检测：舱内已侦测到液体，请及时处理"), LogPane::err);
		QMessageBox::critical(this, QSL("检测到舱内漏液"), QSL("漏液检测装置检测到舱内存在液体，及时处理"));
		QPalette ControlPal(pControlPane->palette());
		ControlPal.setColor(QPalette::Background, QColor(255,0,0));
		pControlPane->setPalette(ControlPal);
	}
}

void MainWindow::on_actChartAniSwitch_triggered(bool checked)
{
	pChartPane->animation(checked);
}

void MainWindow::on_act10Points_triggered(bool checked)
{
	if(checked)
	{
		ui->act20Points->setChecked(false);
		ui->act50Points->setChecked(false);
		ui->act100Points->setChecked(false);
		ui->actChartAniSwitch->setEnabled(true);
//		timer->setInterval(1000);
		chartDispCnt = 10 - 1;
		pChartPane->setDispCount(11);
		pChartPane->chart->setAnimationDuration(950);
	}
	else
	{
		ui->act10Points->setChecked(true);
	}
}

void MainWindow::on_act20Points_triggered(bool checked)
{
	if(checked)
	{
		ui->act10Points->setChecked(false);
		ui->act50Points->setChecked(false);
		ui->act100Points->setChecked(false);
		ui->actChartAniSwitch->setEnabled(true);
		chartDispCnt = 5 - 1;
		pChartPane->setDispCount(21);
		pChartPane->chart->setAnimationDuration(450);
	}
	else
	{
		ui->act20Points->setChecked(true);
	}
}

void MainWindow::on_act50Points_triggered(bool checked)
{
	if(checked)
	{
		ui->act10Points->setChecked(false);
		ui->act20Points->setChecked(false);
		ui->act100Points->setChecked(false);
		ui->actChartAniSwitch->setEnabled(true);
		chartDispCnt = 2 - 1;
		pChartPane->setDispCount(51);
		pChartPane->chart->setAnimationDuration(150);
	}
	else
	{
		ui->act50Points->setChecked(true);
	}
}

void MainWindow::on_act100Points_triggered(bool checked)
{
	if(checked)
	{
		ui->act10Points->setChecked(false);
		ui->act20Points->setChecked(false);
		ui->act50Points->setChecked(false);
		ui->actChartAniSwitch->setEnabled(false);
		chartDispCnt = 1 - 1;
		pChartPane->setDispCount(101);
		pChartPane->chart->setAnimationOptions(QChart::NoAnimation);
	}
	else
	{
		ui->act100Points->setChecked(true);
	}
}

void MainWindow::on_actLogFolder_triggered()
{
	QDesktopServices::openUrl(QUrl(RECORD_DIR));
}

void MainWindow::on_actRecordData_triggered(bool checked)
{
	QMessageBox::information(this,QSL("%1执行了").arg(__FUNCTION__),QSL("状态是%1, 还没有写好").arg(checked),QMessageBox::Ok);
}
