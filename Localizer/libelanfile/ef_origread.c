/*--------------------------------------------------*/
/* Reading original ELAN files module.              */
/* v: 1.00    Aut.: PEA                             */
/* CRNL / INSERM U1028 / CNRS UMR 5292 / Univ-Lyon1 */
/*--------------------------------------------------*/
#define _XOPEN_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <unistd.h>


#include "elanfile.h"
#include "ef_origcommon.h"


#define EF_RD_BUFFER_SIZE 100000


/* External function prototypes. */
/*-------------------------------*/
extern int ef_readeegent(char *header_filename, char *data_filename, elan_struct_t *elan_struct); /* Defined in ef_origeeg.c */
extern void ef_orig_init_struct(elan_struct_t *elan_struct); /* Defined in ef_origtools.c */
extern void ef_orig_test_data(char *filename, elan_struct_t *elan_struct); /* Defined in ef_origtools.c */
extern void swapbyte(void *from, size_t size); /* Defined in ef_origtools.c */
extern size_t freadswab(void *ptr, size_t size, size_t nitems, FILE *stream); /* Defined in ef_origtools.c */
extern int ef_readepheader(char *ep_filename, elan_struct_t *elan_struct); /* Defined in ef_origep.c */
extern size_t fortranfread(void *ptr, size_t size, size_t nitems, FILE *stream); /* Defined in ef_origep.c */
extern int ef_readtfheader(char *nomfic, elan_struct_t *elan_struct);  /* Defined in ef_origtf.c */



/***********************************************************************************/
/* ef_orig_read_header: read Elan data header in old original format (u280 style). */
/***********************************************************************************/
int ef_orig_read_header(char *filename, elan_struct_t *elan_struct)
{
	int err=0;
	char header_filename[1024];

	/* Structure initialization. */
	ef_orig_init_struct(elan_struct);

	/* Test file type. */
	ef_orig_test_data(filename, elan_struct);
	
	if (elan_struct->has_eeg == EF_YES)
	{
		/* Read EEG file header. */
		/*-----------------------*/
		elan_struct->orig_info->has_eeg_info = EF_YES;
		
		sprintf(header_filename, "%s.ent", filename);
		err=ef_readeegent(header_filename, filename, elan_struct);
		
		elan_struct->measure_channel_nb = 1;
		ef_alloc_measure_label_list(elan_struct);
		sprintf(elan_struct->measure_channel_label_list[0], "EEG data");
		elan_struct->eeg.flag_cont_epoch = EF_EEG_CONTINUOUS;
	}
	else if (elan_struct->has_ep == EF_YES)
	{
		/* Read EP file header. */
		/*----------------------*/
		elan_struct->orig_info->has_ep_info = EF_YES;
		err = ef_readepheader(filename, elan_struct);
		if (err != EF_NOERR)
		{
			fprintf(stdout, "ERROR %d reading header of EP file %s.\n", err, filename);
		}
		
		strcpy(elan_struct->comment, "Original Elan EP format import");
		elan_struct->measure_channel_nb = 1;
		ef_alloc_measure_label_list(elan_struct);
		sprintf(elan_struct->measure_channel_label_list[0], "EP data");
	}
	else if (elan_struct->has_tf == EF_YES)
	{
		/* Read TF file header. */
		/*----------------------*/
		elan_struct->orig_info->has_tf_info = EF_YES;
		err = ef_readtfheader(filename, elan_struct);

		strcpy(elan_struct->comment, "Original Elan TF format import");
		elan_struct->measure_channel_nb = 1;
		ef_alloc_measure_label_list(elan_struct);
		sprintf(elan_struct->measure_channel_label_list[0], "TF data");
	}
	else
	{
		fprintf(stdout, "ERROR: original file format: no data found.\n");
		err = -1;
	}
	
	return(err);
}


