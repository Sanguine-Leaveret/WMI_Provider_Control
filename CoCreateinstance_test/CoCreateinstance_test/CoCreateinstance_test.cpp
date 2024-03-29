#include "pch.h"
#include <iostream>
#include <comdef.h>
#include <Wbemidl.h>
#pragma comment(lib, "wbemuuid.lib")

IWbemServices *WMI_Init()
{
	// Initialize COM.
	CoInitializeEx(0, COINIT_MULTITHREADED);

	// Initialize 
	CoInitializeSecurity(
		NULL,
		-1,      // COM negotiates service                  
		NULL,    // Authentication services
		NULL,    // Reserved
		RPC_C_AUTHN_LEVEL_DEFAULT,    // authentication
		RPC_C_IMP_LEVEL_IMPERSONATE,  // Impersonation
		NULL,             // Authentication info 
		EOAC_NONE,        // Additional capabilities
		NULL              // Reserved
	);

	// COM 개체를 인스턴스화합니다. 
	IWbemLocator *pLoc = 0;
	CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID *)& pLoc);

	// WMI 네임 스페이스를 연결합니다.
	IWbemServices *pSvc = 0;
	pLoc->ConnectServer(
		_bstr_t(L"ROOT\\CIMV2"), // WMI namespace
		NULL,                    // User name
		NULL,                    // User password
		0,                       // Locale
		NULL,                    // Security flags                 
		0,                       // Authority       
		0,                       // Context object
		&pSvc                    // IWbemServices proxy
	);

	// 인증 정보를 설정합니다. 
	CoSetProxyBlanket(
		pSvc,                         // the proxy to set
		RPC_C_AUTHN_WINNT,            // authentication service
		RPC_C_AUTHZ_NONE,             // authorization service
		NULL,                         // Server principal name
		RPC_C_AUTHN_LEVEL_CALL,       // authentication level
		RPC_C_IMP_LEVEL_IMPERSONATE,  // impersonation level
		NULL,                         // client identity 
		EOAC_NONE                     // proxy capabilities     
	);

	return pSvc;
}

void WMI_Reg_SetStringValue(IWbemServices *pSvc)
{
	HRESULT hres;
	// WMI 클래스를 가져옵니다
	IWbemClassObject *pClass = 0;
	pSvc->GetObject(_bstr_t(L"StdRegProv"), 0, NULL, &pClass, NULL);

	// 이 클래스에서 메소드를 가져옵니다.
	IWbemClassObject *pInParamsDefinition = 0;
	pClass->GetMethod(_bstr_t(L"SetStringValue"), 0, &pInParamsDefinition, NULL);

	// 클래스의 새 인스턴스를 만듭니다. 
	IWbemClassObject *pClassInstance = 0;
	pInParamsDefinition->SpawnInstance(0, &pClassInstance);

	// 매개 변수를 놓습니다
	VARIANT varRootKey;
	varRootKey.vt = VT_I4;
	//HKLM 키 수정시 관리자 권한 필요
	//varRootKey.uintVal = 0x80000002;	//HKEY_LOCAL_MACHINE
	varRootKey.uintVal = 0x80000001;	//HKEY_CURRENT_USER
	hres = pClassInstance->Put(_bstr_t(L"hDefKey"), 0, &varRootKey, 0);
	printf("%x\n", hres);
	VARIANT varSubKey;
	varSubKey.vt = VT_BSTR;
	varSubKey.bstrVal = SysAllocString(_bstr_t(L"Software\\Microsoft\\Windows\\CurrentVersion\\Run"));
	hres = pClassInstance->Put(_bstr_t(L"sSubKeyName"), 0, &varSubKey, 0);
	printf("%d\n", hres);
	VARIANT varValueName;
	varValueName.vt = VT_BSTR;
	varValueName.bstrVal = SysAllocString(_bstr_t(L"test"));
	hres = pClassInstance->Put(_bstr_t(L"sValueName"), 0, &varValueName, 0);
	printf("%x\n", hres);

	VARIANT varValue;
	varValue.vt = VT_BSTR;
	varValue.bstrVal = SysAllocString(_bstr_t(L"aaaa"));
	hres = pClassInstance->Put(_bstr_t(L"sValue"), 0, &varValue, 0);
	printf("%x\n", hres);

	// 실행합니다.
	IWbemClassObject *pOutParams = 0;
	hres = pSvc->ExecMethod(_bstr_t(L"StdRegProv"), _bstr_t(L"SetStringValue"), 0, NULL, pClassInstance, &pOutParams, NULL);
	printf("%d\n", hres);
	printf("%d\n", GetLastError());
}

