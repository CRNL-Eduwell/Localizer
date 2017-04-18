/*--------------------------------------------------*/
/* Reading original TF ELAN files module.           */
/* v: 1.00    Aut.: PEA                             */
/* CRNL / INSERM U1028 / CNRS UMR 5292 / Univ-Lyon1 */
/*--------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "elanfile.h"
#include "ef_origcommon.h"

/*Commenté car utilisé uniquement pour 
    vieux tf dans case VER_TEXTE :*/
//#include "bibgen.h"

/* Taille de la chaine du nom des electrodes. */
#define TAILLENOMELEC 5
#define TAILLENOMELEC_EXT 256

/* Taille a allouer pour la chaine message des warnings. */
#define TAILLEMAX_WARNING 4096

/***************************************/
/*				       */
/* Definitions des numeros de version. */
/*				       */
/***************************************/
#define VER_BINAIRE_1 1
#define VER_TEXTE 0

/**************************************************/
/*						  */
/* Definitions pour les types de fichiers tfmoy.  */
/*						  */
/**************************************************/
#define INCONNU 0
#define TFMOY_ELEM 1         		/* avg.tf */
#define TFMOY_PE 2			/* ep.tf */
#define TFMOY_P 3			/* pl.tf */
#define TFMOY_REEL 4
#define TFMOY_IMAG 5
#define STAT 6				/* XX.stat.tf */
#define NIVEAU 7
#define TF_CAR 8
#define SYNC 9				/* devrait etre inutilise */
#define SYNC_P 10			/* ph.tf */

/*******************************************************/
/*						       */
/* Definitions pour les sous-types de fichiers tfmoy.  */
/*						       */
/*******************************************************/
/* Origine moyenne. */
/* type 1           */
/********************/
#define STANDARD 1
#define MOY_TFMOY 2
#define MEDIANE_TFMOY 3
#define DIFF_TFMOY 4
#define MOY_POND_TFMOY 5
#define SOMME_TFMOY 6
#define Z_TFMOY 7
#define P_TFMOY 8
#define SYNC_TFMOY 9


/********************/
/* Origine stats.   */
/* type 2           */
/********************/
#define PAS_DE_STAT 1
#define F_QUADE 2
#define T_CONOVER 3
#define Z_SIGNE 4
#define P_SIGNE 5
#define NPLUS_SIGNE 6
#define NMOINS_SIGNE 7
#define Z_WILCOX 8
#define ECART_TYPE 9
#define CONF_SUP 10
#define CONF_INF 11
#define SYNC_RAND 12
#define AMP_RAND 13
#define P_WILCOX 14

/*****************************************/
/*					 */
/* Definitions pour les types de signal. */
/*					 */
/*****************************************/
#define POTENTIEL 1
#define SCD 2

/****************************************/
/*					*/
/* Definitions types de donnees.	*/
/*					*/
/****************************************/
#define TYPE_FLOAT 1

/****************************************/
/*					*/
/* Definitions types de calcul.    	*/
/*					*/
/****************************************/
#define MORLET 1
#define GABOR 2

/********************************************************/
/*							*/
/* Definitions pour les flags (lb, lissage, moyennage).	*/
/*							*/
/********************************************************/
#define NONE 0
#define LB_MOYENNE 1
#define LB_MEDIANE 2
#define LB_MOYENNE_NORM 3
#define LB_MEDIANE_NORM 4
#define LB_RMS_NORM 5
#define LB_MAX_NORM 6
#define LB_MOYENNE_FIC 101
#define LB_MEDIANE_FIC 102
#define LB_MOYENNE_NORM_FIC 103
#define LB_MEDIANE_NORM_FIC 104
#define LB_RMS_NORM_FIC 105
#define LB_MAX_NORM_FIC 106
#define LB_SIGNE_MOY 201
#define LB_SIGNE_MED 202
#define LB_SIGNE_TOUS 203

#define LISSAGE 1
#define MOYENNAGE 1

/********************************************************/
/*							*/
/* Definitions de structure pour les noms d'electrodes. */
/*							*/
/********************************************************/
typedef char tfmoy_label_type[128];
typedef char tfmoy_nom_elec_type[TAILLENOMELEC_EXT];
typedef char tfmoy_nom_elec_fic_type[TAILLENOMELEC];

