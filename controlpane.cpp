#include "controlpane.h"
#include "ui_controlpane.h"
#include "logpane.h"
#include "defines.h"
#include "task_cruise.h"
#include "task_light.h"
#include "task_color.h"
#include "task_test.h"
#include "vision.h"
#include <QDateTime>
#include <QMessageBox>
#include <exception>
#include <assert.h>

ControlPane::ControlPane(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::ControlPane)
{
	ui->setupUi(this);
	ui->btnStdCtrl->setChecked(true);
	ui->stackedWidget->setCurrentIndex(0);

	//时间显示定时器
	timeDispTimer = new QTimer (this);//定义定时器
	if(timeDispTimer)
	{
		connect(timeDispTimer,SIGNAL(timeout()),this,SLOT(timeDispTimerUpdate()));
		timeDispTimer->start(1000);
	}

	//连接信号槽
	connect(ui->btnClearMsg, &QPushButton::clicked, ui->lstCommRecv, &QListWidget::clear);
	connect(ui->btnClearMsg, &QPushButton::clicked, ui->lstCommSend, &QListWidget::clear);

	//设置灯色显示
	ui->LEDColor->setStyleSheet(QSL("background-color: qradialgradient(spread:pad, cx:0.5, cy:0.5, radius:0.5, fx:0.5, fy:0.5, stop:0 rgba(0, 0, 0, 255), stop:1 rgba(255, 255, 255, 255));"));

	//设置闭环值显示
	if(ui->btnDepthClosedLoop->isChecked())
		ui->depthVal->setStyleSheet("background-color: rgb(190, 250, 250);");
	else
		ui->depthVal->setStyleSheet("background-color: rgb(255, 220, 170);");

	if(ui->btnCourseClosedLoop->isChecked())
		ui->courseVal->setStyleSheet("background-color: rgb(190, 250, 250);");
	else
		ui->courseVal->setStyleSheet("background-color: rgb(255, 220, 170);");

	//为任务列表添加项目
	ui->taskList->addItem(Task_Cruise::taskName);
	ui->taskList->addItem(Task_Light::taskName);
	ui->taskList->addItem(Task_Color::taskName);
	ui->taskList->addItem(Task_Test::taskName);
	ui->taskList->setCurrentRow(0);
}

ControlPane::~ControlPane()
{
	//如果当前有任务仍然在执行，销毁当前任务
	if(!curTaskThread.isNull() && curTaskThread->isRunning())
	{
		destoryCurTask();
		taskList.removeFirst();
	}

	if(!taskList.isEmpty())
	{
		foreach (Task *pt, taskList) {
			delete pt;
		}
	}

	if(timeDispTimer)
		delete timeDispTimer;
	delete ui;
}

void ControlPane::setLEDColor(QColor c)
{
//	ui->LEDColor->setText(QSL("%1,%2,%3").arg((bool)(c.red())).arg((bool)(c.green())).arg((bool)(c.blue())));
	ui->LEDColor->setStyleSheet(QSL("background-color: qradialgradient(spread:pad, cx:0.5, cy:0.5, radius:0.5, fx:0.5, fy:0.5, stop:0 rgba(%1, %2, %3, 255), stop:1 rgba(255, 255, 255, 255));")\
								.arg(c.red()).arg(c.green()).arg(c.blue()));
}

void ControlPane::setPosture(PostureData p)
{
	ui->txtYaw->setText(QSL("%1°(%2°/s)").arg(p.yaw, 0, 'f', 2).arg(p.yawSpd, 0, 'f', 2));
	ui->txtPitch->setText(QSL("%1°(%2°/s)").arg(p.pitch, 0, 'f', 2).arg(p.pitSpd, 0, 'f', 2));
	ui->txtRoll->setText(QSL("%1°(%2°/s)").arg(p.roll, 0, 'f', 2).arg(p.rolSpd, 0, 'f', 2));
	curPos = p;
}

void ControlPane::setDepth(double d)
{
	ui->txtDepth->setText(QSL("%1cm").arg(d));
	curDepth = d;
}

void ControlPane::setThrusterDisp(int t1, int t2, int t3, int t4, int t5, int t6)
{
	ui->txtThr1->setText(QString::number(t1));
	ui->txtThr2->setText(QString::number(t2));
	ui->txtThr3->setText(QString::number(t3));
	ui->txtThr4->setText(QString::number(t4));
	ui->txtThr5->setText(QString::number(t5));
	ui->txtThr6->setText(QString::number(t6));
}

void ControlPane::waterLeakWarnSlot()
{
	emit camSwitch(false);
	emit propSwitch(false);
	ui->btnThrsSwitch->setChecked(false);
	ui->btnCamSwitch->setChecked(false);
}

