#include "resource.h"
#include "BForm.h"
#include "mdlOpenSaveDlg.h"	//used to open/save dialog
#include "mdlFileSys.h"		//used to realize the operations for file
#include "mdlPathDlg.h"
#include "mdlShellExec.h"
#include "GLCM.h"	
#include "mdlSvmRun.h"
#include "BADO.h"

CBForm frmBatch(ID_frmBatch);
void EventsMapfrmBatch();
bool m_fCancelSign = false;

//更新ID_cboPosi ID_cboNega
void UpdateClassList()
{
	CBAdoRecordset rs;
	if (!rs.Open(TEXT("SELECT DISTINCT ClassLabel FROM Samples")))
		return;
	frmBatch.Control(ID_cboPosi).ListClear();
	frmBatch.Control(ID_cboNega).ListClear();
	while (!rs.EOFRs())
	{
		frmBatch.Control(ID_cboPosi).AddItem(rs.GetField(TEXT("ClassLabel")));
		frmBatch.Control(ID_cboNega).AddItem(rs.GetField(TEXT("ClassLabel")));
		rs.MoveNext();
	}
	rs.Close();
}

void cmdBrowImgPath_Click()
{
	TCHAR* path = BrowPath(frmBatch.hWnd(), TEXT("请选择一类图像文件所在的文件夹"), 
							true, frmBatch.Control(ID_cboImgPath).Text(), false, true);

	if (*path)
	{
		frmBatch.Control(ID_cboImgPath).TextSet(path);
		//自动填充ID_txtClassLabel
		TCHAR** s;
		int n = Split(path, s, TEXT("\\"));
		frmBatch.Control(ID_txtClassLabel).TextSet(s[n-1]);
	}
		
}

void cmdBrowDataFile_Click()
{
	//浏览选择data.txt时要保存到的位置和文件名
	OsdSetFilter(TEXT("文本文件(*.txt)|*.txt"));
	TCHAR* file = OsdSaveDlg(frmBatch.hWnd(),0,TEXT("请指定要保存的特征文件")); //不是 OsdOpenDlgif(*file)
	if (*file)
		frmBatch.Control(ID_cboDataFile).TextSet(file);
}

