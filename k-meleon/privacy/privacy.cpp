/*
*  Copyright (C) 2004 Romain Vallet
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2, or (at your option)
*  any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program; if not, write to the Free Software
*  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include <windows.h>
#include <stdio.h>
#include "privacy_res.h"

#define KMELEON_PLUGIN_EXPORTS
#include "..\kmeleon_plugin.h"
#include "..\Utils.h"

#define PLUGIN_NAME "Privacy Plugin"

#define _T(x) x

#define PREFERENCE_SETTINGS_DIR         _T("kmeleon.general.settingsDir")
#define PREFERENCE_CACHE_PARENTDIR      _T("browser.cache.disk.parent_directory")
#define PREFERENCE_MRU_MAXURLS          _T("kmeleon.MRU.maxURLs")
#define PREFERENCE_MRU_URL              _T("kmeleon.MRU.URL")
#define PREFERENCE_SIGNON_FILE          _T("signon.SignonFileName")

#define PREFERENCE_CLEARCOOKIES  _T("kmeleon.plugins.privacy.clearCookies")
#define PREFERENCE_CLEARCACHE    _T("kmeleon.plugins.privacy.clearCache")
#define PREFERENCE_CLEARMRU      _T("kmeleon.plugins.privacy.clearMRU")
#define PREFERENCE_CLEARSIGNON   _T("kmeleon.plugins.privacy.clearSignOn")
#define PREFERENCE_CLEARHISTORY  _T("kmeleon.plugins.privacy.clearHistory")

#define PRV_ONSTARTUP 1
#define PRV_ONSHUTDOWN 2

TCHAR settingsDir[MAX_PATH];        // settings directory
TCHAR cacheParentDir[MAX_PATH];     // cache parent directory
TCHAR signonFileName[MAX_PATH];     // sign on file

// Commands IDs
UINT cmdClearCookies;
UINT cmdClearCache;
UINT cmdClearMRU;
UINT cmdClearSignon;
UINT cmdClearHistory;
UINT cmdConfig;

// Preferences
INT prefClearCookies = 0;
INT prefClearCache = 0;
INT prefClearMRU = 0;
INT prefClearSignon = 0;
INT prefClearHistory = 0;

HWND hMainWindow;

BOOL APIENTRY DllMain (
        HANDLE hModule,
        DWORD ul_reason_for_call,
        LPVOID lpReserved)
{
  return TRUE;
}

LRESULT CALLBACK WndProc (
        HWND hWnd, UINT message,
        WPARAM wParam,
        LPARAM lParam);
BOOL CALLBACK DlgProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void * KMeleonWndProc;

INT Init();
void Create(HWND parent);
void Config(HWND parent);
void Quit();
void DoMenu(HMENU menu, char *param);
LONG DoMessage(LPCTSTR to, LPCTSTR from, LPCTSTR subject, LONG data1, LONG data2);
void DoRebar(HWND rebarWnd);
INT DoAccel(LPTSTR param);

kmeleonPlugin kPlugin =
{
   KMEL_PLUGIN_VER,
   PLUGIN_NAME,
   DoMessage
};
kmeleonFunctions *kFuncs;

// Global variables initialization
void InitGlobals()
{
    cmdClearCookies = kFuncs->GetCommandIDs(1);
    cmdClearCache = kFuncs->GetCommandIDs(1);
    cmdClearMRU = kFuncs->GetCommandIDs(1);
    cmdClearSignon = kFuncs->GetCommandIDs(1);
    cmdClearHistory = kFuncs->GetCommandIDs(1);
    cmdConfig = kFuncs->GetCommandIDs(1);
    kFuncs->GetPreference(PREF_STRING, PREFERENCE_SETTINGS_DIR, settingsDir, NULL);
    kFuncs->GetPreference(PREF_STRING, PREFERENCE_CACHE_PARENTDIR, cacheParentDir, NULL);
    kFuncs->GetPreference(PREF_STRING, PREFERENCE_SIGNON_FILE, signonFileName, NULL);
}

// Load the plugin preferences
void LoadPluginPreferences()
{
    kFuncs->GetPreference(PREF_INT, PREFERENCE_CLEARCOOKIES, &prefClearCookies, &prefClearCookies);
    kFuncs->GetPreference(PREF_INT, PREFERENCE_CLEARCACHE, &prefClearCache, &prefClearCache);
    kFuncs->GetPreference(PREF_INT, PREFERENCE_CLEARMRU, &prefClearMRU, &prefClearMRU);
    kFuncs->GetPreference(PREF_INT, PREFERENCE_CLEARSIGNON, &prefClearSignon, &prefClearSignon);
    kFuncs->GetPreference(PREF_INT, PREFERENCE_CLEARHISTORY, &prefClearHistory, &prefClearHistory);
}

// Save the plugin preferences
void SavePluginPreferences()
{
    kFuncs->SetPreference(PREF_INT, PREFERENCE_CLEARCOOKIES, &prefClearCookies);
    kFuncs->SetPreference(PREF_INT, PREFERENCE_CLEARCACHE, &prefClearCache);
    kFuncs->SetPreference(PREF_INT, PREFERENCE_CLEARMRU, &prefClearMRU);
    kFuncs->SetPreference(PREF_INT, PREFERENCE_CLEARSIGNON, &prefClearSignon);
    kFuncs->SetPreference(PREF_INT, PREFERENCE_CLEARHISTORY, &prefClearHistory);
}

// Clear the cookies
void ClearCookies()
{
    TCHAR cookiesFile[MAX_PATH];    // cookies file with full path
    strcpy(cookiesFile, settingsDir);
    strcat(cookiesFile, "cookies.txt");
    DeleteFile(cookiesFile);
}

// Clear the history
void ClearHistory()
{
    TCHAR historyFile[MAX_PATH];    // history file with full path
    strcpy(historyFile, settingsDir);
    strcat(historyFile, "history.txt");
    DeleteFile(historyFile);
}

// Clear the Sign on data
void ClearSignon()
{
    TCHAR signonFile[MAX_PATH];    // sign on file with full path
    strcpy(signonFile, settingsDir);
    strcat(signonFile, signonFileName);
    DeleteFile(signonFile);
}

// Delete all the files in a directory
void EmptyDirectory(LPCTSTR sDirectory)
{
    WIN32_FIND_DATA FindFileData;
    HANDLE hFind = NULL;
    TCHAR fileToDelete[MAX_PATH];
    TCHAR sDirMask[MAX_PATH];
        
    strcpy(sDirMask, sDirectory);
    strcat(sDirMask, "*");
    hFind = FindFirstFile(sDirMask, &FindFileData);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        if (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            strcpy(fileToDelete, sDirectory);
            strcat(fileToDelete, FindFileData.cFileName);
            DeleteFile(fileToDelete);
        }
        while (FindNextFile(hFind, &FindFileData) != 0)
            if (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                strcpy(fileToDelete, sDirectory);
                strcat(fileToDelete, FindFileData.cFileName);
                DeleteFile(fileToDelete);
            }
        FindClose(hFind);
    }
}

// Clear the cache
void ClearCache()
{
    TCHAR cacheDir[MAX_PATH];
    strcpy(cacheDir, cacheParentDir);
    strcat(cacheDir, "Cache\\");
    
    EmptyDirectory(cacheDir);

    strcpy(cacheDir, cacheParentDir);
    strcat(cacheDir, "Cache.Trash\\");

    EmptyDirectory(cacheDir);
}

// Clear the MRUs
void ClearMRU()
{
    UINT i, NbMRUs = 16;
    TCHAR PrefName[256];
    
    kFuncs->GetPreference(PREF_INT, PREFERENCE_MRU_MAXURLS, &NbMRUs, &NbMRUs);
    for (i=0; i<NbMRUs; i++)
    {
        sprintf(PrefName, "%s%i", PREFERENCE_MRU_URL, i);
        kFuncs->SetPreference(PREF_STRING, PrefName, (void*)"");
    }
}

// Process the Shutdown tasks
void DoShutdownTasks()
{
    if (prefClearCookies & PRV_ONSHUTDOWN)
        ClearCookies();
    if (prefClearHistory & PRV_ONSHUTDOWN)
        ClearHistory();
    if (prefClearMRU & PRV_ONSHUTDOWN)
        ClearMRU();
    if (prefClearCache & PRV_ONSHUTDOWN)
        ClearCache();
    if (prefClearSignon & PRV_ONSHUTDOWN)
        ClearSignon();
}

// Process the Startup tasks
void DoStartupTasks()
{
    if (prefClearCookies & PRV_ONSTARTUP)
        ClearCookies();
    if (prefClearMRU & PRV_ONSTARTUP)
        ClearMRU();
    if (prefClearHistory & PRV_ONSTARTUP)
        ClearHistory();
    if (prefClearCache & PRV_ONSTARTUP)
        ClearCache();
    if (prefClearSignon & PRV_ONSTARTUP)
        ClearSignon();
}

/*void DebugInt(INT i)
{
    TCHAR s[256];
    sprintf(s, "%i", i);
    MessageBox(hMainWindow, s, "", 0);
}*/

