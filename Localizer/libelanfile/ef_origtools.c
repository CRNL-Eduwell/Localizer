/*--------------------------------------------------*/
/* Tools for original ELAN files module.            */
/* v: 1.00    Aut.: PEA                             */
/* CRNL / INSERM U1028 / CNRS UMR 5292 / Univ-Lyon1 */
/*--------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "elanfile.h"
#include "ef_origcommon.h"


/* Local funtion prototypes. */
/*---------------------------*/
void swapbyte(void *from, size_t size);


/* External function prototypes. */
/*-------------------------------*/
extern int ef_test_suffix(char *filename, char *suffix); /* Defined in ef_tools.c */


/**********************************************/
/* ef_orig_test_data: test data type of file. */
/**********************************************/
void ef_orig_test_data(char *filename, elan_struct_t *elan_struct)
{
	if ((ef_test_suffix(filename, ".eeg") == 0) || (ef_test_suffix(filename, ".EEG") == 0))
	{
		elan_struct->has_eeg = EF_YES;
	}
	if ((ef_test_suffix(filename, ".p") == 0) || (ef_test_suffix(filename, ".P") == 0))
	{
		elan_struct->has_ep = EF_YES;
	}
	if ((ef_test_suffix(filename, ".tf") == 0) || (ef_test_suffix(filename, ".TF") == 0))
	{
		elan_struct->has_tf = EF_YES;
	}
}



/**************************************************************************/
/* ef_orig_init_struct: structure initialization for original data files. */
/**************************************************************************/
void ef_orig_init_struct(elan_struct_t *elan_struct)
{
	/* Initialize header for original data. */
	elan_struct->version = HIST_VERSION;
	elan_struct->release = 0;
	elan_struct->endian = EF_BIG_ENDIAN;
	
	elan_struct->has_eeg = EF_NO;
	elan_struct->eeg.data_type = EF_DATA_FLOAT;
	elan_struct->has_ep = EF_NO;
	elan_struct->ep.data_type = EF_DATA_FLOAT;
	elan_struct->has_tf = EF_NO;
	elan_struct->tf.data_type = EF_DATA_FLOAT;
	
	elan_struct->orig_info = (orig_info_t *)calloc(1, sizeof(orig_info_t));
	ELAN_CHECK_ERROR_ALLOC(elan_struct->orig_info, "for storing original file format informations.");

	elan_struct->orig_info->has_eeg_info = EF_NO;
	elan_struct->orig_info->eeg_info.bufReadPtr = NULL;
	elan_struct->orig_info->eeg_info.bufReadSize = 0;

	elan_struct->orig_info->has_ep_info = EF_NO;
	elan_struct->orig_info->ep_info.bufReadPtr = NULL;
	elan_struct->orig_info->ep_info.bufReadSize = 0;

	elan_struct->orig_info->has_tf_info = EF_NO;
	elan_struct->orig_info->tf_info.offset_data = 0;
	elan_struct->orig_info->tf_info.bufReadPtr = NULL;
	elan_struct->orig_info->tf_info.bufReadSize = 0;
}


/******************************************************************/
/* ef_orig_read_elecdat: read elec.dat coordinates database file. */
/******************************************************************/
void ef_orig_read_elecdat(int *coord_nb, elec_dat_t **coord_list)
{
	char elec_filename[COMMENT_SIZE], dumstr[COMMENT_SIZE];
	elec_dat_t *ptr_coord_list;
	FILE *elec_file;
	int jread, dumi, elec_nb;
	float dumf;
	
	SetElanPath;
	strcpy(elec_filename, ELAN_ELEC_PATH);
	strcat(elec_filename, "elec.dat");
	elec_file = fopen(elec_filename,"rt");
	if (elec_file == NULL)
	{
		fprintf(stderr, "ERROR : file %s not found.\n", elec_filename);

		/* Return a NULL array of coordinates. */
		*coord_nb = 0;
		*coord_list = NULL;
		
		return;
	}
	
	/* Count number of lines ie number of coordinates. */
	jread=1;
	elec_nb = 0;
	while (jread > 0)
	{
		jread = fscanf(elec_file,"%d %f %f %s",&dumi, &dumf, &dumf, dumstr);
		if (jread > 0)
		{
			elec_nb++;
		}
	}
	
	/* Actually read file. */
	ptr_coord_list = (elec_dat_t *) calloc(elec_nb, sizeof(elec_dat_t));
	ELAN_CHECK_ERROR_ALLOC(ptr_coord_list, "for storing electrode coordinates database (elec.dat).");
	*coord_list = ptr_coord_list;

	fseek(elec_file, 0, SEEK_SET);
	elec_nb=0;
	jread = fscanf(elec_file,"%d %f %f %s",&ptr_coord_list[elec_nb].ind, &ptr_coord_list[elec_nb].theta, &ptr_coord_list[elec_nb].phi, ptr_coord_list[elec_nb].lab);
	while (jread > 0)
		{
		elec_nb++;
		jread = fscanf(elec_file,"%d %f %f %s",&ptr_coord_list[elec_nb].ind, &ptr_coord_list[elec_nb].theta, &ptr_coord_list[elec_nb].phi, ptr_coord_list[elec_nb].lab);
		}
	elec_nb--;
	fclose(elec_file);
	
	*coord_nb = elec_nb;
}


/****************************************************/
/* ef_orig_free_elecdat: free elec.dat coordinates. */
/****************************************************/
void ef_orig_free_elecdat(elec_dat_t **coord_list)
{
	if (*coord_list != NULL)
	{
		free(*coord_list);
		*coord_list = NULL;
	}
}


/**************************************************/
/* swapbyte: swap byte of buffer pointed by from. */
/*           size must be even.                   */
/**************************************************/
void swapbyte(void *from, size_t size)
{
	char *p, *pw, c;
	int i;
	
	p = (char *)from;
	pw = (char *)from + size;
	for(i=0; i<size/2; i++)
	{
		c = *p;
		*(p++)  = *(--pw);
		*pw = c;
	}
}


/************************************************************************/
/*  freadswab : read data from a file and change byte order.            */
/************************************************************************/
size_t freadswab(void *ptr, size_t size, size_t nitems, FILE *stream)
{
  size_t cr;
  int32_t i;

  cr = fread(ptr, size, nitems, stream);
	for (i=0; i<cr; i++)
	{
		swapbyte((char *)ptr+(i*size), size);
	}
  return(cr);
}