/***********************************************************************************************/
/* ef_orig_read_data_all_channels: read Elan data for all channels at once in original format. */
/***********************************************************************************************/
int ef_orig_read_data_all_channels(char *filename, elan_struct_t *elan_struct)
{
	int err=EF_NOERR, i_chan, sampRead, i_samp, j_samp, i_f, off_buf, sampSize=0;
	FILE *data_f;
	int16_t *buf16=NULL;
	int32_t *buf32=NULL;
	float *bufFlt;
	char string[256];

	if (elan_struct->has_eeg == EF_YES)
	{
    /* Read EEG data file. */
		/*---------------------*/
		data_f = fopen(filename, "rb");
		if (data_f == NULL)
		{
			fprintf(stderr, "ERROR: can't open file %s (data reading).\n", filename);
			return(EF_ERR_OPEN_READ);
		}
		
		switch (elan_struct->orig_info->eeg_info.orig_datatype)
		{
			case (ORIG_EEG_DATATYPE_32BITS):
				sampSize = sizeof(int32_t);
				break;
				
			case (ORIG_EEG_DATATYPE_12BITS):
			case (ORIG_EEG_DATATYPE_16BITS):
				sampSize = sizeof(int16_t);
				break;
		}

		if ((EF_RD_BUFFER_SIZE*(elan_struct->chan_nb+2)*sampSize) != elan_struct->orig_info->eeg_info.bufReadSize)
		{
			elan_struct->orig_info->eeg_info.bufReadSize = EF_RD_BUFFER_SIZE*(elan_struct->chan_nb+2)*sampSize;
			elan_struct->orig_info->eeg_info.bufReadPtr = (void *)realloc(elan_struct->orig_info->eeg_info.bufReadPtr, elan_struct->orig_info->eeg_info.bufReadSize);
			ELAN_CHECK_ERROR_ALLOC(elan_struct->orig_info->eeg_info.bufReadPtr, "for data buffer (EEG reading).");
		}

		switch (elan_struct->orig_info->eeg_info.orig_datatype)
		{
			case (ORIG_EEG_DATATYPE_32BITS):
				buf32 = elan_struct->orig_info->eeg_info.bufReadPtr;
				break;
				
			case (ORIG_EEG_DATATYPE_12BITS):
			case (ORIG_EEG_DATATYPE_16BITS):
				buf16 = elan_struct->orig_info->eeg_info.bufReadPtr;
				break;
		}
		
		i_samp=0;
		while (i_samp<elan_struct->eeg.samp_nb)
		{
			/* Read a block of data. */
			sampRead = fread(elan_struct->orig_info->eeg_info.bufReadPtr, sampSize, EF_RD_BUFFER_SIZE*(elan_struct->chan_nb+2), data_f);
				
			/* Endian conversion. */
			for (j_samp=0; j_samp<sampRead; j_samp++)
			{
				swapbyte((char *)elan_struct->orig_info->eeg_info.bufReadPtr+(j_samp*sampSize), sampSize);
			}
			
			/* Convert to physical values. */
			sampRead /= elan_struct->chan_nb+2;
			switch (elan_struct->orig_info->eeg_info.orig_datatype)
			{
				case (ORIG_EEG_DATATYPE_32BITS):
					switch (elan_struct->eeg.data_type)
					{
						case EF_DATA_FLOAT:
							for (j_samp=0; j_samp<sampRead; j_samp++)
							{
								off_buf = j_samp*(elan_struct->chan_nb+2);
								for (i_chan=0; i_chan<elan_struct->chan_nb; i_chan++)
								{
									elan_struct->eeg.data_float[0][i_chan][i_samp+j_samp]=((float)buf32[off_buf+i_chan]*elan_struct->orig_info->eeg_info.eeg_convADC[i_chan])+elan_struct->orig_info->eeg_info.eeg_offsetADC[i_chan];
								}
							}
							break;
							case EF_DATA_DOUBLE:
							for (j_samp=0; j_samp<sampRead; j_samp++)
							{
								off_buf = j_samp*(elan_struct->chan_nb+2);
								for (i_chan=0; i_chan<elan_struct->chan_nb; i_chan++)
								{
									elan_struct->eeg.data_double[0][i_chan][i_samp+j_samp]=((double)buf32[off_buf+i_chan]*elan_struct->orig_info->eeg_info.eeg_convADC[i_chan])+elan_struct->orig_info->eeg_info.eeg_offsetADC[i_chan];
								}
							}
							break;
					}
					break;
					
				case (ORIG_EEG_DATATYPE_12BITS):
				case (ORIG_EEG_DATATYPE_16BITS):
					switch (elan_struct->eeg.data_type)
					{
						case EF_DATA_FLOAT:
							for (j_samp=0; j_samp<sampRead; j_samp++)
							{
								off_buf = j_samp*(elan_struct->chan_nb+2);
								for (i_chan=0; i_chan<elan_struct->chan_nb; i_chan++)
								{
									elan_struct->eeg.data_float[0][i_chan][i_samp+j_samp]=((float)buf16[off_buf+i_chan]*elan_struct->orig_info->eeg_info.eeg_convADC[i_chan])+elan_struct->orig_info->eeg_info.eeg_offsetADC[i_chan];
								}
							}
							break;
							case EF_DATA_DOUBLE:
							for (j_samp=0; j_samp<sampRead; j_samp++)
							{
								off_buf = j_samp*(elan_struct->chan_nb+2);
								for (i_chan=0; i_chan<elan_struct->chan_nb; i_chan++)
								{
									elan_struct->eeg.data_double[0][i_chan][i_samp+j_samp]=((double)buf16[off_buf+i_chan]*elan_struct->orig_info->eeg_info.eeg_convADC[i_chan])+elan_struct->orig_info->eeg_info.eeg_offsetADC[i_chan];
								}
							}
							break;
					}
					break;
			}
			i_samp += sampRead;
		}
		
		fclose(data_f);
	}
	else if (elan_struct->has_ep == EF_YES)
	{
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
		fseek(data_f, elan_struct->orig_info->ep_info.offset_data, SEEK_SET);
		
		err = EF_NOERR;
		for (i_chan =0; i_chan<elan_struct->chan_nb; i_chan++)
		{
			if (fortranfread(bufFlt, sampSize, elan_struct->ep.samp_nb, data_f) != elan_struct->ep.samp_nb)
			{
				err = 1;
			}
			switch (elan_struct->ep.data_type)
			{
				case EF_DATA_FLOAT:
					for (i_samp=0; i_samp<elan_struct->ep.samp_nb; i_samp++)
					{
						elan_struct->ep.data_float[0][i_chan][i_samp] = bufFlt[i_samp];
					}
				break;
				case EF_DATA_DOUBLE:
					for (i_samp=0; i_samp<elan_struct->ep.samp_nb; i_samp++)
					{
						elan_struct->ep.data_double[0][i_chan][i_samp] = (double)bufFlt[i_samp];
					}
				break;
			}
		}
		if  (err == 1)
			{
			return(EF_ERR_READ_DATA);
			}
			
		fclose(data_f);
	}
	else if (elan_struct->has_tf == EF_YES)
	{
		/* Read TF data file. */
		/*--------------------*/
		data_f = fopen(filename, "rb");
		if (data_f == NULL)
		{
			fprintf(stderr, "ERROR: can't open file %s (data reading).\n", filename);
			return(EF_ERR_OPEN_READ);
		}
		sampSize = sizeof(float);
		if ((elan_struct->tf.freq_nb*elan_struct->tf.samp_nb*sampSize) != elan_struct->orig_info->tf_info.bufReadSize)
		{
			elan_struct->orig_info->tf_info.bufReadSize = elan_struct->tf.freq_nb*elan_struct->tf.samp_nb*sampSize;
			elan_struct->orig_info->tf_info.bufReadPtr = (void *)realloc(elan_struct->orig_info->tf_info.bufReadPtr, elan_struct->orig_info->tf_info.bufReadSize);
			ELAN_CHECK_ERROR_ALLOC(elan_struct->orig_info->tf_info.bufReadPtr, "for data buffer (TF reading).");
		}
		bufFlt = (float *)elan_struct->orig_info->tf_info.bufReadPtr;

		/* Read data. */
		/*------------*/
		fseek(data_f, elan_struct->orig_info->tf_info.offset_data, SEEK_SET);
	
		err = EF_NOERR;
		for (i_chan =0; i_chan<elan_struct->chan_nb; i_chan++)
		{
			switch (elan_struct->orig_info->tf_info.orig_version)
			{
				case ORIG_TF_TEXT:
					for (i_f=0; i_f<elan_struct->tf.freq_nb; i_f++)
					{
						off_buf = i_f*elan_struct->tf.samp_nb;
						for (i_samp=0;i_samp<elan_struct->tf.samp_nb;i_samp++)
						{
							fscanf(data_f, "%s\n", string);
							bufFlt[off_buf+i_samp] = atof(string);
						}
					}
					break;
				case ORIG_TF_BINARY:
					sampRead = freadswab(bufFlt, sizeof(float), elan_struct->tf.freq_nb*elan_struct->tf.samp_nb, data_f);

					if (sampRead != elan_struct->tf.freq_nb*elan_struct->tf.samp_nb)
					{
						fprintf(stderr, "Error reading TF data (%d actually read, %d required)\n", sampRead, elan_struct->tf.freq_nb*elan_struct->tf.samp_nb);
						return(EF_ERR_READ_DATA);
					}
					break;
			}

			switch (elan_struct->tf.data_type)
			{
				case EF_DATA_FLOAT:
					for (i_f=0; i_f<elan_struct->tf.freq_nb; i_f++)
					{
						off_buf = i_f*elan_struct->tf.samp_nb;
						for (i_samp=0; i_samp<elan_struct->tf.samp_nb; i_samp++)
						{
							elan_struct->tf.data_float[0][i_chan][i_f][i_samp] = bufFlt[off_buf+i_samp];
						}
					}
				break;
				case EF_DATA_DOUBLE:
					for (i_f=0; i_f<elan_struct->tf.freq_nb; i_f++)
					{
						off_buf = i_f*elan_struct->tf.samp_nb;
						for (i_samp=0; i_samp<elan_struct->tf.samp_nb; i_samp++)
						{
							elan_struct->tf.data_double[0][i_chan][i_f][i_samp] = bufFlt[off_buf+i_samp];
						}
					}
				break;
			}
		}
			
		fclose(data_f);
	}
	else
	{
		fprintf(stdout, "ERROR: original file format: no data found.\n");
		err = -1;
	}
	
	return(err);
}


