/*--------------------------------------------------*/
/* Reading HDF5 ELAN files module.                  */
/* v: 1.00    Aut.: PEA                             */
/* CRNL / INSERM U1028 / CNRS UMR 5292 / Univ-Lyon1 */
/*--------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "elanfile.h"
#include "ef_hdf5common.h"



/***********************************************************/
/* HDF5_read_header: read Elan data header in HDF5 format. */
/***********************************************************/
int HDF5_read_header(char *filename, elan_struct_t *elan_struct)
{
	int err = 0, i_coord, i_val, i_chan, group_stat;
	hid_t f, elan_group, common_group, channel_dset, eeg_group, ep_group, tf_group, eeg_dataset = 0;
	hdf_dataspaces_t hdf_dataspaces;
	hdf_types_t hdf_types;
	hdf_chan_mem_t *hdf_chan_mem;
	hdf_coord_mem_t **ptr_coord_mem;
	float *ptr_float;

	/* Dataspaces and types initialization. */
	ef_HDF5create_basic_types_dataspaces(&hdf_dataspaces, &hdf_types);

	f = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT);
	if (f < 0)
	{
		fprintf(stderr, "ERROR: can't open file %s for reading.\n", filename);
		err = EF_ERR_OPEN_READ;
	}

	elan_group = H5Gopen(f, HDF5_ELAN_GROUP, H5P_DEFAULT);

	common_group = H5Gopen(elan_group, HDF5_COMMON_GROUP, H5P_DEFAULT);

	ef_HDF5open_read_attribute(common_group, HDF5_ENDIAN_ATTRIB, H5P_DEFAULT, H5T_NATIVE_INT, &elan_struct->endian);
	ef_HDF5open_read_attribute(common_group, HDF5_VERSION_ATTRIB, H5P_DEFAULT, CHOOSE_ENDIAN_INT32(elan_struct->endian), &elan_struct->version);
	ef_HDF5open_read_attribute(common_group, HDF5_RELEASE_ATTRIB, H5P_DEFAULT, CHOOSE_ENDIAN_INT32(elan_struct->endian), &elan_struct->release);
	ef_HDF5open_read_attribute(common_group, HDF5_COMMENT_ATTRIB, H5P_DEFAULT, hdf_types.str_comment_type, elan_struct->comment);
	ef_HDF5open_read_attribute(common_group, HDF5_CHANNELNB_ATTRIB, H5P_DEFAULT, CHOOSE_ENDIAN_INT32(elan_struct->endian), &elan_struct->chan_nb);
	ef_HDF5open_read_attribute(common_group, HDF5_MEASURENB_ATTRIB, H5P_DEFAULT, CHOOSE_ENDIAN_INT32(elan_struct->endian), &elan_struct->measure_channel_nb);

	/* Dataspaces and types initialization from data header. */
	ef_HDF5create_fromheader_types_dataspaces(elan_struct, &hdf_dataspaces, &hdf_types);

	/* Memory allocation for reading measure list. */
	ef_alloc_measure_label_list(elan_struct);

	ef_HDF5open_read_attribute(common_group, HDF5_MEASURELIST_ATTRIB, H5P_DEFAULT, hdf_types.str_label_type, elan_struct->measure_channel_label_list);

	H5Gclose(common_group);

	/* Channels dataset creation. */
	/*----------------------------*/
	hdf_chan_mem = (hdf_chan_mem_t*)calloc(elan_struct->chan_nb, sizeof(hdf_chan_mem_t));
	ELAN_CHECK_ERROR_ALLOC(hdf_chan_mem, "for reading channels infos in memory (from HDF5 objects).");

	channel_dset = H5Dopen2(elan_group, HDF5_CHANNEL_DSET, H5P_DEFAULT);
	H5Dread(channel_dset, hdf_types.chan_memtype, H5S_ALL, H5S_ALL, H5P_DEFAULT, hdf_chan_mem); /* Read the dataset data. */

																								/* Close the channel dataset. */
	H5Dclose(channel_dset);

	/* Dispatch data to Elan structure. */
	ef_alloc_channel_list(elan_struct);

	ptr_coord_mem = (hdf_coord_mem_t **)calloc(elan_struct->chan_nb, sizeof(hdf_coord_mem_t *));

	for (i_chan = 0; i_chan<elan_struct->chan_nb; i_chan++)
	{
		strcpy(elan_struct->chan_list[i_chan].lab, hdf_chan_mem[i_chan].lab);
		strcpy(elan_struct->chan_list[i_chan].type, hdf_chan_mem[i_chan].type);
		strcpy(elan_struct->chan_list[i_chan].unit, hdf_chan_mem[i_chan].unit);

		ptr_coord_mem[i_chan] = (hdf_coord_mem_t *)hdf_chan_mem[i_chan].coord_list.p;

		elan_struct->chan_list[i_chan].coord_nb = hdf_chan_mem[i_chan].coord_list.len;

		elan_struct->chan_list[i_chan].coord_list = (coord_t *)calloc(elan_struct->chan_list[i_chan].coord_nb, sizeof(coord_t));
		ELAN_CHECK_ERROR_ALLOC(elan_struct->chan_list[i_chan].coord_list, "for storing channel coordinates in memory (from HDF5 objects).");

		for (i_coord = 0; i_coord<elan_struct->chan_list[i_chan].coord_nb; i_coord++)
		{
			/*			strcpy(elan_struct->chan_list[i_chan].coord_list[i_coord].lab, hdf_chan_mem[i_chan].coord_list.p+(i_chan*sizeof(str_label_t)));*/
			strcpy(elan_struct->chan_list[i_chan].coord_list[i_coord].lab, ptr_coord_mem[i_chan][i_coord].coord_lab);
			elan_struct->chan_list[i_chan].coord_list[i_coord].val_nb = ptr_coord_mem[i_chan][i_coord].value_list.len;
			elan_struct->chan_list[i_chan].coord_list[i_coord].val_list = (float *)calloc(elan_struct->chan_list[i_chan].coord_list[i_coord].val_nb, sizeof(float));
			ELAN_CHECK_ERROR_ALLOC(elan_struct->chan_list[i_chan].coord_list[i_coord].val_list, "for storing channel coordinates in memory (from HDF5 objects).");
			ptr_float = ptr_coord_mem[i_chan][i_coord].value_list.p;
			for (i_val = 0; i_val<elan_struct->chan_list[i_chan].coord_list[i_coord].val_nb; i_val++)
			{
				elan_struct->chan_list[i_chan].coord_list[i_coord].val_list[i_val] = ptr_float[i_val];
			}
		}
	}

	/* EEG group and dataset creation if needed. */
	/*-------------------------------------------*/
	elan_struct->has_eeg = EF_NO;
	group_stat = H5Lexists(elan_group, HDF5_EEG_GROUP, H5P_DEFAULT);
	if (group_stat > 0)
	{
		elan_struct->has_eeg = EF_YES;
		eeg_group = H5Gopen(elan_group, HDF5_EEG_GROUP, H5P_DEFAULT);
		ef_HDF5open_read_attribute(eeg_group, HDF5_DATATYPE_ATTRIB, H5P_DEFAULT, CHOOSE_ENDIAN_INT32(elan_struct->endian), &elan_struct->eeg.data_type);
		ef_HDF5open_read_attribute(eeg_group, HDF5_EEG_SAMPNB_ATTRIB, H5P_DEFAULT, CHOOSE_ENDIAN_INT32(elan_struct->endian), &elan_struct->eeg.samp_nb);
		ef_HDF5open_read_attribute(eeg_group, HDF5_EEG_SAMPFREQ_ATTRIB, H5P_DEFAULT, CHOOSE_ENDIAN_FLT32(elan_struct->endian), &elan_struct->eeg.sampling_freq);

		eeg_dataset = H5Dopen2(eeg_group, HDF5_EEG_CONT_DSET, H5P_DEFAULT);
		if (eeg_dataset > 0)
		{
			elan_struct->eeg.flag_cont_epoch = EF_EEG_CONTINUOUS;
			H5Dclose(eeg_dataset);

			/* Read continuous specific attributes. */
			ef_HDF5open_read_attribute(eeg_group, HDF5_EEG_EVENTNB_ATTRIB, H5P_DEFAULT, CHOOSE_ENDIAN_INT32(elan_struct->endian), &elan_struct->eeg.event_nb);
			ef_alloc_eeg_array_events(elan_struct);
			if (elan_struct->eeg.event_nb > 0) {
				ef_HDF5open_read_attribute(eeg_group, HDF5_EEG_EVENTCODE_ATTRIB, H5P_DEFAULT, CHOOSE_ENDIAN_INT32(elan_struct->endian), elan_struct->eeg.event_code_list);
				ef_HDF5open_read_attribute(eeg_group, HDF5_EEG_EVENTTIME_ATTRIB, H5P_DEFAULT, CHOOSE_ENDIAN_INT32(elan_struct->endian), elan_struct->eeg.event_sample_list);
			}
		}
		else
		{
			eeg_dataset = H5Dopen2(eeg_group, HDF5_EEG_EPOCH_DSET, H5P_DEFAULT);
			if (eeg_dataset > 0)
			{
				elan_struct->eeg.flag_cont_epoch = EF_EEG_EPOCH;
				H5Dclose(eeg_dataset);
			}
			else
			{
				fprintf(stderr, "ERROR: unhandled EEG type (neither CONTINUOUS, nor EPOCH) or no dataset.\n");
				return(EF_ERR_READ_HEADER);
			}
		}
		H5Gclose(eeg_group);
	}

	/* EP group and dataset creation if needed. */
	/*-------------------------------------------*/
	elan_struct->has_ep = EF_NO;
	group_stat = H5Lexists(elan_group, HDF5_EP_GROUP, H5P_DEFAULT);
	if (group_stat > 0)
	{
		elan_struct->has_ep = EF_YES;
		ep_group = H5Gopen(elan_group, HDF5_EP_GROUP, H5P_DEFAULT);
		ef_HDF5open_read_attribute(ep_group, HDF5_DATATYPE_ATTRIB, H5P_DEFAULT, CHOOSE_ENDIAN_INT32(elan_struct->endian), &elan_struct->ep.data_type);
		ef_HDF5open_read_attribute(ep_group, HDF5_EP_SAMPNB_ATTRIB, H5P_DEFAULT, CHOOSE_ENDIAN_INT32(elan_struct->endian), &elan_struct->ep.samp_nb);
		ef_HDF5open_read_attribute(ep_group, HDF5_EP_SAMPFREQ_ATTRIB, H5P_DEFAULT, CHOOSE_ENDIAN_FLT32(elan_struct->endian), &elan_struct->ep.sampling_freq);
		ef_HDF5open_read_attribute(ep_group, HDF5_EP_PRESTIM_ATTRIB, H5P_DEFAULT, CHOOSE_ENDIAN_INT32(elan_struct->endian), &elan_struct->ep.prestim_samp_nb);
		ef_HDF5open_read_attribute(ep_group, HDF5_EP_EVENTCODE_ATTRIB, H5P_DEFAULT, CHOOSE_ENDIAN_INT32(elan_struct->endian), &elan_struct->ep.event_code);
		ef_HDF5open_read_attribute(ep_group, HDF5_EP_EVENTNB_ATTRIB, H5P_DEFAULT, CHOOSE_ENDIAN_INT32(elan_struct->endian), &elan_struct->ep.event_nb);

		ef_alloc_ep_array_otherevents(elan_struct);
		ef_HDF5open_read_attribute(ep_group, HDF5_EPTF_OTHEREVENTNB_ATTRIB, H5P_DEFAULT, CHOOSE_ENDIAN_INT32(elan_struct->endian), elan_struct->ep.other_events_nb);
		ef_alloc_ep_array_otherevents_list(elan_struct);
		if (elan_struct->ep.other_events_list != NULL) {
			ef_HDF5open_read_attribute(ep_group, HDF5_EPTF_OTHEREVENTLIST_ATTRIB, H5P_DEFAULT, CHOOSE_ENDIAN_INT32(elan_struct->endian), elan_struct->ep.other_events_list);
		}
		H5Gclose(ep_group);
	}

	/* TF group and dataset creation if needed. */
	/*-------------------------------------------*/
	elan_struct->has_tf = EF_NO;
	group_stat = H5Lexists(elan_group, HDF5_TF_GROUP, H5P_DEFAULT);
	if (group_stat > 0)
	{
		elan_struct->has_tf = EF_YES;
		tf_group = H5Gopen(elan_group, HDF5_TF_GROUP, H5P_DEFAULT);
		ef_HDF5open_read_attribute(tf_group, HDF5_DATATYPE_ATTRIB, H5P_DEFAULT, CHOOSE_ENDIAN_INT32(elan_struct->endian), &elan_struct->tf.data_type);
		ef_HDF5open_read_attribute(tf_group, HDF5_TF_SAMPNB_ATTRIB, H5P_DEFAULT, CHOOSE_ENDIAN_INT32(elan_struct->endian), &elan_struct->tf.samp_nb);
		ef_HDF5open_read_attribute(tf_group, HDF5_TF_SAMPFREQ_ATTRIB, H5P_DEFAULT, CHOOSE_ENDIAN_FLT32(elan_struct->endian), &elan_struct->tf.sampling_freq);
		ef_HDF5open_read_attribute(tf_group, HDF5_TF_PRESTIM_ATTRIB, H5P_DEFAULT, CHOOSE_ENDIAN_INT32(elan_struct->endian), &elan_struct->tf.prestim_samp_nb);
		ef_HDF5open_read_attribute(tf_group, HDF5_TF_EVENTCODE_ATTRIB, H5P_DEFAULT, CHOOSE_ENDIAN_INT32(elan_struct->endian), &elan_struct->tf.event_code);
		ef_HDF5open_read_attribute(tf_group, HDF5_TF_EVENTNB_ATTRIB, H5P_DEFAULT, CHOOSE_ENDIAN_INT32(elan_struct->endian), &elan_struct->tf.event_nb);
		ef_HDF5open_read_attribute(tf_group, HDF5_TF_WAVELETTYPE_ATTRIB, H5P_DEFAULT, CHOOSE_ENDIAN_INT32(elan_struct->endian), &elan_struct->tf.wavelet_type);
		ef_HDF5open_read_attribute(tf_group, HDF5_TF_BLACKMAN_ATTRIB, H5P_DEFAULT, CHOOSE_ENDIAN_INT32(elan_struct->endian), &elan_struct->tf.blackman_window);
		ef_HDF5open_read_attribute(tf_group, HDF5_TF_FREQUENCYNB_ATTRIB, H5P_DEFAULT, CHOOSE_ENDIAN_INT32(elan_struct->endian), &elan_struct->tf.freq_nb);

		ef_alloc_tf_array_otherevents(elan_struct);
		ef_HDF5open_read_attribute(tf_group, HDF5_EPTF_OTHEREVENTNB_ATTRIB, H5P_DEFAULT, CHOOSE_ENDIAN_INT32(elan_struct->endian), elan_struct->tf.other_events_nb);
		ef_alloc_tf_array_otherevents_list(elan_struct);
		if (elan_struct->tf.other_events_list != NULL) {
			ef_HDF5open_read_attribute(tf_group, HDF5_EPTF_OTHEREVENTLIST_ATTRIB, H5P_DEFAULT, CHOOSE_ENDIAN_INT32(elan_struct->endian), elan_struct->tf.other_events_list);
		}

		ef_alloc_tf_array_byfreq(elan_struct);
		ef_HDF5create_tf_types_dataspaces(elan_struct, &hdf_dataspaces);
		ef_HDF5open_read_attribute(tf_group, HDF5_TF_FREQUENCYLIST_ATTRIB, H5P_DEFAULT, CHOOSE_ENDIAN_FLT32(elan_struct->endian), elan_struct->tf.freq_array);
		ef_HDF5open_read_attribute(tf_group, HDF5_TF_WAVELETCHAR_ATTRIB, H5P_DEFAULT, CHOOSE_ENDIAN_FLT32(elan_struct->endian), elan_struct->tf.wavelet_char);
		H5Gclose(tf_group);
	}

	free(hdf_chan_mem);
	for (i_chan = 0; i_chan<elan_struct->chan_nb; i_chan++)
	{
		for (i_coord = 0; i_coord<elan_struct->chan_list[i_chan].coord_nb; i_coord++)
		{
			free(ptr_coord_mem[i_chan][i_coord].value_list.p);
		}
		free(ptr_coord_mem[i_chan]);
	}
	free(ptr_coord_mem);

	H5Gclose(elan_group);
	H5Fclose(f);

	ef_HDF5close_types_dataspaces(&hdf_dataspaces, &hdf_types);

	return(err);
}