/******************************************/
/*					  */
/* Structure d'entete des fichiers tfmoy. */
/*					  */
/******************************************/
typedef struct
  {
  /************************************/
  /*		       	    	      */
  /* Nouvelle version v1.0 (binaire). */
  /*		       	    	      */
  /************************************/

  /***********************/
  /* 1er enregistrement. */
  /***********************/
  char no_version;
  short long_entete;
  int adr_donnees;
  short type_data;
  char libre[23];

  /************************/
  /* 2eme enregistrement. */
  /************************/
  short type_tfmoy;	/**************************************/
  short type_1_tfmoy; 	/* Mode d'obtention du fichier tfmoy. */
  short type_2_tfmoy;	/**************************************/
  short type_signal;	/* Type de signal utilise (potentiel, scd...). */
  short type_event;
  short *tab_numvoies;	 		/* Selon ELEC.DAT */
  tfmoy_nom_elec_type *tab_nomvoies; 	/* Selon ELEC.DAT */
  short type_calc;	/* Mode de calcul du tfmoy (ondelettes de Morlet, Gabor...). */
  float *tab_freq;		/* Liste des frequences. */
  float *tab_m_ondel;		/* Liste des m_ondel par frequence. */
  float taille_fen_ondel;
  short pas_sousech;
  int *tab_nb_eve;		/* Nb evnt moyennes par voie. */
    			/*****************************************/
  short flag_corr_lb;   /* Flag correction ligne de base         */
  			/* (0=non; 1=moyenne; 2=mediane; 3=tous).        */
    float deb_lb;       /* Debut et fin correction ligne de base */
    float fin_lb;       /* en ms si flag == 1 ou 2 . 	         */
    			/*****************************************/
  short flag_liss_tfmoy; 	/********************************/
    float demi_fen_temp;	/* Flag lissage (0=non; 1=oui). */
    float demi_fen_freq;	/* Parametres de lissage si	*/
    short pas_ss_ech_temp;	/* flag == 1.			*/
    short pas_ss_ech_freq;	/********************************/

  short flag_moy_elec;				    /* Flag de moyennage des    */
    short *tab_nb_elec_parvoie;			    /* electrodes : 0=non,1=oui */
    short **liste_no_elec_parvoie;		    /* Noms et no des elec      */
    tfmoy_nom_elec_type **liste_nom_elec_parvoie;   /* moyennees selon elec.dat */


  /***********************************/
  /*		       		     */
  /* Ancienne version v0.0 (texte).  */
  /*		       		     */
  /***********************************/

  int nb_voies; 	/* Compatible fichiers binaires. */
/*  tfmoy_label_type *voies;	 UNIQUEMENT ANCIENNE VERSION (TEXTE). */
	char **voies;
  int nb_ech;		/* Compatible fichiers binaires. */
  int nb_ech_pre;	/* Compatible fichiers binaires. */
  float freq_ech;	/* Compatible fichiers binaires. */
  int nb_freq;		/* Compatible fichiers binaires. */
  float f_deb, f_pas;		/* UNIQUEMENT ANCIENNE VERSION (TEXTE). */
  int nb_eve;			/* UNIQUEMENT ANCIENNE VERSION (TEXTE). */
  } tfmoy_header_type;

	
	
/* Local function prototypes. */
/*----------------------------*/
int ef_test_version_tfmoy(char *nom_fic);
int ef_free_hd_tfmoy(tfmoy_header_type tf_hd);


/* External function prototypes. */
/*-------------------------------*/
extern void ef_orig_read_elecdat(int *coord_nb, elec_dat_t **coord_list); /* Defined in ef_origtools.c */
extern void ef_orig_free_elecdat(elec_dat_t **coord_list); /* Defined in ef_origtools.c */
extern size_t freadswab(void *ptr, size_t size, size_t nitems, FILE *stream); /* Defined in ef_origtools.c */



