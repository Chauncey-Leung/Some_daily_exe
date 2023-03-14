#include "resource.h"
#include "BForm.h"
#include "mdlOpenSaveDlg.h"
#include "BReadLinesEx.h"
#include "BDijkstra.h"
#include "BADO.h"//Ҫλ�����

CBForm form1(ID_form1);//�ô����Ĵ���ȥ��ʼ����CBForm
bool m_fCancelSign = true;//�Ƿ���;ȡ���ı�־

void ListTaxIDs()
{
	CBAdoRecordset rs;
	tstring sItem;
	int iTaxID = 0;
	int idx = 0;

	//��VS��дSQL��䣬Ȼ�������ݿ���ִ�У��ٽ�������ظ�VS
	//taxes -> rs
	if (!rs.Open(TEXT("SELECT * FROM taxes")))
	{
		MsgBox(ADOConn.ErrorLastStr(), TEXT("��ȡ������taxesʧ�ܣ�"), mb_OK, mb_IconExclamation);
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
	//����Ĭ��ѡ��
	if (form1.Control(ID_cboTaxIDs).ListCount() > 0)
	{
		form1.Control(ID_cboTaxIDs).ListIndexSet(488);
	}
}

void cmdBrose_Click()
{
	LPTSTR szFile;
	OsdSetFilter(TEXT("�ı��ļ�(*.txt)|*.txt|�����ļ�(*.dat)|*.dat"), true);
	szFile = OsdOpenDlg(form1.hWnd(), TEXT("��ѡ��PPI�����ļ�"));
	if (*szFile)
	{
		form1.Control(ID_cboPPIFile).AddItem(szFile);
		form1.Control(ID_cboPPIFile).TextSet(szFile);
	}
}

LONGLONG FindPosInPPIFile(HANDLE hFile, LPCTSTR szFind)
{
	//�ļ���ANSI��ʽ������������ת��ΪANSI��ʽ����szFind��Unicode����Ҫ��������ת��
	char* btFind = StrConvFromUnicode(szFind);
	long iLenFind = strlen(btFind);
	char buff[131072];
	bool fFound = false;
	long iBytesRead,
		 cntCycled = 0;
	LONGLONG llFileLen = EFLOF(hFile);//�ļ����ֽ���
	LONGLONG llPos = 0;//��ǰ��дָ�룬0~1,069,280,000

	int i, j;

	while (llPos < llFileLen)
	{
		//��llPos��ʼ��ȡ131072�ֽ�
		iBytesRead = EFGetBytes(hFile, llPos, buff, sizeof(buff));//default:iShowResume = 1,��ʾ���������Ժ�ȡ��������ť
		if (iBytesRead <= 0)
			break;//��ȡ�ļ���������д��msgbox����

		for (i = 0;i < iBytesRead;i++)
		{
			if (buff[i] == 10 || (llPos == 0 && i == 0))//�ҵ�'\n'�����ҵ�ȫ�ļ���0���ֽ�
			{
				if (buff[i] == 10)
					i++;

				//������ȡ���ֽ��У���i���ֽ�֮��������Ѿ�����һ��szFind�ĳ��ȣ���Ҫ����һ����ȡ���ֽ����Ӻ�Ƚ�
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

		//����Ϊ�ڱ���buff��δ�ҵ�
		llPos += iBytesRead;
		cntCycled++;
		if (cntCycled % 50 == 0)
		{
			form1.Control(ID_lblStatus).TextSet(TEXT("���� "));
			form1.Control(ID_lblStatus).TextAdd((double)(llPos / 1024 / 1024));
			form1.Control(ID_lblStatus).TextAdd(TEXT("MB / "));
			form1.Control(ID_lblStatus).TextAdd((double)(llFileLen / 1024 / 1024));
			form1.Control(ID_lblStatus).TextAdd(TEXT("MB... "));
			DoEvents();
			if (m_fCancelSign)
			{
				form1.Control(ID_lblStatus).TextSet(TEXT("�û��жϡ� "));
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
	if (!rs.EOFRs())//��ʱָ��Ĳ���rs��ĩβ����Ϊ˵��rs�����Ѿ��������ˣ���Ҫѯ���Ƿ����
	{
		if (MsgBox(TEXT("���ݿ�ProtLink�����м�¼��������������¼�¼����ɾ��ԭ�м�¼��\r\n�Ƿ�ɾ��ԭ�м�¼��"),
			TEXT("������ݿ��"), mb_YesNoCancel, mb_IconExclamation) != idYes)
			return 0;
		//������ݱ�
		pApp->MousePointerGlobalSet(IDC_Wait);
		form1.Control(ID_lblStatus).TextSet(TEXT("����ɾ��ProtLink�������м�¼..."));
		ADOConn.Execute(TEXT("DELETE FROM ProtLinks"));
		form1.Control(ID_lblStatus).TextSet(TEXT("�����ԭ���ݼ�¼"));
		pApp->MousePointerGlobalSet(0);
	}

	//�ļ���ANSI��ʽ������������ת��ΪANSI��ʽ����szFind��Unicode����Ҫ��������ת��
	char* btFind = StrConvFromUnicode(szFind);
	long iLenFind = strlen(btFind);
	CBReadLinesEx file;
	tstring sField;//�����ж�һ���ֶ�ֵ
	long cntImported = 0;//�ѵ����¼��
	LPTSTR szLine;
	TCHAR** s;
	int n;

	if (!file.OpenFile(szFile))
		return 0;
	file.SeekFile(posStart);//��λ��ȡָ�뵽posStart���Ӵ˴����ж�ȡ�ļ�
	while (!file.IsEndRead())
	{
		//��ȡ��һ���ַ���->szLine
		file.GetNextLine(szLine);
		if (file.IsErrOccured())
			return 0;
		n = Split(szLine, s, TEXT(" "));
		if (n < 3)
			continue;
		//�ж��ļ��е�9606.ENSP�������Ƿ��Ѷ�ȡ����
		sField = s[1];
		if (sField.substr(0, iLenFind) != szFind)//���s[1]��ǰiLenFind���ַ�����szFind
			break;

		//�����Ŀ�����ݿ���
		rs.AddNew();
		//ָ��s[1]ָ��"9606.ENSP00000123"������ֻ��Ҫ��"ENSP"������ָ�ֵ������ENSPID1
		rs.SetField(TEXT("ENSPID1"), (int)Val(s[1] + iLenFind));
		rs.SetField(TEXT("ENSPID2"), (int)Val(s[2] + iLenFind));
		rs.SetField(TEXT("Distance"), 1000 - (int)Val(s[3]));
		rs.Update();
		cntImported++;
		if (cntImported % 300 == 0)
		{
			form1.Control(ID_lblStatus).TextSet(TEXT("�ѵ��룺"));
			form1.Control(ID_lblStatus).TextAdd(cntImported);
			form1.Control(ID_lblStatus).TextAdd(TEXT(" ����¼\r\n"));
			form1.Control(ID_lblStatus).TextAdd(szLine);
			DoEvents();
			if (m_fCancelSign)
			{
				form1.Control(ID_lblStatus).TextSet(TEXT("�û��жϡ� "));
				break;
			}
		}
	}
	rs.Close();

	if (!m_fCancelSign)
	{
		form1.Control(ID_lblStatus).TextSet(TEXT("���ݵ�����ɡ������� "));
		form1.Control(ID_lblStatus).TextAdd(cntImported);
		form1.Control(ID_lblStatus).TextAdd(TEXT("����¼�� "));
	}
	return cntImported;
}

void cmdExtractTaxID_Click()
{
	
	//������ťΪ��ȡ����ʱ��ִ�еĹ���
	if (_tcscmp(form1.Control(ID_cmdExtractTaxID).Text(), TEXT("ȡ��")) == 0)
	{
		if(MsgBox(TEXT("������δ��ɣ�ȷ��Ҫȡ����"),TEXT("ȡ������"),mb_YesNoCancel,mb_IconQuestion) != idYes)
			return;
		m_fCancelSign = true;//ȫ�ֱ������Ƿ���;ȡ���ı�־���ᴫ�ݸ�FindPosInPPIFile()
		return;
	}

	//������ťΪ����ȡ�����ݿ⡱ʱ��ִ�еĹ���
	LPTSTR szFile = form1.Control(ID_cboPPIFile).Text();
	if (*szFile == 0)//��szFile��һ���ַ�Ϊ'\0'��Ϊ""
	{
		MsgBox(TEXT("��ѡ��PPI�����ļ���"), TEXT("δѡ���ļ�"), mb_OK, mb_IconExclamation);
		return;
	}
	int idx = form1.Control(ID_cboTaxIDs).ListIndex();
	int iTaxID = form1.Control(ID_cboTaxIDs).ItemData(idx);
	if (iTaxID <= 0)
	{
		MsgBox(TEXT("��ѡ������ID��"), TEXT("δѡ������ID"), mb_OK, mb_IconExclamation);
		return;
	}

	//���ļ�
	HANDLE hFile = EFOpen(szFile);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		MsgBox(TEXT("��PPT�����ļ�ʧ�ܣ�"), TEXT("���ļ�ʧ��"), mb_OK, mb_IconExclamation);
		return;
	}

	m_fCancelSign = false;
	form1.Control(ID_cmdExtractTaxID).TextSet(TEXT("ȡ��"));

	//���ļ����ҵ�"9606.ENSP"
	tstring sFind = Str(iTaxID);
	sFind += TEXT(".ENSP");
	LONGLONG llPos = FindPosInPPIFile(hFile, sFind.c_str());

	if (llPos == 0)
	{
		form1.Control(ID_lblStatus).TextSet(TEXT("���ļ�δ�ҵ�"));
		form1.Control(ID_lblStatus).TextAdd(sFind);
	}
	else
	{
		form1.Control(ID_lblStatus).TextSet(TEXT("���ļ����ҵ� "));
		form1.Control(ID_lblStatus).TextAdd(sFind);
		form1.Control(ID_lblStatus).TextAdd(TEXT(" �Ŀ�ʼλ�ã�Ϊ"));
		form1.Control(ID_lblStatus).TextAdd((double)llPos);
	}
	form1.Control(ID_lblStatus).TextAdd(TEXT("��"));
	EFClose(hFile);//�ر��ļ�

	//��llpos��ʼ��ȡ�ļ��������ļ��������ݿ���
	if (llPos)
		ReadPPIToDB(szFile, llPos, sFind.c_str());
	form1.Control(ID_cmdExtractTaxID).TextSet(TEXT("��ȡ�����ݿ�"));
}

void cboPPIFile_FilesDrop(int ptrArrFiles, int count, int x, int y)
{
	//�̶���·����ptrArrFilesǿ��ת��ΪTCHAR ** 
	TCHAR** files = (TCHAR**)ptrArrFiles;
	//��Щ�ַ���{files[1],files[2],...,files[count],}�����û��϶�����ļ�ʱ�����϶����ռ��ϵĸ��ļ���
	//ֻ��һ���ļ������������ļ�
	form1.Control(ID_cboPPIFile).AddItem(files[1]);
	form1.Control(ID_cboPPIFile).TextSet(files[1]);
}

void cmdBroseEnsp_Click()
{
	LPTSTR szFile;
	OsdSetFilter(TEXT("�ı��ļ�(*.txt)|*.txt|�����ļ�(*.dat)|*.dat"), true);
	szFile = OsdOpenDlg(form1.hWnd(), TEXT("��ѡ��ENSP�����б��ļ�"));
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
	int cntTasksCount;//��������
	int cntDone;//�Ѿ����������
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
		form1.Control(ID_lblStatus).TextSet(TEXT("PathTasks���������񣬻���������ɡ�"));
		return;
	}
	form1.Control(ID_cmdAnalysis).TextSet(TEXT("�жϷ���"));
	form1.Control(ID_cmdContinue).EnabledSet(false);
	form1.Control(ID_cmdViewResult).EnabledSet(false);
	form1.Control(ID_lblCount).TextSet(TEXT("��������:"));
	form1.Control(ID_lblCount).TextAdd(cntTasksCount);
	form1.Control(ID_lblDoing).TextSet(TEXT(""));
	form1.Control(ID_lblStatus).TextSet(TEXT(""));

	form1.Control(ID_lblStatus).TextSet(TEXT("���ڹ�������..."));
	rsTasks.Open(TEXT("SELECT COUNT(*) FROM ProtLinks"));
	int cntLinksCount = (int)Val(rsTasks.GetField((long)0));
	rsTasks.Close();
	//���ý�����
	form1.Control(ID_pro1).MaxSet(cntLinksCount);
	form1.Control(ID_pro1).MinSet(0);
	form1.Control(ID_pro1).ValueSet(0);
	form1.Control(ID_pro1).VisibleSet(true);

	//��ӽڵ�֮���������
	dj.Clear();
	if (!rsTasks.Open(TEXT("SELECT * FROM ProtLinks")))
	{
		form1.Control(ID_lblStatus).TextSet(TEXT(" ��������ʧ�ܡ��������ݿ��ProtLinks����"));
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
				form1.Control(ID_lblStatus).TextSet(TEXT("�û��жϡ�"));
				break;
			}
		}
		rsTasks.MoveNext();
	}
	rsTasks.Close();

	if (!m_fCancelSign)
	{
		form1.Control(ID_lblStatus).TextSet(TEXT("����������ɡ�"));
		form1.Control(ID_lblCount).TextSet(cntLinksCount);
		form1.Control(ID_lblDoing).TextSet(TEXT(""));
	}
	else
	{
		form1.Control(ID_cmdAnalysis).TextSet(TEXT("����"));
		form1.Control(ID_cmdContinue).EnabledSet(true);
		form1.Control(ID_cmdViewResult).EnabledSet(true);
		form1.Control(ID_pro1).VisibleSet(false);
		return;
	}
}

void cmdAnalysis_Click()
{
	//������ťΪ���жϷ�����ʱ��ִ�еĹ���
	if (_tcscmp(form1.Control(ID_cmdAnalysis).Text(), TEXT("�жϷ���")) == 0)
	{
		if (MsgBox(TEXT("������δ��ɣ�ȷ��Ҫȡ����\r\n����ɵķ�������ᱣ�������ݿ��У��жϺ��������´μ������з�����"), 
			TEXT("�жϷ���"), mb_YesNoCancel, mb_IconQuestion) == idYes)
			m_fCancelSign = true;
		return;
	}

	//������ťΪ��������ʱ��ִ�еĹ���
	if (MsgBox(TEXT("�˲��������¶�ȡENSP���ݼ��ļ���Ȼ���ؽ����ݿ�Ĵ������������б���ǰ�ķ����������������Ƿ������"),
		TEXT("ȷ�����·���"), mb_YesNoCancel, mb_IconQuestion) != idYes)
		return;

	//У���ļ�ѡ��
	LPTSTR szFile = form1.Control(ID_cboEnspListFile).Text();
	if (*szFile == 0)//��szFile��һ���ַ�Ϊ'\0'��Ϊ""
	{
		MsgBox(TEXT("��ѡ��ENSP�����ļ���"), TEXT("δѡ���ļ�"), mb_OK, mb_IconExclamation);
		return;
	}
	form1.Control(ID_lblStatus).TextSet(TEXT("���TarProts��..."));
	ADOConn.Execute(TEXT("DELETE FROM TarProts"));
	form1.Control(ID_lblStatus).TextSet(TEXT("��ȡENSP���ݼ��ļ�..."));
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
	form1.Control(ID_lblStatus).TextSet(TEXT("���ݿ�TarProts�������"));

	//�������·����PathTasks
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