void cmdCalcBatch_Click()
{
	//一样的套路，点击该按钮“批量计算GLCM特征值”开始批量计算后，按钮变为“取消”可以终止计算
	/*按钮为“取消”是对应的功能*/
	if (_tcscmp(frmBatch.Control(ID_cmdCalcBatch).Text(), TEXT("取消")) == 0)
	{
		if (MsgBox(TEXT("计算尚未完成，确定要终止吗？"), TEXT("终止计算"), mb_YesNoCancel, mb_IconQuestion) != idYes)
			return;
		m_fCancelSign = true;//全局变量，是否中途取消的标志
		return;
	}

	/*按钮为“批量计算GLCM特征值”是对应的功能：
	计算ID_cboImgPath路径下的所有图像文件，以ID_txtClassLabel为标签，将计算结果存入access的Features表中
	*/
	TCHAR* szPath = frmBatch.Control(ID_cboImgPath).Text();//获得需要批处理的图像文件所在的路径
	TCHAR* szLabel = frmBatch.Control(ID_txtClassLabel).Text();//获得此批图像文件的类别标签
	if (*szPath == TEXT('\0'))
	{
		MsgBox(TEXT("请选择图像文件所在文件夹！"), TEXT("未选择文件夹"), mb_OK, mb_IconExclamation);
		return;
	}
	if (*szLabel == TEXT('\0'))
	{
		MsgBox(TEXT("请选择这批图像文件的类别标签！"), TEXT("未输入类别标签"), mb_OK, mb_IconExclamation);
		return;

	}

	//若数据库中已经有了改类别标签，则要先删除数据库中的这批数据
	CBAdoRecordset rs, rsSample;
	tstring sSQL, sPrompt;
	sSQL = TEXT("SELECT * FROM Features, Samples WHERE Features.SampleID=Samples.SampleID AND ClassLabel='");
	sSQL += szLabel;
	sSQL += TEXT("'");
	if (!rs.Open(sSQL, ADOConn))
		return;
	if (!rs.EOFRs())
	{
		//删除已有的重复标签
		sPrompt = TEXT("数据库中已存在类别标签 [");
		sPrompt += szLabel;
		sPrompt += TEXT("]。\r\n");
		sPrompt += TEXT("再次计算同名类别标签的图像文件的特征，会导致数据库中已有的属于该类别的数据被删除，是否继续？");
		if (MsgBox(sPrompt, TEXT("删除已有的重复标签"), mb_YesNo, mb_IconQuestion) == idNo)
			return;
		sSQL = TEXT("DELETE FROM Features WHERE SampleID IN (SELECT SampleID FROM Samples WHERE ClassLabel='");
		sSQL += szLabel;
		sSQL += TEXT("')");
		ADOConn.Execute(sSQL.c_str());
		sSQL = TEXT("DELETE FROM Samples WHERE ClassLabel='");
		sSQL += szLabel;
		sSQL += TEXT("'");
		ADOConn.Execute(sSQL.c_str());
	}
	rs.Close();

	//获得Samples表中最大的SampleID，在此基础上+1获得新的ID
	int idSample = 0;
	if (!rs.Open(TEXT("SELECT MAX(SampleID) FROM Samples"), ADOConn))
		return;
	if (rs.EOFRs())
		return;
	idSample = (int)Val(rs.GetField((long)0));
	rs.Close();

	//这里也可以往后面方方
	if (!rs.Open(TEXT("SELECT * FROM Features"), ADOConn))
		return;
	if (!rsSample.Open(TEXT("SELECT * FROM Samples"), ADOConn))
		return;

	m_fCancelSign = false;
	frmBatch.Control(ID_cmdCalcBatch).TextSet(TEXT("取消"));

	/*计算所有图像文件的GLCM特征值*/
	//首先要读取路径下所有的图像文件
	TCHAR** files = NULL, ** subFolders = NULL;
	int cntFiles = 0, cntSnbFolders = 0;
	CGLCM glcm;
	FMListFilesAPI(szPath, files, subFolders, &cntFiles, &cntSnbFolders);
	//准备进度条 
	frmBatch.Control(ID_proBatch).MinSet(0);
	frmBatch.Control(ID_proBatch).MaxSet(cntFiles);
	frmBatch.Control(ID_proBatch).ValueSet(0);
	frmBatch.Control(ID_proBatch).VisibleSet(true);
	//计算并存储
	for (int i = 1;i <= cntFiles;i++)//注意索引0弃之不用
	{
		frmBatch.Control(ID_lblStatusBatch).TextSet(TEXT("正在计算"));
		frmBatch.Control(ID_lblStatusBatch).TextAdd(FMTrimFileName(files[i]));
		frmBatch.Control(ID_lblStatusBatch).TextAdd(TEXT("..."));
		frmBatch.Control(ID_proBatch).ValueSet(i);
		DoEvents();
		if (m_fCancelSign)
			break;
		if (!glcm.SetBitMapFile(files[i]))
		{
			MsgBox(glcm.ErrDesp(), FMTrimFileName(files[i]), mb_OK, mb_IconExclamation);
			break;
		}
		else//成功打开
		{
			if (rsSample.AddNew())
			{
				idSample++;
				rsSample.SetField(TEXT("SampleID"), idSample);
				rsSample.SetField(TEXT("ClassLabel"), szLabel);
				rsSample.SetField(TEXT("ImageFile"), files[i]);
				if (!rsSample.Update())
				{
					MsgBox(rsSample.ErrorLastStr(), TEXT("向Sample表中新增记录时更新失败"), mb_OK, mb_IconExclamation);
					break;
				}
			}
			else
			{
				MsgBox(rsSample.ErrorLastStr(), TEXT("向Sample表中新增记录失败"), mb_OK, mb_IconExclamation);
				break;
			}
			for (int j = 1;j <= glcm.GetGLCMFeatCount();j++)
			{
				if (rs.AddNew())
				{
					rs.SetField(TEXT("SampleID"), idSample);
					rs.SetField(TEXT("FeatureID"), j);
					rs.SetField(TEXT("FeatureValue"), glcm.GetGLCMFeat((EGLCMFeatureType)j));
					if (!rs.Update())
					{
						MsgBox(rs.ErrorLastStr(), TEXT("向Features表中新增记录时更新失败"), mb_OK, mb_IconExclamation);
						break;
					}
				}
				else
				{
					MsgBox(rs.ErrorLastStr(), TEXT("向Features表中新增记录失败"), mb_OK, mb_IconExclamation);
					break;
				}
			}
		}
	}

	rsSample.Close();
	rs.Close();
	frmBatch.Control(ID_proBatch).VisibleSet(false);
	UpdateClassList();//更新ID_cboPosi ID_cboNega
	frmBatch.Control(ID_cmdCalcBatch).TextSet(TEXT("批量计算GLCM特征值"));
	if (m_fCancelSign)
	{
		frmBatch.Control(ID_lblStatusBatch).TextSet(TEXT("用户终止"));
	}
	else
	{
		frmBatch.Control(ID_lblStatusBatch).TextSet(TEXT("完成。"));
	}
}


