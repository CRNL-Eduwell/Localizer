/*--------------------------------------------------*/
/* Tools for ELAN files module.                     */
/* v: 1.00    Aut.: PEA                             */
/* CRNL / INSERM U1028 / CNRS UMR 5292 / Univ-Lyon1 */
/*--------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "elanfile.h"
#include "ef_hdf5common.h"
#include "ef_origcommon.h"


#if defined(_WIN32) || defined(_WIN64)
	#define LITTLE_ENDIAN 0x41424344UL 
	#define BIG_ENDIAN    0x44434241UL 
	#define PDP_ENDIAN    0x42414443UL 
	#define ENDIAN_ORDER  ('ABCD')    
#endif

/* Global variables initialization. */
/*---------------------------------*/
str_label_t CoordSystemLabelList[]={"Spherical-Head"};
str_label_t ChannelTypeLabelList[]={"EEG"};


/* Local function prototypes. */
/*----------------------------*/
int ef_test_suffix(char *filename, char *suffix);
void ef_init_options(elan_struct_t *elan_struct);



/*--------------------------------------------------------------------------*/
/* ef_init_structure: default value initialization for Elan data structure. */
/*--------------------------------------------------------------------------*/
void ef_init_structure(elan_struct_t *elan_struct)
{
	ef_init_options(elan_struct); /* If not already done. */

        elan_struct->version = CURRENT_VERSION;
	elan_struct->release = CURRENT_RELEASE;
	elan_struct->endian = ef_get_mach_endian();
	elan_struct->eeg.data_type = EF_DATA_FLOAT;
	elan_struct->ep.data_type = EF_DATA_FLOAT;
	elan_struct->tf.data_type = EF_DATA_FLOAT;
        
        elan_struct->measure_channel_label_list = NULL;
        elan_struct->chan_list = NULL;
        
	elan_struct->has_eeg = EF_NO;
	elan_struct->has_ep = EF_NO;
	elan_struct->has_tf = EF_NO;
	
	elan_struct->orig_info = NULL;
}


/*--------------------------------------------------------------------------------*/
/* ef_set_structure_lastversion: set values of structure to save as last version. */
/*--------------------------------------------------------------------------------*/
void ef_set_structure_lastversion(elan_struct_t *elan_struct)
{
  elan_struct->version = CURRENT_VERSION;
	elan_struct->release = CURRENT_RELEASE;
	elan_struct->endian = ef_get_mach_endian();
	elan_struct->eeg.data_type = EF_DATA_FLOAT;
	elan_struct->ep.data_type = EF_DATA_FLOAT;
	elan_struct->tf.data_type = EF_DATA_FLOAT;
}


