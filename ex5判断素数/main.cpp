#include "resource.h"
#include "BForm.h"
#include <math.h>

CBForm form1(ID_form1);//用创建的窗体去初始化该CBForm


bool isPrime(int n)
{
	int i, k = sqrt(double(n));
	for (i = 2;i <= k;i++)
		if (n % i == 0)
			break;
	if (i > k)
		return true;
	else
		return false;
}

void cmdPrime_Click()
{
	double d = form1.Control(ID_txtData).TextVal();
	if (isPrime(d)) {
		form1.Control(ID_txtResult).TextSet(
			StrAppend(Str(d), TEXT("是素数。"))
		);
	}
	else{
		form1.Control(ID_txtResult).TextSet(
			StrAppend(Str(d), TEXT("不是素数。"))
		);
	}
}

void cmdList100_Click()
{
	int i, n = 0;
	form1.Control(ID_list1).ListClear();
	for (i = 2;n < 100;i++)
	{
		if (isPrime(i)) {
			form1.Control(ID_list1).AddItem(Str((i)));
			n++;
		}
	}
}

int main()
{
	form1.EventAdd(ID_cmdPrime, eCommandButton_Click, cmdPrime_Click);
	form1.EventAdd(ID_cmdList100, eCommandButton_Click, cmdList100_Click);
	
	form1.IconSet(IDI_ICON1);
	form1.Show();
	return 0;
}

