#include "resource.h"
#include "BForm.h"
#include "mdlOpenSaveDlg.h"
#include "BReadLinesEx.h"
#include "BDijkstra.h"
#include "BADO.h"//要位于最后

CBForm form1(ID_form1);//用创建的窗体去初始化该CBForm
bool m_fCancelSign = true;//是否中途取消的标志

void ListTaxIDs()
{
	CBAdoRecordset rs;
	tstring sItem;
	int iTaxID = 0;
	int idx = 0;

	//在VS中写SQL语句，然后在数据库中执行，再将结果返回给VS
	//taxes -> rs
	if (!rs.Open(TEXT("SELECT * FROM taxes")))
	{
		MsgBox(ADOConn.ErrorLastStr(), TEXT("获取各物种taxes失败！"), mb_OK, mb_IconExclamation);
		return;
	}

	form1.Control(ID_cboTaxIDs).ListClear();

	while (!rs.EOFRs())
	{
		//show case: Coxiella burnetii Q212(434923)
		sItem = rs.GetField(TEXT("taxID"));
		iTaxID = (int)Val(sItem.c_str());
		sItem = TEXT("(") + sItem;
		sItem = rs.GetField(TEXT("Organism")) + sItem + TEXT(")");
		idx = form1.Control(ID_cboTaxIDs).AddItem(sItem);
		form1.Control(ID_cboTaxIDs).ItemDataSet(idx, iTaxID);

		rs.MoveNext();
	}
	rs.Close();
	//设置默认选项
	if (form1.Control(ID_cboTaxIDs).ListCount() > 0)
	{
		form1.Control(ID_cboTaxIDs).ListIndexSet(488);
	}
}

void cmdBrose_Click()
{
	LPTSTR szFile;
	OsdSetFilter(TEXT("文本文件(*.txt)|*.txt|数据文件(*.dat)|*.dat"), true);
	szFile = OsdOpenDlg(form1.hWnd(), TEXT("请选择PPI数据文件"));
	if (*szFile)
	{
		form1.Control(ID_cboPPIFile).AddItem(szFile);
		form1.Control(ID_cboPPIFile).TextSet(szFile);
	}
}

LONGLONG FindPosInPPIFile(HANDLE hFile, LPCTSTR szFind)
{
	//文件是ANSI格式，将查找内容转换为ANSI格式，而szFind是Unicode，需要进行数据转换
	char* btFind = StrConvFromUnicode(szFind);
	long iLenFind = strlen(btFind);
	char buff[131072];
	bool fFound = false;
	long iBytesRead,
		 cntCycled = 0;
	LONGLONG llFileLen = EFLOF(hFile);//文件总字节数
	LONGLONG llPos = 0;//当前读写指针，0~1,069,280,000

	int i, j;

	while (llPos < llFileLen)
	{
		//从llPos开始读取131072字节
		iBytesRead = EFGetBytes(hFile, llPos, buff, sizeof(buff));//default:iShowResume = 1,提示框中有重试和取消两个按钮
		if (iBytesRead <= 0)
			break;//读取文件出错，可以写个msgbox这里

		for (i = 0;i < iBytesRead;i++)
		{
			if (buff[i] == 10 || (llPos == 0 && i == 0))//找到'\n'或者找到全文件第0个字节
			{
				if (buff[i] == 10)
					i++;

				//本批读取的字节中，第i个字节之后的内容已经不足一个szFind的长度，需要跟下一批读取的字节连接后比较
				if (iBytesRead - i < iLenFind)
				{
					llPos -= (iBytesRead - i);
					break;
				}
				for (j = 0;j < iLenFind;j++)
				{
					if (buff[i + j] != btFind[j])
						break;
				}
				if (j >= iLenFind)
				{
					fFound = true;
					break;
				}
			}
		}
		if (fFound)
			break;

		//以下为在本批buff中未找到
		llPos += iBytesRead;
		cntCycled++;
		if (cntCycled % 50 == 0)
		{
			form1.Control(ID_lblStatus).TextSet(TEXT("查找 "));
			form1.Control(ID_lblStatus).TextAdd((double)(llPos / 1024 / 1024));
			form1.Control(ID_lblStatus).TextAdd(TEXT("MB / "));
			form1.Control(ID_lblStatus).TextAdd((double)(llFileLen / 1024 / 1024));
			form1.Control(ID_lblStatus).TextAdd(TEXT("MB... "));
			DoEvents();
			if (m_fCancelSign)
			{
				form1.Control(ID_lblStatus).TextSet(TEXT("用户中断。 "));
				break;
			}
		}
	}
	return (fFound ? llPos + i : 0);
}