/*------------------------------------------------------------*/
/* ef_alloc_data_array: allocates array for all defined data. */
/*------------------------------------------------------------*/
void ef_alloc_data_array(elan_struct_t *elan_struct)
{
	int i_meas, i_chan, i_freq;
	
	if (elan_struct->has_eeg == EF_YES)
	{
		switch (elan_struct->eeg.data_type)
		{
			case EF_DATA_FLOAT:
				elan_struct->eeg.flat_float = (float *)calloc(elan_struct->measure_channel_nb*elan_struct->chan_nb*elan_struct->eeg.samp_nb, sizeof(float));
				ELAN_CHECK_ERROR_ALLOC(elan_struct->eeg.flat_float, "EEG flat data array of float.");

				elan_struct->eeg.data_float = (float ***)calloc(elan_struct->measure_channel_nb, sizeof(float **));
				ELAN_CHECK_ERROR_ALLOC(elan_struct->eeg.data_float, "EEG matrix data array of float (measure dimension).");
				for (i_meas=0; i_meas<elan_struct->measure_channel_nb; i_meas++)
				{
					elan_struct->eeg.data_float[i_meas] = (float **)calloc(elan_struct->chan_nb, sizeof(float *));
					ELAN_CHECK_ERROR_ALLOC(elan_struct->eeg.data_float[i_meas], "EEG matrix data array of float (channel dimension).");
					for (i_chan=0; i_chan<elan_struct->chan_nb; i_chan++)
					{
						elan_struct->eeg.data_float[i_meas][i_chan] = &elan_struct->eeg.flat_float[(i_meas*elan_struct->chan_nb*elan_struct->eeg.samp_nb)+(i_chan*elan_struct->eeg.samp_nb)]; /* Points to segment of data. */
					}
				}
				break;
			case EF_DATA_DOUBLE:
				elan_struct->eeg.flat_double = (double *)calloc(elan_struct->measure_channel_nb*elan_struct->chan_nb*elan_struct->eeg.samp_nb, sizeof(double));
				ELAN_CHECK_ERROR_ALLOC(elan_struct->eeg.flat_double, "EEG flat data array of double.");

				elan_struct->eeg.data_double = (double ***)calloc(elan_struct->measure_channel_nb, sizeof(double **));
				ELAN_CHECK_ERROR_ALLOC(elan_struct->eeg.data_double, "EEG matrix data array of double (measure dimension).");
				for (i_meas=0; i_meas<elan_struct->measure_channel_nb; i_meas++)
				{
					elan_struct->eeg.data_double[i_meas] = (double **)calloc(elan_struct->chan_nb, sizeof(double *));
					ELAN_CHECK_ERROR_ALLOC(elan_struct->eeg.data_double[i_meas], "EEG matrix data array of double (channel dimension).");
					for (i_chan=0; i_chan<elan_struct->chan_nb; i_chan++)
					{
						elan_struct->eeg.data_double[i_meas][i_chan] = &elan_struct->eeg.flat_double[(i_meas*elan_struct->chan_nb*elan_struct->eeg.samp_nb)+(i_chan*elan_struct->eeg.samp_nb)]; /* Points to segment of data. */
					}
				}
				break;
		}
	}
	
	if (elan_struct->has_ep == EF_YES)
	{
		switch (elan_struct->ep.data_type)
		{
			case EF_DATA_FLOAT:
				elan_struct->ep.flat_float = (float *)calloc(elan_struct->measure_channel_nb*elan_struct->chan_nb*elan_struct->ep.samp_nb, sizeof(float));
				ELAN_CHECK_ERROR_ALLOC(elan_struct->ep.flat_float, "EP flat data array of float.");

				elan_struct->ep.data_float = (float ***)calloc(elan_struct->measure_channel_nb, sizeof(float **));
				ELAN_CHECK_ERROR_ALLOC(elan_struct->ep.data_float, "EP matrix data array of float (measure dimension).");
				for (i_meas=0; i_meas<elan_struct->measure_channel_nb; i_meas++)
				{
					elan_struct->ep.data_float[i_meas] = (float **)calloc(elan_struct->chan_nb, sizeof(float *));
					ELAN_CHECK_ERROR_ALLOC(elan_struct->ep.data_float[i_meas], "EP matrix data array of float (channel dimension).");
					for (i_chan=0; i_chan<elan_struct->chan_nb; i_chan++)
					{
						elan_struct->ep.data_float[i_meas][i_chan] = &elan_struct->ep.flat_float[(i_meas*elan_struct->chan_nb*elan_struct->ep.samp_nb)+(i_chan*elan_struct->ep.samp_nb)]; /* Points to segment of data. */
					}
				}
				break;
			case EF_DATA_DOUBLE:
				elan_struct->ep.flat_double = (double *)calloc(elan_struct->measure_channel_nb*elan_struct->chan_nb*elan_struct->ep.samp_nb, sizeof(double));
				ELAN_CHECK_ERROR_ALLOC(elan_struct->ep.flat_double, "EP flat data array of double.");

				elan_struct->ep.data_double = (double ***)calloc(elan_struct->measure_channel_nb, sizeof(double **));
				ELAN_CHECK_ERROR_ALLOC(elan_struct->ep.data_double, "EP matrix data array of double (measure dimension).");
				for (i_meas=0; i_meas<elan_struct->measure_channel_nb; i_meas++)
				{
					elan_struct->ep.data_double[i_meas] = (double **)calloc(elan_struct->chan_nb, sizeof(double *));
					ELAN_CHECK_ERROR_ALLOC(elan_struct->ep.data_double[i_meas], "EP matrix data array of double (channel dimension).");
					for (i_chan=0; i_chan<elan_struct->chan_nb; i_chan++)
					{
						elan_struct->ep.data_double[i_meas][i_chan] = &elan_struct->ep.flat_double[(i_meas*elan_struct->chan_nb*elan_struct->ep.samp_nb)+(i_chan*elan_struct->ep.samp_nb)]; /* Points to segment of data. */
					}
				}
				break;
		}
	}
	
	if (elan_struct->has_tf == EF_YES)
	{
		switch (elan_struct->tf.data_type)
		{
			case EF_DATA_FLOAT:
				elan_struct->tf.flat_float = (float *)calloc(elan_struct->measure_channel_nb*elan_struct->chan_nb*elan_struct->tf.freq_nb*elan_struct->tf.samp_nb, sizeof(float));
				ELAN_CHECK_ERROR_ALLOC(elan_struct->tf.flat_float, "TF flat data array of float.");

				elan_struct->tf.data_float = (float ****)calloc(elan_struct->measure_channel_nb, sizeof(float ***));
				ELAN_CHECK_ERROR_ALLOC(elan_struct->tf.data_float, "TF matrix data array of float (measure dimension).");
				for (i_meas=0; i_meas<elan_struct->measure_channel_nb; i_meas++)
				{
					elan_struct->tf.data_float[i_meas] = (float ***)calloc(elan_struct->chan_nb, sizeof(float **));
					ELAN_CHECK_ERROR_ALLOC(elan_struct->tf.data_float[i_meas], "TF matrix data array of float (channel dimension).");
					for (i_chan=0; i_chan<elan_struct->chan_nb; i_chan++)
					{
						elan_struct->tf.data_float[i_meas][i_chan] = (float **)calloc(elan_struct->tf.freq_nb, sizeof(float *));
						ELAN_CHECK_ERROR_ALLOC(elan_struct->tf.data_float[i_meas][i_chan], "TF matrix data array of float (frequency dimension).");
						for (i_freq=0; i_freq<elan_struct->tf.freq_nb; i_freq++)
						{
							elan_struct->tf.data_float[i_meas][i_chan][i_freq] = &elan_struct->tf.flat_float[(i_meas*elan_struct->chan_nb*elan_struct->tf.freq_nb*elan_struct->tf.samp_nb)+
								(i_chan*elan_struct->tf.freq_nb*elan_struct->tf.samp_nb)+(i_freq*elan_struct->tf.samp_nb)]; /* Points to segment of data. */
						}
					}
				}
				break;
			case EF_DATA_DOUBLE:
				elan_struct->tf.flat_double = (double *)calloc(elan_struct->measure_channel_nb*elan_struct->chan_nb*elan_struct->tf.freq_nb*elan_struct->tf.samp_nb, sizeof(double));
				ELAN_CHECK_ERROR_ALLOC(elan_struct->tf.flat_double, "TF flat data array of double.");

				elan_struct->tf.data_double = (double ****)calloc(elan_struct->measure_channel_nb, sizeof(double ***));
				ELAN_CHECK_ERROR_ALLOC(elan_struct->tf.data_double, "TF matrix data array of double (measure dimension).");
				for (i_meas=0; i_meas<elan_struct->measure_channel_nb; i_meas++)
				{
					elan_struct->tf.data_double[i_meas] = (double ***)calloc(elan_struct->chan_nb, sizeof(double **));
					ELAN_CHECK_ERROR_ALLOC(elan_struct->tf.data_double[i_meas], "TF matrix data array of double (channel dimension).");
					for (i_chan=0; i_chan<elan_struct->chan_nb; i_chan++)
					{
						elan_struct->tf.data_double[i_meas][i_chan] = (double **)calloc(elan_struct->tf.freq_nb, sizeof(double *));
						ELAN_CHECK_ERROR_ALLOC(elan_struct->tf.data_double[i_meas][i_chan], "TF matrix data array of double (frequency dimension).");
						for (i_freq=0; i_freq<elan_struct->tf.freq_nb; i_freq++)
						{
							elan_struct->tf.data_double[i_meas][i_chan][i_freq] = &elan_struct->tf.flat_double[(i_meas*elan_struct->chan_nb*elan_struct->tf.freq_nb*elan_struct->tf.samp_nb)+
								(i_chan*elan_struct->tf.freq_nb*elan_struct->tf.samp_nb)+(i_freq*elan_struct->tf.samp_nb)]; /* Points to segment of data. */
						}
					}
				}
				break;
		}
	}
}