/****************************************************************************************/
/* HDF5_read_data_all_channels: read Elan data for all channels at once in HDF5 format. */
/****************************************************************************************/
int HDF5_read_data_all_channels(char *filename, elan_struct_t *elan_struct)
{
	hid_t f, elan_group, eeg_group, eeg_dataset, ep_group, ep_dataset, tf_group, tf_dataset;
	int err = EF_NOERR;

	f = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT);
	if (f < 0)
	{
		fprintf(stderr, "ERROR: can't open file %s for reading.\n", filename);
		err = EF_ERR_OPEN_READ;
	}
	elan_group = H5Gopen(f, HDF5_ELAN_GROUP, H5P_DEFAULT);

	/* EEG group and dataset reading if needed. */
	/*------------------------------------------*/
	if (elan_struct->has_eeg == EF_YES)
	{
		eeg_group = H5Gopen(elan_group, HDF5_EEG_GROUP, H5P_DEFAULT);
		switch (elan_struct->eeg.flag_cont_epoch)
		{
		case EF_EEG_CONTINUOUS:
			eeg_dataset = H5Dopen2(eeg_group, HDF5_EEG_CONT_DSET, H5P_DEFAULT);

			switch (elan_struct->eeg.data_type)
			{
			case EF_DATA_FLOAT:
				H5Dread(eeg_dataset, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, elan_struct->eeg.flat_float); /* Read the dataset data. */
				break;
			case EF_DATA_DOUBLE:
				H5Dread(eeg_dataset, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, elan_struct->eeg.flat_double); /* Read the dataset data. */
				break;
			}
			H5Dclose(eeg_dataset);
			break;
		case EF_EEG_EPOCH:
			fprintf(stderr, "ERROR: epoched EEG format not handled.\n");
			break;
		default:
			fprintf(stderr, "ERROR: EEG format %d not handled.\n", elan_struct->eeg.flag_cont_epoch);
		}
		H5Gclose(eeg_group);
	}

	/* EP group and dataset reading if needed. */
	/*-----------------------------------------*/
	if (elan_struct->has_ep == EF_YES)
	{
		ep_group = H5Gopen(elan_group, HDF5_EP_GROUP, H5P_DEFAULT);
		ep_dataset = H5Dopen2(ep_group, HDF5_EP_EPOCH_DSET, H5P_DEFAULT);

		switch (elan_struct->ep.data_type)
		{
		case EF_DATA_FLOAT:
			H5Dread(ep_dataset, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, elan_struct->ep.flat_float); /* Read the dataset data. */
			break;
		case EF_DATA_DOUBLE:
			H5Dread(ep_dataset, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, elan_struct->ep.flat_double); /* Read the dataset data. */
			break;
		}
		H5Dclose(ep_dataset);
		H5Gclose(ep_group);
	}

	/* TF group and dataset reading if needed. */
	/*-----------------------------------------*/
	if (elan_struct->has_tf == EF_YES)
	{
		tf_group = H5Gopen(elan_group, HDF5_TF_GROUP, H5P_DEFAULT);
		tf_dataset = H5Dopen2(tf_group, HDF5_TF_ARRAY_DSET, H5P_DEFAULT);

		switch (elan_struct->tf.data_type)
		{
		case EF_DATA_FLOAT:
			H5Dread(tf_dataset, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, elan_struct->tf.flat_float); /* Read the dataset data. */
			break;
		case EF_DATA_DOUBLE:
			H5Dread(tf_dataset, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, elan_struct->tf.flat_double); /* Read the dataset data. */
			break;
		}
		H5Dclose(tf_dataset);
		H5Gclose(tf_group);
	}

	H5Gclose(elan_group);
	/* Terminate access to the file. */
	H5Fclose(f);

	return(err);
}