/***********************************************************************/
/* ef_orig_read_data_one_channel_file: read all data for one channels. */
/***********************************************************************/
int ef_orig_read_data_one_channel_file(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum)
{
	int err=EF_NOERR, sampRead, i_samp, j_samp, i_f, i_chan, off_buf, sampSize=0;
	FILE *data_f;
	int16_t *buf16=NULL;
	int32_t *buf32=NULL;
	float *bufFlt;
	char string[256];

	if (elan_struct->has_eeg == EF_YES)
	{
    /* Read EEG data file. */
		/*---------------------*/
		data_f = fopen(filename, "rb");
		if (data_f == NULL)
		{
			fprintf(stderr, "ERROR: can't open file %s (data reading).\n", filename);
			return(EF_ERR_OPEN_READ);
		}
		
		switch (elan_struct->orig_info->eeg_info.orig_datatype)
		{
			case (ORIG_EEG_DATATYPE_32BITS):
				sampSize = sizeof(int32_t);
				break;
				
			case (ORIG_EEG_DATATYPE_12BITS):
			case (ORIG_EEG_DATATYPE_16BITS):
				sampSize = sizeof(int16_t);
				break;
		}

		if ((EF_RD_BUFFER_SIZE*(elan_struct->chan_nb+2)*sampSize) != elan_struct->orig_info->eeg_info.bufReadSize)
		{
			elan_struct->orig_info->eeg_info.bufReadSize = EF_RD_BUFFER_SIZE*(elan_struct->chan_nb+2)*sampSize;
			elan_struct->orig_info->eeg_info.bufReadPtr = (void *)realloc(elan_struct->orig_info->eeg_info.bufReadPtr, elan_struct->orig_info->eeg_info.bufReadSize);
			ELAN_CHECK_ERROR_ALLOC(elan_struct->orig_info->eeg_info.bufReadPtr, "for data buffer (EEG reading).");
		}

		switch (elan_struct->orig_info->eeg_info.orig_datatype)
		{
			case (ORIG_EEG_DATATYPE_32BITS):
				buf32 = elan_struct->orig_info->eeg_info.bufReadPtr;
				break;
				
			case (ORIG_EEG_DATATYPE_12BITS):
			case (ORIG_EEG_DATATYPE_16BITS):
				buf16 = elan_struct->orig_info->eeg_info.bufReadPtr;
				break;
		}
		
		i_samp=0;
		while (i_samp<elan_struct->eeg.samp_nb)
		{
			/* Read a block of data. */
			sampRead = fread(elan_struct->orig_info->eeg_info.bufReadPtr, sampSize, EF_RD_BUFFER_SIZE*(elan_struct->chan_nb+2), data_f);
				
			/* Endian and physical value conversions. */
			sampRead /= elan_struct->chan_nb+2;
			switch (elan_struct->orig_info->eeg_info.orig_datatype)
			{
				case (ORIG_EEG_DATATYPE_32BITS):
					switch (elan_struct->eeg.data_type)
					{
						case EF_DATA_FLOAT:
							for (j_samp=0; j_samp<sampRead; j_samp++)
							{
								off_buf = j_samp*(elan_struct->chan_nb+2)+chanNum;
								swapbyte(&buf32[off_buf], sizeof(int32));
								elan_struct->eeg.data_float[measureNum][chanNum][i_samp+j_samp]=((float)buf32[off_buf]*elan_struct->orig_info->eeg_info.eeg_convADC[chanNum])+elan_struct->orig_info->eeg_info.eeg_offsetADC[chanNum];
							}
							break;
							case EF_DATA_DOUBLE:
							for (j_samp=0; j_samp<sampRead; j_samp++)
							{
								off_buf = j_samp*(elan_struct->chan_nb+2)+chanNum;
								swapbyte(&buf32[off_buf], sizeof(int32));
								elan_struct->eeg.data_double[measureNum][chanNum][i_samp+j_samp]=((double)buf32[off_buf]*elan_struct->orig_info->eeg_info.eeg_convADC[chanNum])+elan_struct->orig_info->eeg_info.eeg_offsetADC[chanNum];
							}
							break;
					}
					break;
					
				case (ORIG_EEG_DATATYPE_12BITS):
				case (ORIG_EEG_DATATYPE_16BITS):
					switch (elan_struct->eeg.data_type)
					{
						case EF_DATA_FLOAT:
							for (j_samp=0; j_samp<sampRead; j_samp++)
							{
								off_buf = j_samp*(elan_struct->chan_nb+2)+chanNum;
								swapbyte(&buf16[off_buf], sizeof(int16));
								elan_struct->eeg.data_float[measureNum][chanNum][i_samp+j_samp]=((float)buf16[off_buf]*elan_struct->orig_info->eeg_info.eeg_convADC[chanNum])+elan_struct->orig_info->eeg_info.eeg_offsetADC[chanNum];
							}
							break;
							case EF_DATA_DOUBLE:
							for (j_samp=0; j_samp<sampRead; j_samp++)
							{
								off_buf = j_samp*(elan_struct->chan_nb+2)+chanNum;
								swapbyte(&buf16[off_buf], sizeof(int16));
								elan_struct->eeg.data_double[measureNum][chanNum][i_samp+j_samp]=((double)buf16[off_buf]*elan_struct->orig_info->eeg_info.eeg_convADC[chanNum])+elan_struct->orig_info->eeg_info.eeg_offsetADC[chanNum];
							}
							break;
					}
					break;
			}
			i_samp += sampRead;
		}
		
		fclose(data_f);
	}
	else if (elan_struct->has_ep == EF_YES)
	{
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
		fseek(data_f, elan_struct->orig_info->ep_info.offset_data+(chanNum*((elan_struct->ep.samp_nb+2)*sampSize)), SEEK_SET);
		
		err = EF_NOERR;
		if (fortranfread(bufFlt, sampSize, elan_struct->ep.samp_nb, data_f) != elan_struct->ep.samp_nb)
		{
			err = 1;
		}
		switch (elan_struct->ep.data_type)
		{
			case EF_DATA_FLOAT:
				for (i_samp=0; i_samp<elan_struct->ep.samp_nb; i_samp++)
				{
					elan_struct->ep.data_float[0][chanNum][i_samp] = bufFlt[i_samp];
				}
			break;
			case EF_DATA_DOUBLE:
				for (i_samp=0; i_samp<elan_struct->ep.samp_nb; i_samp++)
				{
					elan_struct->ep.data_double[0][chanNum][i_samp] = (double)bufFlt[i_samp];
				}
			break;
		}
		if  (err == 1)
			{
			return(EF_ERR_READ_DATA);
			}
			
		fclose(data_f);
	}
	else if (elan_struct->has_tf == EF_YES)
	{
		/* Read TF data file. */
		/*--------------------*/
		data_f = fopen(filename, "rb");
		if (data_f == NULL)
		{
			fprintf(stderr, "ERROR: can't open file %s (data reading).\n", filename);
			return(EF_ERR_OPEN_READ);
		}
		sampSize = sizeof(float);
		if ((elan_struct->tf.freq_nb*elan_struct->tf.samp_nb*sampSize) != elan_struct->orig_info->tf_info.bufReadSize)
		{
			elan_struct->orig_info->tf_info.bufReadSize = elan_struct->tf.freq_nb*elan_struct->tf.samp_nb*sampSize;
			elan_struct->orig_info->tf_info.bufReadPtr = (void *)realloc(elan_struct->orig_info->tf_info.bufReadPtr, elan_struct->orig_info->tf_info.bufReadSize);
			ELAN_CHECK_ERROR_ALLOC(elan_struct->orig_info->tf_info.bufReadPtr, "for data buffer (TF reading).");
		}
		bufFlt = (float *)elan_struct->orig_info->tf_info.bufReadPtr;

		/* Read data. */
		/*------------*/
		err = EF_NOERR;
		switch (elan_struct->orig_info->tf_info.orig_version)
		{
			case ORIG_TF_TEXT:
			fseek(data_f, elan_struct->orig_info->tf_info.offset_data, SEEK_SET);
			for (i_chan=0; i_chan<=chanNum; i_chan++)
				{
					for (i_f=0; i_f<elan_struct->tf.freq_nb; i_f++)
					{
						off_buf = i_f*elan_struct->tf.samp_nb;
						for (i_samp=0;i_samp<elan_struct->tf.samp_nb;i_samp++)
						{
							fscanf(data_f, "%s\n", string);
							bufFlt[off_buf+i_samp] = atof(string);
						}
					}
				}
				break;
			case ORIG_TF_BINARY:
				fseek(data_f, elan_struct->orig_info->tf_info.offset_data+(elan_struct->tf.freq_nb*elan_struct->tf.samp_nb*chanNum), SEEK_SET);
				sampRead = freadswab(bufFlt, sizeof(float), elan_struct->tf.freq_nb*elan_struct->tf.samp_nb, data_f);

				if (sampRead != elan_struct->tf.freq_nb*elan_struct->tf.samp_nb)
				{
					fprintf(stderr, "Error reading TF data (%d actually read, %d required)\n", sampRead, elan_struct->tf.freq_nb*elan_struct->tf.samp_nb);
					return(EF_ERR_READ_DATA);
				}
				break;
		}

		switch (elan_struct->tf.data_type)
		{
			case EF_DATA_FLOAT:
				for (i_f=0; i_f<elan_struct->tf.freq_nb; i_f++)
				{
					off_buf = i_f*elan_struct->tf.samp_nb;
					for (i_samp=0; i_samp<elan_struct->tf.samp_nb; i_samp++)
					{
						elan_struct->tf.data_float[0][chanNum][i_f][i_samp] = bufFlt[off_buf+i_samp];
					}
				}
			break;
			case EF_DATA_DOUBLE:
				for (i_f=0; i_f<elan_struct->tf.freq_nb; i_f++)
				{
					off_buf = i_f*elan_struct->tf.samp_nb;
					for (i_samp=0; i_samp<elan_struct->tf.samp_nb; i_samp++)
					{
						elan_struct->tf.data_double[0][chanNum][i_f][i_samp] = bufFlt[off_buf+i_samp];
					}
				}
			break;
		}
			
		fclose(data_f);
	}
	else
	{
		fprintf(stdout, "ERROR: original file format: no data found.\n");
		err = -1;
	}
	
	return(err);
}