/*---------------------------------------------------------------------*/
/* ef_free_struct: free all dynamically allocated fields of structure. */
/*---------------------------------------------------------------------*/
void ef_free_struct(elan_struct_t *elan_struct)
{
	int i_chan, i_coord, i_meas;
	
	for (i_chan=0; i_chan<elan_struct->chan_nb; i_chan++)
	{
		for (i_coord=0; i_coord<elan_struct->chan_list[i_chan].coord_nb; i_coord++)
		{
			free(elan_struct->chan_list[i_chan].coord_list[i_coord].val_list);
		}
		free(elan_struct->chan_list[i_chan].coord_list);
	}
	free(elan_struct->measure_channel_label_list);
	free(elan_struct->chan_list);
	elan_struct->measure_channel_label_list = NULL;
	elan_struct->chan_list = NULL;

	if (elan_struct->has_eeg == EF_YES)
	{
		if (elan_struct->eeg.flag_cont_epoch == EF_EEG_CONTINUOUS) {
			if (elan_struct->eeg.event_nb > 0) {
				free(elan_struct->eeg.event_code_list);
				free(elan_struct->eeg.event_sample_list);
				elan_struct->eeg.event_code_list = NULL;
				elan_struct->eeg.event_sample_list = NULL;
			}
		}
	}
	
	if (elan_struct->has_ep == EF_YES)
	{
		if (elan_struct->ep.other_events_list != NULL) {
			for (i_meas=0; i_meas<elan_struct->measure_channel_nb; i_meas++) {
				free(elan_struct->ep.other_events_list[i_meas]);
			}
			free(elan_struct->ep.other_events_list);
		}
		free(elan_struct->ep.other_events_nb);
		elan_struct->ep.other_events_nb = NULL;
		elan_struct->ep.other_events_list = NULL;
	}

	if (elan_struct->has_tf == EF_YES)
	{
		if (elan_struct->tf.other_events_list != NULL) {
			for (i_meas=0; i_meas<elan_struct->measure_channel_nb; i_meas++) {
				free(elan_struct->tf.other_events_list[i_meas]);
			}
			free(elan_struct->tf.other_events_list);
		}
		free(elan_struct->tf.other_events_nb);
		elan_struct->tf.other_events_nb = NULL;
		elan_struct->tf.other_events_list = NULL;

		free(elan_struct->tf.freq_array);
		free(elan_struct->tf.wavelet_char);
		elan_struct->tf.freq_array = NULL;
		elan_struct->tf.wavelet_char = NULL;
	}
	
	if (elan_struct->orig_info != NULL)
	{
		if (elan_struct->orig_info->has_eeg_info == EF_YES)
		{
			/* Free EEG part. */
			free(elan_struct->orig_info->eeg_info.eeg_convADC);
			free(elan_struct->orig_info->eeg_info.eeg_offsetADC);
			if (elan_struct->orig_info->eeg_info.bufReadPtr != NULL)
			{
				free(elan_struct->orig_info->eeg_info.bufReadPtr);
			}
			elan_struct->orig_info->eeg_info.bufReadPtr = NULL;
			elan_struct->orig_info->eeg_info.bufReadSize = 0;
		}
		if (elan_struct->orig_info->has_ep_info == EF_YES)
		{
			/* Free EP part. */
			if (elan_struct->orig_info->ep_info.bufReadPtr != NULL)
			{
				free(elan_struct->orig_info->ep_info.bufReadPtr);
			}
			elan_struct->orig_info->ep_info.bufReadPtr = NULL;
			elan_struct->orig_info->ep_info.bufReadSize = 0;
		}
		if (elan_struct->orig_info->has_tf_info == EF_YES)
		{
			/* Free TF part. */
			if (elan_struct->orig_info->tf_info.bufReadPtr != NULL)
			{
				free(elan_struct->orig_info->tf_info.bufReadPtr);
			}
			elan_struct->orig_info->tf_info.bufReadPtr = NULL;
			elan_struct->orig_info->tf_info.bufReadSize = 0;
		}
		
		free(elan_struct->orig_info);
		elan_struct->orig_info = NULL;
	}
}


