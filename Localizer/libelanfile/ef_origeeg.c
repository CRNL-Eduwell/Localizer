/*--------------------------------------------------*/
/* Reading original EEG ELAN files module.          */
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
#include <math.h>

#define VERSION_POS 2

/* Definition des structures format standard europeen. */
typedef struct
{
	char version[256];
	char patient[256];
	char identificateur[256];
	char startdate[256];
	char starttime[256];
	char taille_entete[256];
	char reserved[256];
	char nb_record[256];
	char duree_record[256];
	char nb_signaux[256];
} main_header_t;

typedef struct
{
	char label[256];
	char capteur[256];
	char unite[256];
	char min_physic[256];
	char max_physic[256];
	char min_digit[256];
	char max_digit[256];
	char filtre[256];
	char nbech[256];
	char reserved[256];
} channel_header_t;


/* Local function prototypes. */
/*----------------------------*/
int ef_readcar(FILE *f, int nb_car, char *str);
int ef_look_eve_file_backward(FILE *d_file, int nva, int *nb_eve, int **tab_pos, int **tab_eve, int **tab_info, int version_eeg, int sample_size);

/* External function prototypes. */
/*-------------------------------*/
extern void ef_orig_read_elecdat(int *coord_nb, elec_dat_t **coord_list); /* Defined in ef_origtools.c */
extern void ef_orig_free_elecdat(elec_dat_t **coord_list); /* Defined in ef_origtools.c */
extern size_t freadswab(void *ptr, size_t size, size_t nitems, FILE *stream); /* Defined in ef_origtools.c */



