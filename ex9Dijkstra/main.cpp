//����ҽѧ�������ھ�ƽʱ��ҵ2��Windowsͼ�����C++�㷨��̳���
//3019202345��ǫ�� 
#include "resource.h"
#include "BForm.h"
#include "BDijkstra.h"
#include<stack>

CBForm form1(ID_form1);//�ô����Ĵ���ȥ��ʼ����CBForm
std::stack<LPTSTR> lsvDistsItemStack;//�������ɾ������Ŀ����ʵ�ֳ�������

/*��������*/
void form1_Load();//��CBForm form1���г�ʼ������
bool FunDjCalculatingCallBack(int iCycled, int iTotalCycleCurrent, long userData);//��ͨ��ģ����õĻص�����
//�������ĺ���
bool VerifyTxtInput(unsigned short int idResControl, LPCTSTR IdName);//����ı��������
//���������ť������Ӧ�Ķ���
void cmdAdd_Click();
void cmdDel_Click();
void cmdUndo_Click();
void cmdClear_Click();
void cmdExit_Click();
void cmdDo_Click();
void form1_QueryUnload(int pCancel);
//�Ҽ��˵����ݼ���صĶ���
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


//��CBForm form1���г�ʼ������
//��Ҫ��ʼ�����У�ID_lsvDists���б�����б�����
void form1_Load()
{
	form1.Control(ID_lsvDists).ListViewAddColumn(TEXT("Node1"));
	form1.Control(ID_lsvDists).ListViewAddColumn(TEXT("Node2"));
	form1.Control(ID_lsvDists).ListViewAddColumn(TEXT("Distence"));

	form1.Control(ID_lsvDists).ListViewGridLinesSet(true);
	form1.Control(ID_lsvDists).ListViewFullRowSelectSet(true);

	//����������������Ԥ���ص������ڷ�����ԣ�����ÿ�ζ�������������
	//��Dijkstra���·�滮�㷨��ع���ʵ�ֺ���ע�͵�����
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
	//	row_idx = form1.Control(ID_lsvDists).AddItem(Str(preoad_table[i].node1), 0);//index=0 ʱ����ĩβ�������Ŀ
	//	form1.Control(ID_lsvDists).ItemTextSet(row_idx, Str(preoad_table[i].node2), 2);
	//	form1.Control(ID_lsvDists).ItemTextSet(row_idx, Str(preoad_table[i].dis), 3);
	//}
	//
}

//����ı��������
//����true���ı������ݲ�Ϊ�գ�Ϊ��0������
bool VerifyTxtInput(const unsigned short int idResControl, const LPCTSTR IdName)
{
	LPTSTR szText = form1.Control(idResControl).Text();

	if (*szText == TEXT('\0'))
	{
		MsgBox(StrAppend(IdName, TEXT("Ϊ�գ�")), TEXT("�������ݱ���"), mb_OK, mb_IconExclamation);
		return false;
	}
	else if (Val(szText) == 0)//����Ϊ����Ҫ��취�ж������Ƿ�Ϊ���֣���ͨ�����ԵĽ��������Val()�Զ������������ж�
	{
		MsgBox(StrAppend(IdName, TEXT("�������ݱ���Ϊ�������֣�")), TEXT("�������ݱ���"), mb_OK, mb_IconExclamation);
		return false;
	}
	else
		return true;
}


//����µ���Ŀ
//����true�����������ݲ�Ϊ�գ�Ϊ��0������
void cmdAdd_Click()
{
	//��������ı�������ݶ�Ϊ���֣����Ϊ��������µ���Ŀ
	if (!VerifyTxtInput(ID_txtNode1, TEXT("���1")) ||
		!VerifyTxtInput(ID_txtNode2, TEXT("���2")) ||
		!VerifyTxtInput(ID_txtDist, TEXT("����")))
		return;

	//��ȡҪ��ӵ���Ŀ������
	LPTSTR szTextNode1 = form1.Control(ID_txtNode1).Text(),
		szTextNode2 = form1.Control(ID_txtNode2).Text(),
		szTextDist = form1.Control(ID_txtDist).Text();
	//AddItem()���һ����Ŀ����szTextNode1��ӵ�����Ŀ��һ�У�������ӵ���Ŀ��ţ���1��ʼ����������0
	//�����÷��صı��(i)ȥ�������Ŀ�ڶ��к͵�����
	int row_idx = form1.Control(ID_lsvDists).AddItem(szTextNode1, 0);//index=0 ʱ����ĩβ�������Ŀ
	form1.Control(ID_lsvDists).ItemTextSet(row_idx, szTextNode2, 2);
	form1.Control(ID_lsvDists).ItemTextSet(row_idx, szTextDist, 3);

	//��������ı��򣬷�����һ������
	form1.Control(ID_txtNode1).TextSet(TEXT(""));
	form1.Control(ID_txtNode2).TextSet(TEXT(""));
	form1.Control(ID_txtDist).TextSet(TEXT(""));

	//�����ĵ�һ��Сϸ�ڣ������ÿ��Ժ�Tab˳��������ʹ��
	//�ֱ�����ID_txtNode1 ID_txtNode2 ID_txtDist ID_cmdAdd��Tab˳��Ϊ1 2 3 4
	//������������Ŀ��ʱ��ֻ��Ҫ����Tab �س������ּ�������Ҫ����꣬Ч�����
	form1.Control(ID_txtNode1).SetFocus();
}