/*------------------------------------------------------*/
/* ef_free_data_array: free array for all defined data. */
/*------------------------------------------------------*/
void ef_free_data_array(elan_struct_t *elan_struct)
{
	int i_meas, i_chan;
	
	if (elan_struct->has_eeg == EF_YES)
	{
		switch (elan_struct->eeg.data_type)
		{
			case EF_DATA_FLOAT:
				free(elan_struct->eeg.flat_float);
				for (i_meas=0; i_meas<elan_struct->measure_channel_nb; i_meas++)
				{
					free(elan_struct->eeg.data_float[i_meas]);
				}
				free(elan_struct->eeg.data_float);
				elan_struct->eeg.flat_float = NULL;
				elan_struct->eeg.data_float = NULL;
				break;
			case EF_DATA_DOUBLE:
				free(elan_struct->eeg.flat_double);
				for (i_meas=0; i_meas<elan_struct->measure_channel_nb; i_meas++)
				{
					free(elan_struct->eeg.data_double[i_meas]);
				}
				free(elan_struct->eeg.data_double);
				elan_struct->eeg.flat_double = NULL;
				elan_struct->eeg.data_double = NULL;
				break;
		}
	}
	
	if (elan_struct->has_ep == EF_YES)
	{
		switch (elan_struct->ep.data_type)
		{
			case EF_DATA_FLOAT:
				free(elan_struct->ep.flat_float);
				for (i_meas=0; i_meas<elan_struct->measure_channel_nb; i_meas++)
				{
					free(elan_struct->ep.data_float[i_meas]);
				}
				free(elan_struct->ep.data_float);
				elan_struct->ep.flat_float = NULL;
				elan_struct->ep.data_float = NULL;
				break;
			case EF_DATA_DOUBLE:
				free(elan_struct->ep.flat_double);
				for (i_meas=0; i_meas<elan_struct->measure_channel_nb; i_meas++)
				{
					free(elan_struct->ep.data_double[i_meas]);
				}
				free(elan_struct->ep.data_double);
				elan_struct->ep.flat_double = NULL;
				elan_struct->ep.data_double = NULL;
				break;
		}
	}
	
	if (elan_struct->has_tf == EF_YES)
	{
		switch (elan_struct->tf.data_type)
		{
			case EF_DATA_FLOAT:
				free(elan_struct->tf.flat_float);
				for (i_meas=0; i_meas<elan_struct->measure_channel_nb; i_meas++)
				{
					for (i_chan=0; i_chan<elan_struct->chan_nb; i_chan++)
					{
						free(elan_struct->tf.data_float[i_meas][i_chan]);
					}
					free(elan_struct->tf.data_float[i_meas]);
				}
				free(elan_struct->tf.data_float);
				elan_struct->tf.flat_float = NULL;
				elan_struct->tf.data_float = NULL;
				break;
			case EF_DATA_DOUBLE:
				free(elan_struct->tf.flat_double);
				for (i_meas=0; i_meas<elan_struct->measure_channel_nb; i_meas++)
				{
					for (i_chan=0; i_chan<elan_struct->chan_nb; i_chan++)
					{
						free(elan_struct->tf.data_double[i_meas][i_chan]);
					}
					free(elan_struct->tf.data_double[i_meas]);
				}
				free(elan_struct->tf.data_double);
				elan_struct->tf.flat_double = NULL;
				elan_struct->tf.data_double = NULL;
				break;
		}
	}
}


/*********************************************************************************/
/* ef_alloc_array_1d: allocate a buffer of 1 dimension (dim1) of dataSize values */
/*                    and return a pointer to this buffer or NULL if failed.     */
/*********************************************************************************/
void *ef_alloc_array_1d(int dim1, int dataSize)
{
	void *ptr;
	ptr = (void *)calloc(dim1, dataSize);
	ELAN_CHECK_ERROR_ALLOC(ptr, "1d array of (%d, %d bytes) (1st dimension error).", dim1, dataSize);
	return(ptr);
}


/******************************************************/
/* ef_alloc_array_2d: free a buffer of 2 dimensions   */
/*                    and return nothing.             */
/******************************************************/
void ef_free_array_1d(void **ptr)
{
	free(*ptr);
	*ptr = NULL;
}


/*****************************************************************************************/
/* ef_alloc_array_2d: allocate a buffer of 2 dimensions (dim1 x dim2) of dataSize values */
/*                    and return a pointer to this buffer or NULL if failed.             */
/*****************************************************************************************/
void *ef_alloc_array_2d(int dim1, int dim2, int dataSize)
{
	void **ptr;
	int i;
	
	ptr = (void **)calloc(dim1, sizeof(void *));
	ELAN_CHECK_ERROR_ALLOC(ptr, "2d array of (%d x %d, %d bytes) (1st dimension error).", dim1, dim2, dataSize);
	ptr[0] = (void *)calloc(dim1*dim2, dataSize);
	ELAN_CHECK_ERROR_ALLOC(ptr[0], "2d array of (%d x %d, %d bytes) (2nd dimension error).", dim1, dim2, dataSize);
	for (i=0; i<dim1; i++)
	{
		ptr[i] = (char*)ptr[0] + (i*dim2*dataSize);
	}
	
	return(ptr);
}


/******************************************************/
/* ef_alloc_array_2d: free a buffer of 2 dimensions   */
/*                    and return nothing.             */
/******************************************************/
void ef_free_array_2d(void ***ptr)
{
	free(*ptr[0]);
	free(*ptr);
	*ptr = NULL;
}


/************************************************************************************************/
/* ef_alloc_array_3d: allocate a buffer of 3 dimensions (dim1 x dim2 x dim3) of dataSize values */
/*                    and return a pointer to this buffer or NULL if failed.                    */
/************************************************************************************************/
void *ef_alloc_array_3d(int dim1, int dim2, int dim3, int dataSize)
{
	void ***ptr, *contigBuf;
	int i, j;
	
	contigBuf = (void *)calloc(dim1*dim2*dim3, dataSize);
	ELAN_CHECK_ERROR_ALLOC(contigBuf, "3d array of (%d x %d x %d, %d bytes) (3rd dimension error).", dim1, dim2, dim3, dataSize);

	ptr = (void ***)calloc(dim1, sizeof(void **));
	ELAN_CHECK_ERROR_ALLOC(ptr, "3d array of (%d x %d x %d, %d bytes) (1st dimension error).", dim1, dim2, dim3, dataSize);
	for (i=0; i<dim1; i++)
	{
		ptr[i] = (void **)calloc(dim2, sizeof(void *));
		ELAN_CHECK_ERROR_ALLOC(ptr[i], "3d array of (%d x %d x %d, %d bytes) (2nd dimension error).", dim1, dim2, dim3, dataSize);
		for (j=0; j<dim2; j++)
		{
			ptr[i][j] = (char*)contigBuf + (dim2*dim3*i*dataSize) + (dim3*j*dataSize);
		}
	}
	
	return(ptr);
}


/******************************************************/
/* ef_alloc_array_3d: free a buffer of 3 dimensions   */
/*                    and return nothing.             */
/******************************************************/
void ef_free_array_3d(void ****ptr, int dim1, int dim2, int dim3)
{
	int i;
	void ***valPtr;
	
	valPtr = *ptr;
	free(valPtr[0][0]); /* Free contiguous data. */
	for (i=0; i<dim1; i++)
	{
		free(valPtr[i]);
	}
	free(valPtr);
	*ptr = NULL;
}


