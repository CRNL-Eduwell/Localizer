/*--------------------------------------------------*/
/* Writing ELAN files module.                       */
/* v: 1.00    Aut.: PEA                             */
/* CRNL / INSERM U1028 / CNRS UMR 5292 / Univ-Lyon1 */
/*--------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "elanfile.h"
#include "ef_hdf5common.h"


extern int ef_orig_write_header(char *filename, elan_struct_t *elan_struct);


/*-------------------*/
/* Public functions. */
/*-------------------*/

/********************************************************/
/* ef_write_elan_header_file: write file (header+data). */
/********************************************************/
int ef_write_elan_file(char *filename, elan_struct_t *elan_struct)
{
	int err=EF_NOERR;
	
	err = ef_write_elan_header_file(filename, elan_struct);
	err = ef_write_elan_data_all_channels_file(filename, elan_struct);
	
	return(err);
}


/******************************************************/
/* ef_write_elan_header_file: write file header only. */
/******************************************************/
int ef_write_elan_header_file(char *filename, elan_struct_t *elan_struct)
{
	int err=EF_NOERR;
	
	switch (elan_struct->version)
	{
		case HIST_VERSION:
			fprintf(stderr, "Unsupported feature: ef_write_elan_header_file for original file format version.\n");
			break;
			
		case HDF5_VERSION:
			err = HDF5_write_header(filename, elan_struct);
			break;
		default:
			fprintf(stderr, "ERROR: unsupported version: %d.\n", elan_struct->version);
	}
	
	return(err);
}


/******************************************************************************/
/* ef_write_elan_data_all_channels_file: write data for all channels at once. */
/******************************************************************************/
int ef_write_elan_data_all_channels_file(char *filename, elan_struct_t *elan_struct)
{
	int err=EF_NOERR;
	
	switch (elan_struct->version)
	{
		case HIST_VERSION:
			fprintf(stderr, "Unsupported feature: ef_write_elan_data_all_channels_file for original file format version.\n");
			break;
			
		case HDF5_VERSION:
			err = HDF5_write_data_all_channels(filename, elan_struct);

			break;
		default:
			fprintf(stderr, "ERROR: unsupported version: %d.\n", elan_struct->version);
	}
	return(err);
}


/*****************************************************************************/
/* ef_write_elan_data_one_channel_file: write data for all channels at once. */
/*****************************************************************************/
int ef_write_elan_data_one_channel_file(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum)
{
	int err=EF_NOERR;
	
	switch (elan_struct->version)
	{
		case HIST_VERSION:
			fprintf(stderr, "Unsupported feature: ef_write_elan_data_one_channel_file for original file format version.\n");
			break;
			
		case HDF5_VERSION:
			err = HDF5_write_data_one_channel(filename, elan_struct, measureNum, chanNum);

			break;
		default:
			fprintf(stderr, "ERROR: unsupported version: %d.\n", elan_struct->version);
	}
	return(err);
}


/****************************************************************************************************************/
/* ef_write_EEG_data_sampblock_one_chan: write an EEG data block for one channel from data_ptr.                 */
/* char *filename: file to write to.                                                                            */
/* elan_struct_t *elan_struct: Elan structure (read from header).                                               */
/* int measureNum: indice of measure to write (starting from 0).                                                */
/* int chanNum: indice of channel (starting from 0).                                                            */
/* int sampStart: indice of first sample to write (starting from 0).                                            */
/* int sampNb: number of samples to write.                                                                      */
/* void *data_ptr: data array (one value for each sample).                                                      */
/* return value: number of samples actually written.                                                            */
/****************************************************************************************************************/
int ef_write_EEG_data_sampblock_one_chan(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum, int sampStart, int sampNb, void *data_ptr)
{
	int err=EF_NOERR;
	int actualSampNb;
	
	actualSampNb = ELAN_MIN(elan_struct->eeg.samp_nb-sampStart, sampNb);
	switch (elan_struct->version)
	{
		case HIST_VERSION:
			fprintf(stderr, "Unsupported feature: ef_write_EEG_data_sampblock_one_chan for original file format version.\n");
			break;
			
		case HDF5_VERSION:
			err = HDF5_write_EEG_data_sampblock_one_chan(filename, elan_struct, measureNum, chanNum, sampStart, actualSampNb, data_ptr);
			break;
		default:
			fprintf(stderr, "ERROR: unsupported version: %d.\n", elan_struct->version);
	}
	return(err);
}