//ɾ������
void cmdDel_Click()
{
	int row_idx = form1.Control(ID_lsvDists).ListIndex();

	LPTSTR szText[3] = { form1.Control(ID_lsvDists).ItemText(row_idx,1),
						 form1.Control(ID_lsvDists).ItemText(row_idx,2),
						 form1.Control(ID_lsvDists).ItemText(row_idx,3) };
	form1.Control(ID_lsvDists).RemoveItem(row_idx);
	//��ɾ������Ŀѹ���ջ���Թ�����
	lsvDistsItemStack.push(szText[0]);
	lsvDistsItemStack.push(szText[1]);
	lsvDistsItemStack.push(szText[2]);
	lsvDistsItemStack.push((LPTSTR)row_idx);
}

//�������ܣ���ջ��Ԫ�ص���
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

//��գ�һ��Ҫ��ȷ����յĴ��ڣ�һ����գ����޷�ͨ�������ָ��ġ�
void cmdClear_Click()
{
	if (MsgBox(TEXT("ȷ��Ҫ��ս������б���"), TEXT("����б�����"), mb_OkCancel, mb_IconQuestion) == idOk)
	{
		form1.Control(ID_lsvDists).ListClear();
		//��ն�ջ
		while (!lsvDistsItemStack.empty())
		{
			lsvDistsItemStack.pop();
		}
	}
}

//�ص���������ͨ�ú�������
//������ʾ����״̬��ص���ʾ��Ϣ������ID_lblStatus��ʾ��
bool FunDjCalculatingCallBack(int iCycled, int iTotalCycleCurrent, long userData)
{
	form1.Control(ID_lblStatus).TextSet(iCycled);
	form1.Control(ID_lblStatus).TextAdd(TEXT("��ѭ������ɣ�"));
	if (iTotalCycleCurrent)
	{
		form1.Control(ID_lblStatus).TextAdd(TEXT("ʣ��"));
		form1.Control(ID_lblStatus).TextAdd(iTotalCycleCurrent);
		form1.Control(ID_lblStatus).TextAdd(TEXT("��ѭ��..."));
	}
	else
	{
		form1.Control(ID_lblStatus).TextAdd(TEXT("ѭ��������"));
	}
	DoEvents();
	return true;
}

