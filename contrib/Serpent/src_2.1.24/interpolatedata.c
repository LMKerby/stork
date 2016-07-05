/*****************************************************************************/
/*                                                                           */
/* serpent 2 (beta-version) : interpolatedata.c                              */
/*                                                                           */
/* Created:       2010/12/13 (JLe)                                           */
/* Last modified: 2012/01/26 (JLe)                                           */
/* Version:       2.1.0                                                      */
/*                                                                           */
/* Description: Interpolates energy-dependent data from one array to another */
/*                                                                           */
/* Comments: - From Serpent 1.1.5                                            */
/*                                                                           */
/*****************************************************************************/

#include "header.h"

#define FUNCTION_NAME "InterpolateData:"

/*****************************************************************************/

long InterpolateData(const double *EE0, double *f0, long np0, 
		     const double *EE1, const double *f1, long np1, long mode,
		     long *i0, long *nf)
{
  long n, i, ineg;
  double E, E0, E1, xs0, xs1, xs;
  
  /* Reset values */

  E0 = -1.0;
  E1 = -1.0;

  /* Reset grid index */

  i = 0;

#ifdef DEBUG

  /* Check energy array */
  
  for (n = 0; n < np0; n++)
    CheckValue(FUNCTION_NAME, "EE0", "", EE0[n], ZERO, INFTY);

#endif

  /* Reset number of negative energy points */

  ineg = 0;

  /* Reset first non-negative point and number of points */

  if (i0 != NULL)
    *i0 = -1;

  if (nf != NULL)
    *nf = -1;

  /* Check mode */

  if (mode == 0)
    {
      /***** Linear interpolation ********************************************/

      /* Loop over original energy grid */

      for (n = 0; n < np0; n++)
	{
	  /* Get energy value */

	  E = EE0[n];
	  
	  /* Check boundaries */
	  
	  if ((E < EE1[0]) || (E > EE1[np1 - 1]))
	    xs = 0.0;
	  else
	    {
	      /* Find correct interval */
	      /* TODO: t�h�n tehokkaampi algoritmi? */
	      
	      while (i < np1)
		{
		  /* Get boundary values */
		  
		  E0 = EE1[i];
		  E1 = EE1[i + 1];
		  
		  if (E0 != E1)
		    if ((E == E0) || (E == E1) || ((E > E0) && (E < E1)))
		      break;
		  
		  i++;
		}
	      
	      /* Check order and co-incident energy points */
	      
	      if (E0 >= E1)
		Die(FUNCTION_NAME, "Error in original energy grid");
	  
	      /* Get xs values */
	      
	      xs0 = f1[i];
	      xs1 = f1[i + 1];
	      
	      /* Check values. Some isotopes may have VERY large negative   */
	      /* xs values (22046.03c in JEFF-3.1, 27058.00c in JEFF-3.1.1) */
	      
	      CheckValue(FUNCTION_NAME, "E", " (boundaries)", E, E0, E1);
	      CheckValue(FUNCTION_NAME, "E0", "", E0, 0.0, INFTY);
	      CheckValue(FUNCTION_NAME, "E1", "", E1, 0.0, INFTY);	  
	      CheckValue(FUNCTION_NAME, "xs0", "", xs0, -4E+3, MAX_XS);
	      CheckValue(FUNCTION_NAME, "xs1", "", xs1, -4E+3, MAX_XS);
	      
	      /* Convert negative values to zero */
	      
	      if (xs0 < 0.0)
		{
		  xs0 = 0.0;
		  ineg++;
		}
	      if (xs1 < 0.0)
		{
		  xs1 = 0.0;
		  ineg++;
		}
	      
	      /* Check co-inciding points and interpolate */
	      
	      if (E == E0)
		xs = xs0;
	      else if (E == E1)
		xs = xs1;
	      else 
		xs = ((E - E0)/(E1 - E0))*(xs1 - xs0) + xs0;
	    }

	  /* Check if first non-negative */

	  if (i0 != NULL)
	    if ((xs > 0.0) && (*i0 < 0))
	      {
		/* Put values */
		
		*i0 = n;
		
		if (nf != NULL)
		  *nf = np0 - n;
	      }

	  /* Put value */
	  
	  f0[n] = xs;
	}

      /***********************************************************************/
    }

  else if ((mode == 1) || (mode == 2))
    {
      /***** Log-log interpolation on linear scale ***************************/

      /* Loop over original energy grid */

      for (n = 0; n < np0; n++)
	{
	  /* Get energy value */

	  E = log(EE0[n]);
	  
	  /* Check boundaries */
	  
	  if ((E < EE1[0]) || (E > EE1[np1 - 1]))
	    xs = 0.0;
	  else
	    {
	      /* Find correct interval */
	      /* TODO: t�h�n tehokkaampi algoritmi? */
	      
	      while (i < np1)
		{
		  /* Get boundary values */
		  
		  E0 = EE1[i];
		  E1 = EE1[i + 1];
		  
		  if (E0 != E1)
		    if ((E == E0) || (E == E1) || ((E > E0) && (E < E1)))
		      break;
		  
		  i++;
		}
	      
	      /* Check order and co-incident energy points */
	      
	      if (E0 >= E1)
		Die(FUNCTION_NAME, "Error in original energy grid");
	  
	      /* Get xs values */
	      
	      xs0 = f1[i];
	      xs1 = f1[i + 1];

	      /* Convert to log if mode 2 */

	      if (mode == 2)
		{
		  xs0 = log(xs0);
		  xs1 = log(xs1);
		}
	      
	      /* Check values. Some isotopes may have negative xs values */
	      /* (22046.03c in JEFF-3.1) */
	      
	      CheckValue(FUNCTION_NAME, "E", " (boundaries)", E, E0, E1);
	      CheckValue(FUNCTION_NAME, "E0", "", exp(E0), 0.0, INFTY);
	      CheckValue(FUNCTION_NAME, "E1", "", exp(E1), 0.0, INFTY);	  
	      CheckValue(FUNCTION_NAME, "xs0", "", exp(xs0), -1.0, MAX_XS);
	      CheckValue(FUNCTION_NAME, "xs1", "", exp(xs1), -1.0, MAX_XS);
	      
	      /* Interpolate */
	      
	      xs = exp(((E - E0)/(E1 - E0))*(xs1 - xs0) + xs0);
	      
	      /*
	      xs = (E - E0)/(E1 - E0)*(exp(xs1) - exp(xs0)) + exp(xs0);
	      */
	      /*
	      xs = exp((exp(E) - exp(E0))/(exp(E1) - exp(E0))*(xs1 - xs0) + xs0);
	      */
	    }

	  /* Check if first non-negative */
	  
	  if (i0 != NULL)
	    if ((xs > 0.0) && (*i0 < 0))
	      {
		/* Put values */
		
		*i0 = n;
		
		if (nf != NULL)
		  *nf = np0 - n;
	      }
	  
	  /* Put value */
	  
	  f0[n] = xs;
	}

      /***********************************************************************/
    }
  else if (mode == 4)
    {
      /***** xs/E format elastic S(a,b) data *********************************/
      
      /* Loop over original energy grid */

      for (n = 0; n < np0; n++)
	{
	  /* Get energy value */
	  
	  E = EE0[n];
	  
	  /* Check boundaries */

	  if ((E < EE1[0]) || (E > EE1[np1 - 1]))
	    xs = 0.0;
	  else
	    {
	      /* Find correct interval */
	      /* TODO: t�h�n tehokkaampi algoritmi? */
	      
	      while (i < np1)
		{
		  /* Get boundary values */
		  
		  E0 = EE1[i];
		  E1 = EE1[i + 1];
		  
		  if (E0 != E1)
		    if ((E == E0) || (E == E1) || ((E > E0) && (E < E1)))
		      break;
		  
		  i++;
		}
	  
	      /* Check order and co-incident energy points */
	      
	      if (E0 >= E1)
		Die(FUNCTION_NAME, "Error in original energy grid");
	      
	      /* Get xs factor */
	      
	      xs0 = f1[i];
	      
	      /* Check values */

	      CheckValue(FUNCTION_NAME, "E", " (boundaries)", E, E0, E1); 
	      CheckValue(FUNCTION_NAME, "E0", "", E0, 0.0, INFTY);
	      CheckValue(FUNCTION_NAME, "E1", "", E1, 0.0, INFTY);	  
	      CheckValue(FUNCTION_NAME, "xs0", "", xs0, -1.0, MAX_XS);
	  
	      /* Calculate xs */

	      xs = xs0/E;
	    }

	  /* Check if first non-negative */

	  if (i0 != NULL)
	    if ((xs > 0.0) && (*i0 < 0))
	      {
		/* Put values */
		
		*i0 = n;
		
		if (nf != NULL)
		  *nf = np0 - n;
	      }
	  
	  /* Put value */
	  
	  f0[n] = xs;
	}
      
      /***********************************************************************/
    }
  else
    Die(FUNCTION_NAME, "Invalid data mode %ld", mode);

  /* Exit subroutine */

  return ineg;
}

/*****************************************************************************/