long ReadPPIToDB(LPCTSTR szFile, LONGLONG posStart,LPCTSTR szFind)
{
	CBAdoRecordset rs;
	if (!rs.Open(TEXT("SELECT * FROM ProtLinks")))
		return 0;
	if (!rs.EOFRs())//此时指向的不是rs的末尾，因为说明rs表中已经有内容了，需要询问是否清空
	{
		if (MsgBox(TEXT("数据库ProtLink中已有记录。若向其中添加新记录，须删除原有记录。\r\n是否删除原有记录？"),
			TEXT("清空数据库表"), mb_YesNoCancel, mb_IconExclamation) != idYes)
			return 0;
		//清空数据表
		pApp->MousePointerGlobalSet(IDC_Wait);
		form1.Control(ID_lblStatus).TextSet(TEXT("正在删除ProtLink表中已有记录..."));
		ADOConn.Execute(TEXT("DELETE FROM ProtLinks"));
		form1.Control(ID_lblStatus).TextSet(TEXT("已清空原数据记录"));
		pApp->MousePointerGlobalSet(0);
	}

	//文件是ANSI格式，将查找内容转换为ANSI格式，而szFind是Unicode，需要进行数据转换
	char* btFind = StrConvFromUnicode(szFind);
	long iLenFind = strlen(btFind);
	CBReadLinesEx file;
	tstring sField;//用于判断一个字段值
	long cntImported = 0;//已导入记录数
	LPTSTR szLine;
	TCHAR** s;
	int n;

	if (!file.OpenFile(szFile))
		return 0;
	file.SeekFile(posStart);//定位读取指针到posStart，从此处按行读取文件
	while (!file.IsEndRead())
	{
		//读取下一行字符串->szLine
		file.GetNextLine(szLine);
		if (file.IsErrOccured())
			return 0;
		n = Split(szLine, s, TEXT(" "));
		if (n < 3)
			continue;
		//判断文件中的9606.ENSP的区域是否已读取结束
		sField = s[1];
		if (sField.substr(0, iLenFind) != szFind)//如果s[1]的前iLenFind个字符不是szFind
			break;

		//添加条目到数据库中
		rs.AddNew();
		//指针s[1]指向"9606.ENSP00000123"，我们只需要将"ENSP"后的数字赋值给属性ENSPID1
		rs.SetField(TEXT("ENSPID1"), (int)Val(s[1] + iLenFind));
		rs.SetField(TEXT("ENSPID2"), (int)Val(s[2] + iLenFind));
		rs.SetField(TEXT("Distance"), 1000 - (int)Val(s[3]));
		rs.Update();
		cntImported++;
		if (cntImported % 300 == 0)
		{
			form1.Control(ID_lblStatus).TextSet(TEXT("已导入："));
			form1.Control(ID_lblStatus).TextAdd(cntImported);
			form1.Control(ID_lblStatus).TextAdd(TEXT(" 条记录\r\n"));
			form1.Control(ID_lblStatus).TextAdd(szLine);
			DoEvents();
			if (m_fCancelSign)
			{
				form1.Control(ID_lblStatus).TextSet(TEXT("用户中断。 "));
				break;
			}
		}
	}
	rs.Close();

	if (!m_fCancelSign)
	{
		form1.Control(ID_lblStatus).TextSet(TEXT("数据导入完成。共导入 "));
		form1.Control(ID_lblStatus).TextAdd(cntImported);
		form1.Control(ID_lblStatus).TextAdd(TEXT("条记录。 "));
	}
	return cntImported;
}

