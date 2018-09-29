#pragma once
#include "DigraphArc.hpp"
#include <assert.h>
#include <vector>
#include <string>
using namespace std;
#include <QString>

template <typename T>
class Digraph;

//图的结点类

template <typename T>
class DigraphNode
{
public:
	DigraphNode(T d, string nodeName_) :data(d), nodeName(nodeName_){ }
	~DigraphNode(){ }

	void setData(T d) { data = d; }
	T getData() { return data; }
	string getName() { return nodeName; }

	//获取以该节点为尾的第index条弧
	DigraphArc getArc(int index)
	{	
		QString debug;
		foreach (DigraphArc a, arcs) {
			debug += ((QString)a.getArcName().data() + ",");
		}
		qDebug()<<debug;
		if (index >= arcs.size())
			qDebug() << "index = " << index << "but arcs.size()=" << arcs.size();
		assert(index < arcs.size()); 
		return arcs.at(index);
	}

	//添加一条以该结点为头的新弧
	void addArc(DigraphArc a)
	{
		arcs.push_back(a);
	}

	unsigned getArcCnt()
	{
		return arcs.size();
	}

private:
	T data;					//顶点存储的数据
	string nodeName;		//结点名称
	vector<DigraphArc> arcs;	//以该结点为尾的所有弧组成的向量
	bool isVisited = false;	//遍历算法中是否访问过的标志

	//声明个友元
	friend Digraph<T>;
};