void ControlPane::setRecvMsg(QString name, QString msg)
{
	if(!ui->btnPauseMsg->isChecked())
	{
		static unsigned int cnt = 0;
		ui->txtRecvCnt->setText(QSL("%1").arg(++cnt));

		if(ui->lstCommRecv->count() > 300)
			ui->lstCommRecv->clear();

		if((QSL("深度") == name && ui->chkDepthMsg->isChecked()) ||
				(QSL("姿态") == name && ui->chkPosMsg->isChecked()) ||
				(QSL("LED") == name && ui->chkLEDMsg->isChecked()) ||
				(QSL("开关") == name && ui->chkSwitchMsg->isChecked()))
		{
			ui->lstCommRecv->addItem(QSL("%1:%2").arg(name, msg));
			ui->lstCommRecv->setCurrentRow(ui->lstCommRecv->count()-1);
		}
	}
}

void ControlPane::setSendMsg(QString name, QString msg)
{
	if(!ui->btnPauseMsg->isChecked())
	{
		static unsigned int cnt= 0;
		ui->txtSendCnt->setText(QSL("%1").arg(++cnt));

		if(ui->lstCommSend->count() > 300)
			ui->lstCommSend->clear();

		if((QSL("深度") == name && ui->chkDepthMsg->isChecked()) ||
				(QSL("姿态") == name && ui->chkPosMsg->isChecked()) ||
				(QSL("LED") == name && ui->chkLEDMsg->isChecked()) ||
				(QSL("开关") == name && ui->chkSwitchMsg->isChecked()))
		{
			ui->lstCommSend->addItem(QSL("%1:%2").arg(name, msg));
			ui->lstCommSend->setCurrentRow(ui->lstCommSend->count()-1);
		}
	}
}

void ControlPane::addTaskToList(QString taskName)
{
	QPointer <Task> p = taskPointer(taskName);
	if(p)
	{
		taskList.append(p);
		operateLog(QSL("控制面板：添加自动任务\"%1\"，当前任务列表中有%2个任务").arg(taskName).arg(taskList.count()), LogPane::info);
	}
	else
	{
		operateLog(QSL("控制面板：自动任务\"%1\"添加失败").arg(taskName), LogPane::info);
	}
}

QPointer<Task> ControlPane::taskPointer(QString taskName)
{
	QPointer<Task> p = nullptr;
	try
	{
		if(Task_Cruise::taskName == taskName)
		{
			p = new Task_Cruise(100);
		}else if(Task_Light::taskName == taskName)
		{
			p = new Task_Light(100);
		}else if(Task_Color::taskName == taskName)
		{
			p = new Task_Color(100);
		}else if(Task_Test::taskName == taskName)
		{
			p = new Task_Test(10000);
		}
	}
	catch (const std::bad_alloc& e)
	{
		Q_UNUSED(e);
		return nullptr;
	}
	return p;
}

void ControlPane::keyReaction(unsigned short k)
{
	if (k & 0x1)//W
	{
		ui->sldGo->setValue(3);
	}
	else if (k & 0x2)//S
	{
		ui->sldGo->setValue(-3);
	}
	else
	{
		ui->sldGo->setValue(0);
	}

	if (k & 0x4)//A
	{
		ui->sldTurn->setValue(-3);
	}
	else if (k & 0x8)//D
	{
		ui->sldTurn->setValue(3);
	}
	else
	{
		ui->sldTurn->setValue(0);
	}

	if (k & 0x10)//Q
	{
		ui->sldOffet->setValue(-3);
	}
	else if (k & 0x20)//E
	{
		ui->sldOffet->setValue(3);
	}
	else
	{
		ui->sldOffet->setValue(0);
	}
}

void ControlPane::setDepthAutomation(bool b)
{
	emit autoDepthSwitch(b);
	if(b)
	{
		isAutomatic |= 0x1;
		emit automaticSwitch(isAutomatic);
	}
	else
	{
		isAutomatic &= ~0x1;
		emit automaticSwitch(isAutomatic);
	}
}

void ControlPane::setPitchAutomation(bool b)
{
	emit autoPitchSwitch(b);
	if(b)
	{
		isAutomatic |= 0x2;
		emit automaticSwitch(isAutomatic);
	}
	else
	{
		isAutomatic &= ~0x2;
		emit automaticSwitch(isAutomatic);
	}
}

void ControlPane::setHeadingAutomation(bool b)
{
	emit autoHeadingSwitch(b);
	if(b)
	{
		isAutomatic |= 0x4;
		emit automaticSwitch(isAutomatic);
	}
	else
	{
		isAutomatic &= ~0x4;
		emit automaticSwitch(isAutomatic);
	}
}

