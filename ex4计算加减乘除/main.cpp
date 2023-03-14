#include "resource.h"
#include "BForm.h"

CBForm form1(ID_form1);//用创建的窗体去初始化该CBForm

void cmdJia_Click()
{
	double a, b;
	a = form1.Control(ID_txtData1).TextVal();
	b = form1.Control(ID_txtData2).TextVal();
	form1.Control(ID_txtResult).TextSet(a + b);
}

void cmdJian_Click()
{
	double a, b;
	a = form1.Control(ID_txtData1).TextVal();
	b = form1.Control(ID_txtData2).TextVal();
	form1.Control(ID_txtResult).TextSet(a - b);
}

void cmdCheng_Click()
{
	double a, b;
	a = form1.Control(ID_txtData1).TextVal();
	b = form1.Control(ID_txtData2).TextVal();
	form1.Control(ID_txtResult).TextSet(a * b);
}

void cmdChu_Click()
{
	double a, b;
	a = form1.Control(ID_txtData1).TextVal();
	b = form1.Control(ID_txtData2).TextVal();
	if (b == 0)
	{
		MsgBox(TEXT("除数不能为0"), TEXT("出错！", mb_OK, mb_IconExclamation));
		form1.Control(ID_txtResult).TextSet(TEXT(""));
	}
	else
		form1.Control(ID_txtResult).TextSet(a / b);
}

void cmdExit_Click()
{
	form1.UnLoad();
}

int main()
{
	form1.EventAdd(ID_cmdJia, eCommandButton_Click, cmdJia_Click);
	form1.EventAdd(ID_cmdJian, eCommandButton_Click, cmdJian_Click);
	form1.EventAdd(ID_cmdCheng, eCommandButton_Click, cmdCheng_Click);
	form1.EventAdd(ID_cmdChu, eCommandButton_Click, cmdChu_Click);
	form1.EventAdd(ID_cmdExit, eCommandButton_Click, cmdExit_Click);
	
	form1.IconSet(IDI_ICON1);
	form1.Show();
	return 0;
}

