
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include "control_program.h"
#include "global_server_variables.h"
#include "utils.h"
#include "coordination_handler.h"
#include "dio_handler.h"
#include "reciever_handler.h"
#include "timing_handler.h"
#include "dds_handler.h"
#include "log_handler.h"
#include "settings_handler.h"
#include "priority.h"
#include "rtypes.h"
#include "iniparser.h"

extern pthread_mutex_t controlprogram_list_lock,exit_lock,coord_lock;
extern char *controlprogram_list_lock_buffer,*exit_lock_buffer;
extern void *radar_channels[MAX_RADARS*MAX_CHANNELS];
extern struct Thread_List_Item *controlprogram_threads;
extern struct TRTimes bad_transmit_times;
extern int verbose;
extern struct tx_status txstatus[MAX_RADARS];
extern struct SiteSettings site_settings;
extern dictionary *Site_INI;
struct timeval t_ready_first,t_pre_start,t_pre_end;
int unregister_radar_channel(struct ControlProgram *control_program)
{
  int i,r,c,status;
  status=0;
  if (verbose > 1 ) {
    fprintf(stdout,"Unregister_radar_channel: Current State:\n");
    for(i=0;i<MAX_RADARS*MAX_CHANNELS;i++) {
      r=(i / MAX_CHANNELS)+1;
      c=(i % MAX_CHANNELS)+1;
      fprintf(stdout,"  %d : r:%d c:%d :: cp: %p\n",i,r,c,radar_channels[i]);
    }
    fflush(stdout);
  }
  if (control_program!=NULL) {
    for(i=0;i<MAX_RADARS*MAX_CHANNELS;i++) {
      if (radar_channels[i]==(void *)control_program) {
        printf("Unregistering: %d :: cp: %p :: r: %d c: %d\n",i,control_program,control_program->parameters->radar,control_program->parameters->channel);
        status++;
        radar_channels[i]=NULL;
        control_program->parameters->radar=0;
        control_program->parameters->channel=0;
      }
    }
  }
  return status;
}

struct ControlProgram* find_registered_controlprogram_by_radar_channel(int radar,int channel)
{
  int i,r,c;
  struct ControlProgram *control_program;
  control_program=NULL;
  for(i=0;i<MAX_RADARS*MAX_CHANNELS;i++) {
    r=(i / MAX_CHANNELS)+1;
    c=(i % MAX_CHANNELS)+1;
    if (radar==r && channel==c) {
      control_program=radar_channels[i]; 
      break;
    }
  }
  return  control_program;
}

struct ControlPRM controlprogram_fill_parameters(struct ControlProgram *control_program)
{
  struct ControlPRM control_parameters;      
  struct ControlProgram *cp,*best[MAX_RADARS];      
  int priority=99; //Lowest priority wins-- its like golf
  int r,c;
  if (TX_BEAM_PRIORITY |RX_BEAM_PRIORITY |TX_FREQ_PRIORITY|RX_FREQ_PRIORITY|TIME_SEQ_PRIORITY) {  
    for (r=1;r<=MAX_RADARS;r++) {
      priority=99;
      best[r-1]=NULL; 
      for (c=1;c<=MAX_CHANNELS;c++) {
        cp=find_registered_controlprogram_by_radar_channel(r,c);
        if (cp!=NULL) {
          //if (cp->active!=0) {
            if (cp->parameters!=NULL) {
              if (cp->parameters->priority<priority) {
                best[r-1]=cp;
                priority=cp->parameters->priority;
              }
            }
          //}
        }
      }
    }
  }
  if (control_program!=NULL) {
    if (control_program->parameters!=NULL) {
       //strcpy(control_parameters.name,control_program->parameters->name);
       //strcpy(control_parameters.description,control_program->parameters->description);
       control_parameters.radar=control_program->radarinfo->radar;
       control_parameters.channel=control_program->radarinfo->channel;
       r=control_parameters.radar-1;
       control_parameters.current_pulseseq_index=control_program->parameters->current_pulseseq_index;
       control_parameters.priority=control_program->parameters->priority;
       if (TX_BEAM_PRIORITY) {
         control_parameters.tbeam=best[r]->parameters->tbeam;
         control_parameters.tbeamcode=best[r]->parameters->tbeamcode;
         control_parameters.tbeamwidth=best[r]->parameters->tbeamwidth;
         control_parameters.tbeamazm=best[r]->parameters->tbeamazm;
       } else {
         control_parameters.tbeam=control_program->parameters->tbeam;
         control_parameters.tbeamcode=control_program->parameters->tbeamcode;
         control_parameters.tbeamwidth=control_program->parameters->tbeamwidth;
         control_parameters.tbeamazm=control_program->parameters->tbeamazm;
       }
       if (TX_FREQ_PRIORITY) {
         control_parameters.tfreq=best[r]->parameters->tfreq;
         control_parameters.trise=best[r]->parameters->trise;
       } else {
         control_parameters.tfreq=control_program->parameters->tfreq;
         control_parameters.trise=control_program->parameters->trise;
       }
       if (RX_BEAM_PRIORITY) {
         control_parameters.rbeam=best[r]->parameters->rbeam;
         control_parameters.rbeamcode=best[r]->parameters->rbeamcode;
         control_parameters.rbeamwidth=best[r]->parameters->rbeamwidth;
         control_parameters.rbeamazm=best[r]->parameters->rbeamazm;
       } else {
         control_parameters.rbeam=control_program->parameters->rbeam;
         control_parameters.rbeamcode=control_program->parameters->rbeamcode;
         control_parameters.rbeamwidth=control_program->parameters->rbeamwidth;
         control_parameters.rbeamazm=control_program->parameters->rbeamazm;
       }
       if (RX_FREQ_PRIORITY) {
         control_parameters.rfreq=best[r]->parameters->rfreq;
         control_parameters.number_of_samples=best[r]->parameters->number_of_samples;
       } else {
         control_parameters.rfreq=control_program->parameters->rfreq;
         control_parameters.number_of_samples=control_program->parameters->number_of_samples;
       }
       control_parameters.buffer_index=control_program->parameters->buffer_index;
       control_parameters.baseband_samplerate=control_program->parameters->baseband_samplerate;
       control_parameters.filter_bandwidth=control_program->parameters->filter_bandwidth;
       control_parameters.match_filter=control_program->parameters->match_filter;
       control_parameters.status=control_program->parameters->status;
    } else {
    }
  } else {
  }
  return control_parameters;
}