void ControlPane::setTaskAutomation(bool b)
{
	if(b)
	{
		isAutomatic |= 0x8;
		emit automaticSwitch(isAutomatic);
	}
	else
	{
		isAutomatic &= ~0x8;
		emit automaticSwitch(isAutomatic);
	}
}

void ControlPane::setAutoCtrlWidgetEnable(bool isEnable)
{
	if (isEnable)
	{
		ui->taskList->setEnabled(true);
		ui->btnPushIntoTaskStack->setEnabled(true);
		ui->btnDelCurChoosenItem->setEnabled(true);
		ui->btnAbandonTask->setEnabled(false);
		ui->btnAutoControl->setChecked(false);
		ui->ChoosenTaskList->setEnabled(true);
	}
	else
	{
		ui->taskList->setEnabled(false);
		ui->btnPushIntoTaskStack->setEnabled(false);
		ui->btnDelCurChoosenItem->setEnabled(false);
		ui->btnAbandonTask->setEnabled(true);
		ui->btnAutoControl->setChecked(true);
		ui->ChoosenTaskList->setEnabled(false);
	}
}

void ControlPane::taskConnections(QPointer<Task> t)
{
	//销毁线程
	connect(curTaskThread, &QThread::finished, t, &Task::deleteLater);
	connect(curTaskThread, &QThread::finished, curTaskThread, &QThread::deleteLater);

	//任务类向窗口发送（需要转发的）
	connect(t, &Task::LEDFlash, this, &ControlPane::LEDFlash);
	connect(t, &Task::operateLog, this, &ControlPane::operateLog);
	connect(t, &Task::videoImg, this, &ControlPane::videoImg);
	connect(t, &Task::taskGoSgl, this, &ControlPane::taskGoSgl);
	connect(t, &Task::taskTurnSgl, this, &ControlPane::taskTurnSgl);
	connect(t, &Task::taskOffsetSgl, this, &ControlPane::taskOffsetSgl);
	connect(t, &Task::taskDiveSgl, this, &ControlPane::taskDiveSgl);
	connect(t, &Task::autoDepthSwitch, this, &ControlPane::setDepthAutomation);
	connect(t, &Task::setAutoDepth, this, &ControlPane::setAutoDepth);
	connect(t, &Task::autoHeadingSwitch, this ,&ControlPane::setHeadingAutomation);
	connect(t, &Task::setAutoHeading, this, &ControlPane::setAutoHeading);
	connect(t, &Task::autoPitchSwitch, this, &ControlPane::setPitchAutomation);

	//任务向窗口发送（无需转发）
	connect(t, &Task::taskInfos, this, [this](QString s){ ui->txtTaskInfo->setText(s); });
	connect(t, &Task::taskEndSgl, this, &ControlPane::onTaskComplete);

	//窗口向任务发送
	connect(this, &ControlPane::depthSensor, t, &Task::depthSlot);
	connect(this, &ControlPane::postureSensor, t, &Task::postureSlot);
	connect(this, &ControlPane::startTheTask, t, &Task::taskSetUp);
}

bool ControlPane::creatTaskThread(QPointer<Task> t)
{	
	emit operateLog(QSL("任务流程：创建任务\"%1\"线程…").arg(t->getTaskName()), LogPane::info);
	if(!curTaskThread.isNull() && curTaskThread->isRunning())
	{
		emit operateLog(QSL("任务流程：当前仍有任务未结束"), LogPane::err);
		return false;
	}

	curTaskThread = new QThread;
	if (t.isNull())
	{
		emit operateLog(QSL("任务流程：creatTaskThread t指针为空"), LogPane::err);
		return false;
	}
	if( curTaskThread.isNull())
	{
		emit operateLog(QSL("任务流程：creatTaskThread curTaskThread创建失败"), LogPane::err);
		return false;

	}

	t->moveToThread(curTaskThread);
	taskConnections(t);//连接主线程与子线程的通信信号槽

	return true;
}

void ControlPane::startTaskThread()
{
	if(!curTaskThread.isNull())
	{
		curTaskThread->start();//启动
		emit depthSensor(curDepth);//更新一下深度和姿态
		emit postureSensor(curPos);
		emit startTheTask();	//向子线程发送启动信号
	}
}

void ControlPane::destoryCurTask()
{
	if(curTaskThread)
	{
		curTaskThread->quit();
		curTaskThread->wait();
		curTaskThread = nullptr;
		curTask = nullptr;
	}
}

