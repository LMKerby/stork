/*****************************************************************************/
/*                                                                           */
/* serpent 2 (beta-version) : matlaboutput.c                                 */
/*                                                                           */
/* Created:       2011/03/13 (JLe)                                           */
/* Last modified: 2016/02/01 (VVa)                                           */
/* Version:       2.1.25                                                     */
/*                                                                           */
/* Description: Prints standard output in Matlab format file                 */
/*                                                                           */
/* Comments:                                                                 */
/*                                                                           */
/*****************************************************************************/

#include "header.h"
#include "locations.h"

#define FUNCTION_NAME "MatlabOutput:"

/*****************************************************************************/

void MatlabOutput()
{
  long gcu, ptr, adf, ppw, alb, ng, nmg, nd, n, uni, nb, np, ns, nc;
  char outfile[MAX_STR], tmpstr[MAX_STR];
  double S, P, div;
  FILE *fp;

  /* Check mpi task */

  if (mpiid > 0)
    return;

  /* Check corrector step */

  if ((long)RDB[DATA_BURN_STEP_PC] == CORRECTOR_STEP) 
    return;

  /* Check if in active cycles */

  if (((long)RDB[DATA_BURN_DECAY_CALC] == NO) &&
      (RDB[DATA_CYCLE_IDX] < RDB[DATA_CRIT_SKIP]))
    return;

  /* Reduce private results array */

  ReducePrivateRes();

  /* File name */

  sprintf(outfile, "%s_res.m", GetText(DATA_PTR_INPUT_FNAME));

  /* Open file for writing (append files if burnup calculation) */

  if ((long)RDB[DATA_COEF_CALC_IDX] > 0)
    {
      if ((long)RDB[DATA_SIMULATION_COMPLETED] == NO)
	return;
      else if ((long)RDB[DATA_COEF_CALC_RUN_IDX] == 1)
	fp = fopen(outfile, "w");
      else
	fp = fopen(outfile, "a");
    }
  else if ((long)RDB[DATA_PTR_RIA0] > VALID_PTR)
    {
      if ((long)RDB[DATA_SIMULATION_COMPLETED] == NO)
	return;
      else if ((long)RDB[DATA_SIMULATION_MODE] == SIMULATION_MODE_CRIT)
	fp = fopen(outfile, "w");
      else
	fp = fopen(outfile, "a");
    }
  else if (((long)RDB[DATA_BURNUP_CALCULATION_MODE] == NO) ||
      (((long)RDB[DATA_BURN_STEP] == 0) &&
       ((long)RDB[DATA_BURN_STEP_PC] == PREDICTOR_STEP)))
    fp = fopen(outfile, "w");
  else if ((long)RDB[DATA_SIMULATION_COMPLETED] == YES)
    fp = fopen(outfile, "a");
  else
    return;

  /* Check pointer */
  
  if (fp == NULL)
    Die(FUNCTION_NAME, "Unable to open file for writing");

  /* Pointer to gc universe list (this can be null) */

  if ((long)RDB[DATA_NEUTRON_TRANSPORT_MODE] == YES)	
    gcu = (long)RDB[DATA_PTR_GCU0];
  else
    gcu = -1;
  
  /* Loop over gc universes */

  do
    {
      /* increase counter */
      
      fprintf(fp, "\n%% Increase counter:\n\n");
      
      fprintf(fp, "if (exist(\'idx\', \'var\'));\n");
      fprintf(fp, "  idx = idx + 1;\n");
      fprintf(fp, "else;\n");
      fprintf(fp, "  idx = 1;\n"); 
      fprintf(fp, "end;\n");
      
      /***********************************************************************/

      /***** Title, version and date *****************************************/
      
      sprintf(tmpstr, "%s %s", CODE_NAME, CODE_VERSION);
      
      fprintf(fp, "\n");
      
      fprintf(fp, "%% Version, title and date:\n\n");
      
      fprintf(fp, "VERSION                   (idx, [1:%3ld])  = '%s' ;\n", 
	      (long)strlen(tmpstr), tmpstr);

      fprintf(fp, "COMPILE_DATE              (idx, [1:%3ld])  = '%s' ;\n", 
	      (long)strlen(GetText(DATA_PTR_COMPILE_DATE)),
	      GetText(DATA_PTR_COMPILE_DATE));

#ifdef DEBUG
      
      fprintf(fp, "DEBUG                     (idx, 1)        = 1 ;\n");
      
#else
      
      fprintf(fp, "DEBUG                     (idx, 1)        = 0 ;\n");
      
#endif
      
      if ((long)RDB[DATA_PTR_TITLE] > VALID_PTR)
	fprintf(fp, "TITLE                     (idx, [1:%3ld])  = '%s' ;\n", 
		(long)strlen(GetText(DATA_PTR_TITLE)),
		GetText(DATA_PTR_TITLE));
      else
	fprintf(fp, "TITLE                     (idx, [1:%3ld])  = '%s' ;\n", 
		(long)strlen("Untitled"), "Untitled");

      fprintf(fp, "INPUT_FILE_NAME           (idx, [1:%3ld])  = '%s' ;\n", 
	      (long)strlen(GetText(DATA_PTR_INPUT_FNAME)),
	      GetText(DATA_PTR_INPUT_FNAME));

      fprintf(fp, "WORKING_DIRECTORY         (idx, [1:%3ld])  = '%s' ;\n", 
	      (long)strlen(GetText(DATA_PTR_WORKDIR)),
	      GetText(DATA_PTR_WORKDIR));

      fprintf(fp, "HOSTNAME                  (idx, [1:%3ld])  = '%s' ;\n", 
	      (long)strlen(GetText(DATA_PTR_HOSTNAME)),
	      GetText(DATA_PTR_HOSTNAME));

      if ((long)RDB[DATA_PTR_CPU_NAME] > 0.0)
	fprintf(fp, "CPU_TYPE                  (idx, [1:%3ld])  = '%s' ;\n", 
		(long)strlen(GetText(DATA_PTR_CPU_NAME)),
		GetText(DATA_PTR_CPU_NAME));

      if (RDB[DATA_CPU_MHZ] > 0.0)
	fprintf(fp, "CPU_MHZ                   (idx, 1)        = %1.1f ;\n", 
		RDB[DATA_CPU_MHZ]);

      fprintf(fp, "START_DATE                (idx, [1:%3ld])  = '%s' ;\n", 
	      (long)strlen(GetText(DATA_PTR_DATE)),
	      GetText(DATA_PTR_DATE));
      fprintf(fp, "COMPLETE_DATE             (idx, [1:%3ld])  = '%s' ;\n", 
	      (long)strlen(TimeStamp()), TimeStamp());
      
      /***********************************************************************/

      /***** Run parameters **************************************************/
    
      fprintf(fp, "\n");
      
      fprintf(fp, "%% Run parameters:\n\n");
      
      if ((long)RDB[DATA_SIMULATION_MODE] == SIMULATION_MODE_CRIT)
	{
	  fprintf(fp, "POP                       (idx, 1)        = %ld ;\n", 
		  (long)RDB[DATA_CRIT_POP]);
	  fprintf(fp, "CYCLES                    (idx, 1)        = %ld ;\n", 
		  (long)RDB[DATA_CRIT_CYCLES]);
	  fprintf(fp, "SKIP                      (idx, 1)        = %ld ;\n", 
		  (long)RDB[DATA_CRIT_SKIP]);
	  fprintf(fp, "BATCH_INTERVAL            (idx, 1)        = %ld ;\n", 
		  (long)RDB[DATA_BATCH_INTERVAL]);
	}
      else
	{
	  fprintf(fp, "POP                       (idx, 1)        = %ld ;\n", 
		  (long)RDB[DATA_SRC_POP]);
	  fprintf(fp, "BATCHES                   (idx, 1)        = %ld ;\n", 
		  (long)RDB[DATA_SRC_BATCHES]);
	}

      fprintf(fp, "SRC_NORM_MODE             (idx, 1)        = 2 ;\n");
      
      fprintf(fp, "SEED                      (idx, 1)        = %lu ;\n", 
	      parent_seed);
      
      if ((long)RDB[DATA_NEUTRON_TRANSPORT_MODE] == YES)
	{
	  /* Unifrom fission source method */
	  
	  fprintf(fp, "UFS_MODE                  (idx, 1)        = %ld ;\n",
		  (long)RDB[DATA_UFS_MODE]);
	  
	  fprintf(fp, "UFS_ORDER                 (idx, 1)        = %1.5f;\n",
		  RDB[DATA_UFS_ORDER]);
	}

      /* Simulation modes */

      fprintf(fp, "NEUTRON_TRANSPORT_MODE    (idx, 1)        = %ld ;\n", 
	      (long)RDB[DATA_NEUTRON_TRANSPORT_MODE]);

      fprintf(fp, "PHOTON_TRANSPORT_MODE     (idx, 1)        = %ld ;\n", 
	      (long)RDB[DATA_PHOTON_TRANSPORT_MODE]);

      if ((long)RDB[DATA_NEUTRON_TRANSPORT_MODE] == YES)
	{
	  /* Group constant calculation */

	  fprintf(fp, "GROUP_CONSTANT_GENERATION (idx, 1)        = %ld ;\n", 
		  (long)RDB[DATA_OPTI_GC_CALC]);
	  
	  /* B1 calculation */
	  
	  fprintf(fp, "B1_CALCULATION            (idx, [1:  3])  = [ %ld %ld %ld ];\n", (long)RDB[DATA_B1_CALC], (long)RDB[DATA_B1_REPEATED], (long)RDB[DATA_B1_CONVERGED]);

	  fprintf(fp, "B1_BURNUP_CORRECTION      (idx, 1)        = %ld ;\n", 
		  (long)RDB[DATA_B1_BURNUP_CORR]);
	}

      /* Implicit reaction rates */

      fprintf(fp, "IMPLICIT_REACTION_RATES   (idx, 1)        = %ld ;\n", 
	      (long)RDB[DATA_OPTI_IMPLICIT_RR]);

      /***********************************************************************/

      /***** Optimization ****************************************************/
      
      fprintf(fp, "\n");
      
      fprintf(fp, "%% Optimization:\n\n");

      fprintf(fp, "OPTIMIZATION_MODE         (idx, 1)        = %ld ;\n", 
	      (long)RDB[DATA_OPTI_MODE]);

      fprintf(fp, "RECONSTRUCT_MICROXS       (idx, 1)        = %ld ;\n", 
	      (long)RDB[DATA_OPTI_RECONSTRUCT_MICROXS]);

      fprintf(fp, "RECONSTRUCT_MACROXS       (idx, 1)        = %ld ;\n", 
	      (long)RDB[DATA_OPTI_RECONSTRUCT_MACROXS]);

      fprintf(fp, "MG_MAJORANT_MODE          (idx, 1)        = %ld ;\n", 
	      (long)RDB[DATA_OPTI_MG_MODE]);

      if ((long)RDB[DATA_BURNUP_CALCULATION_MODE] == YES)
	fprintf(fp, "SPECTRUM_COLLAPSE         (idx, 1)        = %ld ;\n", 
		(long)RDB[DATA_BU_SPECTRUM_COLLAPSE]);

      /***********************************************************************/

      /***** Parallelization *************************************************/
      
      fprintf(fp, "\n");
      
      fprintf(fp, "%% Parallelization:\n\n");

      fprintf(fp, "MPI_TASKS                 (idx, 1)        = %d ;\n", 
	      mpitasks);

      fprintf(fp, "OMP_THREADS               (idx, 1)        = %ld ;\n", 
	      (long)RDB[DATA_OMP_MAX_THREADS]);

      fprintf(fp, "MPI_REPRODUCIBILITY       (idx, 1)        = %ld ;\n", 
	      (long)RDB[DATA_OPTI_MPI_REPRODUCIBILITY]);

      fprintf(fp, "OMP_REPRODUCIBILITY       (idx, 1)        = %ld ;\n", 
	      (long)RDB[DATA_OPTI_OMP_REPRODUCIBILITY]);

      if ((long)RDB[DATA_OMP_MAX_THREADS] > 1)
	{
	  fprintf(fp, "OMP_HISTORY_PROFILE       (idx, [1: %3ld]) = [ ", 
		  (long)RDB[DATA_OMP_MAX_THREADS]);
	  
	  ptr = (long)RDB[DATA_PTR_OMP_HISTORY_COUNT];
      
	  for (n = 0; n < (long)RDB[DATA_OMP_MAX_THREADS]; n++)
	    {
	      /* Count is zero in decay steps */

	      if (SumPrivateData(ptr) > 0.0)
		fprintf(fp, "%12.5E ", RDB[DATA_OMP_MAX_THREADS]*
			GetPrivateData(ptr, n)/SumPrivateData(ptr));
	      else
		fprintf(fp, "%12.5E ", 0.0);
	    }

	  fprintf(fp, " ];\n");
	}

      /* Shared scoring buffer */

      fprintf(fp, "SHARE_BUF_ARRAY           (idx, 1)        = %ld ;\n", 
	      (long)RDB[DATA_OPTI_SHARED_BUF]);

      fprintf(fp, "SHARE_RES2_ARRAY          (idx, 1)        = %ld ;\n", 
	      (long)RDB[DATA_OPTI_SHARED_RES2]);
      
      /***********************************************************************/

      /***** File paths ******************************************************/
      
      fprintf(fp, "\n");
      
      fprintf(fp, "%% File paths:\n\n");
      
      /* ACE directory file (only first file printed) */

      if ((ptr = (long)RDB[DATA_PTR_ACEDATA_FNAME_LIST]) > VALID_PTR)
	sprintf(tmpstr, "%s", GetText(ptr));
      else
	sprintf(tmpstr, "N/A");      

      fprintf(fp, "XS_DATA_FILE_PATH         (idx, [1:%3ld])  = '%s' ;\n", 
	      (long)strlen(tmpstr), tmpstr);

      /* Decay data file path (only first file printed) */

      if ((ptr = (long)RDB[DATA_PTR_DECDATA_FNAME_LIST]) > VALID_PTR)
	sprintf(tmpstr, "%s", GetText(ptr));
      else
	sprintf(tmpstr, "N/A");
      
      fprintf(fp, "DECAY_DATA_FILE_PATH      (idx, [1:%3ld])  = '%s' ;\n", 
	      (long)strlen(tmpstr), tmpstr);

      if ((long)RDB[DATA_NEUTRON_TRANSPORT_MODE] == YES)
	{
	  /* Spontaneous fission yield (only first file printed) */

	  if ((ptr = (long)RDB[DATA_PTR_NFYDATA_FNAME_LIST]) > VALID_PTR)
	    sprintf(tmpstr, "%s", GetText(ptr));
	  else
	    sprintf(tmpstr, "N/A");
	  
	  fprintf(fp, "SFY_DATA_FILE_PATH        (idx, [1:%3ld])  = '%s' ;\n", 
		  (long)strlen(tmpstr), tmpstr);
	  
	  /* Neutron-induced fission yield (only first file printed) */
	  
	  if ((ptr = (long)RDB[DATA_PTR_NFYDATA_FNAME_LIST]) > VALID_PTR)
	    sprintf(tmpstr, "%s", GetText(ptr));
	  else
	    sprintf(tmpstr, "N/A");
	  
	  fprintf(fp, "NFY_DATA_FILE_PATH        (idx, [1:%3ld])  = '%s' ;\n", 
		  (long)strlen(tmpstr), tmpstr);
	  
	  /* Isomeric branching ratio file path (only first file printed) */
	  
	  if ((ptr = (long)RDB[DATA_PTR_BRADATA_FNAME_LIST]) > VALID_PTR)
	    sprintf(tmpstr, "%s", GetText(ptr));
	  else
	    sprintf(tmpstr, "N/A");

	  fprintf(fp, "BRA_DATA_FILE_PATH        (idx, [1:%3ld])  = '%s' ;\n", 
		  (long)strlen(tmpstr), tmpstr);
	}

      /* Photon data path */

      if ((long)RDB[DATA_PHOTON_TRANSPORT_MODE] == YES)
	{
	  if ((long)RDB[DATA_PHOTON_DATA_DIR] > VALID_PTR)
	    sprintf(tmpstr, "%s", GetText(DATA_PHOTON_DATA_DIR));
	  else
	    sprintf(tmpstr, "N/A");
	  
	  fprintf(fp, "PHOTON_PHYS_DIRECTORY     (idx, [1:%3ld])  = '%s' ;\n", 
		  (long)strlen(tmpstr), tmpstr);
	}

      /***********************************************************************/

      /***** Collision and reaction sampling *********************************/
      
      fprintf(fp, "\n");
      
      fprintf(fp, "%% Collision and reaction sampling (neutrons/photons):\n\n");

      if ((long)RDB[DATA_SIMULATION_MODE] != SIMULATION_MODE_CRIT)
	{
	  PrintValues(fp, "MEAN_SRC_WGT", RES_SRC_MEAN_WGT, 2, -1, -1, 0, 0);

	  PrintValues(fp, "SOURCE_SAMPLING_EFF", RES_SRC_SAMPLING_EFF, 
		      2, -1, -1, 0, 0);
	}

      PrintValues(fp, "MIN_MACROXS", RES_MIN_MACROXS, 2, -1, -1, 0, 0);
      
      fprintf(fp, "DT_THRESH                 (idx, [1:  2])  = [ %12.5E %12.5E ];\n", 
	      1.0 - RDB[DATA_DT_NTHRESH], 1.0 - RDB[DATA_DT_PTHRESH]);

      PrintValues(fp, "ST_FRAC", RES_ST_TRACK_FRAC, 2, -1, -1, 0, 0);
      PrintValues(fp, "DT_FRAC", RES_DT_TRACK_FRAC, 2, -1, -1, 0, 0);
      PrintValues(fp, "DT_EFF", RES_DT_TRACK_EFF, 2, -1, -1, 0, 0);

      if ((long)RDB[DATA_PTR_IFC0] > VALID_PTR)
	PrintValues(fp, "IFC_COL_EFF", RES_IFC_COL_EFF, 2, -1, -1, 0, 0);

      PrintValues(fp, "REA_SAMPLING_EFF", RES_REA_SAMPLING_EFF, 2, -1, -1,0,0);
      PrintValues(fp, "REA_SAMPLING_FAIL", RES_REA_SAMPLING_FAIL, 2,-1,-1,0,0);

      if ((long)RDB[DATA_TMS_MODE] != TMS_MODE_NONE)
	PrintValues(fp, "TMS_SAMPLING_EFF", RES_TMS_SAMPLING_EFF,1,-1,-1,0,0);
      
      PrintValues(fp, "TOT_COL_EFF", RES_TOT_COL_EFF, 2, -1, -1, 0, 0);

      PrintValues(fp, "AVG_TRACKING_LOOPS", RES_AVG_TRACK_LOOPS, 
		  4, -1, -1, 0, 0);

      if ((long)RDB[DATA_TMS_MODE] != TMS_MODE_NONE)
	PrintValues(fp, "TMS_FAIL_STAT", RES_TMS_FAIL_STAT,3,-1,-1,1,0);

      if ((long)RDB[DATA_USE_DBRC] == YES)
	{
	  /* DBRC majorant exceed fraction */

	  ptr = (long)RDB[DATA_PTR_DBRC_COUNT];
	  CheckPointer(FUNCTION_NAME, "(ptr)", PRIVA_ARRAY, ptr);
	  div = SumPrivateData(ptr);
	  
	  ptr = RDB[DATA_PTR_DBRC_EXCEED_COUNT];
	  CheckPointer(FUNCTION_NAME, "(ptr)", PRIVA_ARRAY, ptr);
	  
	  if(div > 0.0)
	    fprintf(fp, 
		    "DBRC_EXCEED_FRAC          (idx, 1)        = %12.5E ;\n",
		    SumPrivateData(ptr)/div);
	}

      /* Average number of tracks, collisions and surface crossings */

      PrintValues(fp, "AVG_TRACKS", RES_AVG_TRACKS, 2, -1, -1, 0, 0);
      PrintValues(fp, "AVG_REAL_COL", RES_AVG_REAL_COL, 2, -1, -1, 0, 0);
      PrintValues(fp, "AVG_VIRT_COL", RES_AVG_VIRT_COL, 2, -1, -1, 0, 0);

      /* NOTE: tää antaa fysikaalisesti oikeita tuloksia 100% ST-moodissa */

      PrintValues(fp, "AVG_SURF_CROSS", RES_AVG_SURF_CROSS, 2, -1, -1, 0, 0);

      /* STL ray test fail rates */

      if ((long)RDB[DATA_PTR_STL0] > VALID_PTR)
	{
	  fprintf(fp, "\n");
      
	  fprintf(fp, "%% STL geometries:\n\n");

	  PrintValues(fp, NULL, RES_STL_RAY_TEST, 5, -1, -1, 0, 0);
	}

      /***********************************************************************/

      /***** Run statistics **************************************************/
      
      fprintf(fp, "\n");
      
      fprintf(fp, "%% Run statistics:\n\n");
      
      fprintf(fp, "CYCLE_IDX                 (idx, 1)        = %ld ;\n", 
	      (long)(RDB[DATA_CYCLE_IDX] - RDB[DATA_CRIT_SKIP]) + 1);
      
      fprintf(fp, "SOURCE_POPULATION         (idx, 1)        = %ld ;\n", 
	      (long)RDB[DATA_NHIST_CYCLE]);

      ptr = (long)RDB[RES_MEAN_POP_SIZE];
      fprintf(fp, "MEAN_POP_SIZE             (idx, [1:  2])  = [ %12.5E %7.5f ];\n", Mean(ptr, 0), RelErr(ptr, 0));

      ptr = (long)RDB[RES_MEAN_POP_WGT];
      fprintf(fp, "MEAN_POP_WGT              (idx, [1:  2])  = [ %12.5E %7.5f ];\n", Mean(ptr, 0), RelErr(ptr, 0));

      fprintf(fp, "SIMULATION_COMPLETED      (idx, 1)        = %ld ;\n",
	      (long)RDB[DATA_SIMULATION_COMPLETED]);

      /***********************************************************************/

      /***** Timers **********************************************************/
      
      fprintf(fp, "\n");
      
      fprintf(fp, "%% Running times:\n\n");
      
      fprintf(fp, "TOT_CPU_TIME              (idx, 1)        = %12.5E ;\n", 
	      TimerCPUVal(TIMER_RUNTIME)/60.);

      fprintf(fp, "RUNNING_TIME              (idx, 1)        = %12.5E ;\n", 
	      TimerVal(TIMER_RUNTIME)/60.0);

      fprintf(fp, "INIT_TIME                 (idx, [1:  2])  = [ %12.5E %12.5E ];\n", 
	      TimerVal(TIMER_INIT_TOTAL)/60.0, TimerVal(TIMER_INIT)/60.0);

      fprintf(fp, "PROCESS_TIME              (idx, [1:  2])  = [ %12.5E %12.5E ];\n", 
	      TimerVal(TIMER_PROCESS_TOTAL)/60.0, TimerVal(TIMER_PROCESS)/60.0);

      fprintf(fp, "TRANSPORT_CYCLE_TIME      (idx, [1:  3])  = [ %12.5E %12.5E %12.5E ];\n", 
	      TimerVal(TIMER_TRANSPORT_TOTAL)/60.0, RDB[DATA_PRED_TRANSPORT_TIME]/60.0, RDB[DATA_CORR_TRANSPORT_TIME]/60.0);

      if ((long)RDB[DATA_PTR_FIN0] > VALID_PTR)
	fprintf(fp, "FINIX_SOLUTION_TIME       (idx, 1)        = %12.5E ;\n", 
		TimerVal(TIMER_FINIX)/60.0);
    
      if ((long)RDB[DATA_BURNUP_CALCULATION_MODE] != NO)
	{
	  fprintf(fp, "BURNUP_CYCLE_TIME         (idx, [1:  2])  = [ %12.5E %12.5E ];\n", 
		  TimerVal(TIMER_BURNUP_TOTAL)/60.0, TimerVal(TIMER_BURNUP)/60.0);
	  
	  fprintf(fp, "BATEMAN_SOLUTION_TIME     (idx, [1:  2])  = [ %12.5E %12.5E ];\n", 
		  TimerVal(TIMER_BATEMAN_TOTAL)/60.0, TimerVal(TIMER_BATEMAN)/60.0);
	}

      fprintf(fp, "MPI_OVERHEAD_TIME         (idx, [1:  2])  = [ %12.5E %12.5E ];\n", 
	      TimerVal(TIMER_MPI_OVERHEAD_TOTAL)/60.0, TimerVal(TIMER_MPI_OVERHEAD)/60.0);

      fprintf(fp, "ESTIMATED_RUNNING_TIME    (idx, [1:  2])  = [ %12.5E %12.5E ];\n", RDB[DATA_ESTIM_CYCLE_TIME]/60.0, RDB[DATA_ESTIM_TOT_TIME]/60.0);

      if (TimerVal(TIMER_RUNTIME) > 0.0)
	fprintf(fp, "CPU_USAGE                 (idx, 1)        = %1.5f ;\n", 
		TimerCPUVal(TIMER_RUNTIME)/TimerVal(TIMER_RUNTIME));     
      else
	fprintf(fp, "CPU_USAGE                 (idx, 1)        = %1.5f ;\n", 
		0.0);

      PrintValues(fp, "TRANSPORT_CPU_USAGE", RES_CPU_USAGE, 1, -1, -1, 0, 0);

      S = (long)RDB[DATA_OMP_MAX_THREADS];
      P = TimerVal(TIMER_OMP_PARA)/TimerVal(TIMER_RUNTIME);

      fprintf(fp, "OMP_PARALLEL_FRAC         (idx, 1)        = %12.5E ;\n", P);
      /*
	fprintf(fp, "OMP_AMDAHL_MAX            (idx, 1)        = %1.2f ;\n",
	      1.0/(1.0 - P + P/S));
      */

      /***********************************************************************/

      /***** Memory usage ****************************************************/
      
      fprintf(fp, "\n");
      
      fprintf(fp, "%% Memory usage:\n\n");

      /* Print available memory */

      if (RDB[DATA_CPU_MEM] > 0.0)
	fprintf(fp, "AVAIL_MEM                 (idx, 1)        = %1.2f ;\n", 
		RDB[DATA_CPU_MEM]*GIGA/MEGA);

      fprintf(fp, "ALLOC_MEMSIZE             (idx, 1)        = %1.2f;\n", 
	      RDB[DATA_REAL_BYTES]/MEGA);

      fprintf(fp, "MEMSIZE                   (idx, 1)        = %1.2f;\n", 
	      RDB[DATA_TOTAL_BYTES]/MEGA);

      fprintf(fp, "XS_MEMSIZE                (idx, 1)        = %1.2f;\n", 
	      RDB[DATA_TOT_XS_BYTES]/MEGA);

      fprintf(fp, "MAT_MEMSIZE               (idx, 1)        = %1.2f;\n", 
	      RDB[DATA_TOT_MAT_BYTES]/MEGA);

      fprintf(fp, "RES_MEMSIZE               (idx, 1)        = %1.2f;\n", 
	      RDB[DATA_TOT_RES_BYTES]/MEGA);

      fprintf(fp, "MISC_MEMSIZE              (idx, 1)        = %1.2f;\n", 
	      RDB[DATA_TOT_MISC_BYTES]/MEGA);

      fprintf(fp, "UNKNOWN_MEMSIZE           (idx, 1)        = %1.2f;\n", 
	      (RDB[DATA_TOTAL_BYTES] - RDB[DATA_TOT_XS_BYTES] -
	       RDB[DATA_TOT_MAT_BYTES] - RDB[DATA_TOT_RES_BYTES] -
	       RDB[DATA_TOT_MISC_BYTES])/MEGA);

      fprintf(fp, "UNUSED_MEMSIZE            (idx, 1)        = %1.2f;\n", 
	      (RDB[DATA_REAL_BYTES] - RDB[DATA_TOTAL_BYTES])/MEGA);

      /***********************************************************************/
      
      /***** Geometry parameters *********************************************/
      
      fprintf(fp, "\n");
      
      fprintf(fp, "%% Geometry parameters:\n\n");

      fprintf(fp, "TOT_CELLS                 (idx, 1)        = %ld ;\n", 
	      (long)RDB[DATA_N_TOT_CELLS]);

      fprintf(fp, "UNION_CELLS               (idx, 1)        = %ld ;\n", 
	      (long)RDB[DATA_N_UNION_CELLS]);
      
      /***********************************************************************/

      /***** Energy grid parameters ******************************************/

      if ((long)RDB[DATA_NEUTRON_TRANSPORT_MODE] == YES)
	{
	  fprintf(fp, "\n");
	  
	  fprintf(fp, "%% Neutron energy grid:\n\n");
	  
	  /* Pointer to unionized grid */
	  
	  if ((ptr = (long)RDB[DATA_ERG_PTR_UNIONIZED_NGRID]) > VALID_PTR)
	    {      
	      fprintf(fp, "NEUTRON_ERG_TOL           (idx, 1)        = %12.5E ;\n", 
		      RDB[DATA_ERG_TOL]);
	      
	      fprintf(fp, "NEUTRON_ERG_NE            (idx, 1)        = %ld ;\n", 
		      (long)RDB[ptr + ENERGY_GRID_NE]);
	    }
	  
	  /* Minimum and maximum */
	  
	  fprintf(fp, "NEUTRON_EMIN              (idx, 1)        = %12.5E ;\n", 
		  RDB[DATA_NEUTRON_EMIN]); 
	  
	  fprintf(fp, "NEUTRON_EMAX              (idx, 1)        = %12.5E ;\n", 
		  RDB[DATA_NEUTRON_EMAX]);
	}

      if ((long)RDB[DATA_PHOTON_TRANSPORT_MODE] == YES)
	{  
	  fprintf(fp, "\n");

	  fprintf(fp, "%% Photon energy grid:\n\n");
	  
	  /* Pointer to unionized grid */
	  
	  if ((ptr = (long)RDB[DATA_ERG_PTR_UNIONIZED_PGRID]) > VALID_PTR)
	    fprintf(fp, "PHOTON_ERG_NE             (idx, 1)        = %ld ;\n", 
		    (long)RDB[ptr + ENERGY_GRID_NE]);
	  
	  /* Minimum and maximum */
	  
	  fprintf(fp, "PHOTON_EMIN               (idx, 1)        = %12.5E ;\n", 
		  RDB[DATA_PHOTON_EMIN]); 
	  
	  fprintf(fp, "PHOTON_EMAX               (idx, 1)        = %12.5E ;\n", 
		  RDB[DATA_PHOTON_EMAX]);
	}

      /***********************************************************************/
      
      /***** Unresolved resonance data ***************************************/

      if ((long)RDB[DATA_NEUTRON_TRANSPORT_MODE] == YES)
	{
	  fprintf(fp, "\n");
	  
	  fprintf(fp, 
		  "%% Unresolved resonance probability table sampling:\n\n");

	  fprintf(fp, "URES_DILU_CUT             (idx, 1)        = %12.5E ;\n", 
		  RDB[DATA_URES_DILU_CUT]);
	  
	  fprintf(fp, "URES_EMIN                 (idx, 1)        = %12.5E ;\n", 
		  RDB[DATA_URES_EMIN]); 
	  
	  fprintf(fp, "URES_EMAX                 (idx, 1)        = %12.5E ;\n", 
	      RDB[DATA_URES_EMAX]);
	  
	  fprintf(fp, "URES_AVAIL                (idx, 1)        = %ld ;\n", 
		  (long)RDB[DATA_URES_AVAIL]);
	  
	  fprintf(fp, "URES_USED                 (idx, 1)        = %ld ;\n", 
		  (long)RDB[DATA_URES_USED]);
	}

      /***********************************************************************/
      
      /***** Nuclides and reaction channels **********************************/
      
      fprintf(fp, "\n");
      
      fprintf(fp, "%% Nuclides and reaction channels:\n\n");
      
      fprintf(fp, "TOT_NUCLIDES              (idx, 1)        = %ld ;\n", 
	      (long)RDB[DATA_N_TOT_NUCLIDES]);
      
      fprintf(fp, "TOT_TRANSPORT_NUCLIDES    (idx, 1)        = %ld ;\n", 
	      (long)RDB[DATA_N_TRANSPORT_NUCLIDES]);

      fprintf(fp, "TOT_DOSIMETRY_NUCLIDES    (idx, 1)        = %ld ;\n", 
	      (long)RDB[DATA_N_DOSIMETRY_NUCLIDES]);
      
      fprintf(fp, "TOT_DECAY_NUCLIDES        (idx, 1)        = %ld ;\n", 
	      (long)RDB[DATA_N_DECAY_NUCLIDES]);

      fprintf(fp, "TOT_PHOTON_NUCLIDES       (idx, 1)        = %ld ;\n", 
	      (long)RDB[DATA_N_PHOTON_NUCLIDES]);
      
      fprintf(fp, "TOT_REA_CHANNELS          (idx, 1)        = %ld ;\n", 
	      (long)RDB[DATA_N_TRANSPORT_REA]);

      fprintf(fp, "TOT_TRANSMU_REA           (idx, 1)        = %ld ;\n", 
	      (long)RDB[DATA_N_TRANSMUTATION_REA]);

      /***********************************************************************/
      
      /***** Phisics options *************************************************/
      
      if ((long)RDB[DATA_NEUTRON_TRANSPORT_MODE] == YES)
	{
	  fprintf(fp, "\n");
      
	  fprintf(fp, "%% Neutron physics options:\n\n");

	  fprintf(fp, "USE_DELNU                 (idx, 1)        = %ld ;\n", 
		  (long)RDB[DATA_USE_DELNU]);
	  
	  fprintf(fp, "USE_URES                  (idx, 1)        = %ld ;\n", 
		  (long)RDB[DATA_USE_URES]);
	  
	  fprintf(fp, "USE_DBRC                  (idx, 1)        = %ld ;\n", 
		  (long)RDB[DATA_USE_DBRC]);
	  
	  fprintf(fp, "IMPL_CAPT                 (idx, 1)        = %ld ;\n", 
		  (long)RDB[DATA_OPT_IMPL_CAPT]);
	  
	  fprintf(fp, "IMPL_NXN                  (idx, 1)        = %ld ;\n", 
		  (long)RDB[DATA_OPT_IMPL_NXN]);
	  
	  fprintf(fp, "IMPL_FISS                 (idx, 1)        = %ld ;\n", 
		  (long)RDB[DATA_OPT_IMPL_FISS]);
	  
	  if ((long)RDB[DATA_OPT_IMPL_FISS] == YES)
	    fprintf(fp, "IMPL_FISS_NUBAR         (idx, 1)        = %12.5E ;\n", 
		    RDB[DATA_OPT_IMPL_FISS_NUBAR]);
	  
	  fprintf(fp, "DOPPLER_PREPROCESSOR      (idx, 1)        = %ld ;\n", 
		  (long)RDB[DATA_USE_DOPPLER_PREPROCESSOR]);
	  
	  fprintf(fp, "TMS_MODE                  (idx, 1)        = %ld ;\n", 
		  (long)RDB[DATA_TMS_MODE]);
	  
	  fprintf(fp, "SAMPLE_FISS               (idx, 1)        = %ld ;\n", 
		  (long)RDB[DATA_NPHYS_SAMPLE_FISS]);
	  
	  fprintf(fp, "SAMPLE_CAPT               (idx, 1)        = %ld ;\n", 
		  (long)RDB[DATA_NPHYS_SAMPLE_CAPT]);

	  fprintf(fp, "SAMPLE_SCATT              (idx, 1)        = %ld ;\n", 
		  (long)RDB[DATA_NPHYS_SAMPLE_SCATT]);
	}

      if ((long)RDB[DATA_PHOTON_TRANSPORT_MODE] == YES)
	{
	  fprintf(fp, "\n");
      
	  fprintf(fp, "%% Photon physics options:\n\n");

	  fprintf(fp, "COMPTON_EKN               (idx, 1)        = %12.5E ;\n", 
		  RDB[DATA_PHOTON_EKN]);

	  fprintf(fp, "COMPTON_DOPPLER           (idx, 1)        = %ld ;\n", 
		  (long)RDB[DATA_PHOTON_USE_DOPPLER]);

	  fprintf(fp, "COMPTON_EANG              (idx, 1)        = %ld ;\n", 
		  (long)RDB[DATA_PHOTON_COMP_EANG]);

	  fprintf(fp, "PHOTON_TTB                (idx, 1)        = %ld ;\n", 
		  (long)RDB[DATA_PHOTON_USE_TTB]);
	}

      /***********************************************************************/

      /***** Radioactivity data **********************************************/

      fprintf(fp, "\n");
    
      fprintf(fp, "%% Radioactivity data:\n\n");

      fprintf(fp, "TOT_ACTIVITY              (idx, 1)        = %12.5E ;\n", 
	      RDB[DATA_TOT_ACTIVITY]);
      fprintf(fp, "TOT_DECAY_HEAT            (idx, 1)        = %12.5E ;\n", 
	      RDB[DATA_TOT_DECAY_HEAT]);

      fprintf(fp, "TOT_SF_RATE               (idx, 1)        = %12.5E ;\n", 
	      RDB[DATA_TOT_SFRATE]);

      fprintf(fp, "ACTINIDE_ACTIVITY         (idx, 1)        = %12.5E ;\n", 
	      RDB[DATA_ACT_ACTIVITY]);
      fprintf(fp, "ACTINIDE_DECAY_HEAT       (idx, 1)        = %12.5E ;\n", 
	      RDB[DATA_ACT_DECAY_HEAT]);
      
      fprintf(fp, "FISSION_PRODUCT_ACTIVITY  (idx, 1)        = %12.5E ;\n", 
	      RDB[DATA_FP_ACTIVITY]);
      fprintf(fp, "FISSION_PRODUCT_DECAY_HEAT(idx, 1)        = %12.5E ;\n", 
	      RDB[DATA_FP_DECAY_HEAT]);

      fprintf(fp, "INHALATION_TOXICITY       (idx, 1)        = %12.5E ;\n", 
	      RDB[DATA_TOT_INH_TOX]);
      fprintf(fp, "INGESTION_TOXICITY        (idx, 1)        = %12.5E ;\n", 
	      RDB[DATA_TOT_ING_TOX]);

      fprintf(fp, "SR90_ACTIVITY             (idx, 1)        = %12.5E ;\n", 
	      RDB[DATA_SR90_ACTIVITY]);
      fprintf(fp, "TE132_ACTIVITY            (idx, 1)        = %12.5E ;\n", 
	      RDB[DATA_TE132_ACTIVITY]);
      fprintf(fp, "I131_ACTIVITY             (idx, 1)        = %12.5E ;\n", 
	      RDB[DATA_I131_ACTIVITY]);
      fprintf(fp, "I132_ACTIVITY             (idx, 1)        = %12.5E ;\n", 
	      RDB[DATA_I132_ACTIVITY]);
      fprintf(fp, "CS134_ACTIVITY            (idx, 1)        = %12.5E ;\n", 
	      RDB[DATA_CS134_ACTIVITY]);
      fprintf(fp, "CS137_ACTIVITY            (idx, 1)        = %12.5E ;\n", 
	      RDB[DATA_CS137_ACTIVITY]);

      fprintf(fp, "TOT_PHOTON_SRC            (idx, 1)        = %12.5E ;\n", 
	      RDB[DATA_TOT_PHOTON_SRC_RATE]);

      fprintf(fp, "\n");
      
      fprintf(fp, "%% Normaliation coefficient:\n\n");

      PrintValues(fp, "NORM_COEF", RES_NORM_COEF, 2, -1, -1, 0, 0);      

      /* Check photon transport mode */

      if ((long)RDB[DATA_PHOTON_TRANSPORT_MODE] == YES)
	{
	  /*******************************************************************/

	  /***** Total reaction rates for photons ****************************/

	  fprintf(fp, "\n");

	  fprintf(fp, "%% Normalized total reaction rates (photons):\n\n");

	  PrintValues(fp, "TOT_PHOTON_LEAKRATE", RES_TOT_PHOTON_LEAKRATE, 
		      1, -1, -1, 0, 0);
	  PrintValues(fp, "TOT_PHOTON_CUTRATE", RES_TOT_PHOTON_CUTRATE, 
		      1, -1, -1, 0, 0);
	  PrintValues(fp, "PHOTOELE_CAPT_RATE", RES_PHOTOELE_CAPT_RATE,
		      1, -1, -1, 0, 0);
	  PrintValues(fp, "PAIRPROD_CAPT_RATE", RES_PAIRPROD_CAPT_RATE,
		      1, -1, -1, 0, 0);
	  PrintValues(fp, "TOT_PHOTON_LOSSRATE", RES_TOT_PHOTON_LOSSRATE, 
		      1, -1, -1, 0, 0);
	  PrintValues(fp, "TOT_PHOTON_SRCRATE", RES_TOT_PHOTON_SRCRATE, 
		      1, -1, -1, 0, 0);
	  PrintValues(fp, "TOT_PHOTON_RR", RES_TOT_PHOTON_RR, 
		      1, -1, -1, 0, 0);
	  PrintValues(fp, "TOT_PHOTON_FLUX", RES_TOT_PHOTON_FLUX, 
		      1, -1, -1, 0, 0);
	  PrintValues(fp, "TOT_PHOTON_HEATRATE", RES_TOT_PHOTON_HEATRATE, 
		      1, -1, -1, 0, 0);

	  fprintf(fp, "\n");
	  
	  fprintf(fp, "%% Analog mean photon lifetime:\n\n");
	  
	  PrintValues(fp, "ANA_LIFETIME", RES_ANA_PHOTON_LIFETIME, 
		      1, -1, -1, 0, 0);
	  
	  /*******************************************************************/
	}

      /* Check neutron transport mode */

      if ((long)RDB[DATA_NEUTRON_TRANSPORT_MODE] == YES)
	{
	  /*******************************************************************/
      
	  /****** Parameters for burnup calculation **************************/
      
	  /* Check mode */
	  
	  if ((long)RDB[DATA_BURNUP_CALCULATION_MODE] == YES)
  	    {
	      fprintf(fp, "\n");
	      
	      fprintf(fp, "%% Parameters for burnup calculation:\n\n");
	      
	      fprintf(fp, "BURN_MATERIALS            (idx, 1)        = %ld ;\n", 
		      (long)RDB[DATA_N_BURN_MATERIALS]);
	      
	      fprintf(fp, "BURN_MODE                 (idx, 1)        = %ld ;\n", 
		      (long)RDB[DATA_BURN_BUMODE]);
	      
	      fprintf(fp, "BURN_STEP                 (idx, 1)        = %ld ;\n", 
		      (long)RDB[DATA_BURN_STEP]);
	      
	      /*
		fprintf(fp, "BURN_TOT_STEPS            (idx, 1)        = %ld ;\n", 
		(long)RDB[DATA_TOT_BURN_STEPS] + 1);
	      */
	      fprintf(fp, "BURNUP                     (idx, [1:  2])  = [ %12.5E %12.5E ];\n", 
		      RDB[DATA_BURN_CUM_BURNUP], RDB[DATA_BURN_CUM_REAL_BURNUP]);
	      fprintf(fp, "BURN_DAYS                 (idx, 1)        = %12.5E ;\n", 
		      RDB[DATA_BURN_CUM_BURNTIME]/(24.0*60.0*60.0));
	      
	      /*
		fprintf(fp, "ENERGY_OUTPUT             (idx, 1)        = %12.5E ;\n", 
		RDB[DATA_TOT_ENERGY_OUTPUT]*SECDAY/1E-6);
		
		fprintf(fp, "DEP_TTA_CUTOFF            (idx, 1)        = %12.5E ;\n", 
		RDB[DATA_DEP_TTA_CUTOFF]);
		fprintf(fp, "DEP_STABILITY_CUTOFF      (idx, 1)        = %12.5E ;\n", 
		RDB[DATA_DEP_STABILITY_CUTOFF]);
		fprintf(fp, "DEP_FP_YIELD_CUTOFF       (idx, 1)        = %12.5E ;\n", 
		RDB[DATA_DEP_FP_YIELD_CUTOFF]);
		fprintf(fp, "DEP_XS_FRAC_CUTOFF        (idx, 1)        = %12.5E ;\n", 
		RDB[DATA_DEP_XS_FRAC_CUTOFF]);
		fprintf(fp, "DEP_XS_ENERGY_CUTOFF      (idx, 1)        = %12.5E ;\n", 
		RDB[DATA_DEP_TRANSMU_E_CUTOFF]);
		
		fprintf(fp, "FP_NUCLIDES_INCLUDED      (idx, 1)        = %ld ;\n", 
		(long)RDB[DATA_BURN_INCL_FP_ISO]);
		
		fprintf(fp, "FP_NUCLIDES_AVAILABLE     (idx, 1)        = %ld ;\n", 
		(long)RDB[DATA_BURN_AVAIL_FP_ISO]);
		
		fprintf(fp, "TOT_ACTIVITY              (idx, 1)        = %12.5E ;\n", 
		RDB[DATA_TOT_ACTIVITY]);
		fprintf(fp, "TOT_DECAY_HEAT            (idx, 1)        = %12.5E ;\n", 
		RDB[DATA_TOT_DECAY_HEAT]);
		
		fprintf(fp, "TOT_SF_RATE               (idx, 1)        = %12.5E ;\n", 
		RDB[DATA_TOT_SFRATE]);
		
		fprintf(fp, "ACTINIDE_ACTIVITY         (idx, 1)        = %12.5E ;\n", 
		RDB[DATA_ACT_ACTIVITY]);
		fprintf(fp, "ACTINIDE_DECAY_HEAT       (idx, 1)        = %12.5E ;\n", 
		RDB[DATA_ACT_DECAY_HEAT]);
		
		fprintf(fp, "FISSION_PRODUCT_ACTIVITY  (idx, 1)        = %12.5E ;\n", 
		RDB[DATA_FP_ACTIVITY]);
		fprintf(fp, "FISSION_PRODUCT_DECAY_HEAT(idx, 1)        = %12.5E ;\n", 
		RDB[DATA_FP_DECAY_HEAT]);
	      */
	    }

	  /* Coefficient calculation */

	  if ((long)RDB[DATA_COEF_CALC_IDX] > -1)
	    {
	      fprintf(fp, "%% Coefficient calculation:\n\n");
                 
	      fprintf(fp, "COEF_IDX                (idx, [1:  2])  = [ %ld %ld ];\n", (long)RDB[DATA_COEF_CALC_RUN_IDX], (long)RDB[DATA_COEF_CALC_TOT_RUNS]);
	      fprintf(fp, "COEF_BRANCH             (idx, 1)        = %ld ;\n", (long)RDB[DATA_COEF_CALC_IDX]);
	      fprintf(fp, "COEF_BU_STEP            (idx, 1)        = %ld ;\n", (long)RDB[DATA_COEF_CALC_BU_IDX]);
	    }

	  /*******************************************************************/
    
	  /***** Analog reaction rate estimators *****************************/

	  fprintf(fp, "\n");
      
	  fprintf(fp, "%% Analog reaction rate estimators:\n\n");
	  
	  PrintValues(fp, "CONVERSION_RATIO", RES_ANA_CONV_RATIO, 
		      1, -1, -1, 0, 0);

	  ptr = (long)RDB[RES_ANA_FISS_FRAC];

	  if (Mean(ptr, 1, 0) > 0.0)
	    PrintValues(fp, "TH232_FISS", RES_ANA_FISS_FRAC, 
			1, 2, -1, 1, 0);
	  
	  if (Mean(ptr, 2, 0) > 0.0)
	    PrintValues(fp, "U233_FISS", RES_ANA_FISS_FRAC, 
			1, 2, -1, 2, 0);

	  if (Mean(ptr, 3, 0) > 0.0)
	    PrintValues(fp, "U235_FISS", RES_ANA_FISS_FRAC, 
			1, 2, -1, 3, 0);

	  if (Mean(ptr, 4, 0) > 0.0)
	    PrintValues(fp, "U238_FISS", RES_ANA_FISS_FRAC, 
			1, 2, -1, 4, 0);

	  if (Mean(ptr, 5, 0) > 0.0)
	    PrintValues(fp, "PU239_FISS", RES_ANA_FISS_FRAC, 
			1, 2, -1, 5, 0);

	  if (Mean(ptr, 6, 0) > 0.0)
	    PrintValues(fp, "PU240_FISS", RES_ANA_FISS_FRAC, 
			1, 2, -1, 6, 0);

	  if (Mean(ptr, 7, 0) > 0.0)
	    PrintValues(fp, "PU241_FISS", RES_ANA_FISS_FRAC, 
			1, 2, -1, 7, 0);

	  ptr = (long)RDB[RES_ANA_CAPT_FRAC];

	  if (Mean(ptr, 1, 0) > 0.0)
	    PrintValues(fp, "TH232_CAPT", RES_ANA_CAPT_FRAC, 
			1, 2, -1, 1, 0);
	  
	  if (Mean(ptr, 2, 0) > 0.0)
	    PrintValues(fp, "U233_CAPT", RES_ANA_CAPT_FRAC, 
			1, 2, -1, 2, 0);

	  if (Mean(ptr, 3, 0) > 0.0)
	    PrintValues(fp, "U235_CAPT", RES_ANA_CAPT_FRAC, 
			1, 2, -1, 3, 0);

	  if (Mean(ptr, 4, 0) > 0.0)
	    PrintValues(fp, "U238_CAPT", RES_ANA_CAPT_FRAC, 
			1, 2, -1, 4, 0);

	  if (Mean(ptr, 5, 0) > 0.0)
	    PrintValues(fp, "PU239_CAPT", RES_ANA_CAPT_FRAC, 
			1, 2, -1, 5, 0);

	  if (Mean(ptr, 6, 0) > 0.0)
	    PrintValues(fp, "PU240_CAPT", RES_ANA_CAPT_FRAC, 
			1, 2, -1, 6, 0);

	  if (Mean(ptr, 7, 0) > 0.0)
	    PrintValues(fp, "PU241_CAPT", RES_ANA_CAPT_FRAC, 
			1, 2, -1, 7, 0);

	  if (Mean(ptr, 8, 0) > 0.0)
	    PrintValues(fp, "XE135_CAPT", RES_ANA_CAPT_FRAC, 
			1, 2, -1, 8, 0);

	  if (Mean(ptr, 9, 0) > 0.0)
	    PrintValues(fp, "SM149_CAPT", RES_ANA_CAPT_FRAC, 
			1, 2, -1, 9, 0);

	  /*******************************************************************/
    
	  /***** Normalised reaction rates  **********************************/
      
	  fprintf(fp, "\n");
	  
	  fprintf(fp, "%% Normalized total reaction rates (neutrons):\n\n");
	  
	  if ((long)RDB[DATA_BURNUP_CALCULATION_MODE] == NO)
	    {        
	      PrintValues(fp, "TOT_POWER", RES_TOT_NEUTRON_POWER, 
			  1, -1, -1, 0, 0);
	      PrintValues(fp, "TOT_POWDENS", RES_TOT_POWDENS, 
			  1, -1, -1, 0, 0);
	      PrintValues(fp, "TOT_GENRATE", RES_TOT_GENRATE, 
			  1, -1, -1, 0, 0);
	      PrintValues(fp, "TOT_FISSRATE", RES_TOT_FISSRATE, 
			  1, -1, -1, 0, 0);
	      PrintValues(fp, "TOT_CAPTRATE", RES_TOT_CAPTRATE, 
			  1, -1, -1, 0, 0);
	      PrintValues(fp, "TOT_ABSRATE", RES_TOT_ABSRATE, 
			  1, -1, -1, 0, 0);
	      PrintValues(fp, "TOT_SRCRATE", RES_TOT_NEUTRON_SRCRATE, 
			  1, -1, -1, 0, 0);
	      PrintValues(fp, "TOT_FLUX", RES_TOT_NEUTRON_FLUX, 
			  1, -1, -1, 0, 0);
	    }
	  else
	    {
	      PrintValues(fp, "TOT_POWER", RES_TOT_NEUTRON_POWER, 
			  3, -1, -1, 0, 0);
	      PrintValues(fp, "TOT_POWDENS", RES_TOT_POWDENS, 
			  3, -1, -1, 0, 0);
	      PrintValues(fp, "TOT_GENRATE", RES_TOT_GENRATE, 
			  3, -1, -1, 0, 0);
	      PrintValues(fp, "TOT_FISSRATE", RES_TOT_FISSRATE, 
			  3, -1, -1, 0, 0);
	      PrintValues(fp, "TOT_CAPTRATE", RES_TOT_CAPTRATE, 
			  3, -1, -1, 0, 0);
	      PrintValues(fp, "TOT_ABSRATE", RES_TOT_ABSRATE, 
			  3, -1, -1, 0, 0);
	      PrintValues(fp, "TOT_SRCRATE", RES_TOT_NEUTRON_SRCRATE, 
			  3, -1, -1, 0, 0);
	      PrintValues(fp, "TOT_FLUX", RES_TOT_NEUTRON_FLUX, 
			  3, -1, -1, 0, 0);
	    }

	  PrintValues(fp, "TOT_LEAKRATE", RES_TOT_NEUTRON_LEAKRATE, 
		      1, -1, -1, 0, 0);
	  PrintValues(fp, "ALBEDO_LEAKRATE", RES_ALB_NEUTRON_LEAKRATE, 
		      1, -1, -1, 0, 0);
	  PrintValues(fp, "TOT_LOSSRATE", RES_TOT_NEUTRON_LOSSRATE, 
		      1, -1, -1, 0, 0);
	  PrintValues(fp, "TOT_CUTRATE", RES_TOT_NEUTRON_CUTRATE, 
		      1, -1, -1, 0, 0);
	  PrintValues(fp, "TOT_RR", RES_TOT_NEUTRON_RR, 1, -1, -1, 0, 0);

	  if ((long)RDB[DATA_XENON_EQUILIBRIUM_MODE] > -1)
	    PrintValues(fp, "TOT_XE135_ABSRATE", RES_XE135_ABSRATE, 1, -1, -1, 
			0, 0);

	  if ((long)RDB[DATA_SAMARIUM_EQUILIBRIUM_MODE] > -1)
	    PrintValues(fp, "TOT_SM149_ABSRATE", RES_SM149_ABSRATE, 1, -1, -1, 
			0, 0);

	  /* Fissile masses*/
	  
	  fprintf(fp, "INI_FMASS                 (idx, 1)        = %12.5E ;\n", 
		  RDB[DATA_INI_FMASS]); 
	  fprintf(fp, "TOT_FMASS                 (idx, 1)        = %12.5E ;\n", 
		  RDB[DATA_TOT_FMASS]); 
	  
	  if ((long)RDB[DATA_BURNUP_CALCULATION_MODE] == YES)
	    {        
	      fprintf(fp, 
		      "INI_BURN_FMASS            (idx, 1)        = %12.5E ;\n", 
		      RDB[DATA_INI_BURN_FMASS]); 
	      fprintf(fp, 
		      "TOT_BURN_FMASS            (idx, 1)        = %12.5E ;\n", 
		      RDB[DATA_TOT_BURN_FMASS]); 
	    }
	  
	  fprintf(fp, "\n");

	  /*******************************************************************/

	  /***** Equilibrium poison iteration ********************************/
      
	  if ((long)RDB[DATA_XENON_EQUILIBRIUM_MODE] > -1)
	    {
	      fprintf(fp, "%% Equilibrium Xe-135 iteration:\n\n");
	  
	      PrintValues(fp, "XE135_EQUIL_CONC", RES_XE135_EQUIL_CONC, 1, -1, 
			  -1, 0, 0);
	      PrintValues(fp, "I135_EQUIL_CONC", RES_I135_EQUIL_CONC, 1, -1, 
			  -1, 0, 0);

	      fprintf(fp, "\n");
	    }

	  if ((long)RDB[DATA_SAMARIUM_EQUILIBRIUM_MODE] > -1)
	    {
	      fprintf(fp, "%% Equilibrium Sm-149 iteration:\n\n");
	  
	      PrintValues(fp, "SM149_EQUIL_CONC", RES_SM149_EQUIL_CONC, 1, -1, 
			  -1, 0, 0);
	      PrintValues(fp, "PM149_EQUIL_CONC", RES_PM149_EQUIL_CONC, 1, -1, 
			  -1, 0, 0);

	      fprintf(fp, "\n");
	    }
      
	  /*
	  fprintf(fp, "%% Xe-135 entropy:\n\n");

	  fprintf(fp, "XE135_ENTROPY             (idx, 1)        = %12.5E ;\n", 
		  RDB[DATA_XENON_ENTROPY]);

	  fprintf(fp, "\n");
	  */

	  /*******************************************************************/
    
	  /***** Eigenvalues *************************************************/
      
	  fprintf(fp, "%% Fission neutron and energy production:\n\n");
	  
	  PrintValues(fp, "NUBAR", RES_TOT_NUBAR, 1, -1, -1, 0, 0);
	  PrintValues(fp, "FISSE", RES_TOT_FISSE, 1, -1, -1, 0, 0);

	  fprintf(fp, "\n%% Criticality eigenvalues:\n\n");
	  
	  if ((long)RDB[DATA_WIELANDT_MODE] == WIELANDT_MODE_NONE)
	    PrintValues(fp, "ANA_KEFF", RES_ANA_KEFF, 3, -1, -1, 0, 0);
	  else
	    PrintValues(fp, "ANA_KEFF", RES_ANA_KEFF, 1, -1, -1, 0, 0);
	  
	  PrintValues(fp, "IMP_KEFF", RES_IMP_KEFF, 1, -1, -1, 0, 0);
	  PrintValues(fp, "COL_KEFF", RES_COL_KEFF, 1, -1, -1, 0, 0);
	  PrintValues(fp, "ABS_KEFF", RES_IMP_KEFF, 1, -1, -1, 0, 0);
	  PrintValues(fp, "ABS_KINF", RES_IMP_KINF, 1, -1, -1, 0, 0);
	  
	  /* External source k-eff, source multiplication and */
	  /* mean number of generations */
	  
	  if (((long)RDB[DATA_SIMULATION_MODE] == SIMULATION_MODE_SRC) ||
	      ((long)RDB[DATA_SIMULATION_MODE] == SIMULATION_MODE_DYN) ||
	      ((long)RDB[DATA_SIMULATION_MODE] == SIMULATION_MODE_DELDYN))
	    {
	      PrintValues(fp, "ANA_EXT_K", RES_EXT_K, MAX_EXT_K_GEN, 
			  -1, -1, 0, 0);
	      PrintValues(fp, "SRC_MULT", RES_SRC_MULT, 1, -1, -1, 0, 0);
	      PrintValues(fp, "MEAN_NGEN", RES_MEAN_NGEN, 1, -1, -1, 0, 0);
	      
	      if ((np = (long)RDB[DATA_MAX_PROMPT_CHAIN_LENGTH]) > 0)
		{
		  PrintValues(fp, NULL, RES_PROMPT_GEN_POP, np, -1, -1, 0, 0);
		  PrintValues(fp, NULL, RES_PROMPT_GEN_CUMU, np, -1, -1, 0, 0);
		  PrintValues(fp, NULL, RES_PROMPT_GEN_TIMES, np, -1, -1, 0, 0);
		}

	      PrintValues(fp, NULL, RES_PROMPT_CHAIN_LENGTH, 1, -1, -1, 0, 0);
	    }

	  /* Albedo */

	  PrintValues(fp, "GEOM_ALBEDO", RES_GEOM_ALBEDO, 3, -1, -1, 0, 0);
	  
	  /* Check Wielandt method */

	  if ((long)RDB[DATA_WIELANDT_MODE] != WIELANDT_MODE_NONE)
	    {
	      fprintf(fp, "\n%% Wielandt method:\n\n");

	      PrintValues(fp, "WIELANDT_K", RES_WIELANDT_K,
			  1, -1, -1, 0, 0);
	      PrintValues(fp, "WIELANDT_P", RES_WIELANDT_P, 
			  1, -1, -1, 0, 0);
	    }

	  /* Alpha eigenvalues */
	  /*
	    PrintValues(fp, "IMPL_ALPHA_EIG", RES_IMPL_ALPHA_EIG, 1, -1, -1);
	    PrintValues(fp, "FIXED_ALPHA_EIG", RES_FIXED_ALPHA, 1, -1, -1);
	  */

	  fprintf(fp, "\n");

	  /*******************************************************************/
    
	  /***** Point-kinetic parameters ************************************/

	  /* Get number of delayed neutron precursor groups and IFP */
	  /* chain length */

	  nd = (long)RDB[DATA_PRECURSOR_GROUPS];
	  np = (long)RDB[DATA_IFP_CHAIN_LENGTH];

	  /* Forward-weighted time constants */

#ifdef SERPENT1_GC

	  fprintf(fp, "%% Forward-weighted time constants:\n\n");

	  PrintValues(fp, NULL, RES_FWD_IMP_GEN_TIME, 1, -1, -1, 0, 0);
	  PrintValues(fp, NULL, RES_FWD_IMP_LIFETIME, 1, -1, -1, 0, 0);

#else

	  fprintf(fp, "%% Forward-weighted delayed neutron parameters:\n\n");

#endif

	  PrintValues(fp, NULL, RES_FWD_ANA_BETA_ZERO, nd + 1, -1, -1, 0, 0);
	  PrintValues(fp, NULL, RES_FWD_ANA_LAMBDA, nd + 1, -1, -1, 0, 0);

	  /* Adjoint-weighted time constants */

	  fprintf(fp, "\n%% Beta-eff using Meulekamp's method:\n\n");

	  PrintValues(fp, NULL, RES_ADJ_MEULEKAMP_BETA_EFF, nd + 1, -1, -1, 
		      0, 0);
	  PrintValues(fp, NULL, RES_ADJ_MEULEKAMP_LAMBDA, nd + 1, -1, -1, 
		      0, 0);

	  /* Check IFP */

	  if (np > 0)
	    {
	      fprintf(fp, "\n%% Adjoint weighted time constants using Nauchi's method:\n\n");
	      PrintValues(fp, NULL, RES_ADJ_NAUCHI_GEN_TIME, 3, -1, -1, 0, 0);
	      PrintValues(fp, NULL, RES_ADJ_NAUCHI_LIFETIME, 3, -1, -1, 0, 0);
	      PrintValues(fp, NULL, RES_ADJ_NAUCHI_BETA_EFF, nd + 1, -1, -1, 0, 0);
	      PrintValues(fp, NULL, RES_ADJ_NAUCHI_LAMBDA, nd + 1, -1, -1, 0, 0);

	      fprintf(fp, "\n%% Adjoint weighted time constants using IFP:\n\n");

	      if ((long)RDB[DATA_IFP_OPT_PRINT_ALL] == NO)
		{
		  PrintValues(fp, NULL, RES_ADJ_IFP_GEN_TIME, 3, 1, -1, 0, 
			      np - 1);
		  PrintValues(fp, NULL, RES_ADJ_IFP_LIFETIME, 3, 1, -1, 0, 
			      np - 1);
		  PrintValues(fp, NULL, RES_ADJ_IFP_IMP_BETA_EFF, nd + 1, 1, 
			      -1, 0, np - 1);
		  PrintValues(fp, NULL, RES_ADJ_IFP_IMP_LAMBDA, nd + 1, 1, 
			      -1, 0, np - 1);
		  PrintValues(fp, NULL, RES_ADJ_IFP_ANA_BETA_EFF, nd + 1, 1, 
			      -1, 0, np - 1);
		  PrintValues(fp, NULL, RES_ADJ_IFP_ANA_LAMBDA, nd + 1, 1, 
			      -1, 0, np - 1);
		  PrintValues(fp, NULL, RES_ADJ_IFP_ROSSI_ALPHA, 1, -1, 
			      -1, np - 1, 0);
		}
	      else
		{
		  PrintValues(fp, NULL, RES_ADJ_IFP_GEN_TIME, 3, np, -1, 0, 0);
		  PrintValues(fp, NULL, RES_ADJ_IFP_LIFETIME, 3, np, -1, 0, 0);
		  PrintValues(fp, NULL, RES_ADJ_IFP_IMP_BETA_EFF, nd + 1, np, 
			      -1, 0, 0);
		  PrintValues(fp, NULL, RES_ADJ_IFP_IMP_LAMBDA, nd + 1, np, -1, 
			      0, 0);
		  PrintValues(fp, NULL, RES_ADJ_IFP_ANA_BETA_EFF, nd + 1, np, 
			      -1, 0, 0);
		  PrintValues(fp, NULL, RES_ADJ_IFP_ANA_LAMBDA, nd + 1, np, 
			      -1, 0, 0);
		  PrintValues(fp, NULL, RES_ADJ_IFP_ROSSI_ALPHA, np, -1, -1, 0, 
			      0);
		}
     
	      fprintf(fp, "\n%% Adjoint weighted time constants using perturbation technique:\n\n");
	      
	      PrintValues(fp, NULL, RES_ADJ_PERT_GEN_TIME, 1, -1, -1, 0, 0);
	      PrintValues(fp, NULL, RES_ADJ_PERT_LIFETIME, 1, -1, -1, 0, 0);
	      PrintValues(fp, NULL, RES_ADJ_PERT_BETA_EFF, 1, -1, -1, 
			  0, 0);
	      PrintValues(fp, NULL, RES_ADJ_PERT_ROSSI_ALPHA, 1, -1, -1, 0, 0);
	    }

	  fprintf(fp, "\n");

	  fprintf(fp, "%% Inverse neutron speed :\n\n");

	  PrintValues(fp, "ANA_INV_SPD", RES_TOT_RECIPVEL,
		      1, -1, -1, 0, 0);
	  
	  fprintf(fp, "\n");

	  fprintf(fp, "%% Analog slowing-down and thermal neutron lifetime (total/prompt/delayed):\n\n");

	  PrintValues(fp, "ANA_SLOW_TIME", RES_ANA_SLOW_TIME, 
		      3, -1, -1, 0, 0);

	  PrintValues(fp, "ANA_THERM_TIME", RES_ANA_THERM_TIME, 
		      3, -1, -1, 0, 0);

	  PrintValues(fp, "ANA_THERM_FRAC", RES_ANA_THERM_FRAC, 
		      3, -1, -1, 0, 0);

	  PrintValues(fp, "ANA_DELAYED_EMTIME", RES_ANA_DELAYED_EMTIME, 
		      1, -1, -1, 0, 0);


	  /* Average number of collisions per history (total and to fission) */
	  /* NOTE: Tää toimii oikein vain analogisessa simulaatiossa. */

	  if (1 != 2)
	    PrintValues(fp, "ANA_MEAN_NCOL", RES_ANA_MEAN_NCOL,	
			2, -1, -1, 0, 0);
	  
	  /*******************************************************************/

	  /***** Dynamic simulation ******************************************/

	  /* Check number of steps */

	  if ((nb = (long)RDB[DATA_DYN_NB]) > 1)
	    {
	      fprintf(fp, "\n");
	      
	      fprintf(fp, "%% Dynamic simulation\n\n");
	      
	      /* Get pointer to time bin structure */
	      
	      ptr = (long)RDB[DATA_DYN_PTR_TIME_BINS];
	      CheckPointer(FUNCTION_NAME, "(ptr)", DATA_ARRAY, ptr);
	      
	      /* Get pointer to bins */
	      
	      ptr = (long)RDB[ptr + TME_PTR_BINS];
	      CheckPointer(FUNCTION_NAME, "(ptr)", DATA_ARRAY, ptr);    

	      /* Print bin data */

	      fprintf(fp, "DYN_NB                    (idx, 1)        = %ld ;\n", nb);

	      fprintf(fp, "DYN_TMIN                  (idx, 1)        = %12.5E ;\n", RDB[DATA_DYN_TMIN]);
	      fprintf(fp, "DYN_TMAX                  (idx, 1)        = %12.5E ;\n", RDB[DATA_DYN_TMAX]);


	      fprintf(fp, "DYN_TIMES                 (idx, [1: %3ld]) = [ ", nb + 1);
	      for (n = 0; n < nb + 1; n++)
		fprintf(fp, "%12.5E ", RDB[ptr + n]);

	      fprintf(fp, "];\n");

	      PrintValues(fp, "DYN_POP", RES_DYN_POP, nb, -1, -1, 0, 0);
	      PrintValues(fp, "DYN_PERIOD", RES_DYN_PERIOD, nb, -1, -1, 0, 0);
	    }

	  /*******************************************************************/
    
	  if (gcu > VALID_PTR)
	    {

#ifdef SERPENT1_GC

	      /***** Parameters for group constant generation ****************/
	  
	      fprintf(fp, "\n");
	      
	      fprintf(fp, "%% Parameters for group constant generation\n\n");
	      
	      /* Pointer to universe */
	      
	      uni = (long)RDB[gcu + GCU_PTR_UNIV];
	      
	      /* Print name */
	      
	      sprintf(tmpstr, "%s", GetText(uni + UNIVERSE_PTR_NAME));
	      fprintf(fp, "GC_UNIVERSE_NAME          (idx, [1:%3ld])  = '%s' ;\n", 
		      (long)strlen(tmpstr), tmpstr);
	  
	      /*
		
		fprintf(fp, "GC_SYM                    (idx, 1)        = %ld ;\n",
		(long)RDB[DATA_GC_SYM]);
	      */
	      
	      
	      /* Get number of energy groups */
	      
	      ng = (long)RDB[DATA_ERG_FG_NG];
	      
	      /* Get pointer to energy group structure */
	      
	      ptr = (long)RDB[DATA_ERG_FG_PTR_GRID];
	      ptr = (long)RDB[ptr + ENERGY_GRID_PTR_DATA];
	      
	      fprintf(fp, "GC_NE                     (idx, 1)        = %ld ;\n", ng);
	      fprintf(fp, "GC_BOUNDS                 (idx, [1: %3ld]) = [ ", ng + 1);
	      
	      fprintf(fp, "%12.5E ", RDB[DATA_NEUTRON_EMAX]);
	      
	      for (n = ng - 1; n > 0; n--)
		fprintf(fp, "%12.5E ", RDB[ptr + n]);
	      
	      fprintf(fp, "%12.5E ];\n", RDB[DATA_NEUTRON_EMIN]);
	      
	      /***************************************************************/
      
	      /***** Few-group cross sections ********************************/
	  
	      fprintf(fp, "\n");
	      
	      fprintf(fp, "%% Few-group cross sections:\n\n");
	      
	      PrintValues(fp, "FLUX", gcu + GCU_RES_FG_FLX, 
			  ng + 1, -1, -1, 0, 0);
	      PrintValues(fp, "LEAK", gcu + GCU_RES_FG_LEAK, 
			  ng + 1, -1, -1, 0, 0);
	      PrintValues(fp, "TOTXS", gcu + GCU_RES_FG_TOTXS, 
			  ng + 1, -1, -1, 0, 0);
	      PrintValues(fp, "FISSXS", gcu + GCU_RES_FG_FISSXS, 
			  ng + 1, -1, -1, 0, 0);
	      PrintValues(fp, "CAPTXS", gcu + GCU_RES_FG_CAPTXS, 
			  ng + 1, -1, -1, 0, 0);
	      PrintValues(fp, "ABSXS", gcu + GCU_RES_FG_ABSXS, 
			  ng + 1, -1, -1, 0, 0);
	      PrintValues(fp, "RABSXS", gcu + GCU_RES_FG_RABSXS, 
			  ng + 1, -1, -1, 0, 0);
	      PrintValues(fp, "ELAXS", gcu + GCU_RES_FG_ELAXS, 
			  ng + 1, -1, -1, 0, 0);
	      PrintValues(fp, "INELAXS", gcu + GCU_RES_FG_INLXS, 
			  ng + 1, -1, -1, 0, 0);
	      PrintValues(fp, "SCATTXS", gcu + GCU_RES_FG_SCATTXS, 
			  ng + 1, -1, -1, 0, 0);
	      PrintValues(fp, "SCATTPRODXS", gcu + GCU_RES_FG_SCATTPRODXS, 
			  ng + 1, -1, -1, 0, 0);
	      PrintValues(fp, "REMXS", gcu + GCU_RES_FG_REMXS, 
			  ng + 1, -1, -1, 0, 0);
	      PrintValues(fp, "NUBAR", gcu + GCU_RES_FG_NUBAR, 
			  ng + 1, -1, -1, 0, 0);
	      PrintValues(fp, "NSF", gcu + GCU_RES_FG_NSF, 
			  ng + 1, -1, -1, 0, 0);
	      PrintValues(fp, "RECIPVEL", gcu + GCU_RES_FG_RECIPVEL, 
			  ng + 1, -1, -1, 0, 0);
	      PrintValues(fp, "FISSE", gcu + GCU_RES_FG_FISSE, 
			  ng + 1, -1, -1, 0, 0);
	
	      /***************************************************************/

	      /***** Fission product poisons *********************************/

	      fprintf(fp, "\n");
		  
	      fprintf(fp, "%% Fission product poison data:\n\n");
	      
	      PrintValues(fp,"I135PRODXS", gcu + GCU_RES_FG_I135_PROD_XS, 
			  ng + 1, -1, -1, 0, 0);
	      PrintValues(fp,"XE135PRODXS", gcu + GCU_RES_FG_XE135_PROD_XS, 
			  ng + 1, -1, -1, 0, 0);
	      PrintValues(fp,"PM149PRODXS", gcu + GCU_RES_FG_PM149_PROD_XS, 
			  ng + 1, -1, -1, 0, 0);
	      PrintValues(fp,"SM149PRODXS", gcu + GCU_RES_FG_SM149_PROD_XS, 
			  ng + 1, -1, -1, 0, 0);
	      PrintValues(fp,"I135ABSXS", gcu + GCU_RES_FG_I135_ABS_XS, 
			  ng + 1, -1, -1, 0, 0);
	      PrintValues(fp,"XE135ABSXS", gcu + GCU_RES_FG_XE135_ABS_XS, 
			  ng + 1, -1, -1, 0, 0);
	      PrintValues(fp,"PM149ABSXS", gcu + GCU_RES_FG_PM149_ABS_XS, 
			  ng + 1, -1, -1, 0, 0);
	      PrintValues(fp,"SM149ABSXS", gcu + GCU_RES_FG_SM149_ABS_XS, 
			  ng + 1, -1, -1, 0, 0);

	      /***************************************************************/
      
	      /***** Fission spectra *****************************************/

	      fprintf(fp, "\n");
	      
	      fprintf(fp, "%% Fission spectra:\n\n");
	      
	      PrintValues(fp, "CHI", gcu + GCU_RES_FG_CHI, ng, -1, -1, 0, 0);
	      PrintValues(fp, "CHIP", gcu + GCU_RES_FG_CHIP, ng, -1, -1, 0, 0);
	      PrintValues(fp, "CHID", gcu + GCU_RES_FG_CHID, ng, -1, -1, 0, 0);
	  
	      /****************************************************************/

	      /***** Group-transfer probabilities and cross sections **********/

	      fprintf(fp, "\n");
	      
	      fprintf(fp, "%% Group-transfer probabilities and cross sections:\n\n");
	  
	      PrintValues(fp, "GTRANSFP", gcu + GCU_RES_FG_GTRANSP,
			  ng, ng, -1, 0, 0);
	      PrintValues(fp, "GTRANSFXS", gcu + GCU_RES_FG_GTRANSXS,
			  ng, ng, -1, 0, 0);

	      fprintf(fp, "\n");
	      
	      fprintf(fp, "%% Group-production probabilities and cross sections:\n\n");
	  
	      PrintValues(fp, "GPRODP", gcu + GCU_RES_FG_GPRODP,
			  ng, ng, -1, 0, 0);
	      PrintValues(fp, "GPRODXS", gcu + GCU_RES_FG_GPRODXS,
			  ng, ng, -1, 0, 0);
	      
	      /****************************************************************/

	      /***** PN scattering cross sections *****************************/

	      fprintf(fp, "\n");
	      
	      fprintf(fp, "%% PN scattering cross sections:\n\n");
	      
	      PrintValues(fp, "SCATT0", gcu + GCU_RES_FG_SCATT0, 
			  ng + 1, -1, -1, 0, 0);
	      PrintValues(fp, "SCATT1", gcu + GCU_RES_FG_SCATT1, 
			  ng + 1, -1, -1, 0, 0);
	      PrintValues(fp, "SCATT2", gcu + GCU_RES_FG_SCATT2,
			  ng + 1, -1, -1, 0, 0);
	      PrintValues(fp, "SCATT3", gcu + GCU_RES_FG_SCATT3,
			  ng + 1, -1, -1, 0, 0);
	      PrintValues(fp, "SCATT4", gcu + GCU_RES_FG_SCATT4,
			  ng + 1, -1, -1, 0, 0);
	      PrintValues(fp, "SCATT5", gcu + GCU_RES_FG_SCATT5, 
			  ng + 1, -1, -1, 0, 0);
	  
	      fprintf(fp, "\n");
	      
	      fprintf(fp, "%% P1 diffusion parameters:\n\n");
	      
	      PrintValues(fp, "P1_TRANSPXS", gcu + GCU_RES_FG_P1_TRANSPXS, ng + 1, -1, -1, 0, 0);
	      PrintValues(fp, "P1_DIFFCOEF", gcu + GCU_RES_FG_P1_DIFFCOEF, ng + 1, -1, -1, 0, 0);
			      
	      PrintValues(fp, "P1_MUBAR", gcu + GCU_RES_FG_P1_MUBAR, 
			  ng + 1, -1, -1, 0, 0);

	      /***************************************************************/
	  
	      /***** B1 critical spectrum cross sections *********************/

	      /* Check if fundamental mode calculation is performed */
	      
	      if ((1 == 2) && ((long)RDB[DATA_B1_CALC] == YES))
		{
		  fprintf(fp, "\n");
		  
		  fprintf(fp, "%% B1 critical spectrum calculation:\n\n");

		  PrintValues(fp, "B1_KINF", gcu + GCU_FUM_FG_B1_KINF,
			      1, -1, -1, 0, 0);
		  PrintValues(fp, "B1_BUCKLING", gcu + GCU_FUM_FG_B1_BUCKLING,
			      1, -1, -1, 0, 0);
		  PrintValues(fp, "B1_FLUX", gcu + GCU_FUM_FG_B1_FLUX, 
			      ng + 1, -1, -1, 0, 0);
		  PrintValues(fp, "B1_TOTXS", gcu + GCU_FUM_FG_B1_TOTXS,
			      ng + 1, -1, -1, 0, 0);
		  PrintValues(fp, "B1_NSF", gcu + GCU_FUM_FG_B1_NSF,
			      ng + 1, -1, -1, 0, 0);
		  PrintValues(fp, "B1_FISSXS", gcu + GCU_FUM_FG_B1_FISSXS,
			      ng + 1, -1, -1, 0, 0);
		  PrintValues(fp, "B1_CHI", gcu + GCU_FUM_FG_B1_CHI, 
			      ng, -1, -1, 0, 0);
		  PrintValues(fp, "B1_ABSXS", gcu + GCU_FUM_FG_B1_ABSXS,
			      ng + 1, -1, -1, 0, 0);
		  PrintValues(fp, "B1_RABSXS", gcu + GCU_FUM_FG_B1_RABSXS,
			      ng + 1, -1, -1, 0, 0);
		  PrintValues(fp, "B1_REMXS", gcu + GCU_FUM_FG_B1_REMXS,
			      ng + 1, -1, -1, 0, 0);
		  PrintValues(fp, "B1_DIFFCOEF", gcu + GCU_FUM_FG_B1_DIFFCOEF,
			      ng + 1, -1, -1, 0, 0);
		  PrintValues(fp, "B1_SCATTXS", gcu + GCU_FUM_FG_B1_SCATTXS,
			      ng, ng, -1, 0, 0);
		  PrintValues(fp, "B1_SCATTPRODXS", 
			      gcu + GCU_FUM_FG_B1_SCATTPRODXS,
			      ng, ng, -1, 0, 0);
		  PrintValues(fp, "B1_I135PRODXS", 
			      gcu + GCU_FUM_FG_I135_PROD_XS, 
			      ng + 1, -1, -1, 0, 0);
		  PrintValues(fp, "B1_XE135PRODXS", 
			      gcu + GCU_FUM_FG_XE135_PROD_XS, 
			      ng + 1, -1, -1, 0, 0);
		  PrintValues(fp, "B1_PM149PRODXS", 
			      gcu + GCU_FUM_FG_PM149_PROD_XS, 
			      ng + 1, -1, -1, 0, 0);
		  PrintValues(fp, "B1_SM149PRODXS", 
			      gcu + GCU_FUM_FG_SM149_PROD_XS, 
			      ng + 1, -1, -1, 0, 0);
		  PrintValues(fp, "B1_I135ABSXS", 
			      gcu + GCU_FUM_FG_I135_ABS_XS, 
			      ng + 1, -1, -1, 0, 0);
		  PrintValues(fp, "B1_XE135ABSXS", 
			      gcu + GCU_FUM_FG_XE135_ABS_XS, 
			      ng + 1, -1, -1, 0, 0);
		  PrintValues(fp, "B1_PM149ABSXS", 
			      gcu + GCU_FUM_FG_PM149_ABS_XS, 
			      ng + 1, -1, -1, 0, 0);
		  PrintValues(fp, "B1_SM149ABSXS", 
			      gcu + GCU_FUM_FG_SM149_ABS_XS, 
			      ng + 1, -1, -1, 0, 0);
		}



#endif


	      /***************************************************************/

	      /***** New group constants *************************************/

	      fprintf(fp, "\n%% Group constant generation:\n\n");

	      /* Pointer to universe */
	      
	      uni = (long)RDB[gcu + GCU_PTR_UNIV];
	      
	      /* Print name */
      
	      sprintf(tmpstr, "%s", GetText(uni + UNIVERSE_PTR_NAME));
	      fprintf(fp, "GC_UNIVERSE_NAME          (idx, [1:%3ld])  = '%s' ;\n", 
		      (long)strlen(tmpstr), tmpstr);

	      /* Group structures */

	      fprintf(fp, "\n%% Micro- and macro-group structures:\n\n");

	      /* Pointer to micro-group structure */

	      ptr = (long)RDB[DATA_MICRO_PTR_EGRID];
	      CheckPointer(FUNCTION_NAME, "(ptr)", DATA_ARRAY, ptr);

	      /* Number of groups */

	      nmg = (long)RDB[ptr + ENERGY_GRID_NE];
	      
	      /* Pointer to data */

	      ptr = (long)RDB[ptr + ENERGY_GRID_PTR_DATA];
	      CheckPointer(FUNCTION_NAME, "(ptr)", DATA_ARRAY, ptr);	      

	      fprintf(fp, "MICRO_NG                  (idx, 1)        = %ld ;\n", nmg - 1);
	      fprintf(fp, "MICRO_E                   (idx, [1: %3ld]) = [ ", nmg);
	      for (n = 0; n < nmg; n++)
		fprintf(fp, "%12.5E ", RDB[ptr + n]);
	      
	      fprintf(fp, "];\n");

	      /* Pointer to macro-group structure */

	      ptr = (long)RDB[DATA_ERG_FG_PTR_GRID];
	      CheckPointer(FUNCTION_NAME, "(ptr)", DATA_ARRAY, ptr);

	      /* Number of groups */

	      ng = (long)RDB[ptr + ENERGY_GRID_NE] - 1;
	      
	      /* Pointer to data */

	      ptr = (long)RDB[ptr + ENERGY_GRID_PTR_DATA];
	      CheckPointer(FUNCTION_NAME, "(ptr)", DATA_ARRAY, ptr);	      

	      fprintf(fp, "\nMACRO_NG                  (idx, 1)        = %ld ;\n", ng);
	      fprintf(fp, "MACRO_E                   (idx, [1: %3ld]) = [ ", ng + 1);
	      for (n = ng; n > -1; n--)
		fprintf(fp, "%12.5E ", RDB[ptr + n]);
	      
	      fprintf(fp, "];\n");

	      /***************************************************************/

	      /***** Infinite spectrum ***************************************/

	      /* Micro-group spectrum */

	      fprintf(fp, "\n%% Micro-group spectrum:\n\n");

	      PrintValues(fp, NULL, gcu + GCU_INF_MICRO_FLX, nmg - 1,
			  -1, -1, 0, 0);

	      /* Integral parameters */

	      fprintf(fp, "\n%% Integral parameters:\n\n");

	      PrintValues(fp, NULL, gcu + GCU_INF_KINF, 1, -1, -1, 0, 0);

	      /* Flux */

	      fprintf(fp, "\n%% Flux spectrum in infinite geometry:\n\n");

	      PrintValues(fp, NULL, gcu + GCU_INF_FLX, ng, -1, -1, 0, 0);

	      /* 1D-cross sections */

	      fprintf(fp, "\n%% Reaction cross sections:\n\n");
	      
	      PrintValues(fp, NULL, gcu + GCU_INF_TOT, ng, -1, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_INF_CAPT, ng, -1, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_INF_ABS, ng, -1, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_INF_FISS, ng, -1, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_INF_NSF, ng, -1, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_INF_NUBAR, ng, -1, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_INF_KAPPA, ng, -1, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_INF_INVV, ng, -1, -1, 0, 0);

	      /* 1D-scattering cross sections */

	      fprintf(fp, "\n%% Total scattering cross sections:\n\n");

	      PrintValues(fp, NULL, gcu + GCU_INF_SCATT0, ng, -1, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_INF_SCATT1, ng, -1, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_INF_SCATT2, ng, -1, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_INF_SCATT3, ng, -1, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_INF_SCATT4, ng, -1, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_INF_SCATT5, ng, -1, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_INF_SCATT6, ng, -1, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_INF_SCATT7, ng, -1, -1, 0, 0);

	      /* 1D-scattering production cross sections */

	      fprintf(fp, 
		      "\n%% Total scattering production cross sections:\n\n");

	      PrintValues(fp, NULL, gcu + GCU_INF_SCATTP0, ng, -1, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_INF_SCATTP1, ng, -1, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_INF_SCATTP2, ng, -1, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_INF_SCATTP3, ng, -1, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_INF_SCATTP4, ng, -1, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_INF_SCATTP5, ng, -1, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_INF_SCATTP6, ng, -1, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_INF_SCATTP7, ng, -1, -1, 0, 0);

	      /* Transport cross section and diffusion coefficient */

	      fprintf(fp, "\n%% Diffusion parameters:\n\n");

	      PrintValues(fp, NULL, gcu + GCU_INF_TRANSPXS, ng, -1, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_INF_DIFFCOEF, ng, -1, -1, 0, 0);

	      /* Reduced absorption and removal cross sections */

	      fprintf(fp, "\n%% Reduced absoption and removal:\n\n");
	      
	      PrintValues(fp, NULL, gcu + GCU_INF_RABSXS, ng, -1, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_INF_REMXS, ng, -1, -1, 0, 0);

	      /* Poison cross sections */

	      fprintf(fp, "\n%% Poison cross sections:\n\n");

	      PrintValues(fp, NULL, gcu + GCU_INF_I135_YIELD, ng, -1, -1, 0,0);
	      PrintValues(fp, NULL, gcu + GCU_INF_XE135_YIELD, ng, -1, -1, 0,0);
	      PrintValues(fp, NULL, gcu + GCU_INF_PM149_YIELD, ng, -1, -1, 0,0);
	      PrintValues(fp, NULL, gcu + GCU_INF_SM149_YIELD, ng, -1, -1, 0,0);

	      PrintValues(fp, NULL, gcu + GCU_INF_I135_ABS, ng, -1, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_INF_XE135_ABS, ng, -1, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_INF_PM149_ABS, ng, -1, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_INF_SM149_ABS, ng, -1, -1, 0, 0);

	      PrintValues(fp, NULL, gcu + GCU_INF_XE135_MACRO_ABS, ng, -1, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_INF_SM149_MACRO_ABS, ng, -1, -1, 0, 0);

	      /* Fission spectra */

	      fprintf(fp, "\n%% Fission spectra:\n\n");

	      PrintValues(fp, NULL, gcu + GCU_INF_CHIT, ng, -1, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_INF_CHIP, ng, -1, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_INF_CHID, ng, -1, -1, 0, 0);

	      /* Scattering matrixes */

	      fprintf(fp, "\n%% Scattering matrixes:\n\n");

	      PrintValues(fp, NULL, gcu + GCU_INF_S0, ng, ng, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_INF_S1, ng, ng, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_INF_S2, ng, ng, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_INF_S3, ng, ng, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_INF_S4, ng, ng, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_INF_S5, ng, ng, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_INF_S6, ng, ng, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_INF_S7, ng, ng, -1, 0, 0);

	      fprintf(fp, "\n%% Scattering production matrixes:\n\n");

	      PrintValues(fp, NULL, gcu + GCU_INF_SP0, ng, ng, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_INF_SP1, ng, ng, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_INF_SP2, ng, ng, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_INF_SP3, ng, ng, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_INF_SP4, ng, ng, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_INF_SP5, ng, ng, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_INF_SP6, ng, ng, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_INF_SP7, ng, ng, -1, 0, 0);

	      /***************************************************************/

	      /***** Critical spectrum ***************************************/

	      /* Micro-group spectrum */

	      fprintf(fp, "\n%% Micro-group spectrum:\n\n");

	      PrintValues(fp, NULL, gcu + GCU_B1_MICRO_FLX, nmg - 1,
			  -1, -1, 0, 0);

	      /* Integral parameters */

	      fprintf(fp, "\n%% Integral parameters:\n\n");

	      PrintValues(fp, NULL, gcu + GCU_B1_KINF, 1, -1, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_B1_KEFF, 1, -1, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_B1_B2, 1, -1, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_B1_ERR, 1, -1, -1, 0, 0);

	      /* Flux */

	      fprintf(fp, "\n%% Critical spectrum in infinite geometry:\n\n");

	      PrintValues(fp, NULL, gcu + GCU_B1_FLX, ng, -1, -1, 0, 0);

	      /* 1D-cross sections */

	      fprintf(fp, "\n%% Reaction cross sections:\n\n");
	      
	      PrintValues(fp, NULL, gcu + GCU_B1_TOT, ng, -1, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_B1_CAPT, ng, -1, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_B1_ABS, ng, -1, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_B1_FISS, ng, -1, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_B1_NSF, ng, -1, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_B1_NUBAR, ng, -1, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_B1_KAPPA, ng, -1, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_B1_INVV, ng, -1, -1, 0, 0);

	      /* 1D-scattering cross sections */

	      fprintf(fp, "\n%% Total scattering cross sections:\n\n");

	      PrintValues(fp, NULL, gcu + GCU_B1_SCATT0, ng, -1, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_B1_SCATT1, ng, -1, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_B1_SCATT2, ng, -1, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_B1_SCATT3, ng, -1, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_B1_SCATT4, ng, -1, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_B1_SCATT5, ng, -1, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_B1_SCATT6, ng, -1, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_B1_SCATT7, ng, -1, -1, 0, 0);

	      /* 1D-scattering production cross sections */

	      fprintf(fp, 
		      "\n%% Total scattering production cross sections:\n\n");

	      PrintValues(fp, NULL, gcu + GCU_B1_SCATTP0, ng, -1, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_B1_SCATTP1, ng, -1, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_B1_SCATTP2, ng, -1, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_B1_SCATTP3, ng, -1, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_B1_SCATTP4, ng, -1, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_B1_SCATTP5, ng, -1, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_B1_SCATTP6, ng, -1, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_B1_SCATTP7, ng, -1, -1, 0, 0);

	      /* Transport cross section and diffusion coefficient */

	      fprintf(fp, "\n%% Diffusion parameters:\n\n");

	      PrintValues(fp, NULL, gcu + GCU_B1_TRANSPXS, ng, -1, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_B1_DIFFCOEF, ng, -1, -1, 0, 0);

	      /* Reduced absorption and removal cross sections */

	      fprintf(fp, "\n%% Reduced absoption and removal:\n\n");
	      
	      PrintValues(fp, NULL, gcu + GCU_B1_RABSXS, ng, -1, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_B1_REMXS, ng, -1, -1, 0, 0);

	      /* Poison cross sections */

	      fprintf(fp, "\n%% Poison cross sections:\n\n");

	      PrintValues(fp, NULL, gcu + GCU_B1_I135_YIELD, ng, -1, -1, 0,0);
	      PrintValues(fp, NULL, gcu + GCU_B1_XE135_YIELD, ng, -1, -1, 0,0);
	      PrintValues(fp, NULL, gcu + GCU_B1_PM149_YIELD, ng, -1, -1, 0,0);
	      PrintValues(fp, NULL, gcu + GCU_B1_SM149_YIELD, ng, -1, -1, 0,0);

	      PrintValues(fp, NULL, gcu + GCU_B1_I135_ABS, ng, -1, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_B1_XE135_ABS, ng, -1, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_B1_PM149_ABS, ng, -1, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_B1_SM149_ABS, ng, -1, -1, 0, 0);

	      PrintValues(fp, NULL, gcu + GCU_B1_XE135_MACRO_ABS, ng, -1, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_B1_SM149_MACRO_ABS, ng, -1, -1, 0, 0);

	      /* Fission spectra */

	      fprintf(fp, "\n%% Fission spectra:\n\n");

	      PrintValues(fp, NULL, gcu + GCU_B1_CHIT, ng, -1, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_B1_CHIP, ng, -1, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_B1_CHID, ng, -1, -1, 0, 0);

	      /* Scattering matrixes */

	      fprintf(fp, "\n%% Scattering matrixes:\n\n");

	      PrintValues(fp, NULL, gcu + GCU_B1_S0, ng, ng, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_B1_S1, ng, ng, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_B1_S2, ng, ng, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_B1_S3, ng, ng, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_B1_S4, ng, ng, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_B1_S5, ng, ng, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_B1_S6, ng, ng, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_B1_S7, ng, ng, -1, 0, 0);

	      fprintf(fp, "\n%% Scattering production matrixes:\n\n");

	      PrintValues(fp, NULL, gcu + GCU_B1_SP0, ng, ng, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_B1_SP1, ng, ng, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_B1_SP2, ng, ng, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_B1_SP3, ng, ng, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_B1_SP4, ng, ng, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_B1_SP5, ng, ng, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_B1_SP6, ng, ng, -1, 0, 0);
	      PrintValues(fp, NULL, gcu + GCU_B1_SP7, ng, ng, -1, 0, 0);

	      /***************************************************************/

	      /***** Delayed neutron parameters ******************************/

	      /* Get number of delayed neutron precursor groups and IFP */
	      /* chain length */

	      nd = (long)RDB[DATA_PRECURSOR_GROUPS];
	      np = (long)RDB[DATA_IFP_CHAIN_LENGTH];

	      fprintf(fp, "\n");
	      
	      fprintf(fp, "%% Delayed neutron parameters (Meulekamp method):\n\n");

	      PrintValues(fp, NULL, gcu + GCU_MEULEKAMP_BETA_EFF, nd + 1, 1, 
			  -1, 0, np - 1);
	      PrintValues(fp, NULL, gcu + GCU_MEULEKAMP_LAMBDA, nd + 1, 1, 
			  -1, 0, np - 1);

	      /***************************************************************/
	      
	      /***** Assembly discontinuity factors **************************/

	      /* Get pointer */

	      if ((adf = (long)RDB[gcu + GCU_PTR_ADF]) > VALID_PTR)
		{
		  fprintf(fp, "\n");
		  /*
		  Warn(FUNCTION_NAME, "Tää järjestys pitää korjata sinne cax-outputtiin");
		  */
		  fprintf(fp, "%% Assembly discontinuity factors (order: W-S-E-N / NW-NE-SE-SW):\n\n");

		  /* Get number of surfaces and corners */

		  ns = (long)RDB[adf + ADF_NSURF];
		  nc = (long)RDB[adf + ADF_NCORN];

		  /* Print geometry data */

		  ptr = (long)RDB[adf + ADF_PTR_SURF];
		  CheckPointer(FUNCTION_NAME, "(ptr)", DATA_ARRAY, ptr);

		  sprintf(tmpstr, "%s", GetText(ptr + SURFACE_PTR_NAME));
		  fprintf(fp, "DF_SURFACE                (idx, [1:%3ld])  = '%s' ;\n", 
		      (long)strlen(tmpstr), tmpstr);

		  fprintf(fp, "DF_SYM                    (idx, 1)        = %ld ;\n", (long)RDB[adf + ADF_SYM]);

		  fprintf(fp, "DF_N_SURF                 (idx, 1)        = %ld ;\n", ns);

		  if (nc > 0)
		    fprintf(fp, "DF_N_CORN                 (idx, 1)        = %ld ;\n", nc);

		  fprintf(fp, "DF_VOLUME                 (idx, 1)        = %12.5E ;\n", RDB[adf + ADF_VOL]);


		  ptr = (long)RDB[adf + ADF_PTR_SURF_AREA];
		  CheckPointer(FUNCTION_NAME, "(ptr)", DATA_ARRAY, ptr);

		  fprintf(fp, "DF_SURF_AREA              (idx, [1:%3ld])  = [", ns);
		  for (n = 0; n < ns; n++)
		    fprintf(fp, "%12.5E ", RDB[ptr++]);
		  fprintf(fp, "];\n");

		  ptr = (long)RDB[adf + ADF_PTR_MID_AREA];
		  CheckPointer(FUNCTION_NAME, "(ptr)", DATA_ARRAY, ptr);

		  fprintf(fp, "DF_MID_AREA               (idx, [1:%3ld])  = [", ns);
		  for (n = 0; n < ns; n++)
		    fprintf(fp, "%12.5E ", RDB[ptr++]);
		  fprintf(fp, "];\n");

		  if (nc > 0)
		    {
		      ptr = (long)RDB[adf + ADF_PTR_CORN_AREA];
		      CheckPointer(FUNCTION_NAME, "(ptr)", DATA_ARRAY, ptr);
		      
		      fprintf(fp, "DF_CORN_AREA              (idx, [1:%3ld])  = [", nc);
		      for (n = 0; n < nc; n++)
			fprintf(fp, "%12.5E ", RDB[ptr++]);
		      fprintf(fp, "];\n");
		    }
		  
		  /* Print values */

		  PrintValues(fp, "DF_SURF_IN_CURR", gcu + GCU_RES_FG_DF_SURF_IN_CURR, 
			      ns, ng, -1, 0, 0);

		  PrintValues(fp, "DF_SURF_OUT_CURR", gcu + GCU_RES_FG_DF_SURF_OUT_CURR, 
			      ns, ng, -1, 0, 0);

		  PrintValues(fp, "DF_SURF_NET_CURR", gcu + GCU_RES_FG_DF_SURF_NET_CURR, 
			      ns, ng, -1, 0, 0);

		  PrintValues(fp, "DF_MID_IN_CURR", gcu + GCU_RES_FG_DF_MID_IN_CURR, 
			      ns, ng, -1, 0, 0);

		  PrintValues(fp, "DF_MID_OUT_CURR", gcu + GCU_RES_FG_DF_MID_OUT_CURR, 
			      ns, ng, -1, 0, 0);

		  PrintValues(fp, "DF_MID_NET_CURR", gcu + GCU_RES_FG_DF_MID_NET_CURR, 
			      ns, ng, -1, 0, 0);

		  if (nc > 0)
		    {
		      PrintValues(fp, "DF_CORN_IN_CURR", gcu + GCU_RES_FG_DF_CORN_IN_CURR, 
				  nc, ng, -1, 0, 0);

		      PrintValues(fp, "DF_CORN_OUT_CURR", gcu + GCU_RES_FG_DF_CORN_OUT_CURR, 
				  nc, ng, -1, 0, 0);

		      PrintValues(fp, "DF_CORN_NET_CURR", gcu + GCU_RES_FG_DF_CORN_NET_CURR, 
				  nc, ng, -1, 0, 0);
		    }

		  PrintValues(fp, "DF_HET_VOL_FLUX", gcu + GCU_RES_FG_DF_HET_VOL_FLUX, ng, 
			      -1, -1, 0, 0);

		  PrintValues(fp, "DF_HET_SURF_FLUX", gcu + GCU_RES_FG_DF_HET_SURF_FLUX, 
			      ns, ng, -1, 0, 0);

		  if (nc > 0)
		    PrintValues(fp, "DF_HET_CORN_FLUX", gcu + GCU_RES_FG_DF_HET_CORN_FLUX, 
				nc, ng, -1, 0, 0);

		  PrintValues(fp, "DF_HOM_VOL_FLUX", gcu + GCU_RES_FG_DF_HOM_VOL_FLUX, ng, 
			      -1, -1, 0, 0);
		  PrintValues(fp, "DF_HOM_SURF_FLUX", gcu + GCU_RES_FG_DF_HOM_SURF_FLUX, 
			      ns, ng, -1, 0, 0);

		  if (nc > 0)
		    PrintValues(fp, "DF_HOM_CORN_FLUX", gcu + GCU_RES_FG_DF_HOM_CORN_FLUX, 
				nc, ng, -1, 0, 0);

		  PrintValues(fp, "DF_SURF_DF", gcu + GCU_RES_FG_DF_SURF_DF, 
			      ns, ng, -1, 0, 0);

		  if (nc > 0)
		    PrintValues(fp, "DF_CORN_DF", gcu + GCU_RES_FG_DF_CORN_DF, 
				nc, ng, -1, 0, 0);
		}

	      /***************************************************************/

	      /***** Pin-power distributions *********************************/
	      
	      /* Get pointer */

	      if ((ppw = (long)RDB[gcu + GCU_PTR_PPW]) > VALID_PTR)
		{
		  fprintf(fp, "\n");

		  fprintf(fp, "%% Pin-power distribution:\n\n");

		  /* Get number of pins */

		  n = (long)RDB[ppw + PPW_NP];

		  /* Print geometry data */

		  ptr = (long)RDB[ppw + PPW_PTR_LAT];
		  sprintf(tmpstr, "%s", GetText(ptr + LAT_PTR_NAME));
		  fprintf(fp, "PPW_LATTICE               (idx, [1:%3ld])  = '%s' ;\n", 
			  (long)strlen(tmpstr), tmpstr);
		  
		  fprintf(fp, "PPW_LATTICE_TYPE          (idx, 1)        = %ld ;\n", (long)RDB[ptr + LAT_TYPE]);
		  fprintf(fp, "PPW_PINS                  (idx, 1)        = %ld ;\n", (long)RDB[ppw + PPW_NP]);
		  
		  /* Print values */

		  PrintValues(fp, "PPW_POW_FRAC", gcu + GCU_RES_FG_PPW_POW, 
			      n, ng, -1, 0, 0);
		  PrintValues(fp, "PPW_HOM_FLUX", gcu + GCU_RES_FG_PPW_HOM_FLUX, 
			      n, ng, -1, 0, 0);
		  PrintValues(fp, "PPW_FF", gcu + GCU_RES_FG_PPW_FF, 
			      n, ng, -1, 0, 0);
		}

	      /***************************************************************/

	      /***** Albedos and partial albedos *****************************/
	      
	      /* Get pointer */

	      if ((alb = (long)RDB[gcu + GCU_PTR_ALB]) > VALID_PTR)
		{
		  fprintf(fp, "\n");

		  fprintf(fp, "%% Albedos and partial albedos:\n\n");

		  /* Get number of surfaces */

		  n = (long)RDB[alb + ALB_NSURF];

		  /* Print geometry data */

		  ptr = (long)RDB[alb + ALB_PTR_SURF];
		  sprintf(tmpstr, "%s", GetText(ptr + SURFACE_PTR_NAME));
		  fprintf(fp, "ALB_SURFACE               (idx, [1:%3ld])  = '%s' ;\n", 
			  (long)strlen(tmpstr), tmpstr);
		  fprintf(fp, "ALB_FLIP_DIR              (idx, 1)        = %ld ;\n", 
			  (long)RDB[alb + ALB_DIR]);

		  fprintf(fp, "ALB_N_SURF                (idx, 1)        = %ld ;\n", n);

		  /* Print values */

		  PrintMVar(fp, gcu + GCU_RES_FG_ALB_IN_CURR);
		  PrintMVar(fp, gcu + GCU_RES_FG_ALB_OUT_CURR);
		  PrintMVar(fp, gcu + GCU_RES_FG_TOT_ALB);
		  PrintMVar(fp, gcu + GCU_RES_FG_PART_ALB);
		}

	      /***************************************************************/

	      /* Next universe */
	      
	      gcu = NextItem(gcu);
	    }
	}
      
      fprintf(fp, "\n");
    }
  while (gcu > VALID_PTR);

  /* Close file and exit */

  fclose(fp);
}

/*****************************************************************************/