/****************************************************************************************/
/* HDF5_read_data_one_channel: read one channel of data (index chanNum) for one measure */
/*                                  (index measureNum) in HDF5 format.                  */
/****************************************************************************************/
int HDF5_read_data_one_channel(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum)
{
	hid_t f, elan_group, eeg_group, eeg_dataset, file_dataspace, mem_dataspace, ep_group, ep_dataset, tf_group, tf_dataset;
	hsize_t *dims, *offset, val[2];
	int err = EF_NOERR;

	f = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT);
	if (f < 0)
	{
		fprintf(stderr, "ERROR: can't open file %s for reading.\n", filename);
		err = EF_ERR_OPEN_READ;
	}
	elan_group = H5Gopen(f, HDF5_ELAN_GROUP, H5P_DEFAULT);

	/* EEG group and dataset reading if needed. */
	/*------------------------------------------*/
	if (elan_struct->has_eeg == EF_YES)
	{
		eeg_group = H5Gopen(elan_group, HDF5_EEG_GROUP, H5P_DEFAULT);
		switch (elan_struct->eeg.flag_cont_epoch)
		{
		case EF_EEG_CONTINUOUS:
			eeg_dataset = H5Dopen2(eeg_group, HDF5_EEG_CONT_DSET, H5P_DEFAULT);
			file_dataspace = H5Dget_space(eeg_dataset);    /* dataspace identifier */

														   /* Hyperslab creation for one measure, one channel, all samples. */
			offset = (hsize_t *)calloc(3, sizeof(hsize_t));
			ELAN_CHECK_ERROR_ALLOC(offset, "for reading one channel of continuous EEG (offset hyperslab).");
			offset[0] = measureNum;
			offset[1] = chanNum;
			offset[2] = 0;
			val[0] = elan_struct->eeg.samp_nb;
			dims = (hsize_t *)calloc(3, sizeof(hsize_t));
			ELAN_CHECK_ERROR_ALLOC(dims, "for reading one channel of continuous EEG (dims hyperslab).");
			dims[0] = 1;
			dims[1] = 1;
			dims[2] = val[0];

			mem_dataspace = H5Screate_simple(1, val, NULL);

			err = H5Sselect_hyperslab(file_dataspace, H5S_SELECT_SET, offset, NULL, dims, NULL);
			switch (elan_struct->eeg.data_type)
			{
			case EF_DATA_FLOAT:
				H5Dread(eeg_dataset, H5T_NATIVE_FLOAT, mem_dataspace, file_dataspace, H5P_DEFAULT, elan_struct->eeg.data_float[measureNum][chanNum]); /* Read the dataset data. */
				break;
			case EF_DATA_DOUBLE:
				H5Dread(eeg_dataset, H5T_NATIVE_DOUBLE, mem_dataspace, file_dataspace, H5P_DEFAULT, elan_struct->eeg.data_double[measureNum][chanNum]); /* Read the dataset data. */
				break;
			}
			H5Dclose(eeg_dataset);
			H5Sclose(mem_dataspace);
			free(dims);
			free(offset);
			break;
		case EF_EEG_EPOCH:
			fprintf(stderr, "ERROR: epoched EEG format not handled.\n");
			break;
		default:
			fprintf(stderr, "ERROR: EEG format %d not handled.\n", elan_struct->eeg.flag_cont_epoch);
		}
		H5Gclose(eeg_group);
	}

	/* EP group and dataset reading if needed. */
	/*-----------------------------------------*/
	if (elan_struct->has_ep == EF_YES)
	{
		ep_group = H5Gopen(elan_group, HDF5_EP_GROUP, H5P_DEFAULT);
		ep_dataset = H5Dopen2(ep_group, HDF5_EP_EPOCH_DSET, H5P_DEFAULT);
		file_dataspace = H5Dget_space(ep_dataset);    /* dataspace identifier */

													  /* Hyperslab creation for one measure, one channel, all samples. */
		offset = (hsize_t *)calloc(3, sizeof(hsize_t));
		ELAN_CHECK_ERROR_ALLOC(offset, "for reading one channel of EP (offset hyperslab).");
		offset[0] = measureNum;
		offset[1] = chanNum;
		offset[2] = 0;
		val[0] = elan_struct->ep.samp_nb;
		dims = (hsize_t *)calloc(3, sizeof(hsize_t));
		ELAN_CHECK_ERROR_ALLOC(dims, "for reading one channel of EP (dims hyperslab).");
		dims[0] = 1;
		dims[1] = 1;
		dims[2] = val[0];

		mem_dataspace = H5Screate_simple(1, val, NULL);

		err = H5Sselect_hyperslab(file_dataspace, H5S_SELECT_SET, offset, NULL, dims, NULL);
		switch (elan_struct->ep.data_type)
		{
		case EF_DATA_FLOAT:
			H5Dread(ep_dataset, H5T_NATIVE_FLOAT, mem_dataspace, file_dataspace, H5P_DEFAULT, elan_struct->ep.data_float[measureNum][chanNum]); /* Read the dataset data. */
			break;
		case EF_DATA_DOUBLE:
			H5Dread(ep_dataset, H5T_NATIVE_DOUBLE, mem_dataspace, file_dataspace, H5P_DEFAULT, elan_struct->ep.data_double[measureNum][chanNum]); /* Read the dataset data. */
			break;
		}
		H5Dclose(ep_dataset);
		H5Gclose(ep_group);
		H5Sclose(mem_dataspace);
		free(dims);
		free(offset);
	}

	/* TF group and dataset reading if needed. */
	/*-----------------------------------------*/
	if (elan_struct->has_tf == EF_YES)
	{
		tf_group = H5Gopen(elan_group, HDF5_TF_GROUP, H5P_DEFAULT);
		tf_dataset = H5Dopen2(tf_group, HDF5_TF_ARRAY_DSET, H5P_DEFAULT);

		file_dataspace = H5Dget_space(tf_dataset);    /* dataspace identifier */

													  /* Hyperslab creation for one measure, one channel, all frequencies, all samples. */
		offset = (hsize_t *)calloc(4, sizeof(hsize_t));
		ELAN_CHECK_ERROR_ALLOC(offset, "for reading one channel of TF (offset hyperslab).");
		offset[0] = measureNum;
		offset[1] = chanNum;
		offset[2] = 0;
		offset[3] = 0;
		dims = (hsize_t *)calloc(4, sizeof(hsize_t));
		ELAN_CHECK_ERROR_ALLOC(dims, "for reading one channel of TF (dims hyperslab).");
		val[0] = elan_struct->tf.freq_nb;
		val[1] = elan_struct->tf.samp_nb;
		dims[0] = 1;
		dims[1] = 1;
		dims[2] = val[0];
		dims[3] = val[1];

		mem_dataspace = H5Screate_simple(2, val, NULL);

		err = H5Sselect_hyperslab(file_dataspace, H5S_SELECT_SET, offset, NULL, dims, NULL);
		switch (elan_struct->tf.data_type)
		{
		case EF_DATA_FLOAT:
			H5Dread(tf_dataset, H5T_NATIVE_FLOAT, mem_dataspace, file_dataspace, H5P_DEFAULT, elan_struct->tf.data_float[measureNum][chanNum][0]); /* Read the dataset data. */
			break;
		case EF_DATA_DOUBLE:
			H5Dread(tf_dataset, H5T_NATIVE_DOUBLE, mem_dataspace, file_dataspace, H5P_DEFAULT, elan_struct->tf.data_double[measureNum][chanNum][0]); /* Read the dataset data. */
			break;
		}
		H5Dclose(tf_dataset);
		H5Gclose(tf_group);
		free(dims);
		free(offset);
	}

	H5Gclose(elan_group);
	/* Terminate access to the file. */
	H5Fclose(f);

	return(err);
}