void ControlPane::timeDispTimerUpdate()
{
	static long long int StartTime(0);
	StartTime++;
	QString str=QString("%1m - %2s").arg((int)(StartTime/60)).arg(StartTime-(int)(StartTime/60)*60);
	ui->txtElapsedTime->setText(str);
	ui->txtElapsedTime->adjustSize();

	QDateTime time = QDateTime::currentDateTime();
	QString sysTimeStr = time.toString(" hh:mm:ss");
	ui->txtTime->setText(sysTimeStr);
	ui->txtTime->adjustSize();

	sysTimeStr = time.toString("yyyy-MM-dd ");
	ui->txtDate->setText(sysTimeStr);
	ui->txtDate->adjustSize();
}

void ControlPane::onTaskComplete(Task::TaskEndStatus s)
{
	if(s & 0x8)//no more
	{
		emit operateLog(QSL("任务流程：任务流因故终止\n"), LogPane::warn);

		//终止当前任务
		destoryCurTask();
		taskList.removeFirst();
		ui->ChoosenTaskList->takeItem(0);

		setAutoCtrlWidgetEnable(true);
		setTaskAutomation(false);
		setDepthAutomation(false);
		setAutoDepth(0);
		setPitchAutomation(false);
		setHeadingAutomation(false);
		//
		LEDSetting s; s.changeTime = 1000; s.flashTimes =3 ; s.colorList<<QColor(255,255,255)<<QColor(0,0,0);
		emit LEDFlash(s);

		QTimer::singleShot(250, [this]{emit mannualGoSgl(0);});

		return;
	}

	setDepthAutomation(s & 0x2);
	setPitchAutomation(s & 0x2);
	setHeadingAutomation(s & 0x4);

	if(!(s & 0x1))//下一个任务
	{
		emit operateLog(QSL("任务流程：\"%1\"任务执行结束并执行下一个任务，保持深度：%2；保持航向：%3")\
					   .arg(curTask->getTaskName())
					   .arg((bool)(s & 0x2))
					   .arg((bool)(s & 0x4)), LogPane::info);

		//终止当前任务
		destoryCurTask();
		taskList.removeFirst();
		ui->ChoosenTaskList->takeItem(0);

		//列表非空则新建下一个任务
		if(taskList.isEmpty())
		{
			emit operateLog(QSL("任务流程：任务流因为没有任务而结束\n"), LogPane::info);
			setAutoCtrlWidgetEnable(true);
			setTaskAutomation(false);
			setDepthAutomation(false);
			setAutoDepth(0);
			setPitchAutomation(false);
			setHeadingAutomation(false);
			LEDSetting s; s.changeTime = 1000; s.flashTimes =3 ; s.colorList<<QColor(255,255,255)<<QColor(0,0,0);
			emit LEDFlash(s);
			QTimer::singleShot(250, [this]{emit mannualGoSgl(0);});

			return;
		}
		else
		{
			if(ui->btnAutoControl->isChecked())
			{
				curTask = taskList.first();
				if (creatTaskThread(curTask))
				{
					startTaskThread();
					emit operateLog(QSL("任务流程：创建成功"), LogPane::info);
					ui->ChoosenTaskList->setCurrentRow(0);
					LEDSetting s; s.changeTime = 200; s.flashTimes = 3 ; s.colorList<<QColor(255,255,255)<<QColor(0,0,0);
					emit LEDFlash(s);
				}
				else
				{
					emit operateLog(QSL("任务流程：创建失败，任务流程终止\n"), LogPane::err);
					setAutoCtrlWidgetEnable(true);
					setTaskAutomation(false);
					setDepthAutomation(false);
					setAutoDepth(0);
					setPitchAutomation(false);
					setHeadingAutomation(false);
					LEDSetting s; s.changeTime = 1000; s.flashTimes =3 ; s.colorList<<QColor(255,255,255)<<QColor(0,0,0);
					emit LEDFlash(s);

					QTimer::singleShot(250, [this]{emit mannualGoSgl(0);});
				}//(creatTaskThread(taskList.first())) else
			}
		}
	}
	else
	{
		emit operateLog(QSL("任务流程：任务执行结束并重新执行此任务"), LogPane::warn);
		destoryCurTask();//销毁当前正在执行的任务
		taskList.removeFirst();

		QPointer<Task> p= taskPointer(ui->ChoosenTaskList->item(0)->text());//重新创建一个相同的任务
		if(p)
		{
			taskList.prepend(p);//添加至任务列表等待执行
			if(ui->btnAutoControl->isChecked())
			{
				curTask = taskList.first();
				if (creatTaskThread(curTask))
				{
					startTaskThread();
					emit operateLog(QSL("任务流程：创建成功"), LogPane::info);
					LEDSetting s; s.changeTime = 100; s.flashTimes = 3 ; s.colorList<<QColor(255,255,255)<<QColor(0,0,0);
					emit LEDFlash(s);
					ui->ChoosenTaskList->setCurrentRow(0);
				}
				else
				{
					emit operateLog(QSL("任务流程：创建失败，任务流程终止\n"), LogPane::err);
					setAutoCtrlWidgetEnable(true);
					setTaskAutomation(false);
					setDepthAutomation(false);
					setAutoDepth(0);
					setPitchAutomation(false);
					setHeadingAutomation(false);
					LEDSetting s; s.changeTime = 1000; s.flashTimes =3 ; s.colorList<<QColor(255,255,255)<<QColor(0,0,0);
					emit LEDFlash(s);
				}//(creatTaskThread(taskList.first())) else
			}
		}
		else
		{
			emit operateLog(QSL("控制面板：重新创建任务\"%1\"失败").arg(ui->ChoosenTaskList->item(0)->text()), LogPane::err);
			ui->ChoosenTaskList->takeItem(0);
		}
	}
}

