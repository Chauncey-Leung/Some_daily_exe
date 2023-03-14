#include "resource.h"
#include "BForm.h"

CBForm form1(ID_form1);//用创建的窗体去初始化该CBForm

struct SPairScores 
{
	int scMatch;		//匹配得分数
	int scNotMatch;		//不匹配得分数
	int scGap;			//空位得分数
};
const int c_PathDiagonal = 1;	//0001B—从左上角斜对角来
const int c_PathVertical = 2;	//0010B—从左面水平而来
const int c_PathHorizontal = 4;	//0100B—从上面垂直而来

bool AlignmentTwoSeqs(LPTSTR seq1, LPTSTR seq2, SPairScores pairScores,
	tstring& sRetPairedSeq1, tstring& sRetPairedSeq2, tstring& sRetIntermedia)
{
	int iLen1 = _tcslen(seq1),
		iLen2 = _tcslen(seq2);
	
	//开辟并准备mat[0~iLen1][0~iLen2] 动态二维数组，用来保存中间得分结果
	//维度为(iLen1 + 1)*(iLen2 + 1),+1的原因是还有一个0行0列作为起始点
	int* matArray = new int[(iLen1 + 1) * (iLen2 + 1)];
	int** mat = new int* [iLen1 + 1];
	for (int i = 0;i <= iLen1;i++)
	{
		mat[i] = matArray + i * (iLen2 + 1);
	}
	for (int i = 0;i <= (iLen1 + 1) * (iLen2 + 1);i++)
	{
		matArray[i] = 0;
	}

	//开辟并准备path[0~iLen1][0~iLen2] 动态二维数组，用来保存每一步的路径
	int* pathArray = new int[(iLen1 + 1) * (iLen2 + 1)];
	int** path = new int* [iLen1 + 1];
	for (int i = 0;i <= iLen1;i++)
	{
		path[i] = pathArray + i * (iLen2 + 1);
	}
	for (int i = 0;i <= (iLen1 + 1) * (iLen2 + 1);i++)
	{
		pathArray[i] = 0;
	}

	//初始化第0行和第0列
	//第0行每个元素只能水平向左得到，第0列每个元素只能垂直向下得到
	mat[0][0] = 0; //没必要再赋值，但看着完整舒服
	path[0][0] = 0;//没必要再赋值，但看着完整舒服
	for (int j = 1;j <= iLen2;j++)
	{
		mat[0][j] = mat[0][j - 1] + pairScores.scGap;
		path[0][j] = c_PathHorizontal;
	}
	for (int i = 1;i <= iLen1;i++)
	{
		mat[i][0] = mat[i - 1][0] + pairScores.scGap;
		path[i][0] = c_PathVertical;
	}

	for (int i = 1;i <= iLen1;i++)
	{
		for (int j = 1;j <= iLen2; j++)
		{
			int iHori, iVert, iDiag;
			iHori = mat[i][j - 1] + pairScores.scGap;
			iVert = mat[i - 1][j] + pairScores.scGap;
			iDiag = ((seq1[i - 1] == seq2[j - 1]) ? 
						(mat[i - 1][j - 1] + pairScores.scMatch) : 
						(mat[i - 1][j - 1] + pairScores.scNotMatch));

			//比较iHori iVert iDiag的大小情况
			if (iHori > iVert)
			{
				mat[i][j] = (iHori > iDiag) ? iHori : iDiag;
				path[i][j] = (iHori > iDiag) ? (path[i][j] | c_PathHorizontal) :
					((iHori < iDiag) ? (path[i][j] | c_PathDiagonal) :
						(path[i][j] | c_PathDiagonal | c_PathHorizontal));
			}
			else if(iVert > iHori)
			{
				mat[i][j] = (iVert > iDiag) ? iVert : iDiag;
				path[i][j] = (iVert > iDiag) ? (path[i][j] | c_PathVertical) :
					((iVert < iDiag) ? (path[i][j] | c_PathDiagonal) :
						(path[i][j] | c_PathDiagonal | c_PathVertical));
			}
			else//iHori == iVert
			{
				mat[i][j] = (iVert > iDiag) ? iVert : iDiag;
				path[i][j] = (iVert > iDiag) ? (path[i][j] | c_PathVertical | c_PathHorizontal) :
					((iVert < iDiag) ? (path[i][j] | c_PathDiagonal) :
						(path[i][j] | c_PathDiagonal | c_PathVertical | c_PathDiagonal));
			}
		}
	}

	//返回矩阵结果
	sRetIntermedia = TEXT("\t\t");

	//show table head
	for (int j = 1;j <= iLen2;j++)
	{
		sRetIntermedia += seq2[j - 1];
		sRetIntermedia += TEXT("\t");
	}
	sRetIntermedia += TEXT("\r\n");
	//show row 0
	sRetIntermedia = TEXT("\t");
	for (int j = 0;j <= iLen2;j++)
	{
		sRetIntermedia += Str(mat[0][j]);
		sRetIntermedia += TEXT("\t");
	}
	sRetIntermedia += TEXT("\r\n");
	//show the rest row
	for (int i = 1;i <= iLen1;i++)
	{
		sRetIntermedia += seq1[i - 1];
		sRetIntermedia += TEXT("\t");
		for (int j = 0;j <= iLen2;j++)
		{
			sRetIntermedia += Str(mat[i][j]);
			sRetIntermedia += TEXT("\t");
		}
		sRetIntermedia += TEXT("\r\n");
	}

	//通过回溯比对结果，得到sRetPairedSeq1，sRetPairedSeq2
	int i = iLen1, 
		j = iLen2;
	while (i > 0 && j > 0)
	{
		if (path[i][j] & c_PathDiagonal)
		{
			sRetPairedSeq1 = seq1[i - 1] + sRetPairedSeq1;
			sRetPairedSeq2 = seq2[j - 1] + sRetPairedSeq2;
			i--;
			j--;
		}
		else if (path[i][j] & c_PathVertical)
		{
			sRetPairedSeq1 = seq1[i - 1] + sRetPairedSeq1;
			sRetPairedSeq2 = TEXT("-") + sRetPairedSeq2;
			i--;
		}
		else if (path[i][j] & c_PathHorizontal)
		{
			sRetPairedSeq1 = TEXT("-") + sRetPairedSeq1;
			sRetPairedSeq2 = seq2[j - 1] + sRetPairedSeq2;
			j--;
		}
	}

	//判断序列前面是否存在空位
	if (i > 0)
	{
		while (i > 0)
		{
			sRetPairedSeq1 = seq1[i - 1] + sRetPairedSeq1;
			sRetPairedSeq2 = TEXT("-") + sRetPairedSeq2;
			i--;
		}
	}
	if (j > 0)
	{
		while (j > 0)
		{
			sRetPairedSeq1 = TEXT("-") + sRetPairedSeq1;
			sRetPairedSeq2 = seq2[j - 1] + sRetPairedSeq2;
			j--;
		}
	}
	delete []path;//no problem
	delete []pathArray;
	delete []mat;//no problem
	delete []matArray;
	return true;
}