/****************************************************************************************************************/
/* HDF5_read_EEG_data_sampblock_one_chan: read an EEG data block for one channel and store in data_ptr.         */
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
int HDF5_read_EEG_data_sampblock_one_chan(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum, int sampStart, int sampNb, void **data_ptr)
{
	float *flt_ptr = NULL;
	double *dbl_ptr = NULL;
	hid_t f, elan_group, eeg_group, eeg_dataset, file_dataspace, mem_dataspace;
	hsize_t *dims, *offset, val;

	if (elan_struct->has_eeg == EF_YES)
	{
		if (*data_ptr == NULL)
		{
			switch (elan_struct->eeg.data_type)
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
			switch (elan_struct->eeg.data_type)
			{
			case EF_DATA_FLOAT:
				flt_ptr = *data_ptr;
				break;
			case EF_DATA_DOUBLE:
				dbl_ptr = *data_ptr;
				break;
			}
		}

		f = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT);
		if (f < 0)
		{
			fprintf(stderr, "ERROR: can't open file %s for reading.\n", filename);
			return(EF_ERR_OPEN_READ);
		}
		elan_group = H5Gopen(f, HDF5_ELAN_GROUP, H5P_DEFAULT);

		/* EEG group and dataset reading. */
		/*--------------------------------*/
		eeg_group = H5Gopen(elan_group, HDF5_EEG_GROUP, H5P_DEFAULT);
		switch (elan_struct->eeg.flag_cont_epoch)
		{
		case EF_EEG_CONTINUOUS:
			eeg_dataset = H5Dopen2(eeg_group, HDF5_EEG_CONT_DSET, H5P_DEFAULT);
			file_dataspace = H5Dget_space(eeg_dataset);    /* dataspace identifier */

														   /* Hyperslab creation for one measure, one channel, some samples. */
			offset = (hsize_t *)calloc(3, sizeof(hsize_t));
			ELAN_CHECK_ERROR_ALLOC(offset, "for reading samples of one channel of continuous EEG (offset hyperslab).");
			offset[0] = measureNum;
			offset[1] = chanNum;
			offset[2] = sampStart;
			dims = (hsize_t *)calloc(3, sizeof(hsize_t));
			ELAN_CHECK_ERROR_ALLOC(dims, "for reading samples of one channel of continuous EEG (dims hyperslab).");
			dims[0] = 1;
			dims[1] = 1;
			dims[2] = sampNb;

			val = sampNb;
			mem_dataspace = H5Screate_simple(1, &val, NULL);

			H5Sselect_hyperslab(file_dataspace, H5S_SELECT_SET, offset, NULL, dims, NULL);
			switch (elan_struct->eeg.data_type)
			{
			case EF_DATA_FLOAT:
				H5Dread(eeg_dataset, H5T_NATIVE_FLOAT, mem_dataspace, file_dataspace, H5P_DEFAULT, flt_ptr); /* Read the dataset data. */
				break;
			case EF_DATA_DOUBLE:
				H5Dread(eeg_dataset, H5T_NATIVE_DOUBLE, mem_dataspace, file_dataspace, H5P_DEFAULT, dbl_ptr); /* Read the dataset data. */
				break;
			}
			H5Dclose(eeg_dataset);
			H5Sclose(mem_dataspace);
			free(dims);
			free(offset);
			break;
		case EF_EEG_EPOCH:
			fprintf(stderr, "ERROR: epoched EEG format not handled.\n");
			break;
		default:
			fprintf(stderr, "ERROR: EEG format %d not handled.\n", elan_struct->eeg.flag_cont_epoch);
		}
		H5Gclose(eeg_group);
		H5Gclose(elan_group);

		/* Terminate access to the file. */
		H5Fclose(f);

		return(sampNb);
	}
	else
	{
		return(0);
	}
}