void ControlPane::on_btnStdCtrl_clicked()
{
	ui->stackedWidget->setCurrentIndex(0);
	ui->btnStdCtrl->setChecked(true);
	ui->btnAdvCtrl->setChecked(false);
	ui->btnTaskCtrl->setChecked(false);
	ui->btnParaSet->setChecked(false);
}

void ControlPane::on_btnAdvCtrl_clicked()
{
	ui->stackedWidget->setCurrentIndex(1);
	ui->btnStdCtrl->setChecked(false);
	ui->btnAdvCtrl->setChecked(true);
	ui->btnTaskCtrl->setChecked(false);
	ui->btnParaSet->setChecked(false);
}

void ControlPane::on_btnTaskCtrl_clicked()
{
	ui->stackedWidget->setCurrentIndex(2);
	ui->btnStdCtrl->setChecked(false);
	ui->btnAdvCtrl->setChecked(false);
	ui->btnTaskCtrl->setChecked(true);
	ui->btnParaSet->setChecked(false);
}

void ControlPane::on_btnParaSet_clicked()
{
	ui->stackedWidget->setCurrentIndex(3);
	ui->btnStdCtrl->setChecked(false);
	ui->btnAdvCtrl->setChecked(false);
	ui->btnTaskCtrl->setChecked(false);
	ui->btnParaSet->setChecked(true);
}

void ControlPane::on_btnDepthClosedLoop_clicked(bool checked)
{
	setDepthAutomation(checked);
	if(checked)
	{
		setPitchAutomation(true);
		emit setAutoDepth(ui->depthVal->value());//更新自动控制的目标深度
		emit operateLog(QSL("控制面板：开启深度闭环和俯仰闭环，目标%1cm").arg(ui->depthVal->value()), LogPane::info);
		ui->btnRefreshDepth->setEnabled(true);
		ui->depthVal->setStyleSheet("background-color: rgb(190, 250, 250);");
		ui->btnPitchClosedLoop->setChecked(true);
	}
	else
	{
		emit setAutoDepth(0);
		emit operateLog(QSL("控制面板：关闭深度闭环"), LogPane::info);
		ui->btnRefreshDepth->setEnabled(false);
		ui->depthVal->setStyleSheet("background-color: rgb(255, 220, 170);");
	}
}

void ControlPane::on_btnCourseClosedLoop_clicked(bool checked)
{
	setHeadingAutomation(checked);
	if(checked)
	{
		emit setAutoHeading(ui->courseVal->value());
		emit operateLog(QSL("控制面板：开启航向闭环，目标%1°").arg(ui->courseVal->value()), LogPane::info);
		ui->btnRefreshCourse->setEnabled(true);
		ui->courseVal->setStyleSheet("background-color: rgb(190, 250, 250);");
	}
	else
	{
		emit setAutoHeading(0);
		emit operateLog(QSL("控制面板：关闭航向闭环"), LogPane::info);
		ui->btnRefreshCourse->setEnabled(false);
		ui->courseVal->setStyleSheet("background-color: rgb(255, 220, 170);");
	}
}

void ControlPane::on_btnPitchClosedLoop_clicked(bool checked)
{
	setPitchAutomation(checked);
	if(checked)
		emit operateLog(QSL("控制面板：开启俯仰闭环"), LogPane::info);
	else
		emit operateLog(QSL("控制面板：关闭俯仰闭环"), LogPane::info);
}

void ControlPane::on_sldTurn_valueChanged(int value)
{
	//	qDebug()<<"on_sldTurn_valueChanged";
	ui->btnTurnZero->setText(QString("%1").arg(value));
	emit mannualTurnSgl(value * 30);
}

void ControlPane::on_btnTurnZero_clicked()
{
	//此处不应该发送指令，接下来还会调用on_sldTurn_valueChanged
	//	qDebug()<<"on_btnTurnZero_clicked";
	ui->sldTurn->setValue(0);
}

