/*******************************************************************
 *
 *    DESCRIPTION: 	
 *    AUTHOR: 		
 *
 *    HISTORY:    
 *
 *******************************************************************/

#ifndef __EDT_MNG_H
#define __EDT_MNG_H

/*---------------------------------------------------------------------------*/
/*				DEFINIZIONE STATI DELLA MACCHINA 				             */
/*---------------------------------------------------------------------------*/
#define EDIT_WELCOME_STATE			  	0	
#define EDIT_COMM_STATE				  	1	
#define EDIT_PRESET_STATE			  	2	
#define EDIT_PLUGINS_STATE			  	3	
#define EDIT_VOLUME_STATE			  	4	
#define EDIT_MUTE_STATE				  	5	

//mod.luca da togliere o rivedere dopo aver inserito la nuova gestione della seriale
#define	COMM_PANELON_STATE		 		0
#define	COMM_PANELOFF_STATE				1
/*---------------------------------------------------------------------------*/
/*				DEFINIZIONI GENERALI            				             */
/*---------------------------------------------------------------------------*/
#define	OSRELEASE		10	//define O.S. release 10 significa 1.0
#define	PROEL_VENDOR	0

	//#define	NEOS10_SAT   	
	#define	NEOS12_SAT
//#define	NEOS15_SAT
//#define	NEOS122_SAT
//#define	NEOS152_SAT
//#define	NEOS212_SAT
	//#define	NEOS115_SUB 
//#define	NEOS118_SUB 
//#define	NEOS215_SUB 
//#define	NEOS218_SUB

// COSTANTI GENERALI DEL DEVICE
// device type
#define	FIRM_NEOS10   	0
#define	FIRM_NEOS12   	1
#define	FIRM_NEOS15   	2
#define	FIRM_NEOS122  	3
#define	FIRM_NEOS152  	4
#define	FIRM_NEOS212  	5
#define	FIRM_NEOS115SUB 6
#define	FIRM_NEOS118SUB 7
#define	FIRM_NEOS215SUB 8
#define	FIRM_NEOS218SUB 9

// Definizioni dipendenti dal tipo di Device (Type, numero PRESET,...)
#ifdef	NEOS10_SAT 
	#define NPRESET_FACTORY 		3
	#define	DEV_TYPE				FIRM_NEOS10
	#define NINPUTCHANNEL			2
	#define NOUTPUTCHANNEL			2
#endif
#ifdef	NEOS12_SAT
	#define NPRESET_FACTORY 		3
	#define	DEV_TYPE				FIRM_NEOS12
	#define NINPUTCHANNEL			2
	#define NOUTPUTCHANNEL			2
#endif
#ifdef	NEOS15_SAT
	#define NPRESET_FACTORY 		3
	#define	DEV_TYPE				FIRM_NEOS15
	#define NINPUTCHANNEL			2
	#define NOUTPUTCHANNEL			2
#endif
#ifdef	NEOS122_SAT
	#define NPRESET_FACTORY 		3
	#define	DEV_TYPE				FIRM_NEOS122
	#define NINPUTCHANNEL			2
	#define NOUTPUTCHANNEL			2
#endif
#ifdef	NEOS152_SAT
	#define NPRESET_FACTORY 		3
	#define	DEV_TYPE				FIRM_NEOS152
	#define NINPUTCHANNEL			2
	#define NOUTPUTCHANNEL			2
#endif
#ifdef	NEOS212_SAT
	#define NPRESET_FACTORY 		3
	#define	DEV_TYPE				FIRM_NEOS212
	#define NINPUTCHANNEL			2
	#define NOUTPUTCHANNEL			2
#endif
#ifdef	NEOS115_SUB
	#define NPRESET_FACTORY 		2
	#define	DEV_TYPE				FIRM_NEOS115SUB
	#define NINPUTCHANNEL			2
	#define NOUTPUTCHANNEL			2
#endif
#ifdef	NEOS118_SUB
	#define NPRESET_FACTORY 		2
	#define	DEV_TYPE				FIRM_NEOS118SUB
	#define NINPUTCHANNEL			2
	#define NOUTPUTCHANNEL			2
#endif
#ifdef	NEOS215_SUB
	#define NPRESET_FACTORY 		2
	#define	DEV_TYPE				FIRM_NEOS215SUB
	#define NINPUTCHANNEL			2
	#define NOUTPUTCHANNEL			2