void frmBatch_Load()
{
	//美化ID_lblVS
	frmBatch.Control(ID_lblVS).FontNameSet(TEXT("黑体"));
	frmBatch.Control(ID_lblVS).FontSizeSet(30);
	frmBatch.Control(ID_lblVS).ForeColorSet(RGB(255, 0, 0));
	frmBatch.Control(ID_lblVS).FontItalicSet(true);

	//向ID_cboKernels中添加选项
	frmBatch.Control(ID_cboKernels).AddItem(TEXT("线性核(Linear，t=0)"));
	frmBatch.Control(ID_cboKernels).AddItem(TEXT("多项式核(Polynomial，t=1)"));
	frmBatch.Control(ID_cboKernels).AddItem(TEXT("高斯径向基核(Radial Basis Function，t=2)"));
	frmBatch.Control(ID_cboKernels).AddItem(TEXT("二层神经网络核(Sigmoid,t=3)"));
	frmBatch.Control(ID_cboKernels).ListIndexSet(3);// 默认选择高斯径向基核

	//lbl不显示Static
	frmBatch.Control(ID_lblStatusBatch).TextSet(TEXT(" "));
	frmBatch.Control(ID_lblStatusSVM).TextSet(TEXT(" "));

	//向ID_cboFeatSelect中添加选项
	frmBatch.Control(ID_cboFeatSelect).AddItem(TEXT("能量(ASM)"));
	frmBatch.Control(ID_cboFeatSelect).AddItem(TEXT("对比度(Constrast)"));
	frmBatch.Control(ID_cboFeatSelect).AddItem(TEXT("相关性(Correlation)"));
	frmBatch.Control(ID_cboFeatSelect).AddItem(TEXT("方差(Variance)"));
	frmBatch.Control(ID_cboFeatSelect).AddItem(TEXT("同质度(Homogeneity)"));
	frmBatch.Control(ID_cboFeatSelect).AddItem(TEXT("和均值(SumAverage)"));
	frmBatch.Control(ID_cboFeatSelect).AddItem(TEXT("和方差(SumVar)"));
	frmBatch.Control(ID_cboFeatSelect).AddItem(TEXT("和熵(SumEntropy)"));
	frmBatch.Control(ID_cboFeatSelect).AddItem(TEXT("熵(Entropy)"));
	frmBatch.Control(ID_cboFeatSelect).AddItem(TEXT("差方差(DiffVar)"));
	frmBatch.Control(ID_cboFeatSelect).AddItem(TEXT("差熵(DiffEntropy)"));
	frmBatch.Control(ID_cboFeatSelect).AddItem(TEXT("非相似性(Dissimilarity)"));
	frmBatch.Control(ID_cboFeatSelect).AddItem(TEXT("均值(Mean)"));
	frmBatch.Control(ID_cboFeatSelect).AddItem(TEXT("三阶聚集性(ClusterShade)"));
	frmBatch.Control(ID_cboFeatSelect).AddItem(TEXT("四阶聚集性(ClusterProm)"));
	frmBatch.Control(ID_cboFeatSelect).AddItem(TEXT("最大概率(MaxProb)"));
	frmBatch.Control(ID_cboFeatSelect).AddItem(TEXT("最小概率(MinProb)"));
	frmBatch.Control(ID_cboFeatSelect).AddItem(TEXT("强度(Strength)"));
	frmBatch.Control(ID_cboFeatSelect).AddItem(TEXT("MassX"));
	frmBatch.Control(ID_cboFeatSelect).AddItem(TEXT("MassY"));
	frmBatch.Control(ID_cboFeatSelect).ListIndexSet(1);
	
	frmBatch.Control(ID_txtFeatSelect).TextSet(TEXT("Contrast SumAverage MassX MassY "));

	//向ID_cboFold中添加选项
	for (int i = 2; i <= 10; i++)
		frmBatch.Control(ID_cboFold).AddItem(Str(i));
	frmBatch.Control(ID_cboFold).TextSet(TEXT("5"));// 默认选择:5-折交叉验证

	frmBatch.Control(ID_proBatch).VisibleSet(false);//隐藏ID_proBatch
	VerifyLibSVMFiles();//验证LibSVM.exe是否存在
	UpdateClassList();//更新ID_cboPosi ID_cboNega
}