void ControlPane::on_btnTurnLeft_clicked()
{
	ui->sldTurn->setValue(ui->sldTurn->value()-1);
}

void ControlPane::on_btnTurnRight_clicked()
{
	ui->sldTurn->setValue(ui->sldTurn->value()+1);
}

void ControlPane::on_sldOffet_valueChanged(int value)
{
	ui->btnOffsetZero->setText(QString("%1").arg(value));
	emit mannualOffsetSgl(value * 30);
}

void ControlPane::on_btnOffsetZero_clicked()
{
	ui->sldOffet->setValue(0);
}

void ControlPane::on_btnOffsetLeft_clicked()
{
	ui->sldOffet->setValue(ui->sldOffet->value()-1);
}

void ControlPane::on_btnOffertRight_clicked()
{
	ui->sldOffet->setValue(ui->sldOffet->value()+1);
}

void ControlPane::on_sldGo_valueChanged(int value)
{
	ui->btnGoZero->setText(QString("%1").arg(value));
	emit mannualGoSgl(value * 30);
}

void ControlPane::on_btnGoZero_clicked()
{
	ui->sldGo->setValue(0);
}

void ControlPane::on_btnForward_clicked()
{
	ui->sldGo->setValue(ui->sldGo->value()+1);
}

void ControlPane::on_btnBackward_clicked()
{
	ui->sldGo->setValue(ui->sldGo->value()-1);
}

void ControlPane::on_btnRefreshDepth_clicked()
{
	assert(ui->btnDepthClosedLoop->isEnabled());
	emit operateLog(QSL("控制面板：设定深度闭环目标%1cm").arg(ui->depthVal->value()), LogPane::info);
	emit setAutoDepth(ui->depthVal->value());
	ui->depthVal->setStyleSheet("background-color: rgb(190, 250, 250);");
}

void ControlPane::on_btnRefreshCourse_clicked()
{
	assert(ui->btnCourseClosedLoop->isEnabled());
	emit operateLog(QSL("控制面板：设定航向闭环目标%1°").arg(ui->courseVal->value()), LogPane::info);
	emit setAutoHeading(ui->courseVal->value());
	ui->courseVal->setStyleSheet("background-color: rgb(190, 250, 250);");
}

/*================================================================*/
/*********************下面为键盘控制指令*******************************/
void ControlPane::keyPressEvent(QKeyEvent *e )
{
	bool isWSADQE = true;
	switch (e->key()) {
	case Qt::Key_W:
		PressedKeys |= 0x1; break;
	case Qt::Key_S:
		PressedKeys |= 0x2; break;
	case Qt::Key_A:
		PressedKeys |= 0x4; break;
	case Qt::Key_D:
		PressedKeys |= 0x8; break;
	case Qt::Key_Q:
		PressedKeys |= 0x10; break;
	case Qt::Key_E:
		PressedKeys |= 0x20; break;
	default:
		isWSADQE = false; break;
	}
	if(isWSADQE && !e->isAutoRepeat())
	{
		keyReaction(PressedKeys);
	}
}

void ControlPane::keyReleaseEvent(QKeyEvent *e)
{
	bool isWSADQE = true;
	switch (e->key()) {
	case Qt::Key_W:
		PressedKeys &= ~0x1; break;
	case Qt::Key_S:
		PressedKeys &= ~0x2; break;
	case Qt::Key_A:
		PressedKeys &= ~0x4; break;
	case Qt::Key_D:
		PressedKeys &= ~0x8; break;
	case Qt::Key_Q:
		PressedKeys &= ~0x10; break;
	case Qt::Key_E:
		PressedKeys &= ~0x20; break;
	default:
		isWSADQE = false; break;
	}
	if(isWSADQE && !e->isAutoRepeat())
		keyReaction(PressedKeys);
}

void ControlPane::on_btnSet0Depth_clicked()
{
	ui->depthVal->setValue(0);
}

void ControlPane::on_btnUseCurYaw_clicked()
{

	ui->courseVal->setValue(curPos.yaw);
}

void ControlPane::on_depthVal_valueChanged(double arg1)
{
	Q_UNUSED(arg1);
	ui->depthVal->setStyleSheet("background-color: rgb(255, 220, 170);");
}

void ControlPane::on_courseVal_valueChanged(double arg1)
{
	Q_UNUSED(arg1);
	ui->courseVal->setStyleSheet("background-color: rgb(255, 220, 170);");

}