/*****************************************************************************************************************/
/* HDF5_read_EEG_data_sampblock_all_chan: read an EEG data block for all channel and store in data_ptr.          */
/* char *filename: file to read from.                                                                            */
/* elan_struct_t *elan_struct: Elan structure (read from header).                                                */
/* int measureNum: indice of measure to read (starting from 0).                                                  */
/* int sampStart: indice of first sample to read (starting from 0).                                              */
/* int sampNb: number of samples to read.                                                                        */
/* void **data_ptr: array of samples to store data. If NULL, data_ptr is allocated in function (must be freed).  */
/*                 If allocated in caller, size must be [elan_struct.chan_nbxsampNb] float or double.            */
/* return value: number of samples actually read.                                                                */
/*****************************************************************************************************************/
int HDF5_read_EEG_data_sampblock_all_chan(char *filename, elan_struct_t *elan_struct, int measureNum, int sampStart, int sampNb, void ***data_ptr)
{
	float **flt_ptr = NULL;
	double **dbl_ptr = NULL;
	hid_t f, elan_group, eeg_group, eeg_dataset, file_dataspace, mem_dataspace;
	hsize_t *dims, *offset, val[2];

	if (elan_struct->has_eeg == EF_YES)
	{
		if (*data_ptr == NULL)
		{
			switch (elan_struct->eeg.data_type)
			{
			case EF_DATA_FLOAT:
				flt_ptr = (float **)ef_alloc_array_2d(elan_struct->chan_nb, sampNb, sizeof(float));
				*data_ptr = (void *)flt_ptr;
				break;
			case EF_DATA_DOUBLE:
				dbl_ptr = (double **)ef_alloc_array_2d(elan_struct->chan_nb, sampNb, sizeof(double));
				*data_ptr = (void *)dbl_ptr;
				break;
			}
		}
		else
		{
			switch (elan_struct->eeg.data_type)
			{
			case EF_DATA_FLOAT:
				flt_ptr = (float **)*data_ptr;
				break;
			case EF_DATA_DOUBLE:
				dbl_ptr = (double **)*data_ptr;
				break;
			}
		}

		f = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT);
		if (f < 0)
		{
			fprintf(stderr, "ERROR: can't open file %s for reading.\n", filename);
			return(EF_ERR_OPEN_READ);
		}
		elan_group = H5Gopen(f, HDF5_ELAN_GROUP, H5P_DEFAULT);

		/* EEG group and dataset reading. */
		/*--------------------------------*/
		eeg_group = H5Gopen(elan_group, HDF5_EEG_GROUP, H5P_DEFAULT);
		switch (elan_struct->eeg.flag_cont_epoch)
		{
		case EF_EEG_CONTINUOUS:
			eeg_dataset = H5Dopen2(eeg_group, HDF5_EEG_CONT_DSET, H5P_DEFAULT);
			file_dataspace = H5Dget_space(eeg_dataset);    /* dataspace identifier */

														   /* Hyperslab creation for one measure, one channel, some samples. */
			offset = (hsize_t *)calloc(3, sizeof(hsize_t));
			ELAN_CHECK_ERROR_ALLOC(offset, "for reading samples of all channels of continuous EEG (offset hyperslab).");
			offset[0] = measureNum;
			offset[1] = 0;
			offset[2] = sampStart;
			dims = (hsize_t *)calloc(3, sizeof(hsize_t));
			ELAN_CHECK_ERROR_ALLOC(dims, "for reading samples of all channels of continuous EEG (dims hyperslab).");
			dims[0] = 1;
			dims[1] = elan_struct->chan_nb;
			dims[2] = sampNb;

			H5Sselect_hyperslab(file_dataspace, H5S_SELECT_SET, offset, NULL, dims, NULL);

			val[0] = elan_struct->chan_nb;
			val[1] = sampNb;
			mem_dataspace = H5Screate_simple(2, val, NULL);

			switch (elan_struct->eeg.data_type)
			{
			case EF_DATA_FLOAT:
				H5Dread(eeg_dataset, H5T_NATIVE_FLOAT, mem_dataspace, file_dataspace, H5P_DEFAULT, &flt_ptr[0][0]); /* Read the dataset data. */
				break;
			case EF_DATA_DOUBLE:
				H5Dread(eeg_dataset, H5T_NATIVE_DOUBLE, mem_dataspace, file_dataspace, H5P_DEFAULT, &dbl_ptr[0][0]); /* Read the dataset data. */
				break;
			}
			H5Dclose(eeg_dataset);
			H5Sclose(mem_dataspace);
			free(dims);
			free(offset);
			break;
		case EF_EEG_EPOCH:
			fprintf(stderr, "ERROR: epoched EEG format not handled.\n");
			break;
		default:
			fprintf(stderr, "ERROR: EEG format %d not handled.\n", elan_struct->eeg.flag_cont_epoch);
		}
		H5Gclose(eeg_group);
		H5Gclose(elan_group);

		/* Terminate access to the file. */
		H5Fclose(f);

		return(sampNb);
	}
	else
	{
		return(0);
	}
}


