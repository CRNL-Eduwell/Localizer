/************************************/
/* Common declaration for HDF5 use. */
/************************************/
#ifndef _ef_hdf5common_h
#define _ef_hdf5common_h

#include <C:\Program Files\HDF_Group\HDF5\1.10.1\include\hdf5.h>


/* Macro for determine endian of attributes in file. */
#define CHOOSE_ENDIAN_INT32(endian) ((endian==EF_LITTLE_ENDIAN)?H5T_STD_I32LE:H5T_STD_I32BE)
#define CHOOSE_ENDIAN_FLT32(endian) ((endian==EF_LITTLE_ENDIAN)?H5T_IEEE_F32LE:H5T_IEEE_F32BE)
#define CHOOSE_ENDIAN_FLT64(endian) ((endian==EF_LITTLE_ENDIAN)?H5T_IEEE_F64LE:H5T_IEEE_F64BE)

/* Macro for HDF5 labels. */
/*------------------------*/
#define HDF5_ELAN_GROUP "/elan"
#define HDF5_COMMON_GROUP "common"
#define HDF5_ENDIAN_ATTRIB "endian"
#define HDF5_VERSION_ATTRIB "version"
#define HDF5_RELEASE_ATTRIB "release"
#define HDF5_DATATYPE_ATTRIB "data_type"
#define HDF5_COMMENT_ATTRIB "comment"
#define HDF5_CHANNELNB_ATTRIB "channel_nb"
#define HDF5_MEASURENB_ATTRIB "measure_nb"
#define HDF5_MEASURELIST_ATTRIB "measure_list"

#define HDF5_CHANNEL_DSET "channels"
#define HDF5_CHANNEL_COORDLAB_TYPE "coord_lab"
#define HDF5_CHANNEL_VALUELIST_TYPE "value_list"
#define HDF5_CHANNEL_LABEL_TYPE "label"
#define HDF5_CHANNEL_UNIT_TYPE "unit"
#define HDF5_CHANNEL_TYPE_TYPE "type"
#define HDF5_CHANNEL_COORDLIST_TYPE "coord_list"

#define HDF5_EEG_GROUP "eeg"
#define HDF5_EEG_SAMPNB_ATTRIB "sample_nb"
#define HDF5_EEG_SAMPFREQ_ATTRIB "sampling_frequency"
#define HDF5_EEG_CONT_DSET "continuous"
#define HDF5_EEG_EVENTNB_ATTRIB "event_nb"
#define HDF5_EEG_EVENTCODE_ATTRIB "event_code_list"
#define HDF5_EEG_EVENTTIME_ATTRIB "event_sample_list"
#define HDF5_EEG_EPOCH_DSET "epoch"

#define HDF5_EP_GROUP "ep"
#define HDF5_EP_SAMPNB_ATTRIB "sample_nb"
#define HDF5_EP_SAMPFREQ_ATTRIB "sampling_frequency"
#define HDF5_EP_PRESTIM_ATTRIB "prestim_sample_nb"
#define HDF5_EP_EVENTCODE_ATTRIB "event_code"
#define HDF5_EP_EVENTNB_ATTRIB "event_nb"
#define HDF5_EP_EPOCH_DSET "epoch"

#define HDF5_EPTF_OTHEREVENTNB_ATTRIB "other_event_nb"
#define HDF5_EPTF_OTHEREVENTLIST_ATTRIB "other_event_list"

#define HDF5_TF_GROUP "tf"
#define HDF5_TF_SAMPNB_ATTRIB "sample_nb"
#define HDF5_TF_SAMPFREQ_ATTRIB "sampling_frequency"
#define HDF5_TF_PRESTIM_ATTRIB "prestim_sample_nb"
#define HDF5_TF_EVENTCODE_ATTRIB "event_code"
#define HDF5_TF_EVENTNB_ATTRIB "event_nb"
#define HDF5_TF_WAVELETTYPE_ATTRIB "wavelet_type"
#define HDF5_TF_BLACKMAN_ATTRIB "blackman_wnd_sample_nb"
#define HDF5_TF_FREQUENCYNB_ATTRIB "frequency_nb"
#define HDF5_TF_FREQUENCYLIST_ATTRIB "frequency_list"
#define HDF5_TF_WAVELETCHAR_ATTRIB "wavelet_characteristic"
#define HDF5_TF_ARRAY_DSET "array"