void ControlPane::on_btnThrsSwitch_clicked(bool checked)
{
	emit propSwitch(checked);
	if(checked)
	{
		emit operateLog(QSL("控制面板：推进器电源已打开"), LogPane::info);
		LEDSetting s;
		s.changeTime = 250;
		s.colorList << QColor(0,255,0) << QColor(0,0,0);
		s.flashTimes = 4;
		emit LEDFlash(s);
	}
	else
	{
		emit operateLog(QSL("控制面板：推进器电源已关闭"), LogPane::info);
		LEDSetting s;
		s.changeTime = 500;
		s.colorList << QColor(0,255,0) << QColor(0,0,0);
		s.flashTimes = 2;
		emit LEDFlash(s);
	}
	emit mannualGoSgl(0);//发送初始化指令
}

void ControlPane::on_btnCamSwitch_clicked(bool checked)
{
	emit camSwitch(checked);
	if(checked)
	{
		emit operateLog(QSL("控制面板：相机电源已打开，正在初始化相机..."), LogPane::info);
		LEDSetting s;
		s.changeTime = 250;
		s.colorList << QColor(0,0,255) << QColor(0,0,0);
		s.flashTimes = 4;
		emit LEDFlash(s);
		QTimer::singleShot(5000, [this]{QString camStatus = Vision::initFCam(); camStatus.isEmpty() ? emit operateLog(QSL("相机：前视成功初始化"), LogPane::info) : emit operateLog(QSL("摄像头：前视初始化失败，原因是%1").arg(camStatus), LogPane::err);});
	}
	else
	{
		emit operateLog(QSL("控制面板：相机电源已关闭"), LogPane::info);
		Vision::destoryDCam();
		Vision::destoryFCam();
		MVTerminateLib();
		LEDSetting s;
		s.changeTime = 500;
		s.colorList << QColor(0,0,255) << QColor(0,0,0);
		s.flashTimes = 2;
		emit LEDFlash(s);
	}
}

void ControlPane::on_btnSendPropOrder_clicked()
{
	QString order = QString(("#1P%1#2P%2#3P%3#4P%4#5P%5#6P%6T200"))
							.arg(ui->spnProp1->value())
							.arg(ui->spnProp2->value())
							.arg(ui->spnProp3->value())
							.arg(ui->spnProp4->value())
							.arg(ui->spnProp5->value())
							.arg(ui->spnProp6->value());
	emit operateLog(QSL("控制面板：直接发送推进器指令") + order, LogPane::info);
	emit propOrder(order + "\r\n");
}

void ControlPane::on_btnDefaultProp_clicked()
{
	ui->spnProp1->setValue(1500);
	ui->spnProp2->setValue(1500);
	ui->spnProp3->setValue(1500);
	ui->spnProp4->setValue(1500);
	ui->spnProp5->setValue(1500);
	ui->spnProp6->setValue(1500);
}

void ControlPane::on_btnPushIntoTaskStack_clicked()
{
	if(ui->taskList->count())
	{
		unsigned listCnt = ui->ChoosenTaskList->count();
		QString taskName = ui->taskList->currentItem()->text();
		ui->ChoosenTaskList->insertItem(listCnt, taskName);
		ui->ChoosenTaskList->setCurrentRow(0);
		addTaskToList(taskName);
	}
}

void ControlPane::on_btnDelCurChoosenItem_clicked()
{
	if (!taskList.empty())
	{
		int index = ui->ChoosenTaskList->currentRow();
		emit operateLog(QSL("控制面板：删除第%1个任务\"%2\"").arg(index+1).arg(taskList.at(index)->getTaskName()), LogPane::info);
//		qDebug()<<"before delete, list:"<<taskList;
		delete taskList.at(index);
		taskList.removeAt(index);
//		qDebug()<<"after delete, list:"<<taskList;
		ui->ChoosenTaskList->takeItem(index);
	}
}

