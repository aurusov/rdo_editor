#include "stdafx.h"
#include "rdoeditorapp.h"
#include "rdoeditormainfrm.h"
#include "rdoabout.h"
#include "../resource.h"
#include "../Htmlhelp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// ----------------------------------------------------------------------------
// ---------- RDOEditorApp
// ----------------------------------------------------------------------------
RDOEditorApp rdoEditorApp;

BEGIN_MESSAGE_MAP( RDOEditorApp, CWinApp )
	ON_COMMAND( ID_APP_ABOUT, OnAppAbout )
END_MESSAGE_MAP()

RDOEditorApp::RDOEditorApp():
	CWinApp(),
	dllHandle( 0 )
{
}

RDOEditorApp::~RDOEditorApp()
{
}

BOOL RDOEditorApp::InitInstance()
{
	CWinApp::InitInstance();

	free( (void*)m_pszProfileName );
	m_pszProfileName = _tcsdup( _T("") );
	free( (void*)m_pszRegistryKey );
	m_pszRegistryKey = _tcsdup( _T("RAO-editor") );

	HFONT hf = (HFONT)::GetStockObject( DEFAULT_GUI_FONT );
	if ( hf ) {
		CFont* f = CFont::FromHandle( hf );
		if ( f ) {
			LOGFONT lf;
			f->GetLogFont( &lf );
			font.CreateFontIndirect( &lf );
		}
	}

	dllHandle = ::LoadLibrary( "RAO-editor.dll" );

	RDOEditorMainFrame* pFrame = new RDOEditorMainFrame();
	m_pMainWnd = pFrame;
	pFrame->LoadFrame( IDR_MAINFRAME, WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, NULL, NULL );
	pFrame->SetIcon( LoadIcon( MAKEINTRESOURCE(IDR_MAINFRAME) ), TRUE );
	pFrame->SetIcon( (HICON)::LoadImage( NULL, MAKEINTRESOURCE(IDR_MAINFRAME), IMAGE_ICON, ::GetSystemMetrics( SM_CXSMICON ), ::GetSystemMetrics( SM_CYSMICON ), LR_DEFAULTCOLOR ), FALSE );
	pFrame->ShowWindow( SW_SHOW );
	pFrame->UpdateWindow();

	rdoEditorProject.canClearBuffer   = rdoEditorApp.GetProfileInt( "buffer", "clear", 1 ) ? true : false;
	rdoEditorProject.clearBufferDelay = rdoEditorApp.GetProfileInt( "buffer", "delay", 7 );
	rdoEditorProject.changeLastModelPath( rdoEditorApp.GetProfileString( "reopen", "lastModelPath", "" ) );

	setupFileAssociation();

	CString fileName( m_lpCmdLine );
	if ( !fileName.IsEmpty() ) {
		int pos = fileName.Find( '"' );
		if ( pos == 0 ) {
			fileName.Delete( 0, 1 );
		}
		pos = fileName.ReverseFind( '"' );
		if ( pos == fileName.GetLength() - 1 ) {
			fileName.Delete( pos, 1 );
		}
		pFrame->insertReopenItem( rdoEditorProject.loadProject( fileName ) );
	}

	return TRUE;
}

int RDOEditorApp::ExitInstance()
{
	if ( dllHandle ) {
		::FreeLibrary( dllHandle );
	}
	HtmlHelp( NULL, NULL, HH_CLOSE_ALL, 0 );
	return CWinApp::ExitInstance();
}

CFont& RDOEditorApp::getFont()
{
	return font;
}

void RDOEditorApp::appendMenu( CMenu* from, const int from_index, CMenu* to )
{
	CString s;
	from->GetMenuString( from_index, s, MF_BYPOSITION );

	CMenu* m_from = from->GetSubMenu( from_index );
	int item_count = 0;
	if ( m_from ) item_count = m_from->GetMenuItemCount();

	if ( item_count ) {

		CMenu* m_to = new CMenu();
		m_to->CreateMenu();
		to->AppendMenu( MF_STRING | MF_POPUP, (int)m_to->m_hMenu, s );

		for ( int i = 0; i < item_count; i++ ) {
			appendMenu( m_from, i, m_to );
		}

	} else {
		UINT itemID = from->GetMenuItemID( from_index );
		if ( itemID ) {
			to->AppendMenu( MF_STRING, itemID, s );
		} else {
			to->AppendMenu( MF_SEPARATOR );
		}
	}
}