/****************************************************************************************************************/
/* HDF5_read_EP_data_sampblock_one_chan: read an EP data block for one channel and store in data_ptr.           */
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
int HDF5_read_EP_data_sampblock_one_chan(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum, int sampStart, int sampNb, void **data_ptr)
{
	float *flt_ptr = NULL;
	double *dbl_ptr = NULL;
	hid_t f, elan_group, ep_group, ep_dataset, file_dataspace, mem_dataspace;
	hsize_t *dims, *offset, val;

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

		f = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT);
		if (f < 0)
		{
			fprintf(stderr, "ERROR: can't open file %s for reading.\n", filename);
			return(EF_ERR_OPEN_READ);
		}
		elan_group = H5Gopen(f, HDF5_ELAN_GROUP, H5P_DEFAULT);

		/* EP group and dataset reading. */
		/*--------------------------------*/
		ep_group = H5Gopen(elan_group, HDF5_EP_GROUP, H5P_DEFAULT);
		ep_dataset = H5Dopen2(ep_group, HDF5_EP_EPOCH_DSET, H5P_DEFAULT);
		file_dataspace = H5Dget_space(ep_dataset);    /* dataspace identifier */

													  /* Hyperslab creation for one measure, one channel, some samples. */
		offset = (hsize_t *)calloc(3, sizeof(hsize_t));
		ELAN_CHECK_ERROR_ALLOC(offset, "for reading samples of one channel of continuous EP (offset hyperslab).");
		offset[0] = measureNum;
		offset[1] = chanNum;
		offset[2] = sampStart;
		dims = (hsize_t *)calloc(3, sizeof(hsize_t));
		ELAN_CHECK_ERROR_ALLOC(dims, "for reading samples of one channel of continuous EP (dims hyperslab).");
		dims[0] = 1;
		dims[1] = 1;
		dims[2] = sampNb;

		val = sampNb;
		mem_dataspace = H5Screate_simple(1, &val, NULL);

		H5Sselect_hyperslab(file_dataspace, H5S_SELECT_SET, offset, NULL, dims, NULL);
		switch (elan_struct->ep.data_type)
		{
		case EF_DATA_FLOAT:
			H5Dread(ep_dataset, H5T_NATIVE_FLOAT, mem_dataspace, file_dataspace, H5P_DEFAULT, flt_ptr); /* Read the dataset data. */
			break;
		case EF_DATA_DOUBLE:
			H5Dread(ep_dataset, H5T_NATIVE_DOUBLE, mem_dataspace, file_dataspace, H5P_DEFAULT, dbl_ptr); /* Read the dataset data. */
			break;
		}
		H5Dclose(ep_dataset);
		H5Gclose(ep_group);
		H5Sclose(mem_dataspace);
		free(dims);
		free(offset);

		H5Gclose(elan_group);

		/* Terminate access to the file. */
		H5Fclose(f);

		return(sampNb);
	}
	else
	{
		return(0);
	}
}


