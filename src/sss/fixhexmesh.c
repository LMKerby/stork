#ifdef __cplusplus 
extern "C" { 
#endif 
/*****************************************************************************/
/*                                                                           */
/* serpent 2 (beta-version) : fixhexmesh.c                                   */
/*                                                                           */
/* Created:       2015/03/27 (VVa)                                           */
/* Last modified: 2015/06/30 (VVa)                                           */
/* Version:       2.1.25                                                     */
/*                                                                           */
/* Description: Splits hexahedral meshes to tetrahedrons                     */
/*                                                                           */
/* Comments:                                                                 */
/*             -Based on "J. Dompierre, P. Labbe, M. Vallet and R. Camarero, */
/*              How to Subdivide Pyramids, Prisms, and Hexahedra             */ 
/*              into Tetrahedra, Proceedings,                                */
/*              8th International Meshing Roundtable,                        */
/*              South Lake Tahoe, CA, U.S.A., pp.195-204, October 1999       */
/*                            3---5                                          */
/*             7-----6        |\ /|                                          */
/*            /|    /|        | 4 |           4                              */
/*           4-----5 |        | | |          /|\                             */
/*           | 3---|-2        0-|-2        3-----2                           */
/*           |/    |/          \|/        /     /                            */
/*           0-----1            1        0-----1                             */
/*                                                                           */
/*****************************************************************************/

#include "header.h"
#include "locations.h"

#define FUNCTION_NAME "FixHexMesh:"

/*****************************************************************************/

void FixHexMesh(long ifc, long nfaces)
{
  long cgns, sdone;
  long ptr, surflist, cellpts, surf, type;
  long ownrlist, nbrlist, *ownrlist2, *nbrlist2;
  long hexfaces[6], hexsides[6], hexnbrs[2][6];
  long nf, nc, i, j, k; 
  long V[8], initFaces[6][4];
  long diags[8][8];

  /* Get number of parent cells */

  nc = (long)RDB[ifc + IFC_NC_PRNTS];

  /* Get number of parent faces */

  nf = (long)RDB[ifc + IFC_NF_PRNTS];
  
  /* Get pointer to parents surfacelist */

  surflist = (long)RDB[ifc + IFC_PTR_SURF_LIST_PRNTS];
  CheckPointer(FUNCTION_NAME, "(surflist)", DATA_ARRAY, surflist);

  /* Get pointer to owner list */

  ownrlist = (long)RDB[ifc + IFC_PTR_OWNR_LIST_PRNTS];
  CheckPointer(FUNCTION_NAME, "(ownrlist)", DATA_ARRAY, ownrlist);

  /* Get pointer to neighbour list */

  nbrlist  = (long)RDB[ifc + IFC_PTR_NBR_LIST_PRNTS];
  CheckPointer(FUNCTION_NAME, "(nbrlist)", DATA_ARRAY, nbrlist);

  /* Allocate memory for temporary second owner list */

  ownrlist2 = (long *)Mem(MEM_ALLOC, nf, sizeof(long));

  /* Allocate memory for temporary second neighbour list */

  nbrlist2 = (long *)Mem(MEM_ALLOC, nf, sizeof(long));

  /* Populate second owner and neighbour lists              */
  /* Since each 4 point face will be split to two triangles */
  /* Each initial face will be having two new neighbours    */
  /* Hence the two lists                                    */

  for (i = 0; i < nf; i++)
    {
      ownrlist2[i] = (long)RDB[ownrlist + i];
      nbrlist2[i] = (long)RDB[nbrlist + i];
    }

  /* Get pointer to tet list */

  cgns = (long)RDB[ifc + IFC_PTR_TET_MSH_PRNTS];
  CheckPointer(FUNCTION_NAME, "(cgns)", DATA_ARRAY, cgns);  

  /* Allocate memory for new owners and neighbors list    */
  /* The number of child faces has to be calculated first */
   
  /* Allocate memory for new owner list */
      
  ptr = ReallocMem(DATA_ARRAY, nfaces);
  WDB[ifc + IFC_PTR_OWNR_LIST] = (double)ptr;

  /* Allocate memory for new neighbour list */

  ptr = ReallocMem(DATA_ARRAY, nfaces);
  WDB[ifc + IFC_PTR_NBR_LIST] = (double)ptr;

  /* Loop over nbr/ownrlists to reset cells */

  for (i = 0; i < nfaces; i++)
    {
      /* Create new surface */

      surf = NewItem(ifc + IFC_PTR_SURF_LIST, SURFACE_BLOCK_SIZE);

      /* Allocate memory for parameters */
		  
      ptr = ReallocMem(DATA_ARRAY, 3);
      WDB[surf + SURFACE_PTR_PARAMS] = (double)ptr;

      /* Store number of parameters */

      WDB[surf + SURFACE_N_PARAMS] = (double)3.0;

    }

  /* Close new surface list */
  surf = (long)RDB[ifc + IFC_PTR_SURF_LIST];
  CloseList(surf);

  /* Reset number of created surfaces */

  sdone = 0;

  /****** Divide cells *******/

  /* Get pointer to tet list */

  cgns = (long)RDB[ifc + IFC_PTR_TET_MSH_PRNTS];
  CheckPointer(FUNCTION_NAME, "(cgns)", DATA_ARRAY, cgns);

  /* Loop over cells to prepare them for division */
 
  for (i = 0; i < nc; i++)
    {      

      /* Get pointer to cell */

      cgns = ListPtr(cgns, i);
      CheckPointer(FUNCTION_NAME, "(cgns)", DATA_ARRAY, cgns);
      
      /* Get mesh cell type */

      type = MeshCellType(cgns, ifc);

      /* Reset V */

      for (j = 0; j < 8; j++)
	V[j] = -1;

#ifdef SPLITPRINT
      printf("Cell type %ld\n", type);
#endif

      /* Create the point list V and auxiliary lists */

      MeshCellFromCGNS(ifc, cgns, V, hexfaces, hexsides, hexnbrs, initFaces,
		       ownrlist2, nbrlist2, type);
 
      /* Rotate the hexahedron so that the smallest index is bottom left, front */

      MeshCellIndirection(V, type);
	        
#ifdef SPLITPRINT
      PrintMeshCell(V, type);
#endif
	  
      /* Reset diagonals */

      for (j = 0; j < 8; j++)
	for (k = 0; k < 8; k++)
	  diags[j][k] = 0;

      /* Get diagonals */

      MeshCellConnectDiags(V, diags, type);

      /* Print connectivity */
#ifdef SPLITPRINT
	  
      for (j = 0; j < 8; j++)
	{
	  for (k = 0; k < 8; k++)
	    printf("%ld ", diags[j][k]);

	  printf("\n");
	}
	  
#endif

      /* Rotate along 0-6 diagonal if needed */
      
      if (type == MESH_CELL_TYPE_HEX)
	HexRotateCell(V, diags);

	  
#ifdef SPLITPRINT
      PrintMeshCell(V, type);
#endif	  
      /* Now we don't have to rotate the cell anymore       */
      /* We can map neighbours to the current top, bot etc. */

      MeshCellRotateLists(V, initFaces, hexfaces, 
			  hexsides, hexnbrs, type);

#ifdef SPLITPRINT
      
      printf("Neighbours:  ");

      for (j = 0; j < 6; j++)
	{
	  printf("%ld ", hexnbrs[0][j]);
	}

      printf("\n");

      printf("Neighbours2: ");

      for (j = 0; j < 6; j++)
	{
	  printf("%ld ", hexnbrs[1][j]);
	}

      printf("\n");

      printf("Faces: ");

      for (j = 0; j < 6; j++)
	{
	  printf("%ld ", hexfaces[j]);
	}
      printf("\n");

      printf("Points: ");

      for (j = 0; j < 8; j++)
	{
	  printf("%ld ", V[j]);
	}
      printf("\n");
#endif	  

      /* Create new cells and faces      */
      
      DivideMeshCell(ifc, cgns, V, diags, hexnbrs, hexfaces, 
		     hexsides, ownrlist2, nbrlist2, &sdone, type);

    }

  /* Get pointer to new tet list */

  cgns = (long)RDB[ifc + IFC_PTR_TET_MSH];
  CheckPointer(FUNCTION_NAME, "(cgns)", DATA_ARRAY, cgns);

  /* Close the new cgns list*/

  CloseList(cgns);

  /* Remove excess surfaces created from the list      */
  /* Number of these can be N_init_cells*2 at max      */
  /* TODO: Maybe some kind of optimization could avoid */
  /* creating these altogether */

  surf = (long)RDB[ifc + IFC_PTR_SURF_LIST];

  /* Get pointer to the last used surface */

  ptr = ListPtr(surf, sdone - 1);

  /* Loop over list and reset direct pointers */

  while (surf > VALID_PTR)
    {
      WDB[surf + LIST_PTR_DIRECT] = NULLPTR;
      surf = NextItem(surf);
    }

  /* Move to last used surface */

  surf = ptr;

  /* Get first unused surface */

  ptr = NextItem(surf);

  /* While there are unused surfaces after surf */
  /* Remove them */
  
  while (ptr > VALID_PTR)
    {
      /* Remove unused surface */

      RemoveItem(ptr);

      /* Get pointer to next unused */

      ptr = NextItem(surf);
    }

  /* Close surface list */

  CloseList(surf);

  /* Free the temporary owner and neighbour lists */

  Mem(MEM_FREE, ownrlist2);
  Mem(MEM_FREE, nbrlist2);

  /* Store number of child cells */

  WDB[ifc + IFC_NC] = (double)ListSize(cgns);

  /* Loop over new cell list to calculate cell centerpoints */

  nc = (long)RDB[ifc + IFC_NC];

  /* Allocate memory for cell centerpoints */

  cellpts = ReallocMem(DATA_ARRAY, nc*3);

  /* Store cell centerpoint list */

  WDB[ifc + IFC_PTR_CELL_CP_LIST] = (double)cellpts;

  /* Get pointer to new tet list */

  cgns = (long)RDB[ifc + IFC_PTR_TET_MSH];
  CheckPointer(FUNCTION_NAME, "(cgns)", DATA_ARRAY, cgns);

  /* Get pointer to surface list */

  surflist = (long)RDB[ifc + IFC_PTR_SURF_LIST];
  CheckPointer(FUNCTION_NAME, "(surflist)", DATA_ARRAY, surflist);

  /* Calculate cell centerpoints for new cells */

  CalculateTetCenter(cgns, surflist, cellpts, -1);

  return;
}

/*****************************************************************************/
#ifdef __cplusplus 
} 
#endif 
