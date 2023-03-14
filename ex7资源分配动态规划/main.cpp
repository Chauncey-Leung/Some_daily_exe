#include "resource.h"
#include "BForm.h"

CBForm form1(ID_form1);//�ô����Ĵ���ȥ��ʼ����CBForm


/*
* DoPartResource() ��̬�滮�����Դ��������
* iItems ��Ŀ����
* iMoneys ��Դ����
* g ��g(i,x),��Ŀi�·�����Դx���������g[0~iItems-1][0~iMoneys-1]
* sRetResult ���ؽ���ַ�����ÿ��Ϊһ����Ŀ������
* retMaxprofit �����������ֵ
* sRetIntermedia �����м���
* return true:�ɹ� false:ʧ��
*/
bool DoPartResource(int iItems,
					int iMoneys,
					float** g,
					tstring& sRetResult,
					float& retMaxprofit,
					tstring& sRetIntermedia)
{
	//�������������м����Ķ�̬��ά����f[][]
	float* fArray = new float[iItems * (iMoneys + 1)];
	float** f = new float* [iItems];
	float fMax;
	for (int i = 0;i < iItems;i++)
	{
		f[i] = fArray + i * (iMoneys + 1);
	}

	//�������������м����Ķ�̬��ά����y[][]
	//y(i,x)��ʾ������Ϊx����Դ�����ǰi����Ŀʱ������i����ĿҪ������ʽ�������ʱf(i,x)ȡ�����ֵ��
	float* yArray = new float[iItems * (iMoneys + 1)];
	float** y = new float* [iItems];;
	for (int i = 0;i < iItems;i++)
	{
		y[i] = yArray + i * (iMoneys + 1);
	}

	//��ʼ��f[][]��g[][]y0��
	for (int j = 0; j <= iMoneys; j++)
	{
		f[0][j] = g[0][j];
		y[0][j] = j;
	}
	//�ӵ�2����Ŀ��ʼ
	for (int i = 1;i < iItems;i++)
	{
		for (int j = 0; j<= iMoneys;j++)
		{
			fMax = -1;//����Сֵ
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

	int iSumX = iMoneys; //����i����Ŀ�ܹ�����Ǯ
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
		MsgBox(TEXT("��������ȷ����Ŀ�����ʽ�����������������"), TEXT("���ݴ���"), mb_OK, mb_IconExclamation);
		return;
	}

	//������̬��ά���飬�����㷨����
	TCHAR* szGsText = form1.Control(ID_txtSrcGs).Text();//������ANSI�滹��Unicode �棬�����ڴ����ж�ͳһ�� TCHAR��Ϊ�ַ����ͣ����� ��char��Ҳ����wchar_t 
	if (*szGsText == TEXT('\0'))
	{
		MsgBox(TEXT("������g(i,x)��ԭʼ���ݣ�"), TEXT("���ݴ���"), mb_OK, mb_IconExclamation);
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
		sErrInfp = TEXT("Ҫ����");
		sErrInfp += Str(iItems);
		sErrInfp += TEXT("��g(i,x)�����ݣ���������Ŀ����ȣ���");
		MsgBox(sErrInfp, TEXT("���ݴ���"), mb_OK, mb_IconExclamation);
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
			sErrInfp = TEXT("�У�\r\n");
			sErrInfp += lines[i];
			sErrInfp += TEXT("\r\nӦ�ð���");
			sErrInfp += Str(iMoneys + 1);
			sErrInfp += TEXT(" ���ֶ�");
			MsgBox(sErrInfp, TEXT("���ݴ���"), mb_OK, mb_IconExclamation);
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
		sItem = TEXT("��Ŀ");
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

