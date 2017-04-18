/*--------------------------------------------------*/
/* Reading original EP ELAN files module.           */
/* v: 1.00    Aut.: PEA                             */
/* CRNL / INSERM U1028 / CNRS UMR 5292 / Univ-Lyon1 */
/*--------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "elanfile.h"
#include "ef_origcommon.h"



/* Local function prototypes. */
/*----------------------------*/
size_t fortranfread(void *ptr, size_t size, size_t nitems, FILE *stream);


/* External function prototypes. */
/*-------------------------------*/
extern void ef_orig_read_elecdat(int *coord_nb, elec_dat_t **coord_list); /* Defined in ef_origtools.c */
extern void ef_orig_free_elecdat(elec_dat_t **coord_list); /* Defined in ef_origtools.c */
extern void swapbyte(void *from, size_t size); /* Defined in ef_origtools.c */
extern size_t freadswab(void *ptr, size_t size, size_t nitems, FILE *stream); /* Defined in ef_origtools.c */



/*----------------------------------------------------------------------------------------------------------*/
/*                                                                                                          */
/* ef_readepheader: reader header part of a .p file (original format) and fill a structure.                 */
/*                                                                                                          */
/*----------------------------------------------------------------------------------------------------------*/
int ef_readepheader(char *ep_filename, elan_struct_t *elan_struct)
{
	FILE *fp;
	int elec_dat_nb, icode, i_chan;
	float xelong, version, *hdr1_ptr, *hdr2_ptr;
	elec_dat_t *elec_dat_list;

	fp = fopen(ep_filename, "rb");
	if (fp == NULL) 
	{
		return(EF_ERR_OPEN_READ);
	}

	/* Reads 1st header. */
	/*-------------------*/
	hdr1_ptr = (float *)calloc(6, sizeof(float));
	ELAN_CHECK_ERROR_ALLOC(hdr1_ptr, "1st header (EP reading).");

	icode = fortranfread(hdr1_ptr, sizeof(float), 6, fp);
	if (icode != 6)
		{
		fclose(fp);
		free(hdr1_ptr);
		fprintf(stderr, "ERROR: reading original EP file format: 1st header length (%d, should be %d).\n", icode, 6);
		return(EF_ERR_READ_HEADER);
		}
	version = hdr1_ptr[0];
	xelong  = hdr1_ptr[1];

	/* Tests EP version */
	/*------------------*/
	if (version != -1.0 && version !=-3.0)
		{
		/* Unknown version. */
		/*------------------*/
		fclose(fp);
		free(hdr1_ptr);
		fprintf(stderr, "ERROR: reading original EP file format: unsupported version=%f\n", version);
		return(EF_ERR_READ_HEADER);
		}

	/* Reads 2nd header (length=xelong bytes). */
	/*-----------------------------------------*/
	hdr2_ptr = (float *)malloc(xelong);
	ELAN_CHECK_ERROR_ALLOC(hdr2_ptr, "2nd header (EP reading).");

	icode = fortranfread(hdr2_ptr, sizeof(float), (int)(xelong/sizeof(float)), fp);
	if (icode != (int)(xelong/sizeof(float)))
		{
		fclose(fp);
		free(hdr1_ptr);
		free(hdr2_ptr);
		fprintf(stderr, "ERROR: reading original EP file format: 2nd header length (%d, should be %d).\n", icode, (int)(xelong/sizeof(float)));
		return(EF_ERR_READ_HEADER);
		}
	
	elan_struct->orig_info->ep_info.offset_data = ftell(fp);

	fclose(fp);

	elan_struct->chan_nb = (int)hdr2_ptr[0];
	ef_alloc_channel_list(elan_struct);
	elan_struct->ep.samp_nb = (int)hdr2_ptr[1];
	elan_struct->ep.sampling_freq = hdr2_ptr[4]*1000.0;
	elan_struct->ep.prestim_samp_nb = (int)hdr2_ptr[3];
	elan_struct->ep.event_code = (int)hdr1_ptr[2];
	elan_struct->ep.event_nb = (int)hdr2_ptr[10+(4*elan_struct->chan_nb)];
	ef_alloc_ep_array_otherevents(elan_struct);
	elan_struct->ep.other_events_nb[0] = 0;
	ef_alloc_ep_array_otherevents_list(elan_struct);

	/* Channel coordinates and labels list. */
	/*--------------------------------------*/
	/* Read electrode coordinates database file. */
	elec_dat_nb = 0;
	elec_dat_list = NULL;
	ef_orig_read_elecdat(&elec_dat_nb, &elec_dat_list);
	for (i_chan=0; i_chan<elan_struct->chan_nb; i_chan++)
	{
		if (((int)hdr2_ptr[10+i_chan] > 0) && ((int)hdr2_ptr[10+i_chan] < elec_dat_nb))
		{
			/* Label found. */
			strcpy(elan_struct->chan_list[i_chan].lab, elec_dat_list[(int)hdr2_ptr[10+i_chan]-1].lab);
			/* Spherical coordinates found. */
			elan_struct->chan_list[i_chan].coord_nb = 1;
			elan_struct->chan_list[i_chan].coord_list = (coord_t *)calloc(elan_struct->chan_list[i_chan].coord_nb, sizeof(coord_t));
			ELAN_CHECK_ERROR_ALLOC(elan_struct->chan_list[i_chan].coord_list, "for storing channel #%d coordinates.", i_chan+1);
			
			strcpy(elan_struct->chan_list[i_chan].coord_list[0].lab, CoordSystemLabelList[CoordSystemSphericalHead]);
			elan_struct->chan_list[i_chan].coord_list[0].val_nb = 3;
			elan_struct->chan_list[i_chan].coord_list[0].val_list = (float *)calloc(elan_struct->chan_list[i_chan].coord_list[0].val_nb, sizeof(float));
			ELAN_CHECK_ERROR_ALLOC(elan_struct->chan_list[i_chan].coord_list[0].val_list, "for storing channel #%d coordinates (%d values).", i_chan+1, elan_struct->chan_list[i_chan].coord_list[0].val_nb);
			elan_struct->chan_list[i_chan].coord_list[0].val_list[0] = 90;
			elan_struct->chan_list[i_chan].coord_list[0].val_list[1] = elec_dat_list[(int)hdr2_ptr[10+i_chan]-1].theta;
			elan_struct->chan_list[i_chan].coord_list[0].val_list[2] = elec_dat_list[(int)hdr2_ptr[10+i_chan]-1].phi;
		}
		else
		{
			/* number not found. */
			elan_struct->chan_list[i_chan].coord_nb = 0;
			elan_struct->chan_list[i_chan].coord_list = NULL;
			strcpy(elan_struct->chan_list[i_chan].lab, "");
		}
		strcpy(elan_struct->chan_list[i_chan].type, ChannelTypeLabelList[ChannelTypeEEG]);
		strcpy(elan_struct->chan_list[i_chan].unit, "microV");
	}

	/* Free allocated memory. */
	/*------------------------*/
	free(hdr1_ptr);
	free(hdr2_ptr);
	ef_orig_free_elecdat(&elec_dat_list);

	return(EF_NOERR);
	
}


