#include "mdlSvmRun.h"
#include "mdlFileSys.h"
#include "mdlShellExec.h"

LPCTSTR mc_szScaleExeFile = TEXT("svm-scale.exe");
LPCTSTR mc_szTrainExeFile = TEXT("svm-train.exe");
LPCTSTR mc_szTestExeFile = TEXT("svm-predict.exe");

//测试文件sFile是否存在，
//如果存在则返回true，
//若不存在，则返回false并提出错误信息szPromptIfNotExist
bool TestFileExist(tstring& sFile, TCHAR* szPromptIfNotExist)
{
	if (FMFileExist(sFile.c_str()) == 1)
		return true;
	else
	{
		tstring sPrompt = sFile + TEXT("\r\n") + szPromptIfNotExist;
		MsgBox(sPrompt, TEXT("文件不存在"), mb_OK, mb_IconExclamation);
		return false;
	}
}

//验证LibSVM.exe是否存在
//如果存在则返回true，
//若不存在，则返回false并提出错误信息szPromptIfNotExist
bool VerifyLibSVMFiles()
{
	//pApp->Path():获得应用程序当前路径
	//FMAddBackSlash(): 确保最后有一个 '\\'。
	tstring sAppPath = FMAddBackSlash(pApp->Path());
	if (!TestFileExist(sAppPath + mc_szScaleExeFile, TEXT("svm-scale.exe未拷贝至当前程序运行文件夹中")))
		return false;
	if (!TestFileExist(sAppPath + mc_szTrainExeFile, TEXT("svm-train.exe未拷贝至当前程序运行文件夹中")))
		return false;
	if (!TestFileExist(sAppPath + mc_szTestExeFile, TEXT("svm-predict.exe未拷贝至当前程序运行文件夹中")))
		return false;

	return true;
}

//创建批处理文件并自动执行
// szPath批处理文件路径
// szBatFileName批处理文件名称(包括.bat)
// sCmdLine批处理文件执行命令
//成功创建并成功运行则返回true，反之false
bool RunBat(TCHAR* szPath, TCHAR* szBatFileName, tstring& sCmdLine)
{
	//构造批处理文件(全路径\\文件名)
	tstring sBatFile;
	sBatFile = FMAddBackSlash(szPath);
	sBatFile = sBatFile + szBatFileName;
	//向批处理文件中写入命令行  EF_OpStyle_Output意思是 仅覆盖写
	HANDLE hFileBat = EFOpen(sBatFile.c_str(), EF_OpStyle_Output);
	if (hFileBat == INVALID_HANDLE_VALUE)
		return false;

	EFPrint(hFileBat, sCmdLine.c_str());//写入命令行
	EFClose(hFileBat);//关闭批处理文件
	//控制控制台显示状态：显示控制台
	SERunWaitTerminate(sBatFile.c_str(), true, SW_ShowNormal);
	//删除批处理文件
	//运行会发现能在回收站找到删除的.bat文件（这和第二个参数设置为true有关）
	//可以去回收站瞅瞅.bat文件里面是啥，也可以直接再此处添加断点在.bat尚未被删除时查看
	FMDeleteFiles(sBatFile.c_str(), true, true, false);

	return true;
}