INT Init()
{
    kFuncs = kPlugin.kFuncs;
    InitGlobals();
    LoadPluginPreferences();
    SavePluginPreferences();    // If the Preferences don't exist yet, this create them
    DoStartupTasks();
    return TRUE;
}

void Create(HWND parent)
{
   KMeleonWndProc = (void *) GetWindowLong(parent, GWL_WNDPROC);
   SetWindowLong(parent, GWL_WNDPROC, (LONG)WndProc);
   hMainWindow = parent;
}

void Config(HWND parent)
{
   DialogBoxParam(kPlugin.hDllInstance, MAKEINTRESOURCE(DLG_CONFIG), parent, (DLGPROC)DlgProc, (LPARAM)NULL);
}

void Quit()
{
    DoShutdownTasks();
}

void DoMenu(HMENU menu, LPTSTR param)
{
   if (*param != 0){
      LPTSTR action = param;
      LPTSTR string = strchr(param, ',');
      if (string) {
         *string = 0;
         string = SkipWhiteSpace(string+1);
      }
      else
         string = action;
      
      UINT command = 0;
      if (stricmp(action, "Config") == 0) {
         command = cmdConfig;
      }
      else if (stricmp(action, "ClearCookies") == 0) {
         command = cmdClearCookies;
      }
      else if (stricmp(action, "ClearHistory") == 0) {
         command = cmdClearHistory;
      }
      else if (stricmp(action, "ClearCache") == 0) {
         command = cmdClearCache;
      }
      else if (stricmp(action, "ClearMRU") == 0) {
         command = cmdClearMRU;
      }
      else if (stricmp(action, "ClearSignon") == 0) {
         command = cmdClearSignon;
      }
      if (command) {
         AppendMenu(menu, MF_STRING, command, string);
      }
   }
}

