/*
 * Winamp httpQ Plugin
 * Copyright (C) 1999-2003 Kosta Arvanitis
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Kosta Arvanitis (karvanitis@hotmail.com)
 */

#include "stdafx.h"
#include "gen_winampvscp.h"
#include "configdialog.h"
#include "pluginmanager.h"
#include "configmanager.h"
#include "util.h"
#include "resource.h"

#include <commctrl.h>
#include <htmlhelp.h>


//---------------------------------------------------
// Application defined window messages
//---------------------------------------------------
#define WM_APP_SERIALIZE (WM_APP + 1)
#define WM_APP_REFRESH (WM_APP + 2)


//---------------------------------------------------
// Type Definitions
//---------------------------------------------------
typedef struct TabItem
{
	CHAR*       pTitle;
	INT         nDialogId;
	DLGPROC	    fnDialogProc;
    HWND        hWnd;

} TabItem;


typedef struct WebCtrlInfo
{
	INT   nCtrl;
	const CHAR* pstrAddr;
	BOOL  bHover;

} WebCtrlInfo;


//---------------------------------------------------
// Function Declerations
//---------------------------------------------------
VOID ShowDialog(HINSTANCE hInstance);

BOOL CALLBACK ConfigProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK GeneralProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK AboutProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK AdvancedProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK SecurityProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK IpAddProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

VOID SetDialogTab(HWND hwndDlg, TabItem *pTabItem);
TabItem *GetDialogTab(HWND hwndDlg);
VOID DestroyDialogTab(TabItem *pTabItem);
VOID DialogEnable(HWND hDlg, WORD wID, BOOL bEnable);


//---------------------------------------------------
// Globals
//---------------------------------------------------
HINSTANCE ghInstance;

static TabItem gTabItems[] =
{
	{"General",  IDD_GENERAL,  GeneralProc,  NULL},
	{"Security", IDD_SECURITY, SecurityProc, NULL},
	{"Advanced", IDD_ADVANCED, AdvancedProc, NULL},
	{"About",    IDD_ABOUT,    AboutProc,    NULL},
};

WebCtrlInfo gWebControls[] = 
{
	{ID_ABOUT_WEB,		WEBADDR,   FALSE},
	{ID_ABOUT_EMAIL,	EMAILADDR, FALSE}
};


WebCtrlInfo* GetWebControlInfo(int nId)
{
	for ( int i=0; i<ARRAYSIZE( gWebControls ); i++ ) {
		if ( gWebControls[i].nCtrl == nId ) {
			return &gWebControls[i];
		}
	}
	return NULL;
}



//--------------------------------------------------
// Class Functions
//--------------------------------------------------
ConfigDialog* ConfigDialog::mpConfigDialog = NULL;

ConfigDialog::ConfigDialog()
{
    // init windows common controls
    INITCOMMONCONTROLSEX lpInitCtrls;
    lpInitCtrls.dwSize = sizeof(INITCOMMONCONTROLSEX);
    lpInitCtrls.dwICC = ICC_INTERNET_CLASSES;
    InitCommonControlsEx(&lpInitCtrls);
}

ConfigDialog::~ConfigDialog()
{
    if(mpConfigDialog != NULL) 
    {
        delete mpConfigDialog;
        mpConfigDialog = NULL;
    }
}

ConfigDialog* ConfigDialog::Get()
{
    if (mpConfigDialog == NULL)
        mpConfigDialog = new ConfigDialog();
    return mpConfigDialog;
}

HWND ConfigDialog::GetDialogWindow()
{
    HWND hWnd = FindWindow(NULL, PluginManager::Get()->GetPluginInterface()->description);
    return hWnd;
}
    