#endif
#ifdef	NEOS218_SUB
	#define NPRESET_FACTORY 		2
	#define	DEV_TYPE				FIRM_NEOS218SUB
	#define NINPUTCHANNEL			2
	#define NOUTPUTCHANNEL			2
#endif

#define NPRESET_USER    		16
#define NMAXPRESET_FACTORY		3

#define PRESET_GROUP_FACTORY 	0
#define PRESET_GROUP_USER    	1

// Altre costanti
#define MAXPAREQCELLNUM			7
#define MAXDYNEQCELLNUM			3

#define DEVICENAMELENGTH		12 //lunghezza a byte
#define PRESETNAMELENGTH		12 //lunghezza a byte
#define DEVICENAMEPACKEDLENGTH	3  // Lunghezza nome device impacchettato in word a 32 bit
#define PASSWORDNAMEPACKEDLENGTH	3  // Lunghezza password impacchettata in word a 32 bit
#define PRESETNAMEPACKEDLENGTH 	3  // Lunghezza nome preset impacchettato in word a 32 bit
//.....................................................................................................
enum 
{
    TOTALLOCK_LEVEL = 0,	// total Lock (edit only mute)
    
	PRESETMODE_LEVEL,		// preset mode (load preset and mute)
    PARTIALLOCK_LEVEL,		// partial Lock (input edit)
    FULLEDIT_LEVEL,			// full edit (input ch. + output ch. edit,
							// for loudspeaker board edit input ch. and only output level)
    USER4_LEVEL,
    
	SUPERVISOR1_LEVEL,		// lock level for preset: Total lock
    SUPERVISOR2_LEVEL,		// lock level for preset: partial lock
    SUPERVISOR3_LEVEL,		// lock level for preset: full edit
    SUPERVISOR4_LEVEL,		
    
	FACTORY1_LEVEL,			// factory level: edit all parameters
    FACTORY2_LEVEL,
    FACTORY3_LEVEL,
    FACTORY4_LEVEL,
    
	RDSUPREME_LEVEL,		// R&D mode enable advancer edit options and parameters

	NUMDLOCK_LEVEL
}LOCKLEVEL;
//.....................................................................................................
//channels
#define CH_INA 				0
#define CH_INB 				1
#define CH_INAB 			2
#define CH_SUM 				3
//.....................................................................................................
//routing
#define INROUTING1			0
#define INROUTING2			1
#define INROUTING12			2
/*
#define CH_OUT1				4
#define CH_OUT2 			5
#define CH_OUT3 			6
#define CH_OUT4 			7
#define CH_OUT5 			8
#define CH_OUT6 			9
*/

//.....................................................................................................
// norm coeff
#define DSPNORMIN1_COEFF			150	//0dB	//1.0
#define DSPNORMIN2_COEFF			150	//0dB	//1.0
#define DSPNORMOUT1_COEFF			150			//1.0
#define DSPNORMOUT2_COEFF			150			//1.0
//.....................................................................................................
// gain level phase mute
#define MUTE_OFF				0
#define MUTE_ON					1

#define POLARITY_NORMAL 		0
#define POLARITY_REVERSE		1

#define LEVINGAIN_0dB 			150
#define LEVINLEVEL_0dB 			150
#define	LEVINGAIN_100msDELTA	1e-4		//delta per rampa crossfade bypass a 10ms@96kHz
#define LEVOUTGAIN_0dB			150
#define LEVOUTLEVEL_0dB			30
#define	LEVOUTGAIN_100msDELTA	1e-4		//delta per rampa crossfade bypass a 10ms@96kHz
#define LEVOUTTRIM_0dB 			30
//.....................................................................................................
// static eq: parametric filter
#define USEDPAREQCELLNUM		5

enum 
{
	EQ_PEAK = 0,
	EQ_NOTCH,
	EQ_LOSH6,
	EQ_LOSH12,
	EQ_LOSHVAR,
	EQ_HISH6,
	EQ_HISH12,
	EQ_HISHVAR,
	EQ_RESLP,
	EQ_RESBP,
	EQ_RESHP,
	EQ_AP1,
	EQ_AP2,
	EQ_APV,