//向文件中以SVM数据文件格式写入一批样本特征值（正样本或负样本)，数据流向：access->txt
//idCboControl控件ID
//iPosiNega==1 写入正样本; 写入负样本iPosiNega==-1
bool OutputDataFile(HANDLE hFile, unsigned int idCboControl, int iPosiNega)
{
	CBAdoRecordset rsSam, rs, rsUseFeatures;
	tstring sSQL;
	bool blRet = false;
	long lRet = 0;
	int iFeatureIndex = 0;//写入文件时的特征编号
	//获得所有样本的 SampleID => rsSam
	if (iPosiNega > 0)
	{
		frmBatch.Control(ID_lblStatusSVM).TextSet(TEXT("获得阳性样本..."));
	}
	else
	{
		frmBatch.Control(ID_lblStatusSVM).TextSet(TEXT("获得阴性样本..."));
	}
	sSQL = TEXT("SELECT * FROM Samples WHERE ClassLabel='");
	sSQL += frmBatch.Control(idCboControl).Text();
	sSQL += TEXT("'ORDER BY SampleID");
	
	if (!rsSam.Open(sSQL, ADOConn))
		return false;

	//显示提示信息，正在写入UseFeature table
	frmBatch.Control(ID_lblStatusSVM).TextSet(TEXT("正在写入UseFeature Table"));
	//根据ID_txtFeatSelect确定UseFeature并写入数据库
	TCHAR** sUseFeatures;
	int cntTotalUseFeat = Split(frmBatch.Control(ID_txtFeatSelect).Text(), sUseFeatures, TEXT(" "));
	if (!rsUseFeatures.Open(TEXT("SELECT * FROM UseFeatures"), ADOConn))
		return false;
	sSQL = TEXT("DELETE FROM UseFeatures");
	ADOConn.Execute(sSQL.c_str());
	int useFeatureID = 1;
	for (int i = 1;i < cntTotalUseFeat;i++)
	{
		if (rsUseFeatures.AddNew())
		{
			
			if (!_tcscmp(sUseFeatures[i], TEXT("ASM")))
				useFeatureID = 1;
			if (!_tcscmp(sUseFeatures[i], TEXT("Contrast")))
				useFeatureID = 2;
			if (!_tcscmp(sUseFeatures[i], TEXT("Correlation")))
				useFeatureID = 3;
			if (!_tcscmp(sUseFeatures[i], TEXT("Variance")))
				useFeatureID = 4;
			if (!_tcscmp(sUseFeatures[i], TEXT("Homogeneity")))
				useFeatureID = 5;
			if (!_tcscmp(sUseFeatures[i], TEXT("SumAverage")))
				useFeatureID = 6;
			if (!_tcscmp(sUseFeatures[i], TEXT("SumVar")))
				useFeatureID = 7;
			if (!_tcscmp(sUseFeatures[i], TEXT("SumEntropy")))
				useFeatureID = 8;
			if (!_tcscmp(sUseFeatures[i], TEXT("Entropy")))
				useFeatureID = 9;
			if (!_tcscmp(sUseFeatures[i], TEXT("DiffVar")))
				useFeatureID = 10;
			if (!_tcscmp(sUseFeatures[i], TEXT("DiffEntropy")))
				useFeatureID = 11;
			if (!_tcscmp(sUseFeatures[i], TEXT("Dissimilarity")))
				useFeatureID = 12;
			if (!_tcscmp(sUseFeatures[i], TEXT("Mean")))
				useFeatureID = 13;
			if (!_tcscmp(sUseFeatures[i], TEXT("ClusterShade")))
				useFeatureID = 14;
			if (!_tcscmp(sUseFeatures[i], TEXT("ClusterProm")))
				useFeatureID = 15;
			if (!_tcscmp(sUseFeatures[i], TEXT("MaxProb")))
				useFeatureID = 16;
			if (!_tcscmp(sUseFeatures[i], TEXT("MinProb")))
				useFeatureID = 17;
			if (!_tcscmp(sUseFeatures[i], TEXT("Strength")))
				useFeatureID = 18;
			if (!_tcscmp(sUseFeatures[i], TEXT("MassX")))
				useFeatureID = 19;
			if (!_tcscmp(sUseFeatures[i], TEXT("MassY")))
				useFeatureID = 20;
			rsUseFeatures.SetField(TEXT("FeatureID"), useFeatureID);
			if (!rsUseFeatures.Update())
			{
				MsgBox(rsUseFeatures.ErrorLastStr(), TEXT("向Sample表中新增记录时更新失败"), mb_OK, mb_IconExclamation);
				break;
			}
		}
		else
		{
			MsgBox(rsUseFeatures.ErrorLastStr(), TEXT("向UseFeatures表中新增记录失败"), mb_OK, mb_IconExclamation);
			break;
		}
	}
	rsUseFeatures.Close();
	

	//对每个ClassLabel，获得其部分Features(选哪几个Features要根据UseFeatures表)
	while (!rsSam.EOFRs())
	{

		//显示提示信息，样本显示样本ID+样本文件名（去除路径部分)
		frmBatch.Control(ID_lblStatusSVM).TextSet(TEXT("写入样本特征值"));
		frmBatch.Control(ID_lblStatusSVM).TextAdd(rsSam.GetField(TEXT("SampleID")));
		frmBatch.Control(ID_lblStatusSVM).TextAdd(TEXT(" - "));
		frmBatch.Control(ID_lblStatusSVM).TextAdd(FMTrimFileName(rsSam.GetField(TEXT("ImageFile"))));


		// EF_LineSeed_None表示写入此内容后，不自动换行
		if (EFPrint(hFile, Str(iPosiNega), EF_LineSeed_None) < 0)//写标签:正样本为1，负样本为-1
			return false;// 写入失败
		if (EFPrint(hFile, TEXT(" "), EF_LineSeed_None) < 0)//写空格
			return false;//写入失败
		//获得要写入文件的特征值
		sSQL = TEXT("SELECT * FROM Features, UseFeatures WHERE ");
		sSQL += TEXT("Features.FeatureID = UseFeatures.FeatureID AND ");
		sSQL += TEXT("SampleID=");
		sSQL += rsSam.GetField(TEXT("SampleID"));
		sSQL += TEXT(" ORDER BY Features.FeatureID");
		if (!rs.Open(sSQL, ADOConn))
			return false;

		//循环所有特征值,以SVM的特征数据文件格式写入文件
		iFeatureIndex = 0;
		while (!rs.EOFRs())
		{
			iFeatureIndex++;
			//format-iFeatureIndex:FeatureValue[blank space]
			if (EFPrint(hFile, Str(iFeatureIndex), EF_LineSeed_None) < 0)//写FeatureID
				return false;// 写入失败
			if (EFPrint(hFile, TEXT(":"), EF_LineSeed_None) < 0)//写:
				return false;// 写入失败
			if (EFPrint(hFile, rs.GetField(TEXT("FeatureValue")), EF_LineSeed_None) < 0)//写FeatureValue
				return false;// 写入失败
			if (EFPrint(hFile, TEXT(" "), EF_LineSeed_None) < 0)//写空格
				return false;// 写入失败
			rs.MoveNext();
		}
		if (EFPrint(hFile, TEXT(""), EF_LineSeed_CrLf) < 0)//换行
			return false;// 写入失败
		rsSam.MoveNext();
	}
	rsSam.Close();
	if (iPosiNega > 0)
	{
		frmBatch.Control(ID_lblStatusSVM).TextSet(TEXT("阳性样本写入完成。"));
	}
	else
	{
		frmBatch.Control(ID_lblStatusSVM).TextSet(TEXT("阴性样本写入完成。"));
	}
	return true;
}

