#ifndef TASK_COLOR_H
#define TASK_COLOR_H

#include "task.hpp"
#include "normalPID.h"
#include "Digraph.hpp"
#include "MVGigE.h"
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

//寻光任务类
/* 灯光：
 * 等待定深、等待寻找目标：0,0,255 - 0,255,0 0.5秒，持续
 * 找到目标：0,0,255 - 0,255,0 0.25秒，持续
*/
class Task_Light : public Task
{
	//=====================================新建任务时，以下内容按原样复制==================================================
public:
	Task_Light(int period, QObject *parent = 0);
	virtual ~Task_Light();
	static QString taskName;

	const unsigned int securityTimerConst = 300;//安保定时器定时时间 in second

public slots:
	virtual void setTaskPara() Q_DECL_OVERRIDE;
	virtual void taskSetUp() Q_DECL_OVERRIDE;//虚开始线程函数
	virtual QString getTaskName() Q_DECL_OVERRIDE { return taskName; }

private slots:
	void ctrlTimerUpdate();

private:
	inline QString infoStr();
	QString extraInfo;
	QString taskStatus;
	QTime elapseTime;
	Digraph<int (Task_Light:: *)(const void *)> taskTgt;//邻接表表示的有向图，模板类型是以 void * 为参数，返回 int 的 Task_Light 的成员函数的 函数指针
	unsigned int tgtDigraphIndex = 0;//当前任务目标在任务目标有向图邻接表中的索引
	//-------------------------------新建任务时，以上内容按原样复制，注意修改构造函数名和析构函数名------------------------------------

	bool isCamThreadRun = false;
	volatile bool isDataUpdated = false;

	//任务参数=====================================================
	const int findTargetTurnLvConst = 50;

	//控制目标函数=================================================
	int ctrlSetSearchDepth(const void *);//设定搜索深度
	double searchDepth = 20;

	int ctrlWaitForDepthOK(const void *);//等待深度ok
	unsigned int depthOKTime = 0;

	int ctrlFindTarget(const void *);//设定寻找目标动作
	double startYaw = 0;

	int ctrlWaitForFindTarget(const void *);//等待找到目标
	unsigned char roundDetect = 0;

	int ctrlChaseTarget(const void *);
	NormalPID GoCtrl;//前进控制PID
	NormalPID XAdjustCtrl;//横向控制PID
	unsigned int shockTime = 0;
	unsigned int depthCtrlAreaTime = 0;
	bool isCheasingLEDSet = false;
	bool isShockingLEDSet = false;
	bool isOutDepthAreaOkDepthEmitted = false;//深度持续控制区外，画面中满足条件的深度已发送的标志

	int ctrlEnd(const void *);

	//与图像线程共享的变量=========================================
	volatile bool isLightInSight = false;
	volatile bool isWholeInSight = false;
	volatile double posX = 0;
	volatile double posY = 0;
	volatile double distance = 0;
	volatile double deltaDepth = 0;

private slots:
	//视觉处理函数
	void camIdentifyTarget(cv::Mat src);
	double SL[5];
	unsigned int medianCnt = 0;
};

#endif // TASK_LIGHT_H
