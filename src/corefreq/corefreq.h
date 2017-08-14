/*
 * CoreFreq
 * Copyright (C) 2015-2016 CYRIL INGENIERIE
 * Licenses: GPL2
 */

#define	TASK_COMM_LEN		16

#define	SHM_FILENAME	"corefreq-shm"

typedef struct
{
	OFFLINE				OffLine;

	CLOCK				Clock;

	unsigned int			Toggle;

	struct {
		unsigned int		ApicID,
					CoreID,
					ThreadID;
		struct {
			Bit32		BSP,
					x2APIC;
		} MP;
		struct {
		unsigned int		Set,
					Size;
		unsigned short int	LineSz,
					Part,
					Way;
		    struct {
		    unsigned short int	WriteBack: 1-0,
					Inclusive: 2-1,
					_pad16	: 16-2;
		    } Feature;
		} Cache[CACHE_MAX_LEVEL];
	} Topology;

	struct {
	    	unsigned int		TM1,
					TM2;
		float			ODCM;
		unsigned int		PowerPolicy;
	} PowerThermal;

	struct FLIP_FLOP {
		struct {
	    	unsigned int		Target,
					Sensor,
					Temp,
					Trip;
		} Thermal;

		struct {
			double		IPS,
					IPC,
					CPI,
					Turbo,
					C0,
					C3,
					C6,
					C7,
					C1;
		} State;

		struct {
			double		Ratio,
					Freq;
		} Relative;
	} FlipFlop[2];
} CPU_STRUCT;

typedef struct
{
	volatile unsigned long long	Sync,
					Room;

	FEATURES			Features;

	unsigned int			SleepInterval;

	struct {
		unsigned int		Count,
					OnLine;
	} CPU;

	unsigned char			Architecture[32];
	unsigned int			Boost[1+1+8],
					PM_version;

	char				Brand[64];

	Bit32				InvariantTSC,
					HyperThreading,
					PowerNow;

	Bit64				SpeedStep,
					SpeedStep_Mask,
					TurboBoost,
					TurboBoost_Mask,
					C1E,
					C1E_Mask,
					C3A,		// Nehalem
					C3A_Mask,
					C1A,		// Nehalem
					C1A_Mask,
					C3U,		// Sandy Bridge
					C3U_Mask,
					C1U,		// Sandy Bridge;
					C1U_Mask;

	struct {
		double			Turbo,
					C0,
					C3,
					C6,
					C7,
					C1;
	} Avg;
} PROC_STRUCT;


typedef	struct
{
	char		AppName[TASK_COMM_LEN];
	IDLEDRIVER	IdleDriver;
	PROC_STRUCT	Proc;
	CPU_STRUCT	Cpu[];
} SHM_STRUCT;