void ControlPane::on_btnAutoControl_clicked(bool checked)
{
	setAutoCtrlWidgetEnable(!checked);

	if (checked)	//开始任务：创建第一个任务
	{
		on_btnDepthClosedLoop_clicked(false);
		on_btnCourseClosedLoop_clicked(false);
		on_btnPitchClosedLoop_clicked(false);
		emit operateLog(QSL("控制面板：开始流程，当前任务队列中有%1个任务").arg(taskList.size()), LogPane::info);
		if (taskList.empty())
		{
			emit operateLog(QSL("任务流程：任务流因为没有任务而结束\n"), LogPane::warn);
			setAutoCtrlWidgetEnable(true);
			setTaskAutomation(false);
			LEDSetting s; s.changeTime = 1000; s.flashTimes =3 ; s.colorList<<QColor(255,255,255)<<QColor(0,0,0);
			emit LEDFlash(s);
		}
		else
		{
			curTask = taskList.first();
			if (creatTaskThread(curTask))
			{

				if(QMessageBox::Ok == QMessageBox::warning(this, QSL("自动任务即将开始"), QSL("点击OK 10秒后开始执行自动任务，请手动断开远程桌面"), QMessageBox::Ok, QMessageBox::Cancel))
				{
					LEDSetting s; s.changeTime = 500; s.flashTimes = 0 ; s.colorList<<QColor(0,255,255)<<QColor(0,0,0);
					emit LEDFlash(s);
					QTimer::singleShot(7000, [this]{LEDSetting s; s.changeTime = 100; s.flashTimes = 0 ; s.colorList<<QColor(0,255,255)<<QColor(0,0,0); emit LEDFlash(s);});
					QTimer::singleShot(10000, [this](){startTaskThread(); setTaskAutomation(true);});

					emit operateLog(QSL("任务流程：创建成功，请断开远程连接，任务将于10s后开始自动执行"), LogPane::info);
					ui->ChoosenTaskList->setCurrentRow(0);
				}
				else
				{
					setDepthAutomation(false);
					setAutoDepth(0);
					setPitchAutomation(false);
					setHeadingAutomation(false);
					setTaskAutomation(false);
					destoryCurTask();
					setAutoCtrlWidgetEnable(true);
				}
			}
			else
			{
				emit operateLog(QSL("任务流程：创建失败，任务流程终止\n"), LogPane::err);
				setAutoCtrlWidgetEnable(true);
				setTaskAutomation(false);
				LEDSetting s; s.changeTime = 1000; s.flashTimes =3 ; s.colorList<<QColor(255,255,255)<<QColor(0,0,0);
				emit LEDFlash(s);
			}//(creatTaskThread(taskList.first())) else
		}//(taskList.empty()) else
	}
	else			//停止任务：手动停止当前正在执行的任务
	{
		setDepthAutomation(false);
		setAutoDepth(0);
		setPitchAutomation(false);
		setHeadingAutomation(false);
		setTaskAutomation(false);
		destoryCurTask();//销毁当前正在执行的任务
		taskList.removeFirst();

		QPointer<Task> p= taskPointer(ui->ChoosenTaskList->item(0)->text());//重新创建一个相同的任务
		if(p)
			taskList.prepend(p);//添加至任务列表等待执行
		else
		{
			emit operateLog(QSL("控制面板：重新创建任务\"%1\"失败").arg(ui->ChoosenTaskList->item(0)->text()), LogPane::err);
			ui->ChoosenTaskList->takeItem(0);
		}
		emit operateLog(QSL("控制面板：手动终止流程\n"), LogPane::warn);
		LEDSetting s; s.changeTime = 1000; s.flashTimes =3 ; s.colorList<<QColor(255,255,255)<<QColor(0,0,0);
		emit LEDFlash(s);
	}
}

void ControlPane::on_ChoosenTaskList_doubleClicked(const QModelIndex &index)
{
//	taskList.at(index.row())->setTaskPara();
	Q_UNUSED(index);
	qDebug()<<QSL("有空使用xml实现");
}


void ControlPane::on_btnAbandonTask_clicked()
{
	emit operateLog(QSL("控制面板：放弃当前任务\"%1\"").arg(curTask->getTaskName()), LogPane::info);
	destoryCurTask();//销毁当前正在执行的任务
	taskList.removeFirst();
	ui->ChoosenTaskList->takeItem(0);
	setDepthAutomation(false);
	setAutoDepth(0);
	setPitchAutomation(false);
	setHeadingAutomation(false);

	if(taskList.isEmpty())
	{
		emit operateLog(QSL("任务流程：任务流因为没有任务而结束\n"), LogPane::info);
		setAutoCtrlWidgetEnable(true);
		setTaskAutomation(false);
		LEDSetting s; s.changeTime = 1000; s.flashTimes =3 ; s.colorList<<QColor(255,255,255)<<QColor(0,0,0);
		emit LEDFlash(s);
	}
	else
	{
		if (ui->btnAutoControl->isChecked())
		{
			curTask = taskList.first();
			if (creatTaskThread(curTask))
			{
				startTaskThread();
				emit operateLog(QSL("任务流程：创建成功"), LogPane::info);
				ui->ChoosenTaskList->setCurrentRow(0);
			}
			else
			{
				emit operateLog(QSL("任务流程：创建失败，任务流程终止\n"), LogPane::err);
				setAutoCtrlWidgetEnable(true);
				setTaskAutomation(false);
				LEDSetting s; s.changeTime = 1000; s.flashTimes =3 ; s.colorList<<QColor(255,255,255)<<QColor(0,0,0);
				emit LEDFlash(s);
			}//(creatTaskThread(taskList.first())) else
		}
	}
}
