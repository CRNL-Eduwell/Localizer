/*--------------------------------------------------*/
/* Tools for HDF5 ELAN files module.                */
/* v: 1.00    Aut.: PEA                             */
/* CRNL / INSERM U1028 / CNRS UMR 5292 / Univ-Lyon1 */
/*--------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>

#include "elanfile.h"
#include "ef_hdf5common.h"


/* Extern function prototype. */
extern int ef_test_suffix(char *filename, char *suffix); /* Defined in ef_tools.c */


/*---------------------------------------------------------------*/
/* ef_HDF5create_write_attribute: create and write an attribute. */
/*---------------------------------------------------------------*/
herr_t ef_HDF5create_write_attribute(hid_t loc_id, char *attr_name, hid_t type_id, hid_t space_id, hid_t acpl_id, hid_t aapl_id, hid_t mem_type_id, void *buf)
{
	hid_t attrib;
	herr_t err;
	attrib = H5Acreate2(loc_id, attr_name, type_id, space_id, acpl_id, aapl_id);
	err = H5Awrite(attrib, mem_type_id, buf); /* Write the attribute data. */
	H5Aclose(attrib); /* Close the attribute. */
	return(err);
}


/*---------------------------------------------------------*/
/* ef_HDF5open_read_attribute: open and read an attribute. */
/*---------------------------------------------------------*/
herr_t ef_HDF5open_read_attribute(hid_t obj_id, char *attr_name, hid_t aapl_id, hid_t mem_type_id, void *buf)
{
	hid_t attrib;
	herr_t err;
	attrib = H5Aopen(obj_id, attr_name, aapl_id);
	err = H5Aread(attrib, mem_type_id, buf); /* Read the attribute data. */
	H5Aclose(attrib); /* Close the attribute. */
	return(err);
}


/******************************************************************************************************************/
/* ef_HDF5create_basic_types_dataspaces: creates basic types and dataspaces (doesn't need header initialisation). */
/******************************************************************************************************************/
void ef_HDF5create_basic_types_dataspaces(hdf_dataspaces_t *dataspaces, hdf_types_t *types)
{
	/* Dataspaces creation. */
	dataspaces->scalar_dataspace_id = H5Screate(H5S_SCALAR); /* Create scalar dataspace. */
	
	/* Special types creation. */
	types->str_comment_type = H5Tcopy(H5T_C_S1);
	H5Tset_size(types->str_comment_type, COMMENT_SIZE); /* create string of length COMMENT_SIZE */
	H5Tset_cset(types->str_comment_type, H5T_CSET_UTF8);
	H5Tset_strpad(types->str_comment_type, H5T_STR_NULLTERM);
	
	types->str_label_type = H5Tcopy(H5T_C_S1);
	H5Tset_size(types->str_label_type, LABEL_SIZE); /* create string of length LABEL_SIZE */
	H5Tset_cset(types->str_label_type, H5T_CSET_UTF8);
	H5Tset_strpad(types->str_label_type, H5T_STR_NULLTERM);

	types->float_array_memtype = H5Tvlen_create(H5T_NATIVE_FLOAT); /* Create simple dataspace (regular array) for coordinates. */
	
	types->coord_memtype = H5Tcreate(H5T_COMPOUND, sizeof(hdf_coord_mem_t));
	H5Tinsert(types->coord_memtype, HDF5_CHANNEL_COORDLAB_TYPE, HOFFSET(hdf_coord_mem_t, coord_lab), types->str_label_type);
	H5Tinsert(types->coord_memtype, HDF5_CHANNEL_VALUELIST_TYPE, HOFFSET(hdf_coord_mem_t, value_list), types->float_array_memtype);
	
	types->coord_list_memtype = H5Tvlen_create(types->coord_memtype);

	/* Create the compound datatype for memory. */
	types->chan_memtype = H5Tcreate(H5T_COMPOUND, sizeof(hdf_chan_mem_t));
	H5Tinsert(types->chan_memtype, HDF5_CHANNEL_LABEL_TYPE, HOFFSET(hdf_chan_mem_t, lab), types->str_label_type);
	H5Tinsert(types->chan_memtype, HDF5_CHANNEL_TYPE_TYPE, HOFFSET(hdf_chan_mem_t, type), types->str_label_type);
	H5Tinsert(types->chan_memtype, HDF5_CHANNEL_UNIT_TYPE, HOFFSET(hdf_chan_mem_t, unit), types->str_label_type);
	H5Tinsert(types->chan_memtype, HDF5_CHANNEL_COORDLIST_TYPE, HOFFSET(hdf_chan_mem_t, coord_list), types->coord_list_memtype);
	
	dataspaces->ep_otherevent_dataspace_id = H5P_DEFAULT; /* Default value if uninitialized. */
	dataspaces->tf_otherevent_dataspace_id = H5P_DEFAULT; /* Default value if uninitialized. */
	dataspaces->freq_dataspace_id = H5P_DEFAULT; /* Default value if uninitialized. */
}