/****************************************************************************************************************/
/* ef_orig_read_EP_data_sampblock_one_chan: read an EP data block for one channel and store in data_ptr.        */
/* char *filename: file to read from.                                                                           */
/* elan_struct_t *elan_struct: Elan structure (read from header).                                               */
/* int measureNum: indice of measure to read (starting from 0).                                                 */
/* int chanNum: indice of channel (starting from 0).                                                            */
/* int sampStart: indice of first sample to read (starting from 0).                                             */
/* int sampNb: number of samples to read.                                                                       */
/* void **data_ptr: array of samples to store data. If NULL, data_ptr is allocated in function (must be freed). */
/*                 If allocated in caller, size must be sampNb float or double.                                 */
/* return value: number of samples actually read.                                                               */
/****************************************************************************************************************/
int ef_orig_read_EP_data_sampblock_one_chan(char *filename, elan_struct_t *elan_struct, int chanNum, int sampStart, int sampNb, void **data_ptr)
{
	FILE *data_f;
	int sampSize, i_samp, sampRead;
	float *bufFlt, *flt_ptr=NULL;
	double *dbl_ptr=NULL;
	
	if (elan_struct->has_ep == EF_YES)
	{
		if (*data_ptr == NULL)
		{
			switch (elan_struct->ep.data_type)
			{
				case EF_DATA_FLOAT:
					flt_ptr = (float *)ef_alloc_array_1d(sampNb, sizeof(float));
					*data_ptr = (void *)flt_ptr;
					break;
				case EF_DATA_DOUBLE:
					dbl_ptr = (double *)ef_alloc_array_1d(sampNb, sizeof(double));
					*data_ptr = (void *)dbl_ptr;
					break;
			}
		}
		else
		{
			switch (elan_struct->ep.data_type)
			{
				case EF_DATA_FLOAT:
					flt_ptr = *data_ptr;
					break;
				case EF_DATA_DOUBLE:
					dbl_ptr = *data_ptr;
					break;
			}
		}
		
		/* Read EP data file. */
		/*--------------------*/
		data_f = fopen(filename, "rb");
		if (data_f == NULL)
		{
			fprintf(stderr, "ERROR: can't open file %s (data reading).\n", filename);
			return(EF_ERR_OPEN_READ);
		}
		sampSize = sizeof(float);
		if ((elan_struct->ep.samp_nb*sampSize) != elan_struct->orig_info->ep_info.bufReadSize)
		{
			elan_struct->orig_info->ep_info.bufReadSize = elan_struct->ep.samp_nb*sampSize;
			elan_struct->orig_info->ep_info.bufReadPtr = (void *)realloc(elan_struct->orig_info->ep_info.bufReadPtr, elan_struct->orig_info->ep_info.bufReadSize);
			ELAN_CHECK_ERROR_ALLOC(elan_struct->orig_info->ep_info.bufReadPtr, "for data buffer (EP reading).");
		}
		bufFlt = (float *)elan_struct->orig_info->ep_info.bufReadPtr;

		/* Read data for one channel. */
		/*----------------------------*/
		fseek(data_f, elan_struct->orig_info->ep_info.offset_data+(((chanNum*(elan_struct->ep.samp_nb+2))+(1+sampStart))*sampSize), SEEK_SET);
		
		sampRead = freadswab(bufFlt, sampSize, sampNb, data_f);
		switch (elan_struct->ep.data_type)
		{
			case EF_DATA_FLOAT:
				for (i_samp=0; i_samp<sampRead; i_samp++)
				{
					flt_ptr[i_samp]= bufFlt[i_samp];
				}
			break;
			case EF_DATA_DOUBLE:
				for (i_samp=0; i_samp<sampRead; i_samp++)
				{
					dbl_ptr[i_samp]= bufFlt[i_samp];
				}
			break;
		}

		fclose(data_f);
		return(sampRead);
	}
	
	return(0);
}