/*****************************************************************************************************************/
/* ef_write_EEG_data_sampblock_all_chan: write an EEG data block for all channel from data_ptr.                  */
/* char *filename: file to write to.                                                                             */
/* elan_struct_t *elan_struct: Elan structure (read from header).                                                */
/* int measureNum: indice of measure to write (starting from 0).                                                 */
/* int sampStart: indice of first sample to write (starting from 0).                                             */
/* int sampNb: number of samples to write.                                                                       */
/* void **data_ptr: data array (one value for each sample and channel).                                          */
/* return value: number of samples actually read.                                                                */
/*****************************************************************************************************************/
int ef_write_EEG_data_sampblock_all_chan(char *filename, elan_struct_t *elan_struct, int measureNum, int sampStart, int sampNb, void **data_ptr)
{
	int err=EF_NOERR;
	int actualSampNb;
	
	actualSampNb = ELAN_MIN(elan_struct->eeg.samp_nb-sampStart, sampNb);
	switch (elan_struct->version)
	{
		case HIST_VERSION:
			fprintf(stderr, "Unsupported feature: ef_write_EEG_data_sampblock_all_chan for original file format version.\n");
			break;
			
		case HDF5_VERSION:
			err = HDF5_write_EEG_data_sampblock_all_chan(filename, elan_struct, measureNum, sampStart, actualSampNb, data_ptr);
			break;
		default:
			fprintf(stderr, "ERROR: unsupported version: %d.\n", elan_struct->version);
	}
	return(err);
}


/****************************************************************************************************************/
/* ef_write_EP_data_sampblock_one_chan: write an EP data block for one channel from data_ptr.                   */
/* char *filename: file to write to.                                                                            */
/* elan_struct_t *elan_struct: Elan structure (read from header).                                               */
/* int measureNum: indice of measure to write (starting from 0).                                                */
/* int chanNum: indice of channel (starting from 0).                                                            */
/* int sampStart: indice of first sample to write (starting from 0).                                            */
/* int sampNb: number of samples to write.                                                                      */
/* void *data_ptr: data array (one value for each sample).                                                      */
/* return value: number of samples actually written.                                                            */
/****************************************************************************************************************/
int ef_write_EP_data_sampblock_one_chan(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum, int sampStart, int sampNb, void *data_ptr)
{
	int err=EF_NOERR;
	int actualSampNb;
	
	actualSampNb = ELAN_MIN(elan_struct->eeg.samp_nb-sampStart, sampNb);
	switch (elan_struct->version)
	{
		case HIST_VERSION:
			fprintf(stderr, "Unsupported feature: ef_write_EP_data_sampblock_one_chan for original file format version.\n");
			break;
			
		case HDF5_VERSION:
			err = HDF5_write_EP_data_sampblock_one_chan(filename, elan_struct, measureNum, chanNum, sampStart, actualSampNb, data_ptr);
			break;
		default:
			fprintf(stderr, "ERROR: unsupported version: %d.\n", elan_struct->version);
	}
	return(err);
}