/***********************************************************************************************************************/
/* ef_HDF5create_fromheader_types_dataspaces: advanced dataspaces and types initialization (needs initialized header). */
/***********************************************************************************************************************/
void ef_HDF5create_fromheader_types_dataspaces(elan_struct_t *elan_struct, hdf_dataspaces_t *dataspaces, hdf_types_t *types)
{
	hsize_t val;
	
	val = elan_struct->measure_channel_nb;
	dataspaces->measure_dataspace_id = H5Screate_simple(1, &val, NULL); /* Create simple dataspace (regular array) for measure labels. */
	val = elan_struct->chan_nb;
	dataspaces->chan_dataspace_id = H5Screate_simple(1, &val, NULL);
	
	types->float_array_filetype = H5Tvlen_create(CHOOSE_ENDIAN_FLT32(elan_struct->endian)); /* Create simple dataspace (regular array) for coordinates. */
	/* Create the compound datatype for the file.  Because the standard
		* types we are using for the file may have different sizes than
		* the corresponding native types, we must manually calculate the
		* offset of each member. */
	types->coord_filetype = H5Tcreate(H5T_COMPOUND, sizeof(hdf_coord_mem_t));
	H5Tinsert(types->coord_filetype, HDF5_CHANNEL_COORDLAB_TYPE, 0, types->str_label_type);
	H5Tinsert(types->coord_filetype, HDF5_CHANNEL_VALUELIST_TYPE, sizeof(str_label_t), types->float_array_filetype);

	types->coord_list_filetype = H5Tvlen_create(types->coord_filetype);
	
	/* Create the compound datatype for the file.  Because the standard
		* types we are using for the file may have different sizes than
		* the corresponding native types, we must manually calculate the
		* offset of each member. */
	types->chan_filetype = H5Tcreate(H5T_COMPOUND, sizeof(hdf_chan_mem_t));
	H5Tinsert(types->chan_filetype, HDF5_CHANNEL_LABEL_TYPE, 0, types->str_label_type);
	H5Tinsert(types->chan_filetype, HDF5_CHANNEL_TYPE_TYPE, sizeof(str_label_t), types->str_label_type);
	H5Tinsert(types->chan_filetype, HDF5_CHANNEL_UNIT_TYPE, 2*sizeof(str_label_t), types->str_label_type);
	H5Tinsert(types->chan_filetype, HDF5_CHANNEL_COORDLIST_TYPE, 3*sizeof(str_label_t), types->coord_list_filetype);
}


/***********************************************************************************************************************/
/* ef_HDF5create_ep_types_dataspaces: advanced dataspaces and types initialization for EP (needs initialized header).  */
/***********************************************************************************************************************/
void ef_HDF5create_ep_types_dataspaces(elan_struct_t *elan_struct, hdf_dataspaces_t *dataspaces)
{
	hsize_t val;
	int i;
	dataspaces->ep_otherevent_dataspace_id = H5P_DEFAULT;
	if (elan_struct->has_ep == EF_YES)
	{
		val = 0;
		for (i=0; i<elan_struct->measure_channel_nb; i++) {
			val += elan_struct->ep.other_events_nb[i];
		}
		if (val > 0) {
			dataspaces->ep_otherevent_dataspace_id = H5Screate_simple(1, &val, NULL);
		}
	}
}


/***********************************************************************************************************************/
/* ef_HDF5create_tf_types_dataspaces: advanced dataspaces and types initialization for TF (needs initialized header).  */
/***********************************************************************************************************************/
void ef_HDF5create_tf_types_dataspaces(elan_struct_t *elan_struct, hdf_dataspaces_t *dataspaces)
{
	hsize_t val;
	int i;
	dataspaces->freq_dataspace_id = H5P_DEFAULT;
	dataspaces->tf_otherevent_dataspace_id = H5P_DEFAULT;
	if (elan_struct->has_tf == EF_YES)
	{
		val = elan_struct->tf.freq_nb;
		dataspaces->freq_dataspace_id = H5Screate_simple(1, &val, NULL);
		val = 0;
		for (i=0; i<elan_struct->measure_channel_nb; i++) {
			val += elan_struct->tf.other_events_nb[i];
		}
		if (val > 0) {
			dataspaces->tf_otherevent_dataspace_id = H5Screate_simple(1, &val, NULL);
		}
	}
	
}


/*-----------------------------------------------------------------------------------*/
/* ef_HDF5close_types_dataspaces: close all dataspaces and types previously created. */
/*-----------------------------------------------------------------------------------*/
void ef_HDF5close_types_dataspaces(hdf_dataspaces_t *dataspaces, hdf_types_t *types)
{
	H5Sclose(dataspaces->scalar_dataspace_id); /* Close the scalar dataspace. */
  H5Sclose(dataspaces->measure_dataspace_id); /* Close the measure dataspace. */
	H5Sclose(dataspaces->chan_dataspace_id); /* Close the channel dataspace. */
	if (dataspaces->ep_otherevent_dataspace_id != H5P_DEFAULT) {
		H5Sclose(dataspaces->ep_otherevent_dataspace_id);
	}
	if (dataspaces->tf_otherevent_dataspace_id != H5P_DEFAULT) {
		H5Sclose(dataspaces->tf_otherevent_dataspace_id);
	}
	if (dataspaces->freq_dataspace_id != H5P_DEFAULT)
	{
		H5Sclose(dataspaces->freq_dataspace_id);
	}
	
	H5Tclose(types->str_comment_type);
	H5Tclose(types->coord_memtype);
	H5Tclose(types->coord_filetype);
	H5Tclose(types->coord_list_memtype);
	H5Tclose(types->coord_list_filetype);
	H5Tclose(types->chan_memtype);
	H5Tclose(types->chan_filetype);
	H5Tclose(types->str_label_type);
	H5Tclose(types->float_array_memtype);
	H5Tclose(types->float_array_filetype);
}


/*------------------------------------------------------------------------------------------------------*/
/* ef_HDF5copy_other_groups: copy other HDF5 members that are not from Elan from source to destination. */
/*------------------------------------------------------------------------------------------------------*/
void ef_HDF5copy_other_groups(char *dest, char *src)
{
}