/*****************************************************************************************************************/
/* HDF5_read_EP_data_sampblock_all_chan: read an EP data block for all channel and store in data_ptr.            */
/* char *filename: file to read from.                                                                            */
/* elan_struct_t *elan_struct: Elan structure (read from header).                                                */
/* int measureNum: indice of measure to read (starting from 0).                                                  */
/* int sampStart: indice of first sample to read (starting from 0).                                              */
/* int sampNb: number of samples to read.                                                                        */
/* void **data_ptr: array of samples to store data. If NULL, data_ptr is allocated in function (must be freed).  */
/*                 If allocated in caller, size must be [elan_struct.chan_nbxsampNb] float or double.            */
/* return value: number of samples actually read.                                                                */
/*****************************************************************************************************************/
int HDF5_read_EP_data_sampblock_all_chan(char *filename, elan_struct_t *elan_struct, int measureNum, int sampStart, int sampNb, void ***data_ptr)
{
	float **flt_ptr = NULL;
	double **dbl_ptr = NULL;
	hid_t f, elan_group, ep_group, ep_dataset, file_dataspace, mem_dataspace;
	hsize_t *dims, *offset, val[2];

	if (elan_struct->has_ep == EF_YES)
	{
		if (*data_ptr == NULL)
		{
			switch (elan_struct->ep.data_type)
			{
			case EF_DATA_FLOAT:
				flt_ptr = (float **)ef_alloc_array_2d(elan_struct->chan_nb, sampNb, sizeof(float));
				*data_ptr = (void *)flt_ptr;
				break;
			case EF_DATA_DOUBLE:
				dbl_ptr = (double **)ef_alloc_array_2d(elan_struct->chan_nb, sampNb, sizeof(double));
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

		f = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT);
		if (f < 0)
		{
			fprintf(stderr, "ERROR: can't open file %s for reading.\n", filename);
			return(EF_ERR_OPEN_READ);
		}
		elan_group = H5Gopen(f, HDF5_ELAN_GROUP, H5P_DEFAULT);

		/* EP group and dataset reading. */
		/*-------------------------------*/
		ep_group = H5Gopen(elan_group, HDF5_EP_GROUP, H5P_DEFAULT);
		ep_dataset = H5Dopen2(ep_group, HDF5_EP_EPOCH_DSET, H5P_DEFAULT);
		file_dataspace = H5Dget_space(ep_dataset);    /* dataspace identifier */

													  /* Hyperslab creation for one measure, one channel, some samples. */
		offset = (hsize_t *)calloc(3, sizeof(hsize_t));
		ELAN_CHECK_ERROR_ALLOC(offset, "for reading samples of all channels of continuous EP (offset hyperslab).");
		offset[0] = measureNum;
		offset[1] = 0;
		offset[2] = sampStart;
		dims = (hsize_t *)calloc(3, sizeof(hsize_t));
		ELAN_CHECK_ERROR_ALLOC(dims, "for reading samples of all channels of continuous EP (dims hyperslab).");
		dims[0] = 1;
		dims[1] = elan_struct->chan_nb;
		dims[2] = sampNb;

		H5Sselect_hyperslab(file_dataspace, H5S_SELECT_SET, offset, NULL, dims, NULL);

		val[0] = elan_struct->chan_nb;
		val[1] = sampNb;
		mem_dataspace = H5Screate_simple(2, val, NULL);

		switch (elan_struct->ep.data_type)
		{
		case EF_DATA_FLOAT:
			H5Dread(ep_dataset, H5T_NATIVE_FLOAT, mem_dataspace, file_dataspace, H5P_DEFAULT, &flt_ptr[0][0]); /* Read the dataset data. */
			break;
		case EF_DATA_DOUBLE:
			H5Dread(ep_dataset, H5T_NATIVE_DOUBLE, mem_dataspace, file_dataspace, H5P_DEFAULT, &dbl_ptr[0][0]); /* Read the dataset data. */
			break;
		}
		H5Dclose(ep_dataset);
		H5Gclose(ep_group);
		H5Sclose(mem_dataspace);
		free(dims);
		free(offset);

		H5Gclose(elan_group);
		/* Terminate access to the file. */
		H5Fclose(f);

		return(sampNb);
	}
	else
	{
		return(0);
	}
}


/****************************************************************************************************************/
/* HDF5_read_TF_tfblock_one_channel: read a time-frequency block for one channel.                               */
/* filename: name of the file to read datra from.                                                               */
/* elan_struct_t *elan_struct: Elan structure (read from header).                                               */
/* int measureNum: indice of measure to read (starting from 0).                                                 */
/* int chanNum: indice of channel to read (starting from 0).                                                    */
/* int sampStart: indice of 1st sample to read (starting from 0).                                               */
/* int sampNb: number of samples to read (starting from 0).                                                     */
/* int freqStart: indice of 1st frequency to read (starting from 0).                                            */
/* int freqNb: number of frequencies to read (starting from 0).                                                 */
/* void ***data_ptr: array of samples to store data. If NULL, data_ptr is allocated in function (must be freed).*/
/*                 If allocated in caller, size must be [freqNbxsampNb] float or double.                        */
/* return value: number of samples actually read.                                                               */
/****************************************************************************************************************/
int HDF5_read_TF_tfblock_one_channel(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum, int sampStart, int sampNb, int freqStart, int freqNb, void ***data_ptr)
{
	float **flt_ptr = NULL;
	double **dbl_ptr = NULL;
	hid_t f, elan_group, tf_group, tf_dataset, file_dataspace, mem_dataspace;
	hsize_t *dims, *offset, val[2];

	if (elan_struct->has_tf == EF_YES)
	{
		if (*data_ptr == NULL)
		{
			switch (elan_struct->tf.data_type)
			{
			case EF_DATA_FLOAT:
				flt_ptr = (float **)ef_alloc_array_2d(freqNb, sampNb, sizeof(float));
				*data_ptr = (void *)flt_ptr;
				break;
			case EF_DATA_DOUBLE:
				dbl_ptr = (double **)ef_alloc_array_2d(freqNb, sampNb, sizeof(double));
				*data_ptr = (void *)dbl_ptr;
				break;
			}
		}
		else
		{
			switch (elan_struct->tf.data_type)
			{
			case EF_DATA_FLOAT:
				flt_ptr = (float **)*data_ptr;
				break;
			case EF_DATA_DOUBLE:
				dbl_ptr = (double **)*data_ptr;
				break;
			}
		}

		f = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT);
		if (f < 0)
		{
			fprintf(stderr, "ERROR: can't open file %s for reading.\n", filename);
			return(EF_ERR_OPEN_READ);
		}
		elan_group = H5Gopen(f, HDF5_ELAN_GROUP, H5P_DEFAULT);

		/* TF group and dataset reading. */
		/*-------------------------------*/
		tf_group = H5Gopen(elan_group, HDF5_TF_GROUP, H5P_DEFAULT);
		tf_dataset = H5Dopen2(tf_group, HDF5_TF_ARRAY_DSET, H5P_DEFAULT);
		file_dataspace = H5Dget_space(tf_dataset);    /* dataspace identifier */

													  /* Hyperslab creation for one measure, one channel, a frequency block, a sample block. */
		offset = (hsize_t *)calloc(4, sizeof(hsize_t));
		ELAN_CHECK_ERROR_ALLOC(offset, "for reading a block of frequencies on all channel of TF (offset hyperslab).");
		offset[0] = measureNum;
		offset[1] = chanNum;
		offset[2] = freqStart;
		offset[3] = sampStart;
		dims = (hsize_t *)calloc(4, sizeof(hsize_t));
		ELAN_CHECK_ERROR_ALLOC(dims, "for reading a block of frequencies on all channel of TF (dims hyperslab).");
		val[0] = freqNb;
		val[1] = sampNb;
		dims[0] = 1;
		dims[1] = 1;
		dims[2] = val[0];
		dims[3] = val[1];

		mem_dataspace = H5Screate_simple(2, val, NULL);

		H5Sselect_hyperslab(file_dataspace, H5S_SELECT_SET, offset, NULL, dims, NULL);
		switch (elan_struct->tf.data_type)
		{
		case EF_DATA_FLOAT:
			H5Dread(tf_dataset, H5T_NATIVE_FLOAT, mem_dataspace, file_dataspace, H5P_DEFAULT, &flt_ptr[0][0]); /* Read the dataset data. */
			break;
		case EF_DATA_DOUBLE:
			H5Dread(tf_dataset, H5T_NATIVE_DOUBLE, mem_dataspace, file_dataspace, H5P_DEFAULT, &dbl_ptr[0][0]); /* Read the dataset data. */
			break;
		}
		H5Dclose(tf_dataset);
		H5Gclose(tf_group);
		H5Sclose(mem_dataspace);
		free(dims);
		free(offset);

		H5Gclose(elan_group);
		/* Terminate access to the file. */
		H5Fclose(f);

		return(freqNb*sampNb);
	}
	else
	{
		return(0);
	}
}