//生成LibSVM特征数据文件
//szFile 文件名
bool GenSVMDataFile(TCHAR* szFile)
{
	if (*szFile == TEXT('\0'))
	{
		MsgBox(TEXT("请指定特征文件的文件名和保存路径"), TEXT("未指定特征文件"), mb_OK, mb_IconExclamation);
		return false;
	}
	TCHAR* szClassLabel = frmBatch.Control(ID_cboPosi).Text();
	if (*szClassLabel == TEXT('\0'))
	{
		MsgBox(TEXT("请选择阳性样本的类别标签"), TEXT("未选择阳性样本的类别标签"), mb_OK, mb_IconExclamation);
		return false;
	}
	szClassLabel = frmBatch.Control(ID_cboNega).Text();
	if (*szClassLabel == TEXT('\0'))
	{
		MsgBox(TEXT("请选择阴性样本的类别标签"), TEXT("未选择阴性样本的类别标签"), mb_OK, mb_IconExclamation);
		return false;
	}

	HANDLE hFile = EFOpen(szFile, EF_OpStyle_Output);
	if (hFile == INVALID_HANDLE_VALUE)
		return false;
	//调用OutputDataFile()写入access样本到txt
	if (!OutputDataFile(hFile, ID_cboPosi, 1))
	{
		MsgBox(TEXT("写入阳性样本特征值到文件失败！"), TEXT("写入阳性样本失败"), mb_OK, mb_IconExclamation);
		return false;
	}
	if (!OutputDataFile(hFile, ID_cboNega, -1))
	{
		MsgBox(TEXT("写入阴性样本特征值到文件失败！"), TEXT("写入阴性样本失败"), mb_OK, mb_IconExclamation);
		return false;
	}
	EFClose(hFile);
	return true;
}