INT DoAccel(LPTSTR param)
{
    if (stricmp(param, "Config") == 0){
        return cmdConfig;
    }
    if (stricmp(param, "ClearCookies") == 0){
        return cmdClearCookies;
    }
    if (stricmp(param, "ClearHistory") == 0){
        return cmdClearHistory;
    }
    if (stricmp(param, "ClearCache") == 0){
        return cmdClearCache;
    }
    if (stricmp(param, "ClearMRU") == 0){
        return cmdClearMRU;
    }
    if (stricmp(param, "ClearSignon") == 0){
        return cmdClearSignon;
    }
    return 0;
}

void DoRebar(HWND rebarWnd)
{
}

LONG DoMessage(LPCTSTR to, LPCTSTR from, LPCTSTR subject, LONG data1, LONG data2)
{
   if (to[0] == '*' || stricmp(to, kPlugin.dllname) == 0)
   {
      if (stricmp(subject, "Init") == 0) {
         Init();
      }
      else if (stricmp(subject, "Create") == 0) {
         Create((HWND)data1);
      }
      else if (stricmp(subject, "Config") == 0) {
         Config((HWND)data1);
      }
      else if (stricmp(subject, "Quit") == 0) {
         Quit();
      }
      else if (stricmp(subject, "DoMenu") == 0) {
         DoMenu((HMENU)data1, (LPTSTR)data2);
      }
      else if (stricmp(subject, "DoRebar") == 0) {
         DoRebar((HWND)data1);
      }
      else if (stricmp(subject, "DoAccel") == 0) {
          *(PINT)data2 = DoAccel((LPTSTR)data1);
      }
      else return 0;

      return 1;
   }
   return 0;
}