/****************************************************************************************************************/
/* HDF5_read_TF_timeblock_all_channel: read a time-frequency block for all channels.                            */
/* filename: name of the file to read datra from.                                                               */
/* elan_struct_t *elan_struct: Elan structure (read from header).                                               */
/* int measureNum: indice of measure to read (starting from 0).                                                 */
/* int sampStart: indice of 1st sample to read (starting from 0).                                               */
/* int sampNb: number of samples to read (starting from 0).                                                     */
/* int freqStart: indice of 1st frequency to read (starting from 0).                                            */
/* int freqNb: number of frequencies to read (starting from 0).                                                 */
/* void ***data_ptr: array of samples to store data. If NULL, data_ptr is allocated in function (must be freed).*/
/*                 If allocated in caller, size must be [elan_struct.chan_nbxfreqNbxsampNb]                     */
/*                 float or double.                                                                             */
/* return value: number of samples actually read.                                                               */
/****************************************************************************************************************/
int HDF5_read_TF_tfblock_all_channel(char *filename, elan_struct_t *elan_struct, int measureNum, int sampStart, int sampNb, int freqStart, int freqNb, void ****data_ptr)
{
	float ***flt_ptr = NULL;
	double ***dbl_ptr = NULL;
	hid_t f, elan_group, tf_group, tf_dataset, file_dataspace, mem_dataspace;
	hsize_t *dims, *offset, val[3];

	if (elan_struct->has_tf == EF_YES)
	{
		if (*data_ptr == NULL)
		{
			switch (elan_struct->tf.data_type)
			{
			case EF_DATA_FLOAT:
				flt_ptr = (float ***)ef_alloc_array_3d(elan_struct->chan_nb, freqNb, sampNb, sizeof(float));
				*data_ptr = (void *)flt_ptr;
				break;
			case EF_DATA_DOUBLE:
				dbl_ptr = (double ***)ef_alloc_array_3d(elan_struct->chan_nb, freqNb, sampNb, sizeof(double));
				*data_ptr = (void *)dbl_ptr;
				break;
			}
		}
		else
		{
			switch (elan_struct->tf.data_type)
			{
			case EF_DATA_FLOAT:
				flt_ptr = (float ***)*data_ptr;
				break;
			case EF_DATA_DOUBLE:
				dbl_ptr = (double ***)*data_ptr;
				break;
			}
		}

		f = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT);
		if (f < 0)
		{
			fprintf(stderr, "ERROR: can't open file %s for reading.\n", filename);
			return(EF_ERR_OPEN_READ);
		}
		elan_group = H5Gopen(f, HDF5_ELAN_GROUP, H5P_DEFAULT);

		/* TF group and dataset reading. */
		/*-------------------------------*/
		tf_group = H5Gopen(elan_group, HDF5_TF_GROUP, H5P_DEFAULT);
		tf_dataset = H5Dopen2(tf_group, HDF5_TF_ARRAY_DSET, H5P_DEFAULT);
		file_dataspace = H5Dget_space(tf_dataset);    /* dataspace identifier */

													  /* Hyperslab creation for one measure, all channel, all frequencies, a sample block. */
		offset = (hsize_t *)calloc(4, sizeof(hsize_t));
		ELAN_CHECK_ERROR_ALLOC(offset, "for reading a block of frequencies on all channel of TF (offset hyperslab).");
		offset[0] = measureNum;
		offset[1] = 0;
		offset[2] = freqStart;
		offset[3] = sampStart;
		dims = (hsize_t *)calloc(4, sizeof(hsize_t));
		ELAN_CHECK_ERROR_ALLOC(dims, "for reading a block of frequencies on all channel of TF (dims hyperslab).");
		val[0] = elan_struct->chan_nb;
		val[1] = freqNb;
		val[2] = sampNb;
		dims[0] = 1;
		dims[1] = val[0];
		dims[2] = val[1];
		dims[3] = val[2];

		mem_dataspace = H5Screate_simple(3, val, NULL);

		H5Sselect_hyperslab(file_dataspace, H5S_SELECT_SET, offset, NULL, dims, NULL);
		switch (elan_struct->tf.data_type)
		{
		case EF_DATA_FLOAT:
			H5Dread(tf_dataset, H5T_NATIVE_FLOAT, mem_dataspace, file_dataspace, H5P_DEFAULT, &flt_ptr[0][0][0]); /* Read the dataset data. */
			break;
		case EF_DATA_DOUBLE:
			H5Dread(tf_dataset, H5T_NATIVE_DOUBLE, mem_dataspace, file_dataspace, H5P_DEFAULT, &dbl_ptr[0][0][0]); /* Read the dataset data. */
			break;
		}
		H5Dclose(tf_dataset);
		H5Gclose(tf_group);
		H5Sclose(mem_dataspace);
		free(dims);
		free(offset);

		H5Gclose(elan_group);
		/* Terminate access to the file. */
		H5Fclose(f);

		return(elan_struct->chan_nb*freqNb*sampNb);
	}
	else
	{
		return(0);
	}
}


/***************************************************/
/* HDF5_read_version: read Elan HDF5 file version. */
/***************************************************/
int HDF5_read_version(char *filename)
{
	int version = UNDEF_VERSION, endian;
	hid_t f, elan_group, common_group;

	f = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT);
	if (f < 0)
	{
		fprintf(stderr, "ERROR: opening HDF5 file %s .\n", filename);
		return(version);
	}
	elan_group = H5Gopen(f, HDF5_ELAN_GROUP, H5P_DEFAULT);

	common_group = H5Gopen(elan_group, HDF5_COMMON_GROUP, H5P_DEFAULT);
	ef_HDF5open_read_attribute(common_group, HDF5_ENDIAN_ATTRIB, H5P_DEFAULT, H5T_NATIVE_INT, &endian);
	ef_HDF5open_read_attribute(common_group, HDF5_VERSION_ATTRIB, H5P_DEFAULT, CHOOSE_ENDIAN_INT32(endian), &version);
	H5Gclose(common_group);

	H5Gclose(elan_group);

	H5Fclose(f);

	return(version);
}