/*-----------------------------------------------------------------------------------------------*/
/*												 */
/*     Lecture de l'entete des fichiers tfmoy     						 */
/*												 */
/* date : 31-10-96	     Ver: 2.0        P.E.A						 */
/*												 */
/*-----------------------------------------------------------------------------------------------*/
int ef_readtfheader(char *nomfic, elan_struct_t *elan_struct)
{
  int k, i, j, i_f, i_chan, elec_dat_nb;
  int test_label;
  int intlu, ok;
	char string[256];
  FILE *fic;
  char tmp[128];
	tfmoy_header_type tf_hd;
	elec_dat_t *elec_dat_list;

  /* Test du no de version. */
  tf_hd.no_version = ef_test_version_tfmoy(nomfic);
  if (tf_hd.no_version == -1)
    { fprintf(stderr, "Unknown version number\n"); return(0); }

  switch (tf_hd.no_version)
	{
    /****************************/
    /*				*/
    /* Fichiers tfmoy binaires. */
    /*				*/
    /****************************/
    case VER_BINAIRE_1 :
 	  /* Ouverture fichier. */
	  /*--------------------*/
	  fic = fopen(nomfic,"rb");
	  if (fic == NULL)
		{
			fprintf(stderr, "ERROR: opening %s \n", nomfic);
			return(EF_ERR_OPEN_READ);
		}

	  /*-----------------------------------------*/
	  /* Lecture 1er enregistrement (32 octets). */
	  /*-----------------------------------------*/
		fread(&tf_hd.no_version, sizeof(tf_hd.no_version), 1, fic);
		freadswab(&tf_hd.long_entete, sizeof(tf_hd.long_entete), 1, fic);
		freadswab(&tf_hd.adr_donnees, sizeof(tf_hd.adr_donnees), 1, fic);
		freadswab(&tf_hd.type_data, sizeof(tf_hd.type_data), 1, fic);
		fread(tf_hd.libre, 1, 23, fic);

	  /*------------------------------------------------*/
	  /* Lecture 2eme enregistrement (taille variable). */
	  /*------------------------------------------------*/
	  freadswab(&tf_hd.type_tfmoy, sizeof(tf_hd.type_tfmoy), 1, fic);
	  freadswab(&tf_hd.type_1_tfmoy, sizeof(tf_hd.type_1_tfmoy), 1, fic);
	  freadswab(&tf_hd.type_2_tfmoy, sizeof(tf_hd.type_2_tfmoy), 1, fic);
	  freadswab(&tf_hd.type_signal, sizeof(tf_hd.type_signal), 1, fic);
	  freadswab(&tf_hd.type_event, sizeof(tf_hd.type_event), 1, fic);
	  freadswab(&tf_hd.nb_voies, sizeof(tf_hd.nb_voies), 1, fic);
		
	  /***** Allocation des tableaux des no et des noms de voies. *****/
	  /****************************************************************/
	  tf_hd.tab_numvoies = (short *)calloc(tf_hd.nb_voies, sizeof(short));
	  tf_hd.tab_nomvoies = (tfmoy_nom_elec_type *)calloc(tf_hd.nb_voies, sizeof(tfmoy_nom_elec_type));

	  freadswab(tf_hd.tab_numvoies, sizeof(short), tf_hd.nb_voies, fic);
		for (i=0; i<tf_hd.nb_voies; i++)
			{
		  fread(&tf_hd.tab_nomvoies[i], sizeof(tfmoy_nom_elec_fic_type), 1, fic); /* ne lit que les 5 1er carcateres. */
			}
	  freadswab(&tf_hd.nb_ech, sizeof(tf_hd.nb_ech), 1, fic);
	  freadswab(&tf_hd.nb_ech_pre, sizeof(tf_hd.nb_ech_pre), 1, fic);
	  freadswab(&tf_hd.freq_ech, sizeof(tf_hd.freq_ech), 1, fic);
	  freadswab(&tf_hd.nb_freq, sizeof(tf_hd.nb_freq), 1, fic);
	  freadswab(&tf_hd.type_calc, sizeof(tf_hd.type_calc), 1, fic);
		
	  /***** Allocation tableau de la liste des frequences. *******/
	  /************************************************************/
	  tf_hd.tab_freq = (float *)calloc(tf_hd.nb_freq, sizeof(float));
	  freadswab(tf_hd.tab_freq, sizeof(float), tf_hd.nb_freq, fic);

	  /***** Allocation tableau de la liste des m_ondel par frequence. *******/
	  /***********************************************************************/
	  tf_hd.tab_m_ondel = (float *)calloc(tf_hd.nb_freq, sizeof(float));
	  freadswab(tf_hd.tab_m_ondel, sizeof(float), tf_hd.nb_freq, fic);
	  freadswab(&tf_hd.taille_fen_ondel, sizeof(tf_hd.taille_fen_ondel), 1, fic);
	  freadswab(&tf_hd.pas_sousech, sizeof(tf_hd.pas_sousech), 1, fic);

	  /***** Allocation tableau du nombre d'events moyennes par voie. *****/
	  /********************************************************************/
	  tf_hd.tab_nb_eve = (int *)calloc(tf_hd.nb_voies, sizeof(int));
	  freadswab(tf_hd.tab_nb_eve, sizeof(int), tf_hd.nb_voies, fic);
				
	  freadswab(&tf_hd.flag_corr_lb, sizeof(tf_hd.flag_corr_lb), 1, fic);

	  /* Lecture parametres correction ligne de base si besoin. */
	  /**********************************************************/
	  if  (tf_hd.flag_corr_lb > NONE)
	    {
	    freadswab(&tf_hd.deb_lb, sizeof(tf_hd.deb_lb), 1, fic);
	    freadswab(&tf_hd.fin_lb, sizeof(tf_hd.deb_lb), 1, fic);
	    }

	  freadswab(&tf_hd.flag_liss_tfmoy, sizeof(tf_hd.flag_liss_tfmoy), 1, fic);

	  /* Lecture parametres lissage si besoin. */
	  /*****************************************/
	  if  (tf_hd.flag_liss_tfmoy > NONE)
	    {
	    freadswab(&tf_hd.demi_fen_temp, sizeof(tf_hd.demi_fen_temp), 1, fic);
	    freadswab(&tf_hd.demi_fen_freq, sizeof(tf_hd.demi_fen_freq), 1, fic);
	    freadswab(&tf_hd.pas_ss_ech_temp, sizeof(tf_hd.pas_ss_ech_temp), 1, fic);
	    freadswab(&tf_hd.pas_ss_ech_freq, sizeof(tf_hd.pas_ss_ech_freq), 1, fic);
	    }

	  freadswab(&tf_hd.flag_moy_elec, sizeof(tf_hd.flag_moy_elec), 1, fic);

	  /* Lecture parametres moyennage des electrodes si besoin. */
	  /**********************************************************/
	  if  (tf_hd.flag_moy_elec > NONE)
	    {
	    /***** Allocation tableaux des nbres, no et noms d'elec par voie. *****/
	    /**********************************************************************/
	    tf_hd.tab_nb_elec_parvoie = (short *)calloc(tf_hd.nb_voies, sizeof(short));
	    tf_hd.liste_no_elec_parvoie = (short **)calloc(tf_hd.nb_voies, sizeof(short *));
	    tf_hd.liste_nom_elec_parvoie = (tfmoy_nom_elec_type **)calloc(tf_hd.nb_voies, sizeof(tfmoy_nom_elec_type *));

	    freadswab(tf_hd.tab_nb_elec_parvoie, sizeof(short), tf_hd.nb_voies, fic);

	    for (i=0; i<tf_hd.nb_voies; i++)
	       {
	       tf_hd.liste_no_elec_parvoie[i] = (short *)calloc(tf_hd.tab_nb_elec_parvoie[i], sizeof(short));
	       tf_hd.liste_nom_elec_parvoie[i] = (tfmoy_nom_elec_type *)calloc(tf_hd.tab_nb_elec_parvoie[i], sizeof(tfmoy_nom_elec_type));
	       }
	     /* Lecture des tableaux des no de voies. */
	     for (i=0; i<tf_hd.nb_voies; i++)
	       {
	       freadswab(tf_hd.liste_no_elec_parvoie[i], sizeof(short), tf_hd.tab_nb_elec_parvoie[i], fic);
	       }
	     /* Lecture des tableaux des noms de voies. */
	     for (i=0; i<tf_hd.nb_voies; i++)
	       {
	       fread(tf_hd.liste_nom_elec_parvoie[i], sizeof(tfmoy_nom_elec_type), tf_hd.tab_nb_elec_parvoie[i], fic);
	       }
	    }


	  /* Fermeture fichier. */
	  /*--------------------*/
	  fclose(fic);
	  break;

    /**************************/
    /*			      */
    /* Fichiers tfmoy textes. */
    /*			      */
    /**************************/
    case VER_TEXTE :
	  /*****************************************/
	  /*					   */
	  /* Lecture de l'entete du fichier texte. */
	  /*					   */
	  /*****************************************/
	 // rchmpi(nomfic, "nb_voies", 1, &intlu, &test_label);
	 // if (test_label != RCHMP_OK)
	 // {
	 //    fprintf(stderr, "Erreur lecture fichier %s champs nb_voies.\n", nomfic);
	 //    return 0;
	 // }
	 // tf_hd.nb_voies = (int)intlu;

	 // /* Allocation tableau des noms de voies. */
	 // tf_hd.voies = (char **)calloc(tf_hd.nb_voies, sizeof(char *));
		//if (tf_hd.voies == NULL)
		//	{
		//	fprintf(stderr, "ERROR : memory allocation for storing channel names.\n");
		//	return 0;
		//	}
		//for (i=0; i<tf_hd.nb_voies; i++)
		//	{
		//	tf_hd.voies[i] = (char *)calloc(sizeof(tfmoy_label_type), sizeof(char));
		//	if (tf_hd.voies[i] == NULL)
		//		{
		//		fprintf(stderr, "ERROR : memory allocation for storing channel names.\n");
		//		return 0;
		//		}
		//	}
	 // rchmpl(nomfic, "voies", tf_hd.nb_voies, tf_hd.voies, sizeof(tfmoy_label_type), &test_label);
	 // if (test_label != RCHMP_OK)
	 // {
	 //    fprintf(stderr, "Erreur lecture fichier %s champs voies.\n", nomfic);
	 //    return 0;
	 // }

	 // rchmpi(nomfic, "nb_ech", 1, &(tf_hd.nb_ech), &test_label);
	 // if (test_label != RCHMP_OK)
	 // {
	 //    fprintf(stderr, "Erreur lecture fichier %s champs nb_ech.\n", nomfic);
	 //    return 0;
	 // }

	 // rchmpi(nomfic, "nb_ech_pre", 1, &(tf_hd.nb_ech_pre), &test_label);
	 // if (test_label != RCHMP_OK)
	 // {
	 //    fprintf(stderr, "Erreur lecture fichier %s champs nb_ech_pre.\n", nomfic);
	 //    return 0;
	 // }

	 // rchmpf(nomfic, "fe", 1, &(tf_hd.freq_ech), &test_label);
	 // if (test_label != RCHMP_OK)
	 // {
	 //    fprintf(stderr, "Erreur lecture fichier %s champs freq_ech.\n", nomfic);
	 //    return 0;
	 // }

	 // rchmpi(nomfic, "nb_freq", 1, &(tf_hd.nb_freq), &test_label);
	 // if (test_label != RCHMP_OK)
	 // {
	 //    fprintf(stderr, "Erreur lecture fichier %s champs nb_freq.\n", nomfic);
	 //    return 0;
	 // }

	 // rchmpf(nomfic, "fdeb", 1, &(tf_hd.f_deb), &test_label);
	 // if (test_label != RCHMP_OK)
	 // {
	 //    fprintf(stderr, "Erreur lecture fichier %s champs fdeb.\n", nomfic);
	 //    return 0;
	 // }

	 // rchmpf(nomfic, "fpas", 1, &(tf_hd.f_pas), &test_label);
	 // if (test_label != RCHMP_OK)
	 // {
	 //    fprintf(stderr, "Erreur lecture fichier %s champs fpas.\n", nomfic);
	 //    return 0;
	 // }

	 // rchmpi(nomfic, "nb_eve", 1, &intlu, &test_label);
	 // if (test_label != RCHMP_OK)
	 // {
	 //    fprintf(stderr, "Erreur lecture fichier %s champs nb_eve.\n", nomfic);
	 //    return 0;
	 // }
	 // tf_hd.nb_eve = intlu;

	 // /*************************************************/
	 // /*						   */
	 // /* Initialisation des autres champs de l'entete. */
	 // /*						   */
	 // /*************************************************/
	 // tf_hd.long_entete = 0;
	 // tf_hd.adr_donnees = 0;
	 // tf_hd.type_data = TYPE_FLOAT; /* float=1 */
	 // tf_hd.type_tfmoy = INCONNU;
	 // tf_hd.type_1_tfmoy = INCONNU;
	 // tf_hd.type_2_tfmoy = INCONNU;
	 // tf_hd.type_signal = POTENTIEL;  /* Signal utilise : potentiel. */
	 // tf_hd.type_event = INCONNU;
	 // /***** Allocation des tableaux des no et des noms de voies. *****/
	 // /****************************************************************/
	 // tf_hd.tab_numvoies = (short *)calloc(tf_hd.nb_voies, sizeof(short));
	 // tf_hd.tab_nomvoies = (tfmoy_nom_elec_type *)calloc(tf_hd.nb_voies, sizeof(tfmoy_nom_elec_type));

	 // /* Transformation des anciens noms (nom.no) en noms et no separes. */
	 // /*******************************************************************/
	 // for (i=0; i<tf_hd.nb_voies; i++)
	 //   {
		//	j = 0;
		//	while ((j<strlen(tf_hd.voies[i])) && (tf_hd.voies[i][j]!='.'))
		//		{
		//		tf_hd.tab_nomvoies[i][j] = tf_hd.voies[i][j];
		//		j++;
		//		}
		//	tf_hd.tab_nomvoies[i][j] = 0;
		//	j++;
		//	k = 0;
		//	while ((j<strlen(tf_hd.voies[i])) && (tf_hd.voies[i][j]!=0x0A))
		//		{
		//		tmp[k] = tf_hd.voies[i][j];
		//		j++; k++;
		//		}
		//	tmp[k] = 0;
		//	tf_hd.tab_numvoies[i] = (short)atoi(tmp);
	 //   }

	 // tf_hd.type_calc = MORLET; /* Ondelettes de Morlet. */

	 // /***** Allocation et construction du tableau de la liste des frequences. *******/
	 // /*******************************************************************************/
	 // tf_hd.tab_freq = (float *)calloc(tf_hd.nb_freq, sizeof(float));
	 // for (i=0; i<tf_hd.nb_freq; i++)
	 //   { tf_hd.tab_freq[i] = tf_hd.f_deb + i*tf_hd.f_pas; }
	 // /***** Allocation et construction du tableau de la liste des m_ondel. *******/
	 // /****************************************************************************/
	 // tf_hd.tab_m_ondel = (float *)calloc(tf_hd.nb_freq, sizeof(float));
	 // for (i=0; i<tf_hd.nb_freq; i++)
	 //   { tf_hd.tab_m_ondel[i] = INCONNU; }
	 // tf_hd.taille_fen_ondel = INCONNU;
	 // tf_hd.pas_sousech = INCONNU;
	 // /***** Allocation tableau du nombre d'events moyennes par voie. *****/
	 // /********************************************************************/
	 // tf_hd.tab_nb_eve = (int *)calloc(tf_hd.nb_voies, sizeof(int));
	 // /* Tous ont le meme nbre d'evenements moyennes par voie. */
	 // for (i=0; i<tf_hd.nb_voies; i++)
	 //   { tf_hd.tab_nb_eve[i] = tf_hd.nb_eve; }
	 // tf_hd.flag_corr_lb = NONE;
	 // tf_hd.flag_liss_tfmoy = NONE;
	 // tf_hd.flag_moy_elec = NONE;
		//
		//	/* Get data offset. */
		//	fic = fopen(nomfic,"rb");
		//	if (fic == NULL)
		//	{
		//		fprintf(stderr, "ERROR: opening %s \n", nomfic);
		//		return(EF_ERR_OPEN_READ);
		//	}
		//	do
		//	{
		//		fscanf(fic, "%s", string);
		//		ok = strncmp(string, "data", 4);
		//	} while ((ok != 0) || (!EOF));

		//	if (ok != 0)
		//	{
		//		fprintf(stderr, "ERROR: can't find data in file %s .\n", nomfic);
		//		tf_hd.adr_donnees = 0;
		//	}
		//	else
		//	{
		//		tf_hd.adr_donnees = ftell(fic);
		//	}
		//	fclose(fic);
	  break;
		
		default:
			fprintf(stderr, "ERROR: unhandled original TF file version %d (sould be 0 or 1).\n", tf_hd.no_version);
			return(EF_ERR_READ_HEADER);
	}

	/* Conversion to new structure. */
	elan_struct->chan_nb = tf_hd.nb_voies;
	elan_struct->tf.event_code = tf_hd.type_event;
	elan_struct->tf.samp_nb = tf_hd.nb_ech;
	elan_struct->tf.prestim_samp_nb = tf_hd.nb_ech_pre;
	elan_struct->tf.sampling_freq = tf_hd.freq_ech;
	elan_struct->tf.freq_nb = tf_hd.nb_freq;
	switch (tf_hd.type_calc)
	{
		case MORLET:
			elan_struct->tf.wavelet_type = MorletWavelet;
			break;
		case GABOR:
			elan_struct->tf.wavelet_type = GaborWavelet;
			break;
	}
	
	ef_alloc_tf_array_byfreq(elan_struct);
	
	elan_struct->tf.event_nb = tf_hd.tab_nb_eve[0]; /* Use first channel number of averaged events. */
	for (i_f=0; i_f<elan_struct->tf.freq_nb; i_f++)
	{
		elan_struct->tf.freq_array[i_f] = tf_hd.tab_freq[i_f];
		elan_struct->tf.wavelet_char[i_f] = tf_hd.tab_m_ondel[i_f];
	}
	elan_struct->tf.blackman_window = tf_hd.taille_fen_ondel;
	
	ef_alloc_tf_array_otherevents(elan_struct);
	elan_struct->tf.other_events_nb[0] = 0;
	ef_alloc_tf_array_otherevents_list(elan_struct);

	/* Channel names and coordinates. */
	elec_dat_nb = 0;
	elec_dat_list = NULL;
	ef_orig_read_elecdat(&elec_dat_nb, &elec_dat_list);
	ef_alloc_channel_list(elan_struct);
	for (i_chan=0; i_chan<elan_struct->chan_nb; i_chan++)
	{
		strcpy(elan_struct->chan_list[i_chan].lab, tf_hd.tab_nomvoies[i_chan]);
		if ((tf_hd.tab_numvoies[i_chan] > 0) && (tf_hd.tab_numvoies[i_chan] < elec_dat_nb))
		{
			/* Spherical coordinates found. */
			elan_struct->chan_list[i_chan].coord_nb = 1;
			elan_struct->chan_list[i_chan].coord_list = (coord_t *)calloc(elan_struct->chan_list[i_chan].coord_nb, sizeof(coord_t));
			ELAN_CHECK_ERROR_ALLOC(elan_struct->chan_list[i_chan].coord_list, "for storing channel #%d coordinates.", i_chan+1);
			
			strcpy(elan_struct->chan_list[i_chan].coord_list[0].lab, CoordSystemLabelList[CoordSystemSphericalHead]);
			elan_struct->chan_list[i_chan].coord_list[0].val_nb = 3;
			elan_struct->chan_list[i_chan].coord_list[0].val_list = (float *)calloc(elan_struct->chan_list[i_chan].coord_list[0].val_nb, sizeof(float));
			ELAN_CHECK_ERROR_ALLOC(elan_struct->chan_list[i_chan].coord_list[0].val_list, "for storing channel #%d coordinates (%d values).", i_chan+1, elan_struct->chan_list[i_chan].coord_list[0].val_nb);
			elan_struct->chan_list[i_chan].coord_list[0].val_list[0] = 90;
			elan_struct->chan_list[i_chan].coord_list[0].val_list[1] = elec_dat_list[tf_hd.tab_numvoies[i_chan]-1].theta;
			elan_struct->chan_list[i_chan].coord_list[0].val_list[2] = elec_dat_list[tf_hd.tab_numvoies[i_chan]-1].phi;
		}
		else
		{
			/* No coordinates. */
			elan_struct->chan_list[i_chan].coord_nb = 0;
			elan_struct->chan_list[i_chan].coord_list = NULL;
		}
		strcpy(elan_struct->chan_list[i_chan].type, ChannelTypeLabelList[ChannelTypeEEG]);
		strcpy(elan_struct->chan_list[i_chan].unit, "microV2");
	}
		
	ef_orig_free_elecdat(&elec_dat_list);
	
  switch (tf_hd.no_version)
	{
		case VER_TEXTE :
			elan_struct->orig_info->tf_info.orig_version = ORIG_TF_TEXT;
			break;
		case VER_BINAIRE_1 :
			elan_struct->orig_info->tf_info.orig_version = ORIG_TF_BINARY;
			break;
	}
	elan_struct->orig_info->tf_info.offset_data = tf_hd.adr_donnees;
	
	ef_free_hd_tfmoy(tf_hd);

  return(EF_NOERR);
}


