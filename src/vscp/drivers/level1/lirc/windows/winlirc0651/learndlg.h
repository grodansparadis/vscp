/* 
 * This file is part of the WinLIRC package, which was derived from
 * LIRC (Linux Infrared Remote Control) 0.5.4pre9.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Copyright (C) 1999 Jim Paris <jim@jtan.com>
 */

#if !defined(AFX_LEARNDLG_H__7119E9A0_C961_11D2_8C7F_004005637418__INCLUDED_)
#define AFX_LEARNDLG_H__7119E9A0_C961_11D2_8C7F_004005637418__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "globals.h"

#include "irdriver.h"
#include "config.h"
#include "remote.h"

typedef enum {lm_learn, lm_analyze, lm_raw} lm;

// learndlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// Clearndlg dialog

class Clearndlg : public CDialog
{
// Construction
public:
	Clearndlg(CIRDriver *ndrv, const char *nfilename, lm nlearn_mode, CWnd* pParent = NULL);
	~Clearndlg();

	CIRDriver *drv;
	const char *filename;
	FILE *fout;
	lm learn_mode;

	struct ir_remote *remotes;
	
	int get_codes(void);
	void get_pre_data(void);  //now matches lirc 0.6.5
	void get_post_data(void); //now matches lirc 0.6.5

	bool GetGap(unsigned long &gap,
				unsigned long &count,
				unsigned long &repeat_gap,
				unsigned long *repeat);
	bool GetRawButton(unsigned long *signals, int &count, bool waitgap);
	bool GetButton(unsigned long *signals, bool repeating);
	void DoGetGap(void);
	void DoGetButtons(void);

	void LearnThreadProc(void);
	void AnalyzeThreadProc(void);
	void RawThreadProc(void);

	void output(const char *s, bool replacelast=false);
	void input(const char *prompt, char *s, int maxlen, bool allowempty=false);
	
	/* Analysis */
	struct ir_ncode *current;
	int analyze(struct ir_remote *remotes);
	int get_sum(struct ir_remote *remote);
	int get_scheme(struct ir_remote *remote);
	int check_lengths(struct ir_remote *remote);
	struct lengths *new_length(unsigned long length);
	int add_length(struct lengths **first,unsigned long length);
	void free_lengths(struct lengths *first);
	void merge_lengths(struct lengths *first);
	void get_header_length(struct ir_remote *remote,struct lengths **first_pulse,
		struct lengths **first_space);
	unsigned long get_length(struct ir_remote *remote,struct lengths *first,
		unsigned long l);
	int is_bit(struct ir_remote *remote,unsigned long pulse, unsigned long space);
	int get_one_length(struct ir_remote *remote,struct lengths **first_pulse,
		struct lengths **first_space);
	int get_zero_length(struct ir_remote *remote,struct lengths **first_pulse,
		struct lengths **first_space);
	int get_trail_length(struct ir_remote *remote,struct lengths **first_pulse);
	int get_lengths(struct ir_remote *remote);
	int get_codes(struct ir_remote *remote);
	unsigned long readdata(unsigned long maxusec);
	void get_pre_data(struct ir_remote *remote);
	void get_post_data(struct ir_remote *remote);
	

	HANDLE GotInput;

// Dialog Data
	//{{AFX_DATA(Clearndlg)
	enum { IDD = IDD_LEARN };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(Clearndlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void EndDialog2( int nResult );

	// Generated message map functions
	//{{AFX_MSG(Clearndlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnEnter();
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LEARNDLG_H__7119E9A0_C961_11D2_8C7F_004005637418__INCLUDED_)