/*----------------------------------------------------------------------------------------------------------*/
/*                                                                                                          */
/* remplissage des structures generales decrivant un enregistrement continu suivant format europeen         */
/*     modifie INSERM par suppression du formatage obligatoire des lignes                                   */
/*     la premiere structure definie en globale comporte les parametres generaux de l'enregistrement        */
/*     la deuxieme structure definie les paramtres propres a chaque voie                                    */
/* les deux structures sont main_header et channel_header                                                   */
/* la fonction retourne normalement 0 sinon -1 si l'ouverture du fichier definissant les structures         */
/*     est impossible, et -2 si une lecture est impossible                                                  */
/*                                                                                                          */
/*----------------------------------------------------------------------------------------------------------*/
int ef_readeegent(char *header_filename, char *data_filename, elan_struct_t *elan_struct)
{
	FILE *fic_ent, *data;
	int j, ns, i_c, elec_num, elec_dat_nb;
	channel_header_t *channel_header;
	main_header_t main_header;
	elec_dat_t *elec_dat_list;
	double *eeg_maxphysic, *eeg_minphysic, *eeg_maxdigit, *eeg_mindigit;
	int pos_err, nb_eve, *tab_pos, *tab_eve, *tab_info, version_eeg, sample_size;

	/* Opening header file. */
	fic_ent = fopen(header_filename, "rt");
	if ( fic_ent == NULL )
		{
		fprintf(stderr, "ERROR opening file %s\n", header_filename);
		return(EF_ERR_OPEN_READ);
		}

	/* Parameters reading. */
	memset(main_header.version, 0, 256);
	if (ef_readcar(fic_ent, 255, main_header.version) == 0)
		{
		fprintf(stderr, "ERROR reading file %s (reading version)\n", header_filename);
		fclose(fic_ent);
		return(EF_ERR_READ_HEADER);
		}

	/* Check version. */
	/*----------------*/
	if (strcmp(main_header.version, "V3") == 0)
	{
		elan_struct->orig_info->eeg_info.orig_datatype = ORIG_EEG_DATATYPE_32BITS;
	}
	else if (strcmp(main_header.version, "V2") == 0)
	{
		elan_struct->orig_info->eeg_info.orig_datatype = ORIG_EEG_DATATYPE_16BITS;
	}
	else
	{
		elan_struct->orig_info->eeg_info.orig_datatype = ORIG_EEG_DATATYPE_12BITS;
	}
		
	/* patient. */
	memset(main_header.patient, 0, 256);
	if (ef_readcar(fic_ent, 255, main_header.patient) == 0)
		{
		fprintf(stderr, "ERROR reading file %s (reading patient information)\n", header_filename);
		fclose(fic_ent);
		return(EF_ERR_READ_HEADER);
		}

	/* comment. */
	memset(main_header.identificateur, 0, 256);
	if (ef_readcar(fic_ent, 255, main_header.identificateur) == 0)
		{
		fprintf(stderr, "ERROR reading file %s (reading local iden.)\n", header_filename);
		fclose(fic_ent);
		return(EF_ERR_READ_HEADER);
		}

	/* recording start date. */
	memset(main_header.startdate, 0, 256);
	if (ef_readcar(fic_ent, 255, main_header.startdate) == 0)
		{
		fprintf(stderr, "ERROR reading file %s (reading start date)\n", header_filename);
		fclose(fic_ent);
		return(EF_ERR_READ_HEADER);
		}

	/* recording start hour. */
	memset(main_header.starttime, 0, 256);
	if (ef_readcar(fic_ent, 255, main_header.starttime) == 0)
		{
		fprintf(stderr, "ERROR reading file %s (reading start time)\n", header_filename);
		fclose(fic_ent);
		return(EF_ERR_READ_HEADER);
		}

	/* Comment string generation. */
	/*----------------------------*/
	if (strlen(main_header.patient) < COMMENT_SIZE)
	{
		sprintf(elan_struct->comment, "%s", main_header.patient);
		if (strlen(main_header.identificateur) < (COMMENT_SIZE-strlen(elan_struct->comment)-3))
		{
			strcat(elan_struct->comment, " / ");
			strcat(elan_struct->comment, main_header.identificateur);
			if (strlen(main_header.startdate) < (COMMENT_SIZE-strlen(elan_struct->comment)-3))
			{
				strcat(elan_struct->comment, " / ");
				strcat(elan_struct->comment, main_header.startdate);
				if (strlen(main_header.starttime) < (COMMENT_SIZE-strlen(elan_struct->comment)-3))
				{
					strcat(elan_struct->comment, " / ");
					strcat(elan_struct->comment, main_header.starttime);
				}
				else
				{
					strcat(elan_struct->comment, " / ");
					strncat(elan_struct->comment, main_header.starttime, (COMMENT_SIZE-strlen(elan_struct->comment)-3));
					elan_struct->comment[COMMENT_SIZE-1] = '\0';
				}
			}
			else
			{
				strcat(elan_struct->comment, " / ");
				strncat(elan_struct->comment, main_header.startdate, (COMMENT_SIZE-strlen(elan_struct->comment)-3));
				elan_struct->comment[COMMENT_SIZE-1] = '\0';
			}
		}
		else
		{
			strcat(elan_struct->comment, " / ");
			strncat(elan_struct->comment, main_header.identificateur, (COMMENT_SIZE-strlen(elan_struct->comment)-3));
			elan_struct->comment[COMMENT_SIZE-1] = '\0';
		}
		sprintf(elan_struct->comment, "%s / %s / %s / %s", main_header.patient, main_header.identificateur, main_header.startdate, main_header.starttime);
	}
	else
	{
		strncpy(elan_struct->comment, main_header.patient, COMMENT_SIZE-1);
		elan_struct->comment[COMMENT_SIZE-1] = '\0';
	}
	

	/* header size in bytes (or -1). */
	memset(main_header.taille_entete, 0, 256);
	if (ef_readcar(fic_ent, 255, main_header.taille_entete) == 0)
	{
		fprintf(stderr, "ERROR reading file %s (reading header size)\n", header_filename);
		fclose(fic_ent);
		return(EF_ERR_READ_HEADER);
	}

	/* reserved. */
	memset(main_header.reserved, 0, 256);
	if (ef_readcar(fic_ent, 255, main_header.reserved) == 0)
	{
		fprintf(stderr, "ERROR reading file %s (reading reserved)\n", header_filename);
		fclose(fic_ent);
		return(EF_ERR_READ_HEADER);
	}

	/* number of samples (or -1). */
	memset(main_header.nb_record, 0, 256);
	if (ef_readcar(fic_ent, 255, main_header.nb_record) == 0)
	{
		fprintf(stderr, "ERROR reading file %s (reading number of record)\n", header_filename);
		fclose(fic_ent);
		return(EF_ERR_READ_HEADER);
	}

	/* sampling period (in s).  */
	memset(main_header.duree_record, 0, 256);
	if (ef_readcar(fic_ent, 255, main_header.duree_record) == 0)
	{
		fprintf(stderr, "ERROR reading file %s (reading sampling period)\n", header_filename);
		fclose(fic_ent);
		return(EF_ERR_READ_HEADER);
	}
		
	/* Sampling frequency. */
	/*---------------------*/
	elan_struct->eeg.sampling_freq = ceil(1.0/strtod(main_header.duree_record, NULL));

	/* total number of channels  */
	memset(main_header.nb_signaux, 0, 256);
	if (ef_readcar(fic_ent, 255, main_header.nb_signaux) == 0)
	{
		fprintf(stderr, "ERROR reading file %s (reading number of channels)\n", header_filename);
		fclose(fic_ent);
		return(EF_ERR_READ_HEADER);
	}

	/* Number of channels. */
	/*---------------------*/
	elan_struct->chan_nb = strtol(main_header.nb_signaux, NULL, 10)-2;
	ef_alloc_channel_list(elan_struct);
	
	/* Memory allocation to store needed original informations. */
	/*----------------------------------------------------------*/
	eeg_maxphysic = (double *)calloc(elan_struct->chan_nb, sizeof(double));
	ELAN_CHECK_ERROR_ALLOC(eeg_maxphysic, "to convert ADC values (max physic).");
	eeg_minphysic = (double *)calloc(elan_struct->chan_nb, sizeof(double));
	ELAN_CHECK_ERROR_ALLOC(eeg_minphysic, "to convert ADC values (min physic)");
	eeg_maxdigit = (double *)calloc(elan_struct->chan_nb, sizeof(double));
	ELAN_CHECK_ERROR_ALLOC(eeg_maxdigit, "to convert ADC values (max digit)");
	eeg_mindigit = (double *)calloc(elan_struct->chan_nb, sizeof(double));
	ELAN_CHECK_ERROR_ALLOC(eeg_mindigit, "to convert ADC values (min digit)");

	/* for channel headers. */
	/*----------------------------------------*/
	ns = atoi(main_header.nb_signaux);
	channel_header = (channel_header_t *)calloc(ns, sizeof(channel_header_t));
	ELAN_CHECK_ERROR_ALLOC(channel_header, "for channel header while reading original EEG format.");
	
	/* Read electrode coordinates database file. */
	elec_dat_nb = 0;
	elec_dat_list = NULL;
	ef_orig_read_elecdat(&elec_dat_nb, &elec_dat_list);

	/* reading channel headers */
	for (j=0; j<ns; j++)
	{
		/* label */
		if (ef_readcar(fic_ent, 255, channel_header[j].label) == 0)
		{
			fprintf(stderr, "ERROR reading file %s (reading channel %d label)\n", header_filename, j+1);
			fclose(fic_ent);
			return(EF_ERR_READ_HEADER);
		}
		
		/* Parse label and number in elec.dat */
		if (j < elan_struct->chan_nb)
		{
			i_c = 0;
			while ((i_c<strlen(channel_header[j].label)) && (channel_header[j].label[i_c] != '.'))
			{
				i_c++;
			}
			if (channel_header[j].label[i_c] == '.')
			{
				/* . found. */
				strncpy(elan_struct->chan_list[j].lab, channel_header[j].label, i_c);
				elan_struct->chan_list[j].lab[i_c] = '\0';
				elan_struct->chan_list[j].coord_nb = 0;
				elan_struct->chan_list[j].coord_list = NULL;
				if (strlen(channel_header[j].label) > i_c+1)
				{
					/* number found. */
					elec_num = strtol(&channel_header[j].label[i_c+1], NULL, 10);
					
					if ((elec_num > 0) && (elec_num < elec_dat_nb))
					{
						/* Spherical coordinates found. */
						elan_struct->chan_list[j].coord_nb = 1;
						elan_struct->chan_list[j].coord_list = (coord_t *)calloc(elan_struct->chan_list[j].coord_nb, sizeof(coord_t));
						ELAN_CHECK_ERROR_ALLOC(elan_struct->chan_list[j].coord_list, "for storing channel #%d coordinates.", j+1);
						
						strcpy(elan_struct->chan_list[j].coord_list[0].lab, CoordSystemLabelList[CoordSystemSphericalHead]);
						elan_struct->chan_list[j].coord_list[0].val_nb = 3;
						elan_struct->chan_list[j].coord_list[0].val_list = (float *)calloc(elan_struct->chan_list[j].coord_list[0].val_nb, sizeof(float));
						ELAN_CHECK_ERROR_ALLOC(elan_struct->chan_list[j].coord_list[0].val_list, "for storing channel #%d coordinates (%d values).", j+1, elan_struct->chan_list[j].coord_list[0].val_nb);
						elan_struct->chan_list[j].coord_list[0].val_list[0] = 90;
							elan_struct->chan_list[j].coord_list[0].val_list[1] = elec_dat_list[elec_num-1].theta;
						elan_struct->chan_list[j].coord_list[0].val_list[2] = elec_dat_list[elec_num-1].phi;
					}
					else
					{
						/* number not found. */
						elan_struct->chan_list[j].coord_nb = 0;
						elan_struct->chan_list[j].coord_list = NULL;
						
						/* Search for same label? */
					}
				}
				else
				{
					/* number not found. */
					elan_struct->chan_list[j].coord_nb = 0;
					elan_struct->chan_list[j].coord_list = NULL;
					
					/* Search for same label? */
				}
			}
			else
			{
				/* No . found. */
				strcpy(elan_struct->chan_list[j].lab, channel_header[j].label);
				elan_struct->chan_list[j].coord_nb = 0;
				elan_struct->chan_list[j].coord_list = NULL;
				
				/* Search for same label? */
			}
		}
	}
	for (j=0; j<ns; j++)
	{
		/* capteur */
		if (ef_readcar(fic_ent, 255, channel_header[j].capteur) == 0)
		{
			fprintf(stderr, "ERROR reading file %s (reading channel %d sensor type)\n", header_filename, j+1);
			fclose(fic_ent);
			return(EF_ERR_READ_HEADER);
		}
		if (j < elan_struct->chan_nb)
		{
			strcpy(elan_struct->chan_list[j].type, channel_header[j].capteur);
		}
	}
	for (j=0; j<ns; j++)
	{
		/* unite */
		if (ef_readcar(fic_ent, 255, channel_header[j].unite) == 0)
		{
			fprintf(stderr, "ERROR reading file %s (reading channel %d unit)\n", header_filename, j+1);
			fclose(fic_ent);
			return(EF_ERR_READ_HEADER);
		}
		if (j < elan_struct->chan_nb)
		{
			strcpy(elan_struct->chan_list[j].unit, channel_header[j].unite);
		}
	}
	for (j=0; j<ns; j++)
	{
		/* min_physic */
		if (ef_readcar(fic_ent, 255, channel_header[j].min_physic) == 0)
		{
			fprintf(stderr, "ERROR reading file %s (reading channel %d physical min)\n", header_filename, j+1);
			fclose(fic_ent);
			return(EF_ERR_READ_HEADER);
		}
		if (j < elan_struct->chan_nb)
		{
			eeg_minphysic[j] = strtod(channel_header[j].min_physic, NULL);
		}
	}
	for (j=0; j<ns; j++)
	{
		/* max_physic */
		if (ef_readcar(fic_ent, 255, channel_header[j].max_physic) == 0)
		{
			fprintf(stderr, "ERROR reading file %s (reading channel %d physical max)\n", header_filename, j+1);
			fclose(fic_ent);
			return(EF_ERR_READ_HEADER);
		}
		if (j < elan_struct->chan_nb)
		{
			eeg_maxphysic[j] = strtod(channel_header[j].max_physic, NULL);
		}
	}
	for (j=0; j<ns; j++)
	{
		/* min_digit */
		if (ef_readcar(fic_ent, 255, channel_header[j].min_digit) == 0)
		{
			fprintf(stderr, "ERROR reading file %s (reading channel %d digital min)\n", header_filename, j+1);
			fclose(fic_ent);
			return(EF_ERR_READ_HEADER);
		}
		if (j < elan_struct->chan_nb)
		{
			eeg_mindigit[j] = strtod(channel_header[j].min_digit, NULL);
		}
	}
	for (j=0; j<ns; j++)
	{
		/* max_digit */
		if (ef_readcar(fic_ent, 255, channel_header[j].max_digit) == 0)
		{
			fprintf(stderr, "ERROR reading file %s (reading channel %d digital max)\n", header_filename, j+1);
			fclose(fic_ent);
			return(EF_ERR_READ_HEADER);
		}
		if (j < elan_struct->chan_nb)
		{
			eeg_maxdigit[j] = strtod(channel_header[j].max_digit, NULL);
		}
	}
	
	/* ADC conversion values. */
	elan_struct->orig_info->eeg_info.eeg_convADC = (double *)calloc(elan_struct->chan_nb, sizeof(double));
	ELAN_CHECK_ERROR_ALLOC(elan_struct->orig_info->eeg_info.eeg_convADC, "to convert ADC values (coefficient).");

	elan_struct->orig_info->eeg_info.eeg_offsetADC = (double *)calloc(elan_struct->chan_nb, sizeof(double));
	ELAN_CHECK_ERROR_ALLOC(elan_struct->orig_info->eeg_info.eeg_offsetADC, "to convert ADC values (offset).");

	for (j=0; j<elan_struct->chan_nb; j++)
	{
		elan_struct->orig_info->eeg_info.eeg_convADC[j] = (eeg_maxphysic[j]-eeg_minphysic[j])/(eeg_maxdigit[j]-eeg_mindigit[j]);
		elan_struct->orig_info->eeg_info.eeg_offsetADC[j] = ((eeg_minphysic[j]*eeg_maxdigit[j])-(eeg_maxphysic[j]*eeg_mindigit[j]))/(eeg_maxdigit[j]-eeg_mindigit[j]);
	}
	if (elan_struct->orig_info->eeg_info.orig_datatype == ORIG_EEG_DATATYPE_12BITS)
	{
		/* Special adjustment for 12 bits data. */
		for (j=0; j<elan_struct->chan_nb; j++)
		{
			elan_struct->orig_info->eeg_info.eeg_convADC[j] /= 16.0;
		}
	}

	for (j=0; j<ns; j++)
		{
		/* filtre */
		if (ef_readcar(fic_ent, 255, channel_header[j].filtre) == 0)
			{
			fprintf(stderr, "ERROR reading file %s (reading channel %d filter)\n", header_filename, j+1);
			fclose(fic_ent);
			return(EF_ERR_READ_HEADER);
			}
		}
	for (j=0; j<ns; j++)
		{
		/* nbech */
		if (ef_readcar(fic_ent, 255, channel_header[j].nbech) == 0)
			{
			fprintf(stderr, "ERROR reading file %s (reading channel %d number of samples)\n", header_filename, j+1);
			fclose(fic_ent);
			return(EF_ERR_READ_HEADER);
			}
		}
	for (j=0; j<ns; j++)
		{
		/* reserved */
		if (ef_readcar(fic_ent, 255, channel_header[j].reserved) == 0)
			{
			fprintf(stderr, "ERROR reading file %s (reading channel %d reserved)\n", header_filename, j+1);
			fclose(fic_ent);
			return(EF_ERR_READ_HEADER);
			}
		}

	/* fin de lecture des entetes */
	fclose(fic_ent);

	free(channel_header);
	free(eeg_maxphysic);
	free(eeg_minphysic);
	free(eeg_maxdigit);
	free(eeg_mindigit);
	ef_orig_free_elecdat(&elec_dat_list);
	
	elan_struct->eeg.flag_cont_epoch = EF_EEG_CONTINUOUS;
	
	/* Read event from data file. */
	/*----------------------------*/
	switch (elan_struct->orig_info->eeg_info.orig_datatype) {
		case ORIG_EEG_DATATYPE_32BITS:
			version_eeg = 3;
			sample_size = 4;
			break;
		
		case ORIG_EEG_DATATYPE_16BITS:
			version_eeg = 2;
			sample_size = 2;
			break;
			
		case ORIG_EEG_DATATYPE_12BITS:
			version_eeg = 1;
			sample_size = 2;
			break;
			
		default:
			fprintf(stderr, "Undefined EEG data type %d.\n", elan_struct->orig_info->eeg_info.orig_datatype);
			sample_size = 1;
			version_eeg = -1;
		}

	/*----------------------- recherche des evenements ---------------------------------------------------------*/
	data = fopen(data_filename, "rb");
	if (data == NULL)
	{
		fprintf(stderr, "ERROR: can't open file %s (data reading).\n", data_filename);
		return(EF_ERR_OPEN_READ);
	}
	nb_eve = 0;
	pos_err = ef_look_eve_file_backward(data, elan_struct->chan_nb, &nb_eve, &tab_pos, &tab_eve, &tab_info, version_eeg, sample_size);

	if (pos_err < 0 ) { fprintf(stderr, "ERROR : %d in event search routine\n", pos_err); }
//	else  { fprintf(stdout, "Number of detected events and acquisition breaks in file %s : %d\n", data_filename, nb_eve); }

	/*--------------------- retournement du tableau de stockage des evenements ---------------------------------*/
	elan_struct->eeg.event_nb = nb_eve;
	ef_alloc_eeg_array_events(elan_struct);
	j=0;
	int i;
	for (i=nb_eve-1; i>=0; i--, j++) {
		elan_struct->eeg.event_code_list[j] = tab_eve[i];
		elan_struct->eeg.event_sample_list[j] = tab_pos[i]-1; // Because first sample is 1, must be 0
		}
	
	/* Get total number of samples. */
	fseek(data, 0, SEEK_END);
	elan_struct->eeg.samp_nb = ftell(data);
	fclose(data);

	elan_struct->eeg.samp_nb = elan_struct->eeg.samp_nb/(elan_struct->chan_nb+2);
	elan_struct->eeg.samp_nb /= sample_size;

	/* Desallocation memoire. */
	free(tab_pos);
	free(tab_eve);
	free(tab_info);

	return(EF_NOERR);
}