/********************************************************************/
/* ef_alloc_measure_label_list: allocate memory for measure labels. */
/********************************************************************/
void ef_alloc_measure_label_list(elan_struct_t *elan_struct)
{
	elan_struct->measure_channel_label_list = (str_label_t *)calloc(elan_struct->measure_channel_nb, sizeof(str_label_t));
	ELAN_CHECK_ERROR_ALLOC(elan_struct->measure_channel_label_list, "for storing measure labels.");
}


/********************************************************************/
/* ef_alloc_channel_list: allocate memory for channel informations. */
/********************************************************************/
void ef_alloc_channel_list(elan_struct_t *elan_struct)
{
	elan_struct->chan_list = (chan_t *)calloc(elan_struct->chan_nb, sizeof(chan_t));
	ELAN_CHECK_ERROR_ALLOC(elan_struct->chan_list, "for storing channel informations.");
}


/****************************************************************************/
/* ef_alloc_eeg_array_events: allocates array of events fro continuous EEG. */
/****************************************************************************/
void ef_alloc_eeg_array_events(elan_struct_t *elan_struct)
{
	if ((elan_struct->has_eeg == EF_YES) && (elan_struct->eeg.flag_cont_epoch == EF_EEG_CONTINUOUS)) {
		if (elan_struct->eeg.event_nb > 0) {
			elan_struct->eeg.event_code_list = (int *)calloc(elan_struct->eeg.event_nb, sizeof(int));
			ELAN_CHECK_ERROR_ALLOC(elan_struct->eeg.event_code_list, "for EEG event codes");
			elan_struct->eeg.event_sample_list = (int *)calloc(elan_struct->eeg.event_nb, sizeof(int));
			ELAN_CHECK_ERROR_ALLOC(elan_struct->eeg.event_sample_list, "for EEG event timestamps");
		} else {
			elan_struct->eeg.event_code_list = NULL;
			elan_struct->eeg.event_sample_list = NULL;
		}
	}
}


/************************************************************************/
/* ef_alloc_ep_array_otherevents: allocates array of other events       */
/*                (number and list for each measure).                   */
/************************************************************************/
void ef_alloc_ep_array_otherevents(elan_struct_t *elan_struct)
{
	if (elan_struct->has_ep == EF_YES) {
		elan_struct->ep.other_events_nb = (int *)calloc(elan_struct->measure_channel_nb, sizeof(int));
		ELAN_CHECK_ERROR_ALLOC(elan_struct->ep.other_events_nb, "for other event numbers of EP file");
		elan_struct->ep.other_events_list = NULL;
	}
}


/************************************************************************/
/* ef_alloc_ep_array_otherevents: allocates array of other events       */
/*                (list size for each measure).                         */
/************************************************************************/
void ef_alloc_ep_array_otherevents_list(elan_struct_t *elan_struct)
{
	int i, eventSum;
	if (elan_struct->has_ep == EF_YES) {
		eventSum = 0;
		for (i=0; i<elan_struct->measure_channel_nb; i++) {
			eventSum += elan_struct->ep.other_events_nb[i];
		}
		if (eventSum > 0) {
			elan_struct->ep.other_events_list = (int **)calloc(elan_struct->measure_channel_nb, sizeof(int *));
			ELAN_CHECK_ERROR_ALLOC(elan_struct->ep.other_events_list, "for other event list of EP file");
			
			elan_struct->ep.other_events_list[0] = (int *)calloc(eventSum, sizeof(int));
			ELAN_CHECK_ERROR_ALLOC(elan_struct->ep.other_events_list[0], "for other event list of EP file");

			for (i=1; i<elan_struct->measure_channel_nb; i++) {
				elan_struct->ep.other_events_list[i] = elan_struct->ep.other_events_list[i-1]+(elan_struct->ep.other_events_nb[i-1]*sizeof(int));
			}
		} else {
			elan_struct->ep.other_events_list = NULL;
		}
	}
}


/************************************************************************/
/* ef_alloc_tf_array_byfreq: allocates array of by frequency parameters */
/*                (frequency list, wavelet characteristic).             */
/************************************************************************/
void ef_alloc_tf_array_byfreq(elan_struct_t *elan_struct)
{
	if (elan_struct->has_tf == EF_YES)
	{
		elan_struct->tf.freq_array = (float *)calloc(elan_struct->tf.freq_nb, sizeof(float));
		ELAN_CHECK_ERROR_ALLOC(elan_struct->tf.freq_array, "for storing frequency values.");
		elan_struct->tf.wavelet_char = (float *)calloc(elan_struct->tf.freq_nb, sizeof(float));
		ELAN_CHECK_ERROR_ALLOC(elan_struct->tf.wavelet_char, "for storing wavelet characteristics.");
	}
}


/************************************************************************/
/* ef_alloc_tf_array_otherevents: allocates array of other events       */
/*                (number and list for each measure).                   */
/************************************************************************/
void ef_alloc_tf_array_otherevents(elan_struct_t *elan_struct)
{
	if (elan_struct->has_tf == EF_YES) {
		elan_struct->tf.other_events_nb = (int *)calloc(elan_struct->measure_channel_nb, sizeof(int));
		ELAN_CHECK_ERROR_ALLOC(elan_struct->tf.other_events_nb, "for other event numbers of TF file");
		elan_struct->tf.other_events_list = NULL;
	}
}


/************************************************************************/
/* ef_alloc_tf_array_otherevents: allocates array of other events       */
/*                (list size for each measure).                         */
/************************************************************************/
void ef_alloc_tf_array_otherevents_list(elan_struct_t *elan_struct)
{
	int i, eventSum;
	if (elan_struct->has_tf == EF_YES) {
		eventSum = 0;
		for (i=0; i<elan_struct->measure_channel_nb; i++) {
			eventSum += elan_struct->tf.other_events_nb[i];
		}
		if (eventSum > 0) {
			elan_struct->tf.other_events_list = (int **)calloc(elan_struct->measure_channel_nb, sizeof(int *));
			ELAN_CHECK_ERROR_ALLOC(elan_struct->tf.other_events_list, "for other event list of TF file");
			
			elan_struct->tf.other_events_list[0] = (int *)calloc(eventSum, sizeof(int));
			ELAN_CHECK_ERROR_ALLOC(elan_struct->tf.other_events_list[0], "for other event list of TF file");

			for (i=1; i<elan_struct->measure_channel_nb; i++) {
				elan_struct->tf.other_events_list[i] = elan_struct->tf.other_events_list[i-1]+(elan_struct->tf.other_events_nb[i-1]*sizeof(int));
			}
		} else {
			elan_struct->tf.other_events_list = NULL;
		}
	}
}


