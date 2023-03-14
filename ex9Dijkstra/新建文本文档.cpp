//检查粘贴时，剪切板数据是否为0
bool VerifyClipboardInput(TCHAR** pField, const unsigned int row_index)
{
	LPTSTR szText = TEXT("剪切板第");
	szText = StrAppend(szText, Str(row_index));
	if (Val(pField[1]) == 0)
	{
		MsgBox(StrAppend(szText, TEXT("行的结点1须非零！")), TEXT("输入数据错误"), mb_OK, mb_IconExclamation);
		return false;
	}
	if (Val(pField[2]) == 0)
	{
		MsgBox(StrAppend(szText, TEXT("行的结点2须非零！")), TEXT("输入数据错误"), mb_OK, mb_IconExclamation);
		return false;
	}
	if (Val(pField[3]) == 0)
	{
		MsgBox(StrAppend(szText, TEXT("行的距离须非零！")), TEXT("输入数据错误"), mb_OK, mb_IconExclamation);
		return false;
	}
	return true;
}