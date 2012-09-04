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
 * Copyright (C) 1998,99 Christoph Bartelmus <columbus@hit.handshake.de>
 * Copyright (C) 1999 Jim Paris <jim@jtan.com>
 */

#include "stdafx.h"
#include "winlirc.h"
#include "learndlg.h"
#include <sys/types.h>
#include <string.h>
#include "dumpcfg.h"
#include "config.h"
#include "remote.h"

#ifndef min
#define min(a,b) (a>b ? b:a)
#endif
#ifndef max
#define max(a,b) (a>b ? a:b)
#endif

unsigned int LearnThread(void *dlg) {((Clearndlg *)dlg)->LearnThreadProc();return 0;}
unsigned int AnalyzeThread(void *dlg) {((Clearndlg *)dlg)->AnalyzeThreadProc();return 0;}
unsigned int RawThread(void *dlg) {((Clearndlg *)dlg)->RawThreadProc();return 0;}


/////////////////////////////////////////////////////////////////////////////
// Clearndlg dialog


Clearndlg::Clearndlg(CIRDriver *ndrv, const char *nfilename, 
					 lm nlearn_mode, CWnd* pParent /*=NULL*/)
	: CDialog(Clearndlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(Clearndlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	drv=ndrv;
	filename=nfilename;
	learn_mode=nlearn_mode;
	fout=NULL;

	::learn_dialog=this;
}

Clearndlg::~Clearndlg()
{
	KillThread(&LearnThreadHandle,&LearnThreadEvent);
	if(fout!=NULL) { fclose(fout); fout=NULL; }
	if(GotInput) { CloseHandle(GotInput); GotInput=NULL; }
}

void Clearndlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(Clearndlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(Clearndlg, CDialog)
	//{{AFX_MSG_MAP(Clearndlg)
	ON_BN_CLICKED(IDC_BUTTON1, OnEnter)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_ENTER, OnEnter)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Clearndlg message handlers

BOOL Clearndlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	((CEdit *)GetDlgItem(IDC_OUT))->SetLimitText(0xFFFF);

	char *mode=(learn_mode==lm_learn)?"w":"r";
	
	if((fout=fopen(filename,mode))==NULL)
	{
		MessageBox("Could not open configuration file.");
		EndDialog2(IDCANCEL);
	}

	if((GotInput=CreateEvent(NULL,FALSE,FALSE,NULL))==NULL)
	{
		MessageBox("Could not create event.");
		EndDialog2(IDCANCEL);
	}

	/* THREAD_PRIORITY_IDLE combined with the REALTIME_PRIORITY_CLASS */
	/* of this program still results in a really high priority. (16 out of 31) */
	
	AFX_THREADPROC tp;
	switch(learn_mode)
	{
	case lm_learn:		tp=LearnThread; break;
	case lm_analyze:	tp=AnalyzeThread; break;
	case lm_raw:		tp=RawThread; break;
	}
	if((LearnThreadHandle=AfxBeginThread(tp,
		(void *)this,THREAD_PRIORITY_IDLE))==NULL)
	{
		CloseHandle(GotInput); GotInput=NULL;
		MessageBox("Could not initialize thread.");
		EndDialog2(IDCANCEL);
	}	
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void Clearndlg::output(const char *s, bool replacelast)
{
	CEdit *out=(CEdit *)GetDlgItem(IDC_OUT);
	if (out==NULL) return;
	if(s==NULL)
	{
		/* clear output */
		out->SetSel(0,-1);
		out->ReplaceSel("");
		return;
	}

	/* limit it to 120 characters, then add \r\n (if not replacing current line) */
	/* we don't add \r\n if replacing current line because that causes the box to */
	/* scroll up and down real quick when the last line is deleted and removed. */
	char t[256];
	_snprintf(t,120,"%s",s);
	t[120]=0;
	
	if(replacelast && out->GetLineCount()>1)
	{
		out->SetSel(out->LineIndex(out->GetLineCount()-2),out->GetWindowTextLength()-2);
	}
	else
	{
		strcat(t,"\r\n");
		out->SetSel(out->GetWindowTextLength(),out->GetWindowTextLength());
	}
	out->ReplaceSel(t);

	if(out->GetLineCount()>500)
	{
		/* start clearing out lines to avoid hitting the 64k limit */
		/* (122 * 500 = 61000) */
                out->SetSel(0,out->LineIndex(100),TRUE);
		out->ReplaceSel("");
	}
	out->SetSel(out->GetWindowTextLength(),out->GetWindowTextLength());
}

void Clearndlg::input(const char *prompt, char *s, int maxlen, bool allowempty)
{
	CEdit *in=(CEdit *)GetDlgItem(IDC_IN);
	CButton *ent=(CButton *)GetDlgItem(IDC_ENTER);

	ent->EnableWindow(true);
	in->SetSel(0,-1);
	in->ReplaceSel("");
	in->SetLimitText(maxlen);
	this->GotoDlgCtrl(in);
	
	int len;
	HANDLE events[2]={GotInput,LearnThreadEvent};
	do {
		output(prompt);
		ResetEvent(GotInput);
		int res=WaitForMultipleObjects(2,events,FALSE,INFINITE);
		if(res==(WAIT_OBJECT_0+1))
		{
			DEBUG("LearnThread terminating\n");
			AfxEndThread(0);
			return;
		}
		len=in->GetWindowTextLength();
	} while((!allowempty && len==0));
	s[len]=0;
	
	ent->EnableWindow(false);
	in->GetWindowText(s,maxlen);
	in->SetSel(0,-1);
	in->ReplaceSel("");
	in->SetLimitText(maxlen);
}

void Clearndlg::OnClose() 
{
	DEBUG("Killing LearnThread\n");
	KillThread(&LearnThreadHandle,&LearnThreadEvent);
	DEBUG("Closing GotInput event\n");
	if(GotInput) { CloseHandle(GotInput); GotInput=NULL; }
	DEBUG("Closing fout\n");
	if(fout!=NULL) fclose(fout); fout=NULL;
	DEBUG("Calling CDialog::OnClose()\n");
	CDialog::OnClose();
}

void Clearndlg::OnEnter() 
{
	SetEvent(GotInput);
}

bool Clearndlg::GetGap(unsigned long &gap,
					   unsigned long &count,
					   unsigned long &repeat_gap,
					   unsigned long *repeat)
{
	unsigned long data, average, signalcount;
	int mode;

	signalcount=average=mode=0;
	while(drv->readdata(250*1000,LearnThreadEvent)!=0)
		; /* wait for signals to stop coming for at least 0.25 sec */
	
	for(;;)
	{
		data=drv->readdata(30*1000*1000,LearnThreadEvent);

		if(data==0) return false;
		if(signalcount>MAX_SIGNALS) return false;

		if(is_space(data) && mode==0)
		{
			if(average==0)
			{
				if(data>100000) continue;
				average=data;
			}
			else
			{
				if(data>10*average ||
					(signalcount>10 && data>5*average))
				{
					count=signalcount;
					signalcount=0;
					gap=data;
					mode=1;
					continue;
				}
				average = (average*signalcount + data)/(signalcount+1);
			}
		}
		/* found gap, getting repeat code */
		if(mode==1)
		{
			if(signalcount<3) repeat[signalcount]=data&(PULSE_BIT-1);
			else 
			{
				if(data < (gap - gap*remote.eps/100))
				{
					/* if value is less than current gap, 
				    then it's probably not a repeat gap */
					DEBUG("repeat not detected, data=%d gap=%d\n",data,gap);
					repeat[0]=repeat[1]=repeat[2]=0;
				}
				else
				{
					/* otherwise, we'll assume it is */
					DEBUG("repeat detected, repeat_gap=%d\n",data);
					repeat_gap=data;
				}
				break;
			}
		}
		signalcount++;
	}
	return true;
}

void Clearndlg::DoGetGap(void)
{
	output("Step One: Determine signal gap, signal length, and repeat codes.");
	output("----------------------------------------------------------------"
		   "----------------------------------------------------------------");
	output("");
	output("You will be asked to press an arbitrary button a number of times.");
	output("Please hold it in for at least one second each time, and wait at");
	output("least one second between keypresses.");
	output("");
	output("If you want to manually enter a signal gap and signal length, you");
	output("may do so now (ie, \"31000 52\").  Otherwise, just hit ENTER.");

	/* First, read it until we get two results in a row that are pretty close. */
	int fault=0;
	unsigned long last_gap=0;
	unsigned long last_count=0;
	unsigned long last_repeat_gap=0;
	unsigned long last_repeat[3]={0,0,0};
	unsigned long gap, count, repeat_gap, repeat[3];
	int i;
	bool manualdata=false;
	char s[1024];

	
	input("Gap and length?",s,1024,true);
	if(strlen(s)!=0 && sscanf(s,"%ld %ld",&gap,&count)==2) manualdata=true;

	if(manualdata)
	{
		DEBUG("manual data entered, gap=%ld count=%ld\n",gap,count);
		code.length=count;
		remote.gap=gap;
		remote.repeat_gap=0;
		remote.prepeat=0;
		remote.srepeat=0;
		remote.ptrail=0;

		fprint_comment(fout,&remote);
		fprint_remote_head(fout,&remote);
		fprint_remote_signal_head(fout,&remote);
		return;
	}

	output("Press a button.");
	bool match=false;
	while(!match)
	{
		if(GetGap(gap,count,repeat_gap,repeat)==false)
		{
			DEBUG("GetGap failed\n");
			output("Error reading signal; please try again.");
			continue;
		}

		if(last_count==0)
		{
			for(i=0;i<3;i++) last_repeat[i]=repeat[i];
			last_count=count;
			last_gap=gap;
			last_repeat_gap=repeat_gap;
			output("Please wait a second and press it again.");
			continue;
		}

		match=true;
		/* match repeats */
		if(repeat[0]==0 && last_repeat[0]!=0) 
		{
			DEBUG("repeat[0] was zero but last_repeat[0] was %d\n",last_repeat[0]);
			match=false;
		}
		else if(last_repeat[0]==0 && repeat[0]!=0)
		{
			DEBUG("last_repeat[0] was zero but repeat[0] was %d\n",repeat[0]);
			match=false;
		}
		else {
			for(i=0;i<3;i++) 
				if(!expect(&remote,last_repeat[i],repeat[i])) 
				{
					DEBUG("repeat[%d]=%d doesn't match last_repeat[%d]=%d\n",
						i,repeat[i],i,last_repeat[i]);
					match=false;
				}
		}

		/* match counts */
		if(count!=last_count) 
		{
			DEBUG("Mismatched count: count=%d, last_count=%d\n",count,last_count);
			match=false;
		}

		/* match gaps */
		if(!expect(&remote,last_gap,gap))
		{
			DEBUG("gaps don't match: last_gap=%d, gap=%d\n",last_gap,gap);
			match=false;
		}

		/* match repeat gaps */
		if(!expect(&remote,last_repeat_gap,repeat_gap))
		{
			DEBUG("repeat gaps don't match: last_repeat_gap=%d, repeat_gap=%d\n",
				last_repeat_gap,repeat_gap);
			match=false;
		}


		if(!match)
		{
			for(i=0;i<3;i++) last_repeat[i]=repeat[i];
			last_count=count;
			last_gap=gap;
			last_repeat_gap=repeat_gap;
			fault++;
			if(fault>5)
			{
				MessageBox("Too many faults.  Possible reasons:\n"
					"1) This remote can't be automatically learned (sorry)\n"
					"2) The system is too slow or bogged down to accurately record\n"
					"      times (try closing all other programs)","Error");
				EndDialog2(IDCANCEL);
				DEBUG("LearnThread terminating\n");
				AfxEndThread(0);
				return;
			}
			DEBUG("Did not get consistent signal\n");
			output("Did not get a consistent signal; please try again.");
		}
	}

	/* Now, refine the information for gap and repeat */
	unsigned long avg_gap;
	unsigned long avg_repeat_gap;
	unsigned long avg_count;
	unsigned long avg_repeat[3];
	avg_count=count;
	avg_gap=(gap+last_gap)/2;
	avg_repeat_gap=(repeat_gap+last_repeat_gap)/2;
	for(i=0;i<3;i++) avg_repeat[i]=(repeat[i]+last_repeat[i])/2;
	int j=0;
	output("Baseline initialized.");
	while(j<10)
	{
		sprintf(s,"Please wait a second and press a button again (%d left)",10-j);
		output(s);
		if(GetGap(gap,count,repeat_gap,repeat)==false)
		{
			DEBUG("GetGap error\n");
			output("Error reading signal; please try again.");
			continue;
		}
		match=true;
		/* match repeats */
		if(repeat[0]==0 && avg_repeat[0]!=0) 
		{
			DEBUG("repeat[0] was zero but avg_repeat[0] was %d\n",avg_repeat[0]);
			match=false;
		}
		else if(avg_repeat[0]==0 && repeat[0]!=0)
		{
			DEBUG("avg_repeat[0] was zero but repeat[0] was %d\n",repeat[0]);
			match=false;
		}
		else if(avg_repeat[0]!=0) {
			for(i=0;i<3;i++) 
				if(!expect(&remote,avg_repeat[i],repeat[i])) 
				{
					DEBUG("repeat[%d]=%d doesn't match avg_repeat[%d]=%d\n",
						i,repeat[i],i,avg_repeat[i]);
					match=false;
				}
		}

		/* match counts */
		if(count!=avg_count) 
		{
			DEBUG("counts didn't match\n");
			match=false;
		}

		/* match gaps */
		if(!expect(&remote,avg_gap,gap))
		{
			DEBUG("gap was too far from old gap\n");
			match=false;
		}

		/* match repeat gaps */
		if(!expect(&remote,last_repeat_gap,repeat_gap))
		{
			DEBUG("repeat gaps was too far from old repeat gap\n");
			match=false;
		}


		if(!match)
		{
			fault++;
			if(fault>10)
			{
				MessageBox("Too many faults.  Possible reasons:\n"
					"1) This remote can't be automatically learned (sorry)\n"
					"2) The system is too slow or bogged down to accurately record\n"
					"      times (try closing all other programs)\n"
					"3) The detection got off to a bad start (try again)","Error");
				EndDialog2(IDCANCEL);
				DEBUG("LearnThread terminating\n");
				AfxEndThread(0);
				return;
			}
			output("Did not get a consistent signal.");
			continue;
		}
		else
		{
			avg_gap=((avg_gap*(j+2)) + gap)/(j+3);
			avg_repeat_gap=((avg_repeat_gap*(j+2)) + repeat_gap)/(j+3);
			for(i=0;i<3;i++) avg_repeat[i]=((avg_repeat[i]*(j+2)) + repeat[i])/(j+3);
			j++;
		}
	}

	/* Save it so far */
	code.length=avg_count;
	remote.gap=avg_gap;
	remote.repeat_gap=avg_repeat_gap;
	remote.prepeat=avg_repeat[0];
	remote.srepeat=avg_repeat[1];
	remote.ptrail=avg_repeat[2];

	fprint_comment(fout,&remote);
	fprint_remote_head(fout,&remote);
	fprint_remote_signal_head(fout,&remote);
}

bool Clearndlg::GetRawButton(unsigned long *signals, int &count, bool waitgap)
{
	count=0;
	unsigned long data;

	while(count<MAX_SIGNALS)
	{
		data=drv->readdata(waitgap?(30*1000*1000):(5*remote.gap),LearnThreadEvent);
		if(data==0) return false;

		if(waitgap)
		{
			if(!is_space(data) || data<remote.gap-remote.gap*remote.eps/100)
				return false;
			waitgap=false;
		}
		else
		{
			/* we've hit the end */
			if(is_space(data) && data>remote.gap-remote.gap*remote.eps/100)
				return true;

			signals[count]=data&(PULSE_BIT-1);
			count++;
		}
	}
	return false;
}

bool Clearndlg::GetButton(unsigned long *signals, bool repeating)
{
	/* read 64 samples if repeating    */
	/* read 8 samples if non-repeating */
	
	int count, i, j;
	unsigned long sig[MAX_SIGNALS+1];
	unsigned long last_sig[MAX_SIGNALS+1];
	
	while(drv->readdata(250*1000,LearnThreadEvent)!=0)
		; /* wait for signals to stop coming for at least 0.25 sec */

	int fault=0, expectfault=0;
	bool match=false;
	bool first=true;
	char s[256];

	if(repeating)
	{
		sprintf(s,"Please press and hold down the '%s' button until told to stop.",code.name);
		output(s);
	}
	else
	{
		sprintf(s,"Please repeatedly press and release the '%s' button until",code.name);
		output(s),
		output("told to stop.  You must wait at least half of a second");
		output("between keypresses or they will not be recognized.");
	}

	while(!match)
	{
		/* read two in a row that match */
		match=true;
		if(!repeating)
			while(drv->readdata(250*1000,LearnThreadEvent)!=0)
				; /* wait for signals to stop coming for at least 0.25 sec */
		if(!GetRawButton(sig,count,repeating?first:true))
		{
			DEBUG("GetRawButton failed\n");
			match=false;
		}
		if(count!=code.length)
		{
			DEBUG("Bad count: count=%d, code.length=%d\n",count,code.length);
			match=false;
		}
		for(i=0;i<code.length&&match;i++)
			if(!expect(&remote,last_sig[i],sig[i]))
			{
				expectfault++;
				DEBUG("signal too far from old signal\n");
				match=false;
			}
		if(!match)
		{
			fault++;
			if(fault>(repeating?10:5))
			{
				DEBUG("failed to get consistent signal\n");
				output("Stop.  Failed to get a consistent initial signal. "
					" Please try again.");
				if(expectfault>(repeating?8:4))
				{
					output("You will probably get better results if you");
					output("increase the margin of error for this remote.");
				}
				return false;
			}
			if(!repeating)
				output("No match yet; please continue");
		}
		first=false;
		for(i=0;i<code.length;i++)
			last_sig[i]=sig[i];
	}


	if(repeating)
		output("Baseline initialized.");
	else
		output("Baseline initialized.  Please continue to press the button.");
	fault=0;
	for(i=0;i<code.length;i++) signals[i]=(sig[i]+last_sig[i])/2;
	j=2;
	output(""); // need this to prevent last line from being overwritten
	while(j<(repeating?64:8))
	{
		sprintf(s,"matches=%d, faults=%d",j,fault);
		output(s,true);

		match=true;
		if(!repeating)
			while(drv->readdata(250*1000,LearnThreadEvent)!=0)
				; /* wait for signals to stop coming for at least 0.25 sec */
		if(!GetRawButton(sig,count,repeating?false:true))
		{
			DEBUG("GetRawButton failed\n");
			match=false;
		}
		if(count!=code.length)
		{
			DEBUG("Bad count: count=%d, code.length=%d\n",count,code.length);
			match=false;
		}
		for(i=0;i<code.length&&match;i++)
			if(!expect(&remote,signals[i],sig[i]))
			{
				DEBUG("signal too far from old signal\n");
				match=false;
			}
		if(!match)
		{
			fault++;
			if(fault>(repeating?64:12)) 
			{
				DEBUG("failed to get consistent signal\n");
				output("Stop.  Failed to get a consistent signal.  Please try again.");
				return false;
			}
		}
		else
		{
			for(i=0;i<code.length;i++)
				signals[i]=(signals[i]*j + sig[i])/(j+1);
			j++;
		}
	}
	sprintf(s,"matches=%d, faults=%d",j,fault);
	output(s,true);
	output("Stop.");
	return true;
}

void Clearndlg::DoGetButtons(void)
{
	output("Step Two: Input buttons.");
	output("----------------------------------------------------------------"
		   "----------------------------------------------------------------");
	output("");
	if(remote.prepeat==0)
	{
		DEBUG("this remote is a signal repeating remote\n");
		output("This is a signal-repeating remote with no special repeat code.");
		output("Holding down the button can quickly yield many copies of that "
			   "button's code.");
		output("Therefore, 64 samples of each button will be taken.");
	}
	else
	{
		DEBUG("this remote has a special repeat code\n");
		output("This remote has a special repeat code.");
		output("The button needs to be pressed and released for each new copy of that");
		output("button's code.  8 samples of each button will be taken.");
	}

	output("");
	output("You will be prompted to enter each button's name in turn.");
	output("To finish recording buttons, enter a blank button name.");
	output("");

	char buffer[BUTTON];
	unsigned long signals[MAX_SIGNALS+1];
	code.signals=signals;
	char s[1024];
	int button=1;

	for(;;)
	{
		if(button>1)
			sprintf(s,"Button %d name? (blank to stop)",button);
		else
			sprintf(s,"Button %d name?",button);
		input(s,buffer,BUTTON,true);
		
		if(strchr(buffer,' ') || strchr(buffer,'\t'))
		{
			output("A button name may not contain any whitespace.");
			continue;
		}
		if(strlen(buffer)==0) break;
		code.name=buffer;
		bool res=false;
		res=GetButton(signals,(remote.prepeat==0)?true:false);
		if(!res) 
		{
			DEBUG("GetButton failed\n");
			continue;
		}
		output("");output("");
		sprintf(s,"Button '%s' recorded.  Do you wish to keep this recording?",buffer);
		if(MessageBox(s,"Keep it?",MB_YESNO)==IDNO) continue;
		fprint_remote_signal(fout,&remote,&code);
		button++;
	}
	fprint_remote_signal_foot(fout,&remote);
	fprint_remote_foot(fout,&remote);
}

void Clearndlg::LearnThreadProc(void)
{
	char s[256];
	char brand[64];
	unsigned long data;
		
	output("This will record the signals from your remote control");
	output("and create a config file for WinLIRC.");
	output("");
	remote.flags=RAW_CODES;
	remote.aeps=AEPS;
	do {
		input("Please enter a name for this remote.",brand,63);	
		if(strchr(brand,' ') || strchr(brand,'\t'))
		{
			output("The remote name may not contain any whitespace.");
			brand[0]=0;
		}
	} while(brand[0]==0);
		
	remote.name=brand;
	output(brand);
	output("");
	output("When learning and analyzing signals, a margin of error is used in order to");
	output("handle the normal variations in the received signal.  The margin of error");
	sprintf(s,"ranges from 1%% to 99%%.  The default is %d%%, but larger values might",EPS);
	output(s);
	output("be necessary depending on your hardware and software.  If you are having");
	output("trouble using your remote, try increasing this value.  You may enter the");
	output("allowable margin of error now, or press ENTER to use the default.");
	output("");
	int user_eps=0;
	do {
		sprintf(s,"Desired margin of error for this remote? (1-99, enter=%d)",EPS);
		input(s,s,63,true);
		if(strlen(s)==0) user_eps=EPS;
		else user_eps=atoi(s);
	} while(user_eps<1 || user_eps>99);
	
	remote.eps=user_eps;
	
	/* Clear out the buffer */
	while(drv->GetData(&data)==true)
		;

	output(NULL);
	DoGetGap();
	
	output(NULL);
	DoGetButtons();
	
	MessageBox("Configuration successfully written.","Success");
	EndDialog2(IDOK);
	DEBUG("LearnThread terminating\n");
	AfxEndThread(0);
	return;
}

void Clearndlg::RawThreadProc(void)
{
	output("Outputting raw mode2 data.");
	DEBUG("Raw mode2 data:\n");
	output("");

	unsigned long int x;
	char s[256];
	for(;;)
	{
		x=drv->readdata(0,LearnThreadEvent);
		if(x&PULSE_BIT)
			sprintf(s,"pulse %ld",x&~PULSE_BIT);
		else
			sprintf(s,"space %ld",x&~PULSE_BIT);
		output(s);
		strcat(s,"\n");
		DEBUG(s);
	}
}

void Clearndlg::AnalyzeThreadProc(void)
{
	output("Analyzing data, please wait...");

	/* Read it in */
	struct ir_remote *myremotes, *sr;
	myremotes=read_config(fout);

	/* See if it's OK */
	bool ok=true;
	if(myremotes==(struct ir_remote *)-1 || myremotes==NULL)
		ok=false;
	for(sr=myremotes;sr!=NULL&&ok;sr=sr->next)
	{
		if(sr->codes==NULL)
		{
			free_config(myremotes);
			myremotes=NULL;
			ok=false;
		}
	}
	if(!ok)
	{
		MessageBox("Error parsing configuration file.\n","Error");
		EndDialog2(IDCANCEL);
		DEBUG("AnalyzeThread terminating\n");
		AfxEndThread(0);
		return;
	}

	fclose(fout); fout=NULL;

	/* Now analyze it */
	if(analyze(myremotes)==-1)
	{
		free_config(myremotes);
		myremotes=NULL;
		MessageBox("Analysis failed.  This remote is probably only\n"
			"supported in raw mode.  Configuration file is unchanged.","Error");
		EndDialog2(IDCANCEL);
		DEBUG("AnalyzeThread terminating\n");
		AfxEndThread(0);
		return;
	}

	/* Yay, success */
	
	if((fout=fopen(filename,"w"))==NULL)
	{
		free_config(myremotes);
		myremotes=NULL;
		MessageBox("Analysis succeeded, but re-open of file failed."
			"Configuration file is unchanged.","Error");
		EndDialog2(IDCANCEL);
		DEBUG("AnalyzeThread terminating\n");
		AfxEndThread(0);
		return;
	}

	fprint_remotes(fout,myremotes);
	free_config(myremotes);
	myremotes=NULL;

	MessageBox("Analysis successful.\n","Success");

	EndDialog2(IDOK);
	DEBUG("AnalyzeThread terminating\n");
	AfxEndThread(0);
	return;
}

/* Analysis stuff from here down */

int Clearndlg::analyze(struct ir_remote *remotes)
{
	int scheme;

	while(remotes!=NULL)
	{
		if(remotes->flags&RAW_CODES)
		{
			scheme=get_scheme(remotes);
			switch(scheme)
			{
			case 0:
				output("Config file does not contain any buttons.");
				return(-1);
				break;
			case SPACE_ENC:
				if(-1==check_lengths(remotes))
				{
					output("check_lengths failed");
					return(-1);
				}
				if(-1==get_lengths(remotes))
				{
					output("get_lengths failed");
					return(-1);
				}
				remotes->flags&=~RAW_CODES;
				remotes->flags|=SPACE_ENC;
				if(-1==get_codes(remotes))
				{
					remotes->flags&=~SPACE_ENC;
					remotes->flags|=RAW_CODES;
					output("get_codes failed");
					return(-1);
				}
				get_pre_data(remotes);
				get_post_data(remotes);
				break;
			case SHIFT_ENC:
				output("Shift encoded remotes are not supported yet.");
				return(-1);
				break;
			}
		}
		remotes=remotes->next;
	}
	return(0);
}

int Clearndlg::get_sum(struct ir_remote *remote)
{
	int sum,i;
	struct ir_ncode *codes;

	sum=0;
	codes=remote->codes;
	for(i=0;codes[i].name!=NULL;i++)
	{
		sum++;
	}
	return(sum);
}

int Clearndlg::get_scheme(struct ir_remote *remote)
{
	struct ir_ncode *codes;
	int match,sum,i,j;

	sum=get_sum(remote);
	if(sum==0)
	{
		return(0);
	}
	codes=remote->codes;
	for(i=0;i<sum;i++)
	{
		match=0;
		for(j=i+1;j<sum;j++)
		{
			if(codes[i].length==codes[j].length)
			{
				match++;
				/* I want less than 20% mismatches */
				if(match>=80*sum/100) 
				{
					/* this is not yet the
					   number of bits */
					remote->bits=codes[i].length;
					return(SPACE_ENC);
				}
			}
		}
	}
	return(SHIFT_ENC);
}

int Clearndlg::check_lengths(struct ir_remote *remote)
{
	int i,flag;
	struct ir_ncode *codes;

	flag=0;
	codes=remote->codes;
	for(i=0;codes[i].name!=NULL;i++)
	{
		if(codes[i].length!=remote->bits)
		{
			char s[128];
			_snprintf(s,127,"Button \"%s\" has wrong signal length.\r\n"
				"Try to record it again.",codes[i].name);
			output(s);
			flag=-1;
		}
	}
	return(flag);
}

struct lengths *Clearndlg::new_length(unsigned long length)
{
	struct lengths *l;

	l=(struct lengths *)malloc(sizeof(struct lengths));
	if(l==NULL) return(NULL);
	l->count=1;
	l->sum=length;
	l->lower_bound=length/100*100;
	l->upper_bound=length/100*100+99;
	l->min=l->max=length;
	l->next=NULL;
	return(l);
}

int Clearndlg::add_length(struct lengths **first,unsigned long length)
{
	struct lengths *l,*last;

	if(*first==NULL)
	{
		*first=new_length(length);
		if(*first==NULL) return(-1);
		return(0);
	}
	l=*first;
	while(l!=NULL)
	{
		if(l->lower_bound<=length && length<=l->upper_bound)
		{
			l->count++;
			l->sum+=length;
			l->min=min(l->min,length);
			l->max=max(l->max,length);
			return(0);
		}
		last=l;
		l=l->next;
	}
	last->next=new_length(length);
	if(last->next==NULL) return(-1);
	return(0);
}

void Clearndlg::free_lengths(struct lengths *first)
{
	struct lengths *next;

	if(first==NULL) return;
	while(first!=NULL)
	{
		next=first->next;
		free(first);
		first=next;
	}
}

void Clearndlg::merge_lengths(struct lengths *first)
{
	struct lengths *l,*inner,*last;
	unsigned long new_sum;
	int new_count;

	l=first;
	while(l!=NULL)
	{
		last=l;
		inner=l->next;
		while(inner!=NULL)
		{
			new_sum=l->sum+inner->sum;
			new_count=l->count+inner->count;
			
			if((l->max<=new_sum/new_count+AEPS &&
			    l->min>=new_sum/new_count-AEPS &&
			    inner->max<=new_sum/new_count+AEPS &&
			    inner->min>=new_sum/new_count-AEPS)
			   ||
			   (l->max<=new_sum/new_count*(100+EPS) &&
			    l->min>=new_sum/new_count*(100-EPS) &&
			    inner->max<=new_sum/new_count*(100+EPS) &&
			    inner->min>=new_sum/new_count*(100-EPS)))
			{
				l->sum=new_sum;
				l->count=new_count;
				l->upper_bound=max(l->upper_bound,
						   inner->upper_bound);
				l->lower_bound=min(l->lower_bound,
						   inner->lower_bound);
				l->min=min(l->min,inner->min);
				l->max=max(l->max,inner->max);
				
				last->next=inner->next;
				free(inner);
				inner=last;
			}
			last=inner;
			inner=inner->next;
		}
		l=l->next;
	}
}

void Clearndlg::get_header_length(struct ir_remote *remote,struct lengths **first_pulse,
		       struct lengths **first_space)
{
	unsigned int sum,match,i;
	struct lengths *p,*s,*plast,*slast;
	struct ir_ncode *codes;

	sum=get_sum(remote);
	p=*first_pulse;
	plast=NULL;
	while(p!=NULL)
	{
		if(p->count>=90*sum/100)
		{
			s=*first_space;
			slast=NULL;
			while(s!=NULL)
			{
				if(s->count>=90*sum/100 &&
				   (p->count<=sum || s->count<=sum))
				{
					
					codes=remote->codes;
					match=0;
					for(i=0;codes[i].name!=NULL;i++)
					{
						if(expect(remote,
							  remote->codes[i].signals[0],
							  (int) (p->sum/p->count))
						   &&
						   expect(remote,
							  remote->codes[i].signals[1],
							  (int) (s->sum/s->count)))
						{
							match++;
						}
					}
					if(match>=sum*90/100)
					{
						remote->phead=p->sum/p->count;
						remote->shead=s->sum/s->count;
						p->sum-=sum*p->sum/p->count;
						s->sum-=sum*s->sum/s->count;
						p->count-=sum;
						s->count-=sum;
						if(p->count<=0)
						{
							if(plast==NULL)
							{
								plast=*first_pulse;
								*first_pulse=plast->next;
								free(plast);
							}
							else
							{
								plast->next=p->next;
								free(p);
							}
						}
						if(s->count<=0)
						{
							if(slast==NULL)
							{
								slast=*first_space;
								*first_space=slast->next;
								free(slast);
							}
							else
							{
								slast->next=s->next;
								free(s);
							}
						}
						return;
					}
				}
				slast=s;
				s=s->next;
			}
		}
		plast=p;
		p=p->next;
	}
}

unsigned long Clearndlg::get_length(struct ir_remote *remote,struct lengths *first,
			 unsigned long l)
{
	while(first!=NULL)
	{
		if(expect(remote,l,first->sum/first->count))
		{
			return(first->sum/first->count);
		}
		first=first->next;
	}
	return(0);
}

int Clearndlg::is_bit(struct ir_remote *remote,unsigned long pulse, unsigned long space)
{
	int i,j,match,sum;
	struct ir_ncode *codes;

	sum=get_sum(remote);
	match=0;
	codes=remote->codes;
	for(i=0;codes[i].name!=NULL;i++)
	{
		for(j=has_header(remote) ? 2:0;j+2<codes[i].length;j+=2)
		{
			if(expect(remote,codes[i].signals[j],pulse) &&
			   expect(remote,codes[i].signals[j+1],space))
			{
				match++;
			}
		}
	}
	sum*=(remote->bits-1-(has_header(remote) ? 2:0));
	sum/=2;
	if(match>=20*sum/100)
	{
		return(1);
	}
	return(0);
}

int Clearndlg::get_one_length(struct ir_remote *remote,struct lengths **first_pulse,
		    struct lengths **first_space)
{
	int i,j;
	struct ir_ncode *codes;
	unsigned long pulse,space;

	codes=remote->codes;
	for(i=0;codes[i].name!=NULL;i++)
	{
		j=has_header(remote) ? 2:0;
		pulse=get_length(remote,*first_pulse,codes[i].signals[j]);
		space=get_length(remote,*first_space,codes[i].signals[j+1]);

		if(pulse!=0 && space!=0)
		{
			if(is_bit(remote,pulse,space))
			{
				remote->pone=pulse;
				remote->sone=space;
				return(0);
			}
		}
	}
	return(-1);
}

int Clearndlg::get_zero_length(struct ir_remote *remote,struct lengths **first_pulse,
		    struct lengths **first_space)
{
	int i,j;
	struct ir_ncode *codes;
	unsigned long pulse,space;

	codes=remote->codes;
	for(i=0;codes[i].name!=NULL;i++)
	{
		for(j=has_header(remote) ? 2:0;j+2<codes[i].length;j+=2)
		{

			if(expect(remote,codes[i].signals[j],
				  remote->pone)==0 
			   || expect(remote,codes[i].signals[j+1],
				     remote->sone)==0)
			{
				pulse=get_length(remote,*first_pulse,
						 codes[i].signals[j]);
				space=get_length(remote,*first_space,
						 codes[i].signals[j+1]);
				if(is_bit(remote,pulse,space))
				{
					remote->pzero=pulse;
					remote->szero=space;
					return(0);
				}
			}
		}
	}
	return(-1);
}

int Clearndlg::get_trail_length(struct ir_remote *remote,struct lengths **first_pulse)
{
	unsigned int sum,match,i;
	struct lengths *p,*plast;
	struct ir_ncode *codes;

	sum=get_sum(remote);
	p=*first_pulse;
	plast=NULL;
	while(p!=NULL)
	{
		if(p->count>=sum)
		{
			codes=remote->codes;
			match=0;
			for(i=0;codes[i].name!=NULL;i++)
			{
				if(expect(remote,
					  remote->codes[i].signals[remote->codes[i].length-1],
					  (int) (p->sum/p->count)))
				{
					match++;
				}
			}
			if(match>=sum*90/100)
			{
				remote->ptrail=p->sum/p->count;
				p->sum-=sum*p->sum/p->count;
				p->count-=sum;
				if(p->count==0)
				{
					if(plast==NULL)
					{
						plast=*first_pulse;
						*first_pulse=plast->next;
						free(plast);
					}
					else
					{
						plast->next=p->next;
						free(p);
					}
				}
				return(0);
			}
		}
		plast=p;
		p=p->next;
	}
	return(-1);
}

int Clearndlg::get_lengths(struct ir_remote *remote)
{
	struct lengths *first_space=NULL,*first_pulse=NULL;
	int i,j;
	struct ir_ncode *codes;

	/* get all spaces */

	codes=remote->codes;
	for(i=0;codes[i].name!=NULL;i++)
	{
		for(j=1;codes[i].signals[j]!=0;j+=2)
		{
			if(-1==add_length(&first_space,codes[i].signals[j]))
			{
				free_lengths(first_space);
				output("add_length failed");
				return(-1);
			}
		}
	}
	merge_lengths(first_space);

	/* and now all pulses */

	codes=remote->codes;
	for(i=0;codes[i].name!=NULL;i++)
	{
		for(j=0;j<codes[i].length;j+=2)
		{
			if(-1==add_length(&first_pulse,codes[i].signals[j]))
			{
				free_lengths(first_space);
				free_lengths(first_pulse);
				output("add_length failed");
				return(-1);
			}
		}
	}
	merge_lengths(first_pulse);
	
	get_header_length(remote,&first_pulse,&first_space);
	if(-1==get_trail_length(remote,&first_pulse))
	{
		free_lengths(first_space);
		free_lengths(first_pulse);
		output("get_trail_length failed");
		return(-1);
	}
	if(-1==get_one_length(remote,&first_pulse,&first_space))
	{
		free_lengths(first_space);
		free_lengths(first_pulse);
		output("get_one_length failed");
		return(-1);
	}
	if(-1==get_zero_length(remote,&first_pulse,&first_space))
	{
		free_lengths(first_space);
		free_lengths(first_pulse);
		output("get_zero_length failed");
		return(-1);
	}

	remote->bits--;
	if(has_header(remote)) remote->bits-=2;
	remote->bits/=2;
	if(remote->bits>64) /* can't handle more than 64 bits in normal mode */
	{
		free_lengths(first_space);
		free_lengths(first_pulse);
		output("bits>64");
		return(-1);
	}
#ifndef LONG_IR_CODE
	if(remote->bits>32)
	{
		output("this remote control sends more than 32 bits");
		output("recompile the package using LONG_IR_CODE");
		return(-1);
	}
#endif

	free_lengths(first_space);
	free_lengths(first_pulse);
	return(0);
}

int Clearndlg::get_codes(struct ir_remote *remote)
{
	struct ir_ncode *codes;

	codes=remote->codes;
	while(codes->name!=NULL)
	{
		rec_buffer.rptr=rec_buffer.wptr=0;
		clear_rec_buffer(remote->gap);

		current=codes;
		use_ir_hardware=false;
		if(decode(remote))
		{
			codes->code=remote->post_data;
			remote->post_data=0;
		}
		else
		{
			return(-1);
		}
		codes++;
	}
	return(0);
}

unsigned long Clearndlg::readdata(unsigned long maxusec)
{
	static int i=0;
	static struct ir_ncode *codes=NULL;
	unsigned long data;

	if(codes!=current)
	{
		i=0;
		codes=current;
	}

	if(i<current->length)
	{
		data=current->signals[i];
		i++;
		return(i%2 ? data|PULSE_BIT:data);
	}
	return(0);
}

void Clearndlg::get_pre_data(struct ir_remote *remote) 
{
	struct ir_ncode *codes;
	ir_code mask,last;
	int count,i;
	
	if(remote->bits==0) return;
	mask=(-1);
	codes=remote->codes;
	if(codes->name==NULL) return; /* at least 2 codes needed */
	last=codes->code;
	codes++;
	if(codes->name==NULL) return; /* at least 2 codes needed */
	while(codes->name!=NULL)
	{
		mask&=~(last^codes->code);
		last=codes->code;
		codes++;
	}
	count=0;
#ifdef LONG_IR_CODE
	while(mask&0x8000000000000000)
#else
	while(mask&0x80000000)
#endif
	{
		count++;
		mask=mask<<1;
	}
	count-=sizeof(ir_code)*CHAR_BIT-remote->bits;

	/* only "even" numbers should go to pre/post data */
	if(count%8 && (remote->bits-count)%8)
	{
		count-=count%8;
	}
	if(count>0)
	{
		mask=0;
		for(i=0;i<count;i++)
		{
			mask=mask<<1;
			mask|=1;
		}
		remote->bits-=count;
		mask=mask<<(remote->bits);
		remote->pre_data_bits=count;
		remote->pre_data=(last&mask)>>(remote->bits);

		codes=remote->codes;
		while(codes->name!=NULL)
		{
			codes->code&=~mask;
			codes++;
		}
	}
}

void Clearndlg::get_post_data(struct ir_remote *remote)
{
	struct ir_ncode *codes;
	ir_code mask,last;
	int count,i;
	
	if(remote->bits==0) return;

	mask=(-1);
	codes=remote->codes;
	if(codes->name==NULL) return; /* at least 2 codes needed */
	last=codes->code;
	codes++;
	if(codes->name==NULL) return; /* at least 2 codes needed */
	while(codes->name!=NULL)
	{
		mask&=~(last^codes->code);
		last=codes->code;
		codes++;
	}
	count=0;
	while(mask&0x1)
	{
		count++;
		mask=mask>>1;
	}
	/* only "even" numbers should go to pre/post data */
	if(count%8 && (remote->bits-count)%8)
	{
		count-=count%8;
	}
	if(count>0)
	{
		mask=0;
		for(i=0;i<count;i++)
		{
			mask=mask<<1;
			mask|=1;
		}
		remote->bits-=count;
		remote->post_data_bits=count;
		remote->post_data=last&mask;

		codes=remote->codes;
		while(codes->name!=NULL)
		{
			codes->code=codes->code>>count;
			codes++;
		}
	}
}

void Clearndlg::EndDialog2( int nResult )
{
	if(fout!=NULL) { fclose(fout); fout=NULL; }
	if (nResult==IDOK)
		PostMessage(WM_COMMAND,IDOK,0);
	else
		PostMessage(WM_SYSCOMMAND,SC_CLOSE,0);
}