	EQ_LPBUT4_CELL1,
	EQ_LPBUT4_CELL2,
	EQ_LPBUT8_CELL1,
	EQ_LPBUT8_CELL2,
	EQ_LPBES8_CELL1,
	EQ_LPBES8_CELL2,
	EQ_HPBUT4_CELL1,
	EQ_HPBUT4_CELL2,
	EQ_HPBUT8_CELL1,
	EQ_HPBUT8_CELL2,
	EQ_HPBES8_CELL1,
	EQ_HPBES8_CELL2,

	NUMPAREQTAB
}ENUMPAREQTYPE;

#define EQGAIN_0dB				76
#define EQGAIN_P15dB			151
#define EQGAIN_N15dB			1

#define CELL_NOTACTIVE			0
#define CELL_ACTIVE				1

#define PEQCELL_ACTIVE			0
#define PEQCELL_FLAT			1

#define FREQ_15p6 				0
#define FREQ_30p2 				19
#define FREQ_62p5 				40	
#define FREQ_79p7 				47	
#define FREQ_82p5 				48
#define FREQ_125  				60
#define FREQ_203  				74	
#define FREQ_250  				80
#define FREQ_319  				87	
#define FREQ_637  				107	
#define FREQ_1k00 				120
#define FREQ_1k27 				127	
#define FREQ_2k00 				140
#define FREQ_5k10 				167	
#define FREQ_6k28 				173

#define Q_0p7 	 				13
#define Q_1p00   				19

#define SLOPE_6dBoct			0
#define SLOPE_12dBoct			10
//.....................................................................................................
// xover
enum 
{
	XOV_THRU = 0,

	XOV_LPBUT6,	
	XOV_LPBUT12,	
	XOV_LPBES12,
	XOV_LPLR12,	
	XOV_LPBUT18,	
	XOV_LPBES18,	
	XOV_LPBUT24,	
	XOV_LPBES24,	
	XOV_LPLR24,	
	XOV_LPBUT48,	
	XOV_LPBES48,	
	XOV_LPLR48,	

	XOV_HPBUT6,	
	XOV_HPBUT12,	
	XOV_HPBES12,	
	XOV_HPLR12,	
	XOV_HPBUT18,	
	XOV_HPBES18,	
	XOV_HPBUT24,	
	XOV_HPBES24,	
	XOV_HPLR24,	
	XOV_HPBUT48,	
	XOV_HPBES48,	
	XOV_HPLR48,	

	NUMXOVERTAB
}ENUMXOVERTYPE;
//.....................................................................................................
//hum cancel
#define HUMC_ATK_1ms		1
#define HUMC_REL_500ms		49
#define HUMC_THR_N60dB		0
#define HUMC_FREQ_50Hz		100
//.....................................................................................................
//noise gate
#define NGATE_ATK_1ms		 1
#define NGATE_REL_500ms		 49
#define NGATE_TRESHOLD_N60dB 10
#define NGATE_RANGE_N80dB	 0
//.....................................................................................................
//dynamic eq.
enum 
{
	DYNEQ_PEAK = 0,
	DYNEQ_LOSH6,
	DYNEQ_HISH6,

	NUMDYNEQTAB
}ENUMDYNEQTYPE;

#define DEQNORMALCHAIN		0
#define DEQSIDECHAIN		1

#define DEQBELOW			0
#define DEQABOVE			1

#define DEQ_ATK_8ms			7
#define DEQ_REL_80ms		7

#define DEQTHRSH_0DB 		120
//.....................................................................................................
// delay
#define DELAY_0M	 		0

enum 
{
	DELAYUNIT_MICROSEC = 0,
	DELAYUNIT_MILLISEC,
	DELAYUNIT_MILLIMETER,
	DELAYUNIT_METER,
	DELAYUNIT_INCHES,
	DELAYUNIT_FEET,

	NUMDLYUNITTAB
}ENUMDELATUNIT;

#define TEMPERATURE_20degC 	50

#define TEMPERATURE_UNIT_C 	0
#define TEMPERATURE_UNIT_F 	1

#define DELAYINPUT_MAX_DISTANCE  200000 /* 200m */
#define DELAYOUTPUT_MAX_DISTANCE 100000	/* 100m */
//.....................................................................................................
// comp/limiter
#define COMPRESSOR_MODE		0
#define LIMITER_MODE 		1

#define LIMRATIO 			15
#define COMPRATIO_MIN		0
#define LIMMAXTHRESH		220
#define LIMTHRESH_0dB		45

