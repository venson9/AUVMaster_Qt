#include <qobject>

//求数组的中值
//p:数组头指针，a:数组个数
double median(double *p, int a)
{
	double *sorted = new double[sizeof(double)*a];
	memcpy(sorted, p, sizeof(double)*a);
	//排序
	int i = 0;
	while (i < a)
	{
		int j = i + 1;
		while (j < a)
		{
			if (sorted[j] >= sorted[i])
			{
				double temp = sorted[j];
				sorted[j] = sorted[i];
				sorted[i] = temp;
			}
			j++;
		}
		i++;
	}
	//for (int k = 0; k < a; k++)
	//{
	//	cout << sorted[k] << ", ";
	//}
	//cout << endl;
	double result;
	if (a % 2 == 0)//双数
		result = (sorted[a / 2 - 1] + sorted[a / 2]) / 2;
	else
		result = (sorted[(a + 1) / 2 - 1]);
	delete[] sorted;
	return result;
}


/*带符号的角度差值计算函数 -180 ~ 180
* 参数：参数2为基准，向右为正，向左为负
* 返回：带符号的角度差值*/
double signedDeltaAngle(double tgt, double base)
{
	while (tgt < 0)
		tgt += 360;
	while (base < 0)
		base += 360;

	double delta = tgt - base;

	while (delta > 180)
		delta -= 360;
	while (delta < -180)
		delta += 360;
	return delta;
}