/*
* 设置默认得分、样例序列与ID_txtResuPair的默认字体字号
*/
void form1_Load()
{
	form1.Control(ID_txtScoreMatch).TextSet(TEXT("1"));
	form1.Control(ID_txtScoreNot).TextSet(TEXT("0"));
	form1.Control(ID_txtScoreGap).TextSet(TEXT("-1"));

	form1.Control(ID_txtSeq1).TextSet(TEXT("alignment"));
	form1.Control(ID_txtSeq2).TextSet(TEXT("assignment"));

	form1.Control(ID_txtResuPair).FontNameSet(TEXT("宋体"));
	form1.Control(ID_txtResuPair).FontSizeSet(14);
}

void cmdAlignment_Click()
{
	//获取用户输入的序列
	LPTSTR szSeq1 = form1.Control(ID_txtSeq1).Text();
	LPTSTR szSeq2 = form1.Control(ID_txtSeq2).Text();
	
	//获取得分函数的分值
	SPairScores pairScores = {
		form1.Control(ID_txtScoreMatch).TextInt(),
		form1.Control(ID_txtScoreNot).TextInt(),
		form1.Control(ID_txtScoreGap).TextInt()
	};

	tstring sPairedSeq1, sPairedSeq2, sIntermedia;
	AlignmentTwoSeqs(szSeq1, szSeq2, pairScores, sPairedSeq1, sPairedSeq2, sIntermedia);

	//show result
	form1.Control(ID_txtIntermedia).TextSet(sIntermedia);
	form1.Control(ID_txtResuPair).TextSet(sPairedSeq1 + TEXT("\r\n") + sPairedSeq2);
}

int main()
{
	form1.EventAdd(0, eForm_Load, form1_Load);
	form1.EventAdd(ID_cmdAlignment, eCommandButton_Click, cmdAlignment_Click);

	form1.IconSet(IDI_ICON1);
	form1.Show();
	return 0;
}