// Main window procedure
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_COMMAND)
    {
        WORD command = LOWORD(wParam);
        if (command == cmdConfig)
        {
                Config(hWnd);
                return TRUE;
        }
        else if (command == cmdClearCookies)
        {
                ClearCookies();
                return TRUE;
        }
        else if (command == cmdClearHistory)
        {
                ClearHistory();
                return TRUE;
        }
        else if (command == cmdClearCache)
        {
                ClearCache();
                return TRUE;
        }
        else if (command == cmdClearMRU)
        {
                ClearMRU();
                return TRUE;
        }
        else if (command == cmdClearSignon)
        {
                ClearSignon();
                return TRUE;
        }
    }

    return CallWindowProc((WNDPROC)KMeleonWndProc, hWnd, message, wParam, lParam);
}

// Preferences dialog procedure
BOOL CALLBACK DlgProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

    switch (uMsg)
    {
        case WM_INITDIALOG:
            if (prefClearHistory & PRV_ONSTARTUP)
                CheckDlgButton(hWnd, CHK_HISTORY_STARTUP, BST_CHECKED);
            if (prefClearHistory & PRV_ONSHUTDOWN)
                CheckDlgButton(hWnd, CHK_HISTORY_SHUTDOWN, BST_CHECKED);
                
            if (prefClearCookies & PRV_ONSTARTUP)
                CheckDlgButton(hWnd, CHK_COOKIES_STARTUP, BST_CHECKED);
            if (prefClearCookies & PRV_ONSHUTDOWN)
                CheckDlgButton(hWnd, CHK_COOKIES_SHUTDOWN, BST_CHECKED);
                
            if (prefClearCache & PRV_ONSTARTUP)
                CheckDlgButton(hWnd, CHK_CACHE_STARTUP, BST_CHECKED);
            if (prefClearCache & PRV_ONSHUTDOWN)
                CheckDlgButton(hWnd, CHK_CACHE_SHUTDOWN, BST_CHECKED);
                
            if (prefClearMRU & PRV_ONSTARTUP)
                CheckDlgButton(hWnd, CHK_MRU_STARTUP, BST_CHECKED);
            if (prefClearMRU & PRV_ONSHUTDOWN)
                CheckDlgButton(hWnd, CHK_MRU_SHUTDOWN, BST_CHECKED);
                
            if (prefClearSignon & PRV_ONSTARTUP)
                CheckDlgButton(hWnd, CHK_SIGNON_STARTUP, BST_CHECKED);
            if (prefClearSignon & PRV_ONSHUTDOWN)
                CheckDlgButton(hWnd, CHK_SIGNON_SHUTDOWN, BST_CHECKED);
                
            break;
        case WM_COMMAND:
			switch(HIWORD(wParam)) {
				case BN_CLICKED:
					switch (LOWORD(wParam)) {
                        case BTN_CLEAR_COOKIES:
                            ClearCookies();
                            break;
                        case BTN_CLEAR_HISTORY:
                            ClearHistory();
                            break;
                        case BTN_CLEAR_CACHE:
                            ClearCache();
                            break;
                        case BTN_CLEAR_MRU:
                            ClearMRU();
                            break;
                        case BTN_CLEAR_SIGNON:
                            ClearSignon();
                            break;
                        case IDOK:
                            // History checkboxes
                            if (IsDlgButtonChecked(hWnd, CHK_HISTORY_STARTUP) == BST_CHECKED)
                                prefClearHistory |= PRV_ONSTARTUP;
                            else
                                prefClearHistory &= ~PRV_ONSTARTUP;
                            if (IsDlgButtonChecked(hWnd, CHK_HISTORY_SHUTDOWN) == BST_CHECKED)
                                prefClearHistory |= PRV_ONSHUTDOWN;
                            else
                                prefClearHistory &= ~PRV_ONSHUTDOWN;

                            // Cookies checkboxes
                            if (IsDlgButtonChecked(hWnd, CHK_COOKIES_STARTUP) == BST_CHECKED)
                                prefClearCookies |= PRV_ONSTARTUP;
                            else
                                prefClearCookies &= ~PRV_ONSTARTUP;
                            if (IsDlgButtonChecked(hWnd, CHK_COOKIES_SHUTDOWN) == BST_CHECKED)
                                prefClearCookies |= PRV_ONSHUTDOWN;
                            else
                                prefClearCookies &= ~PRV_ONSHUTDOWN;
                                
                            // Cache checkboxes
                            if (IsDlgButtonChecked(hWnd, CHK_CACHE_STARTUP) == BST_CHECKED)
                                prefClearCache |= PRV_ONSTARTUP;
                            else
                                prefClearCache &= ~PRV_ONSTARTUP;
                            if (IsDlgButtonChecked(hWnd, CHK_CACHE_SHUTDOWN) == BST_CHECKED)
                                prefClearCache |= PRV_ONSHUTDOWN;
                            else
                                prefClearCache &= ~PRV_ONSHUTDOWN;
                                
                            // MRU checkboxes
                            if (IsDlgButtonChecked(hWnd, CHK_MRU_STARTUP) == BST_CHECKED)
                                prefClearMRU |= PRV_ONSTARTUP;
                            else
                                prefClearMRU &= ~PRV_ONSTARTUP;
                            if (IsDlgButtonChecked(hWnd, CHK_MRU_SHUTDOWN) == BST_CHECKED)
                                prefClearMRU |= PRV_ONSHUTDOWN;
                            else
                                prefClearMRU &= ~PRV_ONSHUTDOWN;
                                
                            // Sign on checkboxes
                            if (IsDlgButtonChecked(hWnd, CHK_SIGNON_STARTUP) == BST_CHECKED)
                                prefClearSignon |= PRV_ONSTARTUP;
                            else
                                prefClearSignon &= ~PRV_ONSTARTUP;
                            if (IsDlgButtonChecked(hWnd, CHK_SIGNON_SHUTDOWN) == BST_CHECKED)
                                prefClearSignon |= PRV_ONSHUTDOWN;
                            else
                                prefClearSignon &= ~PRV_ONSHUTDOWN;
                                
                            SavePluginPreferences();
                        case IDCANCEL:
                            SendMessage(hWnd, WM_CLOSE, 0, 0);
                    }
            }
            break;
        case WM_CLOSE:
			EndDialog(hWnd, (LPARAM)NULL);
			break;
		default:
			return FALSE;
   }
   return TRUE;
}

extern "C"
{
   KMELEON_PLUGIN kmeleonPlugin *GetKmeleonPlugin()
   {
          return &kPlugin;
   }

   KMELEON_PLUGIN INT DrawBitmap(DRAWITEMSTRUCT *dis)
   {
          return 14; // 14 = icon width
   }
}