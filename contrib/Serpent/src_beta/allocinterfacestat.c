/*****************************************************************************/
/*                                                                           */
/* serpent 2 (beta-version) : allocinterfacestat.c                           */
/*                                                                           */
/* Created:       2012/09/11 (JLe)                                           */
/* Last modified: 2016/01/31 (VVa)                                           */
/* Version:       2.1.25                                                     */
/*                                                                           */
/* Description: Sets up stats and associated structures for multi-physics    */
/*              interfaces.                                                  */
/*                                                                           */
/* Comments: - Separeted from processinterface.c, because routine needs      */
/*             material fissile flags to be set.                             */
/*                                                                           */
/*           - Polttoaineinterfacen aksiaalijako lisätty 3.4.2013            */
/*           - Atsimutaali ja aika-askeljako lisätty poltoaineinterfacelle   */
/*             2.1.19                                                        */
/*                                                                           */
/*****************************************************************************/

#include "header.h"
#include "locations.h"

#define FUNCTION_NAME "AllocInterfaceStat:"

/*****************************************************************************/

void AllocInterfaceStat()
{
  long loc0, loc1, loc2, ptr, nz, nr, na, nt, n, tme, nnt;
  double zmin, zmax, z, rmin, rmax, r2, amin, amax;

  /* Loop over interfaces */

  loc0 = (long)RDB[DATA_PTR_IFC0];
  while (loc0 > VALID_PTR)
    {
      /* Check output flag */

      if ((long)RDB[loc0 + IFC_CALC_OUTPUT] == YES)
	{
	  /* Check type */

	  if (((long)RDB[loc0 + IFC_TYPE] == IFC_TYPE_FUEP) || 
	      ((long)RDB[loc0 + IFC_TYPE] == IFC_TYPE_FPIP))
	    {
	      /***************************************************************/

	      /***** Interface for fuel performance codes ********************/

	      /* Loop over pins */

	      loc1 = (long)RDB[loc0 + IFC_PTR_FUEP];
	      while (loc1 > VALID_PTR)
		{
		  /* Get number of radial and axial zones for power */

		  loc2 = (long)RDB[loc1 + IFC_FUEP_OUT_PTR_LIM];

		  /* Check number of different zones */

                  if ((nr = (long)RDB[loc2 + FUEP_NR]) < 1)
                    Die(FUNCTION_NAME, "Error in number of radial zones %ld",
			(long)RDB[loc2 + FUEP_NR]);
                  if ((nz = (long)RDB[loc2 + FUEP_NZ]) < 1)
                    Die(FUNCTION_NAME, "Error in number of axial zones %ld",
			(long)RDB[loc2 + FUEP_NZ]);
                  if ((na = (long)RDB[loc2 + FUEP_NA]) < 1)
                    Die(FUNCTION_NAME, "Error in number of angular zones %ld",
			(long)RDB[loc2 + FUEP_NA]);
		  if ((nt = (long)RDB[loc2 + FUEP_NT]) < 1)
		    Die(FUNCTION_NAME, "Error in number of time bins %ld",
			(long)RDB[loc2 + FUEP_NT]);
		  else if (nt == 1)
		    {
		      /* Time bins not ready when interface was read */
		      /* interface is created at ReadInput, but time */
		      /* structures are handled in ProcessTimeBins   */

		      nt = (long)RDB[DATA_DYN_NB];

		      WDB[loc2 + FUEP_NT] = (double)nt;
		    }

		  /* Allocate memory for stats */

		  /* Power relaxed between iterations */
		  
		  ptr = ReallocMem(DATA_ARRAY,nz*na*nr*nt);
                  WDB[loc1 + IFC_FUEP_PTR_POWER_REL] = (double)ptr;      

		  /* Gradient of power between subsequent iterations */

		  ptr = ReallocMem(DATA_ARRAY,nz*na*nr*nt);
                  WDB[loc1 + IFC_FUEP_PTR_POWER_GRD] = (double)ptr;      

		  /* Power on a single iteration */

		  ptr = NewStat("INTERFACE_POWER", 4, nz, na, nr, nt);
                  WDB[loc1 + IFC_FUEP_PTR_POWER] = (double)ptr;      

		  /* Put axial limits if not given */

		  if ((long)RDB[loc1 + IFC_FUEP_OUT_PTR_Z] < VALID_PTR)
		    {
		      /* Allocate memory for axial zones */

		      ptr = ReallocMem(DATA_ARRAY, nz + 1);
		      WDB[loc1 + IFC_FUEP_OUT_PTR_Z] = (double)ptr;

		      /* Get limits */

		      zmin = RDB[loc2 + FUEP_ZMIN];
		      zmax = RDB[loc2 + FUEP_ZMAX];
		      
		      /* Swap boundaries if necessary */

		      if (zmin > zmax)
			{
			  z = zmin;
			  zmin = zmax;
			  zmax = z;
			}

		      /* Put values */

		      for (n = 0; n < nz + 1; n++)
			{
			  z = ((double)n)/((double)(nz))*(zmax - zmin) + zmin;
			  WDB[ptr++] = z;
			}		      
		    }

		  /* Put angular limits if not given */

		  if ((long)RDB[loc1 + IFC_FUEP_OUT_PTR_PHI] < VALID_PTR)
		    {
		      /* Allocate memory for angular zones */

		      ptr = ReallocMem(DATA_ARRAY, na + 1);
		      WDB[loc1 + IFC_FUEP_OUT_PTR_PHI] = (double)ptr;

		      /* Get limits */

		      amin = RDB[loc2 + FUEP_AMIN];
		      amax = RDB[loc2 + FUEP_AMAX];
		      
		      /* Swap boundaries if necessary */

		      if (amin > amax)
			{
			  r2 = amin;
			  amin = amax;
			  amax = r2;
			}

		      /* Put values */

		      for (n = 0; n < na + 1; n++)
			{
			  WDB[ptr++] = ((amax-amin)/((double)na)*(double)n + amin)*2*PI/360.0;
			}
		    }

		  /* Put radial limits if not given */

		  if ((long)RDB[loc1 + IFC_FUEP_OUT_PTR_R2] < VALID_PTR)
		    {
		      /* Allocate memory for radial zones */

		      ptr = ReallocMem(DATA_ARRAY, nr + 1);
		      WDB[loc1 + IFC_FUEP_OUT_PTR_R2] = (double)ptr;

		      /* Get limits */

		      rmin = RDB[loc2 + FUEP_RMIN];
		      rmax = RDB[loc2 + FUEP_RMAX];
		      
		      /* Swap boundaries if necessary */

		      if (rmin > rmax)
			{
			  r2 = rmin;
			  rmin = rmax;
			  rmax = r2;
			}

		      /* Put values */

		      for (n = 0; n < nr + 1; n++)
			{
			  r2 = (double)n/(double)nr*
			    (rmax*rmax - rmin*rmin) + rmin*rmin;
			  WDB[ptr++] = r2;
			}
		    }

		  /* Put time limits if not given */

		  if((long)RDB[loc1 + IFC_FUEP_OUT_PTR_TB] < VALID_PTR)
		    {
		      /* Allocate memory for time bins */

		      ptr = ReallocMem(DATA_ARRAY, nt + 1);
		      
		      WDB[loc1 + IFC_FUEP_OUT_PTR_TB] = ptr;

		      if(((long)RDB[DATA_SIMULATION_MODE] == SIMULATION_MODE_SRC) ||
			 ((long)RDB[DATA_SIMULATION_MODE] == SIMULATION_MODE_DYN))
			{

			  /*
			  tmin = RDB[loc2 + FUEP_TMIN];
			  tmax = RDB[loc2 + FUEP_TMAX];
			  */

			  nnt = (long)(nt/RDB[DATA_DYN_NB]);

			  tme = (long)RDB[DATA_DYN_PTR_TIME_BINS];	     
		      
			  tme = (long)RDB[tme + TME_PTR_BINS];

			  for (n= 0; n < nt + 1; n++)
			    WDB[ptr++] = RDB[tme + (long)(n/nnt)] + n%nnt*(RDB[tme + (long)(n/nnt) + 1] - RDB[tme + (long)(n/nnt)])/nnt;

			}
		      else
			{
			  WDB[ptr] = -INFTY;
			  WDB[ptr+1] = INFTY;
			  
			}
		    }

		  /* Get number of radial and axial zones for flux */

		  loc2 = (long)RDB[loc1 + IFC_FUEP_OUT_PTR_FLIM];

                  if ((nr = (long)RDB[loc2 + FUEP_NR]) < 1)
                    Die(FUNCTION_NAME, "Error in number of radial zones (flux)");
                  if ((nz = (long)RDB[loc2 + FUEP_NZ]) < 1)
                    Die(FUNCTION_NAME, "Error in number of axial zones (flux)");
                  if ((na = (long)RDB[loc2 + FUEP_NA]) < 1)
                    Die(FUNCTION_NAME, "Error in number of angular zones (flux)");
                  if ((nt = (long)RDB[loc2 + FUEP_NT]) < 1)
                    Die(FUNCTION_NAME, "Error in number of temporal zones (flux)");
		  else if (nt == 1)
		    {
		      /* Time bins not ready when interface was read */
		      /* interface is created at ReadInput, but time */
		      /* structures are handled in ProcessTimeBins   */

		      nt = (long)RDB[DATA_DYN_NB];

		      WDB[loc2 + FUEP_NT] = (double)nt;
		    }

		  ptr = NewStat("INTERFACE_FLUX", 4, nz, na, nr, nt);
		  WDB[loc1 + IFC_FUEP_PTR_FLUX] = (double)ptr;
		  
		  /* Put axial limits if not given */

		  if ((long)RDB[loc1 + IFC_FUEP_OUT_PTR_FZ] < VALID_PTR)
		    {
		      /* Allocate memory for axial zones */

		      ptr = ReallocMem(DATA_ARRAY, nz + 1);
		      WDB[loc1 + IFC_FUEP_OUT_PTR_FZ] = (double)ptr;

		      /* Get limits */

		      zmin = RDB[loc2 + FUEP_ZMIN];
		      zmax = RDB[loc2 + FUEP_ZMAX];
		      
		      /* Swap boundaries if necessary */

		      if (zmin > zmax)
			{
			  z = zmin;
			  zmin = zmax;
			  zmax = z;
			}

		      /* Put values */

		      for (n = 0; n < nz + 1; n++)
			{
			  z = ((double)n)/((double)(nz))*(zmax - zmin) + zmin;
			  WDB[ptr++] = z;
			}		      
		    }

		  /* Put radial limits if not given */

		  if ((long)RDB[loc1 + IFC_FUEP_OUT_PTR_FR2] < VALID_PTR)
		    {
		      /* Allocate memory for radial zones */

		      ptr = ReallocMem(DATA_ARRAY, nr + 1);
		      WDB[loc1 + IFC_FUEP_OUT_PTR_FR2] = (double)ptr;

		      /* Get limits */

		      rmin = RDB[loc2 + FUEP_RMIN];
		      rmax = RDB[loc2 + FUEP_RMAX];
		      
		      /* Swap boundaries if necessary */

		      if (rmin > rmax)
			{
			  r2 = rmin;
			  rmin = rmax;
			  rmax = r2;
			}

		      /* Put values */

		      for (n = 0; n < nr + 1; n++)
			{
			  r2 = (double)n/(double)nr*
			    (rmax*rmax - rmin*rmin) + rmin*rmin;
			  WDB[ptr++] = r2;
			}
		    }

		  /* Put angular limits if not given */

		  if ((long)RDB[loc1 + IFC_FUEP_OUT_PTR_FPHI] < VALID_PTR)
		    {
		      /* Allocate memory for angular zones */

		      ptr = ReallocMem(DATA_ARRAY, na + 1);
		      WDB[loc1 + IFC_FUEP_OUT_PTR_FPHI] = (double)ptr;

		      /* Get limits */

		      amin = RDB[loc2 + FUEP_AMIN];
		      amax = RDB[loc2 + FUEP_AMAX];
		      
		      /* Swap boundaries if necessary */

		      if (amin > amax)
			{
			  r2 = amin;
			  amin = amax;
			  amax = r2;
			}

		      /* Put values */

		      for (n = 0; n < na + 1; n++)
			{
			  WDB[ptr++] = ((amax-amin)/((double)na)*(double)n + amin)*2*PI/360.0;
			}
		    }

		  /* Put time limits if not given */

		  if((long)RDB[loc1 + IFC_FUEP_OUT_PTR_FTB] < VALID_PTR)
		    {
		      /* Allocate memory for time bins */

		      ptr = ReallocMem(DATA_ARRAY, nt + 1);
		      
		      WDB[loc1 + IFC_FUEP_OUT_PTR_FTB] = ptr;

		      if (((long)RDB[DATA_SIMULATION_MODE] == SIMULATION_MODE_SRC) ||
			  ((long)RDB[DATA_SIMULATION_MODE] == SIMULATION_MODE_DYN))
			{
			  /*
			  tmin = RDB[loc2 + FUEP_TMIN];
			  tmax = RDB[loc2 + FUEP_TMAX];
			  */

			  nnt = (long)(nt/RDB[DATA_DYN_NB]);

			  tme = (long)RDB[DATA_DYN_PTR_TIME_BINS];	     
		      
			  tme = (long)RDB[tme + TME_PTR_BINS];

			  for (n= 0; n < nt + 1; n++)
			    WDB[ptr++] = RDB[tme + (long)(n/nnt)] + 
			      n%nnt*(RDB[tme + (long)(n/nnt) + 1] - 
				     RDB[tme + (long)(n/nnt)])/nnt;

			}
		      else
			{
			  WDB[ptr] = -INFTY;
			  WDB[ptr+1] = INFTY;
			  
			}
		    }

		  /* Next pin */

		  loc1 = NextItem(loc1);
		}

	      /***************************************************************/
	    }
	  else if ((long)RDB[loc0 + IFC_TYPE] != IFC_TYPE_TET_MESH)
	    {
	      /***************************************************************/

	      /***** Non-tet mesh interfaces *********************************/
	      
	      /* Find interface regions for this interface */

	      FindInterfaceRegions(loc0, -1, -1, 0, 0.0, 0.0, 0.0, 0);

	      /* Sort output list by position */
	      
	      if ((loc1 = (long)RDB[loc0 + IFC_PTR_OUT]) > VALID_PTR)
		{
		  SortList(loc1, IFC_OUT_Y0, SORT_MODE_ASCEND);
		  SortList(loc1, IFC_OUT_X0, SORT_MODE_ASCEND);
		}

	      /* Reset number of regions */

	      n = 0;

	      /* Check pointer to score list */

	      if ((loc1 = (long)RDB[loc0 + IFC_PTR_SCORE]) > VALID_PTR)
		{
		  /* Sort list by region index */

		  SortList(loc1, IFC_SCORE_REG_IDX, SORT_MODE_ASCEND);

		  /* Close */
		  
		  CloseList(loc1);

		  /* Loop over regions and set stat indexes */
		  
		  n = 0;
		  while (loc1 > VALID_PTR)
		    {
		      /* Put index */

		      WDB[loc1 + IFC_SCORE_STAT_IDX] = (double)(n++);

		      /* Next */

		      loc1 = NextItem(loc1);
		    }
		}

	      /* Put total number of score regions */

	      WDB[loc0 + IFC_STAT_NREG] = (double)n;

	      /* Allocate memory for results */

	      if (n > 0)
		{
		  /* Get number of axial zones (aikaisemmassa versiossa tän */
		  /* sallittiin olla nolla?) */

		  if ((nz = (long)RDB[loc0 + IFC_NZ]) < 1)
		    Die(FUNCTION_NAME, "Error in number of axial zones");
		  
		  /* Get number of radial zones */
		  
		  if ((nr = (long)RDB[loc0 + IFC_NR]) < 1)
		    Die(FUNCTION_NAME, "Error in number of radial zones");
		  
		  /* Use three-dimensional array */
		  
		  ptr = NewStat("INTERFACE_POWER", 3, n, nz, nr);
		  WDB[loc0 + IFC_PTR_STAT] = (double)ptr;      

		  /* Alloc memory for relaxed power */

		  ptr = ReallocMem(DATA_ARRAY,n*nz*nr);
                  WDB[loc0 + IFC_PTR_STAT_REL] = (double)ptr;      

		  /* Gradient of power between subsequent iterations */

		  ptr = ReallocMem(DATA_ARRAY, n*nz*nr);
		  WDB[loc0 + IFC_PTR_STAT_GRD] = (double)ptr;      

		}
	      else
		{
		  /* No regions, reset output flag */
		  
		  WDB[loc0 + IFC_CALC_OUTPUT] = (double)NO;
		}
	      
	      /***************************************************************/
	    }
	  else if ((long)RDB[loc0 + IFC_TYPE] == IFC_TYPE_TET_MESH)
	    {
	      /***************************************************************/

	      /***** Tetrahedral mesh based interface ************************/

	      /* Reset maximum number of bins */

	      n = -1;

	      /* Loop over tet cells */

	      loc1 = (long)RDB[loc0 + IFC_PTR_TET_MSH];
	      while (loc1 > VALID_PTR)
		{
		  /* Compare index to maximum */

		  if ((long)RDB[loc1 + IFC_TET_MSH_STAT_IDX] > n)
		    n = (long)RDB[loc1 + IFC_TET_MSH_STAT_IDX];

		  /* Next */

		  loc1 = NextItem(loc1);
		}
	      
	      /* Put total number of score regions */
	      
	      WDB[loc0 + IFC_STAT_NREG] = (double)(n + 1);

	      /* Allocate memory for results and volumes */
		  
	      if (n > -1)
		{
		  /* Use one-dimensional array */

		  ptr = NewStat("INTERFACE_POWER", 1, n + 1);
		  WDB[loc0 + IFC_PTR_STAT] = (double)ptr;      

		  if(RDB[DATA_RUN_CC] == YES)
		    {

		      /* Separate array for relaxed power */

		      ptr = ReallocMem(DATA_ARRAY, n + 1);
		      WDB[loc0 + IFC_PTR_STAT_REL] = (double)ptr;      

		      /* Gradient of power between subsequent iterations */

		      ptr = ReallocMem(DATA_ARRAY, n + 1);
		      WDB[loc0 + IFC_PTR_STAT_GRD] = (double)ptr;      

		    }

		  /* Allocate memory for volumes */

		  ptr = ReallocMem(DATA_ARRAY, n + 1);
		  WDB[loc0 + IFC_PTR_STAT_VOL] = (double)ptr;

		  /* Calculate volumes */
		  
		  loc1 = (long)RDB[loc0 + IFC_PTR_TET_MSH];
		  while (loc1 > VALID_PTR)
		    {
		      /* Get index */

		      n = (long)RDB[loc1 + IFC_TET_MSH_STAT_IDX];

		      /* Add volume */

		      WDB[ptr + n] = RDB[ptr + n] + TetraVol(loc0, loc1);

		      /* Next */
		      
		      loc1 = NextItem(loc1);
		    }
		}
	      else
		{
		  /* No regions, reset output flag */
		  
		  WDB[loc0 + IFC_CALC_OUTPUT] = (double)NO;
		}

	      /***************************************************************/
	    }
	  else
	    Die(FUNCTION_NAME, "Invalid interface type");
	}

      /* Next interface */

      loc0 = NextItem(loc0);
    }
}

/*****************************************************************************/