// 将szFileData归一化，并将文件名保存在sRetString中
// szFileDataScaleParam为归一化文件，如果szFileDataScaleParam=NULL，则不归一化
// 如果归一化，则fParamReadOrWrite=true时，读参数文件，否则写入新的参数文件并覆盖之前内容
// 成功创建并成功运行则返回true，反之false
bool SVMScaleData(	TCHAR* szFileData,
					TCHAR* szFileDataScaleParam,
					bool fParamReadOrWrite,
					tstring& sRetString)
{
	//通式：路径+文件名即可实现访问
	tstring sAppPath = FMAddBackSlash(pApp->Path());
	tstring sSVMExeScale = sAppPath + mc_szScaleExeFile;//svm-scale.exe 全路径+文件名

	TCHAR* szPath = NULL;//szDataFile中的路径部分
	TCHAR* szExp = NULL;//szDataFile中的扩展名部分
	// 将文件名全名（含路径）中的路径部分、扩展名部分都除去，只返回主名部分
	TCHAR* szFileNameOnly = FMTrimFileName(szFileData, true, true, &szPath, &szExp);
	tstring sFileDataScale;
	tstring sCmdLine;

	//构造将要生成的归一化后的数据文件的全路径与文件名
	//当szFileDataScaleParam != NULL时则需要归一化
	if (szFileDataScaleParam)
	{
		//调用svm-scale.exe 
		//构造归一化后的数据文件的名称eg. 路径\\data.txt  -> 路径\\data.scale.txt
		sFileDataScale = szPath;
		sFileDataScale += TEXT("\\");
		sFileDataScale += szFileNameOnly;
		sFileDataScale += TEXT(".scale.");
		sFileDataScale += szExp;
		//如果归一化后的数据文件 和 归一化参数文件名称相同，则改名 路径\\data.scale.txt -> 路径\\data.scale.data.txt
		if (_tcscmp(sFileDataScale.c_str(), szFileDataScaleParam) == 0)
		{
			sFileDataScale = szPath;
			sFileDataScale += TEXT("\\");
			sFileDataScale += szFileNameOnly;
			sFileDataScale += TEXT(".scale.data.");
			sFileDataScale += szExp;
		}

		//如果sFileDataScale已经存在则删除，并进行是否删除成功的检查
		FMDeleteFiles(sFileDataScale.c_str(), true, true, false);
		if (FMFileExist(sFileDataScale.c_str()) == 1)
		{
			sRetString = TEXT("不能删除之前残留的归一化数据结果文件: ") + sFileDataScale;
			return false;
		}

		//准备归一化参数保存到文件夹中
		if (!fParamReadOrWrite)//If fParamReadOrWrite==false, writer the param(overwrite the previous content)
		{
			FMDeleteFiles(szFileDataScaleParam, true, true, false);
			if (FMFileExist(szFileDataScaleParam) == 1)
			{
				sRetString = TEXT("不能删除之前残留的归一化参数文件:") + sFileDataScale;//错误说明
				return false;
			}
			//生成命令行 svm-scale.exe  -s
			//将data.txt文件中的数据归一化后生成结果文件data.scale.txt，其中归一化所用的参数保存进文件scaleparams.txt中
			sCmdLine = TEXT("\"") + sSVMExeScale + TEXT("\" -s ");
		}
		else//If fParamReadOrWrite == true, read the param file,
		{
			//生成命令行 svm-scale.exe  -r
			//将data.txt文件中的数据归一化后生成结果文件data.scale.txt，其中归一化所用的参数使用之前生成的文件scaleparams.txt
			sCmdLine = TEXT("\"") + sSVMExeScale + TEXT("\" -r ");
		}
		//继续生产归一化的命令行: svm-scale.exe  -s  scaleparams.txt  data.txt > data.scale.txt
		//要将data.txt文件中的数据归一化后生成结果文件data.scale.txt，其中归一化所用的参数保存进文件scaleparams.txt中
		sCmdLine = sCmdLine + TEXT("\"") + szFileDataScaleParam + TEXT("\" ");
		sCmdLine = sCmdLine + TEXT("\"") + szFileData + TEXT("\" ");
		sCmdLine = sCmdLine + TEXT("> \"") + sFileDataScale + TEXT("\" ");

		//通过批处理文件运行此命令，并等待运行结束
		if (!RunBat(szPath, TEXT("svmscale.bat"), sCmdLine))
		{
			sRetString = TEXT("数据归一化失败！无法在文件夹");
			sRetString = sRetString + szPath + TEXT(" 中创建或运行批处理文件:svmscale.bat。");//错误说明
			return false;
		}
		if (FMFileExist(sFileDataScale.c_str()) != 1)
		{
			sRetString = TEXT("数据归一化失败！未能生成归一化后的结果数据文件：") + sFileDataScale;//错误说明
			return false;
		}
	}
	else // normalization wouldn't be operated if szFileDataScaleParam==NULL
	{
		//直接设置sFileDataScale为原数据文件
		sFileDataScale = szFileData;
	}
	sRetString = sFileDataScale;
	return true;
}