void cmdExtractTaxID_Click()
{
	
	//当本按钮为“取消”时，执行的功能
	if (_tcscmp(form1.Control(ID_cmdExtractTaxID).Text(), TEXT("取消")) == 0)
	{
		if(MsgBox(TEXT("操作尚未完成，确定要取消吗？"),TEXT("取消操作"),mb_YesNoCancel,mb_IconQuestion) != idYes)
			return;
		m_fCancelSign = true;//全局变量，是否中途取消的标志，会传递给FindPosInPPIFile()
		return;
	}

	//当本按钮为“提取到数据库”时，执行的功能
	LPTSTR szFile = form1.Control(ID_cboPPIFile).Text();
	if (*szFile == 0)//即szFile第一个字符为'\0'，为""
	{
		MsgBox(TEXT("请选择PPI数据文件！"), TEXT("未选择文件"), mb_OK, mb_IconExclamation);
		return;
	}
	int idx = form1.Control(ID_cboTaxIDs).ListIndex();
	int iTaxID = form1.Control(ID_cboTaxIDs).ItemData(idx);
	if (iTaxID <= 0)
	{
		MsgBox(TEXT("请选择物种ID！"), TEXT("未选择物种ID"), mb_OK, mb_IconExclamation);
		return;
	}

	//打开文件
	HANDLE hFile = EFOpen(szFile);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		MsgBox(TEXT("打开PPT数据文件失败！"), TEXT("打开文件失败"), mb_OK, mb_IconExclamation);
		return;
	}

	m_fCancelSign = false;
	form1.Control(ID_cmdExtractTaxID).TextSet(TEXT("取消"));

	//在文件中找到"9606.ENSP"
	tstring sFind = Str(iTaxID);
	sFind += TEXT(".ENSP");
	LONGLONG llPos = FindPosInPPIFile(hFile, sFind.c_str());

	if (llPos == 0)
	{
		form1.Control(ID_lblStatus).TextSet(TEXT("在文件未找到"));
		form1.Control(ID_lblStatus).TextAdd(sFind);
	}
	else
	{
		form1.Control(ID_lblStatus).TextSet(TEXT("在文件已找到 "));
		form1.Control(ID_lblStatus).TextAdd(sFind);
		form1.Control(ID_lblStatus).TextAdd(TEXT(" 的开始位置，为"));
		form1.Control(ID_lblStatus).TextAdd((double)llPos);
	}
	form1.Control(ID_lblStatus).TextAdd(TEXT("。"));
	EFClose(hFile);//关闭文件

	//从llpos开始读取文件，并将文件存入数据库中
	if (llPos)
		ReadPPIToDB(szFile, llPos, sFind.c_str());
	form1.Control(ID_cmdExtractTaxID).TextSet(TEXT("提取到数据库"));
}

void cboPPIFile_FilesDrop(int ptrArrFiles, int count, int x, int y)
{
	//固定套路：将ptrArrFiles强制转换为TCHAR ** 
	TCHAR** files = (TCHAR**)ptrArrFiles;
	//这些字符串{files[1],files[2],...,files[count],}就是用户拖动多个文件时，被拖动到空间上的各文件名
	//只打开一个文件，忽略其他文件
	form1.Control(ID_cboPPIFile).AddItem(files[1]);
	form1.Control(ID_cboPPIFile).TextSet(files[1]);
}

void cmdBroseEnsp_Click()
{
	LPTSTR szFile;
	OsdSetFilter(TEXT("文本文件(*.txt)|*.txt|数据文件(*.dat)|*.dat"), true);
	szFile = OsdOpenDlg(form1.hWnd(), TEXT("请选择ENSP蛋白列表文件"));
	if (*szFile)
	{
		form1.Control(ID_cboEnspListFile).AddItem(szFile);
		form1.Control(ID_cboEnspListFile).TextSet(szFile);
	}
}

