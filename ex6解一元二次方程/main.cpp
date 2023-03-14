#include "resource.h"
#include "BForm.h"
#include <math.h>

CBForm form1(ID_form1);//用创建的窗体去初始化该CBForm

void cmdSolve_Click()
{
	double a = form1.Control(ID_txtA).TextVal(),
		b = form1.Control(ID_txtB).TextVal(),
		c = form1.Control(ID_txtC).TextVal();
	double delt = b * b - 4 * a * c;
	double x1, x2;
	if (delt < 0)
	{
		form1.Control(ID_txtX1).TextSet(TEXT("方程无实根！"));
		form1.Control(ID_txtX2).TextSet(TEXT("方程无实根！"));
	}
	else if (delt == 0)
	{
		x1 = (-b) / (2 * a);
		form1.Control(ID_txtX1).TextSet(x1);
		form1.Control(ID_txtX2).TextSet(TEXT(" "));
	}
	else
	{
		x1 = (-b + sqrt(delt)) / (2 * a);
		x2 = (-b - sqrt(delt)) / (2 * a);
		form1.Control(ID_txtX1).TextSet(x1);
		form1.Control(ID_txtX2).TextSet(x2);
	}
}


int main()
{
	form1.EventAdd(ID_cmdSolve, eCommandButton_Click, cmdSolve_Click);
	
	
	form1.IconSet(IDI_ICON1);
	form1.Show();
	return 0;
}