/* Type definitions. */
typedef struct
{
	str_label_t coord_lab;
	hvl_t value_list;
} hdf_coord_mem_t;
typedef struct
{
	str_label_t lab;
	str_label_t type;
	str_label_t unit;
	hvl_t coord_list;
} hdf_chan_mem_t;

typedef struct
{
	hid_t str_comment_type;
	hid_t str_label_type;
	hid_t float_array_memtype;
	hid_t float_array_filetype;
	hid_t coord_memtype;
	hid_t coord_filetype;
	hid_t coord_list_memtype;
	hid_t coord_list_filetype;
	hid_t chan_memtype;
	hid_t chan_filetype;
} hdf_types_t;

typedef struct
{
	hid_t scalar_dataspace_id;
	hid_t measure_dataspace_id;
	hid_t chan_dataspace_id;
	hid_t ep_otherevent_dataspace_id;
	hid_t freq_dataspace_id;
	hid_t tf_otherevent_dataspace_id;
} hdf_dataspaces_t;


/* Function prototypes. */
/*----------------------*/
int HDF5_write_header(char *filename, elan_struct_t *elan_struct);
int HDF5_write_data_all_channels(char *filename, elan_struct_t *elan_struct);
int HDF5_write_data_one_channel(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum);
int HDF5_write_EEG_data_sampblock_one_chan(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum, int sampNum, int sampNb, void *ptr_data);
int HDF5_write_EEG_data_sampblock_all_chan(char *filename, elan_struct_t *elan_struct, int measureNum, int sampNum, int sampNb, void **ptr_data);
int HDF5_write_EP_data_sampblock_one_chan(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum, int sampNum, int sampNb, void *ptr_data);
int HDF5_write_EP_data_sampblock_all_chan(char *filename, elan_struct_t *elan_struct, int measureNum, int sampNum, int sampNb, void **ptr_data);
int HDF5_write_TF_tf_one_channel(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum, int sampStart, int sampNb, int freqStart, int freqNb, void **ptr_data);
int HDF5_write_TF_tf_all_channel(char *filename, elan_struct_t *elan_struct, int measureNum, int sampStart, int sampNb, int freqStart, int freqNb, void ***ptr_data);

int HDF5_read_version(char *filename);
int HDF5_read_header(char *filename, elan_struct_t *elan_struct);
int HDF5_read_data_all_channels(char *filename, elan_struct_t *elan_struct);
int HDF5_read_data_one_channel(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum);
int HDF5_read_EEG_data_sampblock_one_chan(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum, int sampStart, int sampNb, void **data_ptr);
int HDF5_read_EEG_data_sampblock_all_chan(char *filename, elan_struct_t *elan_struct, int measureNum, int sampStart, int sampNb, void ***data_ptr);
int HDF5_read_EP_data_sampblock_one_chan(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum, int sampStart, int sampNb, void **data_ptr);
int HDF5_read_EP_data_sampblock_all_chan(char *filename, elan_struct_t *elan_struct, int measureNum, int sampStart, int sampNb, void ***data_ptr);
int HDF5_read_TF_tfblock_one_channel(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum, int sampStart, int sampNb, int freqStart, int freqNb, void ***data_ptr);
int HDF5_read_TF_tfblock_all_channel(char *filename, elan_struct_t *elan_struct, int measureNum, int sampStart, int sampNb, int freqStart, int freqNb, void ****data_ptr);

