/*--------------------------------------------------*/
/* Writing HDF5 ELAN files module.                  */
/* v: 1.00    Aut.: PEA                             */
/* CRNL / INSERM U1028 / CNRS UMR 5292 / Univ-Lyon1 */
/*--------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "elanfile.h"
#include "ef_hdf5common.h"

#define EEG_CHUNK_SIZE 100000

/*************************************************************/
/* HDF5_write_header: write Elan data header in HDF5 format. */
/*************************************************************/
int HDF5_write_header(char *filename, elan_struct_t *elan_struct)
{
	int i_chan, i_coord, i_c, i_meas;
	hdf_chan_mem_t *hdf_chan_mem;
	hdf_coord_mem_t **ptr_coord_mem;
	hid_t f, elan_group, common_group, channel_dset, eeg_event_dataspace_id;
	hid_t eeg_group, ep_group, tf_group;
	hid_t data_space, eeg_dataset=0, ep_dataset=0, tf_dataset=0;
	hid_t eeg_property, tf_property;
	hsize_t *data_dims, eeg_chunk_size[3], tf_chunk_size[4], event_size;
	unsigned int filter_info;
	int err=0;
	hdf_dataspaces_t hdf_dataspaces;
	hdf_types_t hdf_types;

	f = H5Fcreate(filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT); /* H5F_ACC_TRUNC: Truncate file, if it already exists, erasing all data previously stored in the file */
	if (f < 0)
	{
		fprintf(stderr, "ERROR: can't open file %s for writing.\n", filename);
		err = EF_ERR_OPEN_WRITE;
	}

	/* Dataspaces and types initialization. */
  ef_HDF5create_basic_types_dataspaces(&hdf_dataspaces, &hdf_types);
	ef_HDF5create_fromheader_types_dataspaces(elan_struct, &hdf_dataspaces, &hdf_types);
	
	/* Copy Elan structure values to HDF5 fiile and memory structures. */
	hdf_chan_mem = (hdf_chan_mem_t*)calloc(elan_struct->chan_nb, sizeof(hdf_chan_mem_t));
	ELAN_CHECK_ERROR_ALLOC(hdf_chan_mem, "for storing channels infos in memory (as HDF5 objects).");
	ptr_coord_mem = (hdf_coord_mem_t**)calloc(elan_struct->chan_nb, sizeof(hdf_coord_mem_t*));
	ELAN_CHECK_ERROR_ALLOC(ptr_coord_mem, "for storing coordinates infos in memory (as HDF5 objects).");
	for (i_chan=0; i_chan<elan_struct->chan_nb; i_chan++)
	{
		strcpy(hdf_chan_mem[i_chan].lab, elan_struct->chan_list[i_chan].lab);
		strcpy(hdf_chan_mem[i_chan].type, elan_struct->chan_list[i_chan].type);
		strcpy(hdf_chan_mem[i_chan].unit, elan_struct->chan_list[i_chan].unit);
		
		ptr_coord_mem[i_chan] = (hdf_coord_mem_t*)calloc(elan_struct->chan_list[i_chan].coord_nb, sizeof(hdf_coord_mem_t));
		ELAN_CHECK_ERROR_ALLOC(ptr_coord_mem[i_chan], "for storing coordinates (for one channel) infos in memory (as HDF5 objects).");
		for (i_coord=0; i_coord<elan_struct->chan_list[i_chan].coord_nb; i_coord++)
		{
			strcpy(ptr_coord_mem[i_chan][i_coord].coord_lab, elan_struct->chan_list[i_chan].coord_list[i_coord].lab);
			ptr_coord_mem[i_chan][i_coord].value_list.len = elan_struct->chan_list[i_chan].coord_list[i_coord].val_nb;
			ptr_coord_mem[i_chan][i_coord].value_list.p = elan_struct->chan_list[i_chan].coord_list[i_coord].val_list;
		}
		
		hdf_chan_mem[i_chan].coord_list.len = elan_struct->chan_list[i_chan].coord_nb;
		hdf_chan_mem[i_chan].coord_list.p = ptr_coord_mem[i_chan];
	}

	/* Common group creation. */
  elan_group = H5Gcreate2(f, HDF5_ELAN_GROUP, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

	/* Common group creation. */
  common_group = H5Gcreate2(elan_group, HDF5_COMMON_GROUP, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	
	/* Create attributes of common group. */
	/*------------------------------------*/
	ef_HDF5create_write_attribute(common_group, HDF5_ENDIAN_ATTRIB, H5T_STD_I32LE, hdf_dataspaces.scalar_dataspace_id, 
																H5P_DEFAULT, H5P_DEFAULT, H5T_NATIVE_INT, &elan_struct->endian);
	ef_HDF5create_write_attribute(common_group, HDF5_VERSION_ATTRIB, CHOOSE_ENDIAN_INT32(elan_struct->endian), hdf_dataspaces.scalar_dataspace_id, 
																H5P_DEFAULT, H5P_DEFAULT, H5T_NATIVE_INT, &elan_struct->version);
	ef_HDF5create_write_attribute(common_group, HDF5_RELEASE_ATTRIB, CHOOSE_ENDIAN_INT32(elan_struct->endian), hdf_dataspaces.scalar_dataspace_id, 
																H5P_DEFAULT, H5P_DEFAULT, H5T_NATIVE_INT, &elan_struct->release);
	for (i_c=strlen(elan_struct->comment); i_c<COMMENT_SIZE; i_c++)
	{
		elan_struct->comment[i_c] = '\0'; /* need to pas with 0 to avoid valgrind warnings (uninitialized bytes). */
	}
	ef_HDF5create_write_attribute(common_group, HDF5_COMMENT_ATTRIB, hdf_types.str_comment_type, hdf_dataspaces.scalar_dataspace_id, 
																H5P_DEFAULT, H5P_DEFAULT, hdf_types.str_comment_type, elan_struct->comment);
	ef_HDF5create_write_attribute(common_group, HDF5_CHANNELNB_ATTRIB, CHOOSE_ENDIAN_INT32(elan_struct->endian), hdf_dataspaces.scalar_dataspace_id, 
																H5P_DEFAULT, H5P_DEFAULT, H5T_NATIVE_INT, &elan_struct->chan_nb);
	ef_HDF5create_write_attribute(common_group, HDF5_MEASURENB_ATTRIB, CHOOSE_ENDIAN_INT32(elan_struct->endian), hdf_dataspaces.scalar_dataspace_id, 
																H5P_DEFAULT, H5P_DEFAULT, H5T_NATIVE_INT, &elan_struct->measure_channel_nb);
	for (i_meas=0; i_meas<elan_struct->measure_channel_nb; i_meas++)
	{
		for (i_c=strlen(elan_struct->measure_channel_label_list[i_meas]); i_c<LABEL_SIZE; i_c++)
		{
			elan_struct->measure_channel_label_list[i_meas][i_c] = '\0'; /* need to pas with 0 to avoid valgrind warnings (uninitialized bytes). */
		}
	}
	ef_HDF5create_write_attribute(common_group, HDF5_MEASURELIST_ATTRIB, hdf_types.str_label_type, hdf_dataspaces.measure_dataspace_id, 
																H5P_DEFAULT, H5P_DEFAULT, hdf_types.str_label_type, elan_struct->measure_channel_label_list);

	/* Close the group. */
  H5Gclose(common_group);

	/* Channels dataset creation. */
	/*----------------------------*/
  channel_dset = H5Dcreate2(elan_group, HDF5_CHANNEL_DSET, hdf_types.chan_filetype, hdf_dataspaces.chan_dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	H5Dwrite(channel_dset, hdf_types.chan_memtype, H5S_ALL, H5S_ALL, H5P_DEFAULT, hdf_chan_mem); /* Write the dataset data. */
	
	/* Close the channel dataset. */
  H5Dclose(channel_dset);

	/* EEG group and dataset creation if needed. */
	/*-------------------------------------------*/
	if (elan_struct->has_eeg == EF_YES)
	{
		eeg_group = H5Gcreate2(elan_group, HDF5_EEG_GROUP, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
		ef_HDF5create_write_attribute(eeg_group, HDF5_DATATYPE_ATTRIB, CHOOSE_ENDIAN_INT32(elan_struct->endian), hdf_dataspaces.scalar_dataspace_id, 
																H5P_DEFAULT, H5P_DEFAULT, H5T_NATIVE_INT, &elan_struct->eeg.data_type);
		ef_HDF5create_write_attribute(eeg_group, HDF5_EEG_SAMPNB_ATTRIB, CHOOSE_ENDIAN_INT32(elan_struct->endian), hdf_dataspaces.scalar_dataspace_id, 
																H5P_DEFAULT, H5P_DEFAULT, H5T_NATIVE_INT, &elan_struct->eeg.samp_nb);
		ef_HDF5create_write_attribute(eeg_group, HDF5_EEG_SAMPFREQ_ATTRIB, CHOOSE_ENDIAN_FLT32(elan_struct->endian), hdf_dataspaces.scalar_dataspace_id, 
																	H5P_DEFAULT, H5P_DEFAULT, H5T_NATIVE_FLOAT, &elan_struct->eeg.sampling_freq);
		
		switch (elan_struct->eeg.flag_cont_epoch)
		{
			case EF_EEG_CONTINUOUS:
				ef_HDF5create_write_attribute(eeg_group, HDF5_EEG_EVENTNB_ATTRIB, CHOOSE_ENDIAN_INT32(elan_struct->endian), hdf_dataspaces.scalar_dataspace_id, 
																H5P_DEFAULT, H5P_DEFAULT, H5T_NATIVE_INT, &elan_struct->eeg.event_nb);
				if (elan_struct->eeg.event_nb > 0) {
					event_size = elan_struct->eeg.event_nb;
					eeg_event_dataspace_id = H5Screate_simple(1, &event_size, NULL);
					ef_HDF5create_write_attribute(eeg_group, HDF5_EEG_EVENTCODE_ATTRIB, CHOOSE_ENDIAN_INT32(elan_struct->endian), eeg_event_dataspace_id, 
																		H5P_DEFAULT, H5P_DEFAULT, H5T_NATIVE_INT, elan_struct->eeg.event_code_list);
					ef_HDF5create_write_attribute(eeg_group, HDF5_EEG_EVENTTIME_ATTRIB, CHOOSE_ENDIAN_INT32(elan_struct->endian), eeg_event_dataspace_id, 
																		H5P_DEFAULT, H5P_DEFAULT, H5T_NATIVE_INT, elan_struct->eeg.event_sample_list);
				}

				data_dims = (hsize_t *)calloc(3, sizeof(hsize_t));
				ELAN_CHECK_ERROR_ALLOC(data_dims, "for continuous EEG data dimensions.");
				data_dims[0] = elan_struct->measure_channel_nb;
				data_dims[1] = elan_struct->chan_nb;
				data_dims[2] = elan_struct->eeg.samp_nb;
				
				data_space = H5Screate_simple(3, data_dims, NULL);
				
				/* Compression if needed. */
				/*------------------------*/
				eeg_property = H5P_DEFAULT;
				if (elan_struct->options.compress == EF_YES)
				{
					if (elan_struct->options.compressionType == EF_COMP_GZIP)
					{
						if (!H5Zfilter_avail(H5Z_FILTER_DEFLATE)) 
						{
							fprintf(stderr, "ERROR: compression: gzip filter not available.\n");
							return(EF_ERR_WRITE_DATA);
						}
						H5Zget_filter_info (H5Z_FILTER_DEFLATE, &filter_info);
						if ( !(filter_info & H5Z_FILTER_CONFIG_ENCODE_ENABLED) || !(filter_info & H5Z_FILTER_CONFIG_DECODE_ENABLED) ) 
						{
							fprintf(stderr, "ERROR: compression: gzip filter not available for encoding and decoding.\n");
							return(EF_ERR_WRITE_DATA);
						}
						eeg_property = H5Pcreate(H5P_DATASET_CREATE);
						H5Pset_deflate(eeg_property, 9);
						eeg_chunk_size[0] = 1;
						eeg_chunk_size[1] = 1;
						eeg_chunk_size[2] = ELAN_MIN(EEG_CHUNK_SIZE, elan_struct->eeg.samp_nb);
						H5Pset_chunk(eeg_property, 3, eeg_chunk_size);
					}
				}
				switch (elan_struct->eeg.data_type)
				{
					case EF_DATA_FLOAT:
						eeg_dataset = H5Dcreate2(eeg_group, HDF5_EEG_CONT_DSET, CHOOSE_ENDIAN_FLT32(elan_struct->endian), data_space, H5P_DEFAULT, eeg_property, H5P_DEFAULT);
						break;
					case EF_DATA_DOUBLE:
						eeg_dataset = H5Dcreate2(eeg_group, HDF5_EEG_CONT_DSET, CHOOSE_ENDIAN_FLT64(elan_struct->endian), data_space, H5P_DEFAULT, eeg_property, H5P_DEFAULT);
						break;
				}
				if (elan_struct->options.compress == EF_YES)
				{
					if (elan_struct->options.compressionType == EF_COMP_GZIP)
					{
					H5Pclose(eeg_property);
					}
				}
				H5Sclose(data_space);
				H5Dclose(eeg_dataset);
				free(data_dims);
				break;
			case EF_EEG_EPOCH:
				fprintf(stderr, "ERROR: epoched EEG format not handled.\n");
				break;
			default:
				fprintf(stderr, "ERROR: EEG format %d not handled.\n", elan_struct->eeg.flag_cont_epoch);				
		}
		H5Gclose(eeg_group);
	}

	/* EP group and dataset creation if needed. */
	/*------------------------------------------*/
	if (elan_struct->has_ep == EF_YES)
	{
		ep_group = H5Gcreate2(elan_group, HDF5_EP_GROUP, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
		ef_HDF5create_write_attribute(ep_group, HDF5_DATATYPE_ATTRIB, CHOOSE_ENDIAN_INT32(elan_struct->endian), hdf_dataspaces.scalar_dataspace_id, 
																H5P_DEFAULT, H5P_DEFAULT, H5T_NATIVE_INT, &elan_struct->ep.data_type);
		ef_HDF5create_write_attribute(ep_group, HDF5_EP_SAMPNB_ATTRIB, CHOOSE_ENDIAN_INT32(elan_struct->endian), hdf_dataspaces.scalar_dataspace_id, 
																H5P_DEFAULT, H5P_DEFAULT, H5T_NATIVE_INT, &elan_struct->ep.samp_nb);
		ef_HDF5create_write_attribute(ep_group, HDF5_EP_SAMPFREQ_ATTRIB, CHOOSE_ENDIAN_FLT32(elan_struct->endian), hdf_dataspaces.scalar_dataspace_id, 
																	H5P_DEFAULT, H5P_DEFAULT, H5T_NATIVE_FLOAT, &elan_struct->ep.sampling_freq);
		ef_HDF5create_write_attribute(ep_group, HDF5_EP_PRESTIM_ATTRIB, CHOOSE_ENDIAN_INT32(elan_struct->endian), hdf_dataspaces.scalar_dataspace_id, 
																H5P_DEFAULT, H5P_DEFAULT, H5T_NATIVE_INT, &elan_struct->ep.prestim_samp_nb);
		ef_HDF5create_write_attribute(ep_group, HDF5_EP_EVENTCODE_ATTRIB, CHOOSE_ENDIAN_INT32(elan_struct->endian), hdf_dataspaces.scalar_dataspace_id, 
																H5P_DEFAULT, H5P_DEFAULT, H5T_NATIVE_INT, &elan_struct->ep.event_code);
		ef_HDF5create_write_attribute(ep_group, HDF5_EP_EVENTNB_ATTRIB, CHOOSE_ENDIAN_INT32(elan_struct->endian), hdf_dataspaces.scalar_dataspace_id, 
																H5P_DEFAULT, H5P_DEFAULT, H5T_NATIVE_INT, &elan_struct->ep.event_nb);
		
		ef_HDF5create_ep_types_dataspaces(elan_struct, &hdf_dataspaces);
		ef_HDF5create_write_attribute(ep_group, HDF5_EPTF_OTHEREVENTNB_ATTRIB, CHOOSE_ENDIAN_INT32(elan_struct->endian), hdf_dataspaces.measure_dataspace_id, 
																H5P_DEFAULT, H5P_DEFAULT, H5T_NATIVE_INT, elan_struct->ep.other_events_nb);
		if (elan_struct->ep.other_events_list != NULL) {
			ef_HDF5create_write_attribute(ep_group, HDF5_EPTF_OTHEREVENTLIST_ATTRIB, CHOOSE_ENDIAN_INT32(elan_struct->endian), hdf_dataspaces.ep_otherevent_dataspace_id, 
																H5P_DEFAULT, H5P_DEFAULT, H5T_NATIVE_INT, elan_struct->ep.other_events_list);
		}
		
		data_dims = (hsize_t *)calloc(3, sizeof(hsize_t));
		ELAN_CHECK_ERROR_ALLOC(data_dims, "for EP data dimensions.");
		data_dims[0] = elan_struct->measure_channel_nb;
		data_dims[1] = elan_struct->chan_nb;
		data_dims[2] = elan_struct->ep.samp_nb;
		
		data_space = H5Screate_simple(3, data_dims, NULL);
		switch (elan_struct->ep.data_type)
		{
			case EF_DATA_FLOAT:
				ep_dataset = H5Dcreate2(ep_group, HDF5_EP_EPOCH_DSET, CHOOSE_ENDIAN_FLT32(elan_struct->endian), data_space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
				break;
			case EF_DATA_DOUBLE:
				ep_dataset = H5Dcreate2(ep_group, HDF5_EP_EPOCH_DSET, CHOOSE_ENDIAN_FLT64(elan_struct->endian), data_space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
				break;
		}
		H5Sclose(data_space);
		H5Dclose(ep_dataset);
		H5Gclose(ep_group);
		free(data_dims);
	}

	/* TF group and dataset creation if needed. */
	/*------------------------------------------*/
	if (elan_struct->has_tf == EF_YES)
	{
		tf_group = H5Gcreate2(elan_group, HDF5_TF_GROUP, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
		ef_HDF5create_write_attribute(tf_group, HDF5_DATATYPE_ATTRIB, CHOOSE_ENDIAN_INT32(elan_struct->endian), hdf_dataspaces.scalar_dataspace_id, 
																H5P_DEFAULT, H5P_DEFAULT, H5T_NATIVE_INT, &elan_struct->tf.data_type);
		ef_HDF5create_write_attribute(tf_group, HDF5_TF_SAMPNB_ATTRIB, CHOOSE_ENDIAN_INT32(elan_struct->endian), hdf_dataspaces.scalar_dataspace_id, 
																H5P_DEFAULT, H5P_DEFAULT, H5T_NATIVE_INT, &elan_struct->tf.samp_nb);
		ef_HDF5create_write_attribute(tf_group, HDF5_TF_SAMPFREQ_ATTRIB, CHOOSE_ENDIAN_FLT32(elan_struct->endian), hdf_dataspaces.scalar_dataspace_id, 
																	H5P_DEFAULT, H5P_DEFAULT, H5T_NATIVE_FLOAT, &elan_struct->tf.sampling_freq);
		ef_HDF5create_write_attribute(tf_group, HDF5_TF_PRESTIM_ATTRIB, CHOOSE_ENDIAN_INT32(elan_struct->endian), hdf_dataspaces.scalar_dataspace_id, 
																H5P_DEFAULT, H5P_DEFAULT, H5T_NATIVE_INT, &elan_struct->tf.prestim_samp_nb);
		ef_HDF5create_write_attribute(tf_group, HDF5_TF_EVENTCODE_ATTRIB, CHOOSE_ENDIAN_INT32(elan_struct->endian), hdf_dataspaces.scalar_dataspace_id, 
																H5P_DEFAULT, H5P_DEFAULT, H5T_NATIVE_INT, &elan_struct->tf.event_code);
		ef_HDF5create_write_attribute(tf_group, HDF5_TF_EVENTNB_ATTRIB, CHOOSE_ENDIAN_INT32(elan_struct->endian), hdf_dataspaces.scalar_dataspace_id, 
																H5P_DEFAULT, H5P_DEFAULT, H5T_NATIVE_INT, &elan_struct->tf.event_nb);
		ef_HDF5create_write_attribute(tf_group, HDF5_TF_WAVELETTYPE_ATTRIB, CHOOSE_ENDIAN_INT32(elan_struct->endian), hdf_dataspaces.scalar_dataspace_id, 
																H5P_DEFAULT, H5P_DEFAULT, H5T_NATIVE_INT, &elan_struct->tf.wavelet_type);
		ef_HDF5create_write_attribute(tf_group, HDF5_TF_BLACKMAN_ATTRIB, CHOOSE_ENDIAN_INT32(elan_struct->endian), hdf_dataspaces.scalar_dataspace_id, 
																H5P_DEFAULT, H5P_DEFAULT, H5T_NATIVE_INT, &elan_struct->tf.blackman_window);
		ef_HDF5create_write_attribute(tf_group, HDF5_TF_FREQUENCYNB_ATTRIB, CHOOSE_ENDIAN_INT32(elan_struct->endian), hdf_dataspaces.scalar_dataspace_id, 
																H5P_DEFAULT, H5P_DEFAULT, H5T_NATIVE_INT, &elan_struct->tf.freq_nb);
		
		ef_HDF5create_tf_types_dataspaces(elan_struct, &hdf_dataspaces);

		ef_HDF5create_write_attribute(tf_group, HDF5_EPTF_OTHEREVENTNB_ATTRIB, CHOOSE_ENDIAN_INT32(elan_struct->endian), hdf_dataspaces.measure_dataspace_id, 
																H5P_DEFAULT, H5P_DEFAULT, H5T_NATIVE_INT, elan_struct->tf.other_events_nb);
		if (elan_struct->tf.other_events_list != NULL) {
			ef_HDF5create_write_attribute(tf_group, HDF5_EPTF_OTHEREVENTLIST_ATTRIB, CHOOSE_ENDIAN_INT32(elan_struct->endian), hdf_dataspaces.tf_otherevent_dataspace_id, 
																H5P_DEFAULT, H5P_DEFAULT, H5T_NATIVE_INT, elan_struct->tf.other_events_list);
		}

		ef_HDF5create_write_attribute(tf_group, HDF5_TF_FREQUENCYLIST_ATTRIB, CHOOSE_ENDIAN_FLT32(elan_struct->endian), hdf_dataspaces.freq_dataspace_id, 
																H5P_DEFAULT, H5P_DEFAULT, H5T_NATIVE_FLOAT, elan_struct->tf.freq_array);
		ef_HDF5create_write_attribute(tf_group, HDF5_TF_WAVELETCHAR_ATTRIB, CHOOSE_ENDIAN_FLT32(elan_struct->endian), hdf_dataspaces.freq_dataspace_id, 
																H5P_DEFAULT, H5P_DEFAULT, H5T_NATIVE_FLOAT, elan_struct->tf.wavelet_char);
		
		data_dims = (hsize_t *)calloc(4, sizeof(hsize_t));
		ELAN_CHECK_ERROR_ALLOC(data_dims, "for TF data dimensions.");
		data_dims[0] = elan_struct->measure_channel_nb;
		data_dims[1] = elan_struct->chan_nb;
		data_dims[2] = elan_struct->tf.freq_nb;
		data_dims[3] = elan_struct->tf.samp_nb;
		data_space = H5Screate_simple(4, data_dims, NULL);
		
		/* Compression if needed. */
		/*------------------------*/
		tf_property = H5P_DEFAULT;
		if (elan_struct->options.compress == EF_YES)
		{
			
			if (elan_struct->options.compressionType == EF_COMP_GZIP)
			{
				if (!H5Zfilter_avail(H5Z_FILTER_DEFLATE)) 
				{
					fprintf(stderr, "ERROR: compression: gzip filter not available.\n");
					return(EF_ERR_WRITE_DATA);
				}
				H5Zget_filter_info (H5Z_FILTER_DEFLATE, &filter_info);
				if ( !(filter_info & H5Z_FILTER_CONFIG_ENCODE_ENABLED) || !(filter_info & H5Z_FILTER_CONFIG_DECODE_ENABLED) ) 
				{
					fprintf(stderr, "ERROR: compression: gzip filter not available for encoding and decoding.\n");
					return(EF_ERR_WRITE_DATA);
				}
				tf_property = H5Pcreate(H5P_DATASET_CREATE);
				H5Pset_deflate(tf_property, 9);
				tf_chunk_size[0] = 1;
				tf_chunk_size[1] = 1;
				tf_chunk_size[2] = elan_struct->tf.freq_nb;
				tf_chunk_size[3] = elan_struct->tf.samp_nb;
				H5Pset_chunk(tf_property, 4, tf_chunk_size);
			}
		}
		switch (elan_struct->tf.data_type)
		{
			case EF_DATA_FLOAT:
				tf_dataset = H5Dcreate2(tf_group, HDF5_TF_ARRAY_DSET, CHOOSE_ENDIAN_FLT32(elan_struct->endian), data_space, H5P_DEFAULT, tf_property, H5P_DEFAULT);
				break;
			case EF_DATA_DOUBLE:
				tf_dataset = H5Dcreate2(tf_group, HDF5_TF_ARRAY_DSET, CHOOSE_ENDIAN_FLT64(elan_struct->endian), data_space, H5P_DEFAULT, tf_property, H5P_DEFAULT);
				break;
		}
		if (elan_struct->options.compress == EF_YES)
		{
			if (elan_struct->options.compressionType == EF_COMP_GZIP)
			{
			H5Pclose(tf_property);
			}
		}

		H5Sclose(data_space);
		H5Dclose(tf_dataset);
		H5Gclose(tf_group);
		free(data_dims);
	}

	ef_HDF5close_types_dataspaces(&hdf_dataspaces, &hdf_types);

	/* Close the ELAN group. */
  H5Gclose(elan_group);
	
	/* Terminate access to the file. */
  H5Fclose(f); 

	for (i_chan=0; i_chan<elan_struct->chan_nb; i_chan++)
	{
		free(ptr_coord_mem[i_chan]);
	}
	free(ptr_coord_mem);
	free(hdf_chan_mem);

	return(err);
}


/******************************************************************************************/
/* HDF5_write_data_all_channels: write Elan data for all channels at once in HDF5 format. */
/******************************************************************************************/
int HDF5_write_data_all_channels(char *filename, elan_struct_t *elan_struct)
{
	hid_t f, elan_group, eeg_group, eeg_dataset, ep_group, ep_dataset, tf_group, tf_dataset;
	int err=0;
	
	f = H5Fopen(filename, H5F_ACC_RDWR, H5P_DEFAULT);
	if (f < 0)
	{
		fprintf(stderr, "ERROR: can't open file %s for writing.\n", filename);
		err = EF_ERR_OPEN_WRITE;
	}
	elan_group = H5Gopen(f, HDF5_ELAN_GROUP, H5P_DEFAULT);
	
	/* EEG group and dataset writing if needed. */
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
						H5Dwrite(eeg_dataset, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, elan_struct->eeg.flat_float); /* Write the dataset data. */
						break;
					case EF_DATA_DOUBLE:
						H5Dwrite(eeg_dataset, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, elan_struct->eeg.flat_double); /* Write the dataset data. */
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

	/* EP group and dataset writing if needed. */
	/*-----------------------------------------*/
	if (elan_struct->has_ep == EF_YES)
	{
		ep_group = H5Gopen(elan_group, HDF5_EP_GROUP, H5P_DEFAULT);
		ep_dataset = H5Dopen2(ep_group, HDF5_EP_EPOCH_DSET, H5P_DEFAULT);
		
		switch (elan_struct->ep.data_type)
		{
			case EF_DATA_FLOAT:
				H5Dwrite(ep_dataset, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, elan_struct->ep.flat_float); /* Write the dataset data. */
				break;
			case EF_DATA_DOUBLE:
				H5Dwrite(ep_dataset, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, elan_struct->ep.flat_double); /* Write the dataset data. */
				break;
		}
		H5Dclose(ep_dataset);
		H5Gclose(ep_group);
	}

	/* TF group and dataset writing if needed. */
	/*-----------------------------------------*/
	if (elan_struct->has_tf == EF_YES)
	{
		tf_group = H5Gopen(elan_group, HDF5_TF_GROUP, H5P_DEFAULT);
		tf_dataset = H5Dopen2(tf_group, HDF5_TF_ARRAY_DSET, H5P_DEFAULT);
		
		switch (elan_struct->tf.data_type)
		{
			case EF_DATA_FLOAT:
				H5Dwrite(tf_dataset, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, elan_struct->tf.flat_float); /* Write the dataset data. */
				break;
			case EF_DATA_DOUBLE:
				H5Dwrite(tf_dataset, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, elan_struct->tf.flat_double); /* Write the dataset data. */
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


/******************************************************************************************/
/* HDF5_write_data_one_channel: write one channel of data (index chanNum) for one measure */
/*                                  (index measureNum) in HDF5 format.                    */
/******************************************************************************************/
int HDF5_write_data_one_channel(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum)
{
	hid_t f, elan_group, eeg_group, eeg_dataset, ep_group, ep_dataset, file_dataspace, mem_dataspace, tf_group, tf_dataset;
	hsize_t *dims, *offset, val[2];
	int err=0;
	
	f = H5Fopen(filename, H5F_ACC_RDWR, H5P_DEFAULT);
	if (f < 0)
	{
		fprintf(stderr, "ERROR: can't open file %s for writing.\n", filename);
		err = EF_ERR_OPEN_WRITE;
	}
	elan_group = H5Gopen(f, HDF5_ELAN_GROUP, H5P_DEFAULT);
	
	/* EEG group and dataset writing if needed. */
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
				ELAN_CHECK_ERROR_ALLOC(offset, "for writing one channel of continuous EEG (offset hyperslab).");
				offset[0] = measureNum;
				offset[1] = chanNum;
				offset[2] = 0;
				dims = (hsize_t *)calloc(3, sizeof(hsize_t));
				ELAN_CHECK_ERROR_ALLOC(dims, "for writing one channel of continuous EEG (dims hyperslab).");
				val[0] = elan_struct->eeg.samp_nb;
				dims[0] = 1;
				dims[1] = 1;
				dims[2] = val[0];

				mem_dataspace = H5Screate_simple(1, val, NULL); 
				
				err = H5Sselect_hyperslab(file_dataspace, H5S_SELECT_SET, offset, NULL, dims, NULL);
				switch (elan_struct->eeg.data_type)
				{
					case EF_DATA_FLOAT:
						H5Dwrite(eeg_dataset, H5T_NATIVE_FLOAT, mem_dataspace, file_dataspace, H5P_DEFAULT, elan_struct->eeg.data_float[measureNum][chanNum]); /* Write the dataset data. */
						break;
					case EF_DATA_DOUBLE:
						H5Dwrite(eeg_dataset, H5T_NATIVE_DOUBLE, mem_dataspace, file_dataspace, H5P_DEFAULT, elan_struct->eeg.data_double[measureNum][chanNum]); /* Write the dataset data. */
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
	
	/* EP group and dataset writing if needed. */
	/*-----------------------------------------*/
	if (elan_struct->has_ep == EF_YES)
	{
		ep_group = H5Gopen(elan_group, HDF5_EP_GROUP, H5P_DEFAULT);
		ep_dataset = H5Dopen2(ep_group, HDF5_EP_EPOCH_DSET, H5P_DEFAULT);
		
		file_dataspace = H5Dget_space(ep_dataset);    /* dataspace identifier */
		
		/* Hyperslab creation for one measure, one channel, all samples. */
		offset = (hsize_t *)calloc(3, sizeof(hsize_t));
		ELAN_CHECK_ERROR_ALLOC(offset, "for writing one channel of EP (offset hyperslab).");
		offset[0] = measureNum;
		offset[1] = chanNum;
		offset[2] = 0;
		dims = (hsize_t *)calloc(3, sizeof(hsize_t));
		ELAN_CHECK_ERROR_ALLOC(dims, "for writing one channel of EP (dims hyperslab).");
		val[0] = elan_struct->ep.samp_nb;
		dims[0] = 1;
		dims[1] = 1;
		dims[2] = val[0];

    mem_dataspace = H5Screate_simple(1, val, NULL); 
		
		err = H5Sselect_hyperslab(file_dataspace, H5S_SELECT_SET, offset, NULL, dims, NULL);
		switch (elan_struct->ep.data_type)
		{
			case EF_DATA_FLOAT:
				H5Dwrite(ep_dataset, H5T_NATIVE_FLOAT, mem_dataspace, file_dataspace, H5P_DEFAULT, elan_struct->ep.data_float[measureNum][chanNum]); /* Write the dataset data. */
				break;
			case EF_DATA_DOUBLE:
				H5Dwrite(ep_dataset, H5T_NATIVE_DOUBLE, mem_dataspace, file_dataspace, H5P_DEFAULT, elan_struct->ep.data_double[measureNum][chanNum]); /* Write the dataset data. */
				break;
		}
		H5Dclose(ep_dataset);
		H5Gclose(ep_group);
		H5Sclose(mem_dataspace);
		free(dims);
		free(offset);
	}
	
	/* TF group and dataset writing if needed. */
	/*-----------------------------------------*/
	if (elan_struct->has_tf == EF_YES)
	{
		tf_group = H5Gopen(elan_group, HDF5_TF_GROUP, H5P_DEFAULT);
		tf_dataset = H5Dopen2(tf_group, HDF5_TF_ARRAY_DSET, H5P_DEFAULT);
		
		file_dataspace = H5Dget_space(tf_dataset);    /* dataspace identifier */
		
		/* Hyperslab creation for one measure, one channel, all frequencies, all samples. */
		offset = (hsize_t *)calloc(4, sizeof(hsize_t));
		ELAN_CHECK_ERROR_ALLOC(offset, "for writing one channel of TF (offset hyperslab).");
		offset[0] = measureNum;
		offset[1] = chanNum;
		offset[2] = 0;
		offset[3] = 0;
		dims = (hsize_t *)calloc(4, sizeof(hsize_t));
		ELAN_CHECK_ERROR_ALLOC(dims, "for writing one channel of TF (dims hyperslab).");
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
				H5Dwrite(tf_dataset, H5T_NATIVE_FLOAT, mem_dataspace, file_dataspace, H5P_DEFAULT, &elan_struct->tf.data_float[measureNum][chanNum][0][0]); /* Write the dataset data. */
				break;
			case EF_DATA_DOUBLE:
				H5Dwrite(tf_dataset, H5T_NATIVE_DOUBLE, mem_dataspace, file_dataspace, H5P_DEFAULT, &elan_struct->tf.data_double[measureNum][chanNum][0][0]); /* Write the dataset data. */
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


/*****************************************************************************************************************/
/* HDF5_write_EEG_data_sampblock_one_chan: write an EEG data block for one channel.                              */
/* char *filename: file to write.                                                                                */
/* elan_struct_t *elan_struct: Elan structure (read from header).                                                */
/* int measureNum: indice of measure to write (starting from 0).                                                 */
/* int chanNum: indice of channel (starting from 0).                                                             */
/* int sampNum: indice of first sample to write (starting from 0).                                               */
/* int sampNb: number of samples to write.                                                                       */
/* void *data_ptr: array of samples.                                                                             */
/* return value: number of samples actually written.                                                             */
/*****************************************************************************************************************/
int HDF5_write_EEG_data_sampblock_one_chan(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum, int sampNum, int sampNb, void *ptr_data)
{
	hid_t f, elan_group, eeg_group, eeg_dataset, file_dataspace, mem_dataspace;
	hsize_t *dims, *offset, val;
	
	f = H5Fopen(filename, H5F_ACC_RDWR, H5P_DEFAULT);
	if (f < 0)
	{
		fprintf(stderr, "ERROR: can't open file %s for writing.\n", filename);
		return(EF_ERR_OPEN_WRITE);
	}
	elan_group = H5Gopen(f, HDF5_ELAN_GROUP, H5P_DEFAULT);
		
	/* EEG group and dataset writing if needed. */
	/*------------------------------------------*/
	if (elan_struct->has_eeg == EF_YES)
	{
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
				offset[2] = sampNum;
				dims = (hsize_t *)calloc(3, sizeof(hsize_t));
				ELAN_CHECK_ERROR_ALLOC(dims, "for reading samples of one channel of continuous EEG (dims hyperslab).");
				val = sampNb;
				dims[0] = 1;
				dims[1] = 1;
				dims[2] = val;

				mem_dataspace = H5Screate_simple(1, &val, NULL); 
				
				H5Sselect_hyperslab(file_dataspace, H5S_SELECT_SET, offset, NULL, dims, NULL);
				switch (elan_struct->eeg.data_type)
				{
					case EF_DATA_FLOAT:
						H5Dwrite(eeg_dataset, H5T_NATIVE_FLOAT, mem_dataspace, file_dataspace, H5P_DEFAULT, ptr_data); /* Write the dataset data. */
						break;
					case EF_DATA_DOUBLE:
						H5Dwrite(eeg_dataset, H5T_NATIVE_DOUBLE, mem_dataspace, file_dataspace, H5P_DEFAULT, ptr_data); /* Write the dataset data. */
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
		H5Gclose(elan_group);
		/* Terminate access to the file. */
		H5Fclose(f);
		return(0);
	}
}


/*****************************************************************************************************************/
/* HDF5_write_EEG_data_sampblock_all_chan: write an EEG data block for all channel.                              */
/* char *filename: file to write.                                                                                */
/* elan_struct_t *elan_struct: Elan structure (read from header).                                                */
/* int measureNum: indice of measure to write (starting from 0).                                                 */
/* int sampNum: indice of first sample to write (starting from 0).                                               */
/* int sampNb: number of samples to write.                                                                       */
/* void **data_ptr: array of samples.                                                                            */
/* return value: number of samples actually written.                                                             */
/*****************************************************************************************************************/
int HDF5_write_EEG_data_sampblock_all_chan(char *filename, elan_struct_t *elan_struct, int measureNum, int sampNum, int sampNb, void **ptr_data)
{
	hid_t f, elan_group, eeg_group, eeg_dataset, file_dataspace, mem_dataspace;
	hsize_t *dims, *offset, val[2];
	
	f = H5Fopen(filename, H5F_ACC_RDWR, H5P_DEFAULT);
	if (f < 0)
	{
		fprintf(stderr, "ERROR: can't open file %s for writing.\n", filename);
		return(EF_ERR_OPEN_WRITE);
	}
	elan_group = H5Gopen(f, HDF5_ELAN_GROUP, H5P_DEFAULT);
	
	/* EEG group and dataset writing if needed. */
	/*------------------------------------------*/
	if (elan_struct->has_eeg == EF_YES)
	{
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
				offset[1] = 0;
				offset[2] = sampNum;
				dims = (hsize_t *)calloc(3, sizeof(hsize_t));
				ELAN_CHECK_ERROR_ALLOC(dims, "for reading samples of one channel of continuous EEG (dims hyperslab).");
				dims[0] = 1;
				dims[1] = elan_struct->chan_nb;
				dims[2] = sampNb;

				val[0] = elan_struct->chan_nb;
				val[1] = sampNb;
				mem_dataspace = H5Screate_simple(2, val, NULL); 
				
				H5Sselect_hyperslab(file_dataspace, H5S_SELECT_SET, offset, NULL, dims, NULL);
				switch (elan_struct->eeg.data_type)
				{
					case EF_DATA_FLOAT:
						H5Dwrite(eeg_dataset, H5T_NATIVE_FLOAT, mem_dataspace, file_dataspace, H5P_DEFAULT, ptr_data[0]); /* Write the dataset data. */
						break;
					case EF_DATA_DOUBLE:
						H5Dwrite(eeg_dataset, H5T_NATIVE_DOUBLE, mem_dataspace, file_dataspace, H5P_DEFAULT, ptr_data[0]); /* Write the dataset data. */
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
		return(elan_struct->chan_nb*sampNb);
	}
	else
	{
		H5Gclose(elan_group);
		/* Terminate access to the file. */
		H5Fclose(f);
		return(0);
	}
}


/*****************************************************************************************************************/
/* HDF5_write_EP_data_sampblock_one_chan: write an EP data block for one channel.                                */
/* char *filename: file to write.                                                                                */
/* elan_struct_t *elan_struct: Elan structure (read from header).                                                */
/* int measureNum: indice of measure to write (starting from 0).                                                 */
/* int chanNum: indice of channel (starting from 0).                                                             */
/* int sampNum: indice of first sample to write (starting from 0).                                               */
/* int sampNb: number of samples to write.                                                                       */
/* void *data_ptr: array of samples.                                                                             */
/* return value: number of samples actually written.                                                             */
/*****************************************************************************************************************/
int HDF5_write_EP_data_sampblock_one_chan(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum, int sampNum, int sampNb, void *ptr_data)
{
	hid_t f, elan_group, ep_group, ep_dataset, file_dataspace, mem_dataspace;
	hsize_t *dims, *offset, val;
	
	f = H5Fopen(filename, H5F_ACC_RDWR, H5P_DEFAULT);
	if (f < 0)
	{
		fprintf(stderr, "ERROR: can't open file %s for writing.\n", filename);
		return(EF_ERR_OPEN_WRITE);
	}
	elan_group = H5Gopen(f, HDF5_ELAN_GROUP, H5P_DEFAULT);
		
	/* EP group and dataset writing if needed. */
	/*-----------------------------------------*/
	if (elan_struct->has_ep == EF_YES)
	{
		ep_group = H5Gopen(elan_group, HDF5_EP_GROUP, H5P_DEFAULT);
		ep_dataset = H5Dopen2(ep_group, HDF5_EP_EPOCH_DSET, H5P_DEFAULT);
		file_dataspace = H5Dget_space(ep_dataset);    /* dataspace identifier */
		
		/* Hyperslab creation for one measure, one channel, some samples. */
		offset = (hsize_t *)calloc(3, sizeof(hsize_t));
		ELAN_CHECK_ERROR_ALLOC(offset, "for reading samples of one channel of continuous EP (offset hyperslab).");
		offset[0] = measureNum;
		offset[1] = chanNum;
		offset[2] = sampNum;
		dims = (hsize_t *)calloc(3, sizeof(hsize_t));
		ELAN_CHECK_ERROR_ALLOC(dims, "for reading samples of one channel of continuous EP (dims hyperslab).");
		val = sampNb;
		dims[0] = 1;
		dims[1] = 1;
		dims[2] = val;

		mem_dataspace = H5Screate_simple(1, &val, NULL); 
		
		H5Sselect_hyperslab(file_dataspace, H5S_SELECT_SET, offset, NULL, dims, NULL);
		switch (elan_struct->ep.data_type)
		{
			case EF_DATA_FLOAT:
				H5Dwrite(ep_dataset, H5T_NATIVE_FLOAT, mem_dataspace, file_dataspace, H5P_DEFAULT, ptr_data); /* Write the dataset data. */
				break;
			case EF_DATA_DOUBLE:
				H5Dwrite(ep_dataset, H5T_NATIVE_DOUBLE, mem_dataspace, file_dataspace, H5P_DEFAULT, ptr_data); /* Write the dataset data. */
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
		H5Gclose(elan_group);
		/* Terminate access to the file. */
		H5Fclose(f);
		return(0);
	}
}


/*****************************************************************************************************************/
/* HDF5_write_EP_data_sampblock_all_chan: write an EP data block for all channel.                                */
/* char *filename: file to write.                                                                                */
/* elan_struct_t *elan_struct: Elan structure (read from header).                                                */
/* int measureNum: indice of measure to write (starting from 0).                                                 */
/* int sampNum: indice of first sample to write (starting from 0).                                               */
/* int sampNb: number of samples to write.                                                                       */
/* void **data_ptr: array of samples.                                                                            */
/* return value: number of samples actually written.                                                             */
/*****************************************************************************************************************/
int HDF5_write_EP_data_sampblock_all_chan(char *filename, elan_struct_t *elan_struct, int measureNum, int sampNum, int sampNb, void **ptr_data)
{
	hid_t f, elan_group, ep_group, ep_dataset, file_dataspace, mem_dataspace;
	hsize_t *dims, *offset, val[2];
	
	f = H5Fopen(filename, H5F_ACC_RDWR, H5P_DEFAULT);
	if (f < 0)
	{
		fprintf(stderr, "ERROR: can't open file %s for writing.\n", filename);
		return(EF_ERR_OPEN_WRITE);
	}
	elan_group = H5Gopen(f, HDF5_ELAN_GROUP, H5P_DEFAULT);
		
	/* EP group and dataset writing if needed. */
	/*-----------------------------------------*/
	if (elan_struct->has_ep == EF_YES)
	{
		ep_group = H5Gopen(elan_group, HDF5_EP_GROUP, H5P_DEFAULT);
		ep_dataset = H5Dopen2(ep_group, HDF5_EP_EPOCH_DSET, H5P_DEFAULT);
		file_dataspace = H5Dget_space(ep_dataset);    /* dataspace identifier */
		
		/* Hyperslab creation for one measure, one channel, some samples. */
		offset = (hsize_t *)calloc(3, sizeof(hsize_t));
		ELAN_CHECK_ERROR_ALLOC(offset, "for reading samples of one channel of continuous EP (offset hyperslab).");
		offset[0] = measureNum;
		offset[1] = 0;
		offset[2] = sampNum;
		dims = (hsize_t *)calloc(3, sizeof(hsize_t));
		ELAN_CHECK_ERROR_ALLOC(dims, "for reading samples of one channel of continuous EP (dims hyperslab).");
		dims[0] = 1;
		dims[1] = elan_struct->chan_nb;
		dims[2] = sampNb;

		val[0] = elan_struct->chan_nb;
		val[1] = sampNb;
		mem_dataspace = H5Screate_simple(2, val, NULL); 
		
		H5Sselect_hyperslab(file_dataspace, H5S_SELECT_SET, offset, NULL, dims, NULL);
		switch (elan_struct->ep.data_type)
		{
			case EF_DATA_FLOAT:
				H5Dwrite(ep_dataset, H5T_NATIVE_FLOAT, mem_dataspace, file_dataspace, H5P_DEFAULT, ptr_data[0]); /* Write the dataset data. */
				break;
			case EF_DATA_DOUBLE:
				H5Dwrite(ep_dataset, H5T_NATIVE_DOUBLE, mem_dataspace, file_dataspace, H5P_DEFAULT, ptr_data[0]); /* Write the dataset data. */
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
		return(elan_struct->chan_nb*sampNb);
	}
	else
	{
		H5Gclose(elan_group);
		/* Terminate access to the file. */
		H5Fclose(f);
		return(0);
	}
}


/****************************************************************************************************************/
/* HDF5_write_TF_tf_one_channel: write a time-frequency block for one channel.                                  */
/* filename: name of the file to read datra from.                                                               */
/* elan_struct_t *elan_struct: Elan structure (read from header).                                               */
/* int measureNum: indice of measure to write to (starting from 0).                                             */
/* int chanNum: indice of channel to write (starting from 0).                                                   */
/* int sampStart: indice of 1st sample to write (starting from 0).                                              */
/* int sampNb: number of samples to write (starting from 0).                                                    */
/* int freqStart: indice of 1st frequency to read (starting from 0).                                            */
/* int freqNb: number of frequencies to read (starting from 0).                                                 */
/* void **ptr_data: array of samples.                                                                           */
/* return value: number of samples actually written.                                                            */
/****************************************************************************************************************/
int HDF5_write_TF_tf_one_channel(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum, int sampStart, int sampNb, int freqStart, int freqNb, void **ptr_data)
{
	hid_t f, elan_group, tf_group, tf_dataset, file_dataspace, mem_dataspace;
	hsize_t *dims, *offset, val[2];
	int err;
	
	f = H5Fopen(filename, H5F_ACC_RDWR, H5P_DEFAULT);
	if (f < 0)
	{
		fprintf(stderr, "ERROR: can't open file %s for writing.\n", filename);
		return(EF_ERR_OPEN_WRITE);
	}
	elan_group = H5Gopen(f, HDF5_ELAN_GROUP, H5P_DEFAULT);
		
	/* TF group and dataset writing if needed. */
	/*-----------------------------------------*/
	if (elan_struct->has_tf == EF_YES)
	{
		tf_group = H5Gopen(elan_group, HDF5_TF_GROUP, H5P_DEFAULT);
		tf_dataset = H5Dopen2(tf_group, HDF5_TF_ARRAY_DSET, H5P_DEFAULT);
		file_dataspace = H5Dget_space(tf_dataset);    /* dataspace identifier */
		
		/* Hyperslab creation for one measure, one channel, a block of frequencies, all samples. */
		offset = (hsize_t *)calloc(4, sizeof(hsize_t));
		ELAN_CHECK_ERROR_ALLOC(offset, "for writing a block of samples on one channel of TF (offset hyperslab).");
		offset[0] = measureNum;
		offset[1] = chanNum;
		offset[2] = freqStart;
		offset[3] = sampStart;
		dims = (hsize_t *)calloc(4, sizeof(hsize_t));
		ELAN_CHECK_ERROR_ALLOC(dims, "for writing a block of samples on one channel of TF (dims hyperslab).");
		val[0] = freqNb;
		val[1] = sampNb;
		dims[0] = 1;
		dims[1] = 1;
		dims[2] = val[0];
		dims[3] = val[1];

    mem_dataspace = H5Screate_simple(2, val, NULL); 
		
		err = H5Sselect_hyperslab(file_dataspace, H5S_SELECT_SET, offset, NULL, dims, NULL);
		if (err < 0)
		{
			fprintf(stderr, "ERROR: in HDF5 hyperslab selection of file.\n");
		}
		switch (elan_struct->tf.data_type)
		{
			case EF_DATA_FLOAT:
				H5Dwrite(tf_dataset, H5T_NATIVE_FLOAT, mem_dataspace, file_dataspace, H5P_DEFAULT, ptr_data[0]); /* Write the dataset data. */
				break;
			case EF_DATA_DOUBLE:
				H5Dwrite(tf_dataset, H5T_NATIVE_DOUBLE, mem_dataspace, file_dataspace, H5P_DEFAULT, ptr_data[0]); /* Write the dataset data. */
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
		H5Gclose(elan_group);
		/* Terminate access to the file. */
		H5Fclose(f);
		return(0);
	}
}


/****************************************************************************************************************/
/* HDF5_write_TF_tf_all_channel: write a time-frequency block for all channels.                                 */
/* filename: name of the file to read datra from.                                                               */
/* elan_struct_t *elan_struct: Elan structure (read from header).                                               */
/* int measureNum: indice of measure to write to (starting from 0).                                             */
/* int sampStart: indice of 1st sample to write (starting from 0).                                              */
/* int sampNb: number of samples to write (starting from 0).                                                    */
/* int freqStart: indice of 1st frequency to read (starting from 0).                                            */
/* int freqNb: number of frequencies to read (starting from 0).                                                 */
/* void **ptr_data: array of samples.                                                                           */
/* return value: number of samples actually written.                                                            */
/****************************************************************************************************************/
int HDF5_write_TF_tf_all_channel(char *filename, elan_struct_t *elan_struct, int measureNum, int sampStart, int sampNb, int freqStart, int freqNb, void ***ptr_data)
{
	hid_t f, elan_group, tf_group, tf_dataset, file_dataspace, mem_dataspace;
	hsize_t *dims, *offset, val[3];
	int err;
	
	f = H5Fopen(filename, H5F_ACC_RDWR, H5P_DEFAULT);
	if (f < 0)
	{
		fprintf(stderr, "ERROR: can't open file %s for writing.\n", filename);
		return(EF_ERR_OPEN_WRITE);
	}
	elan_group = H5Gopen(f, HDF5_ELAN_GROUP, H5P_DEFAULT);
		
	/* TF group and dataset writing if needed. */
	/*-----------------------------------------*/
	if (elan_struct->has_tf == EF_YES)
	{
		tf_group = H5Gopen(elan_group, HDF5_TF_GROUP, H5P_DEFAULT);
		tf_dataset = H5Dopen2(tf_group, HDF5_TF_ARRAY_DSET, H5P_DEFAULT);
		file_dataspace = H5Dget_space(tf_dataset);    /* dataspace identifier */
		
		/* Hyperslab creation for one measure, one channel, a block of frequencies, all samples. */
		offset = (hsize_t *)calloc(4, sizeof(hsize_t));
		ELAN_CHECK_ERROR_ALLOC(offset, "for writing a block of samples on all channel of TF (offset hyperslab).");
		offset[0] = measureNum;
		offset[1] = 0;
		offset[2] = freqStart;
		offset[3] = sampStart;
		dims = (hsize_t *)calloc(4, sizeof(hsize_t));
		ELAN_CHECK_ERROR_ALLOC(dims, "for writing a block of samples on all channel of TF (dims hyperslab).");
		val[0] = elan_struct->chan_nb;
		val[1] = freqNb;
		val[2] = sampNb;
		dims[0] = 1;
		dims[1] = val[0];
		dims[2] = val[1];
		dims[3] = val[2];

    mem_dataspace = H5Screate_simple(3, val, NULL); 
		
		err = H5Sselect_hyperslab(file_dataspace, H5S_SELECT_SET, offset, NULL, dims, NULL);
		if (err < 0)
		{
			fprintf(stderr, "ERROR: in HDF5 hyperslab selection of file.\n");
		}
		switch (elan_struct->tf.data_type)
		{
			case EF_DATA_FLOAT:
				H5Dwrite(tf_dataset, H5T_NATIVE_FLOAT, mem_dataspace, file_dataspace, H5P_DEFAULT, ptr_data[0][0]); /* Write the dataset data. */
				break;
			case EF_DATA_DOUBLE:
				H5Dwrite(tf_dataset, H5T_NATIVE_DOUBLE, mem_dataspace, file_dataspace, H5P_DEFAULT, ptr_data[0][0]); /* Write the dataset data. */
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
		H5Gclose(elan_group);
		/* Terminate access to the file. */
		H5Fclose(f);
		return(0);
	}
}

