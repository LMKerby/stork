/*****************************************************************************/
/*                                                                           */
/* serpent 2 (beta-version) : surfacedistance.c                              */
/*                                                                           */
/* Created:       2010/10/10 (JLe)                                           */
/* Last modified: 2015/03/05 (JLe)                                           */
/* Version:       2.1.23                                                     */
/*                                                                           */
/* Description: Calculates minimum distance to a surface                     */
/*                                                                           */
/* Comments:                                                                 */
/*                                                                           */
/*****************************************************************************/

#include "header.h"
#include "locations.h"

#define FUNCTION_NAME "SurfaceDistance:"

/*****************************************************************************/

double SurfaceDistance(long surf, const double *params, long type, long np, 
		       double x, double y, double z,
		       double u, double v, double w, long id)
{
  long n, i, ptr;
  double r, l, h, min, a, b, c, d, d0, d1, k, th1, th2;
  double A, B, C, D, E, F, G, H, J, K, L, M, N, S, r0, r1;
  double x1, y1, z1, x2, y2, z2, x3, y3, z3;
  double La, Lb, Lc, th, ps, ph, xmax, ymax, zmax;
  double a1, b1, c1, a2, b2, c2, d2, a3, b3, c3, d3, a4, b4, c4, d4, a5, b5;
  double c5, d5, a6, b6, c6, d6;
  double tmp[MAX_SURFACE_PARAMS];

  /* Check pointer */

#ifdef DEBUG

  if (params == NULL)
    Die(FUNCTION_NAME, "Null pointer");

#endif

  /* Do coordinate transformation */

  if (surf > VALID_PTR)
    if ((ptr = (long)RDB[surf + SURFACE_PTR_TRANS]) > VALID_PTR)
      CoordTrans(ptr, &x, &y, &z, &u, &v, &w, id);

  /* Reset parameter index */

  n = 0;

  /* Set minimum distance to infinity */

  min = INFTY;

  /* Check surface type */

  switch (type)
    {
      /***********************************************************************/
      
    case SURF_INF:
      {
	return INFTY;
      }

      /***********************************************************************/

    case SURF_PX:
      {
	/* shift origin */

	x = x - params[n];

	if (u == 0.0)
	  return INFTY;
	else if ((d = -x/u) < 0)
	  return INFTY;
	else
	  return d;
      }

      /***********************************************************************/

    case SURF_PY:
      {
	/* shift origin */

	y = y - params[n];
	
	if (v == 0.0)
	  return INFTY;
	else if ((d = -y/v) < 0)
	  return INFTY;
	else
	  return d;
      }

      /***********************************************************************/

    case SURF_PZ:
      {
	/* shift origin */

	z = z - params[n];

	if (w == 0.0)
	  return INFTY;
	else if ((d = -z/w) < 0)
	  return INFTY;
	else
	  return d;
      }

      /***********************************************************************/

    case SURF_SQC:
      {
	/* Shift origin */
	
	x = x - params[n++];
	y = y - params[n++];

	/* Get radius */

	r = params[n++];

	/* Calculate minimum distance */

	if (u != 0.0)
	  {
	    if (((d = -(x - r)/u) > 0.0) && (d < min))
	      min = d;
	    if (((d = -(x + r)/u) > 0.0) && (d < min))
	      min = d;
	  }

	if (v != 0.0)
	  {
	    if (((d = -(y - r)/v) > 0.0) && (d < min))
	      min = d;
	    if (((d = -(y + r)/v) > 0.0) && (d < min))
	      min = d;
	  }
	
	/* Rounded corners */

	if (n < np)
	  {
	    /* Get radius */

	    r0 = params[n];
	    tmp[2] = r0;

	    /* Check each corner */

	    tmp[0] = -r + r0; 
	    tmp[1] = -r + r0;

	    if (((d = SurfaceDistance(-1, tmp, SURF_CYL, 3, x, y, z, 
				      u, v, w, id))
		 > 0.0) && (d < min))
	      min = d;

	    tmp[0] =  r - r0; 
	    tmp[1] = -r + r0;

	    if (((d = SurfaceDistance(-1, tmp, SURF_CYL, 3, x, y, z, 
				      u, v, w, id))
		 > 0.0) && (d < min))
	      min = d;

	    tmp[0] =  r - r0;
	    tmp[1] =  r - r0;
	    
	    if (((d = SurfaceDistance(-1, tmp, SURF_CYL, 3, x, y, z, 
				      u, v, w, id))
		 > 0.0) && (d < min))
	      min = d;

	    tmp[0] = -r + r0;
	    tmp[1] =  r - r0;
	    
	    if (((d = SurfaceDistance(-1, tmp, SURF_CYL, 3, x, y, z, 
				      u, v, w, id))
		 > 0.0) && (d < min))
	      min = d;
	  }

	/* Return distance */

	return min;
      }
      
      /***********************************************************************/
      
    case SURF_CUBE:
      {
	/* Shift origin */
	
	x = x - params[n++];
	y = y - params[n++];
	z = z - params[n++];

	/* Get radius */

	r = params[n];

	/* Calculate minimum distance */

	if (u != 0.0)
	  {
	    if (((d = -(x - r)/u) > 0.0) && (d < min))
	      min = d;
	    if (((d = -(x + r)/u) > 0.0) && (d < min))
	      min = d;
	  }
	
	if (v != 0.0)
	  {
	    if (((d = -(y - r)/v) > 0.0) && (d < min))
	      min = d;
	    if (((d = -(y + r)/v) > 0.0) && (d < min))
	      min = d;
	  }

	if (w != 0.0)
	  {
	    if (((d = -(z - r)/w) > 0.0) && (d < min))
	      min = d;
	    if (((d = -(z + r)/w) > 0.0) && (d < min))
	      min = d;
	  }
	
	/* Return distance */

	return min;
      }

      /***********************************************************************/

    case SURF_CUBOID:
      {
	/* Calculate minimum distance */

	if (u != 0.0)
	  {
	    if (((d = -(x - params[n])/u) > 0.0) && (d < min))
	      min = d;
	    if (((d = -(x - params[n + 1])/u) > 0.0) && (d < min))
	      min = d;
	  }
	
	if (v != 0.0)
	  {
	    if (((d = -(y - params[n + 2])/v) > 0.0) && (d < min))
	      min = d;
	    if (((d = -(y - params[n + 3])/v) > 0.0) && (d < min))
	      min = d;
	  }

	if (w != 0.0)
	  {
	    if (((d = -(z - params[n + 4])/w) > 0.0) && (d < min))
	      min = d;
	    if (((d = -(z - params[n + 5])/w) > 0.0) && (d < min))
	      min = d;
	  }
	
	/* Return distance */

	return min;
      }

      /***********************************************************************/

    case SURF_RECT:
      {
	/* Calculate minimum distance */

	if (u != 0.0)
	  {
	    if (((d = -(x - params[n])/u) > 0.0) && (d < min))
	      min = d;
	    if (((d = -(x - params[n + 1])/u) > 0.0) && (d < min))
	      min = d;
	  }
	
	if (v != 0.0)
	  {
	    if (((d = -(y - params[n + 2])/v) > 0.0) && (d < min))
	      min = d;
	    if (((d = -(y - params[n + 3])/v) > 0.0) && (d < min))
	      min = d;
	  }
	
	/* Return distance */

	return min;
      }

      /***********************************************************************/

    case SURF_HEXYC:
      {
	/* Shift origin */
	
	x = x - params[n++];
	y = y - params[n++];
      
	/* Get radius */

	r = params[n++];

	/* Calculate minimum distance */
	
	if (v != 0.0)
	  {
	    if (((d = -(y - r)/v) > 0) && (d < min))
	      min = d;
	    if (((d = -(y + r)/v) > 0) && (d < min))
	      min = d;
	  }

	if ((v - SQRT3*u) != 0.0)
	  {
	    if (((d = (-y + SQRT3*x + 2*r)/(v - SQRT3*u)) > 0) && (d < min))
	      min = d;
	    
	    if (((d = (-y + SQRT3*x - 2*r)/(v - SQRT3*u)) > 0) && (d < min))
	      min = d;
	  }
	
	if ((v + SQRT3*u) != 0.0)
	  {
	    if (((d = (-y - SQRT3*x + 2*r)/(v + SQRT3*u)) > 0) && (d < min))
	      min = d;
	    if (((d = (-y - SQRT3*x - 2*r)/(v + SQRT3*u)) > 0) && (d < min))
	      min = d;
	  }

	/* Rounded corners */

	if (n < np)
	  {
	    /* Get radius */

	    r0 = params[n];
	    tmp[2] = r0;

	    /* Check each corner */

	    tmp[0] = (-r + r0)/(2.0*COS30);
	    tmp[1] =  -r + r0;

	    if (((d = SurfaceDistance(-1, tmp, SURF_CYL, 3, x, y, z, 
				      u, v, w, id))
		 > 0.0) && (d < min))
	      min = d;

	    tmp[0] = (r - r0)/(2.0*COS30); 
	    tmp[1] = -r + r0;

	    if (((d = SurfaceDistance(-1, tmp, SURF_CYL, 3, x, y, z, 
				      u, v, w, id))
		 > 0.0) && (d < min))
	      min = d;

	    tmp[0] = (r - r0)/(2.0*COS30);
	    tmp[1] =  r - r0;
	    
	    if (((d = SurfaceDistance(-1, tmp, SURF_CYL, 3, x, y, z, 
				      u, v, w, id))
		 > 0.0) && (d < min))
	      min = d;

	    tmp[0] = (-r + r0)/(2.0*COS30);
	    tmp[1] =   r - r0;
	    
	    if (((d = SurfaceDistance(-1, tmp, SURF_CYL, 3, x, y, z, 
				      u, v, w, id))
		 > 0.0) && (d < min))
	      min = d;

	    tmp[0] = (-r + r0)/(COS30);
	    tmp[1] =  0.0;
	    
	    if (((d = SurfaceDistance(-1, tmp, SURF_CYL, 3, x, y, z, 
				      u, v, w, id))
		 > 0.0) && (d < min))
	      min = d;

	    tmp[0] = (r - r0)/(COS30);
	    tmp[1] =  0.0;
	    
	    if (((d = SurfaceDistance(-1, tmp, SURF_CYL, 3, x, y, z, 
				      u, v, w, id))
		 > 0.0) && (d < min))
	      min = d;
	  }

	/* Return distance */

	return min;
      }

      /***********************************************************************/
      
    case SURF_HEXXC:      
      {
	/* Shift origin */
	
	x = x - params[n++];
	y = y - params[n++];
	
	/* Get radius */

	r = params[n++];

	/* Calculate minimum distance */
	
	if (u != 0.0)
	  {
	    if (((d = -(x - r)/u) > 0) && (d < min))
	      min = d;
	    if (((d = -(x + r)/u) > 0) && (d < min))
	      min = d;
	  }

	if ((u - SQRT3*v) != 0.0)
	  {
	    if (((d = (-x + SQRT3*y + 2*r)/(u - SQRT3*v)) > 0) && (d < min))
	      min = d;

	    if (((d = (-x + SQRT3*y - 2*r)/(u - SQRT3*v)) > 0) && (d < min))
	      min = d;
	  }

	if ((u + SQRT3*v) != 0.0)
	  {
	    if (((d = (-x - SQRT3*y + 2*r)/(u + SQRT3*v)) > 0) && (d < min))
	      min = d;
	    if (((d = (-x - SQRT3*y - 2*r)/(u + SQRT3*v)) > 0) && (d < min))
	      min = d;
	  }

	/* Rounded corners */

	if (n < np)
	  {
	    /* Get radius */

	    r0 = params[n];
	    tmp[2] = r0;

	    /* Check each corner */

	    tmp[1] = (-r + r0)/(2.0*COS30);
	    tmp[0] =  -r + r0;

	    if (((d = SurfaceDistance(-1, tmp, SURF_CYL, 3, x, y, z, 
				      u, v, w, id))
		 > 0.0) && (d < min))
	      min = d;

	    tmp[1] = (r - r0)/(2.0*COS30); 
	    tmp[0] = -r + r0;

	    if (((d = SurfaceDistance(-1, tmp, SURF_CYL, 3, x, y, z, 
				      u, v, w, id))
		 > 0.0) && (d < min))
	      min = d;

	    tmp[1] = (r - r0)/(2.0*COS30);
	    tmp[0] =  r - r0;
	    
	    if (((d = SurfaceDistance(-1, tmp, SURF_CYL, 3, x, y, z, 
				      u, v, w, id))
		 > 0.0) && (d < min))
	      min = d;

	    tmp[1] = (-r + r0)/(2.0*COS30);
	    tmp[0] =   r - r0;
	    
	    if (((d = SurfaceDistance(-1, tmp, SURF_CYL, 3, x, y, z, 
				      u, v, w, id))
		 > 0.0) && (d < min))
	      min = d;

	    tmp[1] = (-r + r0)/(COS30);
	    tmp[0] =  0.0;
	    
	    if (((d = SurfaceDistance(-1, tmp, SURF_CYL, 3, x, y, z, 
				      u, v, w, id))
		 > 0.0) && (d < min))
	      min = d;

	    tmp[1] = (r - r0)/(COS30);
	    tmp[0] =  0.0;
	    
	    if (((d = SurfaceDistance(-1, tmp, SURF_CYL, 3, x, y, z, 
				      u, v, w, id))
		 > 0.0) && (d < min))
	      min = d;
	  }

	/* Return distance */

	return min;
      }

      /***********************************************************************/

    case SURF_HEXYPRISM:
      {
	/* Shift origin */
	
	x = x - params[n++];
	y = y - params[n++];
      
	/* Get radius */

	r = params[n++];

	/* Calculate minimum distance */
	
	if (v != 0.0)
	  {
	    if (((d = -(y - r)/v) > 0) && (d < min))
	      min = d;
	    if (((d = -(y + r)/v) > 0) && (d < min))
	      min = d;
	  }

	if ((v - SQRT3*u) != 0.0)
	  {
	    if (((d = (-y + SQRT3*x + 2*r)/(v - SQRT3*u)) > 0) && (d < min))
	      min = d;
	    
	    if (((d = (-y + SQRT3*x - 2*r)/(v - SQRT3*u)) > 0) && (d < min))
	      min = d;
	  }
	
	if ((v + SQRT3*u) != 0.0)
	  {
	    if (((d = (-y - SQRT3*x + 2*r)/(v + SQRT3*u)) > 0) && (d < min))
	      min = d;
	    if (((d = (-y - SQRT3*x - 2*r)/(v + SQRT3*u)) > 0) && (d < min))
	      min = d;
	  }

	if (w != 0.0)
	  {
	    if (((d = -(z - params[n++])/w) > 0.0) && (d < min))
	      min = d;
	    if (((d = -(z - params[n++])/w) > 0.0) && (d < min))
	      min = d;
	  }


	/* Return distance */

	return min;
      }

      /***********************************************************************/

    case SURF_HEXXPRISM:
      {
	/* Shift origin */
	
	x = x - params[n++];
	y = y - params[n++];
	
	/* Get radius */

	r = params[n++];

	/* Calculate minimum distance */
	
	if (u != 0.0)
	  {
	    if (((d = -(x - r)/u) > 0) && (d < min))
	      min = d;
	    if (((d = -(x + r)/u) > 0) && (d < min))
	      min = d;
	  }

	if ((u - SQRT3*v) != 0.0)
	  {
	    if (((d = (-x + SQRT3*y + 2*r)/(u - SQRT3*v)) > 0) && (d < min))
	      min = d;

	    if (((d = (-x + SQRT3*y - 2*r)/(u - SQRT3*v)) > 0) && (d < min))
	      min = d;
	  }

	if ((u + SQRT3*v) != 0.0)
	  {
	    if (((d = (-x - SQRT3*y + 2*r)/(u + SQRT3*v)) > 0) && (d < min))
	      min = d;
	    if (((d = (-x - SQRT3*y - 2*r)/(u + SQRT3*v)) > 0) && (d < min))
	      min = d;
	  }

	if (w != 0.0)
	  {
	    if (((d = -(z - params[n++])/w) > 0.0) && (d < min))
	      min = d;
	    if (((d = -(z - params[n++])/w) > 0.0) && (d < min))
	      min = d;
	  }


	/* Return distance */

	return min;
      }

      /***********************************************************************/

    case SURF_CYL:
    case SURF_CYLZ:
      {
	/* NOTE: Tähän lisättiin katkaisu z-suunnassa 3.12.2009 (JLE) */

	/* Shift origin */
	
	x = x - params[n++];
	y = y - params[n++];
	
	/* Get radius */

	r = params[n++];

	/* Check if cylinder is cut */

	if (np == 5)
	  {
	    /* First surface */

	    if (w == 0.0)
	      d0 = INFTY;
	    else if ((d0 = -(z - params[n++])/w) < 0)
	      d0 = INFTY;

	    /* Second surface */
	    
	    if (w == 0.0)
	      d1 = INFTY;
	    else if ((d1 = -(z - params[n++])/w) < 0)
	      d1 = INFTY;

	    /* Compare */

	    if (d0 < d1)
	      min = d0;
	    else
	      min = d1;
	  }
	else
	  min = INFTY;

	/* Calculate constants */

	if ((a = 1.0 - w*w) == 0.0)
	  return min;
	
	b = u*x + v*y;      
	c = x*x + y*y - r*r;

	if ((d0 = b*b - a*c) < 0.0)
	  {
	    /* No line-of-sight */

	    return INFTY;
	  }
	else if (c < 0)
	  {
	    /* Point is inside, only one root possible */

	    d = -(b - sqrt(d0))/a;
	  }
	else if ((d = -(b + sqrt(d0))/a) < 0.0)
	  {
	    /* Point is outside, line-of-sight, opposite direction */

	    return INFTY;
	  }
	
	/* Compare to minimum */

	if (d < min)
	  min = d;

	/* Return distance */

	return min;
      }	
      
      /***********************************************************************/

    case SURF_CYLX:
      {
	/* Shift origin */
	
	y = y - params[n++];
	z = z - params[n++];
	
	/* Get radius */

	r = params[n++];

	/* Check if cylinder is cut */

	if (np == 5)
	  {
	    /* First surface */

	    if (u == 0.0)
	      d0 = INFTY;
	    else if ((d0 = -(x - params[n++])/u) < 0)
	      d0 = INFTY;

	    /* Second surface */
	    
	    if (u == 0.0)
	      d1 = INFTY;
	    else if ((d1 = -(x - params[n++])/u) < 0)
	      d1 = INFTY;

	    /* Compare */

	    if (d0 < d1)
	      min = d0;
	    else
	      min = d1;
	  }
	else
	  min = INFTY;

	/* Calculate constants */

	if ((a = 1.0 - u*u) == 0.0)
	  return min;
	
	b = v*y + w*z;      
	c = y*y + z*z - r*r;

	if ((d0 = b*b - a*c) < 0.0)
	  {
	    /* No line-of-sight */

	    return INFTY;
	  }
	else if (c < 0)
	  {
	    /* Point is inside, only one root possible */

	    d = -(b - sqrt(d0))/a;
	  }
	else if ((d = -(b + sqrt(d0))/a) < 0.0)
	  {
	    /* Point is outside, line-of-sight, opposite direction */

	    return INFTY;
	  }
	
	/* Compare to minimum */

	if (d < min)
	  min = d;

	/* Return distance */

	return min;
      }	
      
      /***********************************************************************/

    case SURF_CYLY:
      {
	/* Shift origin */
	
	x = x - params[n++];
	z = z - params[n++];
	
	/* Get radius */

	r = params[n++];

	/* Check if cylinder is cut */

	if (np == 5)
	  {
	    /* First surface */

	    if (v == 0.0)
	      d0 = INFTY;
	    else if ((d0 = -(y - params[n++])/v) < 0)
	      d0 = INFTY;

	    /* Second surface */
	    
	    if (v == 0.0)
	      d1 = INFTY;
	    else if ((d1 = -(y - params[n++])/v) < 0)
	      d1 = INFTY;

	    /* Compare */

	    if (d0 < d1)
	      min = d0;
	    else
	      min = d1;
	  }
	else
	  min = INFTY;

	/* Calculate constants */

	if ((a = 1.0 - v*v) == 0.0)
	  return min;
	
	b = u*x + w*z;      
	c = x*x + z*z - r*r;

	if ((d0 = b*b - a*c) < 0.0)
	  {
	    /* No line-of-sight */

	    return INFTY;
	  }
	else if (c < 0)
	  {
	    /* Point is inside, only one root possible */

	    d = -(b - sqrt(d0))/a;
	  }
	else if ((d = -(b + sqrt(d0))/a) < 0.0)
	  {
	    /* Point is outside, line-of-sight, opposite direction */

	    return INFTY;
	  }
	
	/* Compare to minimum */

	if (d < min)
	  min = d;

	/* Return distance */

	return min;
      }	
      
      /***********************************************************************/

    case SURF_CONE:
      {
	/* Shift origin */
	
	x = x - params[n++];
	y = y - params[n++];
	z = z - params[n++];
	
	/* Get radius */

	r = params[n++];

	/* Get height */

	h = params[n];

	/* Simplify calculation by coordinate substitution */

	z = r*(1 - z/h);
	w = -r*w/h;

	/* Calculate constants */

	a = u*u + v*v - w*w;
	b = x*u + v*y - w*z;
	c = x*x + y*y - z*z;

	/* Calculate discriminant */

	if ((d = b*b - a*c) < 0.0)
	  {
	    /* No line-of-sight */

	    return INFTY;
	  }
	else
	  d = sqrt(d);

	/* Calculate roots */

	d0 = -(b + d)/a;
	d1 = -(b - d)/a;

	/* Check different possibilities */

	if ((d0 < 0.0) && (d1 < 0.0))
	  {
	    /* Both negative, no line-of-sight */

	    return INFTY;
	  }
	else if ((d0 > 0.0) && (d1 > 0.0))
	  {
	    /* Both positive, pick shortest */

	    if (d0 > d1)
	      return d1;
	    else
	      return d0;
	  }
	else
	  {
	    /* One positive, one negative, pick positive */
	    
	    if (d0 > 0.0)
	      return d0;
	    else
	      return d1;
	  }
      }	
      
      /***********************************************************************/

    case SURF_SPH:
      {
	/* Shift origin */
	
	x = x - params[n++];
	y = y - params[n++];
	z = z - params[n++];
	
	/* Get radius */

	r = params[n];

	/* Calculate constants */

	b = u*x + v*y + w*z;      
	c = x*x + y*y + z*z - r*r;

	if ((d0 = b*b - c) < 0.0)
	  {
	    /* No line-of-sight */

	    return INFTY;
	  }
	else if (c < 0)
	  {
	    /* Point is inside, only one root possible */

	    min = -(b - sqrt(d0));
	  }
	else if ((d = -(b + sqrt(d0))) < 0.0)
	  {
	    /* Point is outside, line-of-sight, opposite direction */

	    return INFTY;
	  }
	else
	  {
	    /* Point is outside, line-of-sight, towards surface */

	    min = d;
	  }

	/* Return distance */

	return min;
      }	
      
      /***********************************************************************/

    case SURF_PAD:
      {
	/* Shift origin */
	
	x = x - params[n++];
	y = y - params[n++];

	/* Get parameters */

	r0 = params[n++];
	r1 = params[n++];
	th1 = PI*params[n++]/180.0;
	th2 = PI*params[n]/180.0;

	/* Distance from inner ring */

	tmp[0] = 0.0;
	tmp[1] = 0.0;
	tmp[2] = r0;

	if (((d = SurfaceDistance(-1, tmp, SURF_CYL, 3, x, y, z, u, v, w, id))
	     > 0.0) && (d < min))
	  min = d;

	/* Distance from outer ring */

	tmp[0] = 0.0;
	tmp[1] = 0.0;
	tmp[2] = r1;

	if (((d = SurfaceDistance(-1, tmp, SURF_CYL, 3, x, y, z, u, v, w, id))
	     > 0.0) && (d < min))
	  min = d;

	/* Check sector width */

	if (th1 - th2 < 2.0*PI)
	  {
	    /* Distance from sectors */
	    
	    k = -sin(th1)/cos(th1);
	    
	    if (((v - k*u) != 0.0) &&
		((d = (k*x - y)/(v - k*u)) > 0.0) && (d < min))
	      min = d;
	    
	    /* Distance from sectors */
	    
	    k = -sin(th2)/cos(th2);
	    
	    if (((v - k*u) != 0.0) &&
		((d = (k*x - y)/(v - k*u)) > 0.0) && (d < min))
	      min = d;
	  }

	/* Return distance */
	
	return min;
      }

      /***********************************************************************/
      
    case SURF_CROSS:
      {
	/* Shift origin */
	
	x = x - params[n++];
	y = y - params[n++];

	/* Get length and width */

	l = params[n++];
	r = params[n++];

	/* Calculate minimum distance */

	if (u != 0.0)
	  {
	    if (((d = -(x - l)/u) > 0.0) && (d < min))
	      min = d;
	    if (((d = -(x - r)/u) > 0.0) && (d < min))
	      min = d;
	    if (((d = -(x + r)/u) > 0.0) && (d < min))
	      min = d;
	    if (((d = -(x + l)/u) > 0.0) && (d < min))
	      min = d;
	  }
	
	if (v != 0.0)
	  {
	    if (((d = -(y - l)/v) > 0.0) && (d < min))
	      min = d;
	    if (((d = -(y - r)/v) > 0.0) && (d < min))
	      min = d;
	    if (((d = -(y + r)/v) > 0.0) && (d < min))
	      min = d;
	    if (((d = -(y + l)/v) > 0.0) && (d < min))
	      min = d;
	  }

	/* Rounded corners */

	if (n < np)
	  {
	    /* Get radius (use width) */

	    r0 = r;
	    tmp[2] = r0;

	    /* Check each corner */

	    tmp[0] = 0.0;
	    tmp[1] = -l + r0;

	    if (((d = SurfaceDistance(-1, tmp, SURF_CYL, 3, x, y, z, 
				      u, v, w, id))
		 > 0.0) && (d < min))
	      min = d;

	    tmp[0] = 0.0;
	    tmp[1] = l - r0;

	    if (((d = SurfaceDistance(-1, tmp, SURF_CYL, 3, x, y, z, 
				      u, v, w, id))
		 > 0.0) && (d < min))
	      min = d;

	    tmp[0] = l - r0;
	    tmp[1] = 0.0;
	    
	    if (((d = SurfaceDistance(-1, tmp, SURF_CYL, 3, x, y, z, 
				      u, v, w, id))
		 > 0.0) && (d < min))
	      min = d;

	    tmp[0] = -l + r0;
	    tmp[1] = 0.0;
	    
	    if (((d = SurfaceDistance(-1, tmp, SURF_CYL, 3, x, y, z, 
				      u, v, w, id))
		 > 0.0) && (d < min))
	      min = d;
	  }

	/* Return distance */

	return min;
      }

      /***********************************************************************/
      
    case SURF_DODE:
      {
	/* Shift origin */
	
	x = x - params[n++];
	y = y - params[n++];
      
	/* Get radius */
	
	r = params[n++];

	if (np == 4)
	  l = params[n++];
	else
	  l = r;	
	
	/* Calculate minimum distance */
	
	if (v != 0.0)
	  {
	    /* Yläreuna (y-suunta) pinta 1 */
	    
	    if (((d = -(y - l)/v) > 0) && (d < min))
	      min = d;
	    
	    /* Alareuna (y-suunta) pinta 7 */
	    
	    if (((d = -(y + l)/v) > 0) && (d < min))
	      min = d;
	  }
	
	if (u != 0.0)
	  {
	    /* Oikea reuna (x-suunta) pinta 10 */
		
	    if (((d = -(x - r)/u) > 0) && (d < min))
	      min = d;
		
	    /* Vasen reuna (x-suunta) pinta 4 */
	    
	    if (((d = -(x + r)/u) > 0) && (d < min))
	      min = d;
	  }

	/* Yläpinnasta alkaen positiiviseen kiertosuuntaan pinnat 3 ja 9 */
	
	if ((v - SQRT3*u) != 0.0)
	  {
	    if (((d = (-y + SQRT3*x + 2*l)/(v - SQRT3*u)) > 0) && (d < min))
	      min = d;
	    
	    if (((d = (-y + SQRT3*x - 2*l)/(v - SQRT3*u)) > 0) && (d < min))
	      min = d;
	  }

	/* Yläpinnasta alkaen positiiviseen kiertosuuntaan pinnat 5 ja 11 */
	
	if ((v + SQRT3*u) != 0.0)
	  {
	    if (((d = (-y - SQRT3*x + 2*l)/(v + SQRT3*u)) > 0) && (d < min))
	      min = d;
	    
	    if (((d = (-y - SQRT3*x - 2*l)/(v + SQRT3*u)) > 0) && (d < min))
	      min = d;
	  }

	/* Yläpinnasta alkaen positiiviseen kiertosuuntaan pinnat 2 ja 8 */
	
	if ((u - SQRT3*v) != 0.0)
	  {
	    if (((d = (-x + SQRT3*y + 2*r)/(u - SQRT3*v)) > 0) && (d < min))
	      min = d;
	    
	    if (((d = (-x + SQRT3*y - 2*r)/(u - SQRT3*v)) > 0) && (d < min))
	      min = d;
	  }
	
	/* Yläpinnasta alkaen positiiviseen kiertosuuntaan pinnat 6 ja 12 */
	
	if ((u + SQRT3*v) != 0.0)
	  {
	    if (((d = (-x - SQRT3*y + 2*r)/(u + SQRT3*v)) > 0) && (d < min))
	      min = d;
	    
	    if (((d = (-x - SQRT3*y - 2*r)/(u + SQRT3*v)) > 0) && (d < min))
	      min = d;
	  }
	
	/* Return distance */
	
	return min;
      }

      /***********************************************************************/

    case SURF_OCTA:
      {
	/* Shift origin */
	
	x = x - params[n++];
	y = y - params[n++];
      
	/* Get radius */
	
	r = params[n++];

	if (np == 4)
	  l = params[n++];
	else
	  l = r;	
	
	/* Calculate minimum distance */
	
	if (v != 0.0)
	  {
	    /* Yläreuna (y-suunta) pinta 1 */
	    
	    if (((d = -(y - r)/v) > 0) && (d < min))
	      min = d;
	    
	    /* Alareuna (y-suunta) pinta 5 */
	    
	    if (((d = -(y + r)/v) > 0) && (d < min))
	      min = d;
	  }
	
	if (u != 0.0)
	  {
	    /* Oikea reuna (x-suunta) pinta 7 */
		
	    if (((d = -(x - r)/u) > 0) && (d < min))
	      min = d;
		
	    /* Vasen reuna (x-suunta) pinta 3 */
	    
	    if (((d = -(x + r)/u) > 0) && (d < min))
	      min = d;
	  }

	/* Yläpinnasta alkaen positiiviseen kiertosuuntaan pinnat 2 ja 6 */
	
	if ((v - u) != 0.0)
	  {
	    if (((d = (-y + x + SQRT2*l)/(v - u)) > 0) && (d < min))
	      min = d;
	    
	    if (((d = (-y + x - SQRT2*l)/(v - u)) > 0) && (d < min))
	      min = d;
	  }

	/* Yläpinnasta alkaen positiiviseen kiertosuuntaan pinnat 4 ja 8 */
	
	if ((v + u) != 0.0)
	  {
	    if (((d = (-y - x + SQRT2*l)/(v + u)) > 0) && (d < min))
	      min = d;
	    
	    if (((d = (-y - x - SQRT2*l)/(v + u)) > 0) && (d < min))
	      min = d;
	  }

	/* Return distance */
	
	return min;
      }
	  
      /***********************************************************************/
 
    case SURF_ASTRA:
      {
	/* Shift origin */
	
	x = x - params[n++];
	y = y - params[n++];
      
	/* Get radius */
	
	r = params[n++];
	l = params[n++];
	
	/* Calculate minimum distance */
	
	if (v != 0.0)
	  {
	    /* Yläreuna (y-suunta) pinta 1 */
	    
	    if (((d = -(y - r)/v) > 0) && (d < min))
	      min = d;
	    
	    /* Alareuna (y-suunta) pinta 5 */
	    
	    if (((d = -(y + r)/v) > 0) && (d < min))
	      min = d;
	  }
	
	if (u != 0.0)
	  {
	    /* Oikea reuna (x-suunta) pinta 7 */
		
	    if (((d = -(x - r)/u) > 0) && (d < min))
	      min = d;
		
	    /* Vasen reuna (x-suunta) pinta 3 */
	    
	    if (((d = -(x + r)/u) > 0) && (d < min))
	      min = d;
	  }

	/* Yläpinnasta alkaen positiiviseen kiertosuuntaan pinnat 2 ja 6 */
	
	if ((v - u) != 0.0)
	  {
	    if (((d = (-y + x + SQRT2*l)/(v - u)) > 0) && (d < min))
	      min = d;
	    
	    if (((d = (-y + x - SQRT2*l)/(v - u)) > 0) && (d < min))
	      min = d;
	  }

	/* Yläpinnasta alkaen positiiviseen kiertosuuntaan pinnat 4 ja 8 */
	
	if ((v + u) != 0.0)
	  {
	    if (((d = (-y - x + SQRT2*l)/(v + u)) > 0) && (d < min))
	      min = d;
	    
	    if (((d = (-y - x - SQRT2*l)/(v + u)) > 0) && (d < min))
	      min = d;
	  }
	
	/* Ihme pallerot */
	
	/* Get radius */

	r0 = params[n];
	tmp[2] = r0;

	/* Upper left */

	tmp[0] = 0.5*r - 0.75*SQRT2*l;
	tmp[1] = 0.5*r + 0.25*SQRT2*l;
	    
	if (((d = SurfaceDistance(-1, tmp, SURF_CYL, 3, x, y, z, u, v, w, id))
	  > 0.0) && (d < min))
	  min = d;

	tmp[0] = -0.5*r - 0.25*SQRT2*l;
	tmp[1] = -0.5*r + 0.75*SQRT2*l;
	    
	if (((d = SurfaceDistance(-1, tmp, SURF_CYL, 3, x, y, z, u, v, w, id))
	  > 0.0) && (d < min))
	  min = d;
		  
	/* Lower left */

	tmp[0] = -0.5*r - 0.25*SQRT2*l;
	tmp[1] =  0.5*r - 0.75*SQRT2*l;
	    
	if (((d = SurfaceDistance(-1, tmp, SURF_CYL, 3, x, y, z, u, v, w, id))
	  > 0.0) && (d < min))
	  min = d;
		  
	tmp[0] =  0.5*r - 0.75*SQRT2*l;
	tmp[1] = -0.5*r - 0.25*SQRT2*l;
	    
	if (((d = SurfaceDistance(-1, tmp, SURF_CYL, 3, x, y, z, u, v, w, id))
	  > 0.0) && (d < min))
	  min = d;
		
	/* Lower right */
		  
	tmp[0] = -0.5*r + 0.75*SQRT2*l;
	tmp[1] = -0.5*r - 0.25*SQRT2*l;

	if (((d = SurfaceDistance(-1, tmp, SURF_CYL, 3, x, y, z, u, v, w, id))
	  > 0.0) && (d < min))
	  min = d;

	tmp[0] =  0.5*r + 0.25*SQRT2*l;
	tmp[1] =  0.5*r - 0.75*SQRT2*l;
	    
	if (((d = SurfaceDistance(-1, tmp, SURF_CYL, 3, x, y, z, u, v, w, id))
	  > 0.0) && (d < min))
	  min = d;
		  
	/* Upper right */
		
	tmp[0] =  0.5*r + 0.25*SQRT2*l;
	tmp[1] = -0.5*r + 0.75*SQRT2*l;

	if (((d = SurfaceDistance(-1, tmp, SURF_CYL, 3, x, y, z, u, v, w, id))
	  > 0.0) && (d < min))
	  min = d;

	tmp[0] = -0.5*r + 0.75*SQRT2*l;
	tmp[1] =  0.5*r + 0.25*SQRT2*l;

	if (((d = SurfaceDistance(-1, tmp, SURF_CYL, 3, x, y, z, u, v, w, id))
	  > 0.0) && (d < min))
	  min = d;		  

	/* Return distance */

	return min;
      }
      
      /***********************************************************************/
      
    case SURF_PLANE:
      {
	/* Check number of parameters */

	if (np == 9)
	  {
	    /* Surface defined by three points, read coordinates */
	    
	    x1 = params[n++];
	    y1 = params[n++];
	    z1 = params[n++];
	    x2 = params[n++];
	    y2 = params[n++];
	    z2 = params[n++];
	    x3 = params[n++];
	    y3 = params[n++];
	    z3 = params[n++];
	    
	    /* Calculate coefficients */
	    
	    A = y2*z3 - y3*z2 - y1*(z3 - z2) + z1*(y3 - y2);
	    B = z2*x3 - z3*x2 - z1*(x3 - x2) + x1*(z3 - z2);
	    C = x2*y3 - x3*y2 - x1*(y3 - y2) + y1*(x3 - x2);
	    D = x1*(y2*z3 - y3*z2) - y1*(x2*z3 - x3*z2) + z1*(x2*y3 - x3*y2);

	    /* Check */
	    
	    if (fabs(A*x1 + B*y1 + C*z1 - D) > 1E-4)
	      Die(FUNCTION_NAME, "Error in coefficients (1): %E",
		  fabs(A*x1 + B*y1 + C*z1 - D));
	    
	    if (fabs(A*x2 + B*y2 + C*z2 - D) > 1E-4)
	      Die(FUNCTION_NAME, "Error in coefficients (2): %E",
		  fabs(A*x2 + B*y2 + C*z2 - D));
	    
	    if (fabs(A*x3 + B*y3 + C*z3 - D) > 1E-4)
	      Die(FUNCTION_NAME, "Error in coefficients (3): %E",
		  fabs(A*x3 + B*y3 + C*z3 - D));
	  }
	else
	  {
	    /* Surface defined by parameters, reset optional*/

	    B = 0.0;
	    C = 0.0;
	    D = 0.0;
	    
	    /* Get parameters */
	    
	    A = params[n++];
	    
	    if (n < np)
	      B = params[n++];
	    
	    if (n < np)
	      C = params[n++];
	    
	    if (n < np)
	      D = params[n++];
	  }

	/* Calculate first constant */

	M = A*u + B*v + C*w;

	/* Check */

	if (M == 0.0)
	  return INFTY;

	/* Calculate second constant */

	L = A*x + B*y + C*z - D;

	/* Calculate distance */

	d = -L/M;

	/* Check sign */

	if (d > 0.0)
	  return d;
	else
	  return INFTY;
      }
      
      /***********************************************************************/

    case SURF_QUADRATIC:
      {
	/* Reset optional parameters */

	B = 0.0;
	C = 0.0;
	D = 0.0;
	E = 0.0;
	F = 0.0;
	G = 0.0;
	H = 0.0;
	J = 0.0;
	K = 0.0;

	/* Get parameters */
	
	A = params[n++];

	if (n < np)
	  B = params[n++];

	if (n < np)
	  C = params[n++];
	
	if (n < np)
	  D = params[n++];

	if (n < np)
	  E = params[n++];

	if (n < np)
	  F = params[n++];

	if (n < np)
	  G = params[n++];

	if (n < np)
	  H = params[n++];

	if (n < np)
	  J = params[n++];

	if (n < np)
	  K = params[n++];

	/* Calculate constants */
	
	N = A*u*u + B*v*v + C*w*w + D*u*v + E*v*w + F*u*w;
	M = 2*A*u*x + 2*B*v*y + 2*C*w*z + D*(v*x + u*y) + E*(w*y + v*z) + 
	  F*(w*x + u*z) + G*u + H*v + J*w;
	L = A*x*x + B*y*y + C*z*z + D*x*y + E*y*z + F*x*z + G*x + H*y + J*z 
	  + K;

	/* Check number of roots */

	if (N == 0.0)
	  {
	    /* Single root */

	    if (M == 0.0)
	      return INFTY;
	    else if ((d = -L/M) < 0.0)
	      return INFTY;
	    else 
	      return d;
	  }

	/* Two roots */

	if ((S = M*M - 4 * N * L) < 0.0)
	  return INFTY;
	else
	  S = sqrt(S);
	
	/* Get solutions */
	
	d0 = (-M + S)/(2*N);
	d1 = (-M - S)/(2*N);
      
	/* Choose smallest positive */

	if ((d0 < 0.0) && (d1 < 0.0))
	  {
	    /* Both negative, no line-of-sight */
	    
	    return INFTY;
	  }
	else if ((d0 > 0.0) && (d1 > 0.0))
	  {
	    /* Both positive, pick shortest */

	    if (d0 > d1)
	      return d1;
	    else
	      return d0;
	  }
	else
	  {
	    /* One positive, one negative, pick positive */
	    
	    if (d0 > 0.0)
	      return d0;
	    else
	      return d1;
	  }
      }
      
      /***********************************************************************/
    
    case SURF_GCROSS:
      {
	/* Shift origin */
	
	x = x - params[n++];
	y = y - params[n++];

	/* Calculate minimum distance to x-planes */

	if (u != 0.0)
	  {
	    for (i = n; i < np; i++)
	      {
		if (((d = -(x - params[i])/u) > 0.0) && (d < min))
		  min = d;
		if (((d = -(x + params[i])/u) > 0.0) && (d < min))
		  min = d;
	      }
	  }

	/* Calculate minimum distance to y-planes */

	if (v != 0.0)
	  {
	    for (i = n; i < np; i++)
	      {
		if (((d = -(y - params[i])/v) > 0.0) && (d < min))
		  min = d;
		if (((d = -(y + params[i])/v) > 0.0) && (d < min))
		  min = d;
	      }
	  }

	/* Return distance */

	return min;
      }	
       
      /***********************************************************************/
      
    case SURF_PPD:
      {
	/* Coordinate transformation */
	
	x = x - params[n++];
	y = y - params[n++];
	z = z - params[n++];
	
	/* Lengths */
	
	La = params[n++];
	Lb = params[n++];
	Lc = params[n++];
	
	/* Angles */
	
	th = params[n++]*PI/180.0;
	ps = params[n++]*PI/180.0;
	ph = params[n++]*PI/180.0;

        /*Determine the maximum possible x, y, z values*/
        xmax =  La+(Lb+Lc*(sin(th)*sin(ph)/cos(ps)))*sin(ps)+Lc*(sin(th)*cos(ph));
        ymax =  Lb*cos(ps) + Lc*(sin(th)*sin(ph));
        zmax =  Lc*cos(th);

        /*Calculate the plane equations for surface 1 of the parallelepiped*/
        x1 = 0.0;
        y1 = 0.0;
        z1 = 0.0;
        x2 = La;
        y2 = 0.0;
        z2 = 0.0;
        x3 = La+(Lc*(sin(th)*sin(ph)/cos(ps)))*sin(ps)+Lc*(sin(th)*cos(ph));
        y3 = Lc*(sin(th)*sin(ph));
        z3 = zmax; 

        a1 = y2*z3 - y3*z2 - y1*(z3 - z2) + z1*(y3 - y2);
        b1 = z2*x3 - z3*x2 - z1*(x3 - x2) + x1*(z3 - z2);
        c1 = x2*y3 - x3*y2 - x1*(y3 - y2) + y1*(x3 - x2);
        d1 = -x1*(y2*z3 - y3*z2) + y1*(x2*z3 - x3*z2) - z1*(x2*y3 - x3*y2);

        /* Check Surface 1 Coefficients*/

        if (fabs(a1*x1 + b1*y1 + c1*z1 + d1) > 1E-6)
          Die(FUNCTION_NAME, "Error in coefficients (1) surface (1): %E",
            fabs(a1*x1 + b1*y1 + c1*z1 + d1));
  
        if (fabs(a1*x2 + b1*y2 + c1*z2 + d1) > 1E-6)
          Die(FUNCTION_NAME, "Error in coefficients (2) surface (1): %E",
            fabs(a1*x2 + b1*y2 + c1*z2 + d1));
    
        if (fabs(a1*x3 + b1*y3 + c1*z3 + d1) > 1E-6)
          Die(FUNCTION_NAME, "Error in coefficients (3) surface (1): %E",
            fabs(a1*x3 + b1*y3 + c1*z3 + d1));

       /*Calculate First Constant*/
	M = a1*u + b1*v + c1*w;

	/* Check */
	if (M != 0.0)
        {
	  /* Calculate second constant */
	  L = a1*x + b1*y + c1*z + d1;

	  /* Calculate distance */
	  d = -L/M;

	  /* Check sign */
          if (d > 0.0 && d < min)
	    min = d;
        } 

        /*Calculate the plane equations for surface 2 of the parallelepiped*/
        x1 = Lb*sin(ps);
        y1 = Lb*cos(ps);
        z1 = 0.0;
        x2 = La+Lb*sin(ps);
        y2 = Lb*cos(ps);
        z2 = 0.0;
        x3 = xmax;
        y3 = ymax;
        z3 = zmax;

        a2 = y2*z3 - y3*z2 - y1*(z3 - z2) + z1*(y3 - y2);
        b2 = z2*x3 - z3*x2 - z1*(x3 - x2) + x1*(z3 - z2);
        c2 = x2*y3 - x3*y2 - x1*(y3 - y2) + y1*(x3 - x2);
        d2 = -x1*(y2*z3 - y3*z2) + y1*(x2*z3 - x3*z2) - z1*(x2*y3 - x3*y2);

        /* Check Surface 2 Coefficients*/

        if (fabs(a2*x1 + b2*y1 + c2*z1 + d2) > 1E-6)
          Die(FUNCTION_NAME, "Error in coefficients (1) surface (1): %E",
            fabs(a2*x1 + b2*y1 + c2*z1 + d2));

        if (fabs(a2*x2 + b2*y2 + c2*z2 + d2) > 1E-6)
          Die(FUNCTION_NAME, "Error in coefficients (2) surface (1): %E",
            fabs(a2*x2 + b2*y2 + c2*z2 + d2));

        if (fabs(a2*x3 + b2*y3 + c2*z3 + d2) > 1E-6)
          Die(FUNCTION_NAME, "Error in coefficients (3) surface (1): %E",
            fabs(a2*x3 + b2*y3 + c2*z3 + d2));

       /*Calculate First Constant*/
        M = a2*u + b2*v + c2*w;

        /* Check */
        if (M != 0.0)
        {
          /* Calculate second constant */
          L = a2*x + b2*y + c2*z + d2;

          /* Calculate distance */
          d = -L/M;

          /* Check sign */
          if (d > 0.0 && d < min)
            min = d;
        }


        /*Calculate the plane equations for surface 2 of the parallelepiped*/
        x1 = 0.0;
        y1 = 0.0;
        z1 = 0.0;
        x2 = (Lc*(sin(th)*sin(ph)/cos(ps)))*sin(ps)+Lc*(sin(th)*cos(ph));
        y2 = Lc*(sin(th)*sin(ph));
        z2 = zmax;
        x3 = Lb*sin(ps);
        y3 = Lb*cos(ps);
        z3 = 0.0;

        a3 = y2*z3 - y3*z2 - y1*(z3 - z2) + z1*(y3 - y2);
        b3 = z2*x3 - z3*x2 - z1*(x3 - x2) + x1*(z3 - z2);
        c3 = x2*y3 - x3*y2 - x1*(y3 - y2) + y1*(x3 - x2);
        d3 = -x1*(y2*z3 - y3*z2) + y1*(x2*z3 - x3*z2) - z1*(x2*y3 - x3*y2);

        /* Check Surface 3 Coefficients*/

        if (fabs(a3*x1 + b3*y1 + c3*z1 + d3) > 1E-6)
          Die(FUNCTION_NAME, "Error in coefficients (1) surface (1): %E",
            fabs(a3*x1 + b3*y1 + c3*z1 + d3));

        if (fabs(a3*x2 + b3*y2 + c3*z2 + d3) > 1E-6)
          Die(FUNCTION_NAME, "Error in coefficients (2) surface (1): %E",
            fabs(a3*x2 + b3*y2 + c3*z2 + d3));

        if (fabs(a3*x3 + b3*y3 + c3*z3 + d3) > 1E-6)
          Die(FUNCTION_NAME, "Error in coefficients (3) surface (1): %E",
            fabs(a3*x3 + b3*y3 + c3*z3 + d3));

       /*Calculate First Constant*/
        M = a3*u + b3*v + c3*w;

        /* Check */
        if (M != 0.0)
        {
          /* Calculate second constant */
          L = a3*x + b3*y + c3*z + d3;

          /* Calculate distance */
          d = -L/M;

          /* Check sign */
          if (d > 0.0 && d < min)
            min = d;
        }


        /*Calculate the plane equations for surface 4 of the parallelepiped*/
        x1 = La;
        y1 = 0.0;
        z1 = 0.0;
        x2 = La+(Lc*(sin(th)*sin(ph)/cos(ps)))*sin(ps)+Lc*(sin(th)*cos(ph));
        y2 = Lc*(sin(th)*sin(ph));
        z2 = zmax;
        x3 = La+Lb*sin(ps);
        y3 = Lb*cos(ps);
        z3 = 0.0;

        a4 = y2*z3 - y3*z2 - y1*(z3 - z2) + z1*(y3 - y2);
        b4 = z2*x3 - z3*x2 - z1*(x3 - x2) + x1*(z3 - z2);
        c4 = x2*y3 - x3*y2 - x1*(y3 - y2) + y1*(x3 - x2);
        d4 = -x1*(y2*z3 - y3*z2) + y1*(x2*z3 - x3*z2) - z1*(x2*y3 - x3*y2);      

        /* Check Surface 4 Coefficients*/

        if (fabs(a4*x1 + b4*y1 + c4*z1 + d4) > 1E-6)
          Die(FUNCTION_NAME, "Error in coefficients (1) surface (1): %E",
            fabs(a4*x1 + b4*y1 + c4*z1 + d4));

        if (fabs(a4*x2 + b4*y2 + c4*z2 + d4) > 1E-6)
          Die(FUNCTION_NAME, "Error in coefficients (2) surface (1): %E",
            fabs(a4*x2 + b4*y2 + c4*z2 + d4));

        if (fabs(a4*x3 + b4*y3 + c4*z3 + d4) > 1E-6)
          Die(FUNCTION_NAME, "Error in coefficients (3) surface (1): %E",
            fabs(a4*x3 + b4*y3 + c4*z3 + d4));

       /*Calculate First Constant*/
        M = a4*u + b4*v + c4*w;

        /* Check */
        if (M != 0.0)
        {
          /* Calculate second constant */
          L = a4*x + b4*y + c4*z + d4;

          /* Calculate distance */
          d = -L/M;

          /* Check sign */
          if (d > 0.0 && d < min)
            min = d;
        }


        /*Calculate the plane equations for surface 5 of the parallelepiped*/
        x1 = 0.0;
        y1 = 0.0;
        z1 = 0.0;
        x2 = La;
        y2 = 0.0;
        z2 = 0.0;
        x3 = La+Lb*sin(ps);
        y3 = Lb*cos(ps);
        z3 = 0.0;

        a5 = y2*z3 - y3*z2 - y1*(z3 - z2) + z1*(y3 - y2);
        b5 = z2*x3 - z3*x2 - z1*(x3 - x2) + x1*(z3 - z2);
        c5 = x2*y3 - x3*y2 - x1*(y3 - y2) + y1*(x3 - x2);
        d5 = -x1*(y2*z3 - y3*z2) + y1*(x2*z3 - x3*z2) - z1*(x2*y3 - x3*y2);

        /* Check Surface 5 Coefficients*/

        if (fabs(a5*x1 + b5*y1 + c5*z1 + d5) > 1E-6)
          Die(FUNCTION_NAME, "Error in coefficients (1) surface (1): %E",
            fabs(a5*x1 + b5*y1 + c5*z1 + d5));

        if (fabs(a5*x2 + b5*y2 + c5*z2 + d5) > 1E-6)
          Die(FUNCTION_NAME, "Error in coefficients (2) surface (1): %E",
            fabs(a5*x2 + b5*y2 + c5*z2 + d5));

        if (fabs(a5*x3 + b5*y3 + c5*z3 + d5) > 1E-6)
          Die(FUNCTION_NAME, "Error in coefficients (3) surface (1): %E",
            fabs(a5*x3 + b5*y3 + c5*z3 + d5));

       /*Calculate First Constant*/
        M = a5*u + b5*v + c5*w;

        /* Check */
        if (M != 0.0)
        {
          /* Calculate second constant */
          L = a5*x + b5*y + c5*z + d5;

          /* Calculate distance */
          d = -L/M;

          /* Check sign */
          if (d > 0.0 && d < min)
            min = d;
        }


        /*Calculate the plane equations for surface 6 of the parallelepiped*/
        x1 = (Lc*(sin(th)*sin(ph)/cos(ps)))*sin(ps)+Lc*(sin(th)*cos(ph));
        y1 = Lc*(sin(th)*sin(ph));
        z1 = zmax;
        x2 = La+(Lc*(sin(th)*sin(ph)/cos(ps)))*sin(ps)+Lc*(sin(th)*cos(ph));
        y2 = Lc*(sin(th)*sin(ph));
        z2 = zmax;
        x3 = xmax;
        y3 = ymax;
        z3 = zmax;

        a6 = y2*z3 - y3*z2 - y1*(z3 - z2) + z1*(y3 - y2);
        b6 = z2*x3 - z3*x2 - z1*(x3 - x2) + x1*(z3 - z2);
        c6 = x2*y3 - x3*y2 - x1*(y3 - y2) + y1*(x3 - x2);
        d6 = -x1*(y2*z3 - y3*z2) + y1*(x2*z3 - x3*z2) - z1*(x2*y3 - x3*y2);

        /* Check Surface 6 Coefficients*/

        if (fabs(a6*x1 + b6*y1 + c6*z1 + d6) > 1E-6)
          Die(FUNCTION_NAME, "Error in coefficients (1) surface (1): %E",
              fabs(a6*x1 + b6*y1 + c6*z1 + d6));

        if (fabs(a6*x2 + b6*y2 + c6*z2 + d6) > 1E-6)
          Die(FUNCTION_NAME, "Error in coefficients (2) surface (1): %E",
            fabs(a6*x2 + b6*y2 + c6*z2 + d6));

        if (fabs(a6*x3 + b6*y3 + c6*z3 + d6) > 1E-6)
          Die(FUNCTION_NAME, "Error in coefficients (3) surface (1): %E",
            fabs(a6*x3 + b6*y3 + c6*z3 + d6));

       /*Calculate First Constant*/
        M = a6*u + b6*v + c6*w;

        /* Check */
        if (M != 0.0)
        {
          /* Calculate second constant */
          L = a6*x + b6*y + c6*z + d6;

          /* Calculate distance */
          d = -L/M;

          /* Check sign */
          if (d > 0.0 && d < min)
            min = d;
        }	
	
	/* Return distance */
	
	return min;

      }

      /**********************************************************************/
      
    case SURF_USER:
      {
	/* User-defined surface, call special routine */
	
	UserSurf(3, np, params, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		 &min, x, y, z,  u, v, w);
	
	return min;
      }
      
      /**********************************************************************/
      
    default:
      Die(FUNCTION_NAME, "Surface type %d not supported", type);	
      
      /***********************************************************************/
    }
  
  /* Something wrong */
  
  fprintf(err, "%s WTF?\n", FUNCTION_NAME);

  exit(-1);
}

/****************************************************************************/
