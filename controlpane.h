#ifndef CONTROLPANE_H
#define CONTROLPANE_H

#include "logpane.h"
#include "structs.h"
#include "task.hpp"
#include <QWidget>
#include <QTimer>
#include <QKeyEvent>
#include <QPointer>
#include <QThread>

/* 添加任务的方法：
 * 编写好任务类
 * 在controlpane.cpp中include相应的类头文件
 * 在ControlPane构造函数中为任务列表添加任务类的taskName静态成员变量
 * 在controlpane.cpp中的taskPointer函数中添加if分支：if(Task_Test::taskName == taskName){ .. }
 * 以上。
*/

namespace Ui {
class ControlPane;
}

class ControlPane : public QWidget
{
	Q_OBJECT

public:
	explicit ControlPane(QWidget *parent = 0);
	~ControlPane();

public slots:
	void setLEDColor(QColor c);												//设定界面上的灯色
	void setPosture(PostureData p);											//设定显示的姿态信息
	void setDepth(double d);												//设定深度值
	void setThrusterDisp(int t1, int t2, int t3, int t4, int t5, int t6);	//设定推进器显示值
	void waterLeakWarnSlot();												//漏液处理
	void setRecvMsg(QString name, QString msg);								//显示接收消息
	void setSendMsg(QString name, QString msg);								//显示发送消息

signals:
	void operateLog(QString, LogPane::WarnLevel);							//操作日志
	void LEDFlash(LEDSetting);												//LED闪烁设定
	void mannualGoSgl(int Lv);
	void mannualTurnSgl(int Lv);
	void mannualOffsetSgl(int Lv);
	void taskGoSgl(int lv);
	void taskTurnSgl(int lv);
	void taskOffsetSgl(int lv);
	void taskDiveSgl(int lv);
	void autoPitchSwitch(bool);
	void autoDepthSwitch(bool);
	void setAutoDepth(double);
	void autoHeadingSwitch(bool);
	void setAutoHeading(double);
	void propSwitch(bool);
	void camSwitch(bool);
	void automaticSwitch(unsigned char);
	void propOrder(QString);
	void startTheTask();
	void videoImg(cv::Mat);
	void depthSensor(double);
	void postureSensor(PostureData);

private:
	Ui::ControlPane *ui;

	QTimer * timeDispTimer;
	unsigned short PressedKeys = 0;
	unsigned char isAutomatic = 0;//bit1:定深 //bit2:定俯仰 //bit3:定航 //bit4:任务
	PostureData curPos;
	double curDepth = 0;
	QList<QPointer<Task>> taskList;
	QPointer<Task> curTask;
	QPointer<QThread> curTaskThread;

	void keyReaction(unsigned short k);

	void setDepthAutomation(bool b);
	void setPitchAutomation(bool b);
	void setHeadingAutomation(bool b);
	void setTaskAutomation(bool b);

	void addTaskToList(QString taskName);
	QPointer<Task> taskPointer(QString taskName);
	void setAutoCtrlWidgetEnable(bool isEnable);
	void taskConnections(QPointer<Task> t);
	bool creatTaskThread(QPointer<Task> t);//创建任务线程函数
	void startTaskThread();
	void destoryCurTask();

private slots:
	void timeDispTimerUpdate();
	void onTaskComplete(Task::TaskEndStatus s);

	void on_btnStdCtrl_clicked();
	void on_btnAdvCtrl_clicked();
	void on_btnTaskCtrl_clicked();
	void on_btnParaSet_clicked();
	void on_btnRefreshDepth_clicked();
	void on_btnRefreshCourse_clicked();
	void on_btnDepthClosedLoop_clicked(bool checked);
	void on_btnCourseClosedLoop_clicked(bool checked);
	void on_btnPitchClosedLoop_clicked(bool checked);
	void on_sldTurn_valueChanged(int value);
	void on_btnTurnZero_clicked();
	void on_btnTurnLeft_clicked();
	void on_btnTurnRight_clicked();
	void on_sldOffet_valueChanged(int value);
	void on_btnOffsetZero_clicked();
	void on_btnOffsetLeft_clicked();
	void on_btnOffertRight_clicked();
	void on_sldGo_valueChanged(int value);
	void on_btnGoZero_clicked();
	void on_btnForward_clicked();
	void on_btnBackward_clicked();
	void on_btnSet0Depth_clicked();
	void on_btnUseCurYaw_clicked();
	void on_depthVal_valueChanged(double arg1);
	void on_courseVal_valueChanged(double arg1);
	void on_btnThrsSwitch_clicked(bool checked);
	void on_btnCamSwitch_clicked(bool checked);
	void on_btnSendPropOrder_clicked();
	void on_btnDefaultProp_clicked();
	void on_btnPushIntoTaskStack_clicked();
	void on_btnDelCurChoosenItem_clicked();
	void on_btnAutoControl_clicked(bool checked);
	void on_ChoosenTaskList_doubleClicked(const QModelIndex &index);
	void on_btnAbandonTask_clicked();

protected:
	void keyPressEvent(QKeyEvent * e);
	void keyReleaseEvent(QKeyEvent *e);

};

#endif // CONTROLPANE_H