/*****************************************************************************************************************/
/* ef_write_EP_data_sampblock_all_chan: write an EP data block for all channel from data_ptr.                    */
/* char *filename: file to write to.                                                                             */
/* elan_struct_t *elan_struct: Elan structure (read from header).                                                */
/* int measureNum: indice of measure to write (starting from 0).                                                 */
/* int sampStart: indice of first sample to write (starting from 0).                                             */
/* int sampNb: number of samples to write.                                                                       */
/* void **data_ptr: data array (one value for each sample and channel).                                          */
/* return value: number of samples actually read.                                                                */
/*****************************************************************************************************************/
int ef_write_EP_data_sampblock_all_chan(char *filename, elan_struct_t *elan_struct, int measureNum, int sampStart, int sampNb, void **data_ptr)
{
	int err=EF_NOERR;
	int actualSampNb;
	
	actualSampNb = ELAN_MIN(elan_struct->eeg.samp_nb-sampStart, sampNb);
	switch (elan_struct->version)
	{
		case HIST_VERSION:
			fprintf(stderr, "Unsupported feature: ef_write_EP_data_sampblock_all_chan for original file format version.\n");
			break;
			
		case HDF5_VERSION:
			err = HDF5_write_EP_data_sampblock_all_chan(filename, elan_struct, measureNum, sampStart, actualSampNb, data_ptr);
			break;
		default:
			fprintf(stderr, "ERROR: unsupported version: %d.\n", elan_struct->version);
	}
	return(err);
}


/****************************************************************************************************************/
/* ef_write_TF_data_one_channel: write a TF data block for one channel from data_ptr.                           */
/* char *filename: file to write to.                                                                            */
/* elan_struct_t *elan_struct: Elan structure (read from header).                                               */
/* int measureNum: indice of measure to write (starting from 0).                                                */
/* int chanNum: indice of channel (starting from 0).                                                            */
/* int sampStart: indice of first sample to write (starting from 0).                                            */
/* int sampNb: number of samples to write.                                                                      */
/* void *data_ptr: data array (one value for each sample).                                                      */
/* return value: number of samples actually written.                                                            */
/****************************************************************************************************************/
int ef_write_TF_data_one_channel(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum, void **data_ptr)
{
	int err=EF_NOERR;
	
	switch (elan_struct->version)
	{
		case HIST_VERSION:
			fprintf(stderr, "Unsupported feature: ef_write_TF_data_one_TF for original file format version.\n");
			break;
			
		case HDF5_VERSION:
			err = HDF5_write_TF_tf_one_channel(filename, elan_struct, measureNum, chanNum, 0, elan_struct->tf.samp_nb, 0, elan_struct->tf.freq_nb, data_ptr);
			break;
		default:
			fprintf(stderr, "ERROR: unsupported version: %d.\n", elan_struct->version);
	}
	return(err);
}


/****************************************************************************************************************/
/* ef_write_TF_data_all_channel: write a TF data block for all channels from data_ptr.                          */
/* char *filename: file to write to.                                                                            */
/* elan_struct_t *elan_struct: Elan structure (read from header).                                               */
/* int measureNum: indice of measure to write (starting from 0).                                                */
/* int chanNum: indice of channel (starting from 0).                                                            */
/* int sampStart: indice of first sample to write (starting from 0).                                            */
/* int sampNb: number of samples to write.                                                                      */
/* void *data_ptr: data array (one value for each sample).                                                      */
/* return value: number of samples actually written.                                                            */
/****************************************************************************************************************/
int ef_write_TF_data_all_channel(char *filename, elan_struct_t *elan_struct, int measureNum, void ***data_ptr)
{
	int err=EF_NOERR;
	
	switch (elan_struct->version)
	{
		case HIST_VERSION:
			fprintf(stderr, "Unsupported feature: ef_write_TF_data_one_TF for original file format version.\n");
			break;
			
		case HDF5_VERSION:
			err = HDF5_write_TF_tf_all_channel(filename, elan_struct, measureNum, 0, elan_struct->tf.samp_nb, 0, elan_struct->tf.freq_nb, data_ptr);
			break;
		default:
			fprintf(stderr, "ERROR: unsupported version: %d.\n", elan_struct->version);
	}
	return(err);
}



