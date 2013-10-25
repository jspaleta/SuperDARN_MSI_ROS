#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sched.h>

#include <sys/neutrino.h>

typedef int bool;

#define ASSERT(expression) assert(expression, __FILE__, __LINE__)
void assert(const bool expression, const char* file, const int line) {
   if(!expression) {
      printf("Assert %s:%i\n", file, line);
      exit(-1);
   }
}

#define NSEC_PER_SEC 1000000000

void timespecnorm(struct timespec* ts) {
   while(ts->tv_nsec >= NSEC_PER_SEC) {
      ts->tv_nsec -= NSEC_PER_SEC;
      ts->tv_sec++;
   }
}

int timespecdiffns(const struct timespec* after, const struct timespec* before) {
   int diffns;
   diffns = NSEC_PER_SEC * ((int) after->tv_sec - (int) before->tv_sec);
   diffns += (int) after->tv_nsec - (int) before->tv_nsec;
   return diffns;
}

int main(int argc, char* argv[]) {
   int result;
   struct timespec ts;
   struct timespec interval;
   struct timespec before;
   struct timespec after;
   struct sched_param sp;
   struct _clockperiod clockperiod;

   printf("Build Time = %s\n", __TIME__);

   if(argc != 3) {
      printf("Usage: testsleep clockus intervalus\n");
      exit(-1);
   }

   int clockus = atoi(argv[1]);
   int intervalus = atoi(argv[2]);

   interval.tv_sec = 0;
   interval.tv_nsec = 1000 * intervalus;
   timespecnorm(&interval);

   printf("Interval = %i us\n", intervalus);

   printf("Set Scheduler Policy\n");
   sp.sched_priority = 80;
   result = sched_setscheduler(0, SCHED_FIFO, &sp);
   ASSERT(result == 0);

   printf("Set Neutrino Clock Time = %i us\n", clockus);
   clockperiod.nsec = 1000 * clockus;
   clockperiod.fract = 0;
   result = ClockPeriod(CLOCK_REALTIME, &clockperiod, NULL, 0);
   ASSERT(result == 0);

   printf("Get Clock Resolution Result\n");
   result = clock_getres(CLOCK_REALTIME, &ts);
   ASSERT(result == 0);
   printf(" = %i s %i ns\n", (int) ts.tv_sec, (int) ts.tv_nsec);

   result = clock_gettime(CLOCK_REALTIME, &before);
   ASSERT(result == 0);
   printf("Time Before = %i sec %li nsec\n", before.tv_sec, before.tv_nsec);

   printf("Before Sleep\n");
   result = clock_nanosleep(CLOCK_REALTIME, 0, &interval, NULL);
   ASSERT(result == 0);
   printf("After Sleep\n");

   result = clock_gettime(CLOCK_REALTIME, &after);
   ASSERT(result == 0);
   printf("Time After = %i sec %li nsec\n", after.tv_sec, after.tv_nsec);

   before.tv_nsec += 1000 * intervalus;
   timespecnorm(&before);

   int diffns = timespecdiffns(&after, &before);
   int diffus = diffns / 1000;

   printf("Diff = %i us\n", diffus);

   return 0;
} 
