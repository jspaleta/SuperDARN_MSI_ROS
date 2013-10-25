#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include "control_program.h"
#include "global_server_variables.h"
#include "dio_handler.h"
#include "utils.h"
#include "iniparser.h"

extern int verbose;
extern pthread_mutex_t settings_lock;
extern dictionary *Site_INI;
extern int trigger_type;
extern int32 gpsrate;

void *settings_parse_ini_file(struct SiteSettings *ros_settings) {
     char ini_name[80]="/root/test.ini";
     char entry_name[80]="";
     char entry_value[256]="";
     int exists_flag;

     pthread_mutex_unlock(&settings_lock);

     if(Site_INI!=NULL) {
       iniparser_freedict(Site_INI);
       Site_INI=NULL;
     }
     sprintf(ini_name,"%s/site.ini",SITE_DIR);
     fprintf(stderr, "parsing file: %s\n", ini_name);
     Site_INI=iniparser_load(ini_name);
     if (Site_INI==NULL) {
       fprintf(stderr, "cannot parse file: %s\n", ini_name);
       pthread_mutex_unlock(&settings_lock);
       pthread_exit(NULL);
     }
     ros_settings->ifmode=iniparser_getboolean(Site_INI,"site_settings:ifmode",IF_ENABLED);
     ros_settings->use_beam_table=iniparser_getboolean(Site_INI,"beam_lookup_table:use_table",0);
     sprintf(ros_settings->beam_table_1,iniparser_getstring(Site_INI,"beam_lookup_table:beam_table_1",""));
     sprintf(ros_settings->beam_table_2,iniparser_getstring(Site_INI,"beam_lookup_table:beam_table_2",""));

     trigger_type=iniparser_getint(Site_INI,"site_settings:trigger_type",0);
     fprintf(stdout,"Trigger_type: %d\n",trigger_type); 
     switch(trigger_type) {
       case 0:
         gpsrate=0;
         gpsrate=iniparser_getint(Site_INI,"gps:trigger_rate",GPS_DEFAULT_TRIGRATE);
         fprintf(stdout,"GPSrate: %d\n",gpsrate); 
         break;
       case 1:
       case 2:
         gpsrate=iniparser_getint(Site_INI,"gps:trigger_rate",GPS_DEFAULT_TRIGRATE);
         fprintf(stdout,"GPSrate: %d\n",gpsrate); 
         break;
       default:
         break;
     }


     sprintf(ros_settings->name,"%s",iniparser_getstring(Site_INI,"site_settings:name",SITE_NAME));
     sprintf(entry_value,"%s",iniparser_getstring(Site_INI,"beam_lookup_table:use_table","ERROR"));
     printf("beam_lookup: %s\n",entry_value);
     sprintf(entry_value,"%s",iniparser_getstring(Site_INI,"beam_lookup_table:beam_table_1","ERROR"));
     printf("radar_1: %s\n",entry_value);
     sprintf(entry_value,"%s",iniparser_getstring(Site_INI,"beam_lookup_table:beam_table_2","ERROR"));
     printf("radar_2: %s\n",entry_value);
     pthread_mutex_unlock(&settings_lock);
     pthread_exit(NULL);

}

void *settings_rxfe_update_rf(struct RXFESettings *rxfe_rf_settings)
{

  pthread_mutex_lock(&settings_lock);

     fprintf(stdout,"RXFE RF Mode Settings from INI File ::\n");

     rxfe_rf_settings->ifmode=0;
            fprintf(stdout,"RXFE :: IF: %d\n",rxfe_rf_settings->ifmode);                              

     rxfe_rf_settings->amp1=iniparser_getboolean(Site_INI,"rxfe_rf:enable_amp1",0);
            fprintf(stdout,"RXFE :: amp1: %d\n",rxfe_rf_settings->amp1);                              
     rxfe_rf_settings->amp2=iniparser_getboolean(Site_INI,"rxfe_rf:enable_amp2",0);
            fprintf(stdout,"RXFE :: amp2: %d\n",rxfe_rf_settings->amp2);                              
     rxfe_rf_settings->amp3=iniparser_getboolean(Site_INI,"rxfe_rf:enable_amp3",0);
            fprintf(stdout,"RXFE :: amp3: %d\n",rxfe_rf_settings->amp3);                              
  
     rxfe_rf_settings->att1=iniparser_getboolean(Site_INI,"rxfe_rf:enable_att1",0);
            fprintf(stdout,"RXFE :: att1: %d\n",rxfe_rf_settings->att1);                              
     rxfe_rf_settings->att2=iniparser_getboolean(Site_INI,"rxfe_rf:enable_att2",0);
            fprintf(stdout,"RXFE :: att2: %d\n",rxfe_rf_settings->att2);                              
     rxfe_rf_settings->att3=iniparser_getboolean(Site_INI,"rxfe_rf:enable_att3",0);
            fprintf(stdout,"RXFE :: att3: %d\n",rxfe_rf_settings->att3);                              
     rxfe_rf_settings->att4=iniparser_getboolean(Site_INI,"rxfe_rf:enable_att4",0);
            fprintf(stdout,"RXFE :: att4: %d\n",rxfe_rf_settings->att4);                              


  pthread_mutex_unlock(&settings_lock);
  pthread_exit(NULL);
}

void *settings_rxfe_update_if(struct RXFESettings *rxfe_if_settings)
{

  pthread_mutex_lock(&settings_lock);

     fprintf(stdout,"RXFE IF Mode Settings from INI File ::\n");

     rxfe_if_settings->ifmode=1;
            fprintf(stdout,"RXFE :: IF: %d\n",rxfe_if_settings->ifmode);                              

     rxfe_if_settings->amp1=iniparser_getboolean(Site_INI,"rxfe_if:enable_amp1",0);
            fprintf(stdout,"RXFE :: amp1: %d\n",rxfe_if_settings->amp1);                              
     rxfe_if_settings->amp2=iniparser_getboolean(Site_INI,"rxfe_if:enable_amp2",0);
            fprintf(stdout,"RXFE :: amp2: %d\n",rxfe_if_settings->amp2);                              
     rxfe_if_settings->amp3=iniparser_getboolean(Site_INI,"rxfe_if:enable_amp3",0);
            fprintf(stdout,"RXFE :: amp3: %d\n",rxfe_if_settings->amp3);                              
  
     rxfe_if_settings->att1=iniparser_getboolean(Site_INI,"rxfe_if:enable_att1",0);
            fprintf(stdout,"RXFE :: att1: %d\n",rxfe_if_settings->att1);                              
     rxfe_if_settings->att2=iniparser_getboolean(Site_INI,"rxfe_if:enable_att2",0);
            fprintf(stdout,"RXFE :: att2: %d\n",rxfe_if_settings->att2);                              
     rxfe_if_settings->att3=iniparser_getboolean(Site_INI,"rxfe_if:enable_att3",0);
            fprintf(stdout,"RXFE :: att3: %d\n",rxfe_if_settings->att3);                              
     rxfe_if_settings->att4=iniparser_getboolean(Site_INI,"rxfe_if:enable_att4",0);
            fprintf(stdout,"RXFE :: att4: %d\n",rxfe_if_settings->att4);                              


  pthread_mutex_unlock(&settings_lock);
  pthread_exit(NULL);
}



