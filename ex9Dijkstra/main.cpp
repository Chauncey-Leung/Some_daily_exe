//生物医学大数据挖掘平时作业2：Windows图像界面C++算法编程初步
//3019202345梁谦禧 
#include "resource.h"
#include "BForm.h"
#include "BDijkstra.h"
#include<stack>

CBForm form1(ID_form1);//用创建的窗体去初始化该CBForm
std::stack<LPTSTR> lsvDistsItemStack;//用来存放删除的条目，以实现撤销功能

/*函数声明*/
void form1_Load();//对CBForm form1进行初始化设置
bool FunDjCalculatingCallBack(int iCycled, int iTotalCycleCurrent, long userData);//被通用模块调用的回调函数
//检查输入的函数
bool VerifyTxtInput(unsigned short int idResControl, LPCTSTR IdName);//检查文本框的输入
//点击各个按钮产生对应的动作
void cmdAdd_Click();
void cmdDel_Click();
void cmdUndo_Click();
void cmdClear_Click();
void cmdExit_Click();
void cmdDo_Click();
void form1_QueryUnload(int pCancel);
//右键菜单与快捷键相关的动作
void CopyDist();
void PasteDist();
void lsvDists_KeyDown(int button, int shift, int pbCancel);
void lsvDists_MouseDown(int button, int shift, int x, int y);
void form1_MenuClick(int menuID, int bIsFormAcce, int bIsFromSysMenu);

int main()
{
	form1.EventAdd(0, eForm_Load, form1_Load);

	form1.EventAdd(ID_cmdAdd, eCommandButton_Click, cmdAdd_Click);
	form1.EventAdd(ID_cmdDel, eCommandButton_Click, cmdDel_Click);
	form1.EventAdd(ID_cmdUndo, eCommandButton_Click, cmdUndo_Click);
	form1.EventAdd(ID_cmdClear, eCommandButton_Click, cmdClear_Click);
	form1.EventAdd(ID_cmdDo, eCommandButton_Click, cmdDo_Click);
	form1.EventAdd(ID_cmdExit, eCommandButton_Click, cmdExit_Click);
	form1.EventAdd(0, eForm_QueryUnload, form1_QueryUnload);

	form1.EventAdd(ID_lsvDists, eMouseDown, lsvDists_MouseDown);
	form1.EventAdd(0, eMenu_Click, form1_MenuClick);
	form1.EventAdd(ID_lsvDists, eKeyDown, lsvDists_KeyDown);


	form1.IconSet(IDI_ICON1);
	form1.Show();
	return 0;
}


//对CBForm form1进行初始化设置
//需要初始化的有：ID_lsvDists的列标题和列表属性
void form1_Load()
{
	form1.Control(ID_lsvDists).ListViewAddColumn(TEXT("Node1"));
	form1.Control(ID_lsvDists).ListViewAddColumn(TEXT("Node2"));
	form1.Control(ID_lsvDists).ListViewAddColumn(TEXT("Distence"));

	form1.Control(ID_lsvDists).ListViewGridLinesSet(true);
	form1.Control(ID_lsvDists).ListViewFullRowSelectSet(true);

	//以下内容是事例，预加载到窗体内方便调试，不用每次都复制输入数据
	//等Dijkstra最短路规划算法相关功能实现后再注释掉即可
	//int row_idx;
	//int nLines = 9;
	//struct preLoadTable
	//{
	//	int node1;
	//	int node2;
	//	int dis;
	//};
	//preLoadTable preoad_table[9] = {
	//	{101,102,6},
	//	{101,103,3},
	//	{102,103,2},
	//	{102,104,5},
	//	{103,104,3},
	//	{103,105,4},
	//	{104,105,2},
	//	{104,106,3},
	//	{105,106,5 } };
	//for (int i = 0;i < nLines;i++)
	//{
	//	row_idx = form1.Control(ID_lsvDists).AddItem(Str(preoad_table[i].node1), 0);//index=0 时，在末尾添加新条目
	//	form1.Control(ID_lsvDists).ItemTextSet(row_idx, Str(preoad_table[i].node2), 2);
	//	form1.Control(ID_lsvDists).ItemTextSet(row_idx, Str(preoad_table[i].dis), 3);
	//}
	//
}