/*-----------------------------------------------------------------------------------------------*/
/*												 */
/* Teste le numero de version du fichier tfmoy stocke dans le premier octet.			 */
/*      val 1er octet :										 */
/*		* 1 : version 1.0 binaire.							 */
/*		* 'n' ou autre caractere ASCII : version 0.0 texte.				 */
/*												 */
/* Retourne le numero de version (0, 1...) ou -1 si erreur.					 */
/*												 */
/* date : 24-10-96 		Ver: 1.0	P.E.A.						 */
/*												 */
/*-----------------------------------------------------------------------------------------------*/
int ef_test_version_tfmoy(char *nom_fic)
{
  FILE *fic_tf;
  unsigned char version;

  fic_tf = fopen(nom_fic,"rb");
  if (fic_tf == NULL)
    {
    fprintf(stderr, "Can't open file %s .\n", nom_fic);
    return -1;
    }
  fread(&version, 1, 1, fic_tf);
  if ((int)version == (int)1)  { version = VER_BINAIRE_1; } /* fichier binaire v1.0 */
  else if (version>32)  { version = VER_TEXTE; }	    /* fichier texte	*/
  else { version = -1; }			/* format inconnu	*/

  fclose(fic_tf);
  return((int)version);
}


/*-----------------------------------------------------------------------------------------------*/
/*												 */
/* Desalloue la memoire reservee pour l'entete des fichiers tf_moy (allouee pour le tableau des   */
/* noms de voies).										 */
/*												 */
/* date : 30-10-96	     Ver: 2.0        P.E.A						 */
/*												 */
/*-----------------------------------------------------------------------------------------------*/
int ef_free_hd_tfmoy(tfmoy_header_type tf_hd)
{
  int i;

  if (tf_hd.no_version == VER_BINAIRE_1)
	{
    free(tf_hd.tab_numvoies);
    free(tf_hd.tab_nomvoies);
    free(tf_hd.tab_freq);
    free(tf_hd.tab_m_ondel);
    free(tf_hd.tab_nb_eve);
    if (tf_hd.flag_moy_elec != NONE)
		{
      for (i=0; i<tf_hd.nb_voies; i++)
			{
        free(tf_hd.liste_no_elec_parvoie[i]);
        free(tf_hd.liste_nom_elec_parvoie[i]);
			}
      free(tf_hd.liste_no_elec_parvoie);
      free(tf_hd.liste_nom_elec_parvoie);
      free(tf_hd.tab_nb_elec_parvoie);
		}
	}
  else  if (tf_hd.no_version == VER_TEXTE)
	{
    free(tf_hd.tab_numvoies);
    free(tf_hd.tab_nomvoies);
		free(tf_hd.tab_freq);
		free(tf_hd.tab_m_ondel);
    free(tf_hd.tab_nb_eve);
 		for (i=0; i<tf_hd.nb_voies; i++)
		{
			free(tf_hd.voies[i]);
		}
		free(tf_hd.voies);
	}

return 1;
}


