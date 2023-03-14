#include "resource.h"
#include "BForm.h"

#include "mdlOpenSaveDlg.h"
#include "mdlSvmRun.h"
#include "mdlShellExec.h"
#include "mdlFileSys.h"

CBForm form1(ID_form1);

void cmdBrowDataFile_Click()
{
	// 浏览选择特征文件
	OsdSetFilter(TEXT("文本文件(*.txt)|*.txt"));
	TCHAR* file = OsdOpenDlg(form1.hWnd());
	if (*file)
	{
		form1.Control(ID_cboDataFile).AddItem(file);
		form1.Control(ID_cboDataFile).TextSet(file);
	}
}
void cmdBrowScaleParamFileTrain_Click()
{
	// 设置训练时的缩放操作的参数要保存到的文件
	OsdSetFilter(TEXT("文本文件(*.txt)|*.txt"));
	TCHAR* file = OsdSaveDlg(form1.hWnd());
	if (*file)
	{
		form1.Control(ID_cboScaleParamFileTrain).AddItem(file);
		form1.Control(ID_cboScaleParamFileTrain).TextSet(file);
	}
}
void cmdBrowModelFileTest_Click()
{
	//测试时，浏览选择要打开的模型文件
	OsdSetFilter(TEXT("模型文件(*.model)|*.model|文本文件(*.txt)|*.txt"));
	TCHAR* file = OsdOpenDlg(form1.hWnd());//不是 OsdSaveDlg
	if (*file)
	{
		form1.Control(ID_cboModelFileTest).AddItem(file);
		form1.Control(ID_cboModelFileTest).TextSet(file);
	}
}

void cmdBrowResultFileTest_Click()
{			
	//设置测试时的最终结果要保存到的文件
	OsdSetFilter(TEXT("文本文件(*.txt)|*.txt"));
	TCHAR* file = OsdSaveDlg(form1.hWnd());//不是 OsdOpenDlgif(*file)
	{
		form1.Control(ID_cboResultFileTest).AddItem(file);
		form1.Control(ID_cboResultFileTest).TextSet(file);
	}
}
void cmdBrowScaleParamFileTest_Click()
{
	//测试时，若缩放数据，浏览选择要打开的缩放参数文件
	OsdSetFilter(TEXT("文本文件(*.txt)|*.txt"));
	TCHAR* file = OsdOpenDlg(form1.hWnd());// 不是 0sdSaveDlgif(*file)
	{
		form1.Control(ID_cboScaleParamFileTest).AddItem(file);
		form1.Control(ID_cboScaleParamFileTest).TextSet(file);
	}
}

//若训练时的归一化参数要保存到的文件名尚未慎写，则自动填入一个 
void AutoFillScaleParamFileTrain()
{
	LPTSTR szFileData = form1.Control(ID_cboDataFile).Text();
	LPTSTR szFileParamTrain = form1.Control(ID_cboScaleParamFileTrain).Text();
	if (*szFileParamTrain == 0 && *szFileData != 0)
	{
		//在szFileData 文件名的扩展名之前加.param，消除扩展名并用.txt
		//这样生成要保存的参数文件的文件名，设置到ID_cboScaleParamFileTrain
		LPTSTR szPath, szExp;
		LPTSTR szFile = FMTrimFileName(szFileData, true, true, &szPath, &szExp);
		tstring sParamFile = szPath;
		sParamFile = sParamFile + TEXT("\\") + szFile + TEXT(".param.txt");
		form1.Control(ID_cboScaleParamFileTrain).TextSet(sParamFile);
	}
}

void chkScaleTrain_Click()
{
	//如勾选，设置相关浏览控件使能;否则禁用（变灰)它们
	bool fChecked = form1.Control(ID_chkScaleTrain).ValueChecked();
	form1.Control(ID_lblScaleParamTrain).EnabledSet(fChecked);
	form1.Control(ID_cboScaleParamFileTrain).EnabledSet(fChecked);
	form1.Control(ID_cmdBrowScaleParamFileTrain).EnabledSet(fChecked);
	//如勾选为有效，且尚未填写文件名,自动填入一个文件名
	if (fChecked)
		AutoFillScaleParamFileTrain();
}

void chkScaleTest_Click()
{
	//如勾选，设置相关浏览控件使能;否则禁用（变灰)它们
	bool fChecked = form1.Control(ID_chkScaleTest).ValueChecked();
	form1.Control(ID_lblScaleParamTest).EnabledSet(fChecked);
	form1.Control(ID_cboScaleParamFileTest).EnabledSet(fChecked);
	form1.Control(ID_cmdBrowScaleParamFileTest).EnabledSet(fChecked);
}