//运行SVM 训练并等待程序结束
//szFileData为特征数据文件的文件名
//szFileModel为要读取的model文件的文件名
//szFileModel != NULL，则生产model文件，此时不做n-折交叉验证，反之，做n-交叉折叠验证
//szFileDataScaleParam为要保存的归一化参数文件
//szFileDataScaleParam==NULL 时不归一化 
//iKernelFunc为要使用的核函数（=0，1,2,3)
//iFold为要几折交叉验证.(仅在szFileModel==NULL 时有效)
//szOtherParams为其他参数（命令行字符串)
//成功返回true，并由sRetString 返回结果文件名，失败返回false，并由参数sRetString返回失败原因说明
bool SVMTrain(	TCHAR* szFileData,
				TCHAR* szFileModel,
				TCHAR* szFileDataScaleParam,
				int iKernelFunc,
				int iFold,
				TCHAR* szOtherParams, 
				tstring& sRetString)
{
	// 路径\\svm-train.exe
	tstring sAppPath = FMAddBackSlash(pApp->Path());
	tstring sSVMExeTrain = sAppPath + mc_szTrainExeFile;

	if (szFileData && szFileModel && _tcscmp(szFileData, szFileModel) == 0)
	{
		sRetString = TEXT("特征数据文件和要生成的model文件不能是同一文件。");
		return false;
	}
	if (szFileData && szFileDataScaleParam && _tcscmp(szFileData, szFileDataScaleParam) == 0)
	{
		sRetString = TEXT("特征数据文件和归一化参数文件不能是同一文件。");
		return false;
	}
	if (szFileModel && szFileDataScaleParam && _tcscmp(szFileModel, szFileDataScaleParam) == 0)
	{
		sRetString = TEXT("要生成的model文件件和归一化参数文件不能是同一文件。");
		return false;
	}

	//提取szFileData的文件名
	TCHAR* szPath = NULL;// szDataFile 中的路径部分
	TCHAR* szExp = NULL; // szDataFile 中的扩展名部分
	TCHAR* szFileNameOnly = FMTrimFileName(szFileData, true, true, &szPath, &szExp);
	tstring sFileDataScale, sCmdLine;
	//归一化数据，如成功，sFileDataScale中保存了归一化后的结果数据文件名
	if (!SVMScaleData(szFileData, szFileDataScaleParam, false, sFileDataScale))
	{
		sRetString = sFileDataScale;//失败时 sFileDataScaIe 中为出错信息，传递给sRetString
		return false;
	}

	//调用SVM 训练程序测试模型
	//自动生成了结果文件data.scale.txt(t=2).training_result，其中计算出了各项统计指标
	//命令行：svm-train.exe -t 2 -v 5 data.scale.txt 
	//生成模型文件data.scale.model.txt
	//命令行：svm-train.exe -t 2      data.scale.txt data.scale.model.txt
	sCmdLine = TEXT("\"") + sSVMExeTrain + TEXT("\" ");
	sCmdLine = sCmdLine + TEXT(" -t ") + Str(iKernelFunc);
	if (szFileModel == NULL)//若不生成model文件，则执行n-折交叉验证
	{
		sCmdLine = sCmdLine + TEXT(" -v ") + Str(iFold);
	}
	sCmdLine = sCmdLine + TEXT(" ") + szOtherParams + TEXT(" ");//可扩展性
	sCmdLine = sCmdLine + TEXT(" \"") + sFileDataScale + TEXT("\" ");
	if (szFileModel != NULL)//若不生成model文件，则执行n-折交叉验证
	{
		sCmdLine = sCmdLine + TEXT(" \"") + szFileModel + TEXT("\"");
	}

	//通过批处理文件的方式，运行此命令行，并等待运行结束
	//批处理文件设为同一目录下的svmtrain.bat(文件名设置为其他名也可)
	if (!RunBat(szPath, TEXT("svmtrain.bat"), sCmdLine))
	{
		sRetString = TEXT("SVM训练失败！无法在文件夹");
		sRetString = sRetString + szPath + TEXT(" 中创建或运行批处理文件: svmtrain.bat。");
		return false;
	}

	//显示结果
	tstring sFileResult;
	if (szFileModel == NULL)//不生成model文件
	{
		//检查训练结果文件是否已生成szFileResult
		//eg.data.scale.txt(t=2).training_esult
		sFileResult = sFileDataScale + TEXT("(t=") + Str(iKernelFunc) + TEXT(").training_result");
		if (FMFileExist(sFileResult.c_str()) != 1)//检查结果文件是否已生成
		{
			sRetString = TEXT("SVM训练失败!未能生成训练结果文件:") + sFileResult;
			return false;
		}
		sRetString = sFileResult;//成功生成，sRetString保存结果文件名
	}
	else//生成model文件
	{
		//检查model文件是否已生成
		if (FMFileExist(szFileModel) != 1)
		{ 
			sRetString = TEXT("model文件生成失败!未能生成model文件: ");
			sRetString = sRetString + szFileModel;
			return false;
		}
		//成功: model文件已生成，sRetString保存model文件名
		sRetString = szFileModel ;
	}
	return true;
}