//检查文本框的输入
//返回true：文本框内容不为空，为非0的数字
bool VerifyTxtInput(const unsigned short int idResControl, const LPCTSTR IdName)
{
	LPTSTR szText = form1.Control(idResControl).Text();

	if (*szText == TEXT('\0'))
	{
		MsgBox(StrAppend(IdName, TEXT("为空！")), TEXT("输入数据报错"), mb_OK, mb_IconExclamation);
		return false;
	}
	else if (Val(szText) == 0)//本以为还需要想办法判断输入是否为数字，但通过调试的结果来看，Val()自动帮我做好了判断
	{
		MsgBox(StrAppend(IdName, TEXT("输入数据必须为非零数字！")), TEXT("输入数据报错"), mb_OK, mb_IconExclamation);
		return false;
	}
	else
		return true;
}


//添加新的条目
//返回true：剪贴板内容不为空，为非0的数字
void cmdAdd_Click()
{
	//检查三个文本框的内容都为数字，如果为空则不添加新的条目
	if (!VerifyTxtInput(ID_txtNode1, TEXT("结点1")) ||
		!VerifyTxtInput(ID_txtNode2, TEXT("结点2")) ||
		!VerifyTxtInput(ID_txtDist, TEXT("距离")))
		return;

	//获取要添加的条目的内容
	LPTSTR szTextNode1 = form1.Control(ID_txtNode1).Text(),
		szTextNode2 = form1.Control(ID_txtNode2).Text(),
		szTextDist = form1.Control(ID_txtDist).Text();
	//AddItem()添加一个条目并将szTextNode1添加到新条目第一列，返回添加的条目编号（从1开始），出错返回0
	//再利用返回的编号(i)去添加新条目第二列和第三列
	int row_idx = form1.Control(ID_lsvDists).AddItem(szTextNode1, 0);//index=0 时，在末尾添加新条目
	form1.Control(ID_lsvDists).ItemTextSet(row_idx, szTextNode2, 2);
	form1.Control(ID_lsvDists).ItemTextSet(row_idx, szTextDist, 3);

	//清空三个文本框，方便下一次输入
	form1.Control(ID_txtNode1).TextSet(TEXT(""));
	form1.Control(ID_txtNode2).TextSet(TEXT(""));
	form1.Control(ID_txtDist).TextSet(TEXT(""));

	//很贴心的一个小细节，该设置可以和Tab顺序结合起来使用
	//分别设置ID_txtNode1 ID_txtNode2 ID_txtDist ID_cmdAdd的Tab顺序为1 2 3 4
	//这样输入多个条目的时候，只需要键盘Tab 回车和数字键，不需要用鼠标，效率提高
	form1.Control(ID_txtNode1).SetFocus();
}

//删除功能
void cmdDel_Click()
{
	int row_idx = form1.Control(ID_lsvDists).ListIndex();

	LPTSTR szText[3] = { form1.Control(ID_lsvDists).ItemText(row_idx,1),
						 form1.Control(ID_lsvDists).ItemText(row_idx,2),
						 form1.Control(ID_lsvDists).ItemText(row_idx,3) };
	form1.Control(ID_lsvDists).RemoveItem(row_idx);
	//将删除的条目压入堆栈，以供撤销
	lsvDistsItemStack.push(szText[0]);
	lsvDistsItemStack.push(szText[1]);
	lsvDistsItemStack.push(szText[2]);
	lsvDistsItemStack.push((LPTSTR)row_idx);
}

//撤销功能，将栈顶元素弹出
void cmdUndo_Click()
{
	if (!lsvDistsItemStack.empty())
	{
		LPTSTR szText[3];
		int row_idx = (int)lsvDistsItemStack.top();
		lsvDistsItemStack.pop();
		szText[2] = lsvDistsItemStack.top();
		lsvDistsItemStack.pop();
		szText[1] = lsvDistsItemStack.top();
		lsvDistsItemStack.pop();
		szText[0] = lsvDistsItemStack.top();
		lsvDistsItemStack.pop();
		form1.Control(ID_lsvDists).AddItem(szText[0], row_idx);
		form1.Control(ID_lsvDists).ItemTextSet(row_idx, szText[1], 2);
		form1.Control(ID_lsvDists).ItemTextSet(row_idx, szText[2], 3);
	}
}

//清空，一定要有确认清空的窗口，一旦清空，是无法通过撤销恢复的。
void cmdClear_Click()
{
	if (MsgBox(TEXT("确定要清空结点距离列表吗？"), TEXT("清空列表数据"), mb_OkCancel, mb_IconQuestion) == idOk)
	{
		form1.Control(ID_lsvDists).ListClear();
		//清空堆栈
		while (!lsvDistsItemStack.empty())
		{
			lsvDistsItemStack.pop();
		}
	}
}