/*--------------------------------------------------------------------------------------------------------------*/
/* ef_write_TF_frequencyblock_one_channel: write all time samples for a frequency block and one channel.        */
/* filename: name of the file to read datra from.                                                               */
/* elan_struct_t *elan_struct: Elan structure (read from header).                                               */
/* int measureNum: indice of measure to read (starting from 0).                                                 */
/* int chanNum: indice of channel (starting from 0).                                                            */
/* int freqStart: indice of 1st frequency to read (starting from 0).                                            */
/* int freqNb: number of frequencies to read (starting from 0).                                                 */
/* void **data_ptr: array of samples to store data. If NULL, data_ptr is allocated in function (must be freed). */
/*                 If allocated in caller, size must be [elan_struct.chan_nbxfreqNbxsampNb] double or double.    */
/* return value: number of samples actually read.                                                               */
/*--------------------------------------------------------------------------------------------------------------*/
int ef_write_TF_frequencyblock_one_channel(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum, int freqStart, int freqNb, void **data_ptr)
{
	int err=EF_NOERR;
	
	if (freqStart>=elan_struct->tf.freq_nb)
	{
		fprintf(stderr, "ERROR: bad value of 1st frequency index (should be < %d).\n", elan_struct->tf.freq_nb);
		err = EF_ERR_BAD_PARM;
		return(err);
	}
	if ((freqStart+freqNb)>elan_struct->tf.freq_nb)
	{
		fprintf(stderr, "ERROR: bad value of frequency number (should be < %d, 1st frequency index is %d, number of frequencies is %d).\n", 
						elan_struct->tf.freq_nb-freqStart, freqStart, elan_struct->tf.freq_nb);
		err = EF_ERR_BAD_PARM;
		return(err);
	}
		
	switch (elan_struct->version)
	{
		case HIST_VERSION:
			fprintf(stderr, "Unsupported feature: ef_write_TF_frequencyblock_one_channel for original file format version.\n");
			break;
			
		case HDF5_VERSION:
			err = HDF5_write_TF_tf_one_channel(filename, elan_struct, measureNum, chanNum, 0, elan_struct->tf.samp_nb, freqStart, freqNb, data_ptr);
			break;
		default:
			fprintf(stderr, "ERROR: unsupported version: %d.\n", elan_struct->version);
	}
	return(err);
}


/*--------------------------------------------------------------------------------------------------------------*/
/* ef_write_TF_frequencyblock_all_channel: write all time samples for a frequency block and all channels.       */
/* filename: name of the file to read datra from.                                                               */
/* elan_struct_t *elan_struct: Elan structure (read from header).                                               */
/* int measureNum: indice of measure to read (starting from 0).                                                 */
/* int freqStart: indice of 1st frequency to read (starting from 0).                                            */
/* int freqNb: number of frequencies to read (starting from 0).                                                 */
/* void ***data_ptr: array of samples to store data. If NULL, data_ptr is allocated in function (must be freed).*/
/*                 If allocated in caller, size must be [elan_struct.chan_nbxfreqNbxsampNb] double or double.    */
/* return value: number of samples actually read.                                                               */
/*--------------------------------------------------------------------------------------------------------------*/
int ef_write_TF_frequencyblock_all_channel(char *filename, elan_struct_t *elan_struct, int measureNum, int freqStart, int freqNb, void ***data_ptr)
{
	int err=EF_NOERR;
	
	if (freqStart>=elan_struct->tf.freq_nb)
	{
		fprintf(stderr, "ERROR: bad value of 1st frequency index (should be < %d).\n", elan_struct->tf.freq_nb);
		err = EF_ERR_BAD_PARM;
		return(err);
	}
	if ((freqStart+freqNb)>elan_struct->tf.freq_nb)
	{
		fprintf(stderr, "ERROR: bad value of frequency number (should be < %d, 1st frequency index is %d, number of frequencies is %d).\n", 
						elan_struct->tf.freq_nb-freqStart, freqStart, elan_struct->tf.freq_nb);
		err = EF_ERR_BAD_PARM;
		return(err);
	}
		
	switch (elan_struct->version)
	{
		case HIST_VERSION:
			fprintf(stderr, "Unsupported feature: ef_write_TF_frequencyblock_all_channel for original file format version.\n");
			break;
			
		case HDF5_VERSION:
			err = HDF5_write_TF_tf_all_channel(filename, elan_struct, measureNum, 0, elan_struct->tf.samp_nb, freqStart, freqNb, data_ptr);
			break;
		default:
			fprintf(stderr, "ERROR: unsupported version: %d.\n", elan_struct->version);
	}
	return(err);
}