/*------------------------------------------------------------------------------*/
/* ef_readcar : Fonction de lecture d'une chaine. La lecture s'arrete au nombre */
/*              de caracteres demandes ou  a la fin de la ligne (toute la ligne */
/*              est lue quand meme).                                            */
/*------------------------------------------------------------------------------*/
int ef_readcar(FILE *f, int nb_car, char *str)
{
int n;
char c;

n=0;

/* \n for Unix, \r for DOS/Win */
while ((fscanf(f, "%c", &c) == 1) && (c != '\n') && (c != '\r'))
  {
	if (n<nb_car) str[n]=c;
	n++;
	}
if ((n<nb_car) && ((str[n] == '\n') || (str[n] == '\r'))) str[n]='\0';
else str[nb_car]='\0';

return(n);
}


/****************************************************************************************************************/
/* ef_orig_read_EEG_data_sampblock_one_chan: read an EEG data block for one channel and store in data_ptr.      */
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
int ef_orig_read_EEG_data_sampblock_one_chan(char *filename, elan_struct_t *elan_struct, int chanNum, int sampStart, int sampNb, void **data_ptr)
{
	int sampSize=0, sampRead, i_samp, off_buf, eegSampStart;
	float *flt_ptr=NULL;
	double *dbl_ptr=NULL;
	int16_t *buf16=NULL;
	int32_t *buf32=NULL;
	FILE *data_f;
	
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

		if ((sampNb*(elan_struct->chan_nb+2)*sampSize) != elan_struct->orig_info->eeg_info.bufReadSize)
		{
			elan_struct->orig_info->eeg_info.bufReadSize = sampNb*(elan_struct->chan_nb+2)*sampSize;
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
		
		eegSampStart = sampStart*((elan_struct->chan_nb+2)*sampSize);
		fseek(data_f, eegSampStart, SEEK_SET);
		
		/* Read a block of data. */
		sampRead = fread(elan_struct->orig_info->eeg_info.bufReadPtr, sampSize, sampNb*(elan_struct->chan_nb+2), data_f);
			
		/* Endian and physical value conversions. */
		sampRead /= elan_struct->chan_nb+2;
		switch (elan_struct->orig_info->eeg_info.orig_datatype)
		{
			case (ORIG_EEG_DATATYPE_32BITS):
				switch (elan_struct->eeg.data_type)
				{
					case EF_DATA_FLOAT:
						for (i_samp=0; i_samp<sampRead; i_samp++)
						{
							off_buf = (i_samp*(elan_struct->chan_nb+2))+chanNum;
							_swab(&buf32[off_buf], &buf32[off_buf], sizeof(int32));
							flt_ptr[i_samp]=((float)buf32[off_buf]*elan_struct->orig_info->eeg_info.eeg_convADC[chanNum])+elan_struct->orig_info->eeg_info.eeg_offsetADC[chanNum];
						}
						break;
						case EF_DATA_DOUBLE:
						for (i_samp=0; i_samp<sampRead; i_samp++)
						{
							off_buf = (i_samp*(elan_struct->chan_nb+2))+chanNum;
							_swab(&buf32[off_buf], &buf32[off_buf], sizeof(int32));
							dbl_ptr[i_samp]=((double)buf32[off_buf]*elan_struct->orig_info->eeg_info.eeg_convADC[chanNum])+elan_struct->orig_info->eeg_info.eeg_offsetADC[chanNum];
						}
						break;
				}
				break;
				
			case (ORIG_EEG_DATATYPE_12BITS):
			case (ORIG_EEG_DATATYPE_16BITS):
				switch (elan_struct->eeg.data_type)
				{
					case EF_DATA_FLOAT:
						for (i_samp=0; i_samp<sampRead; i_samp++)
						{
							off_buf = (i_samp*(elan_struct->chan_nb+2))+chanNum;
							_swab(&buf16[off_buf], &buf16[off_buf], sizeof(int16));
							flt_ptr[i_samp]=((float)buf16[off_buf]*elan_struct->orig_info->eeg_info.eeg_convADC[chanNum])+elan_struct->orig_info->eeg_info.eeg_offsetADC[chanNum];
						}
						break;
						case EF_DATA_DOUBLE:
						for (i_samp=0; i_samp<sampRead; i_samp++)
						{
							off_buf = (i_samp*(elan_struct->chan_nb+2))+chanNum;
							_swab(&buf16[off_buf], &buf16[off_buf], sizeof(int16));
							dbl_ptr[i_samp]=((double)buf16[off_buf]*elan_struct->orig_info->eeg_info.eeg_convADC[chanNum])+elan_struct->orig_info->eeg_info.eeg_offsetADC[chanNum];
						}
						break;
				}
				break;
		}
		
		fclose(data_f);
		return(sampRead);
	}
	else
	{
		return(0);
	}
}


