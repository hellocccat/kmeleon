/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: NPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is 
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Chak Nanga <chak@netscape.com> 
 *
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the NPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the NPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef _DIALOGS_H_
#define _DIALOGS_H_

#include "resource.h"
#include "DialogEx.h"

class CBrowserView;
class CBrowserFrame;

#include <afxtempl.h>
class CFindRebar: public CReBar
{
public:
#ifndef _UNICODE
	CFindRebar(WCHAR* szSearch, PRBool bMatchCase,
				PRBool bMatchWholeWord, PRBool bWrapAround, 
				PRBool bHighlight, CBrowserFrame* pOwner);
#else
	CFindRebar(CString csSearchStr, PRBool bMatchCase,
		PRBool bMatchWholeWord, PRBool bWrapAround, 
		PRBool bHighlight, CBrowserFrame* pOwner);
#endif
	virtual ~CFindRebar();
	BOOL Create(CWnd* parent, DWORD dwStyle);
	void OnNotFound() {
		if (!m_NotFound) {
			m_NotFound=true;
			m_cEdit.Invalidate();
			m_cEdit.SetFocus();
			MessageBeep(MB_ICONASTERISK);
		}
	}
	void OnFound() {m_NotFound=false;Invalidate();m_cEdit.Invalidate();}
	inline CString GetFindString( ) const {return m_csSearchStr;}

	inline BOOL WrapAround() {return m_cToolbar.GetToolBarCtrl().IsButtonChecked(ID_WRAP_AROUND);}
	inline BOOL MatchCase() {return m_cToolbar.GetToolBarCtrl().IsButtonChecked(ID_MATCH_CASE);}
	inline BOOL Highlight() {return m_cToolbar.GetToolBarCtrl().IsButtonChecked(ID_HIGHLIGHT);}
	inline bool StartSel() {return m_bStartsel;}

	CEdit m_cEdit;

#ifndef _UNICODE
	inline const WCHAR* GetUFindString( ) const { return m_szUStr; }
#else 
	inline const WCHAR* GetUFindString( ) const { return m_csSearchStr; }
#endif


private:
	CString m_csSearchStr;
#ifndef _UNICODE
	WCHAR m_szUStr[256];
#endif
	PRBool m_bMatchCase;
	PRBool m_bMatchWholeWord;
	PRBool m_bWrapAround;
	PRBool m_bHighlight;
	UINT m_hid;

	CBrowserFrame* m_pOwner;
	CBrush m_brBkgnd;
	COLORREF m_clrBkgnd;
	
	CToolBar m_cToolbar;
	CToolBar closeBar;
	CImageList m_ilHot;
	CImageList m_ilCold;
	CImageList m_ilDead;

	bool m_NotFound;
	bool m_bStartsel;
	bool m_bAutoSearch; 

public:
	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void Close();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
#ifndef FINDBAR_USE_TYPEAHEAD
	afx_msg void OnEnChangeSearchStr();
#endif
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnWrapAround();
	afx_msg void OnMatchCase();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnUpdateNothing(CCmdUI* pCmdUI) { pCmdUI->Enable(); };

protected:
	virtual void PostNcDestroy();
};

class CFindDialog : public CFindReplaceDialog	
{
public:
	CFindDialog(CString& csSearchStr, PRBool bMatchCase,
				PRBool bMatchWholeWord, PRBool bWrapAround,
				PRBool bSearchBackwards, CBrowserView* pOwner);
	BOOL WrapAround();
	BOOL SearchBackwards();

private:
	CString m_csSearchStr;
	PRBool m_bMatchCase;
	PRBool m_bMatchWholeWord;
	PRBool m_bWrapAround;
	PRBool m_bSearchBackwards;
	CBrowserView* m_pOwner;

protected:
   virtual void OnCancel();
	virtual BOOL OnInitDialog();
	virtual void PostNcDestroy();
};

class CSelectDialog : public CDialog
{
//	DECLARE_DYNAMIC(CSelectDialog)

public:
	CSelectDialog(CWnd* pParent, LPCTSTR pTitle, LPCTSTR pText);   // constructeur standard
	virtual ~CSelectDialog();
	inline int GetChoice() {return m_iChoice;}
	void AddChoice(LPCTSTR);

// Donn�es de bo��e de dialogue
	enum { IDD = IDD_PROMPT_SELECT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // Prise en charge DDX/DDV
	CString m_csDialogTitle;
	CString m_csMsgText;
	CListBox m_cList;
	CList<CString, LPCTSTR> m_clChoices;
	int m_iChoice;
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	afx_msg void OnLbnDblclkListSelect();
};

#endif //_DIALOG_H_
