#ifdef __cplusplus 
extern "C" { 
#endif 
/*****************************************************************************/
/*                                                                           */
/* serpent 2 (beta-version) : tracking.c                                     */
/*                                                                           */
/* Created:       2011/03/10 (JLe)                                           */
/* Last modified: 2016/02/17 (VVa)                                           */
/* Version:       2.1.25                                                     */
/*                                                                           */
/* Description: Main tracking loop for single particle and secondaries       */
/*                                                                           */
/* Comments: - Fission neutrons are treated as secondaries in external       */
/*             source mode. In criticality source mode they form the         */
/*             source distribution for the next generation. In dynamic       */
/*             simulations fission neutrons are redistributed between the    */
/*             OpenMP-threads to increase parallel efficiency                */
/*                                                                           */
/*           - Revised 14.8.2014 / 2.1.22                                    */
/*                                                                           */
/*           - Density factorin käsittelyä on muutettu niin että se on nyt   */
/*             pelkkä rejektio siinä vaiheessa kun arvotaan että mennäänkö   */
/*             törmäykseen vai ei. Vuo otetaan kuitenkin täyden tiheyden     */
/*             vaikutusalasta. Näyttää siltä että makroskooppiset reaktio-   */
/*             nopeudet saattaa mennä tolla tavalla ihan tsägällä oikein,    */
/*             lukuunottamatta niitä rutiineja joissa kutsutaan MacroXS():ää */
/*             kokonaisvaikutusalalle.                                       */
/*                                                                           */
/*****************************************************************************/

#include "header.h"
#include "locations.h"

#define FUNCTION_NAME "Tracking:"

/*****************************************************************************/

void Tracking(long id)
{
  long part, cell, mat, type, loop, ptr, loc0, trk, mode, bc, gmax, n, lmax;
  long quen, next;
  double x, y, z, u, v, w, E, wgt, l, totxs, majorant, minxs, xs;
  double spd, t, dt, g, x0, y0, z0, t0;

  /* Add to OpenMP history counter (onko tää vähän turha?, tää menee */
  /* pieleen dynamic criticality source moodissa) */
 
  ptr = (long)RDB[DATA_PTR_OMP_HISTORY_COUNT];
  AddPrivateData(ptr, 1.0, id);

  /* Reset maximum generation and avoid compiler warning */

  gmax = 0;
  t = -1.0;

  /* Reset que count */

  quen = 0;

  /* Reset next particle pointer */

  next = 0;

  /* In the new dynamic mode we start from the end of the que     */
  /* and go backwards until we reach the dummy in the beginning   */
  /* this way we can add the secondaries in the end of the que    */
  /* and still only simulate the primaries during one call of     */
  /* Tracking() */

  if ((RDB[DATA_SIMULATION_MODE] != SIMULATION_MODE_CRIT) &&
      (RDB[DATA_SIMULATION_MODE] != SIMULATION_MODE_SRC))
    {
      /* Get pointer to que */

      next = (long)RDB[OMPPtr(DATA_PART_PTR_QUE, id)];
      CheckPointer(FUNCTION_NAME, "(next)", DATA_ARRAY, next);

      /* Get last particle */

      next = LastItem(next);
      CheckPointer(FUNCTION_NAME, "(ptr)", DATA_ARRAY, ptr);

    }

  /* Main loop over particle history and secondaries */

  /* Secondaries are only simulated straightaway in  */
  /* the traditional simulation modes (CRIT/SRC)     */
  /* In the new dynamic mode, they are redistributed */
  /* between the OpenMP threads to increase parallel */
  /* efficiency */

  while (1 == 1)
    {

      /* Traditional simulation modes */

      if ((RDB[DATA_SIMULATION_MODE] == SIMULATION_MODE_CRIT) ||
	  (RDB[DATA_SIMULATION_MODE] == SIMULATION_MODE_SRC))
	{
	  /* Get next particle from que or break the while loop */

	  if ((part = FromQue(id)) < VALID_PTR)
	    break;
	}
      else
	{
	  /* If we have reached the beginning of the que */
	  /* we have simulated all of the primaries      */

	  if (RDB[next + PARTICLE_TYPE] == PARTICLE_TYPE_DUMMY)
	    break;

	  /* There is still at least one particle to simulate */

	  /* Get the particle to simulate on next cycle (part) */
	  /* Either this current one again due to multiplicity */
	  /* or the next one */

	  if ((long)RDB[next + PARTICLE_MULTIPLICITY] > 0)
	    {
	      /* Subtract value */

	      WDB[next + PARTICLE_MULTIPLICITY] = 
		RDB[next + PARTICLE_MULTIPLICITY] - 1.0;

	      /* Duplicate */

	      part = DuplicateParticle(next, id);
	      WDB[ptr + PARTICLE_MULTIPLICITY] = 0.0;
	    }
	  else
	    {
	      /* No more copies of the current particle in que    */
	      /* Get previous particle in list and remove current */
	      /* from que */
	      
	      part = next;
	
	      /* Next particle will be previous particle in Que-list */

	      next = PrevItem(part);

	      /* Remove current particle from Que-list (no more duplicates) */

	      RemoveItem(part);
	    }
	}
          
      /***********************************************************************/
      
      /***** Start history ***************************************************/
 
      /* Check multiplicity */

      if ((long)RDB[part + PARTICLE_MULTIPLICITY] > 0)
	Die(FUNCTION_NAME, "Multiplicity");

      /* Check MPI index (this is for reproducible MPI mode) */

      if ((long)RDB[part + PARTICLE_MPI_ID] != mpiid)
	{
	  /* Check reproducibility */

	  if ((long)RDB[DATA_OPTI_MPI_REPRODUCIBILITY] == NO)
	    Die(FUNCTION_NAME, "Error in mpi mode");

	  /* Put particle back in stack */
	      
	  ToStack(part, id);

	  /* Return (pitäiskö tässä olla continue?) */
	  
	  return;
	}

      /* Check generation cut-off */

      if ((long)RDB[part + PARTICLE_GEN_IDX] >= (long)RDB[DATA_GEN_CUT])
	{
	  /* Score cut-off (toi indeksi on määritetty vain neutroneille) */
	  
	  ptr = (long)RDB[RES_TOT_NEUTRON_CUTRATE];
	  CheckPointer(FUNCTION_NAME, "(ptr)", DATA_ARRAY, ptr);
	  AddBuf1D(1.0, wgt, ptr, id, 0);	      

	  /* Put particle back in stack */
	      
	  ToStack(part, id);

	  /* Break loop */

	  continue;
	}

      /* Check generation index */

      if ((long)RDB[part + PARTICLE_GEN_IDX] > gmax)
	gmax = (long)RDB[part + PARTICLE_GEN_IDX];

      /* Add to population */

      if ((ptr = (long)RDB[RES_PROMPT_GEN_POP]) > VALID_PTR)
	AddBuf1D(1.0, 1.0, ptr, id, (long)RDB[part + PARTICLE_GEN_IDX]);

      /* Get particle type */

      type = (long)RDB[part + PARTICLE_TYPE];    

      /* Get spatial coordinates */

      x = RDB[part + PARTICLE_X];
      y = RDB[part + PARTICLE_Y];
      z = RDB[part + PARTICLE_Z];

      /* Set initial coordinates for surface based tallies */

      x0 = x;
      y0 = y;
      z0 = z;
      
      /* Get direction cosines */

      u = RDB[part + PARTICLE_U];
      v = RDB[part + PARTICLE_V];
      w = RDB[part + PARTICLE_W];
      
      /* Get energy, weight and time */

      E = RDB[part + PARTICLE_E];
      wgt = RDB[part + PARTICLE_WGT];
      t = RDB[part + PARTICLE_T];

      /* Score source rate for weight window current */

      if (quen == 0)
	ScoreWWDCurr(x, y, z, u, v, w, YES, id);

      /* Update que count */

      quen++;

      /* Remember initial time */

      t0 = t;

      /* Check with cut-off */

      if ((t0 < RDB[DATA_TIME_CUT_TMIN]) || (t0 >= RDB[DATA_TIME_CUT_TMAX]))
	Die(FUNCTION_NAME, "Error in time (%1.2E : %1.2E %1.2E)",
	    t0, RDB[DATA_TIME_CUT_TMIN], RDB[DATA_TIME_CUT_TMAX]);

      /* Apply weight window */

      if (WeightWindow(-1, part, x, y, z, u, v, w, E, &wgt, t, NO, id) ==
	  TRACK_END_WCUT)
	{
	  /* Cycle loop */

	  continue;
	}

      /* Find initial location */

      cell = WhereAmI(x, y, z, u, v, w, id);
      CheckPointer(FUNCTION_NAME, "(cell)", DATA_ARRAY, cell);

      /* Get material pointer */

      mat = (long)RDB[cell + CELL_PTR_MAT];
      mat = MatPtr(mat, id);

      /* Store starting point in history array */
      
      StoreHistoryPoint(part, mat, -1, x, y, z, u, v, w, E, t, wgt, -1.0, 
			TRACK_END_STRT);

      /***********************************************************************/

      /***** Tracking loop ***************************************************/

      /* Avoid compiler warning */

      lmax = -1;
      trk = -1;

      /* Get maximum number of loops */

      if (type == PARTICLE_TYPE_NEUTRON)
	lmax = (long)RDB[DATA_NEUTRON_MAX_TRACK_LOOP];
      else if (type == PARTICLE_TYPE_GAMMA)
	lmax = (long)RDB[DATA_PHOTON_MAX_TRACK_LOOP];
      else
	Die(FUNCTION_NAME, "Invalid particle type");
      
      /* Check value */

      CheckValue(FUNCTION_NAME, "lmax", "", lmax, 1, 100000000000);

      /* Main loop */

      for (loop = 0; loop < lmax; loop++)
	{
	  /* Check photon energy */

	  if (type == PARTICLE_TYPE_GAMMA)
	    if (E < RDB[DATA_PHOTON_EMIN])
	      Die(FUNCTION_NAME, "Photon energy below minimum");

	  /* Calculate speed */

	  spd = Speed(type, E);
	  CheckValue(FUNCTION_NAME, "spd", "", spd, ZERO, INFTY);

	  /* Get minimum cross section */

	  minxs = MinXS(type, spd, id);

	  /* Add to track counter */

	  ptr = (long)RDB[DATA_PTR_COLLISION_COUNT];
	  AddPrivateData(ptr, 1.0, id);

	  /* Get total cross section and majorant */

	  totxs = TotXS(mat, type, E, id);
	  majorant = DTMajorant(type, E, id);

	  /* Compare majorant to minimum */

	  if (majorant < minxs)
	    majorant = minxs;

	  /* Check cross sections */	      

	  CheckValue(FUNCTION_NAME, "majorant", "", majorant, ZERO, INFTY);
	  CheckValue(FUNCTION_NAME, "minxs", "", minxs, ZERO, INFTY);
	  CheckValue(FUNCTION_NAME, "totxs", "", totxs, 0.0, INFTY);

	  /* Avoid compiler warning */

	  trk = -1;
	  cell = -1;

	  /* Get tracking mode */
	  
	  mode = TrackMode(part, mat, E, totxs, majorant, type, id);

	  /* Move particle forward */

	  if (mode == TRACK_MODE_DT)
	    {
	      /* Use delta-tracking */
	      
	      trk = MoveDT(part, majorant, minxs, &cell, &xs, &x, &y, &z, &l,
			   &u, &v, &w, E, id);
	    }
	  else if (mode == TRACK_MODE_ST)
	    {
	      /* Use surface-tracking */
	      
	      trk = MoveST(part, totxs, minxs, &cell, &xs, &x, &y, &z, &l,
			   u, v, w, id);
	    }
	  else
	    Die(FUNCTION_NAME, "Invalid tracking mode");

	  /* Check distance */

	  CheckValue(FUNCTION_NAME, "l", "", l, ZERO, INFTY);

	  /* Weight window boundary */
	  
	  trk = StopAtWWBound(trk, &x, &y, &z, u, v, w, spd, &dt, &l, 
			      &cell, id);

	  /* Calculate change in time (suhtis?) */

	  dt = l/spd;

	  /* Do time cut-off */

	  trk = TimeCutoff(trk, part, &cell, &dt, &x, &y, &z, u, v, w, E, t, 
			   wgt, spd, mode, id);

	  /* Update time */

	  t = t + dt;

	  /* Check cell pointer */

	  if (cell < VALID_PTR)
	    Die(FUNCTION_NAME, "Particle lost");

	  /* Get material pointer */

	  mat = (long)RDB[cell + CELL_PTR_MAT];
	  mat = MatPtr(mat, id);

	  /* Check track type */

	  if (trk == TRACK_END_VIRT)
	    {
	      /***************************************************************/

	      /***** Virtual collision ***************************************/

	      /* Score collision */
	      
	      ptr = (long)RDB[RES_AVG_VIRT_COL];
	      CheckPointer(FUNCTION_NAME, "(ptr)", DATA_ARRAY, ptr);
	      AddBuf1D(1.0, 1.0, ptr, id, 2 - type);	      

	      /* Weight adjustment in alpha-eigenvalue mode */

	      Alpha(E, majorant, &wgt);

	      /* Score collision (NOTE: Cross section is set to -1 if the */
	      /* collision is not to be scored) */

	      if (xs > 0.0)
		{
		  /* Get density factor */
		  
		  g = DensityFactor(mat, x, y, z, t, id);
		  CheckValue(FUNCTION_NAME, "g", "", g, 0.0, 1.0);

		  /* Store point in history array */

		  StoreHistoryPoint(part, mat, -1, x, y, z, u, v, w, E, 
				    t, wgt, 1.0/xs, trk);

		  /* Score collision */

		  Score(mat, part, 1.0/xs, x, y, z, u, v, w, E, wgt, t, 
			spd, g, id);
		}

	      /**************************************************************/
	    }
	  else if (trk == TRACK_END_COLL)
	    {
	      /***************************************************************/

	      /***** Physical collision **************************************/

	      /* Score surface tallies */

	      ScoreSurf(part, &x0, &y0, &z0, x, y, z, u, v, w, E, wgt, t, id);

	      /* Score track and collision */
	      
	      ptr = (long)RDB[RES_AVG_TRACKS];
	      CheckPointer(FUNCTION_NAME, "(ptr)", DATA_ARRAY, ptr);
	      AddBuf1D(1.0, 1.0, ptr, id, 2 - type);	      

	      ptr = (long)RDB[RES_AVG_REAL_COL];
	      CheckPointer(FUNCTION_NAME, "(ptr)", DATA_ARRAY, ptr);
	      AddBuf1D(1.0, 1.0, ptr, id, 2 - type);	      

	      /* Weight adjustment in alpha-eigenvalue mode */

	      Alpha(E, majorant, &wgt);

	      /* Check material pointer */

	      if (mat < VALID_PTR)
		TrackingError(TRACK_ERR_NO_MATERIAL, -1, -1, -1, id);

	      CheckPointer(FUNCTION_NAME, "(mat)", DATA_ARRAY, mat);

	      /* Get density factor */
	      
	      g = DensityFactor(mat, x, y, z, t, id);
	      CheckValue(FUNCTION_NAME, "g", "", g, 0.0, 1.0);
	      	      
	      /* Score collision */

	      Score(mat, part, 1.0/xs, x, y, z, u, v, w, E, wgt, t, spd, 
		    g, id);

	      /* Additional rejection by density factor */

	      if (RandF(id) < g)
		{
		  /* Store point in history array */
	      
		  StoreHistoryPoint(part, mat, -1, x, y, z, u, v, w, E,
				    t, wgt, 1.0/xs, trk);

		  /* Sample collision */

		  trk = Collision(mat, part, x, y, z, &u, &v, &w, &E, 
				  &wgt, t, id);
		}
	      else
		{
		  /* Virtual collision, change track type */

		  trk = TRACK_END_VIRT;
		}

	      /* Store point in history array */
	      
	      StoreHistoryPoint(part, mat, -1, x, y, z, u, v, w, E,
				t, wgt, 1.0/xs, trk);
	      
	      /* Score efficiency of ifc collision rejection */
	      
	      if ((long)RDB[mat + MATERIAL_USE_IFC] == YES)
		{
		  ptr = (long)RDB[RES_IFC_COL_EFF];
		  CheckPointer(FUNCTION_NAME, "(ptr)", DATA_ARRAY, ptr);
		  AddBuf1D(g, 1.0, ptr, id, 2 - type);
		}
	      
	      /* Score total collision effiency */

	      if (trk != TRACK_END_VIRT)
		{
		  ptr = (long)RDB[RES_TOT_COL_EFF];
		  CheckPointer(FUNCTION_NAME, "(ptr)", DATA_ARRAY, ptr);
		  AddBuf1D(g, 1.0, ptr, id, 2 - type);
		}	      

	      /* Check termination or reset infinite loop counter */

	      if ((trk == TRACK_END_CAPT) || (trk == TRACK_END_FISS) ||
		  (trk == TRACK_END_ECUT) || (trk == TRACK_END_WCUT))
		break;
	      else if (trk == TRACK_END_SCAT)
		loop = 0;
	    
	      /***************************************************************/
	    }
	  else if (trk == TRACK_END_SURF)
	    {
	      /***************************************************************/

	      /***** Surface crossing ****************************************/

	      /* Check if outer boundary was crossed and score */ 
	      /* surface tallies */
   
	      if ((long)RDB[cell + CELL_TYPE] == CELL_TYPE_OUTSIDE)
		ScoreSurf(part, &x0, &y0, &z0, x, y, z, u, v, w, E, 
			  wgt, t, id);

	      /* Score surface crossing */
	      
	      ptr = (long)RDB[RES_AVG_SURF_CROSS];
	      CheckPointer(FUNCTION_NAME, "(ptr)", DATA_ARRAY, ptr);
	      AddBuf1D(1.0, 1.0, ptr, id, 2 - type);	      

	      /* Store first point in history array */

	      StoreHistoryPoint(part, mat, -1, x, y, z, u, v, w, E,
				t, wgt, -1.0, trk);
	      
	      /* Apply boundary conditions */

	      bc = BoundaryConditions(&cell, &x, &y, &z, &u, &v, &w, &wgt, id);

	      /* Check cell pointer */

	      if (cell < VALID_PTR)
		Die(FUNCTION_NAME, "Particle lost");

	      /* Check leakage and repeated */

	      if (bc < 0)
		{
		  /* Score track */
		  
		  ptr = (long)RDB[RES_AVG_TRACKS];
		  CheckPointer(FUNCTION_NAME, "(ptr)", DATA_ARRAY, ptr);
		  AddBuf1D(1.0, 1.0, ptr, id, 2 - type);	      

		  /* Score leakage */

		  Leak(part, x, y, z, u, v, w, E, wgt, id);
		  
		  /* Set track type to leak */

		  trk = TRACK_END_LEAK;

		  /* Break loop */

		  break;
		}
	      else if (bc == YES)
		{	     
		  /* Adjust previous position (Tässä ja scoresurf.c:ssä */
		  /* käytetään kaksinkertaista ekstrapolaatiopituutta.) */
		  
		  x0 = x - 2.0*EXTRAP_L*u;
		  y0 = y - 2.0*EXTRAP_L*v;
		  z0 = z - 2.0*EXTRAP_L*w;

		  /* Score surface tallies */

		  ScoreSurf(part, &x0, &y0, &z0, x, y, z, u, v, w, E, wgt, t, 
			    id);

		  /* Get material pointer */

		  mat = (long)RDB[cell + CELL_PTR_MAT];
		  mat = MatPtr(mat, id);

		  /* Store second point in history array */

		  StoreHistoryPoint(part, mat, -1, x, y, z, u, v, w, E, 
				    t, wgt, -1.0, TRACK_END_BC);
 		}

	      /***************************************************************/
	    }
	  else if (trk == TRACK_END_TCUT)
	    {
	      /***************************************************************/

	      /***** Time cut-off ********************************************/

	      /* Score surface tallies */

	      ScoreSurf(part, &x0, &y0, &z0, x, y, z, u, v, w, E, wgt, t, id);

	      /* Score track */
	      
	      ptr = (long)RDB[RES_AVG_TRACKS];
	      CheckPointer(FUNCTION_NAME, "(ptr)", DATA_ARRAY, ptr);
	      AddBuf1D(1.0, 1.0, ptr, id, 2 - type);	      

	      /* Break loop */
	      
	      break;

	      /***************************************************************/
	    }
	  else if (trk == TRACK_END_WWIN)
	    {
	      /***************************************************************/

	      /***** Weight window boundary **********************************/

	      /* Store history point */
	      
	      StoreHistoryPoint(part, mat, -1, x, y, z, u, v, w, E, t, wgt, 
				-1.0, trk);

	      /* Apply weight window */

	      trk =  WeightWindow(trk, part, x, y, z, u, v, w, E, &wgt,
				  t, YES, id);

	      /* Check weight cut-off */

	      if (trk == TRACK_END_WCUT)
		break;

	      /***************************************************************/
	    }
	  else 
	    Die(FUNCTION_NAME, "Invalid track type %ld", trk);

	  /* Check track type */

	  if (trk == TRACK_END_VIRT)
	    {
	      /* Score total collision efficency */
	      
	      ptr = (long)RDB[RES_TOT_COL_EFF];
	      CheckPointer(FUNCTION_NAME, "(ptr)", DATA_ARRAY, ptr);
	      AddBuf1D(1.0, 1.0, ptr, id, 4 - type);	      
	    }
	  else if ((trk != TRACK_END_SCAT) && (trk != TRACK_END_SURF) &&
		   (trk != TRACK_END_WWIN))
	    Die(FUNCTION_NAME, "Loop not terminated by track type %ld", trk);
	}

      /***********************************************************************/
     
      /***** History terminated **********************************************/

      /* Get mean number of collisions */

      n = (long)RDB[part + PARTICLE_COL_IDX];

      /* Score total and collisions to fission */
      
      ptr = (long)RDB[RES_ANA_MEAN_NCOL];
      CheckPointer(FUNCTION_NAME, "(ptr)", DATA_ARRAY, ptr);

      AddBuf1D(n, 1.0, ptr, id, 0);

      if (trk == TRACK_END_FISS)
	AddBuf1D(n, 1.0, ptr, id, 1);

      /* Score number of loops */

      ptr = (long)RDB[RES_AVG_TRACK_LOOPS];
      CheckPointer(FUNCTION_NAME, "(ptr)", DATA_ARRAY, ptr);
      AddBuf1D((double)(loop + 1), 1.0, ptr, id, 2 - type);

      /* Check for infinite loop */

      if (loop == lmax)
	{
	  /* Check type and fail flag */

	  if (((type == PARTICLE_TYPE_NEUTRON) && 
	       ((long)RDB[DATA_NEUTRON_MAX_TRACK_LOOP_ERR] == YES)) ||
	      ((type == PARTICLE_TYPE_GAMMA) && 
	       ((long)RDB[DATA_PHOTON_MAX_TRACK_LOOP_ERR] == YES)))
	    TrackingError(TRACK_ERR_INF_LOOP, E, mat, type, id);

	  /* Score error */

	  AddBuf1D(1.0, 1.0, ptr, id, 4 - type);

	  /* Put particle back in stack */

	  ToStack(part, id);

	  /* Cycle outer loop */

	  continue;
	}
    
      /* Score time constants (kato toi) */

      ScoreTimeConstants(t, wgt, part, trk, id);
      
      /* Store point in history array */

      StoreHistoryPoint(part, mat, -1, x, y, z, u, v, w, E, t, wgt, -1.0, trk);

      /***********************************************************************/

      /***** Clear events ****************************************************/

      /* Check if events are recorded (no flags set) */

      if ((long)RDB[DATA_EVENT_RECORD_FLAGS] == 0)
	continue;

      /* Check if track plotter mode */

      if ((long)RDB[DATA_STOP_AFTER_PLOT] == STOP_AFTER_PLOT_TRACKS)
	continue;

      /* Put OpenMP critical barrier (vissiin tarvitaan) */
  
#ifdef OPEN_MP
#pragma omp critical
#endif
      {
	/* Loop over events and update counters */
 
	ptr = (long)RDB[part + PARTICLE_PTR_EVENTS];
	while (ptr > VALID_PTR)
	  {
	    /* Update count */
	
	    WDB[ptr + EVENT_HIS_COUNT]--;

	    /* Check */
	
	    if ((long)RDB[ptr + EVENT_HIS_COUNT] == 0)
	      {
		/* Remove item (ei pitäisi olla muita jotka operoi tähän) */
		
		loc0 = ptr;
		
		/* Pointer to next */
		
		ptr = NextItem(loc0);
		
		/* Back to bank */
		
		EventToBank(loc0);
	      }
	    else if ((long)RDB[ptr + EVENT_HIS_COUNT] > 0)
	      {
		/* Pointer to next */
		
		ptr = NextItem(ptr);
	      }
	    else
	      Die(FUNCTION_NAME, "WTF?");
	  }
      }
      
      /***********************************************************************/
    }

  /* Add to mean prompt chain length */

  if (gmax > 0)
    {
      ptr = (long)RDB[RES_PROMPT_CHAIN_LENGTH];
      CheckPointer(FUNCTION_NAME, "(ptr)", DATA_ARRAY, ptr);
      AddBuf1D(gmax, 1.0, ptr, id, 0);
    }

  /* Check maximum generation */

  if (gmax > (long)RDB[DATA_MAX_PROMPT_CHAIN_LENGTH])
    gmax = (long)RDB[DATA_MAX_PROMPT_CHAIN_LENGTH];

  /* Add to prompt generation fractions and time */

  if ((ptr = (long)RDB[RES_PROMPT_GEN_CUMU]) > VALID_PTR)
    for (n = 0; n < gmax + 1; n++)
      AddBuf1D(1.0, 1.0, ptr, id, n);

  if ((ptr = (long)RDB[RES_PROMPT_GEN_TIMES]) > VALID_PTR)
    AddBuf1D(t, 1.0, ptr, id, gmax);
}

/*****************************************************************************/
#ifdef __cplusplus 
} 
#endif 