/*****************************************************************************************************************/
/* ef_orig_read_EP_data_sampblock_all_chan: read an EP data block for all channel and store in data_ptr.        */
/* char *filename: file to read from.                                                                            */
/* elan_struct_t *elan_struct: Elan structure (read from header).                                                */
/* int measureNum: indice of measure to read (starting from 0).                                                  */
/* int sampStart: indice of first sample to read (starting from 0).                                              */
/* int sampNb: number of samples to read.                                                                        */
/* void ***data_ptr: array of samples to store data. If NULL, data_ptr is allocated in function (must be freed). */
/*                 If allocated in caller, size must be [elan_struct.chan_nbxsampNb] float or double.            */
/* return value: number of samples actually read.                                                                */
/*****************************************************************************************************************/
int ef_orig_read_EP_data_sampblock_all_chan(char *filename, elan_struct_t *elan_struct, int sampStart, int sampNb, void **data_ptr)
{
	FILE *data_f;
	int sampSize, i_samp, i_chan, sampRead=0;
	float *bufFlt, **flt_ptr=NULL;
	double **dbl_ptr=NULL;
	
	if (elan_struct->has_ep == EF_YES)
	{
		if (*data_ptr == NULL)
		{
			switch (elan_struct->ep.data_type)
			{
				case EF_DATA_FLOAT:
					flt_ptr = (float **)ef_alloc_array_2d(elan_struct->chan_nb, sampNb, sizeof(float));
					ELAN_CHECK_ERROR_ALLOC(flt_ptr, "for reading samples of all channels of continuous EEG (data type float).");
					*data_ptr = (void *)flt_ptr;
					break;
				case EF_DATA_DOUBLE:
					dbl_ptr = (double **)ef_alloc_array_2d(elan_struct->chan_nb, sampNb, sizeof(double));
					ELAN_CHECK_ERROR_ALLOC(dbl_ptr, "for reading samples of all channels of continuous EEG (data type double).");
					*data_ptr = (void *)dbl_ptr;
					break;
			}
		}
		else
		{
			switch (elan_struct->ep.data_type)
			{
				case EF_DATA_FLOAT:
					flt_ptr = (float **)*data_ptr;
					break;
				case EF_DATA_DOUBLE:
					dbl_ptr = (double **)*data_ptr;
					break;
			}
		}
	
		/* Read EP data file. */
		/*--------------------*/
		data_f = fopen(filename, "rb");
		if (data_f == NULL)
		{
			fprintf(stderr, "ERROR: can't open file %s (data reading).\n", filename);
			return(EF_ERR_OPEN_READ);
		}
		sampSize = sizeof(float);
		if ((elan_struct->ep.samp_nb*sampSize) != elan_struct->orig_info->ep_info.bufReadSize)
		{
			elan_struct->orig_info->ep_info.bufReadSize = elan_struct->ep.samp_nb*sampSize;
			elan_struct->orig_info->ep_info.bufReadPtr = (void *)realloc(elan_struct->orig_info->ep_info.bufReadPtr, elan_struct->orig_info->ep_info.bufReadSize);
			ELAN_CHECK_ERROR_ALLOC(elan_struct->orig_info->ep_info.bufReadPtr, "for data buffer (EP reading).");
		}
		bufFlt = (float *)elan_struct->orig_info->ep_info.bufReadPtr;

		/* Read data. */
		/*------------*/
		for (i_chan =0; i_chan<elan_struct->chan_nb; i_chan++)
		{
			fseek(data_f, elan_struct->orig_info->ep_info.offset_data+(((i_chan*(elan_struct->ep.samp_nb+2))+(1+sampStart))*sampSize), SEEK_SET);
			
			sampRead = freadswab(bufFlt, sampSize, sampNb, data_f);
			switch (elan_struct->ep.data_type)
			{
				case EF_DATA_FLOAT:
					for (i_samp=0; i_samp<sampRead; i_samp++)
					{
						flt_ptr[i_chan][i_samp] = bufFlt[i_samp];
					}
				break;
				case EF_DATA_DOUBLE:
					for (i_samp=0; i_samp<sampRead; i_samp++)
					{
						dbl_ptr[i_chan][i_samp] = (double)bufFlt[i_samp];
					}
				break;
			}
		}
			
		fclose(data_f);
		return(sampRead);
	}
	else
	{
		return(0);
	}
}


