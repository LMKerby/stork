/*****************************************************************************/
/*                                                                           */
/* serpent 2 (beta-version) : rroutput.c                                     */
/*                                                                           */
/* Created:       2011/01/20 (JLe)                                           */
/* Last modified: 2015/06/04 (JLe)                                           */
/* Version:       2.1.24                                                     */
/*                                                                           */
/* Description: Writes analog reaction rate estimates into file              */
/*                                                                           */
/* Comments: - Tää printtaa nyt typerästi myös decay-nuklidit                */
/*                                                                           */
/*****************************************************************************/

#include "header.h"
#include "locations.h"

#define FUNCTION_NAME "RROutput:"

/*****************************************************************************/

void RROutput()
{
  long nuc, rea, ptr, i, mat;
  char tmpstr[MAX_STR];
  FILE *fp;

  /***************************************************************************/

  /**** Open file etc. *******************************************************/

  /* Check mode */

  if (((long)RDB[DATA_ANA_RR_NCALC] == ARR_MODE_NONE) &&
      ((long)RDB[DATA_ANA_RR_PCALC] == ARR_MODE_NONE))
    return;

  /* Check mpi task */

  if (mpiid > 0)
    return;

  /* Open file for writing */
  
  sprintf(tmpstr, "%s_arr%ld.m", GetText(DATA_PTR_INPUT_FNAME),
	  (long)RDB[DATA_BURN_STEP]);
  
  if ((fp = fopen(tmpstr, "w")) == NULL)
    Warn(FUNCTION_NAME, "Unable to open file for writing");

  /***************************************************************************/

  /***** Nuclide-wise reaction rates *****************************************/

  fprintf(fp, "\n%% ----- Analog nuclide-wise reaction rate estimates\n\n");

  /* Reset index */

  i = 1;

  /* Loop over nuclides */
  
  nuc = (long)RDB[DATA_PTR_NUC0];
  while (nuc > VALID_PTR)
    {
      /* Check pointer to reaction sampling list */

      if ((long)RDB[nuc + NUCLIDE_PTR_SAMPLE_REA_LIST] < VALID_PTR)
	{
	  /* Reset temporary index */
	  
	  WDB[nuc + NUCLIDE_TMP_IDX] = -1.0;
	}
      else
	{
	  /* Set temporary index */
	  
	  WDB[nuc + NUCLIDE_TMP_IDX] = (double)(i++);
	  
	  /* Print name */

	  fprintf(fp, "nuc(%4ld, :) = '%11s';\n", (long)RDB[nuc + NUCLIDE_IDX],
		  GetText(nuc + NUCLIDE_PTR_NAME));
	}

      /* Next nuclide */
      
      nuc = NextItem(nuc);
    }
  
  /* Print reaction rates */

  fprintf(fp, "\nrr = [\n");

  /* Loop over nuclides */
  
  nuc = (long)RDB[DATA_PTR_NUC0];
  while (nuc > VALID_PTR)
    {
      /* Get temporary index */

      if ((i = (long)RDB[nuc + NUCLIDE_TMP_IDX]) > 0)
	{
	  /* Loop over reactions */

	  rea = (long)RDB[nuc + NUCLIDE_PTR_REA];
	  while (rea > VALID_PTR)
	    {
	      /* Check tally pointer and density */
	      
	      if ((ptr = (long)RDB[rea + REACTION_PTR_ANA_RATE]) > VALID_PTR)
		{
		  /* Print nuclide index, ZAI and reaction mt */
		  
		  fprintf(fp, "%4ld %7ld %4ld ", i, 
			  (long)RDB[nuc + NUCLIDE_ZAI],
			  (long)RDB[rea + REACTION_MT]);
		  
		  /* Print minimum and maximum energy */
		  
		  fprintf(fp, "%12.5E %12.5E ", RDB[rea + REACTION_EMIN],
			  RDB[rea + REACTION_EMAX]);
		  
		  /* Print reaction rate */
		  
		  fprintf(fp, "%12.5E %7.5f ", Mean(ptr, 0), RelErr(ptr, 0));
		  
		  /* Print comment */
		  
		  fprintf(fp, " %% %10s mt %ld", 
			  GetText(nuc + NUCLIDE_PTR_NAME),
			  (long)RDB[rea + REACTION_MT]);
		  
		  /* Newline */
		  
		  fprintf(fp, "\n");
		}

	      /* Next reaction */
	      
	      rea = NextItem(rea);
	    }
	}

      /* Next nuclide */

      nuc = NextItem(nuc);
    }

  fprintf(fp, "];\n\n");

  /***************************************************************************/

  /***** Print material-wise decay source rates ******************************/

  /* Check if decay source is used */

  if ((long)RDB[DATA_USE_DECAY_SRC] == YES)
    {
      /* Loop over materials */
      
      mat = (long)RDB[DATA_PTR_M0];
      while (mat > VALID_PTR)
	{
	  /* Check source rate */
	  
	  if ((long)RDB[mat + MATERIAL_PHOTON_SRC_RATE] > 0.0)
	    {
	      /* Print sampled and calculated source rate */
	      
	      ptr = (long)RDB[mat + MATERIAL_SAMPLED_PHOTON_SRC];
	      CheckPointer(FUNCTION_NAME, "(ptr)", DATA_ARRAY, ptr);
	      fprintf(fp, "gsrc_%s = [ %12.5E %12.5E %7.5f %8.5f ];\n",
		      GetText(mat + MATERIAL_PTR_NAME),
		      RDB[mat + MATERIAL_PHOTON_SRC_RATE],
		      Mean(ptr, 0), RelErr(ptr, 0),
		      Mean(ptr, 0)/RDB[mat + MATERIAL_PHOTON_SRC_RATE] - 1.0);
	    }
	  
	  /* Next material */
	  
	  mat = NextItem(mat);
	}
    }

  /***************************************************************************/

  /* Close file */

  fclose(fp);
}

/*****************************************************************************/
