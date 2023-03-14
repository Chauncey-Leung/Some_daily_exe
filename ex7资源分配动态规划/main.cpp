#include "resource.h"
#include "BForm.h"

CBForm form1(ID_form1);//用创建的窗体去初始化该CBForm


/*
* DoPartResource() 动态规划求解资源分配问题
* iItems 项目总数
* iMoneys 资源总数
* g 即g(i,x),项目i下分配资源x所获得利润，g[0~iItems-1][0~iMoneys-1]
* sRetResult 返回结果字符串，每行为一个项目分配结果
* retMaxprofit 返回最大利润值
* sRetIntermedia 返回中间结果
* return true:成功 false:失败
*/
bool DoPartResource(int iItems,
					int iMoneys,
					float** g,
					tstring& sRetResult,
					float& retMaxprofit,
					tstring& sRetIntermedia)
{
	//开辟用来保存中间结果的动态二维数组f[][]
	float* fArray = new float[iItems * (iMoneys + 1)];
	float** f = new float* [iItems];
	float fMax;
	for (int i = 0;i < iItems;i++)
	{
		f[i] = fArray + i * (iMoneys + 1);
	}

	//开辟用来保存中间结果的动态二维数组y[][]
	//y(i,x)表示把总数为x的资源分配给前i个项目时，给第i个项目要分配的资金数（此时f(i,x)取得最大值）
	float* yArray = new float[iItems * (iMoneys + 1)];
	float** y = new float* [iItems];;
	for (int i = 0;i < iItems;i++)
	{
		y[i] = yArray + i * (iMoneys + 1);
	}

	//初始化f[][]和g[][]y0行
	for (int j = 0; j <= iMoneys; j++)
	{
		f[0][j] = g[0][j];
		y[0][j] = j;
	}
	//从第2个项目开始
	for (int i = 1;i < iItems;i++)
	{
		for (int j = 0; j<= iMoneys;j++)
		{
			fMax = -1;//代表极小值
			for (int k = 0;k <= j;k++)
			{
				if (fMax < g[i][k] + f[i - 1][j - k])
				{
					fMax = g[i][k] + f[i - 1][j - k];
					y[i][j] = k;
				}
			}
			f[i][j] = fMax;
		}
	}
	retMaxprofit = f[iItems - 1][iMoneys];

	int iSumX = iMoneys; //到第i个项目总共多少钱
	sRetResult = TEXT("");
	for (int i = iItems - 1;i >= 0;i--)
	{
		sRetResult = TEXT("\r\n") + sRetResult;
		sRetResult = Str(y[i][iSumX]) + sRetResult;
		iSumX -= y[i][iSumX];
	}

	sRetIntermedia = TEXT("");
	for (int i = 0; i < iItems;i++)
	{
		for (int j = 0;j <= iMoneys;j++)
		{
			sRetIntermedia += Str(f[i][j]);
			sRetIntermedia += TEXT("(y=");
			sRetIntermedia += Str(y[i][j]);
			sRetIntermedia += TEXT(")");
			if (j < iMoneys)
			{
				sRetIntermedia += TEXT("\t");
			}
		}
		sRetIntermedia += TEXT("\r\n");
	}

	/*for (int i = 0;i < iItems;i++)
	{
		delete y[i];
	}*/
	delete[] y;
	
	delete[] yArray;
	
	/*for (int i = 0;i < iItems;i++)
	{
		delete f[i];
	}*/
	delete[] f;
	
	delete[] fArray;
	

	return true;
}

void cmdDo_Click()
{
	int iItems = form1.Control(ID_txtItems).TextInt();
	int iMoneys = form1.Control(ID_txtMoneys).TextInt();
	if (iItems <= 0 || iMoneys <= 0)
	{
		MsgBox(TEXT("请输入正确的项目数和资金总数（正整数）。"), TEXT("数据错误"), mb_OK, mb_IconExclamation);
		return;
	}

	//构建动态二维数组，用于算法计算
	TCHAR* szGsText = form1.Control(ID_txtSrcGs).Text();//无论是ANSI版还是Unicode 版，我们在代码中都统一用 TCHAR作为字符类型，而不 用char、也不用wchar_t 
	if (*szGsText == TEXT('\0'))
	{
		MsgBox(TEXT("请输入g(i,x)的原始数据！"), TEXT("数据错误"), mb_OK, mb_IconExclamation);
		return;
	}

	float* gArray = new float[iItems * (iMoneys + 1)];
	float** g = new float* [iItems];
	for (int i = 0;i < iItems;i++)
	{
		g[i] = gArray + i * (iMoneys + 1);
	}

	TCHAR** lines = NULL;
	int iLines = 0;
	iLines = Split(szGsText, lines, TEXT("\r\n"));
	if (iLines < iItems)
	{
		tstring sErrInfp;
		sErrInfp = TEXT("要包含");
		sErrInfp += Str(iItems);
		sErrInfp += TEXT("行g(i,x)的数据（行数和项目数相等）。");
		MsgBox(sErrInfp, TEXT("数据错误"), mb_OK, mb_IconExclamation);
		return;
	}

	for (int i = 1;i <= iLines;i++)
	{
		TCHAR** flds = NULL;
		int iFields = 0;
		if(*lines[i] == TEXT('\0'))
			continue;
		if (InStr(lines[i], TEXT("\t"))==0)
			iFields = Split(lines[i], flds, TEXT(" "));
		else
			iFields = Split(lines[i], flds, TEXT("\t"));
		if (iFields != iMoneys + 1)
		{
			tstring sErrInfp;
			sErrInfp = TEXT("行：\r\n");
			sErrInfp += lines[i];
			sErrInfp += TEXT("\r\n应该包含");
			sErrInfp += Str(iMoneys + 1);
			sErrInfp += TEXT(" 个字段");
			MsgBox(sErrInfp, TEXT("数据错误"), mb_OK, mb_IconExclamation);
			return;
		}

		for (int j = 1; j <= iFields;j++)
		{
			g[i - 1][j - 1] = Val(flds[j]);
		}
	}

	float fMaxProfit = 0;
	tstring sResults, sIntermedia;
	DoPartResource(iItems,iMoneys, g, sResults, fMaxProfit, sIntermedia);

	//show
	form1.Control(ID_txtFs).TextSet(sIntermedia);
	form1.Control(ID_txtMaxProfit).TextSet(fMaxProfit);
	iLines = Split(sResults.c_str(), lines, TEXT("\r\n"));
	form1.Control(ID_lstResult).ListClear();
	for (int i = 1;i <= iLines;i++)
	{
		if (*lines[i] == TEXT('\0'))
			continue;
		tstring sItem;
		sItem = TEXT("项目");
		sItem += Str(i);
		sItem += TEXT("  ");
		sItem += lines[i];
		form1.Control(ID_lstResult).AddItem(sItem);
	}

	/*for (int i = 0;i < iItems;i++)
	{
		delete g[i];
	}*/
	delete [] g;
	
	delete[] gArray;

}

void cmdExit_Click()
{
	form1.UnLoad();
}

int main()
{
	form1.EventAdd(ID_cmdDo, eCommandButton_Click, cmdDo_Click);
	form1.EventAdd(ID_cmdExit, eCommandButton_Click, cmdExit_Click);
	
	form1.IconSet(IDI_ICON1);
	form1.Show();
	return 0;
}