/*--------------------------------------------------------------------------------------------------------------*/
/* ef_write_TF_timeblock_one_channel: write a time block for all frequencies and one channel.                   */
/* filename: name of the file to read datra from.                                                               */
/* elan_struct_t *elan_struct: Elan structure (read from header).                                               */
/* int measureNum: indice of measure to read (starting from 0).                                                 */
/* int chanNum: indice of channel (starting from 0).                                                            */
/* int sampStart: indice of 1st sample to read (starting from 0).                                               */
/* int sampNb: number of samples to read (starting from 0).                                                     */
/* void **data_ptr: array of samples to store data. If NULL, data_ptr is allocated in function (must be freed). */
/*                 If allocated in caller, size must be [elan_struct.tf.freq_nbxsampNb] double or double.        */
/* return value: number of samples actually read.                                                               */
/*--------------------------------------------------------------------------------------------------------------*/
int ef_write_TF_timeblock_one_channel(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum, int sampStart, int sampNb, void **data_ptr)
{
	int err=EF_NOERR;
	
	if (sampStart>=elan_struct->tf.samp_nb)
	{
		fprintf(stderr, "ERROR: bad value of 1st sample index (should be < %d).\n", elan_struct->tf.samp_nb);
		err = EF_ERR_BAD_PARM;
		return(err);
	}
	if ((sampStart+sampNb)>elan_struct->tf.samp_nb)
	{
		fprintf(stderr, "ERROR: bad value of sample number (should be < %d, 1st sample index is %d, number of samples is %d).\n", 
						elan_struct->tf.samp_nb-sampStart, sampStart, elan_struct->tf.samp_nb);
		err = EF_ERR_BAD_PARM;
		return(err);
	}
		
	switch (elan_struct->version)
	{
		case HIST_VERSION:
			fprintf(stderr, "Unsupported feature: ef_write_TF_timeblock_one_channel for original file format version.\n");
			break;
			
		case HDF5_VERSION:
			err = HDF5_write_TF_tf_one_channel(filename, elan_struct, measureNum, chanNum, sampStart, sampNb, 0, elan_struct->tf.freq_nb, data_ptr);
			break;
		default:
			fprintf(stderr, "ERROR: unsupported version: %d.\n", elan_struct->version);
	}
	return(err);
}