void cbo_FilesDrop(int ptrArrFiles, int count,int x, int y) 
{
	//将参数ptrArrFiles强制转换为TCHAR**类型（固定套路)
	TCHAR** files = (TCHAR**)ptrArrFiles;
	//则files可被当做是TCHAR（字符）的二维数组，也是字符串的一维数组
	//用files[1]、 files[2]、……、files[count]获得各个字符串
	//这些字符串就是用户拖动多个文件时，被拖动到控件土的各文件名
	//但这里我们只打开一个文件 files[1]，当用户拖动多个文件时，忽略其他文件
	//将所选择的文件全路径添加到组合框（多个组合框的事件都共用本函数)
	form1.Control(form1.IDRaisingEvent()).AddItem(files[1]);
	//在组合框中显示所选择的文件全路径
	form1.Control(form1.IDRaisingEvent()).TextSet(files[1]);
}
	
void form1_Load()
{
	//向控件ID_cboKernels（核函数）中添加选项:检查此控件的Sort属性一定设为False
	CBControl cboKernels(ID_cboKernels);
	cboKernels.AddItem(TEXT("线性核(Linear，t=0)"));
	cboKernels.AddItem(TEXT("多项式核(Polynomial，t=1)"));
	cboKernels.AddItem(TEXT("高斯径向基核(Radial Basis Function，t=2)"));
	cboKernels.AddItem(TEXT("二层神经网络核(Sigmoid,t=3)"));
	cboKernels.ListIndexSet(3);// 默认选中第$个条目(RBF)
	//向控件ID_cboFold(交叉验证)中添加选项:检查此控件的Sort属性一定设为False
	CBControl cboFold(ID_cboFold);
	for (int i = 2; i <= 10; i++)
		cboFold.AddItem(Str(i));
	cboFold.TextSet(TEXT("5"));//默认选择:5-折交叉验证
	//在运行时，就显示控件的使能状态（必要时声明以下函数)chkScaleTrain_Click() ;
	chkScaleTest_Click();
	chkScaleTrain_Click();
}


void Train_Click()
{
	//获得特征文件文件名
	LPTSTR szFileData = form1.Control(ID_cboDataFile).Text();

	if (*szFileData == 0)
	{
		MsgBox(TEXT("请选择训练用的特征数据文件。"), TEXT("未指定训练用的特征数据文件"), mb_OK, mb_IconExclamation);
		return;
	}
	//获得缩放数据的参数要保存到的文件名szFileScaleParam
	//不缩放数据时，其为NULL
	LPTSTR szFileScaleParam = NULL;
	if (form1.Control(ID_chkScaleTrain).ValueChecked())
	{
		szFileScaleParam = form1.Control(ID_cboScaleParamFileTrain).Text();
		if (*szFileScaleParam == 0)
		{
			MsgBox(TEXT("请输入训练时缩放参数要保存到的文件。"), TEXT("未指定缩放参数文件"), mb_OK, mb_IconExclamation);
			return;
		}
	}

	// model文件(生成model文件时使用;如仅训练不生成model文件，此为NULL)
	TCHAR* szFileModel = NULL;
	if (form1.IDRaisingEvent() == ID_cmdGenModel)
	{
		//生成model文件时，选择保存model文件
		OsdSetFilter(TEXT("模型文件(*.model)|*.model|文本文件(*.txt)|*.txt"));
		szFileModel = OsdSaveDlg(form1.hWnd());
		if (*szFileModel == 0)
			return;// 用户在选择model文件时取消
	}

	tstring sReturn;
	bool ret = SVMTrain(szFileData, szFileModel, szFileScaleParam,
						form1.Control(ID_cboKernels).ListIndex() - 1,
						form1.Control(ID_cboFold).TextInt(),
						form1.Control(ID_txtParamsTrain).Text(), sReturn);
	if (ret)
	{
		if (form1.IDRaisingEvent() == ID_cmdGenModel)//生成model文件
		{
					
			sReturn = TEXT("model文件生成成功:\r\n") + sReturn;
			MsgBox(sReturn, TEXT("生成model文件"), mb_OK, mb_IconExclamation);
		}
		else//仅训练不生成model
		{
			//通过记事本显示各评估指标的结果文件（文件名为sReturn)/ / SEShellRun函数是mdlShellExec通用模块中的函数
			tstring sCmd;
			sCmd = TEXT("NotePad.exe \"") + sReturn + TEXT(" \"");
			SEShellRun(sCmd.c_str(), true);
		}
	}
	else//失败，显示错误信息
	{
		MsgBox(sReturn, TEXT("训练失败"), mb_OK, mb_IconExclamation);
	}
	
}