/*****************************************************************************************************************/
/* ef_orig_read_EEG_data_sampblock_all_chan: read an EEG data block for all channel and store in data_ptr.       */
/* char *filename: file to read from.                                                                            */
/* elan_struct_t *elan_struct: Elan structure (read from header).                                                */
/* int measureNum: indice of measure to read (starting from 0).                                                  */
/* int sampStart: indice of first sample to read (starting from 0).                                              */
/* int sampNb: number of samples to read.                                                                        */
/* void ***data_ptr: array of samples to store data. If NULL, data_ptr is allocated in function (must be freed). */
/*                 If allocated in caller, size must be [elan_struct.chan_nbxsampNb] float or double.            */
/* return value: number of samples actually read.                                                                */
/*****************************************************************************************************************/
int ef_orig_read_EEG_data_sampblock_all_chan(char *filename, elan_struct_t *elan_struct, int sampStart, int sampNb, void **data_ptr)
{
	int sampSize=0, sampRead, i_chan, i_samp, off_buf, eegSampStart;
	float **flt_ptr=NULL;
	double **dbl_ptr=NULL;
	int16_t *buf16=NULL;
	int32_t *buf32=NULL;
	FILE *data_f;
	
	if (elan_struct->has_eeg == EF_YES)
	{
		if (*data_ptr == NULL)
		{
			switch (elan_struct->eeg.data_type)
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

		if ((sampNb*(elan_struct->chan_nb+2)*sampSize) != elan_struct->orig_info->eeg_info.bufReadSize)
		{
			elan_struct->orig_info->eeg_info.bufReadSize = sampNb*(elan_struct->chan_nb+2)*sampSize;
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
		
		eegSampStart = sampStart*((elan_struct->chan_nb+2)*sampSize);
		fseek(data_f, eegSampStart, SEEK_SET);
		
		/* Read a block of data. */
		sampRead = fread(elan_struct->orig_info->eeg_info.bufReadPtr, sampSize, sampNb*(elan_struct->chan_nb+2), data_f);
			
		/* Endian and physical value conversions. */
		sampRead /= elan_struct->chan_nb+2;
		switch (elan_struct->orig_info->eeg_info.orig_datatype)
		{
			case (ORIG_EEG_DATATYPE_32BITS):
				switch (elan_struct->eeg.data_type)
				{
					case EF_DATA_FLOAT:
						for (i_samp=0; i_samp<sampRead; i_samp++)
						{
							for (i_chan=0; i_chan<elan_struct->chan_nb; i_chan++)
							{
								off_buf = (i_samp*(elan_struct->chan_nb+2))+i_chan;
								_swab(&buf32[off_buf], &buf32[off_buf], sizeof(int32));
								flt_ptr[i_chan][i_samp]=((float)buf32[off_buf]*elan_struct->orig_info->eeg_info.eeg_convADC[i_chan])+elan_struct->orig_info->eeg_info.eeg_offsetADC[i_chan];
							}
						}
						break;
						case EF_DATA_DOUBLE:
						for (i_samp=0; i_samp<sampRead; i_samp++)
						{
							for (i_chan=0; i_chan<elan_struct->chan_nb; i_chan++)
							{
								off_buf = (i_samp*(elan_struct->chan_nb+2))+i_chan;
								_swab(&buf32[off_buf], &buf32[off_buf], sizeof(int32));
								dbl_ptr[i_chan][i_samp]=((double)buf32[off_buf]*elan_struct->orig_info->eeg_info.eeg_convADC[i_chan])+elan_struct->orig_info->eeg_info.eeg_offsetADC[i_chan];
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
						for (i_samp=0; i_samp<sampRead; i_samp++)
						{
							for (i_chan=0; i_chan<elan_struct->chan_nb; i_chan++)
							{
								off_buf = (i_samp*(elan_struct->chan_nb+2))+i_chan;
								_swab(&buf16[off_buf], &buf16[off_buf], sizeof(int16));
								flt_ptr[i_chan][i_samp]=((float)buf16[off_buf]*elan_struct->orig_info->eeg_info.eeg_convADC[i_chan])+elan_struct->orig_info->eeg_info.eeg_offsetADC[i_chan];
							}
						}
						break;
						case EF_DATA_DOUBLE:
						for (i_samp=0; i_samp<sampRead; i_samp++)
						{
							for (i_chan=0; i_chan<elan_struct->chan_nb; i_chan++)
							{
								off_buf = (i_samp*(elan_struct->chan_nb+2))+i_chan;
								_swab(&buf16[off_buf], &buf16[off_buf], sizeof(int16));
								dbl_ptr[i_chan][i_samp]=((double)buf16[off_buf]*elan_struct->orig_info->eeg_info.eeg_convADC[i_chan])+elan_struct->orig_info->eeg_info.eeg_offsetADC[i_chan];
							}
						}
						break;
				}
				break;
		}
		
		fclose(data_f);
		return(sampRead);
	}
	else
	{
		return(0);
	}
}




/*----------------------------------------------------------------------------------------------------------*/
/* look_eve_file_backward      sous programme de recherche des evenements dans fichier de type CELI         */
/*                                                                                                         */
/* appel: cr = look_eve_file_backward(d_file,l_nvoi_a,r_file)                                               */
/*                                                                                                          */
/*                                                                                                          */
/*        la fonction retourne le nombre d'evenements detectes                                              */
/*                                                                                                          */
/* definition des arguments:                                                                                */
/*                                                                                                          */
/*        d_file  :                                                                                         */
/*                  d_file est le descripteur du fichier de type CELI ou se trouve le resultat des          */
/*                  conversions.                                                                            */
/*                  la matrice issue d'une serie de conversions sur la chaine CELI se presente de la facon  */
/*                  suivante:                                                                               */
/*                            soit nech le numero de la salve et nvoi le numero de la voie echantillonnee;  */
/*                                                                                                          */
/*                  !nvoi=0  !nvoi=1  !nvoi=2  !nvoi=3  !nvoi=4  !                                          */
/*          ________!________!________!________!________!________!                                          */
/*          nech=0  ! y[0][0]! y[0][1]! y[0][2]! y[0][3]! y[0][4]!                                          */
/*          ________!________!________!________!________!________!                                          */
/*          nech=1  ! y[1][0]! y[1][1]! y[1][2]! y[1][3]! y[1][4]!                                          */
/*          ________!________!________!________!________!________!                                          */
/*          nech=2  ! y[2][0]! y[2][1]! y[2][2]! y[2][3]! y[2][4]!                                          */
/*          ________!________!________!________!________!________!                                          */
/*          nech=3  ! y[3][0]! y[3][1]! y[3][2]! y[3][3]! y[3][4]!                                          */
/*          ________!________!________!________!________!________!                                          */
/*          nech=4  ! y[4][0]! y[4][1]! y[4][2]! y[4][3]! y[4][4]!                                          */
/*          ________!________!________!________!________!________!                                          */
/*          nech=5  ! y[5][0]! y[5][1]! y[5][2]! y[5][3]! y[5][4]!                                          */
/*          ________!________!________!________!________!________!                                          */
/*          nech=6  ! y[6][0]! y[6][1]! y[6][2]! y[6][3]! y[6][4]!                                          */
/*          ________!________!________!________!________!________!                                          */
/*                                                                                                          */
/*                  dans ce tableau les voies 3 et 4 sont des voies numeriques. Dans la voie 3 on trouve la */
/*                  valeur du compteur inter-evenements et dans la voie 4 l'octet de poids faible  donne le */
/*                  type d'evenement, celui de poids fort les huits bits d'info.                            */
/*                                                                                                          */
/*                                                                                                          */
/*        l_nvoi_a    :                                                                                     */
/*                  nombre de voies analogiques. Par exemple 3                                              */
/*                                                                         */
/*        r_file   :                                                        */
/*                  fichier  donnant la position des evenements en nombre d'echantillons par rapport au     */
/*                  debut du fichier de type CELI ainsi que le code de l'evenement detecte                  */
/*                  (1long + 1short par evenement)                                                          */
/*                                                                                                          */
/* type des variables:                                                                                      */
/*                                                                                                          */
/*        d_file      : FILE *                                                                              */
/*        l_nvoi_a    : long                                                                                */
/*        r_file      : FILE *                                                                              */
/* type de la fonction:                                                                                     */
/*        look_eve_file_backward    : long                                                                  */
/*                                                                                                          */
/*----------------------------------------------------------------------------------------------------------*/
int ef_look_eve_file_backward(FILE *d_file, int nva, int *nb_eve, int **tab_pos, int **tab_eve, int **tab_info, int version_eeg, int sample_size)
{
unsigned short val;
unsigned short eve;
int val32, val32_1;
int eve32;
int delta_ech_octet;
long long int offset, position;
int *local_tab_pos=NULL, *local_tab_eve=NULL, *local_tab_info=NULL;
int return_value=0, local_nb_eve;

local_nb_eve = 0;
delta_ech_octet = ((nva + 2) * sample_size);

/*---------------- lecture de la derniere valeur du compteur dans le fichier -------------------------------*/
offset = -((long long)2*(long long)sample_size);
if (fseek(d_file, offset, SEEK_END) != 0)
  {
  return_value=-1;
  goto lab_return;
  }
/*fprintf(stdout, "%d %lld %lld\n", err, offset, (long long int)ftell(d_file));*/

switch (version_eeg)
  {
  case 3:
    if (freadswab(&val32, sample_size, 1, d_file) != 1)
      {
      return_value=-2;
      goto lab_return;
      }
    break;

  case 1:
  case 2:
  default:
    if (freadswab(&val, sample_size, 1, d_file) != 1)
      {
      return_value=-2;
      goto lab_return;
      }
    val32 = (int)val;
    break;
  }

next:
if (val32 != 0)   /* on lit la position du precedent passage a 0 du compteur inter-evenement (en nombre      */
                  /* d'echantillon), ainsi qu'une nouvelle valeur de val                                      */
  {
  offset = -(long long)val32*(long long)delta_ech_octet - (long long)sample_size;
  if (fseek(d_file, offset, SEEK_CUR) != 0)
    {
    return_value=local_nb_eve;
    goto lab_return;
    }
  switch (version_eeg)
    {
    case 3:
      if (freadswab(&val32, sample_size, 1, d_file) != 1)
        {
        return_value=-2;
        goto lab_return;
        }
      break;

    case 1:
    case 2:
    default:
      if (freadswab(&val, sample_size, 1, d_file) != 1)
        {
        return_value=-2;
        goto lab_return;
        }
      val32 = (int)val;
      break;
    }

  if (val32 != 0)
    {
    /* detection d'une rupture d'acquisition; recherche du dernier echantillon avant la rupture           */
    /* d'acquisition                                                                                      */
    val32_1 = val32;
    ech_suivant:;
    offset = (long long)delta_ech_octet - (long long)sample_size;
    fseek(d_file, offset, SEEK_CUR);
    switch (version_eeg)
      {
      case 3:
        if (freadswab(&val32, sample_size, 1, d_file) != 1)
          {
          return_value=-2;
          goto lab_return;
          }
        break;

      case 1:
      case 2:
      default:
        if (freadswab(&val, sample_size, 1, d_file) != 1)
          {
          return_value=-2;
          goto lab_return;
          }
        val32 = (int)val;
        break;
      }
    if (val32 == 0)
      {
      /* on a trouve un evenement en marche avant */
      val32_1 = 0;
      goto ech_suivant;
      }
    if (val32 == (val32_1 + 1))
      {
      /* on est toujours dans le meme bloc d'acquisition */
      val32_1 = val32;
      goto ech_suivant;
      }
    else
      {
      /* on est dans le bloc suivant; on revient a l'echantillon precedent et on lit la valeur du        */
      /* compteur inter evenement juste avant la rupture d'acquisition                                   */
      offset = -(long long)delta_ech_octet - (long long)sample_size;
      fseek(d_file, offset, SEEK_CUR);
      switch (version_eeg)
        {
        case 3:
          if (freadswab(&val32, sample_size, 1, d_file) != 1)
            {
            return_value=-2;
            goto lab_return;
            }
          break;

        case 1:
        case 2:
        default:
          if (freadswab(&val, sample_size, 1, d_file) != 1)
            {
            return_value=-2;
            goto lab_return;
            }
          val32 = (int)val;
          break;
        }
      /* indication exacte de la rupture d'acquisition et marque de fin de bloc */
      printf("Acquisition break after sample # %lld\n",(ftell(d_file)+(long long)sample_size)/(long long)delta_ech_octet);
      /*  printf("numero de l'octet ( a partir de 1), correspondant au changement de bloc %ld\n",(ftell(d_file)+2L));  */
      local_nb_eve++;
      local_tab_pos = (int *)realloc(local_tab_pos, local_nb_eve*sizeof(int));
      if (local_tab_pos == NULL)
        {
        fprintf(stderr, "ERROR : memory allocation for event number %d.\n", local_nb_eve);
        return_value=-4;
        goto lab_return;
        }
      local_tab_eve = (int *)realloc(local_tab_eve, local_nb_eve*sizeof(int));
      if (local_tab_eve == NULL)
        {
        fprintf(stderr, "ERROR : memory allocation for event number %d.\n", local_nb_eve);
        return_value=-4;
        goto lab_return;
        }
      position = (((long long)ftell(d_file)+(long long)sample_size)/(long long)delta_ech_octet);
      local_tab_pos[local_nb_eve-1] = (int)position;
      local_tab_eve[local_nb_eve-1] = 255;
      switch ( VERSION_POS )
        {
        case 1:
          break;
        case 2:
          local_tab_info = (int *)realloc(local_tab_info, local_nb_eve*sizeof(int));
          if (local_tab_info == NULL)
            {
            fprintf(stderr, "ERROR : memory allocation for event number %d.\n", local_nb_eve);
            return_value=-4;
            goto lab_return;
            }
          local_tab_info[local_nb_eve-1] = 0;
          break;
        }
      /* poursuite de la recherche en marche arriere */
      }
    }
  goto next;
  }

if (val32 == 0)  /* le comteur est a zero; on a trouve un evenement ou un overflow du compteur               */
  {
  switch (version_eeg)
    {
    case 3:
      if (freadswab(&eve32, sample_size, 1, d_file) != 1)
        {
        return_value=-2;
        goto lab_return;
        }
      break;

    case 1:
    case 2:
    default:
      if (freadswab(&eve, sample_size, 1, d_file) != 1)
        {
        return_value=-2;
        goto lab_return;
        }
      eve32 = (int)eve;
      break;
    }
  if ( (eve32 & 0xffffffff) != 0)
    {
    local_nb_eve++;
    local_tab_pos = (int *)realloc(local_tab_pos, local_nb_eve*sizeof(int));
    if (local_tab_pos == NULL)
      {
      fprintf(stderr, "ERROR : memory allocation for event number %d.\n", local_nb_eve);
      return_value=-4;
      goto lab_return;
      }
    local_tab_eve = (int *)realloc(local_tab_eve, local_nb_eve*sizeof(int));
    if (local_tab_eve == NULL)
      {
      fprintf(stderr, "ERROR : memory allocation for event number %d.\n", local_nb_eve);
      return_value=-4;
      goto lab_return;
      }
    switch ( VERSION_POS )
      {
      case 1:
        position = ((long long)ftell(d_file)/(long long)delta_ech_octet);
        local_tab_pos[local_nb_eve-1] = (int)position;
        local_tab_eve[local_nb_eve-1] = eve32;
        break;
      case 2:
				position = (((long long)ftell(d_file)+(long long)sample_size)/(long long)delta_ech_octet);
        local_tab_pos[local_nb_eve-1] = (int)position;
        /* Les evenements vont de 0 a 65535. */
        /* Plus de byte info. */
        local_tab_eve[local_nb_eve-1] = eve32;
        local_tab_info = (int *)realloc(local_tab_info, local_nb_eve*sizeof(int));
        if (local_tab_info == NULL)
          {
          fprintf(stderr, "ERROR : memory allocation for event number %d.\n", local_nb_eve);
          return_value=-4;
          goto lab_return;
          }
        local_tab_info[local_nb_eve-1] = 0;
        break;
      }

    }
  offset = (long long)(-delta_ech_octet)-(long long)((long long)2*(long long)sample_size);
  if ( fseek(d_file, offset, SEEK_CUR) != 0)
    {
    return_value=local_nb_eve;
    goto lab_return;
    }
  else
    {
    switch (version_eeg)
      {
      case 3:
        if (freadswab(&val32, sample_size, 1, d_file) != 1)
          {
          return_value=-2;
          goto lab_return;
          }
        break;

      case 1:
      case 2:
      default:
        if (freadswab(&val, sample_size, 1, d_file) != 1)
          {
          return_value=-2;
          goto lab_return;
          }
        val32 = (int)val;
        break;
      }
    goto next;
    }
  }

return_value=-3;

lab_return:
*nb_eve = local_nb_eve;
*tab_pos = local_tab_pos;
*tab_eve = local_tab_eve;
*tab_info = local_tab_info;

return(return_value);

}