/*--------------------------------------------------------------------------------------------------------------*/
/* ef_orig_TF_read_tfblock_one_channel: read a time-frequency block for one channel.                            */
/* filename: name of the file to read datra from.                                                               */
/* elan_struct_t *elan_struct: Elan structure (read from header).                                               */
/* int chanNum: indice of channel (starting from 0).                                                            */
/* int sampStart: indice of 1st sample to read (starting from 0).                                               */
/* int sampNb: number of samples to read (starting from 0).                                                     */
/* int freqStart: indice of 1st frequency to read (starting from 0).                                            */
/* int freqNb: number of frequencies to read (starting from 0).                                                 */
/* void **data_ptr: array of samples to store data. If NULL, data_ptr is allocated in function (must be freed). */
/*                 If allocated in caller, size must be [freqNbxsampNb] float or double.                        */
/* return value: number of samples actually read.                                                               */
/*--------------------------------------------------------------------------------------------------------------*/
int ef_orig_TF_read_tfblock_one_channel(char *filename, elan_struct_t *elan_struct, int chanNum, int sampStart, int sampNb, int freqStart, int freqNb, void ***data_ptr)
{
	FILE *data_f;
	int sampSize, i_samp, i_chan, i_f, sampRead, off_buf;
	float *bufFlt, **flt_ptr=NULL;
	double **dbl_ptr=NULL;
	char string[256];
	
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

		/* Read data for one channel. */
		/*----------------------------*/
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
		
		/* Store data. */
		switch (elan_struct->tf.data_type)
		{
			case EF_DATA_FLOAT:
				for (i_f=freqStart; i_f<freqStart+freqNb; i_f++)
				{
					off_buf = i_f*elan_struct->tf.samp_nb;
					for (i_samp=sampStart; i_samp<sampStart+sampNb; i_samp++)
					{
						flt_ptr[i_f-freqStart][i_samp-sampStart] = bufFlt[off_buf+i_samp];
					}
				}
			break;
			case EF_DATA_DOUBLE:
				for (i_f=freqStart; i_f<freqStart+freqNb; i_f++)
				{
					off_buf = i_f*elan_struct->tf.samp_nb;
					for (i_samp=sampStart; i_samp<sampStart+sampNb; i_samp++)
					{
						dbl_ptr[i_f-freqStart][i_samp-sampStart] = bufFlt[off_buf+i_samp];
					}
				}
			break;
		}

		fclose(data_f);
		return(freqNb*sampNb);
	}
	return(0);
}