void cmdContinue_Click()
{
	CBAdoRecordset rsTasks;
	CBDijkstra dj;
	int cntTasksCount;//总任务数
	int cntDone;//已经完成任务数
	long idNode1 = 0, idNode2 = 0, distance = 0;
	int i;
	m_fCancelSign = false;

	rsTasks.Open(TEXT("SELECT COUNT(*) FROM PathTasks WHERE DoneState=0"));
	cntTasksCount = (int)Val(rsTasks.GetField((long)0));
	rsTasks.Close();
	if (cntTasksCount <= 0)
	{
		form1.Control(ID_lblCount).TextSet(TEXT(""));
		form1.Control(ID_lblDoing).TextSet(TEXT(""));
		form1.Control(ID_lblStatus).TextSet(TEXT("PathTasks表内无任务，或任务都已完成。"));
		return;
	}
	form1.Control(ID_cmdAnalysis).TextSet(TEXT("中断分析"));
	form1.Control(ID_cmdContinue).EnabledSet(false);
	form1.Control(ID_cmdViewResult).EnabledSet(false);
	form1.Control(ID_lblCount).TextSet(TEXT("总任务数:"));
	form1.Control(ID_lblCount).TextAdd(cntTasksCount);
	form1.Control(ID_lblDoing).TextSet(TEXT(""));
	form1.Control(ID_lblStatus).TextSet(TEXT(""));

	form1.Control(ID_lblStatus).TextSet(TEXT("正在构建网络..."));
	rsTasks.Open(TEXT("SELECT COUNT(*) FROM ProtLinks"));
	int cntLinksCount = (int)Val(rsTasks.GetField((long)0));
	rsTasks.Close();
	//设置进度条
	form1.Control(ID_pro1).MaxSet(cntLinksCount);
	form1.Control(ID_pro1).MinSet(0);
	form1.Control(ID_pro1).ValueSet(0);
	form1.Control(ID_pro1).VisibleSet(true);

	//添加节点之间距离数据
	dj.Clear();
	if (!rsTasks.Open(TEXT("SELECT * FROM ProtLinks")))
	{
		form1.Control(ID_lblStatus).TextSet(TEXT(" 构建网络失败。访问数据库表ProtLinks出错。"));
		return;
	}
	i = 0;
	while (!rsTasks.EOFRs())
	{
		idNode1 = (long)Val(rsTasks.GetField(TEXT("ENSPID1")));
		idNode2 = (long)Val(rsTasks.GetField(TEXT("ENSPID2")));
		distance = (long)Val(rsTasks.GetField(TEXT("Distance")));
		dj.AddNodesDist(idNode1, idNode2, distance);
		i++;
		form1.Control(ID_pro1).ValueSet(i);
		if (i % 599 == 0)
		{
			form1.Control(ID_lblDoing).TextSet(idNode1);
			form1.Control(ID_lblDoing).TextAdd(TEXT("--"));
			form1.Control(ID_lblDoing).TextAdd(idNode2);
			form1.Control(ID_lblDoing).TextAdd(TEXT("  "));
			form1.Control(ID_lblDoing).TextAdd(distance);
			form1.Control(ID_lblCount).TextSet(i);
			form1.Control(ID_lblCount).TextAdd(TEXT(" / "));
			form1.Control(ID_lblCount).TextAdd(cntLinksCount);

			DoEvents();
			if (m_fCancelSign)
			{
				form1.Control(ID_lblStatus).TextSet(TEXT("用户中断。"));
				break;
			}
		}
		rsTasks.MoveNext();
	}
	rsTasks.Close();

	if (!m_fCancelSign)
	{
		form1.Control(ID_lblStatus).TextSet(TEXT("构建网络完成。"));
		form1.Control(ID_lblCount).TextSet(cntLinksCount);
		form1.Control(ID_lblDoing).TextSet(TEXT(""));
	}
	else
	{
		form1.Control(ID_cmdAnalysis).TextSet(TEXT("分析"));
		form1.Control(ID_cmdContinue).EnabledSet(true);
		form1.Control(ID_cmdViewResult).EnabledSet(true);
		form1.Control(ID_pro1).VisibleSet(false);
		return;
	}
}