void cmdPredict_Click()
{
	//获得特征文件文件名
	LPTSTR szFileData = form1.Control(ID_cboDataFile).Text();
	if (*szFileData == 0)
	{
		MsgBox(TEXT("请选择测试用的特征数据文件。"),
			TEXT("未指定数据文件"), mb_OK, mb_IconExclamation);
		return;
	}
	// 获得model文件文件名
	TCHAR* szFileModel = form1.Control(ID_cboModelFileTest).Text();
	if (*szFileModel == 0)
	{
		MsgBox(TEXT("请选择测试用的 model（模型）文件，该文件由训练步生成。"),
			TEXT("未指定model文件"), mb_OK, mb_IconExclamation);
		return;
	}

	//获得结果文件文件名
	TCHAR* szFileResult = form1.Control(ID_cboResultFileTest).Text();
	if (*szFileResult == 0)
	{
		MsgBox(TEXT("请指定测试后的结果要保存到的文件。"),
			TEXT("未指定结果文件"), mb_OK, mb_IconExclamation);
		return;
	}
	//获得缩放数据的参数要保存到的文件名=>szFileScaleParam
	//不缩放数据时,其为NULL
	LPTSTR szFileScaleParam = NULL;
	if (form1.Control(ID_chkScaleTest).ValueChecked())
	{
		szFileScaleParam = form1.Control(ID_cboScaleParamFileTest).Text();
		if (*szFileScaleParam == 0)
		{
			MsgBox(TEXT("请输入测试时缩放参数文件（该文件要与训练时的相同）。"),
				TEXT("未指定缩放参数文件"), mb_OK, mb_IconExclamation);
			return;
		}
	}

	//获得是否要预测概率估计值=> iPararB（如是此值为1)
	int iParamB = 0;
	if (form1.Control(ID_chkPredictB).ValueChecked())
		iParamB = 1;
	tstring sReturn;
	bool ret = SVMPredict(szFileData, szFileModel, szFileScaleParam, szFileResult, iParamB, sReturn);
	if (ret)
	{
		if (MsgBox(TEXT("预测结果文件生成成功!要现在打开吗? "),
			TEXT("预测完成"), mb_OkCancel, mb_IconQuestion) == idOk)
		{
			//打开预测结果文件
			tstring sCmd;
			sCmd = TEXT("NotePad.exe \"") + sReturn + TEXT(" \"");
			SEShellRun(sCmd.c_str(), true);
		}
	}
	else //失败
	{
		MsgBox(sReturn, TEXT("测试失败"), mb_OK, mb_IconExclamation);
	}
}

int main()
{
	if (!VerifyLibSVMFiles())
	{
		return 1;
	}

	form1.EventAdd(0, eForm_Load, form1_Load);
	form1.EventAdd(ID_cmdBrowDataFile, eCommandButton_Click, cmdBrowDataFile_Click);
	form1.EventAdd(ID_cmdBrowScaleParamFileTest, eCommandButton_Click, cmdBrowScaleParamFileTest_Click);
	form1.EventAdd(ID_cmdBrowModelFileTest, eCommandButton_Click, cmdBrowModelFileTest_Click);
	form1.EventAdd(ID_cmdBrowResultFileTest, eCommandButton_Click, cmdBrowResultFileTest_Click);
	form1.EventAdd(ID_cmdBrowScaleParamFileTrain, eCommandButton_Click, cmdBrowScaleParamFileTrain_Click);
	form1.EventAdd(ID_chkScaleTrain, eCommandButton_Click, chkScaleTrain_Click);
	form1.EventAdd(ID_chkScaleTest, eCommandButton_Click, chkScaleTest_Click);

	//支持5个组合框文件拖放:都关联到同一事件函数: 
	form1.EventAdd(ID_cboDataFile, eFilesDrop, cbo_FilesDrop);
	form1.EventAdd(ID_cboScaleParamFileTrain, eFilesDrop, cbo_FilesDrop);
	form1.EventAdd(ID_cboModelFileTest, eFilesDrop, cbo_FilesDrop);
	form1.EventAdd(ID_cboResultFileTest,eFilesDrop,cbo_FilesDrop);
	form1.EventAdd(ID_cboScaleParamFileTest, eFilesDrop, cbo_FilesDrop);

	// ID_cmdTrain和 ID_cmdGenModel 两个按钮都关联Train_Click
	form1.EventAdd(ID_cmdTrain, eCommandButton_Click, Train_Click);
	form1.EventAdd(ID_cmdGenModel, eCommandButton_Click, Train_Click);
	form1.EventAdd(ID_cmdPredict, eCommandButton_Click, cmdPredict_Click);

	form1.IconSet(IDI_ICON1);

	form1.Show();

	return 0;
}