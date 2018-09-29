#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	MainWindow w;

	//设定组织名、产品名
	a.setOrganizationName(QSL("西北工业大学 NORTHWESTERN POLYTECHNICAL UNIVERSITY"));
	a.setApplicationName(QSL("鱼小雷AUV主控系统 YuXiaolei AUV Master System"));

	//设定背景颜色
	QPalette MainWinPal(w.palette());
	MainWinPal.setColor(QPalette::Background, QColor(100, 100, 100));
	w.setAutoFillBackground(true);
	w.setPalette(MainWinPal);

	//设定界面字体字号
	QFont font;
	font.setPointSize(9);
	font.setFamily(QStringLiteral("微软雅黑"));
	a.setFont(font);

	w.show();
	w.setWindowTitle(QStringLiteral("AUV 主控系统 - AUV Master System"));
	return a.exec();
}
