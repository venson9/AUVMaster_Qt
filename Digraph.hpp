#pragma once
#include "DigraphNode.hpp"
#include <vector>
#include <string>
using std::vector;

//有向图类

template <typename T>
class Digraph
{
public:
	Digraph()
	{
	}
	~Digraph()
	{
	}

	vector<DigraphNode<T>> nodes;//顶点向量

	//添加结点
	bool addNode(DigraphNode<T> n);
	bool addNode(T t, string name);

	//添加弧
	void addArc(unsigned int index, DigraphArc a);
	void addArc(unsigned int startIndex, unsigned int endIndex, string arcName,int weight, bool isWait);

	//根据名称添加弧
	bool addArc(string startName, DigraphArc a);
	bool addArc(string startName, string endName, string arcName, int weight, bool isWait);

	//访问结点
	DigraphNode<T> getNode(int index) { return nodes.at(index); }

	//深度优先搜索算法遍历图
	void depthFirstSearch(unsigned int index)
	{
		initSearchStatus();
		DFS(index);
	}

private:
	void initSearchStatus()
	{
		for each (DigraphNode<T> v in nodes)
		{
			v.isVisited = false;
		}
	}

	void DFS(unsigned int index)
	{
		assert(index < nodes.size());
		nodes[index].isVisited = true;		//置index索引的结点遍历标志为true
		DigraphNode<T> v = getNode(index);		//访问索引为index的结点v
		cout << "get " << index << " \"" << nodes.at(index).nodeName << '\"' << endl;
		for each (DigraphArc a in v.arcs)		//对v尚未访问的邻接顶点调用dfs
		{
			if (nodes[a.getHeadIndex()].isVisited == false)
			{
				cout<<"get arc "<<"\""<<a.getArcName()<<"\""<<endl;
				DFS(a.getHeadIndex());
			}
		}

	}
};






//添加结点
template<typename T>
inline bool Digraph<T>::addNode(DigraphNode<T> n)
{
	//检查是否有重名项
	for each (DigraphNode<T> v in nodes)
	{
		if (v.getName() == n.getName())
			return false;
	}
	nodes.push_back(n);
	return true;
}

//添加结点
template<typename T>
inline bool Digraph<T>::addNode(T t, string name)
{
	//检查是否有重名项
	for each (DigraphNode<T> v in nodes)
	{
		if (v.getName() == name)
			return false;
	}
	nodes.push_back(DigraphNode<T>(t, name));
	return true;
}

//添加弧
template<typename T>
inline void Digraph<T>::addArc(unsigned int index, DigraphArc a)
{
	assert(index < nodes.size())
		assert(a.adjvex < nodes.size());
	nodes.at(index).addArc(a);
}

//添加弧
template<typename T>
inline void Digraph<T>::addArc(unsigned int startIndex, unsigned int endIndex, string arcName, int weight, bool isWait)
{
	assert(startIndex < nodes.size());//弧尾（起始点）索引不能越过向量长度
	assert(endIndex < nodes.size());//弧头（终止点）索引不能越过向量长度
									//assert(endIndex < nodes[startIndex].arcs.size());//弧头（终止点）索引不能越过
	//检查是否有重名项
	for each (DigraphArc a in nodes.at(startIndex).arcs)
	{
		if(a.getArcName() == arcName)
			return;
	}
	nodes.at(startIndex).addArc(DigraphArc(endIndex, weight, isWait, arcName));
}

//根据名称添加弧
template<typename T>
inline bool Digraph<T>::addArc(string startName, DigraphArc a)
{
	unsigned index = 0;
	//寻找输入名称对应结点的索引
	for each (DigraphNode<T> node in nodes)
	{
		if (node.getName() == startName)
		{
			addArc(index, a);
			break;
		}
		index++;
	}
	return true;
}

//根据名称添加弧
template<typename T>
inline bool Digraph<T>::addArc(string startName, string endName, string arcName, int weight, bool isWait)
{
	int startIndex = -1, endIndex = -1;
	unsigned index = 0;
	//寻找输入名称对应起始点的索引
	for each (DigraphNode<T> node in nodes)
	{
		if (node.getName() != startName)
			index++;
		else
		{
			startIndex = index;
			break;
		}
	}
	//判断是否找到了这个点
	if (startIndex < 0)
	{
		return false;
	}
	//寻找指定的结束点名称对应的索引
	index = 0;
	for each (DigraphNode<T> node in nodes)
	{
		if (node.getName() != endName)
			index++;
		else
		{
			endIndex = index;
			break;
		}
	}
	if(endIndex < 0)
	{
		return false;
	}

	//检查是否有重名项
	for each (DigraphArc a in nodes.at(startIndex).arcs)
	{
		if(a.getArcName() == arcName)
			return false;
	}
	if (startIndex >= 0 && endIndex >= 0)
		addArc(startIndex, endIndex, arcName, weight, isWait);
	return true;
}