void ConfigDialog::ShowDialog()
{
    HWND hWnd = GetDialogWindow();
	if(hWnd != NULL)
	{
		SetFocus(hWnd);
		return;
	}
    else
    {
        mConfigInfo = *ConfigManager::Get()->GetConfig();
        ghInstance = PluginManager::Get()->GetPluginInterface()->hDllInstance;
        DialogBox(ghInstance, MAKEINTRESOURCE(IDD_DIALOG), NULL, ConfigProc);
    }
}

void ConfigDialog::Serialize()
{
    ConfigManager::Get()->SetConfig(mConfigInfo);
    ConfigManager::Get()->Serialize();
}

void ConfigDialog::Refresh()
{
    HWND hWnd = GetDialogWindow();
	if(hWnd != NULL)
    {
        SendMessage(hWnd, WM_APP_REFRESH, 0, 0);
    }
}


//---------------------------------------------------
// Global Functions
//---------------------------------------------------
BOOL CALLBACK ConfigProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
        {
            // set dialog box title
			char *tmp = PluginManager::Get()->GetPluginInterface()->description;
			SendMessage(hwndDlg, WM_SETTEXT, 0, (LPARAM)(LPCSTR)tmp);

			// enable stop and start buttons
            //if (S erver::Get()->IsRunning() )
			//	DialogEnable(hwndDlg, IDC_START, FALSE);
			//else
			//	DialogEnable(hwndDlg, IDC_STOP, FALSE);
            
            // init the tree view control
            HWND hwndTreeView = GetDlgItem(hwndDlg, IDC_TREE);

            for(int i=0; i< sizeof(gTabItems)/sizeof(gTabItems[0]); i++)
            {
                TabItem *titem = &gTabItems[i];
        
                TVINSERTSTRUCT tvitem;
                tvitem.hParent = NULL;
                tvitem.hInsertAfter = TVI_LAST;
                tvitem.item.mask = TVIF_TEXT|TVIF_PARAM;
                tvitem.item.pszText = titem->pTitle;
                tvitem.item.cchTextMax = strlen(titem->pTitle);
                tvitem.item.lParam = (LPARAM)titem;

                TreeView_InsertItem(hwndTreeView, &tvitem);
            }
            return TRUE;
        }

		case WM_APP_SERIALIZE:
        {
            TabItem *titem = GetDialogTab(hwndDlg);

            // save the state of the current tab dialog
            SendMessage(titem->hWnd, WM_APP_SERIALIZE, 0, 0);

            ConfigDialog::Get()->Serialize();

            break;
        } // WM_APP_SERIALIZE

        case WM_APP_REFRESH:
        {
            /*if(Server::Get()->IsRunning())
            {
				DialogEnable(hwndDlg, IDC_START, FALSE);
				DialogEnable(hwndDlg, IDC_STOP, TRUE);
            }
            else
            {
				DialogEnable(hwndDlg, IDC_START, TRUE);
				DialogEnable(hwndDlg, IDC_STOP, FALSE);
            }*/

            break;
        } // WM_APP_REFRESH

		case WM_COMMAND:
        {
            switch(LOWORD(wParam))
            {
                case IDOK:
                {
                    SendMessage(hwndDlg, WM_APP_SERIALIZE, 0, 0);
#ifndef _DEBUG
                    EndDialog(hwndDlg, LOWORD(wParam));
#endif
                    return TRUE;
                }

                case IDCANCEL:
                {
				    EndDialog(hwndDlg, LOWORD(wParam));
				    return TRUE;
                }

                case IDC_START:
                {
                    /*if(!Server::Get()->IsRunning())
                    {
						DialogEnable(hwndDlg, IDC_START, FALSE);
						DialogEnable(hwndDlg, IDC_STOP, TRUE);
                        Server::Get()->Start();
                    }*/
                    break;
                }

                case IDC_STOP:
                {
                    /*if(Server::Get()->IsRunning())
                    {
						DialogEnable(hwndDlg, IDC_START, TRUE);
						DialogEnable(hwndDlg, IDC_STOP, FALSE);
                        Server::Get()->Stop();
                    }*/
                    break;
                }


            }
            break;
        }

		case WM_NOTIFY:
        {
			switch(((LPNMHDR)lParam)->code) 
            {
				case TVN_SELCHANGING:
				{
                    break;
                }
				
                case TVN_SELCHANGED:
				{
                    LPNMTREEVIEW pNMTREEVIEW = (LPNMTREEVIEW)lParam;

                    TabItem *titemold = (TabItem *)pNMTREEVIEW->itemOld.lParam;
                    TabItem *titemnew = (TabItem *)pNMTREEVIEW->itemNew.lParam;

                    if (!titemnew)
                        break;
    
                    if (titemold)
                        DestroyDialogTab(titemold);

                    if (titemnew)
                        SetDialogTab(hwndDlg, titemnew);

                    break;
                }
            }
            break;
        } // WM_NOTIFY

		/*
		case WM_HELP:
		{
            LPHELPINFO pHelpInfo = (LPHELPINFO)lParam;

	        HH_POPUP popup;

	        memset(&popup, 0, sizeof(popup)); 
	        popup.cbStruct = sizeof(popup);
	        popup.hinst = ghInstance;
	        popup.idString = pHelpInfo->iCtrlId;
	        
	        popup.pt.x = pHelpInfo->MousePos.x;
	        popup.pt.y = pHelpInfo->MousePos.y;
            popup.clrBackground = -1;
	        popup.clrForeground = -1;
	        popup.rcMargins.bottom = -1;
	        popup.rcMargins.left = -1;
	        popup.rcMargins.right = -1;
	        popup.rcMargins.top = -1;
	        
	        HtmlHelp((HWND)pHelpInfo->hItemHandle, NULL, HH_DISPLAY_TEXT_POPUP, (DWORD)(LPVOID)&popup);

			break;
		}*/

	}
    return FALSE;
}