void RDOEditorApp::eraseMenu( CMenu* from, const int from_index )
{
	CMenu* m_from;
	if ( from_index != -1 )	{
		m_from = from->GetSubMenu( from_index );
	} else {
		m_from = from;
	}
	int item_count = 0;
	if ( m_from ) item_count = m_from->GetMenuItemCount();

	if ( item_count ) {

		for ( int i = item_count-1; i >= 0; i-- ) {
			eraseMenu( m_from, i );
		}

		if ( from_index != -1 ) delete m_from;

	}

	if ( from_index != -1 ) from->DeleteMenu( from_index, MF_BYPOSITION );
}

void RDOEditorApp::OnAppAbout()
{
	RDOAbout aboutDlg;
	aboutDlg.BMSTU.LoadString( ID_ABOUT_BMSTU );
	aboutDlg.tel.LoadString( ID_ABOUT_TEL );
	aboutDlg.buttonOkText.LoadString( ID_ABOUT_OKBUTTON );
	aboutDlg.hPixmap = LoadIcon( MAKEINTRESOURCE(IDR_MAINFRAME) );
	aboutDlg.DoModal();
}

void RDOEditorApp::setCaption( const CString& str )
{
	CWnd* wnd = AfxGetMainWnd();
	if ( wnd ) {
		CString s;
		s.LoadString( AFX_IDS_APP_TITLE );
		if ( !str.IsEmpty() ) {
			s += " - " + str;
		}
		wnd->SetWindowText( s );
	}
}

CString RDOEditorApp::getFullFileName()
{
	CString fileName = "";
	TCHAR szExeName[ MAX_PATH + 1 ];
	if ( ::GetModuleFileName( NULL, szExeName, MAX_PATH ) ) {
		fileName = szExeName;
	}
	return fileName;
}

CString RDOEditorApp::extractFilePath( CString str )
{
	int pos = str.ReverseFind( '\\' );
	if ( pos == -1 ) {
		pos = str.ReverseFind( '/' );
	}
	if ( pos != -1 && pos < str.GetLength() - 1 ) {
		str = str.Left( pos + 1 );
		str.TrimLeft();
		str.TrimRight();
	}
	pos = str.ReverseFind( '\\' );
	if ( pos == -1 ) {
		pos = str.ReverseFind( '/' );
	}
	if ( pos != str.GetLength() - 1 && str.GetLength() ) {
		str += "/";
	}
	return str;
}