/*--------------------------------------------------------------------------------------------------------------*/
/* ef_orig_TF_read_tfblock_all_channel: read a time-frequency block forall channels.                            */
/* filename: name of the file to read datra from.                                                               */
/* elan_struct_t *elan_struct: Elan structure (read from header).                                               */
/* int sampStart: indice of 1st sample to read (starting from 0).                                               */
/* int sampNb: number of samples to read (starting from 0).                                                     */
/* int freqStart: indice of 1st frequency to read (starting from 0).                                            */
/* int freqNb: number of frequencies to read (starting from 0).                                                 */
/* void **data_ptr: array of samples to store data. If NULL, data_ptr is allocated in function (must be freed). */
/*                 If allocated in caller, size must be [elan_struct.chan_nbxfreqNbxsampNb]                     */
/*                 float or double.                                                                             */
/* return value: number of samples actually read.                                                               */
/*--------------------------------------------------------------------------------------------------------------*/
int ef_orig_TF_read_tfblock_all_channel(char *filename, elan_struct_t *elan_struct, int sampStart, int sampNb, int freqStart, int freqNb, void ****data_ptr)
{
	FILE *data_f;
	int sampSize, i_samp, i_chan, i_f, sampRead, off_buf;
	float *bufFlt, ***flt_ptr=NULL;
	double ***dbl_ptr=NULL;
	char string[256];
	
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

		/* Read data for all channel. */
		/*----------------------------*/
		switch (elan_struct->orig_info->tf_info.orig_version)
		{
			case ORIG_TF_TEXT:
				fseek(data_f, elan_struct->orig_info->tf_info.offset_data, SEEK_SET);
				for (i_chan=0; i_chan<elan_struct->chan_nb; i_chan++)
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
					/* Store data. */
					switch (elan_struct->tf.data_type)
					{
						case EF_DATA_FLOAT:
							for (i_f=freqStart; i_f<freqStart+freqNb; i_f++)
							{
								off_buf = i_f*elan_struct->tf.samp_nb;
								for (i_samp=sampStart; i_samp<sampStart+sampNb; i_samp++)
								{
									flt_ptr[i_chan][i_f-freqStart][i_samp-sampStart] = bufFlt[off_buf+i_samp];
								}
							}
						break;
						case EF_DATA_DOUBLE:
							for (i_f=freqStart; i_f<freqStart+freqNb; i_f++)
							{
								off_buf = i_f*elan_struct->tf.samp_nb;
								for (i_samp=sampStart; i_samp<sampStart+sampNb; i_samp++)
								{
									dbl_ptr[i_chan][i_f-freqStart][i_samp-sampStart] = bufFlt[off_buf+i_samp];
								}
							}
						break;
					}
				}
				break;
			case ORIG_TF_BINARY:
				for (i_chan=0; i_chan<elan_struct->chan_nb; i_chan++)
				{
					fseek(data_f, elan_struct->orig_info->tf_info.offset_data+(elan_struct->tf.freq_nb*elan_struct->tf.samp_nb*i_chan), SEEK_SET);
					sampRead = freadswab(bufFlt, sizeof(float), elan_struct->tf.freq_nb*elan_struct->tf.samp_nb, data_f);

					if (sampRead != elan_struct->tf.freq_nb*elan_struct->tf.samp_nb)
					{
						fprintf(stderr, "Error reading TF data (%d actually read, %d required)\n", sampRead, elan_struct->tf.freq_nb*elan_struct->tf.samp_nb);
						return(EF_ERR_READ_DATA);
					}
					/* Store data. */
					switch (elan_struct->tf.data_type)
					{
						case EF_DATA_FLOAT:
							for (i_f=freqStart; i_f<freqStart+freqNb; i_f++)
							{
								off_buf = i_f*elan_struct->tf.samp_nb;
								for (i_samp=sampStart; i_samp<sampStart+sampNb; i_samp++)
								{
									flt_ptr[i_chan][i_f-freqStart][i_samp-sampStart] = bufFlt[off_buf+i_samp];
								}
							}
						break;
						case EF_DATA_DOUBLE:
							for (i_f=freqStart; i_f<freqStart+freqNb; i_f++)
							{
								off_buf = i_f*elan_struct->tf.samp_nb;
								for (i_samp=sampStart; i_samp<sampStart+sampNb; i_samp++)
								{
									dbl_ptr[i_chan][i_f-freqStart][i_samp-sampStart] = bufFlt[off_buf+i_samp];
								}
							}
						break;
					}
				}
				break;
		}
		
		fclose(data_f);
		return(elan_struct->chan_nb*freqNb*sampNb);
	}
	return(0);
}

