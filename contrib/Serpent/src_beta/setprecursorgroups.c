/*****************************************************************************/
/*                                                                           */
/* serpent 2 (beta-version) : setprecursorgroups.c                           */
/*                                                                           */
/* Created:       2011/08/31 (JLe)                                           */
/* Last modified: 2015/03/05 (JLe)                                           */
/* Version:       2.1.23                                                     */
/*                                                                           */
/* Description: Sets global precursor group structure                        */
/*                                                                           */
/* Comments:                                                                 */
/*                                                                           */
/*****************************************************************************/

#include "header.h"
#include "locations.h"

#define FUNCTION_NAME "SetPrecursorGroups:"

/*****************************************************************************/

void SetPrecursorGroups()
{
  long n, nuc, ace, ptr, ng, ZAI;

  for (n = 0; n < 5; n++)
    {
      /* Try common actinides first */

      if (n == 0)
	ZAI = 922350;
      else if (n == 1)
	ZAI = 942390;
      else if (n == 2) /* 1) LMK */
	ZAI = 922330;
      else if (n == 3) /* 1) LMK */
	ZAI = 922380;
      else
	ZAI = -1;
      
      /* Loop over nuclides */
      
      nuc = (long)RDB[DATA_PTR_NUC0];
      while (nuc > VALID_PTR)
	{
	  /* Check initial composition and source flags */
	  
	  if (((long)RDB[nuc + NUCLIDE_TYPE_FLAGS] & NUCLIDE_FLAG_INITIAL) ||
	      ((long)RDB[nuc + NUCLIDE_TYPE_FLAGS] & NUCLIDE_FLAG_SRC))
	    {
	      /* Pointer to ACE data */
	      
	      if ((ace = (long)RDB[nuc + NUCLIDE_PTR_ACE]) > VALID_PTR)
		{
		  /* Pointer to NXS array */
		  
		  ptr = (long)ACE[ace + ACE_PTR_NXS];
		  CheckPointer(FUNCTION_NAME, "(ptr)", ACE_ARRAY, ptr);
		  
		  /* Get number of groups */
		  
		  ng = (long)ACE[ptr + 7];
		  
/*                  fprintf (err, "Precursor groups!!!! \n n = %d, nuc = %d, ng = %d \n", n, nuc, ng); /* LMK */
 
		  /* Check number */
		  
		  if (((ng == 6) || (ng == 8)) && 
		      ((ZAI == -1) || (ZAI == (long)RDB[nuc + NUCLIDE_ZAI])))
		    {
		      /* Set number of groups */
		      
		      WDB[DATA_PRECURSOR_GROUPS] = (double)ng;
/*                      fprintf (err, "WDB[DATA_PRECURSOR_GROUPS] = %d, RDB[nuc + NUCLIDE_ZAI] = %d", WDB[DATA_PRECURSOR_GROUPS], (long)RDB[nuc+NUCLIDE_ZAI]); /* LMK */
		      
		      /* Exit subroutine */

		      return;
		    }
		}
	    }
	  
	  /* Next nuclide */
	  
	  nuc = NextItem(nuc);
	}
    }
}

/*****************************************************************************/
