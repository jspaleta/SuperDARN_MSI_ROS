#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include "control_program.h"
#include "global_server_variables.h"
#include "dio_handler.h"
#include "timing_handler.h"
#include "reciever_handler.h"
#include "dds_handler.h"

extern struct Thread_List_Item *controlprogram_threads;
//extern pthread_mutex_t coord_lock;
extern int *trigger_state_pointer; //0-no activity,1-pre-trigger, 2-triggering 
extern int *ready_state_pointer; //0-no cntrolprograms ready,1-some controlprograms ready, 2-all controlprograms ready 
extern int trigger_type;  //0-strict controlprogram ready  1-elasped software time  2-external gps event 
extern int txread[MAX_RADARS];
extern int verbose;
extern int gpssock;
extern struct timeval t_ready_first,t_ready_final,t_pre_start,t_pre_end,t_post_start,t_post_end;
int oldv;
void *coordination_handler(struct ControlProgram *control_program)
{
   int numcontrolprograms=0,numready=0,numprocessing=0,rc,i,temp;
   char *timestr;
   pthread_t threads[4];
   struct Thread_List_Item *thread_list;
   int gpssecond,gpsnsecond;
   struct DriverMsg msg;
   struct ControlProgram *cprog;
   int ready_state,trigger_state;
   struct timeval t0,t1,t2,t3,t4,t5,t6;
   unsigned long elapsed;
   if(verbose > 1 ) gettimeofday(&t0,NULL);
   if (verbose > 1 ) fprintf(stderr,"Coord: Start Coord Handler: %ld %ld\n",(long)t0.tv_sec,(long)t0.tv_usec);
//   pthread_mutex_lock(&coord_lock); //lock the global structures

   ready_state=*ready_state_pointer;
   trigger_state=*trigger_state_pointer;
   if (control_program->active==1) {
     control_program->state->ready=1;
     control_program->state->processing=0;
     txread[control_program->parameters->radar-1]=1;
   } else {
   }
/*Calculate Ready State*/
   if(trigger_state<2) { 
     thread_list=controlprogram_threads;
     while(thread_list!=NULL){
       cprog=thread_list->data;
         if (cprog!=NULL) {
           if (cprog->active==1) {
             numcontrolprograms++;
             if (cprog->state->ready==1) {
               numready++;
             }
             if (cprog->state->processing==1) {
               numprocessing++;
             }

           }
         } else {
       }
       thread_list=thread_list->prev;
     }
     if (numready==0) {
       ready_state=0;
     } else {
       if(numready==1) gettimeofday(&t_ready_first,NULL);
       if ((numready!=numcontrolprograms) && (numready > 0)) {
         ready_state=1;
       } else {
         if ((numready==numcontrolprograms) && (numready > 0)) {
           ready_state=2;
           gettimeofday(&t_ready_final,NULL);
           if (verbose > 1) { 
              elapsed=(t_ready_final.tv_sec-t_ready_first.tv_sec)*1E6;
              elapsed+=(t_ready_final.tv_usec-t_ready_first.tv_usec);
              fprintf(stderr,"Coord: Client Ready Wait Elapsed Microseconds: %10ld :: sec: %10d usec: %10d\n",elapsed,t_ready_final.tv_sec,t_ready_final.tv_usec);
	      fflush(stderr);
           }
         } else {
         }
       }
     }
   }
   if (verbose > 1 ) fprintf(stderr,"Coord: Num ready: %d  Num clients: %d Ready State: %d \n",numready,numcontrolprograms,ready_state);
   
/*Coordinate Trigger events*/
      switch(ready_state) {
        case 0:
        /*no control program ready for trigger*/
          break;
        case 1:
        /*some control programs ready for trigger*/
          break;
        case 2:
        if(verbose > 1 ) gettimeofday(&t1,NULL);
          if (verbose > 1) { 
              elapsed=(t1.tv_sec-t0.tv_sec)*1E6;
              elapsed+=(t1.tv_usec-t0.tv_usec);
              printf("Coord: Start Ready State Case 2 Elapsed Microseconds: %ld\n",elapsed);
          }
        /*all control programs ready for trigger*/
          trigger_state=1;
          thread_list=controlprogram_threads;


          thread_list=controlprogram_threads;
          while(thread_list!=NULL){
            cprog=thread_list->data;
            if (cprog!=NULL) {
              if (cprog->active==1) {
                if (cprog->state->ready==1) {
                }
                if (cprog->state->processing==1) {
                }
              }
            } else {
            }
            thread_list=thread_list->prev;
          }

          if(verbose > 1 ) gettimeofday(&t2,NULL);
          if (verbose > 1) { 
              elapsed=(t2.tv_sec-t1.tv_sec)*1E6;
              elapsed+=(t2.tv_usec-t1.tv_usec);
              printf("Coord: Pre-Trigger Active Check %d Elapsed Microseconds: %ld\n",i,elapsed);
          }
 
          gettimeofday(&t_pre_start,NULL);
          i=0;
          rc = pthread_create(&threads[i], NULL, (void *) &receiver_pretrigger, NULL);
            pthread_join(threads[i],NULL);
          i++;
          rc = pthread_create(&threads[i], NULL, (void *) &dds_pretrigger, NULL);
            pthread_join(threads[i],NULL);
          i++;
          rc = pthread_create(&threads[i], NULL, (void *) &DIO_pretrigger, NULL);
            pthread_join(threads[i],NULL);
          i++;
          rc = pthread_create(&threads[i], NULL, (void *) &timing_pretrigger, NULL);
            pthread_join(threads[i],NULL);
          for (;i>=0;i--) {
            pthread_join(threads[i],NULL);
          }
          i=0;

          trigger_state=2; //trigger
/*
 *             trigger_type:  0: free run  1: elapsed-time  2: gps
 */       
          usleep(1000);
          gettimeofday(&t_pre_end,NULL);
          if (verbose > 1) { 
              elapsed=(t_pre_end.tv_sec-t_pre_start.tv_sec)*1E6;
              elapsed+=(t_pre_end.tv_usec-t_pre_start.tv_usec);
              fprintf(stderr,"Coord: Pre-Trigger Thread Run Elapsed Microseconds: %10ld :: sec: %10d usec: %10d\n",elapsed,t_pre_end.tv_sec,t_pre_end.tv_usec);
	      fflush(stderr);
          }
          gettimeofday(&t3,NULL);
          rc = pthread_create(&threads[0], NULL, (void *) &timing_trigger, (void *)trigger_type);
          pthread_join(threads[0],NULL);
          if (verbose > 1) { 
            gettimeofday(&t4,NULL);
            elapsed=(t4.tv_sec-t3.tv_sec)*1E6;
            elapsed+=(t4.tv_usec-t3.tv_usec);
            printf("Coord: Trigger Elapsed Microseconds: %ld\n",elapsed);
          }
          trigger_state=3;//post-trigger
          msg.type=GPS_GET_EVENT_TIME;
          msg.status=1;
          send_data(gpssock, &msg, sizeof(struct DriverMsg));
          recv_data(gpssock,&gpssecond, sizeof(int));
          recv_data(gpssock,&gpsnsecond, sizeof(int));
          recv_data(gpssock, &msg, sizeof(struct DriverMsg));

          thread_list=controlprogram_threads;
          while(thread_list!=NULL){
            cprog=thread_list->data;
            if (cprog!=NULL) {
              if (cprog->active==1) {
                cprog->state->gpssecond=gpssecond;
                cprog->state->gpsnsecond=gpsnsecond;
                cprog->data->event_secs=cprog->state->gpssecond;
                cprog->data->event_nsecs=cprog->state->gpsnsecond;

                if (txread[cprog->parameters->radar-1]){
                  i=0;
                  rc = pthread_create(&threads[i], NULL, (void *) &DIO_transmitter_status, (void *)cprog->parameters->radar);
                  pthread_join(threads[i],NULL);
                  txread[cprog->parameters->radar-1]=0;
                }
              }
            }
            thread_list=thread_list->prev;
          }

          //fprintf(stderr,"Coord: Post Start:\n");
          i=0; 
          rc = pthread_create(&threads[i], NULL, (void *) &receiver_posttrigger, NULL);
//              pthread_join(threads[i],NULL);
          i++;
          rc = pthread_create(&threads[i], NULL, (void *) &timing_posttrigger, NULL);
//              pthread_join(threads[i],NULL);
          for (;i>=0;i--) {
            pthread_join(threads[i],NULL);
          }
          //fprintf(stderr,"Coord: Post End:\n");
          thread_list=controlprogram_threads;
          while(thread_list!=NULL){
            cprog=thread_list->data;
            if (cprog!=NULL) {
                if (cprog->state!=NULL) {
                  if (cprog->state->ready==1) {
                    cprog->state->ready=0;
                    cprog->state->processing=1;
                  }
                }
            }
            thread_list=thread_list->prev;
          }
          trigger_state=0;//post-trigger
          ready_state=0;
          if(verbose > 1 ) gettimeofday(&t6,NULL);
          if (verbose > 1) { 
            elapsed=(t6.tv_sec-t4.tv_sec)*1E6;
            elapsed+=(t6.tv_usec-t4.tv_usec);
            printf("Coord: Post Trigger Elapsed Microseconds: %ld\n",elapsed);
          }
          if (verbose > 1) { 
            elapsed=(t6.tv_sec-t0.tv_sec)*1E6;
            elapsed+=(t6.tv_usec-t0.tv_usec);
            printf("Coord: Total Elapsed Microseconds: %ld\n",elapsed);
          }
          gettimeofday(&t_post_end,NULL);
          if (verbose > 1) { 
              elapsed=(t_post_end.tv_sec-t_pre_start.tv_sec)*1E6;
              elapsed+=(t_post_end.tv_usec-t_pre_start.tv_usec);
              fprintf(stderr,"Coord: Pre-trigger start to Post-Trigger end Elapsed Microseconds: %10ld :: sec: %10d usec: %10d\n",elapsed,t_post_end.tv_sec,t_post_end.tv_usec);
	      fflush(stderr);
          }
          break; 
   } // end of ready_state switch
   *ready_state_pointer=ready_state;
   *trigger_state_pointer=trigger_state;

//   pthread_mutex_unlock(&coord_lock); //unlock 
   if(verbose > 1 ) gettimeofday(&t1,NULL);
   if (verbose > 1 ) fprintf(stderr,"Coord: End Coord Handler: %ld %ld\n",(long)t1.tv_sec,(long)t1.tv_usec);
   pthread_exit(NULL);
};