VOID DestroyDialogTab(TabItem *pTabItem)
{
    if (pTabItem->hWnd)
    {
        // save the state of the current tab dialog
        SendMessage(pTabItem->hWnd, WM_APP_SERIALIZE, 0, 0);

        DestroyWindow(pTabItem->hWnd);
    }
}

VOID SetDialogTab(HWND hwndDlg, TabItem *pTabItem)
{
    HWND hwndStatic = GetDlgItem(hwndDlg, IDC_FRAME);
	RECT rc;
	GetClientRect(hwndStatic, &rc);

	HDWP hdwp = BeginDeferWindowPos(1); 

    pTabItem->hWnd = CreateDialogParam(
		ghInstance,                             // module that contains it
		MAKEINTRESOURCE(pTabItem->nDialogId),   // dialog box template name
		hwndStatic,                             // handle to window that owns it
		pTabItem->fnDialogProc,                 // dlg box procedure
		0);                                     // passed to WM_INITDIALOG as lParam


	DeferWindowPos(hdwp, pTabItem->hWnd, HWND_TOP, 
        rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, 0); 

	ShowWindow(pTabItem->hWnd, SW_SHOW);
	EndDeferWindowPos(hdwp); 
}

// get the current tab item
TabItem *GetDialogTab(HWND hwndDlg)
{
    HWND hwndTreeView = GetDlgItem(hwndDlg, IDC_TREE);
    HTREEITEM hitem = TreeView_GetSelection(hwndTreeView);
    TVITEM tvitem;
    tvitem.mask = TVIF_PARAM;
    tvitem.hItem = hitem;
    TreeView_GetItem(hwndTreeView, &tvitem);
    TabItem *titem = (TabItem *)tvitem.lParam;

    return titem;
}