/***************************************************/
/* ef_get_mach_endian: get running machine endian. */
/***************************************************/
int ef_get_mach_endian()
{
#if defined(_WIN32) || defined(_WIN64)
	//Windows Byte Order
	if (ENDIAN_ORDER == LITTLE_ENDIAN)
	{
		fprintf(stdout, "machine is little endian\n");
		return(LITTLE_ENDIAN);
	}
	else if (ENDIAN_ORDER == BIG_ENDIAN)
	{
		fprintf(stdout, "machine is big endian\n");
		return(BIG_ENDIAN);
	}
	else if (ENDIAN_ORDER == PDP_ENDIAN)
	{
		fprintf(stdout, "jeez, machine is PDP!\n");
		return(PDP_ENDIAN);
	}
	else
	{
		fprintf(stdout, "What kind of hardware is this?!\n");
		return(-1); /* unsupported endian. */
	}
#else
	if (__BYTE_ORDER == LITTLE_ENDIAN)
	{
		return(EF_LITTLE_ENDIAN);
	}
	else if (__BYTE_ORDER == BIG_ENDIAN)
	{
		return(EF_BIG_ENDIAN);
	}
	/*	else if (__BYTE_ORDER==PDP_ENDIAN)
	{
	return(EF_PDP_ENDIAN);
	}*/
	else
	{
		return(-1); /* unsupported endian. */
	}
#endif
}



/*****************************************/
/* ef_test_version: get version of file. */
/*****************************************/
int ef_test_version(char *filename)
{
	int version=UNDEF_VERSION;
	char sigBuf[8], HDF5Signature[8]={137, 72, 68, 70, 13, 10, 26, 10};
	FILE *test;
	
	/* Check signature. */
	test = fopen(filename, "r");
	if (test == NULL) {
		fprintf(stderr, "ERROR: opening file %s .\n", filename);
		return(version);
	}
	fread(sigBuf, sizeof(char), 8, test);
	fclose(test);
	
	if (memcmp(sigBuf, HDF5Signature, 8) == 0) {
		version = HDF5_read_version(filename);
	} else {
		if ((ef_test_suffix(filename, ".eeg") == 0) || (ef_test_suffix(filename, ".EEG") == 0))
		{
			version = HIST_VERSION;
		}
		else if ((ef_test_suffix(filename, ".p") == 0) || (ef_test_suffix(filename, ".P") == 0))
		{
			version = HIST_VERSION;
		}
		else if ((ef_test_suffix(filename, ".tf") == 0) || (ef_test_suffix(filename, ".TF") == 0))
		{
			version = HIST_VERSION;
		}
	}

	return(version);
}


/********************************************************/
/* ef_test_suffix: test if filename's suffix is suffix. */
/********************************************************/
int ef_test_suffix(char *filename, char *suffix)
{
	return(strcmp(&filename[strlen(filename)-strlen(suffix)], suffix));
}