//回调函数，被通用函数调用
//用于显示计算状态相关的提示信息（借助ID_lblStatus显示）
bool FunDjCalculatingCallBack(int iCycled, int iTotalCycleCurrent, long userData)
{
	form1.Control(ID_lblStatus).TextSet(iCycled);
	form1.Control(ID_lblStatus).TextAdd(TEXT("次循环已完成，"));
	if (iTotalCycleCurrent)
	{
		form1.Control(ID_lblStatus).TextAdd(TEXT("剩余"));
		form1.Control(ID_lblStatus).TextAdd(iTotalCycleCurrent);
		form1.Control(ID_lblStatus).TextAdd(TEXT("次循环..."));
	}
	else
	{
		form1.Control(ID_lblStatus).TextAdd(TEXT("循环结束。"));
	}
	DoEvents();
	return true;
}

void cmdDo_Click()
{
	/*数据声明*/
	CBDijkstra dj;
	long cntNode;// 最短路径上的结点个数 
	long* idNodes;//记录所得到的最短路径上的各结点
	long distance;//最短距离
	long cntLsvDistItem;//ID_lsvDists的条目数


	/*从txt中获取源点和目标点参数*/
	//long而非int是为了适应函数的GetDistance()的参数声明，其实用int也足够了，long可以处理更大的数据
	long idNodeStart = form1.Control(ID_txtNodeCalc1).TextInt();
	long idNodeEnd = form1.Control(ID_txtNodeCalc2).TextInt();

	/*增强鲁棒性*/
	//本以为只判断是否为0还是不够的，还应该判断的是源点和目标点是否在导入的数据里面
	//但是经过阅读BDijikstra.cpp的源码发现，已经在计算的时候检查了这个问题，因此不用在此再做判断
	if (idNodeStart == 0 || idNodeEnd == 0)
	{
		MsgBox(TEXT("结点必须非零！"), TEXT("输入数据报错"), mb_OK, mb_IconExclamation);
		form1.Control(ID_txtNodeCalc1).SetFocus();
		return;
	}
	if (idNodeStart == idNodeEnd)
	{
		MsgBox(TEXT("源点和目标点不能是同一结点"), TEXT("结点ID错误"), mb_OK, mb_IconExclamation);
		form1.Control(ID_txtNodeCalc1).SetFocus();
		return;
	}


	dj.Clear();//最开始必须情况，防止上次计算数据未清除影响下次计算
	form1.Control(ID_cmdDo).EnabledSet(false);

	/*从lst中获取结点和距离信息，并显示网络构建情况*/
	form1.Control(ID_lblStatus).TextSet(TEXT("正在构建网络..."));
	cntLsvDistItem = form1.Control(ID_lsvDists).ListCount();
	for (int i = 1;i <= cntLsvDistItem;i++)
	{
		LPTSTR szLsvListNode1 = form1.Control(ID_lsvDists).ItemText(i, 1),
			   szLsvListNode2 = form1.Control(ID_lsvDists).ItemText(i, 2),
  			   szLsvListDist = form1.Control(ID_lsvDists).ItemText(i, 3);
		dj.AddNodesDist((long)Val(szLsvListNode1), (long)Val(szLsvListNode2), (long)Val(szLsvListDist));

		if (i % 999 == 0)
		{
			form1.Control(ID_lblStatus).TextSet(TEXT("正在构建网络..."));
			form1.Control(ID_lblStatus).TextAdd(i);
			form1.Control(ID_lblStatus).TextAdd(TEXT("/"));
			form1.Control(ID_lblStatus).TextAdd(cntLsvDistItem);
		}
		DoEvents();//让出CPU防止程序假死
	}
	form1.Control(ID_lblStatus).TextSet(TEXT("网络构建完成。"));
	
	/*计算最短路径，同时给用户显示计算的状态*/
	cntNode = dj.GetDistance(idNodeStart, idNodeEnd, distance, idNodes, FunDjCalculatingCallBack);


	/*显示运算结果*/
	form1.Control(ID_lstPath).ListClear();
	if (cntNode > 0)
	{
		form1.Control(ID_txtDistResu).TextSet(distance);

		for (int i = 0;i < cntNode;i++)
		{
			form1.Control(ID_lstPath).AddItem(Str(idNodes[i]));
		}
	}
	else if (cntNode < 0)
	{
		MsgBox(TEXT("结点之间无路径！"), TEXT("计算结果"), mb_OK, mb_IconExclamation);
	}
	else
	{
		MsgBox(TEXT("计算出错！"), TEXT("计算结果"), mb_OK, mb_IconExclamation);
	}

	dj.Clear();
	form1.Control(ID_cmdDo).EnabledSet(true);
}