BOOL CALLBACK GeneralProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
        {
            // port
            char port[256];
			wsprintf(port, "%d", ConfigDialog::Get()->GetConfig()->GetPort());
			SendDlgItemMessage(hwndDlg, ID_GENERAL_PORT, WM_SETTEXT, 0, (LPARAM)(LPCTSTR)port);

            // password
            const char *pass = ConfigDialog::Get()->GetConfig()->GetPassword();
			SendDlgItemMessage(hwndDlg, ID_GENERAL_PASSWORD, WM_SETTEXT, 0, (LPARAM)(LPCTSTR)pass);

        } // WM_INITDIALOG

        case WM_APP_SERIALIZE:
        {
            char tmp[256];

            // port
			SendDlgItemMessage(hwndDlg, ID_GENERAL_PORT, WM_GETTEXT, (WPARAM)sizeof(tmp), (LPARAM)&tmp);
            ConfigDialog::Get()->GetConfig()->SetPort(atoi(tmp));

            // password
			SendDlgItemMessage(hwndDlg, ID_GENERAL_PASSWORD, WM_GETTEXT, (WPARAM)sizeof(tmp), (LPARAM)&tmp);
            ConfigDialog::Get()->GetConfig()->SetPassword(tmp);

            // ip
			SendDlgItemMessage(hwndDlg, ID_GENERAL_IP_ADDRESS, WM_GETTEXT, (WPARAM)sizeof(tmp), (LPARAM)&tmp);
            DWORD ipaddr = (DWORD)inet_addr(tmp);
            ConfigDialog::Get()->GetConfig()->SetServerAddress(ipaddr);

            break;
        } // WM_APP_SERIALIZE

    }

    return FALSE;
}


BOOL CALLBACK AdvancedProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
        {
            break;
        } // WM_INITDIALOG

        case WM_APP_SERIALIZE:
        {
		 

            break;
        } // WM_APP_SERIALIZE

    }

    return FALSE;
}

BOOL CALLBACK SecurityProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
        {
			

            break;
        } // WM_INITDIALOG

        case WM_APP_SERIALIZE:
        {
			// deny/ allow ip addresses
			bool allow = (IsDlgButtonChecked(hwndDlg, ID_SECURITY_ALLOW_ALL) == BST_CHECKED);
            break;
        } // WM_APP_SERIALIZE


		case WM_COMMAND:
        {
            switch(LOWORD(wParam))
            {
                case ID_SECURITY_ADD:
                {
                    int rc = DialogBoxParam(PluginManager::Get()->GetPluginInterface()->hDllInstance, 
                        MAKEINTRESOURCE(IDD_IP), hwndDlg, IpAddProc, (LPARAM)0);

                    if (rc == IDOK)
                    {
						;
                    }
                    break;
                }

                case ID_SECURITY_REMOVE:
                {
					
                    break;
                }

                case ID_SECURITY_DENY_ALL:
				{
			        

                    break;
				}

				case ID_SECURITY_ALLOW_ALL:
				{
			        
					break;
				}

            }
            break;
        } // WM_COMMAND

    }

    return FALSE;
}


