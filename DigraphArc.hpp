#pragma once
#include <string>
using namespace std;
//图的弧类

class ArcNodeInfo
{
public:
	ArcNodeInfo(int weight_, bool isWait) :weight(weight_), isWait100ms(isWait) {}
	~ArcNodeInfo() {}

	int getExecValue() const { return weight; }
	bool getWaitStatus() const { return isWait100ms; }
	void setExecValue(int weight_) { weight = weight_; }
	void setWaitStatus(bool isWait) { isWait100ms = isWait; }
private:
	int weight;
	bool isWait100ms;
};

class DigraphArc
{
public:
	DigraphArc(int a, int weight_, bool isWait, string name) :adjvex(a), info(weight_, isWait), arcName(name) {}
	DigraphArc() :adjvex(0), info(0,false) {}
	~DigraphArc() {}

	int getHeadIndex() const { return adjvex; }
	ArcNodeInfo getArcInfo() const { return info; }
	string getArcName() const { return arcName; }

private:
	int adjvex;//这条弧的头（终止点）在邻接表中的索引
	ArcNodeInfo info;
	string arcName;
};