void CopyDist()
{
	pApp->MousePointerGlobalSet(IDC_Wait);

	int nLines = form1.Control(ID_lsvDists).ListCount();//总行数
	tstring sBuff = TEXT("");

	for (int i = 1;i <= nLines;i++)
	{
		sBuff += form1.Control(ID_lsvDists).ItemText(i, 1);
		sBuff += TEXT("\t");
		sBuff += form1.Control(ID_lsvDists).ItemText(i, 2);
		sBuff += TEXT("\t");
		sBuff += form1.Control(ID_lsvDists).ItemText(i, 3);
		sBuff += TEXT("\n");
	}
	ClipboardSetText(sBuff);//复制文本到剪贴板

	pApp->MousePointerGlobalSet(0);
}

void PasteDist()
{
	pApp->MousePointerGlobalSet(IDC_Wait);

	TCHAR* szText = ClipboardGetText();
	TCHAR** pLines, ** pFields;
	int nLines, nFields;
	nLines = Split(szText, pLines, TEXT("\n"), 30000);//限制粘贴前30000行文本
	for (int i = 1;i <= nLines;i++)
	{

		nFields = Split(pLines[i], pFields, TEXT("\t"));
		if (nFields >= 3)
		{
			int row_idx = form1.Control(ID_lsvDists).AddItem(pFields[1], 0);
			form1.Control(ID_lsvDists).ItemTextSet(row_idx, pFields[2], 2);
			form1.Control(ID_lsvDists).ItemTextSet(row_idx, pFields[3], 3);
		}
	}
	pApp->MousePointerGlobalSet(0);
}

void lsvDists_KeyDown(int keyCode, int shift, int pbCancel)
{
	//原本是eKeyUp，这样必须先松开C/V后松开Ctrl，系统在C/V keyup的时候并且Ctrl还未按下的时候
	//设定keyCode == 67/86 && shift == 2，但是这个平时使用习惯不符，
	//我将eKeyUp改为eKeyDown，这样更符合平时的使用手感
	//eKeyDown存在的问题是，如果按下Ctrl V不松手，会一直粘贴，这个看起来不应该，但是我在记事本等软件，以及QQ聊天框中试验了一下
	//发现Ctrl V不松手，也会一直粘贴，即咱们用的软件就这么设置的，多方考虑，我最终选择eKeyDown
	if (keyCode == 67 && shift == 2)//Ctrl+C被按下
	{
		CopyDist();
	}
	else if (keyCode == 86 && shift == 2)//Ctrl+V被按下
	{
		PasteDist();
	}
	else if (keyCode == 90 && shift == 2)//Ctrl+Z被按下
	{
		cmdUndo_Click();
	}
	else if (keyCode == 68 && shift == 2)//Ctrl+D被按下
	{
		cmdDel_Click();
	}
	else if (keyCode == 68 && shift == 4)//Alt+D被按下
	{
		cmdClear_Click();
	}
}

void lsvDists_MouseDown(int button, int shift, int x, int y)
{
	if (button == 2)//若鼠标右键按下
	{
		//则弹出菜单IDR_MENU1
		form1.PopupMenu(IDR_MENU1, x, y);
	}
}

void form1_MenuClick(int menuID, int bIsFormAcce, int bIsFromSysMenu)
{
	switch (menuID)
	{
	case ID_mnuPopCopy:
		CopyDist();
		break;
	case ID_mnuPopPaste:
		PasteDist();
		break;
	case ID_mnuPopUndo:
		cmdUndo_Click();
		break;
	case ID_mnuPopDel:
		cmdDel_Click();
		break;
	case ID_mnuPopClear:
		cmdClear_Click();
		break;
	}
}

void cmdExit_Click()
{
	//这是一个编程过程中的发现，Unload()函数会触发eForm_QueryUnload，从而执行函数form1_QueryUnload(int pCancel)
	//如果我在函数cmdExit_Click()里面再加一条MsgBox，点击确定退出后，会再弹出一个MsgBox让你是否确定退出
	form1.UnLoad();
}

void form1_QueryUnload(int pCancel)
{
	if (MsgBox(TEXT("确实要退出吗？"), TEXT("确认退出"), mb_YesNo, mb_IconQuestion) == idNo)
	{

		*((int*)pCancel) = 1;//取消退出操作
	}
}