int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	LRESULT copyDataResult;
	HWND pOtherWnd = FindWindow(NULL, "WinLirc");
	if (pOtherWnd)
	{
		COPYDATASTRUCT cpd;
		cpd.dwData = 0;
		cpd.cbData = strlen(lpCmdLine);
		cpd.lpData = (void*)lpCmdLine;
		copyDataResult = SendMessage(pOtherWnd,WM_COPYDATA,(WPARAM)hInstance,(LPARAM)&cpd);
        // copyDataResult has value returned by other app
	}
	else
	{
		return 1;
	}
	return 0;
}