BOOL CALLBACK AboutProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			char tmp[256];
            wsprintf(tmp, "%s %s", APPNAME, VERSION, COPYRIGHT, AUTHOR);
            SendDlgItemMessage(hwndDlg, ID_ABOUT_LINE1, WM_SETTEXT, 0, (LPARAM)(LPCTSTR)tmp);
            
            wsprintf(tmp, "%s %s", COPYRIGHT, AUTHOR);
            SendDlgItemMessage(hwndDlg, ID_ABOUT_LINE2, WM_SETTEXT, 0, (LPARAM)(LPCTSTR)tmp);

			int nBolds[] = {ID_ABOUT_LINE1};
			for(int i=0; i<sizeof(nBolds)/sizeof(nBolds[0]); i++)
			{
				LOGFONT lf;
				HWND hwnd = GetDlgItem(hwndDlg, nBolds[i]);				
				HFONT hFont = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0);
				GetObject(hFont, sizeof(LOGFONT), &lf);
				lf.lfWeight = FW_BOLD;
				hFont = CreateFontIndirect(&lf);
				SendMessage(hwnd, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);
			}

            break;
		} // WM_INITDIALOG

        case WM_LBUTTONUP:
        case WM_MOUSEMOVE:
        {
        	HWND hwnd;
            RECT rc;
            POINT pt;
            
			for(int i=0; i<ARRAYSIZE(gWebControls); i++)
			{
				hwnd = GetDlgItem(hwndDlg, gWebControls[i].nCtrl);
				GetWindowRect(hwnd, &rc);
				GetCursorPos(&pt);
	            if(PtInRect(&rc, pt))
				{
					if(gWebControls[i].bHover == FALSE)
					{
						gWebControls[i].bHover = TRUE;
						RedrawWindow(hwnd, NULL, NULL, RDW_UPDATENOW|RDW_INVALIDATE|RDW_ERASE);
					}

					SetCapture(hwndDlg);
					
					// set the cursor to hand
					HCURSOR hcursor;
					if((hcursor = LoadCursor(NULL, IDC_HAND)))
						SetCursor(hcursor);
					

					// goto url on btn up
					if(uMsg == WM_LBUTTONUP)
						openurl(gWebControls[i].pstrAddr);

					return FALSE;
				}

				if(gWebControls[i].bHover == TRUE)
				{
					gWebControls[i].bHover = FALSE;
					RedrawWindow(hwnd, NULL, NULL, RDW_UPDATENOW|RDW_INVALIDATE|RDW_ERASE);
				}

			}

            // release mouse capture
            ReleaseCapture();

            break;
        } // WM_LBUTTONUP, WM_MOUSEMOVE
        
        case WM_CTLCOLORSTATIC:
		{
            HDC hdc = (HDC)wParam;
			HWND hwnd = (HWND)lParam;
			int nId = GetDlgCtrlID(hwnd);

			switch(nId)
			{
                case ID_ABOUT_WEB:	
				case ID_ABOUT_EMAIL:
				{
                    LOGFONT lf;
					HFONT hFont;
					WebCtrlInfo *info = GetWebControlInfo(nId);
					hFont = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0);
					GetObject(hFont, sizeof(LOGFONT), &lf);
					if(info->bHover)
					{
                        //lf.lfUnderline = TRUE;
						SetTextColor(hdc, GetSysColor(COLOR_HOTLIGHT));
					}
					else
					{
						//lf.lfUnderline = FALSE;
						SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));
					}
					hFont = CreateFontIndirect(&lf);
					SendMessage(hwnd, WM_SETFONT, (WPARAM)hFont, (LPARAM)FALSE);
					SetBkMode(hdc, TRANSPARENT);
					HBRUSH br = (HBRUSH)GetStockObject(NULL_BRUSH);
                    return ((LRESULT)br);
				}
			}
			break;

		} // WM_CTLCOLORSTATIC

	}
    return FALSE;
}

// Callbacks for ad IP dialog box
BOOL CALLBACK IpAddProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITDIALOG:
		{	
			break;
		}

		case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
				case IDOK:
				{
					// get the ip address
					DWORD dwAddr;
					if(4 != SendDlgItemMessage(hwndDlg, ID_IP_ADDRESS, IPM_GETADDRESS, 0, (LPARAM)(LPDWORD)&dwAddr))
						break;
						
                    ConfigDialog::Get()->SetLong(dwAddr);
					
					EndDialog(hwndDlg, IDOK);
					break;
				}

				case IDCANCEL:
				{
					EndDialog(hwndDlg, IDCANCEL);
					break;
				}
			}
			break;
		} // WM_COMMAND

	} // switch
	return FALSE;
}


// Enables/disables a button
void DialogEnable(HWND hDlg, WORD wId, BOOL bEnable) 
{  
	HWND hControl = GetDlgItem(hDlg, wId);
	if (hControl)
		EnableWindow(hControl, bEnable);
}