#define LIM_ATK_8ms			7
#define LIM_REL_80ms		7
//.....................................................................................................
// rms power prot.
#define PWRPROT_ACTIVE		 		0		//
#define PWRPROT_NOTACTIVE  			1		//

#define PWRPROT_REDUCTTHRESHOLD		150		//0dB detect reduction
#define PWRPROT_MAXCNTRHOT_1SEC		10 		//96000	= 1sec
#define PWRPROT_LEVELREDUCT_N3DB	135		//-3dB level reduction
#define PWRPROT_LEVELDELTA_500ms 	+5e-6	//n
//.....................................................................................................
/*
// vecchie impostazioni (versione di sviluppo)
// #define LIMTHRESH_MAX	 	161	//limiter max a +3.8dBu di ingresso NOTA: l'uscida del dsp e' normalizzata a -0.5dB
// #define LIMTTHMAX_POWER		160 // valore MAX del limiter per attivare protezione in potenza media

//impostazioni correnti (21/09/06)
#define LIMTHRESH_MAX	 	155	//limiter max a +2.6dBu di ingresso NOTA: l'uscida del dsp e' normalizzata a -0.5dB
#define LIMTTHMAX_POWER		154 // valore MAX del limiter per attivare protezione in potenza media
#define LIMRATIO 			14

#define LIMTHRSH_UNIT_DBU 	0
#define LIMTHRSH_UNIT_MV  	1

#define LIMTIME_SLOW		0
#define LIMTIME_NORMAL 		1
#define LIMTIME_FAST 		2

#define TEMPERATURE_UNIT_C 	0
#define TEMPERATURE_UNIT_F 	1

#define TEMPERATURE_20degC 	50

#define INPUTSELECT_ANALOG  0
#define INPUTSELECT_DIGITAL 1
*/
#define METERS_PREMUTE  	0
#define METERS_POSTMUTE 	1

#define WAKEUPMODE_NORMAL 	0
#define WAKEUPMODE_MUTE   	1

#define	LOCK_OFF			0	// pannello attivo
#define	LOCK_ON				1	// pannello non attivo

#define OFF					0
#define ON					1
//.....................................................................................................

#define PROX_ACTIVE			1			//processore attivo 
#define PROX_NOTACTIVE		0			//processore non attivo (bypassato)

#define POWER_OVLD_ACTIVE	 1
#define POWER_OVLD_NOTACTIVE 0


#define PRESET_VALID		1	//preset modificato	(campo m_nValid in struttura c++)
#define PRESET_NOTVALID		0	//preset non modificato	(campo m_nValid in struttura c++)
#define PRESET_MODIFIED		1	//preset modificato
#define PRESET_NOTMODIFIED	0	//preset non modificato
#define CONFIG_MODIFIED		1	//setup modificato
#define CONFIG_NOTMODIFIED	0	//setup non modificato

#define DEFAULT_PRESET		0	//preset default 