//运行SVM 测试并等待程序结束
//szFileData为特征数据文件的文件名
//szFileModel为要读取的model文件的文件名
//szFileDataScaleParam为要读取的归一化参数文件，
//需与训练时的归一化参数文件相同; szFileDataScaleParam == NULL时不归一化l 
//szFileResult为测试后要生成的结果文件
//iParamBT为 -b 参数的值选择核函数
//成功返回true，并由sRetString返回结果文件名
//失败返回false，并由参数sRetString返回失败原因说明
bool SVMPredict(TCHAR* szFileData,
				TCHAR* szFileModel,
				TCHAR* szFileDataScaleParam,
				TCHAR* szFileResult,
				int iParamB,
				tstring& sRetString)
{
	// 路径\\svm-predict.exe
	tstring sAppPath = FMAddBackSlash(pApp->Path());
	tstring sSVMExePredict = sAppPath + mc_szTestExeFile;

	//检查各文件不能相同
	if (szFileData && szFileModel && _tcscmp(szFileData, szFileModel) == 0)
	{
		sRetString = TEXT("特征数据文件和model文件不能是同一文件。");
		return false;
	}
	if (szFileData && szFileDataScaleParam && _tcscmp(szFileData, szFileDataScaleParam) == 0)
	{
		sRetString = TEXT("特征数据文件和归一化参数文件不能是同一文件。");
		return false;
	}
	if (szFileData && szFileResult && _tcscmp(szFileData, szFileResult) == 0)
	{
		sRetString = TEXT("特征数据文件和结果文件不能是同一文件。");
		return false;
	}
	if (szFileModel && szFileDataScaleParam && _tcscmp(szFileModel, szFileDataScaleParam) == 0)
	{
		sRetString = TEXT(" model文件和归一化参数文件不能是同一文件。");
		return false;
	}
	if (szFileModel && szFileResult && _tcscmp(szFileModel, szFileResult) == 0)
	{
		sRetString = TEXT(" model文件和结果文件不能是同一文件。");
		return false;
	}
	if (szFileDataScaleParam && szFileResult && _tcscmp(szFileDataScaleParam, szFileResult) == 0)
	{
		sRetString = TEXT("归一化参数文件和结果文件不能是同一文件。");
		return false;
	}

	//准备文件名
	TCHAR* szPath = NULL;// szDataFile 中的路径部分
	TCHAR* szExp = NULL; // szDataFile 中的扩展名部分
	TCHAR* szFileNameOnly = FMTrimFileName(szFileData, true, true, &szPath, &szExp);
	tstring sFileDataScale, sCmdLine;

	//归一化数据，如成功，sFileDataScale中保存了归一化后的结果数据文件名
	if (!SVMScaleData(szFileData, szFileDataScaleParam, true, sFileDataScale))
	{
		sRetString = sFileDataScale;//失败时 sFileDataScaIe 中为出错信息
		return false;
	}
	//调用SVM 测试程序测试模型
	//命令行：svm-predict -b 1/0 test.scale.txt  data.scale.model.txt  out.txt
	sCmdLine = TEXT("\"") + sSVMExePredict + TEXT("\" ");
	if (iParamB > 0)
	{ 
		//在命令行中加-b 参数选择核函数
		sCmdLine = sCmdLine + TEXT(" -b ") + Str(iParamB);
	}
	sCmdLine = sCmdLine + TEXT(" \"") + sFileDataScale + TEXT("\" ");
	sCmdLine = sCmdLine + TEXT(" \"") + szFileModel + TEXT("\" ");
	sCmdLine = sCmdLine + TEXT(" \"") + szFileResult + TEXT("\"");
	//通过批处理文件的方式，运行此命令行，并等待运行结束
	//批处理文件设为同一目录下的svmpredict.bat(文件名设置为其他名也可)
	if (!RunBat(szPath, TEXT("svmpredict.bat"), sCmdLine))
	{ 
		sRetString = TEXT("SVM测试失败！无法在文件夹");
		sRetString = sRetString + szPath + TEXT(" 中创建或运行批处理文件:svmpredict. bat。");
		return false;
	}
	//显示结果
	//检查结果文件是否已生成
	if (FMFileExist(szFileResult) != 1)
	{
		sRetString = TEXT("SVM测试失败!未能生成结果文件:");
		sRetString = sRetString + szFileResult;
		return false;
	}
	//成功:结果文件已生成，由sRetString保存结果文件名
	sRetString = szFileResult;
	return true;
}