void cmdAnalysis_Click()
{
	//当本按钮为“中断分析”时，执行的功能
	if (_tcscmp(form1.Control(ID_cmdAnalysis).Text(), TEXT("中断分析")) == 0)
	{
		if (MsgBox(TEXT("分析尚未完成，确定要取消吗？\r\n已完成的分析结果会保存在数据库中，中断后，您可以下次继续进行分析。"), 
			TEXT("中断分析"), mb_YesNoCancel, mb_IconQuestion) == idYes)
			m_fCancelSign = true;
		return;
	}

	//当本按钮为“分析”时，执行的功能
	if (MsgBox(TEXT("此操作将重新读取ENSP数据集文件，然后重建数据库的待分析蛋白质列表，先前的分析结果将被清楚，是否继续？"),
		TEXT("确认重新分析"), mb_YesNoCancel, mb_IconQuestion) != idYes)
		return;

	//校验文件选择
	LPTSTR szFile = form1.Control(ID_cboEnspListFile).Text();
	if (*szFile == 0)//即szFile第一个字符为'\0'，为""
	{
		MsgBox(TEXT("请选择ENSP数据文件！"), TEXT("未选择文件"), mb_OK, mb_IconExclamation);
		return;
	}
	form1.Control(ID_lblStatus).TextSet(TEXT("清空TarProts表..."));
	ADOConn.Execute(TEXT("DELETE FROM TarProts"));
	form1.Control(ID_lblStatus).TextSet(TEXT("读取ENSP数据集文件..."));
	CBReadLinesEx file;
	CBAdoRecordset rs;
	LPTSTR szLine;
	tstring sField;
	if (!rs.Open(TEXT("SELECT * FROM TarProts")))
		return;
	if (!file.OpenFile(szFile))
		return;
	while (!file.IsEndRead())
	{
		file.GetNextLine(szLine);
		if (file.IsErrOccured())
			return;
		sField = szLine;
		if (sField.substr(0, 4) == TEXT("ENSP"))
		{
			rs.AddNew();
			rs.SetField((long)0, szLine + 4);
			rs.Update();
		}
	}
	rs.Close();
	form1.Control(ID_lblStatus).TextSet(TEXT("数据库TarProts表保存完成"));

	//生成最短路径表PathTasks
	ADOConn.Execute(TEXT("DROP TABLE PathTasks"));
	ADOConn.Execute(TEXT("SELECT T1.ENSPID AS ENSPID1, T2.ENSPID AS ENSPID2, 0 AS DoneState \
INTO PathTasks FROM TarProts T1,TarProts T2 WHERE T1.ENSPID<T2.ENSPID \
GROUP BY T1.ENSPID, T2.ENSPID"));
	ADOConn.Execute(TEXT("DELETE FROM BetwResults"));
	cmdContinue_Click();
}

int main()
{

	form1.EventAdd(ID_cmdBrowse, eCommandButton_Click, cmdBrose_Click);
	form1.EventAdd(ID_cmdBrowseEnsp, eCommandButton_Click, cmdBroseEnsp_Click);
	form1.EventAdd(ID_cmdExtractTaxID, eCommandButton_Click, cmdExtractTaxID_Click);
	form1.EventAdd(ID_cboPPIFile, eFilesDrop, cboPPIFile_FilesDrop);
	form1.EventAdd(ID_cmdContinue, eCommandButton_Click, cmdContinue_Click);
	form1.EventAdd(ID_cmdAnalysis, eCommandButton_Click, cmdAnalysis_Click);

	form1.IconSet(IDI_ICON1);
	form1.Show();

	//link the Database
	if (!ADOConn.Open(TEXT("PPI.accdb")))
	{
		MsgBox(ADOConn.ErrorLastStr(), TEXT("Failed to link the database in main()"), mb_OK, mb_IconExclamation);
	}
	ListTaxIDs();

	return 0;
}