/*--------------------------------------------------------------------------------------------------------------*/
/* ef_write_TF_timeblock_all_channel: write a time block for all frequencies and all channels.                  */
/* filename: name of the file to read datra from.                                                               */
/* elan_struct_t *elan_struct: Elan structure (read from header).                                               */
/* int measureNum: indice of measure to read (starting from 0).                                                 */
/* int sampStart: indice of 1st sample to read (starting from 0).                                               */
/* int sampNb: number of samples to read (starting from 0).                                                     */
/* void ***data_ptr: array of samples to store data. If NULL, data_ptr is allocated in function (must be freed).*/
/*                 If allocated in caller, size must be [elan_struct.chan_nbxelan_struct.tf.freq_nbxsampNb]     */
/*                 double or double.                                                                             */
/* return value: number of samples actually read.                                                               */
/*--------------------------------------------------------------------------------------------------------------*/
int ef_write_TF_timeblock_all_channel(char *filename, elan_struct_t *elan_struct, int measureNum, int sampStart, int sampNb, void ***data_ptr)
{
	int err=EF_NOERR;
	
	if (sampStart>=elan_struct->tf.samp_nb)
	{
		fprintf(stderr, "ERROR: bad value of 1st sample index (should be < %d).\n", elan_struct->tf.samp_nb);
		err = EF_ERR_BAD_PARM;
		return(err);
	}
	if ((sampStart+sampNb)>elan_struct->tf.samp_nb)
	{
		fprintf(stderr, "ERROR: bad value of sample number (should be < %d, 1st sample index is %d, number of samples is %d).\n", 
						elan_struct->tf.samp_nb-sampStart, sampStart, elan_struct->tf.samp_nb);
		err = EF_ERR_BAD_PARM;
		return(err);
	}
		
	switch (elan_struct->version)
	{
		case HIST_VERSION:
			fprintf(stderr, "Unsupported feature: ef_write_TF_timeblock_all_channel for original file format version.\n");
			break;
			
		case HDF5_VERSION:
			err = HDF5_write_TF_tf_all_channel(filename, elan_struct, measureNum, sampStart, sampNb, 0, elan_struct->tf.freq_nb, data_ptr);
			break;
		default:
			fprintf(stderr, "ERROR: unsupported version: %d.\n", elan_struct->version);
	}
	return(err);
}


/*--------------------------------------------------------------------------------------------------------------*/
/* ef_write_TF_tfblock_one_channel: write a time-frequency block for one channel.                               */
/* filename: name of the file to read datra from.                                                               */
/* elan_struct_t *elan_struct: Elan structure (read from header).                                               */
/* int measureNum: indice of measure to read (starting from 0).                                                 */
/* int chanNum: indice of channel (starting from 0).                                                            */
/* int sampStart: indice of 1st sample to read (starting from 0).                                               */
/* int sampNb: number of samples to read (starting from 0).                                                     */
/* int freqStart: indice of 1st frequency to read (starting from 0).                                            */
/* int freqNb: number of frequencies to read (starting from 0).                                                 */
/* void **data_ptr: array of samples to store data. If NULL, data_ptr is allocated in function (must be freed). */
/*                 If allocated in caller, size must be [freqNbxsampNb] double or double.                        */
/* return value: number of samples actually read.                                                               */
/*--------------------------------------------------------------------------------------------------------------*/
int ef_write_TF_tfblock_one_channel(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum, int sampStart, int sampNb, int freqStart, int freqNb, void **data_ptr)
{
	int err=EF_NOERR;
	
	if (sampStart>=elan_struct->tf.samp_nb)
	{
		fprintf(stderr, "ERROR: bad value of 1st sample index (should be < %d).\n", elan_struct->tf.samp_nb);
		err = EF_ERR_BAD_PARM;
		return(err);
	}
	if ((sampStart+sampNb)>elan_struct->tf.samp_nb)
	{
		fprintf(stderr, "ERROR: bad value of sample number (should be < %d, 1st sample index is %d, number of samples is %d).\n", 
						elan_struct->tf.samp_nb-sampStart, sampStart, elan_struct->tf.samp_nb);
		err = EF_ERR_BAD_PARM;
		return(err);
	}
	if (freqStart>=elan_struct->tf.freq_nb)
	{
		fprintf(stderr, "ERROR: bad value of 1st frequency index (should be < %d).\n", elan_struct->tf.freq_nb);
		err = EF_ERR_BAD_PARM;
		return(err);
	}
	if ((freqStart+freqNb)>elan_struct->tf.freq_nb)
	{
		fprintf(stderr, "ERROR: bad value of frequency number (should be < %d, 1st frequency index is %d, number of frequencies is %d).\n", 
						elan_struct->tf.freq_nb-freqStart, freqStart, elan_struct->tf.freq_nb);
		err = EF_ERR_BAD_PARM;
		return(err);
	}
		
	switch (elan_struct->version)
	{
		case HIST_VERSION:
			fprintf(stderr, "Unsupported feature: ef_write_TF_tfblock_one_channel for original file format version.\n");
			break;
			
		case HDF5_VERSION:
			err = HDF5_write_TF_tf_one_channel(filename, elan_struct, measureNum, chanNum, sampStart, sampNb, freqStart, freqNb, data_ptr);

			break;
		default:
			fprintf(stderr, "ERROR: unsupported version: %d.\n", elan_struct->version);
	}
	return(err);
}


