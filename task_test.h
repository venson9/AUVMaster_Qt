#ifndef TASK_DIGRAPHCRUISE_H
#define TASK_DIGRAPHCRUISE_H

#include "task.hpp"
#include "Digraph.hpp"


//测试任务类
class Task_Test : public Task
{
	//=====================================新建任务时，以下内容按原样复制==================================================
public:
	Task_Test(int period, QObject *parent = 0);
	virtual ~Task_Test();
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
	Digraph<int (Task_Test:: *)(const void *)> taskTgt;//邻接表表示的有向图，模板类型是以 void * 为参数，返回 int 的 task_test 的成员函数的 函数指针
	unsigned int tgtDigraphIndex = 0;//当前任务目标在任务目标有向图邻接表中的索引
	//-------------------------------新建任务时，以上内容按原样复制，注意修改构造函数名和析构函数名------------------------------------

	int ctrlTest0(const void *p);
	int ctrlTest1(const void *p);
	int ctrlTest2(const void *p);
	int ctrlTest3(const void *p);
	int ctrlTest4(const void *p);
};

#endif // TASK_DIGRAPHCRUISE_H