//按ID_cmdTrain和ID_cmdGenModel是的回调函数，其功能有：生成SVM特征文件，生成model文件(可选)
void Train_Click()
{
	TCHAR* szFileModel = NULL;
	if (frmBatch.IDRaisingEvent() == ID_cmdGenModel)
	{
		//浏览选择model要保存到的路径和文件名
		OsdSetFilter(TEXT("模型文件(*.model)|*.model|文本文件(*.txt)|*.txt"));
		szFileModel = OsdSaveDlg(frmBatch.hWnd());
		if (*szFileModel == 0)
			return;
	}

	TCHAR* szFile = frmBatch.Control(ID_cboDataFile).Text();
	if (!GenSVMDataFile(szFile))
		return;

	//自动确定缩放参数文件：文件名中加sacale.param，路径+文件名 szFileScaleParam
	TCHAR* szPath = NULL;//szFileScaleParam中的路径部分
	TCHAR* szExp = NULL;//szFileScaleParam中的扩展名部分
	// 将文件名全名（含路径）中的路径部分、扩展名部分都除去，只返回主名部分
	TCHAR* szFileNameOnly = FMTrimFileName(szFile, true, true, &szPath, &szExp);
	TCHAR* szFileScaleParam = StrAppend(szPath, TEXT("\\"), szFileNameOnly, TEXT(".scale.param."), szExp);
	//删除已经存在的文件
	FMDeleteFiles(szFileScaleParam, true, true, false);
	if (FMFileExist(szFileScaleParam) == 1)
	{
		MsgBox(ID_cboFold, szFileScaleParam, mb_OK, mb_IconExclamation);
		return;
	}
	tstring sReturn;
	frmBatch.Control(ID_lblStatusSVM).TextSet(TEXT("SVM正在训练..."));
	bool ret = SVMTrain(szFile, szFileModel, szFileScaleParam,
		frmBatch.Control(ID_cboKernels).ListIndex() - 1,
		frmBatch.Control(ID_cboFold).TextInt(),
		frmBatch.Control(ID_txtParams).Text(), sReturn);
	if (!ret)
	{
		frmBatch.Control(ID_lblStatusSVM).TextSet(TEXT("训练失败。"));
		MsgBox(sReturn, TEXT("训练失败"), mb_OK, mb_IconExclamation);
	}
	else
	{
		frmBatch.Control(ID_lblStatusSVM).TextSet(TEXT("训练成功。"));
		if (frmBatch.IDRaisingEvent() == ID_cmdGenModel)
		{
			sReturn = TEXT("model 文件生成成功:\r\n") + sReturn;
			MsgBox(sReturn, TEXT("训练失败"), mb_OK, mb_IconExclamation);
		}
		else
		{
			tstring sCmd;
			sCmd = TEXT("NotePad.exe \"") + sReturn + TEXT(" \"");
			SEShellRun(sCmd.c_str(), true);
		}
	}
}