/************************************************************************/
/*  fortranfread.c	sous programme de lecture de record sequentiel non	*/
/*		formate compatible avec ceux du fortran 77		*/
/*									*/
/*  appel : cr = fortranfread(ptr,size,nitems,stream);			*/
/*									*/
/*  definition des arguments:						*/
/*	ptr:								*/
/*		pointeur sur le vecteur caractere a ecrire 		*/
/*	size:								*/
/*		taille en octet des items a ecrire         		*/
/*	nitems:								*/
/*		nombre d'items a ecire                     		*/
/*	stream:								*/
/*		descripteur du fichier a lire           		*/
/*									*/
/*  type des variables:							*/
/*	ptr:	pointeur sur caractere					*/
/*	size:	long							*/
/*	nitems:	long							*/
/*	stream:	FILE							*/
/*									*/
/*  type de la fonction: size_t						*/
/*									*/
/*  retourne -1 s'il y a imcompatibilite entre le nombre d'items        */
/*   demandes et lus                                                    */
/************************************************************************/
size_t fortranfread(void *ptr, size_t size, size_t nitems, FILE *stream)
{
  size_t cr;
  int32_t ptr_len, i;

  fread(&ptr_len, 4, 1, stream);
	/* Due to Fortran problems, do not check anymore (PEA : 03-05-2011) */
/*	if ( ptr_len != size * nitems ) return(-1); */
  cr = fread(ptr, size, nitems, stream);
	for (i=0; i<nitems; i++)
	{
		swapbyte((char *)ptr+(i*size), size);
	}
  fread(&ptr_len, 4, 1, stream);
  return(cr);
}