#define THERMAL_PROTECTION	1
#define THERMAL_NORMAL		0
//---------------------------------------------------------------------------------------
//      STRUTTURE DATI
//---------------------------------------------------------------------------------------
// STRUTTURA DI EDIT
typedef struct _edit_tag 
{
    int EditState;
	int CommState;
	int PageIndex;
	int NPage;
	int Index;
	int Field;
	int PresetNumberValue;	// 0÷(NPRESET_FACTORY+NPRESET_USER) (non sbancato)
	int PresetGroupValue; 	// indica se sto lavorando con un preset FACTORY o USER
	unsigned PresetPackedNameValue[PRESETNAMEPACKEDLENGTH];
	int LockModeValue;
	unsigned Prst_Change;
	unsigned old_state;
	int IdentifyActive;
} EDIT;
//-----------------------------------------------
//STRUTTURA DATI DI EDIT EREDITATA DAL CLIENT C++
//-----------------------------------------------
typedef struct _level_tag  
{
	unsigned m_nGain		:8;
	unsigned m_nLevel		:8;
	unsigned m_nMute		:2;
	unsigned m_nPolarity	:2;
}LEVEL;
//-----------------------------------------------
typedef struct _routing_tag  
{
	unsigned m_nRouting		:3;
}ROUTING;
//-----------------------------------------------
typedef struct _delay_tag  
{
	unsigned m_nDelay		:32;
	unsigned m_nDelayUnit	:3;
	unsigned m_nTemp		:9;
	unsigned m_nTempUnit	:1;
	unsigned m_nUmidity		:6;
	unsigned m_nMaxDelay    :32;  
}DELAY;
//-----------------------------------------------
typedef struct _vum_tag  
{
	unsigned m_nVum			:5;
	unsigned m_nVumMode		:2;
	unsigned m_nVumTime		:2;
} VUM;
//-----------------------------------------------
typedef struct _noise_tag  
{
	unsigned m_nAttack		:7;	// 85 steps 
    unsigned m_nRelease		:7;	// 100 steps 
    unsigned m_nThreshold	:7;	// 92 steps
	unsigned m_nRange		:7;	// 111 steps
	unsigned m_nBypass		:2;	// 2 steps
	VUM m_Vreduct;
}NOISE;
//-----------------------------------------------
typedef struct _humcanc_tag  
{
	unsigned m_nAttack		:7;	// 85 steps 
    unsigned m_nRelease		:7;	// 100 steps 
    unsigned m_nThreshold	:6;	// 61 steps
	unsigned m_nFreq		:9;	// 301 steps
	unsigned m_nBypass		:2;	// 2 steps
	VUM m_Vreduct;
}HUMCANC;
//-----------------------------------------------
typedef struct _comp_tag  
{
	unsigned m_nAttack			:7;	// 100 steps
	unsigned m_nRelease			:7;	// 100 steps
    unsigned m_nRatio			:5;	// 1.0,1.1,1.3,1.5,1.7,2.0,2.5,3.0,3.5,4.0,5.0,6.0,8.0,10.0,20.0,limiter (0..15;16 step)
    unsigned m_nMaxThreshold	:8;
    unsigned m_nThreshold		:8;	// 220 steps (0..220)
	unsigned m_nLimiter			:1; 
	unsigned m_nThreshUnit		:2; // 2 steps
	VUM m_Vreduct;
}COMP;
//-----------------------------------------------
typedef struct _xover_tag  
{
	unsigned m_nType		:5;	 
    unsigned m_nFreq		:8;	// 201 steps 
    unsigned m_nBypass		:2;	// 2 steps 
}XOVER;
//-----------------------------------------------
typedef struct _pareq_tag  
{
	unsigned m_nType		:5;	//  
	unsigned m_nFreq		:8;	// 201 steps 
	unsigned m_nBand		:8; // 152 steps 
	unsigned m_nGain		:8; 
	unsigned m_nFlat		:2; // 2 steps
	unsigned m_nSlope		:6; 
}PAREQ;
//-----------------------------------------------
typedef struct _staticeq_tag  
{
	unsigned m_nCfgType		:4;
	unsigned m_nBypass		:2;
	PAREQ m_ParEqArray[MAXPAREQCELLNUM];
}STATICEQ;
//-----------------------------------------------
typedef struct _dyneq_tag  
{
	unsigned m_nFreq		 :8;		// n steps
	unsigned m_nGain		 :8;	  	// n steps
	unsigned m_nBand		 :6;		// n steps
	unsigned m_nType		 :3;		// n steps
	unsigned m_nThreshold	 :8;		// n steps
	unsigned m_nAttack		 :7;		// n steps
	unsigned m_nRelease		 :7;		// n steps
	unsigned m_nInv			 :2;		// 2 steps
	unsigned m_nSide		 :2;		// 2 steps
	unsigned m_nBypass		 :2;		// 2 steps	//1 cella bypass, 0 cella attiva
	unsigned m_bDefaultState :2;	//indica se qulche parametro e' stato modificaro rispetto al default

	VUM m_Vlevel;
	VUM m_Vgain;

}DYNEQ;
//-----------------------------------------------
typedef struct _dynamiceq_tag  
{
	unsigned m_nCfgType		:4;	//configurazione corrente
	unsigned m_nBypass		:2;

	DYNEQ m_DynEqArray[MAXDYNEQCELLNUM];
}DYNAMICEQ;
//-----------------------------------------------
typedef struct _pwrprotect_tag  
{
	unsigned m_nReductThresh[2];	
	unsigned m_nReduction[2];	
	unsigned m_nMaxCntr[2];
	unsigned m_nBypass[2];
}PWRPROTECT;
//-----------------------------------------------
typedef struct _inputch_tag  
{
	ROUTING		m_Routing;
	LEVEL		m_Level;
	VUM			m_Vum;
	COMP		m_Comp;
	NOISE		m_Noise;
	HUMCANC		m_HumCanc;
	DYNAMICEQ	m_DynamicEq;
	STATICEQ	m_StaticEq;
	DELAY		m_Delay;
}INPUTCH;
//-----------------------------------------------
typedef struct _outputch_tag  
{
	ROUTING		m_Routing;
	DELAY		m_Delay;
	XOVER		m_Xover[2];
	STATICEQ	m_StaticEq;
	LEVEL		m_Level;
	VUM			m_Vum;
	COMP		m_Comp;
}OUTPUTCH;
//-----------------------------------------------
typedef struct _config_tag  
{
	unsigned m_DevName[DEVICENAMEPACKEDLENGTH];
	unsigned m_Password[PASSWORDNAMEPACKEDLENGTH];
	unsigned m_DevType				:8;
	unsigned m_FwRelease			:8;
	unsigned m_Group				:6;	
	unsigned m_Zone					:6;
	unsigned m_LockEnable			:1;
	unsigned m_LockLevel			:4;  
	unsigned m_InGanging			:2; // 2 steps 
	unsigned m_OutGanging			:3; // 2 steps 
	unsigned m_GraphOption			:3; // bit 15=bAddInputEq,bit 14=bAddOutputGain,bit 13=m_bAddOutputEq
	unsigned m_WakeUpMode			:2; // 2 steps
	unsigned m_Vsupply				:4;	// tensione sul primario
	unsigned m_TempAmpl				:4; // temp sul radiatore
	unsigned m_OvlTempAmpl			:1; // finale spento
	unsigned m_ThermalProt			:2; // 2, step protezione potenza media
	unsigned m_LoadSensing 			:4; // lettura carico di linea ?
	unsigned m_nInDelayUnit			:4;
	unsigned m_nOutDelayUnit		:4;
	unsigned m_nTemp				:9;
	unsigned m_nTempUnit			:2;
	unsigned m_nUmidity				:6;
	unsigned m_nEqbandUnit			:2;
	// dati aggiunti rispetto al config c++
    unsigned m_nCurrentPresetNumber :10; // 0÷(NPRESET_FACTORY+NPRESET_USER) (non sbancato) 
	unsigned m_nCurrentPresetGroup  :2;  // indica se si lavora con un preset FACTORY o USER
	unsigned m_nickname				:8;
}CONFIG;
//-----------------------------------------------
typedef struct _preset_tag  
{
	unsigned m_PrstName[PRESETNAMEPACKEDLENGTH];
	//preset user valid (0=preset uguale al default, 1=valid cioe' modificato, diverso dal default user)
	unsigned m_Valid               :4;
	unsigned m_LockLevel			:4;
	INPUTCH m_InputCh[NINPUTCHANNEL];
	OUTPUTCH m_OutputCh[NOUTPUTCHANNEL];
	PWRPROTECT m_PwrProtect;
}PRESET;
//-----------------------------------------------
typedef struct _singleprst_tag  
{
	unsigned m_PrstName[PRESETNAMEPACKEDLENGTH];
	//preset user valid (0=preset uguale al default, 1=valid cioe' modificato, diverso dal default user)
	unsigned m_Valid               :4;
	unsigned m_LockLevel			:4;
}SINGLEPRST;
//-----------------------------------------------
typedef struct _prst_tag  
{
	SINGLEPRST m_PrstArray[NPRESET_USER];
}PRST;
//-----------------------------------------------
typedef struct _factoryprst_tag  
{
	SINGLEPRST m_PrstArray[NMAXPRESET_FACTORY];
}FACTORYPRST;
//-----------------------------------------------
typedef struct _setup_tag  
{
	CONFIG m_Config;
	PRESET m_Preset;						 // preset current
	PRST   m_PrstList;						 // lista in RAM dei nomi dei preset user
	FACTORYPRST   m_FactoryPrstList;		 // lista in RAM dei nomi dei preset factory
	unsigned m_nSave              		:1;  // 0=default 1=salvato
    unsigned m_nCurrentPresetModified 	:1;  // Current preset modificato
	unsigned m_nConfigModified          :1;  // Setup modificato
}SETUP;	
//---------------------------------------------------------------------------------------

#endif