struct ControlPRM* controlprogram_link_parameters(struct ControlPRM *control_parameters)
{
  return control_parameters;

}
struct ControlPRM* controlprogram_verify_parameters(struct ControlPRM *control_parameters)
{
/*
       control_parameters.name=control_program->parameters->name;
       control_parameters.description=control_program->parameters->description;
       control_parameters.radar=control_program->radarinfo->radar;
       control_parameters.channel=control_program->radarinfo->channel;
       control_parameters.current_pulseseq_index=control_program->parameters->current_pulseseq_index;
       control_parameters.priority=control_program->parameters->priority;
       control_parameters.tbeam=control_program->parameters->tbeam;
       control_parameters.tbeamcode=control_program->parameters->tbeamcode;
       control_parameters.tbeamwidth=control_program->parameters->tbeamwidth;
       control_parameters.tbeamazm=control_program->parameters->tbeamazm;
       control_parameters.tfreq=control_program->parameters->tfreq;
       control_parameters.trise=control_program->parameters->trise;
       control_parameters.rbeam=control_program->parameters->rbeam;
       control_parameters.rbeamcode=control_program->parameters->rbeamcode;
       control_parameters.rbeamwidth=control_program->parameters->rbeamwidth;
       control_parameters.rbeamazm=control_program->parameters->rbeamazm;
       control_parameters.rfreq=control_program->parameters->rfreq;
       control_parameters.number_of_samples=control_program->parameters->number_of_samples;
       control_parameters.buffer_index=control_program->parameters->buffer_index;
       control_parameters.baseband_samplerate=control_program->parameters->baseband_samplerate;
       control_parameters.filter_bandwidth=control_program->parameters->filter_bandwidth;
       control_parameters.match_filter=control_program->parameters->match_filter;
       control_parameters.status=control_program->parameters->status;
*/
  return control_parameters;
}

int register_radar_channel(struct ControlProgram *control_program,int radar,int channel)
{
  int i,r,c,status;
  if (control_program!=NULL) unregister_radar_channel(control_program);
  status=-1;
  if (verbose > 1 ) {
    fprintf(stdout,"Register_radar_channel: Current State:\n");
    for(i=0;i<MAX_RADARS*MAX_CHANNELS;i++) {
      r=(i / MAX_CHANNELS)+1;
      c=(i % MAX_CHANNELS)+1;
      fprintf(stdout,"  %d : r:%d c:%d :: cp: %p\n",i,r,c,radar_channels[i]);
    }
    fflush(stdout);
  }
  for(i=0;i<MAX_RADARS*MAX_CHANNELS;i++) {
    r=(i / MAX_CHANNELS)+1;
    c=(i % MAX_CHANNELS)+1;
    if (radar_channels[i]==NULL) {
      if (radar<=0) radar=r;
      if (channel<=0) channel=c;
      if (radar==r && channel==c) {
        fprintf(stdout,"  Registering:: radar: %d channel: %d :: cp: %p\n",radar,channel,control_program);
        fflush(stdout); 
        status=1;
        control_program->parameters->radar=radar; 
        control_program->parameters->channel=channel; 
        control_program->radarinfo->radar=radar; 
        control_program->radarinfo->channel=channel; 
        radar_channels[i]=control_program;
        break;
      }
    }
  }
  if (status==-1) {
        fprintf(stderr,"  Register Error:: radar: %d channel: %d :: cp: %p\n",radar,channel,control_program);
        fflush(stderr); 

  } 
  return status;
}

struct ControlProgram *control_init() {
       int i;
       struct ControlProgram *control_program;

       control_program=malloc(sizeof(struct ControlProgram));
       control_program->active=-1;
       control_program->clrfreqsearch.start=0;
       control_program->clrfreqsearch.end=0;
       control_program->parameters=malloc(sizeof(struct ControlPRM));
       control_program->state=malloc(sizeof(struct ControlState));
       control_program->radarinfo=malloc(sizeof(struct RadarPRM));
       control_program->data=malloc(sizeof(struct DataPRM));
       control_program->main=NULL;
       control_program->back=NULL;
       control_program->main_address=NULL;
       control_program->back_address=NULL;
       strcpy(control_program->parameters->name,"Generic Control Program Name - 80");
       strcpy(control_program->parameters->description,"Generic  Control Program  Description - 120");
       control_program->parameters->radar=-1;
       control_program->parameters->channel=-1;
       control_program->parameters->current_pulseseq_index=-1;
       control_program->parameters->priority=50;
       control_program->parameters->tbeam=-1;
       control_program->parameters->tbeamcode=-1;
       control_program->parameters->tbeamwidth=-1;
       control_program->parameters->tbeamazm=-1;
       control_program->parameters->tfreq=-1;
       control_program->parameters->trise=10;
       control_program->parameters->rbeam=-1;
       control_program->parameters->rbeamcode=-1;
       control_program->parameters->rbeamwidth=-1;
       control_program->parameters->rbeamazm=-1;
       control_program->parameters->rfreq=-1;
       control_program->parameters->number_of_samples=-1;
       control_program->parameters->buffer_index=-1;
       control_program->parameters->baseband_samplerate=-1;
       control_program->parameters->filter_bandwidth=-1;
       control_program->parameters->match_filter=-1;
       control_program->parameters->status=-1;


//       control_program->parameters->phased=-1;
//       control_program->parameters->filter=-1;
//       control_program->parameters->gain=-1;
//       control_program->parameters->seq_no=-1;
//       control_program->parameters->seq_id=-1;
//       control_program->parameters->fstatus=-1;
//       control_program->parameters->center_freq=-1;