/*******************************************************************************/
/* ef_copy_elan_struct: copy and allocates memory for fields from src to dest. */
/*******************************************************************************/
void ef_copy_elan_struct(elan_struct_t *dest, elan_struct_t src)
{
	int i_meas, i_chan, i_coord, i_val, i_f, i_evt;
	
	dest->endian = src.endian;
	dest->version = src.version;
	dest->release = src.release;
	strcpy(dest->comment, src.comment);
	dest->chan_nb = src.chan_nb;
	dest->measure_channel_nb = src.measure_channel_nb;
	ef_alloc_measure_label_list(dest);
	for (i_meas=0; i_meas<dest->measure_channel_nb; i_meas++)
	{
		strcpy(dest->measure_channel_label_list[i_meas], src.measure_channel_label_list[i_meas]);
	}
	ef_alloc_channel_list(dest);
	for (i_chan=0; i_chan<dest->chan_nb; i_chan++)
	{
		strcpy(dest->chan_list[i_chan].lab, src.chan_list[i_chan].lab);
		strcpy(dest->chan_list[i_chan].type, src.chan_list[i_chan].type);
		strcpy(dest->chan_list[i_chan].unit, src.chan_list[i_chan].unit);
		dest->chan_list[i_chan].coord_nb = src.chan_list[i_chan].coord_nb;
		dest->chan_list[i_chan].coord_list = (coord_t *)calloc(dest->chan_list[i_chan].coord_nb, sizeof(coord_t));
		ELAN_CHECK_ERROR_ALLOC(dest->chan_list[i_chan].coord_list, "coordinates allocation during header copy (chan # %d, coord nb %d).", i_chan+1, dest->chan_list[i_chan].coord_nb);
		for (i_coord=0; i_coord<dest->chan_list[i_chan].coord_nb; i_coord++)
		{
			strcpy(dest->chan_list[i_chan].coord_list[i_coord].lab, src.chan_list[i_chan].coord_list[i_coord].lab);
			dest->chan_list[i_chan].coord_list[i_coord].val_nb = src.chan_list[i_chan].coord_list[i_coord].val_nb;
			dest->chan_list[i_chan].coord_list[i_coord].val_list = (float *)calloc(dest->chan_list[i_chan].coord_list[i_coord].val_nb, sizeof(float));
			ELAN_CHECK_ERROR_ALLOC(dest->chan_list[i_chan].coord_list[i_coord].val_list, "coordinate values allocation during header copy (chan # %d, coord # %d, val nb %d).", i_chan+1, i_coord+1, dest->chan_list[i_chan].coord_list[i_coord].val_nb);
			for (i_val=0; i_val<dest->chan_list[i_chan].coord_list[i_coord].val_nb; i_val++)
			{
				dest->chan_list[i_chan].coord_list[i_coord].val_list[i_val] = src.chan_list[i_chan].coord_list[i_coord].val_list[i_val];
			}
		}
	}
	
	dest->has_eeg = src.has_eeg;
	dest->eeg.data_type = src.eeg.data_type;
	dest->eeg.samp_nb = src.eeg.samp_nb;
	dest->eeg.sampling_freq = src.eeg.sampling_freq;
	dest->eeg.flag_cont_epoch = src.eeg.flag_cont_epoch;
	if (dest->eeg.flag_cont_epoch == EF_EEG_CONTINUOUS) {
		dest->eeg.event_nb = src.eeg.event_nb;
		ef_alloc_eeg_array_events(dest);
		for (i_evt=0; i_evt<dest->eeg.event_nb; i_evt++) {
			dest->eeg.event_code_list[i_evt] = src.eeg.event_code_list[i_evt];
			dest->eeg.event_sample_list[i_evt] = src.eeg.event_sample_list[i_evt];
		}
	}
	dest->eeg.prestim_samp_nb = src.eeg.prestim_samp_nb;
	dest->eeg.poststim_samp_nb = src.eeg.poststim_samp_nb;
	dest->eeg.event_code = src.eeg.event_code;
	dest->eeg.epoch_nb = src.eeg.epoch_nb;
	dest->eeg.data_float = NULL;
	dest->eeg.flat_float = NULL;
	dest->eeg.data_double = NULL;
	dest->eeg.flat_double = NULL;
	
	dest->has_ep = src.has_ep;
	dest->ep.data_type = src.ep.data_type;
	dest->ep.samp_nb = src.ep.samp_nb;
	dest->ep.sampling_freq = src.ep.sampling_freq;
	dest->ep.prestim_samp_nb = src.ep.prestim_samp_nb;
	dest->ep.event_code = src.ep.event_code;
	dest->ep.event_nb = src.ep.event_nb;
	ef_alloc_ep_array_otherevents(dest);  /* Other events on TF file. */
	for (i_meas=0; i_meas<dest->measure_channel_nb; i_meas++) {
		dest->ep.other_events_nb[i_meas] = src.ep.other_events_nb[i_meas];
	}
	ef_alloc_ep_array_otherevents_list(dest);
	for (i_meas=0; i_meas<dest->measure_channel_nb; i_meas++) {
		for (i_evt=0; i_evt<dest->ep.other_events_nb[i_meas]; i_evt++) {
			dest->ep.other_events_list[i_meas][i_evt] = src.ep.other_events_list[i_meas][i_evt];
		}
	}
	dest->ep.data_float = NULL;
	dest->ep.flat_float = NULL;
	dest->ep.data_double = NULL;
	dest->ep.flat_double = NULL;
	
	dest->has_tf = src.has_tf;
	dest->tf.data_type = src.tf.data_type;
	dest->tf.samp_nb = src.tf.samp_nb;
	dest->tf.sampling_freq = src.tf.sampling_freq;
	dest->tf.prestim_samp_nb = src.tf.prestim_samp_nb;
	dest->tf.event_code = src.tf.event_code;
	dest->tf.event_nb = src.tf.event_nb;
	dest->tf.wavelet_type = src.tf.wavelet_type;
	dest->tf.blackman_window = src.tf.blackman_window;
	dest->tf.freq_nb = src.tf.freq_nb;
	ef_alloc_tf_array_byfreq(dest);
	for (i_f=0; i_f<dest->tf.freq_nb; i_f++)
	{
		dest->tf.freq_array[i_f] = src.tf.freq_array[i_f];
		dest->tf.wavelet_char[i_f] = src.tf.wavelet_char[i_f];
	}
	ef_alloc_tf_array_otherevents(dest);  /* Other events on TF file. */
	for (i_meas=0; i_meas<dest->measure_channel_nb; i_meas++) {
		dest->tf.other_events_nb[i_meas] = src.tf.other_events_nb[i_meas];
	}
	ef_alloc_tf_array_otherevents_list(dest);
	for (i_meas=0; i_meas<dest->measure_channel_nb; i_meas++) {
		for (i_evt=0; i_evt<dest->tf.other_events_nb[i_meas]; i_evt++) {
			dest->tf.other_events_list[i_meas][i_evt] = src.tf.other_events_list[i_meas][i_evt];
		}
	}
	dest->tf.data_float = NULL;
	dest->tf.flat_float = NULL;
	dest->tf.data_double = NULL;
	dest->tf.flat_double = NULL;
	
	dest->orig_info = NULL; /* Original information not copied. */
	
	dest->options.compress = src.options.compress;
	dest->options.compressionType = src.options.compressionType;
	dest->options.dataType = src.options.dataType;
}


/************************************************/
/* 	ef_init_options: default values of options. */
/************************************************/
void ef_init_options(elan_struct_t *elan_struct)
{
	static int done=0;
	if (done == 0)
	{
		done = 1;
		elan_struct->options.compress = EF_NO;
		elan_struct->options.compressionType = EF_COMP_NONE;
		elan_struct->options.dataType = EF_DATA_FLOAT;
	}
}


/**************************************************************/
/* ef_parse_options: parse command line options and retrieve  */
/*           libelanfile specific options (compression, ...). */
/**************************************************************/
void ef_parse_options(int argc, char **argv, elan_struct_t *elan_struct)
{
	int i_arg;

	ef_init_options(elan_struct);

	for (i_arg=1; i_arg<argc; i_arg++)
	{
		if (strcmp(argv[i_arg], "-efz") == 0)
		{
			elan_struct->options.compress = EF_YES;
			elan_struct->options.compressionType = EF_COMP_GZIP;
		}
		if (strcmp(argv[i_arg], "-ef32b") == 0)
		{
			elan_struct->options.dataType = EF_DATA_FLOAT;
		}
		if (strcmp(argv[i_arg], "-ef64b") == 0)
		{
			elan_struct->options.dataType = EF_DATA_DOUBLE;
		}
	}
}