herr_t ef_HDF5create_write_attribute(hid_t loc_id, char *attr_name, hid_t type_id, hid_t space_id, hid_t acpl_id, hid_t aapl_id, hid_t mem_type_id, void *buf);
herr_t ef_HDF5open_read_attribute(hid_t obj_id, char *attr_name, hid_t aapl_id, hid_t mem_type_id, void *buf);
void ef_HDF5create_basic_types_dataspaces(hdf_dataspaces_t *dataspaces, hdf_types_t *types);
void ef_HDF5create_fromheader_types_dataspaces(elan_struct_t *elan_struct, hdf_dataspaces_t *dataspaces, hdf_types_t *types);
void ef_HDF5create_ep_types_dataspaces(elan_struct_t *elan_struct, hdf_dataspaces_t *dataspaces);
void ef_HDF5create_tf_types_dataspaces(elan_struct_t *elan_struct, hdf_dataspaces_t *dataspaces);
void ef_HDF5close_types_dataspaces(hdf_dataspaces_t *dataspaces, hdf_types_t *types);

#else
extern int HDF5_write_header(char *filename, elan_struct_t *elan_struct);
extern int HDF5_write_data_all_channels(char *filename, elan_struct_t *elan_struct);
extern int HDF5_write_data_one_channel(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum);
extern int HDF5_write_EEG_data_sampblock_one_chan(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum, int sampNum, int sampNb, void *ptr_data);
extern int HDF5_write_EEG_data_sampblock_all_chan(char *filename, elan_struct_t *elan_struct, int measureNum, int sampNum, int sampNb, void **ptr_data);
extern int HDF5_write_EP_data_sampblock_one_chan(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum, int sampNum, int sampNb, void *ptr_data);
extern int HDF5_write_EP_data_sampblock_all_chan(char *filename, elan_struct_t *elan_struct, int measureNum, int sampNum, int sampNb, void **ptr_data);
extern int HDF5_write_TF_tf_one_channel(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum, int sampStart, int sampNb, int freqStart, int freqNb, void **ptr_data);
extern int HDF5_write_TF_tf_all_channel(char *filename, elan_struct_t *elan_struct, int measureNum, int sampStart, int sampNb, int freqStart, int freqNb, void ***ptr_data);

extern int HDF5_read_version(char *filename);
extern int HDF5_read_header(char *filename, elan_struct_t *elan_struct);
extern int HDF5_read_data_all_channels(char *filename, elan_struct_t *elan_struct);
extern int HDF5_read_data_one_channel(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum);
extern int HDF5_read_EEG_data_sampblock_one_chan(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum, int sampStart, int sampNb, void **data_ptr);
extern int HDF5_read_EEG_data_sampblock_all_chan(char *filename, elan_struct_t *elan_struct, int measureNum, int sampStart, int sampNb, void **data_ptr);
extern int HDF5_read_EP_data_sampblock_one_chan(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum, int sampStart, int sampNb, void **data_ptr);
extern int HDF5_read_EP_data_sampblock_all_chan(char *filename, elan_struct_t *elan_struct, int measureNum, int sampStart, int sampNb, void **data_ptr);
extern int HDF5_read_TF_tfblock_one_channel(char *filename, elan_struct_t *elan_struct, int measureNum, int chanNum, int sampStart, int sampNb, int freqStart, int freqNb, void ***data_ptr);
extern int HDF5_read_TF_tfblock_all_channel(char *filename, elan_struct_t *elan_struct, int measureNum, int sampStart, int sampNb, int freqStart, int freqNb, void ****data_ptr);

extern herr_t ef_HDF5create_write_attribute(hid_t loc_id, char *attr_name, hid_t type_id, hid_t space_id, hid_t acpl_id, hid_t aapl_id, hid_t mem_type_id, void *buf);
extern herr_t ef_HDF5open_read_attribute(hid_t obj_id, char *attr_name, hid_t aapl_id, hid_t mem_type_id, void *buf);
extern void ef_HDF5create_basic_types_dataspaces(hdf_dataspaces_t *dataspaces, hdf_types_t *types);
extern void ef_HDF5create_fromheader_types_dataspaces(elan_struct_t *elan_struct, hdf_dataspaces_t *dataspaces, hdf_types_t *types);
extern void ef_HDF5create_ep_types_dataspaces(elan_struct_t *elan_struct, hdf_dataspaces_t *dataspaces);
extern void ef_HDF5create_tf_types_dataspaces(elan_struct_t *elan_struct, hdf_dataspaces_t *dataspaces);
extern void ef_HDF5close_types_dataspaces(hdf_dataspaces_t *dataspaces, hdf_types_t *types);


#endif