       control_program->state->cancelled=0;
       control_program->state->ready=0;
       control_program->state->linked=0;
       control_program->state->processing=0;
       control_program->state->best_assigned_freq=0;
       control_program->state->current_assigned_freq=0;
       control_program->state->freq_change_needed=0;
       control_program->state->thread=NULL;
       control_program->state->fft_array=NULL;
       control_program->radarinfo->site=-1;
       control_program->radarinfo->radar=-1;
       control_program->radarinfo->channel=-1;

       for (i=0;i<MAX_SEQS;i++) {
         control_program->state->pulseseqs[i]=NULL;
       }
       return control_program;
}

void controlprogram_exit(struct ControlProgram *control_program)
{
   pthread_t tid;
   pthread_t thread;
   int i,rc;
   int socket;
   struct ControlProgram *linker_program,*data;
   struct Thread_List_Item *thread_list,*thread_item,*thread_next,*thread_prev;
   int r,c;
   struct timeval t0,t1;

   pthread_t threads[10];

   gettimeofday(&t0,NULL);
   if (verbose > -1 ) fprintf(stderr,"Client: Start EXIT: %ld %ld\n",(long)t0.tv_sec,(long)t0.tv_usec);

   if(control_program!=NULL) {
     if (verbose > -1 ) fprintf(stderr,"Client: Exit Command\n");
     pthread_mutex_lock(&controlprogram_list_lock);
     control_program->state->cancelled=1;
     r=control_program->radarinfo->radar-1;
     c=control_program->radarinfo->channel-1;

     if (verbose > -1 ) fprintf(stderr,"  Client: EXIT:: r: %d c: %d\n",r,c);
     pthread_mutex_lock(&exit_lock);

     thread_list=controlprogram_threads;
     tid = pthread_self();
     control_program->active=0;
     control_program->state->ready=0;
     pthread_mutex_lock(&coord_lock);
     rc = pthread_create(&thread, NULL, (void *)&coordination_handler,(void *) control_program);
     pthread_join(thread,NULL);
     pthread_mutex_unlock(&coord_lock);
     i=0;
     rc = pthread_create(&threads[i], NULL,(void *) &timing_wait, NULL);
     pthread_join(threads[0],NULL);
     i=0;
     rc = pthread_create(&threads[i], NULL, (void *) &dds_end_controlprogram, NULL);
     i++;
     rc = pthread_create(&threads[i], NULL, (void *) &timing_end_controlprogram, NULL);
//   i++;
//   rc = pthread_create(&threads[i], NULL, (void *) &DIO_end_controlprogram, NULL);
     i++;
     rc = pthread_create(&threads[i], NULL, (void *) &receiver_end_controlprogram, NULL);
     for (;i>=0;i--) {
       pthread_join(threads[i],NULL);
     }
     if (verbose > -1 ) fprintf(stderr,"Closing Client Socket: %d\n",control_program->state->socket);
     close(control_program->state->socket);
     unregister_radar_channel(control_program);
/*     if (verbose>0) printf("Checking for programs linked to %p\n",control_program); 
     while (thread_list!=NULL) {
       linker_program=thread_list->data;
       if (verbose>0) printf("%p  %p %p\n",linker_program,linker_program->state->linked_program,control_program); 
       if (linker_program->state->linked_program==control_program) {
         if (verbose>0) printf("Found linked program %p that need to also die\n",linker_program); 
         pthread_cancel(thread_list->id);       
         pthread_join(thread_list->id,NULL);
         thread_item=thread_list;   
         thread_next=thread_item->next;
         thread_list=thread_item->prev;
         if (thread_next != NULL) thread_next->prev=thread_list;
         else controlprogram_threads=thread_list;
         if (thread_list != NULL) thread_list->next=thread_item->next;
         if(thread_item!=NULL) {
           free(thread_item);
           thread_item=NULL; 
         }
      } else {
         thread_list=thread_list->prev;
       }
     }
*/
     if (verbose> 1) fprintf(stderr,"Client Exit: Freeing internal structures for %p\n",control_program); 
     for (i=0;i<MAX_SEQS;i++) {
       if(control_program->state->pulseseqs[i]!=NULL) TSGFree(control_program->state->pulseseqs[i]);
     }
     if (verbose> 1) fprintf(stderr,"Client Exit: Freeing controlprogram state %p\n",control_program->state); 
     if(control_program->state!=NULL) {
       t0.tv_sec=0;
       t0.tv_usec=0;
       control_program->state->thread->last_seen=t0;
       if(control_program->state->fft_array!=NULL) {
         free(control_program->state->fft_array);
         control_program->state->fft_array=NULL;
       }
       free(control_program->state);
       control_program->state=NULL;
     }
     if (verbose>1) fprintf(stderr,"Client Exit: Freed controlprogram state %p\n",control_program->state);
     if (verbose>1) fprintf(stderr,"Client Exit: Freeing controlprogram parameters %p ",control_program->parameters); 
     if(control_program->parameters!=NULL) {
       free(control_program->parameters);
       control_program->parameters=NULL;
     }
     if (verbose>1) fprintf(stderr," %p\n",control_program->parameters); 
     if (verbose>1) fprintf(stderr,"Client Exit: Freeing controlprogram data %p\n",control_program->data); 
     if(control_program->main!=NULL) munmap(control_program->main);
     if(control_program->back!=NULL) munmap(control_program->back);
     control_program->main=NULL;
     control_program->back=NULL;
     control_program->main_address=NULL;
     control_program->back_address=NULL;
     if(control_program->data!=NULL) {
       free(control_program->data);
       control_program->data=NULL;
     }
     control_program->active=0;
     if (verbose>1) fprintf(stderr,"Client Exit: Done with control program %p\n",control_program); 
   }
   if (verbose>1) fprintf(stderr,"Client Exit: Waiting on Coordination thread\n");
   if (verbose>1) fprintf(stderr,"Client Exit: Done Waiting\n");

   pthread_mutex_unlock(&exit_lock);

   //printf("Client Exit: List UnLock\n");
   pthread_mutex_unlock(&controlprogram_list_lock);
   gettimeofday(&t1,NULL);
   if (verbose > -1 ) fprintf(stderr,"Client: End EXIT: %ld %ld :: %d %d\n",(long)t1.tv_sec,(long)t1.tv_usec,r,c);
}