/*--------------------------------------------------------------------------------------------------------------*/
/* ef_write_TF_timeblock_all_channel: write a time block for all frequencies and all channels.                  */
/* filename: name of the file to read datra from.                                                               */
/* elan_struct_t *elan_struct: Elan structure (read from header).                                               */
/* int measureNum: indice of measure to read (starting from 0).                                                 */
/* int sampStart: indice of 1st sample to read (starting from 0).                                               */
/* int sampNb: number of samples to read (starting from 0).                                                     */
/* int freqStart: indice of 1st frequency to read (starting from 0).                                            */
/* int freqNb: number of frequencies to read (starting from 0).                                                 */
/* void ***data_ptr: array of samples to store data. If NULL, data_ptr is allocated in function (must be freed).*/
/*                 If allocated in caller, size must be [elan_struct.chan_nbxfreqNbxsampNb]                     */
/*                 double or double.                                                                             */
/* return value: number of samples actually read.                                                               */
/*--------------------------------------------------------------------------------------------------------------*/
int ef_write_TF_tfblock_all_channel(char *filename, elan_struct_t *elan_struct, int measureNum, int sampStart, int sampNb, int freqStart, int freqNb, void ***data_ptr)
{
	int err=EF_NOERR;
	
	if (sampStart>=elan_struct->tf.samp_nb)
	{
		fprintf(stderr, "ERROR: bad value of 1st sample index (should be < %d).\n", elan_struct->tf.samp_nb);
		err = EF_ERR_BAD_PARM;
		return(err);
	}
	if ((sampStart+sampNb)>elan_struct->tf.samp_nb)
	{
		fprintf(stderr, "ERROR: bad value of sample number (should be < %d, 1st sample index is %d, number of samples is %d).\n", 
						elan_struct->tf.samp_nb-sampStart, sampStart, elan_struct->tf.samp_nb);
		err = EF_ERR_BAD_PARM;
		return(err);
	}
	if (freqStart>=elan_struct->tf.freq_nb)
	{
		fprintf(stderr, "ERROR: bad value of 1st frequency index (should be < %d).\n", elan_struct->tf.freq_nb);
		err = EF_ERR_BAD_PARM;
		return(err);
	}
	if ((freqStart+freqNb)>elan_struct->tf.freq_nb)
	{
		fprintf(stderr, "ERROR: bad value of frequency number (should be < %d, 1st frequency index is %d, number of frequencies is %d).\n", 
						elan_struct->tf.freq_nb-freqStart, freqStart, elan_struct->tf.freq_nb);
		err = EF_ERR_BAD_PARM;
		return(err);
	}
		
	switch (elan_struct->version)
	{
		case HIST_VERSION:
			fprintf(stderr, "Unsupported feature: ef_write_TF_tfblock_all_channel for original file format version.\n");
			break;
			
		case HDF5_VERSION:
			err = HDF5_write_TF_tf_all_channel(filename, elan_struct, measureNum, sampStart, sampNb, freqStart, freqNb, data_ptr);

			break;
		default:
			fprintf(stderr, "ERROR: unsupported version: %d.\n", elan_struct->version);
	}
	return(err);
}



