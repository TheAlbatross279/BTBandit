/* Smartalloc.c       Copyright Clinton Staley 1991
 * 
 * Smartalloc provides an malloc version which checks for several possible
 * errors:
 *
 * 1. Failure of malloc or calloc call for any reason.
 * 2. Attempt to free memory not allocated by malloc/calloc, or already freed.
 * 3. Writing past the end or before the beginning of allocated memory.
 * 4. Failure to free memory by some point in the program.
 * 5. Use of data after it has been freed.
 * 6. Assumption that data returned by malloc is set to 0.
 *
 * Use smartalloc by including smartalloc.h in any file that calls malloc,
 * calloc or free.  Also, compile smartalloc.c along with your other .c files.
 * If you make any of errors 1-3 above, smartalloc will report the error
 * and the file/line on which it occured.  To find out if you have left
 * memory unfreed, call report_space().  If any unfreed memory is
 * outstanding, report_space will return the number of bytes of unfreed
 * memory.  If no memory is unfreed, report_space returns 0.
 * Use of freed data is "detected" by writing garbage into the freed data,
 * which will usually generate a runtime fault if the data is later used.
 *
 * All rights to this package are reserved by its author.  Duplication of
 * source or object code is permitted only with the author's permission.
 */

 /*
  * Changes by John Bellardo to merge C and C++ versions into unified
  * source.
  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>

#define PATTERN 0xA
#define MARGIN 4
#define HASH_SIZE	113
#define smartalloc_hash(x) ( ((unsigned long)(x)) % HASH_SIZE )

#ifdef __cplusplus
extern "C" {
#endif

static pthread_mutex_t mutt = PTHREAD_MUTEX_INITIALIZER;

typedef struct track_t {
   char *data;
   unsigned long space;
   unsigned char needs_free;
   unsigned short margin;
   struct track_t *next;
} track_t, *track_t_ptr;

static track_t_ptr track_hash[HASH_SIZE];
static unsigned long allocated = 0;
static int inited = 0;

static void completion_function(void)
{
	if (allocated == 0)
		return;
	fprintf(stderr, "%lu bytes left unfreed at end of program!\n", allocated);
}

void smartalloc_track(char *data, unsigned long space, unsigned char needs_free, unsigned short margin)
{
   track_t *temp;

   if (inited == 0) {
      atexit(completion_function);
	  memset(track_hash, 0, sizeof(track_hash));
	  inited = 1;
   }
   if ((temp = (track_t *) malloc(sizeof(track_t))) == NULL)
   {
      fprintf(stderr, "Malloc failure in in smartalloc\n");
      exit(1);
   }

   pthread_mutex_lock(&mutt);
   allocated += space;
   temp->needs_free = needs_free;
   temp->data = data;
   temp->space = space;
   temp->margin = margin;
   temp->next = track_hash[smartalloc_hash(data)];
   track_hash[smartalloc_hash(data)] = temp;
   pthread_mutex_unlock(&mutt);
}

void *smartalloc(unsigned long bytes, char *file, int line, char fill)
{
   char *data;

   if ( (data = (char *)malloc(bytes + 2*MARGIN)) == NULL) {
      fprintf(stderr, "Malloc failure in file %s on line %d\n", file, line);
      exit(1);
   }
   data += MARGIN;

   memset(data, fill, bytes);
   memset(data-MARGIN, PATTERN, MARGIN);
   memset(data+bytes, PATTERN, MARGIN);
   smartalloc_track(data, bytes, 1, MARGIN);

   return data;
}

void *smartvalloc(unsigned long bytes, char *file, int line, char fill)
{
   char *data;

   if ( (data = (char *)valloc(bytes + 2*MARGIN)) == NULL) {
      fprintf(stderr, "Malloc failure in file %s on line %d\n", file, line);
      exit(1);
   }
   data += MARGIN;

   memset(data, fill, bytes);
   memset(data-MARGIN, PATTERN, MARGIN);
   memset(data+bytes, PATTERN, MARGIN);
   smartalloc_track(data, bytes, 1, MARGIN);

   return data;
}

track_t *removeTrackNode(void *address)
{
   track_t *temp, *to_free;

   if (track_hash[smartalloc_hash(address)] == NULL)
      return NULL;

   pthread_mutex_lock(&mutt);
   if (track_hash[smartalloc_hash(address)]->data == address) {
      to_free = track_hash[smartalloc_hash(address)];
      track_hash[smartalloc_hash(address)] = track_hash[smartalloc_hash(address)]->next;
   } 
   else {
      for (temp = track_hash[smartalloc_hash(address)]; temp->next != NULL && temp->next->data != address;)
         temp = temp->next;
      if (temp->next == NULL) {
		   pthread_mutex_unlock(&mutt);
         return NULL;
      }
      to_free = temp->next;
      temp->next = to_free->next;
   }
   allocated -= to_free->space;
   pthread_mutex_unlock(&mutt);

   return to_free;
}

void freechecks(track_t *check, char *file, int line)
{
   int i;
   for (i = 0; i < check->margin; i++)
      if (check->data[check->space + i] != PATTERN ||
       check->data[-check->margin + i] != PATTERN) {
         fprintf(stderr, 
          "Space freed in file %s at line %d has data written past bounds.\n",
          file, line);
         break;
      }
   memset(check->data, PATTERN, check->space);
}

void smartfree(void *address, char *file, int line)
{
   track_t *to_free;

   to_free = removeTrackNode(address);
   if (NULL == to_free) {
      fprintf(stderr, 
       "Attempt to free non-malloced space in file %s at line %d\n",
       file, line);
      return;
   }

   freechecks(to_free, file, line);
   if (to_free->needs_free)
      free(to_free->data - to_free->margin);

   free(to_free);
}

void* smartrealloc(void* ptr, unsigned long newSize, int freeOnFailure,
      char *file, int line, char fill)
{
   track_t *to_free;
   int limit;
   void *newMem;
   char *s, *d;

   to_free = removeTrackNode(ptr);
   if (NULL == to_free) {
      fprintf(stderr, 
       "Attempt to free non-malloced space in file %s at line %d\n",
       file, line);
      return NULL;
   }

   newMem = smartalloc(newSize, file, line, fill);
   if (NULL == newMem) {
      if (freeOnFailure) {
         freechecks(to_free, file, line);
         if (to_free->needs_free)
            free(to_free->data - to_free->margin);
      }
      else
         smartalloc_track(ptr, to_free->space, to_free->needs_free, MARGIN);

      free(to_free);
      return NULL;
   }

   limit = newSize < to_free->space ? newSize : to_free->space;
   s = (char*)ptr;
   d = (char*)newMem;
   while (limit-- > 0)
      *d++ = *s++;

   freechecks(to_free, file, line);
   if (to_free->needs_free)
      free(to_free->data - to_free->margin);
   free(to_free);
   return newMem;
}

unsigned long report_space()
{
   return allocated;
}


#ifdef __cplusplus
}
#endif
