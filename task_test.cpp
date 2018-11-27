#include "task_test.h"

//=======================================新建任务时，以下函数按原样复制，注意修改类名======================================
QString Task_Test::taskName = QSL("有向图的流程实现测试");

Task_Test::Task_Test(int period, QObject *parent) :Task(period, parent)
{

}

Task_Test::~Task_Test()
{
	if(timer)
		delete timer;
}

//设置任务参数
void Task_Test::setTaskPara()
{

}

//任务从此函数启动
void Task_Test::taskSetUp()
{
	//set control timer
	timer = new QTimer(this);
	if(!timer)
	{
		emit operateLog(QSL("%1：控制定时器创建失败").arg(taskName), LogPane::err);
		emit taskEndSgl(Task::noMore);
		return;
	}
	connect( timer, &QTimer::timeout, this, &Task_Test::ctrlTimerUpdate );

	//elapsed task time start
	elapseTime.start();

	//set security timer, when timed up, task flow will continue to run next task
	QTimer::singleShot(securityTimerConst*1000, this, \
					   [this]{ emit operateLog((QSL("%1：安保定时时间到")).arg(taskName), LogPane::warn); emit taskEndSgl(Task::nextOne);});

	//test task unique set up code
	taskTgt.addNode(&Task_Test::ctrlTest0, "test0");//向图中添加结点
	taskTgt.addNode(&Task_Test::ctrlTest1, "test1");
	taskTgt.addNode(&Task_Test::ctrlTest2, "test2");
	taskTgt.addNode(&Task_Test::ctrlTest3, "test3");
	taskTgt.addNode(&Task_Test::ctrlTest4, "test4");
	taskTgt.addArc("test0", "test1", "0to1", 0, true);//1 - 2: 0
	taskTgt.addArc("test0", "test4", "0to4", 0, true);//1 - 5: 1
	taskTgt.addArc("test1", "test3", "2to4", 0, true);//2 - 4: 0
	taskTgt.addArc("test1", "test2", "1to2", 0, true);//2 - 3: 1
	taskTgt.addArc("test2", "test0", "2to0", 0, true);
	taskTgt.addArc("test3", "test0", "3to0", 0, true);
	taskTgt.addArc("test4", "test1", "4to1", 0, true);
	//执行第一个任务目标函数
	bool isWait = true;
	do
	{
		qDebug()<<"current index = "<<tgtDigraphIndex;
		DigraphNode<int (Task_Test:: *)(const void *)> node = taskTgt.getNode(tgtDigraphIndex);		//当前执行结点
		int returnVal = (this->*(node.getData()))(nullptr);			//执行这个结点的函数指针，返回值为下一步执行的弧的索引（小于0再次执行自己）
		qDebug() << "get arc index:" << returnVal;

		if(returnVal >= 0)
		{
			DigraphArc arc= node.getArc(returnVal);						//当前执行结点函数返回值对应的那条弧
			isWait = arc.getArcInfo().getWaitStatus();					//这条弧的执行是否需要等待（即需要定时器运行）
			tgtDigraphIndex = arc.getHeadIndex();						//下一个应当执行的有向图结点在邻接表中的索引
		}
		else
		{
			isWait = true;
		}
		qDebug() << "index of next node is" << tgtDigraphIndex;
	}
	while(!isWait);
	qDebug()<<endl;

	emit taskInfos(infoStr());//send task run-time info to GUI
	timer->start(ctrlPeriod);
}

//任务控制定时器更新
void Task_Test::ctrlTimerUpdate()
{
	bool isWait = true;
	do
	{
		qDebug()<<"current index = "<<tgtDigraphIndex;
		DigraphNode<int (Task_Test:: *)(const void *)> node = taskTgt.getNode(tgtDigraphIndex);		//当前执行结点
		int returnVal = (this->*(node.getData()))(nullptr);			//执行这个结点的函数指针，返回值为下一步执行的弧的索引（小于0再次执行自己）
		qDebug() << "get arc index:" << returnVal;

		if(returnVal >= 0)
		{
			DigraphArc arc= node.getArc(returnVal);						//当前执行结点函数返回值对应的那条弧
			isWait = arc.getArcInfo().getWaitStatus();					//这条弧的执行是否需要等待（即需要定时器运行）
			tgtDigraphIndex = arc.getHeadIndex();						//下一个应当执行的有向图结点在邻接表中的索引
		}
		else
		{
			isWait = true;
		}
		qDebug() << "head of this arc is " << tgtDigraphIndex;
	}
	while(!isWait);
	qDebug()<<endl;

	emit taskInfos(infoStr());//send task run-time info to GUI
}

//信息字符串
QString Task_Test::infoStr()
{
	return	QSL("任务时间：%1s\n任务状态：%2\n%3\n").arg((double)elapseTime.elapsed() / 1000).arg(taskStatus).arg(extraInfo);
}
//=======================================新建任务时，以上函数按原样复制======================================

int Task_Test::ctrlTest0(const void *p)
{
	taskStatus = QSL(__FUNCTION__);
	return 1;
}

int Task_Test::ctrlTest1(const void *p)
{
	taskStatus = QSL(__FUNCTION__);
	return 1;
}

int Task_Test::ctrlTest2(const void *p)
{
	taskStatus = QSL(__FUNCTION__);
	return -1;
}

int Task_Test::ctrlTest3(const void *p)
{
	taskStatus = QSL(__FUNCTION__);
	return 0;
}

int Task_Test::ctrlTest4(const void *p)
{
	taskStatus = QSL(__FUNCTION__);
	return 0;
}