bool RDOEditorApp::fileExists( CString str )
{
	bool result = true;
	HANDLE hFile;
	hFile = ::CreateFile( str, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if ( hFile == INVALID_HANDLE_VALUE ) result = false;
	::CloseHandle(hFile);
	return result;
}

CString RDOEditorApp::getFullHelpFileName( CString str )
{
	str.Insert( 0, extractFilePath( getFullFileName() ) );
	
	if ( !fileExists( str ) ) {
		CString msg;
		msg.Format( ID_MSG_NO_HELP_FILE, str );
		::MessageBox( NULL, msg, NULL, MB_ICONEXCLAMATION | MB_OK );
		return "";
	}

	return str;
}

void RDOEditorApp::setupFileAssociation()
{
	if ( !GetProfileInt( "fileAssociation", "checkInFuture", true ) ) return;

	CString strFileTypeId   = _T("RAO.FileInfo");
	CString strFileTypeName = _T("RAO FileInfo");
	CString strParam        = _T(" \"%1\"");
	CString strPathName     = getFullFileName();
	CString strRAOExt       = _T(".smr");

	bool win2000 = false;
	OSVERSIONINFO osvi;
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if ( ::GetVersionEx( &osvi ) ) {
		win2000 = osvi.dwMajorVersion >= 5;
	}

	HKEY hCurUserSoftClasses = 0;
	HKEY hKey;
	DWORD result;
	if ( win2000 ) {
		if ( ::RegOpenKeyEx( HKEY_CURRENT_USER, _T("Software\\Classes"), 0, KEY_ALL_ACCESS, &hCurUserSoftClasses ) != ERROR_SUCCESS ) {
			hCurUserSoftClasses = 0;
		}
	} else {
		hCurUserSoftClasses = HKEY_CLASSES_ROOT;
	}
	if ( hCurUserSoftClasses ) {
		if ( ::RegCreateKeyEx( hCurUserSoftClasses, strFileTypeId, 0, _T(""), REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &result ) == ERROR_SUCCESS ) {
			bool mustBeRegistered = true;
			DWORD size;
			if ( ::RegQueryValueEx( hKey, _T(""), NULL, NULL, NULL, &size ) == ERROR_SUCCESS ) {
				if ( size > 0 ) {
					HKEY hKeyPath;
					if ( ::RegOpenKeyEx( hKey, _T("shell\\open\\command"), 0, KEY_READ, &hKeyPath ) == ERROR_SUCCESS ) {
						CString s;
						DWORD length = MAX_PATH + lstrlen( strParam ) * sizeof(TCHAR) + 1;
						::RegQueryValueEx( hKeyPath, _T(""), NULL, NULL, (LPBYTE)s.GetBuffer( length ), &length );
						s.ReleaseBuffer();
						::RegCloseKey( hKeyPath );

						int pos = s.Find( strParam );
						if ( pos != -1 ) {
							s.Delete( pos, strParam.GetLength() );
							if ( s != strPathName ) {
								RDOFileAssociationDlg dlg;
								mustBeRegistered = dlg.DoModal() == IDOK;
								WriteProfileInt( "fileAssociation", "checkInFuture", dlg.checkInFuture ? true : false );
							} else {
								mustBeRegistered = false;
							}
						}
					}
				}
			}
			if ( mustBeRegistered ) {
				HKEY hKey_tmp;
				if ( ::RegCreateKeyEx( hCurUserSoftClasses, strFileTypeId, 0, _T(""), REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey_tmp, &result ) == ERROR_SUCCESS ) {
					CString s = strFileTypeName;
					::RegSetValueEx( hKey_tmp, _T(""), 0, REG_SZ, (LPBYTE)((LPCSTR)s), s.GetLength() );
					::RegCloseKey( hKey_tmp );
				}
				if ( ::RegCreateKeyEx( hCurUserSoftClasses, strFileTypeId + _T("\\DefaultIcon"), 0, _T(""), REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey_tmp, &result ) == ERROR_SUCCESS ) {
					CString s = strPathName + _T(",0");
					::RegSetValueEx( hKey_tmp, _T(""), 0, REG_SZ, (LPBYTE)((LPCSTR)s), s.GetLength() );
					::RegCloseKey( hKey_tmp );
				}
				if ( ::RegCreateKeyEx( hCurUserSoftClasses, strFileTypeId + _T("\\shell\\open\\command"), 0, _T(""), REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey_tmp, &result ) == ERROR_SUCCESS ) {
					CString s = strPathName + strParam;
					::RegSetValueEx( hKey_tmp, _T(""), 0, REG_SZ, (LPBYTE)((LPCSTR)s), s.GetLength() );
					::RegCloseKey( hKey_tmp );
				}
				if ( ::RegCreateKeyEx( hCurUserSoftClasses, strRAOExt, 0, _T(""), REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey_tmp, &result ) == ERROR_SUCCESS ) {
					CString s = strFileTypeId;
					::RegSetValueEx( hKey_tmp, _T(""), 0, REG_SZ, (LPBYTE)((LPCSTR)s), s.GetLength() );
					::RegCloseKey( hKey_tmp );
				}
				if ( ::RegCreateKeyEx( hCurUserSoftClasses, strRAOExt, 0, _T(""), REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey_tmp, &result ) == ERROR_SUCCESS ) {
					CString s = strFileTypeId;
					::RegSetValueEx( hKey_tmp, _T(""), 0, REG_SZ, (LPBYTE)((LPCSTR)s), s.GetLength() );
					::RegCloseKey( hKey_tmp );
				}
				if ( ::RegCreateKeyEx( hCurUserSoftClasses, strRAOExt + _T("\\ShellNew"), 0, _T(""), REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey_tmp, &result ) == ERROR_SUCCESS ) {
					CString s = "";
					::RegSetValueEx( hKey_tmp, _T("NullFile"), 0, REG_SZ, (LPBYTE)((LPCSTR)s), s.GetLength() );
					::RegCloseKey( hKey_tmp );
				}
			}
			::RegCloseKey( hKey );
		}
		if ( hCurUserSoftClasses != HKEY_CLASSES_ROOT ) {
			::RegCloseKey( hCurUserSoftClasses );
		}
	}
}

// ----------------------------------------------------------------------------
// ---------- RDOFileAssociationDlg
// ----------------------------------------------------------------------------
RDOFileAssociationDlg::RDOFileAssociationDlg( CWnd* pParentWnd ):
	CDialog( IDD_FILEASSOCIATION_DIALOG, pParentWnd ),
	checkInFuture( true )
{
}

RDOFileAssociationDlg::~RDOFileAssociationDlg()
{
}

void RDOFileAssociationDlg::DoDataExchange( CDataExchange* pDX )
{
	CDialog::DoDataExchange( pDX );

	DDX_Check( pDX, IDC_FILEASSOCIATION_CHECK, checkInFuture );
}

void RDOFileAssociationDlg::OnCancel()
{
	CDialog::UpdateData( true );
	CDialog::OnCancel();
}