void cmdFeatAdd_Click()
{
	int idxFeatToAdd = frmBatch.Control(ID_cboFeatSelect).ListIndex();
	CGLCM glcm_temp;
	TCHAR* szFeatToAdd = glcm_temp.GetGLCMFeatName(EGLCMFeatureType(idxFeatToAdd));
	TCHAR* szFeatAdded = frmBatch.Control(ID_txtFeatSelect).Text();
	//检查是否有特征值重复添加
	TCHAR* pFind = _tcsstr(szFeatAdded, szFeatToAdd);
	if (pFind != NULL)
	{
		MsgBox(TEXT("同一个特征值只能添加一次！"), TEXT("添加特征值失败"), mb_OK, mb_IconExclamation);
		return;
	}
	frmBatch.Control(ID_txtFeatSelect).TextAdd(szFeatToAdd);
	frmBatch.Control(ID_txtFeatSelect).TextAdd(TEXT(" "));
}

void cmdFeatDel_Click()
{
	int idxFeatToDel = frmBatch.Control(ID_cboFeatSelect).ListIndex();
	CGLCM glcm_temp;
	TCHAR* szFeatToDel = glcm_temp.GetGLCMFeatName(EGLCMFeatureType(idxFeatToDel));//待删除字串
	
	TCHAR* szFeatAdded = frmBatch.Control(ID_txtFeatSelect).Text();//原字符串
	TCHAR* szFeatChanged = NULL;//删除字串后的字符串
	//检查是否待删除特征值是否已经添加
	TCHAR* pFind = _tcsstr(szFeatAdded, szFeatToDel);
	if (pFind == NULL)
	{
		MsgBox(TEXT("该特征值未添加，故无法删除"), TEXT("删除特征值失败"), mb_OK, mb_IconExclamation);
		return;
	}
	else {
		//删除字符串的操作，因为同一特征值不能重复添加，所以只要删除一个即可
		szFeatAdded[(int)(pFind - szFeatAdded)] = TEXT('\0');
		pFind = pFind + _tcslen(szFeatToDel) + 1;
		if (pFind != NULL)
			_tcscat(szFeatAdded, pFind);
	}
	frmBatch.Control(ID_txtFeatSelect).TextSet(szFeatAdded);
}

void cmdFeatDefault_Click()
{
	frmBatch.Control(ID_txtFeatSelect).TextSet(TEXT("Contrast SumAverage MassX MassY "));
}

void EventsMapfrmBatch()
{
	frmBatch.EventAdd(0, eForm_Load, frmBatch_Load);
	frmBatch.EventAdd(ID_cmdBrowImgPath, eCommandButton_Click, cmdBrowImgPath_Click);
	frmBatch.EventAdd(ID_cmdBrowDataFile, eCommandButton_Click, cmdBrowDataFile_Click);
	frmBatch.EventAdd(ID_cmdCalcBatch, eCommandButton_Click, cmdCalcBatch_Click);

	frmBatch.EventAdd(ID_cmdTrain, eCommandButton_Click, Train_Click);
	frmBatch.EventAdd(ID_cmdGenModel, eCommandButton_Click, Train_Click);

	frmBatch.EventAdd(ID_cmdFeatAdd, eCommandButton_Click, cmdFeatAdd_Click);
	frmBatch.EventAdd(ID_cmdFeatDel, eCommandButton_Click, cmdFeatDel_Click);
	frmBatch.EventAdd(ID_cmdFeatDefault, eCommandButton_Click, cmdFeatDefault_Click);
}