void *control_handler(struct ControlProgram *control_program)
{
   int tid,i,r=-1,c=-1,status,rc,tmp;
   struct pollfd pfd;
   char command;
   int sockflag,poll_err_count,retval,socket,oldv,socket_err,length=sizeof(int);
   int32_t current_freq,radar=0,channel=0;
   struct timeval tv,current_time,last_report,t_get_data_start,t_get_data_end;
   struct ROSMsg msg; 
   struct DriverMsg dmsg; 
   struct DataPRM control_data; 
   struct ControlPRM control_parameters; 
   struct SiteSettings settings;
   struct TSGbuf *pulseseq;
   struct SeqPRM tprm;
   int data_int;
   pthread_t thread,threads[10];
   struct timeval t0,t1,t2,t3,t4;
   unsigned long elapsed;
   unsigned long ultemp;
   int32_t data_length;
   char entry_type,entry_name[80];
   int return_type,entry_exists;
   char *temp_strp;
   int32_t temp_int32;
   double temp_double;
/*
*  Init the Control Program state
*/
   r=control_program->radarinfo->radar-1;
   c=control_program->radarinfo->channel-1;
   pthread_mutex_lock(&exit_lock);
   pthread_mutex_lock(&controlprogram_list_lock);

   setbuf(stdout, 0);
   setbuf(stderr, 0);
   tid = pthread_self();
/* set the cancellation parameters --
   - Enable thread cancellation 
   - Defer the action of the cancellation 
*/
   pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
   pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
   pthread_cleanup_push((void *)&controlprogram_exit,(void *)control_program);
     if(control_program!=NULL) {
       socket=control_program->state->socket;
     }
   pthread_mutex_unlock(&controlprogram_list_lock);
   pthread_mutex_unlock(&exit_lock);

   poll_err_count=0;
   gettimeofday(&last_report,NULL);
   while (1) {
      if(control_program==NULL) {
        fprintf(stderr,"Client Error: cp is null: %p\n",control_program);
        fflush(stderr);
        break;
      }
      retval=getsockopt(socket, SOL_SOCKET, SO_ERROR, &socket_err, &length);
      if ((retval!=0) || (socket_err!=0)) {
            fprintf(stderr,"Client Error: socket error: %d : %d %d : %p\n",socket,retval,socket_err,control_program);
            fflush(stderr);
        break;
      }
      /* poll the socket and check for waiting messages */
      pfd.fd = socket;
      pfd.events = POLLIN | POLLHUP | POLLRDNORM ;
      pfd.revents = 0;
      sockflag=0;
      retval=poll(&pfd, 1, 1000);
      if (retval > 0) {
        if(pfd.revents & POLLHUP) {
          fprintf(stderr,"Client: poll socket: %d cp: %p retval: %d poll_revents: %d :: %d %d %d\n",socket,control_program,retval,
          pfd.revents,POLLHUP,POLLIN,POLLRDNORM);
          break;
        } else {

          char buffer[32];
          if (recv(socket, buffer, sizeof(buffer), MSG_PEEK | MSG_DONTWAIT) > 0) {
            sockflag=1;
            poll_err_count=0;      
          } else {
            fprintf(stdout,"Client: poll socket: %d cp: %p poll_revents: %d :: No recv que'd\n",socket,control_program,pfd.revents);
            fflush(stdout);
            poll_err_count++;      
          }
        } 
      } else {
        if (retval < 0 ) {
           perror("poll()");
           break; 
        }
        if (retval == 0 ){
          fprintf(stdout,"Client: poll socket: %d cp: %p retval: %d :: timeout\n",socket,control_program,retval);
          fflush(stdout);
          poll_err_count++;      
        }
      }
      if (poll_err_count > 3 ) {
            break;    
      }
/* sockflag: socket looks good, time to  read from it */
      if(sockflag) {
        r=control_program->radarinfo->radar-1;
        c=control_program->radarinfo->channel-1;
        pthread_mutex_lock(&controlprogram_list_lock);
        r=control_program->radarinfo->radar-1;
        c=control_program->radarinfo->channel-1;
        if ((r<0) || (c<0)) control_program->data->status=-1;
 
       /* Read controlprogram msg */
        recv_data(socket, &msg, sizeof(struct ROSMsg));
        gettimeofday(&current_time,NULL);
        if((current_time.tv_sec-last_report.tv_sec)>5) {
          system("date -t > /tmp/server_cmd_time");
          last_report=current_time;
        }
        control_program->state->thread->last_seen=current_time;
        pthread_mutex_unlock(&controlprogram_list_lock);

        /* Process controlprogram msg */
        switch(msg.type) {
          case PING:
            //printf("PING: START\n");
            gettimeofday(&t0,NULL);
            msg.status=1;
            send_data(socket, &msg, sizeof(struct ROSMsg));
            //printf("PING: END\n");
            break;
          case SET_INACTIVE:
            //printf("SET_RADAR_INACTIVE\n");
            gettimeofday(&t0,NULL);
            if (verbose > 1 ) fprintf(stderr,"Client: Set INACTIVE: %ld %ld :: %d %d\n",(long)t0.tv_sec,(long)t0.tv_usec,r,c);
            if ( (r < 0) || (c < 0)) {
              msg.status=-1;
              send_data(socket, &msg, sizeof(struct ROSMsg));
            } else {
              pthread_mutex_lock(&controlprogram_list_lock);
              if(control_program->active!=0) {
                control_program->active=-1;
                control_program->state->ready=0;
                pthread_mutex_lock(&coord_lock);
                rc = pthread_create(&thread, NULL, (void *)&coordination_handler,(void *)
control_program);
                pthread_join(thread,NULL);
                pthread_mutex_unlock(&coord_lock);
              }
              pthread_mutex_unlock(&controlprogram_list_lock);
              msg.status=1;
              send_data(socket, &msg, sizeof(struct ROSMsg));
            }
            gettimeofday(&t1,NULL);
            if (verbose > 1 ) fprintf(stderr,"Client: End INACTIVE: %ld %ld :: %d %d\n",(long)t1.tv_sec,(long)t1.tv_usec,r,c);
            //printf("end SET_RADAR_INACTIVE\n");
            break;
          case SET_ACTIVE:
            gettimeofday(&t0,NULL);
            if (verbose > 1 ) fprintf(stderr,"Client: Set ACTIVE: %ld %ld :: %d %d\n",(long)t0.tv_sec,(long)t0.tv_usec,r,c);
            //printf("SET_RADAR_ACTIVE\n");
            gettimeofday(&t0,NULL);
            if ( (r < 0) || (c < 0)) {
              msg.status=-1;
              send_data(socket, &msg, sizeof(struct ROSMsg));
            } else {
              pthread_mutex_lock(&controlprogram_list_lock);
              if(control_program->active!=0) {
                control_program->active=1;
                control_program->state->ready=0;
/*  JDS : Do not need to run coordinator when setting active
                pthread_mutex_lock(&coord_lock);
                rc = pthread_create(&thread, NULL, (void *)&coordination_handler,(void *)
control_program);
                pthread_join(thread,NULL);
                pthread_mutex_unlock(&coord_lock);
*/
              }
              pthread_mutex_unlock(&controlprogram_list_lock);
              msg.status=1;
              send_data(socket, &msg, sizeof(struct ROSMsg));
            }
            gettimeofday(&t1,NULL);
            if (verbose > 1 ) fprintf(stderr,"Client: End ACTIVE: %ld %ld :: %d %d\n",(long)t1.tv_sec,(long)t1.tv_usec,r,c);
            //printf("end SET_RADAR_ACTIVE\n");
            break;
/*
          case UPDATE_SITE_SETTINGS:
            gettimeofday(&t0,NULL);
            SettingsInit(&settings);
            recv_data(socket, &settings, sizeof(struct SiteSettings));
            msg.status=-1;
            send_data(socket, &msg, sizeof(struct ROSMsg));
            SettingsCpy(&settings,&site_settings);
            rc = pthread_create(&thread, NULL, (void *)&settings_rxfe_update_rf,(void *)&site_settings.rf_settings);
            pthread_join(thread,NULL);
            rc = pthread_create(&thread, NULL, (void *)&settings_rxfe_update_if,(void *)&site_settings.if_settings);
            pthread_join(thread,NULL);
            break;
*/
          case QUERY_INI_SETTING:
            //fprintf(stdout,"start QUERY_INI_SETTING\n");
            recv_data(socket, &data_length, sizeof(int32_t));
            recv_data(socket, &entry_name, data_length*sizeof(char));
            recv_data(socket, &entry_type, sizeof(char));
            entry_exists=iniparser_find_entry(Site_INI,entry_name);
            msg.status=entry_exists;
            switch(entry_type) {
              case 'i':
                //fprintf(stdout,"  entry type: i\n");
                return_type='i';
                temp_int32=iniparser_getint(Site_INI,entry_name,-1);
                send_data(socket, &return_type, sizeof(char));
                data_length=1;
                send_data(socket, &data_length, sizeof(int32_t));
                send_data(socket, &temp_int32, data_length*sizeof(int32_t));
                break;
              case 'b':
                //fprintf(stdout,"  entry type: b\n");
                return_type='b';
                temp_int32=iniparser_getboolean(Site_INI,entry_name,-1);
                send_data(socket, &return_type, sizeof(char));
                data_length=1;
                send_data(socket, &data_length, sizeof(int32_t));
                send_data(socket, &temp_int32, data_length*sizeof(int32_t));
                break;
              case 's':
                //fprintf(stdout,"  entry type: s\n");
                return_type='s';
                temp_strp=iniparser_getstring(Site_INI,entry_name,NULL);
                send_data(socket, &return_type, sizeof(char));
                data_length=strlen(temp_strp);
                send_data(socket, &data_length, sizeof(int32_t));
                send_data(socket, temp_strp, data_length*sizeof(char));
              default:
                return_type=' ';
                send_data(socket, &return_type, sizeof(char));
                data_length=0;
                send_data(socket, &data_length, sizeof(int32_t));
                send_data(socket, temp_strp, data_length*sizeof(char));
            }
            send_data(socket, &msg, sizeof(struct ROSMsg));
            //fprintf(stdout,"end QUERY_INI_SETTING\n");
            break;
          case GET_SITE_SETTINGS:
            gettimeofday(&t0,NULL);
            settings=site_settings;
            send_data(socket, &settings, sizeof(struct SiteSettings));
            msg.status=-1;
            send_data(socket, &msg, sizeof(struct ROSMsg));
            break;
          case SET_SITE_IFMODE:
            gettimeofday(&t0,NULL);
            settings=site_settings;
            recv_data(socket, &settings.ifmode, sizeof(settings.ifmode));
            msg.status=-1;
            send_data(socket, &msg, sizeof(struct ROSMsg));
            break;
          case SET_RADAR_CHAN:
            gettimeofday(&t0,NULL);
            fprintf(stdout,"SET_RADAR_CHAN: %d.%d\n",(int)t0.tv_sec,(int)t0.tv_usec);
            fflush(stdout);
              msg.status=1;
              recv_data(socket, &radar, sizeof(int32_t)); //requested radar
              recv_data(socket, &channel, sizeof(int32_t)); //requested channel
            fprintf(stdout,"  Radar: %d Chan: %d\n",radar,channel);
            fflush(stdout);
              pthread_mutex_lock(&controlprogram_list_lock);
              status=register_radar_channel(control_program,radar,channel);
              if (status) {
              }
              else {
                if (verbose>-1) fprintf(stderr,"Control Program thread %p Bad status %d no radar channel registered\n", tid,status);
              }
              msg.status=status;
              pthread_mutex_unlock(&controlprogram_list_lock);
              send_data(socket, &msg, sizeof(struct ROSMsg));
              //printf(" END SET_RADAR_CHAN\n");
            break;
          case LINK_RADAR_CHAN:
            gettimeofday(&t0,NULL);
            msg.status=1;
            recv_data(socket, &r, sizeof(r)); //requested radar
            recv_data(socket, &c, sizeof(c)); //requested channel
            pthread_mutex_lock(&controlprogram_list_lock);
            control_program->state->linked_program=find_registered_controlprogram_by_radar_channel(r,c);
            control_program->state->linked=1;
            if (control_program->state->linked_program!=NULL) {
              status=1;
            }
            else {
              status=0;
            }
            msg.status=status;
            pthread_mutex_unlock(&controlprogram_list_lock);
            send_data(socket, &msg, sizeof(struct ROSMsg));
            break;
          case GET_PARAMETERS:
            //fprintf(stdout,"GET_PARAMETERS: START\n");
            //fflush(stdout);
            gettimeofday(&t0,NULL);
            if ( (r < 0) || (c < 0)) {
              send_data(socket, &control_parameters, sizeof(struct ControlPRM));
              msg.status=-1;
              send_data(socket, &msg, sizeof(struct ROSMsg));
            } else {
              pthread_mutex_lock(&controlprogram_list_lock);
              msg.status=status;
              control_parameters=controlprogram_fill_parameters(control_program);
              pthread_mutex_unlock(&controlprogram_list_lock);
              send_data(socket, &control_parameters, sizeof(struct ControlPRM));
              send_data(socket, &msg, sizeof(struct ROSMsg));
            }
            //fprintf(stdout,"GET_PARAMETERS: END\n");
            //fflush(stdout);
            break;
          case GET_DATA:
            //printf("GET_DATA: START\n");
            //printf("GET_DATA: Event :: sec: %d nsec: %d\n",control_program->data->event_secs,control_program->data->event_nsecs);
            //printf("GET_DATA: bad_transmit_times:: length %d \n",bad_transmit_times.length);
            //for(i=0;i<bad_transmit_times.length;i++) {
            //  printf("GET_DATA: bad_transmit_times:: %d : %d %d\n",i,bad_transmit_times.start_usec[i],bad_transmit_times.duration_usec[i]);
            //}
            gettimeofday(&t0,NULL);
            if (verbose > 0 ) fprintf(stdout,"GET_DATA: r: %d c: %d  %d.%d\n",r,c,(int)t0.tv_sec,(int)t0.tv_usec);
            gettimeofday(&t_get_data_start,NULL);
            if (control_program->active != 1) { 
              control_program->data->status=-1;
              send_data(socket, control_program->data, sizeof(struct DataPRM));
              msg.status=-1;
              send_data(socket, &msg, sizeof(struct ROSMsg));
            } else {
              if ( (r < 0) || (c < 0)) {
                control_program->data->status=-1;
                send_data(socket, control_program->data, sizeof(struct DataPRM));
                msg.status=-1;
                send_data(socket, &msg, sizeof(struct ROSMsg));
              } else {
                msg.status=status;
                rc = pthread_create(&thread, NULL,(void *)&receiver_controlprogram_get_data,(void *) control_program);
                pthread_join(thread,NULL);
                send_data(socket, control_program->data, sizeof(struct DataPRM));
                if(control_program->data->status==0) {
                  //printf("GET_DATA: main: %d %d\n",sizeof(uint32_t),sizeof(uint32)*control_program->data->samples);
                  send_data(socket, control_program->main, sizeof(uint32_t)*control_program->data->samples);
                  send_data(socket, control_program->back, sizeof(uint32_t)*control_program->data->samples);
                  send_data(socket, &bad_transmit_times.length, sizeof(bad_transmit_times.length));
                  send_data(socket, bad_transmit_times.start_usec, sizeof(uint32_t)*bad_transmit_times.length);
                  send_data(socket, bad_transmit_times.duration_usec, sizeof(uint32_t)*bad_transmit_times.length);
                  tmp=MAX_TRANSMITTERS;
                  send_data(socket,&tmp,sizeof(int));
                  send_data(socket,&txstatus[r].AGC,sizeof(int)*tmp);
                  send_data(socket,&txstatus[r].LOWPWR,sizeof(int)*tmp);
                } else {
                  printf("GET_DATA: Bad status %d\n",control_program->data->status);
                } 
                send_data(socket, &msg, sizeof(struct ROSMsg));
              }
            }
            gettimeofday(&t1,NULL);
            if (verbose > 1) {
              elapsed=(t1.tv_sec-t0.tv_sec)*1E6;
              elapsed+=(t1.tv_usec-t0.tv_usec);
              if (verbose > 1 ) printf("Client:  Get Data Elapsed Microseconds: %ld\n",elapsed);
            }
            //printf("GET_DATA: END\n");
            gettimeofday(&t_get_data_end,NULL);
            if (verbose > 1) {
              elapsed=(t_get_data_end.tv_sec-t_pre_start.tv_sec)*1E6;
              elapsed+=(t_get_data_end.tv_usec-t_pre_start.tv_usec);
              fprintf(stderr,"Client %2d %2d:  From Pretrig start to Get Data end Elapsed Microseconds: %10ld  :: sec: %10d usec: %10d\n",r,c,elapsed,t_get_data_end.tv_sec,t_get_data_end.tv_usec);
              elapsed=(t_get_data_end.tv_sec-t_ready_first.tv_sec)*1E6;
              elapsed+=(t_get_data_end.tv_usec-t_ready_first.tv_usec);
              fprintf(stderr,"Client %2d %2d:  From Client Ready start to Get Data end Elapsed Microseconds: %10ld  :: sec: %10d usec: %10d\n",r,c,elapsed,t_get_data_end.tv_sec,t_get_data_end.tv_usec);
             fflush(stderr); 
            }
            break;
          case SET_PARAMETERS:
            //printf("SET_PARAMETERS: START\n");
            gettimeofday(&t0,NULL);
            if ( (r < 0) || (c < 0)) {
              recv_data(socket, control_program->parameters, sizeof(struct ControlPRM));
              msg.status=-1;
              send_data(socket, &msg, sizeof(struct ROSMsg));
            } else {
              msg.status=1;
              pthread_mutex_lock(&controlprogram_list_lock);
              recv_data(socket, control_program->parameters, sizeof(struct ControlPRM));
              if(control_program->parameters->rfreq<0) control_program->parameters->rfreq=control_program->parameters->tfreq;
              send_data(socket, &msg, sizeof(struct ROSMsg));
              pthread_mutex_unlock(&controlprogram_list_lock);
            }
            //printf("SET_PARAMETERS: END\n");
            break;
          case REGISTER_SEQ:
            gettimeofday(&t0,NULL);
            fprintf(stdout,"REGISTER_SEQ: r: %d c: %d  %d.%d\n",r,c,(int)t0.tv_sec,(int)t0.tv_usec);
            fflush(stdout);
            msg.status=1;
            recv_data(socket,&tprm, sizeof(struct SeqPRM)); // requested pulseseq
            pthread_mutex_lock(&controlprogram_list_lock);
            control_program->state->pulseseqs[tprm.index]=malloc(sizeof(struct TSGbuf));
            control_program->parameters->current_pulseseq_index=tprm.index;
            control_program->state->pulseseqs[tprm.index]->len=tprm.len;
            control_program->state->pulseseqs[tprm.index]->step=tprm.step;
            control_program->state->pulseseqs[tprm.index]->index=tprm.index;
            control_program->state->pulseseqs[tprm.index]->rep=
                malloc(sizeof(unsigned char)*control_program->state->pulseseqs[tprm.index]->len);
            control_program->state->pulseseqs[tprm.index]->code=
                malloc(sizeof(unsigned char)*control_program->state->pulseseqs[tprm.index]->len);
            recv_data(socket,control_program->state->pulseseqs[tprm.index]->rep, 
                sizeof(unsigned char)*control_program->state->pulseseqs[tprm.index]->len); // requested pulseseq
            recv_data(socket,control_program->state->pulseseqs[tprm.index]->code, 
                sizeof(unsigned char)*control_program->state->pulseseqs[tprm.index]->len); // requested pulseseq
            if ( (r < 0) || (c < 0)) {
              msg.status=-1;
            } else {
            //send on to timing socket
              rc = pthread_create(&threads[0], NULL, (void *)&timing_register_seq,(void *) control_program);
            //send on to dds socket
              rc = pthread_create(&threads[1], NULL, (void *)&dds_register_seq,(void *) control_program);
              //printf("Waiting on Timing Thread\n");
              pthread_join(threads[0],NULL);
              //printf("Waiting on DDS\n"); 
              pthread_join(threads[1],NULL);
            }
            pthread_mutex_unlock(&controlprogram_list_lock);
            //printf("REGISTER_SEQ: SEND ROSMsg\n");
            send_data(socket, &msg, sizeof(struct ROSMsg));
            gettimeofday(&t1,NULL);
            if (verbose > 1) {
              elapsed=(t1.tv_sec-t0.tv_sec)*1E6;
              elapsed+=(t1.tv_usec-t0.tv_usec);
              if (verbose > 1 ) printf("Client:  Reg Seq Elapsed Microseconds: %ld\n",elapsed);
            }
            //printf("REGISTER_SEQ: END\n");
            break;
          case SET_READY_FLAG:
            gettimeofday(&t0,NULL);
            if (verbose > 1 ) fprintf(stderr,"Client: Set READY: %ld %ld :: %d %d\n",(long)t0.tv_sec,(long)t0.tv_usec,r,c);
            if ( (r < 0) || (c < 0)) {
              msg.status=-1;
            } else {
              msg.status=0;
              pthread_mutex_lock(&controlprogram_list_lock);
              if (control_program->active!=0) { 
                control_program->active=1;
                control_program->state->ready=1;
              } 
              pthread_mutex_unlock(&controlprogram_list_lock);
              i=0;
              rc = pthread_create(&threads[i], NULL,(void *) &timing_wait, NULL);
              pthread_join(threads[0],NULL);
              pthread_mutex_lock(&controlprogram_list_lock);
              i=0;
              rc = pthread_create(&threads[i], NULL, (void *) &DIO_ready_controlprogram, control_program);
                pthread_join(threads[i],NULL);
              i++;
              rc = pthread_create(&threads[i], NULL, (void *) &timing_ready_controlprogram, control_program);
                pthread_join(threads[i],NULL);
              i++;
              rc = pthread_create(&threads[i], NULL, (void *) &dds_ready_controlprogram, control_program);
                pthread_join(threads[i],NULL);
              i++;
              rc = pthread_create(&threads[i], NULL, (void *) &receiver_ready_controlprogram, control_program);
                pthread_join(threads[i],NULL);
              for (;i>=0;i--) {
                gettimeofday(&t2,NULL);
                pthread_join(threads[i],NULL);
                gettimeofday(&t3,NULL);
                if (verbose > 1) {
                   elapsed=(t3.tv_sec-t2.tv_sec)*1E6;
                   elapsed+=(t3.tv_usec-t2.tv_usec);
                   if (verbose > 1 ) printf("Client:Set Ready: %d Elapsed Microseconds: %ld\n",i,elapsed);
                }
              }
              gettimeofday(&t2,NULL);
              pthread_mutex_lock(&coord_lock);
              rc = pthread_create(&thread, NULL, (void *)&coordination_handler,(void *) control_program);
              pthread_join(thread,NULL);
              pthread_mutex_unlock(&coord_lock);
              gettimeofday(&t3,NULL);
              if (verbose > 1) {
                   elapsed=(t3.tv_sec-t2.tv_sec)*1E6;
                   elapsed+=(t3.tv_usec-t2.tv_usec);
                   if (verbose > 1 ) printf("Client:Set Ready: Coord Elapsed Microseconds: %ld\n",elapsed);
              }
              pthread_mutex_unlock(&controlprogram_list_lock);
            }
            send_data(socket, &msg, sizeof(struct ROSMsg));
            gettimeofday(&t1,NULL);
            if (verbose > 1) {
              elapsed=(t1.tv_sec-t0.tv_sec)*1E6;
              elapsed+=(t1.tv_usec-t0.tv_usec);
              if (verbose > 1 ) printf("Client:  Set Ready Elapsed Microseconds: %ld\n",elapsed);
            }
            if (verbose > 1 ) fprintf(stderr,"Client: End READY: %ld %ld :: %d %d\n",(long)t1.tv_sec,(long)t1.tv_usec,r,c);
            break;

          case REQUEST_CLEAR_FREQ_SEARCH:
            gettimeofday(&t0,NULL);
            pthread_mutex_lock(&controlprogram_list_lock);
            recv_data(socket,&control_program->clrfreqsearch, sizeof(struct CLRFreqPRM)); // requested search parameters
            if (verbose > 1 )printf("Client: Requst CLRSearch: %d %d\n",control_program->clrfreqsearch.start,control_program->clrfreqsearch.end);
            if ( (r < 0) || (c < 0)) {
              msg.status=-1;
            } else {
              rc = pthread_create(&threads[0], NULL, (void *) &DIO_clrfreq,control_program);
              pthread_join(threads[0],NULL);
              rc = pthread_create(&threads[0], NULL, (void *) &receiver_clrfreq,control_program);
              pthread_join(threads[0],NULL);
              rc = pthread_create(&threads[0], NULL, (void *) &DIO_rxfe_reset,NULL);
              pthread_join(threads[0],NULL);
              msg.status=control_program->state->freq_change_needed;
            }
            send_data(socket, &msg, sizeof(struct ROSMsg));
            pthread_mutex_unlock(&controlprogram_list_lock);
            gettimeofday(&t1,NULL);
            if (verbose > 1) {
              elapsed=(t1.tv_sec-t0.tv_sec)*1E6;
              elapsed+=(t1.tv_usec-t0.tv_usec);
              if (verbose > 1 ) printf("Client:  CLR Elapsed Microseconds: %ld\n",elapsed);
            }
            break;
          case REQUEST_ASSIGNED_FREQ:
            gettimeofday(&t0,NULL);
            pthread_mutex_lock(&controlprogram_list_lock);
            if ( (r < 0) || (c < 0)) {
              msg.status=-1;
              control_program->state->current_assigned_freq=0;
              control_program->state->current_assigned_noise=0;
            } else {
              rc = pthread_create(&threads[0], NULL, (void *) &receiver_assign_frequency,(void *)  control_program);
              pthread_join(threads[0],NULL);
              msg.status=control_program->state->best_assigned_freq!=control_program->state->current_assigned_freq;
            }
            //control_program->state->current_assigned_noise=1;
            current_freq=control_program->state->current_assigned_freq; 
            send_data(socket, &current_freq, sizeof(int32_t));
            send_data(socket, &control_program->state->current_assigned_noise, sizeof(float));
            send_data(socket, &msg, sizeof(struct ROSMsg));
            pthread_mutex_unlock(&controlprogram_list_lock);
            gettimeofday(&t1,NULL);
            if (verbose > 1) {
              elapsed=(t1.tv_sec-t0.tv_sec)*1E6;
              elapsed+=(t1.tv_usec-t0.tv_usec);
              if (verbose > 1 ) printf("Client:  Request Freq Elapsed Microseconds: %ld\n",elapsed);
            }
            break;

          case QUIT:
            gettimeofday(&t0,NULL);
            fprintf(stdout,"Client QUIT:: %d.%06d \n",(int)t0.tv_sec,(int)t0.tv_usec);
            fflush(stdout);
            msg.status=0;
            send_data(socket, &msg, sizeof(struct ROSMsg));
            //controlprogram_exit(control_program);
            pthread_exit(NULL);
            break;
          default:
            msg.status=1;
            send_data(socket, &msg, sizeof(struct ROSMsg));
        }
          /* FD_ISSET(0, &rfds) will be true. */
      } //else printf("No data within five seconds.\n");
//        if (verbose>1) printf("Client: test cancel\n");
        
        pthread_testcancel();
   }
   fprintf(stdout,"Outside of socket while loop : %p\n",control_program);
   fflush(stdout);
   pthread_testcancel();
   pthread_cleanup_pop(0);
   controlprogram_exit(control_program);
   pthread_exit(NULL);
};

void *controlprogram_free(struct ControlProgram *control_program)
{

}