/*************************************************************/
/* ef_display_header: display informations from data header. */
/*************************************************************/
void ef_display_header(elan_struct_t *elan_struct)
{
	int i_chan, i_coord, i_val, i_meas, i_freq, i_evt;
	
	/* Display informations. */
	fprintf(stdout, "endian: %d\n", elan_struct->endian);
	fprintf(stdout, "version: %d\n", elan_struct->version);
	fprintf(stdout, "release: %d\n", elan_struct->release);
	fprintf(stdout, "comment: %s\n", elan_struct->comment);
	fprintf(stdout, "channel_nb: %d\n", elan_struct->chan_nb);
	fprintf(stdout, "measure_nb: %d\n", elan_struct->measure_channel_nb);
	for (i_meas=0; i_meas<elan_struct->measure_channel_nb; i_meas++)
	{
		fprintf(stdout, "measure_list[%d]: %s\n", i_meas, elan_struct->measure_channel_label_list[i_meas]);
	}
	fprintf(stdout, "CHANNELS:\n");
	for (i_chan=0; i_chan<elan_struct->chan_nb; i_chan++)
	{
		fprintf(stdout, "\t%d lab: %s\n", i_chan, elan_struct->chan_list[i_chan].lab);
		fprintf(stdout, "\t%d type: %s\n", i_chan, elan_struct->chan_list[i_chan].type);
		fprintf(stdout, "\t%d unit: %s\n", i_chan, elan_struct->chan_list[i_chan].unit);
		fprintf(stdout, "\t%d coord_nb: %d\n", i_chan, elan_struct->chan_list[i_chan].coord_nb);
		for (i_coord=0; i_coord<elan_struct->chan_list[i_chan].coord_nb; i_coord++)
		{
			fprintf(stdout, "\t\t%d %d lab: %s\n", i_chan, i_coord, elan_struct->chan_list[i_chan].coord_list[i_coord].lab);
			fprintf(stdout, "\t\t%d %d val_nb: %d\n", i_chan, i_coord, elan_struct->chan_list[i_chan].coord_list[i_coord].val_nb);
			for (i_val=0; i_val<elan_struct->chan_list[i_chan].coord_list[i_coord].val_nb; i_val++)
			{
				fprintf(stdout, "\t\t\t%d %d %d val_list: %lf\n", i_chan, i_coord, i_val, elan_struct->chan_list[i_chan].coord_list[i_coord].val_list[i_val]);
			}
		}
	}
	
	if (elan_struct->has_eeg == EF_YES)
	{
		fprintf(stdout, "********** EEG **********\n");
		fprintf(stdout, "data_type: %d\n", elan_struct->eeg.data_type);
		fprintf(stdout, "sample_nb: %d\n", elan_struct->eeg.samp_nb);
		fprintf(stdout, "sampling_freq: %f\n", elan_struct->eeg.sampling_freq);
		fprintf(stdout, "flag_cont_epoch: %d\n", elan_struct->eeg.flag_cont_epoch);
		fprintf(stdout, "*************************\n");
	}

	if (elan_struct->has_ep == EF_YES)
	{
		fprintf(stdout, "********** EP **********\n");
		fprintf(stdout, "data_type: %d\n", elan_struct->ep.data_type);
		fprintf(stdout, "sample_nb: %d\n", elan_struct->ep.samp_nb);
		fprintf(stdout, "sampling_freq: %f\n", elan_struct->ep.sampling_freq);
		fprintf(stdout, "prestim_samp_nb: %d\n", elan_struct->ep.prestim_samp_nb);
		fprintf(stdout, "event_code: %d\n", elan_struct->ep.event_code);
		fprintf(stdout, "event_nb: %d\n", elan_struct->ep.event_nb);
		for (i_meas=0; i_meas<elan_struct->measure_channel_nb; i_meas++) {
			fprintf(stdout, "other_event_nb[%d]: %d\n", i_meas, elan_struct->ep.other_events_nb[i_meas]);
			for (i_evt=0; i_evt<elan_struct->ep.other_events_nb[i_meas]; i_evt++) {
				fprintf(stdout, "\tother_event_list[%d][%d]: %d\n", i_meas, i_evt, elan_struct->ep.other_events_list[i_meas][i_evt]);
			}
		}
		fprintf(stdout, "************************\n");
	}
	
	if (elan_struct->has_tf == EF_YES)
	{
		fprintf(stdout, "********** TF **********\n");
		fprintf(stdout, "data_type: %d\n", elan_struct->tf.data_type);
		fprintf(stdout, "sample_nb: %d\n", elan_struct->tf.samp_nb);
		fprintf(stdout, "sampling_freq: %f\n", elan_struct->tf.sampling_freq);
		fprintf(stdout, "prestim_samp_nb: %d\n", elan_struct->tf.prestim_samp_nb);
		fprintf(stdout, "event_code: %d\n", elan_struct->tf.event_code);
		fprintf(stdout, "event_nb: %d\n", elan_struct->tf.event_nb);
		fprintf(stdout, "wavelet_type: %d\n", elan_struct->tf.wavelet_type);
		fprintf(stdout, "blackman_window: %d\n", elan_struct->tf.blackman_window);
		fprintf(stdout, "freq_nb: %d\n", elan_struct->tf.freq_nb);
		for (i_freq=0; i_freq<elan_struct->tf.freq_nb; i_freq++)
		{
			fprintf(stdout, "\tfreq_array[%d]=%f\n", i_freq, elan_struct->tf.freq_array[i_freq]);
		}
		for (i_freq=0; i_freq<elan_struct->tf.freq_nb; i_freq++)
		{
			fprintf(stdout, "\twavelet_char[%d]=%f\n", i_freq, elan_struct->tf.wavelet_char[i_freq]);
		}
		for (i_meas=0; i_meas<elan_struct->measure_channel_nb; i_meas++) {
			fprintf(stdout, "other_event_nb[%d]: %d\n", i_meas, elan_struct->tf.other_events_nb[i_meas]);
			for (i_evt=0; i_evt<elan_struct->tf.other_events_nb[i_meas]; i_evt++) {
				fprintf(stdout, "\tother_event_list[%d][%d]: %d\n", i_meas, i_evt, elan_struct->tf.other_events_list[i_meas][i_evt]);
			}
		}
		fprintf(stdout, "************************\n");
	}
}