void WMI_Reg_CreateKey(IWbemServices *pSvc)
{
	// WMI 클래스를 가져옵니다
	IWbemClassObject *pClass = 0;
	pSvc->GetObject(_bstr_t(L"StdRegProv"), 0, NULL, &pClass, NULL);

	// 이 클래스에서 메소드를 가져옵니다.
	IWbemClassObject *pInParamsDefinition = 0;
	pClass->GetMethod(_bstr_t(L"CreateKey"), 0, &pInParamsDefinition, NULL);

	// 클래스의 새 인스턴스를 만듭니다. 
	IWbemClassObject *pClassInstance = 0;
	pInParamsDefinition->SpawnInstance(0, &pClassInstance);

	// 매개 변수를 놓습니다
	VARIANT varRootKey;
	varRootKey.vt = VT_UINT;
	//HKLM 키 수정시 관리자 권한 필요
	varRootKey.uintVal = 0x80000002;	//HKEY_LOCAL_MACHINE
	//varRootKey.uintVal = 0x80000001;	//HKEY_CURRENT_USER
	pClassInstance->Put(_bstr_t(L"hDefKey"), 0, &varRootKey, 0);

	VARIANT varSubKey;
	varSubKey.vt = VT_BSTR;
	varSubKey.bstrVal = _bstr_t(L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\test.exe");
	pClassInstance->Put(_bstr_t(L"sSubKeyName"), 0, &varSubKey, 0);

	// 실행합니다.
	IWbemClassObject *pOutParams = 0;
	pSvc->ExecMethod(_bstr_t(L"StdRegProv"), _bstr_t(L"CreateKey"), 0, NULL, pClassInstance, &pOutParams, NULL);
}

void WMI_Process_CreateProcess(IWbemServices *pSvc)
{
	// WMI 클래스를 가져옵니다
	IWbemClassObject *pClass = 0;
	pSvc->GetObject(_bstr_t(L"Win32_Process"), 0, NULL, &pClass, NULL);

	// 이 클래스에서 메소드를 가져옵니다.
	IWbemClassObject *pInParamsDefinition = 0;
	pClass->GetMethod(_bstr_t(L"Create"), 0, &pInParamsDefinition, NULL);

	// 클래스의 새 인스턴스를 만듭니다. 
	IWbemClassObject *pClassInstance = 0;
	pInParamsDefinition->SpawnInstance(0, &pClassInstance);

	// 매개 변수를 놓습니다
	VARIANT varCommand;
	varCommand.vt = VT_BSTR;
	//HKLM 키 수정시 관리자 권한 필요
	varCommand.bstrVal = _bstr_t(L"C:\\Windows\\system32\\reg.exe add HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\run /v test /t REG_SZ /d aaaa");
	pClassInstance->Put(_bstr_t(L"CommandLine"), 0, &varCommand, 0);

	// 실행합니다.
	IWbemClassObject *pOutParams = 0;
	pSvc->ExecMethod(_bstr_t(L"Win32_Process"), _bstr_t(L"Create"), 0, NULL, pClassInstance, &pOutParams, NULL);
}

int main()
{
	IWbemServices *pSvc = 0;
	pSvc = WMI_Init();
	WMI_Reg_SetStringValue(pSvc);	
}