void cmdDo_Click()
{
	/*��������*/
	CBDijkstra dj;
	long cntNode;// ���·���ϵĽ����� 
	long* idNodes;//��¼���õ������·���ϵĸ����
	long distance;//��̾���
	long cntLsvDistItem;//ID_lsvDists����Ŀ��


	/*��txt�л�ȡԴ���Ŀ������*/
	//long����int��Ϊ����Ӧ������GetDistance()�Ĳ�����������ʵ��intҲ�㹻�ˣ�long���Դ�����������
	long idNodeStart = form1.Control(ID_txtNodeCalc1).TextInt();
	long idNodeEnd = form1.Control(ID_txtNodeCalc2).TextInt();

	/*��ǿ³����*/
	//����Ϊֻ�ж��Ƿ�Ϊ0���ǲ����ģ���Ӧ���жϵ���Դ���Ŀ����Ƿ��ڵ������������
	//���Ǿ����Ķ�BDijikstra.cpp��Դ�뷢�֣��Ѿ��ڼ����ʱ������������⣬��˲����ڴ������ж�
	if (idNodeStart == 0 || idNodeEnd == 0)
	{
		MsgBox(TEXT("��������㣡"), TEXT("�������ݱ���"), mb_OK, mb_IconExclamation);
		form1.Control(ID_txtNodeCalc1).SetFocus();
		return;
	}
	if (idNodeStart == idNodeEnd)
	{
		MsgBox(TEXT("Դ���Ŀ��㲻����ͬһ���"), TEXT("���ID����"), mb_OK, mb_IconExclamation);
		form1.Control(ID_txtNodeCalc1).SetFocus();
		return;
	}


	dj.Clear();//�ʼ�����������ֹ�ϴμ�������δ���Ӱ���´μ���
	form1.Control(ID_cmdDo).EnabledSet(false);

	/*��lst�л�ȡ���;�����Ϣ������ʾ���繹�����*/
	form1.Control(ID_lblStatus).TextSet(TEXT("���ڹ�������..."));
	cntLsvDistItem = form1.Control(ID_lsvDists).ListCount();
	for (int i = 1;i <= cntLsvDistItem;i++)
	{
		LPTSTR szLsvListNode1 = form1.Control(ID_lsvDists).ItemText(i, 1),
			   szLsvListNode2 = form1.Control(ID_lsvDists).ItemText(i, 2),
  			   szLsvListDist = form1.Control(ID_lsvDists).ItemText(i, 3);
		dj.AddNodesDist((long)Val(szLsvListNode1), (long)Val(szLsvListNode2), (long)Val(szLsvListDist));

		if (i % 999 == 0)
		{
			form1.Control(ID_lblStatus).TextSet(TEXT("���ڹ�������..."));
			form1.Control(ID_lblStatus).TextAdd(i);
			form1.Control(ID_lblStatus).TextAdd(TEXT("/"));
			form1.Control(ID_lblStatus).TextAdd(cntLsvDistItem);
		}
		DoEvents();//�ó�CPU��ֹ�������
	}
	form1.Control(ID_lblStatus).TextSet(TEXT("���繹����ɡ�"));
	
	/*�������·����ͬʱ���û���ʾ�����״̬*/
	cntNode = dj.GetDistance(idNodeStart, idNodeEnd, distance, idNodes, FunDjCalculatingCallBack);


	/*��ʾ������*/
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
		MsgBox(TEXT("���֮����·����"), TEXT("������"), mb_OK, mb_IconExclamation);
	}
	else
	{
		MsgBox(TEXT("�������"), TEXT("������"), mb_OK, mb_IconExclamation);
	}

	dj.Clear();
	form1.Control(ID_cmdDo).EnabledSet(true);
}

void CopyDist()
{
	pApp->MousePointerGlobalSet(IDC_Wait);

	int nLines = form1.Control(ID_lsvDists).ListCount();//������
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
	ClipboardSetText(sBuff);//�����ı���������

	pApp->MousePointerGlobalSet(0);
}

void PasteDist()
{
	pApp->MousePointerGlobalSet(IDC_Wait);

	TCHAR* szText = ClipboardGetText();
	TCHAR** pLines, ** pFields;
	int nLines, nFields;
	nLines = Split(szText, pLines, TEXT("\n"), 30000);//����ճ��ǰ30000���ı�
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
	//ԭ����eKeyUp�������������ɿ�C/V���ɿ�Ctrl��ϵͳ��C/V keyup��ʱ����Ctrl��δ���µ�ʱ��
	//�趨keyCode == 67/86 && shift == 2���������ƽʱʹ��ϰ�߲�����
	//�ҽ�eKeyUp��ΪeKeyDown������������ƽʱ��ʹ���ָ�
	//eKeyDown���ڵ������ǣ��������Ctrl V�����֣���һֱճ���������������Ӧ�ã��������ڼ��±���������Լ�QQ�������������һ��
	//����Ctrl V�����֣�Ҳ��һֱճ�����������õ��������ô���õģ��෽���ǣ�������ѡ��eKeyDown
	if (keyCode == 67 && shift == 2)//Ctrl+C������
	{
		CopyDist();
	}
	else if (keyCode == 86 && shift == 2)//Ctrl+V������
	{
		PasteDist();
	}
	else if (keyCode == 90 && shift == 2)//Ctrl+Z������
	{
		cmdUndo_Click();
	}
	else if (keyCode == 68 && shift == 2)//Ctrl+D������
	{
		cmdDel_Click();
	}
	else if (keyCode == 68 && shift == 4)//Alt+D������
	{
		cmdClear_Click();
	}
}

void lsvDists_MouseDown(int button, int shift, int x, int y)
{
	if (button == 2)//������Ҽ�����
	{
		//�򵯳��˵�IDR_MENU1
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
	//����һ����̹����еķ��֣�Unload()�����ᴥ��eForm_QueryUnload���Ӷ�ִ�к���form1_QueryUnload(int pCancel)
	//������ں���cmdExit_Click()�����ټ�һ��MsgBox�����ȷ���˳��󣬻��ٵ���һ��MsgBox�����Ƿ�ȷ���˳�
	form1.UnLoad();
}

void form1_QueryUnload(int pCancel)
{
	if (MsgBox(TEXT("ȷʵҪ�˳���"), TEXT("ȷ���˳�"), mb_YesNo, mb_IconQuestion) == idNo)
	{

		*((int*)pCancel) = 1;//ȡ���˳�����
	}
}