
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define MEMSIZE 10
#define LOOP 20

void *producer (void *args);
void *consumer (void *args);

typedef struct {
	int a_mem[MEMSIZE];
	int b_mem[MEMSIZE];

	pthread_mutex_t *a_mutex;
	pthread_mutex_t *b_mutex;

	int a_is_processing;
	int b_is_processing;
	int a_is_empty;
	int b_is_empty;

	pthread_cond_t *a_is_ready, *a_isnt_processing;
	pthread_cond_t *b_is_ready, *b_isnt_processing;
} queue;

queue *queueInit (void);
void queueDelete (queue *q);
void queueAdd (queue *q, int in);
void queueDel (queue *q, int *out);

int main ()
{
	queue *fifo;
	pthread_t pro, con;

	fifo = queueInit ();
	if (fifo ==  NULL) {
		fprintf (stderr, "main: Queue Init failed.\n");
		exit (1);
	}
	pthread_create (&pro, NULL, producer, fifo);
	pthread_create (&con, NULL, consumer, fifo);
	pthread_join (pro, NULL);
	pthread_join (con, NULL);
	queueDelete (fifo);

	return 0;
}

void *producer (void *q)
{
	queue *fifo;
	int i;

	fifo = (queue *)q;

	for (i = 0; i < LOOP; i++) {
		// Copy from FPGA to Memeory A
		pthread_mutex_lock (fifo->a_mutex);
		while (fifo->a_is_processing) {
			printf ("producer: memory A is processing.\n");
			pthread_cond_wait (fifo->a_isnt_processing, fifo->a_mutex);
		}
		// Copy data to A
		fifo->a_is_empty = 0;
		printf("producer: copying a\n");
		pthread_mutex_unlock (fifo->a_mutex);
		pthread_cond_signal (fifo->a_is_ready);

		// Copy from FPGA to Memeory B
		pthread_mutex_lock (fifo->b_mutex);
		while (fifo->b_is_processing) {
			printf ("producer: memory B is processing.\n");
			pthread_cond_wait (fifo->b_isnt_processing, fifo->b_mutex);
		}
		// Copy data to B
		fifo->b_is_empty = 0;
		printf("producer: copying b\n");
		pthread_mutex_unlock (fifo->b_mutex);
		pthread_cond_signal (fifo->b_is_ready);
	}
	return (NULL);
}

void *consumer (void *q)
{
	queue *fifo;
	int i, d;

	fifo = (queue *)q;

	for (i = 0; i < LOOP; i++) {
		// Copy from FPGA to Memeory A
		pthread_mutex_lock (fifo->a_mutex);
		while (fifo->a_is_empty) {
			printf ("consumer: memory A is empty, wait producer.\n");
			pthread_cond_wait (fifo->a_is_ready, fifo->a_mutex);
		}
		// Copy data to A
		fifo->a_is_processing = 1;

		printf("process A\n");
		//usleep(100);
		
		fifo->a_is_processing = 0;
		fifo->a_is_empty = 1;
		pthread_mutex_unlock (fifo->a_mutex);
		pthread_cond_signal (fifo->a_isnt_processing);

		// Copy from FPGA to Memeory B
		pthread_mutex_lock (fifo->b_mutex);
		while (fifo->b_is_empty) {
			printf ("consumer: memory B is empty, wait producer.\n");
			pthread_cond_wait (fifo->b_is_ready, fifo->b_mutex);
		}
		// Copy data to B
		fifo->b_is_processing = 1;
		printf("process B\n");
		//usleep(100);
		
		fifo->b_is_processing = 0;
		fifo->b_is_empty = 1;
		pthread_mutex_unlock (fifo->b_mutex);
		pthread_cond_signal (fifo->b_isnt_processing);
	}

	return (NULL);
}

queue *queueInit (void)
{
	queue *q;

	q = (queue *)malloc (sizeof (queue));
	if (q == NULL) return (NULL);

	q->a_is_processing = 0;
	q->b_is_processing = 0;
	q->a_is_empty = 1;
	q->b_is_empty = 1;

	q->a_mutex= (pthread_mutex_t *) malloc (sizeof (pthread_mutex_t));
	pthread_mutex_init (q->a_mutex, NULL);
	q->b_mutex= (pthread_mutex_t *) malloc (sizeof (pthread_mutex_t));
	pthread_mutex_init (q->b_mutex, NULL);

	q->a_is_ready= (pthread_cond_t *) malloc (sizeof (pthread_cond_t));
	pthread_cond_init (q->a_is_ready, NULL);
	q->b_is_ready= (pthread_cond_t *) malloc (sizeof (pthread_cond_t));
	pthread_cond_init (q->b_is_ready, NULL);
	q->a_isnt_processing= (pthread_cond_t *) malloc (sizeof (pthread_cond_t));
	pthread_cond_init (q->a_isnt_processing, NULL);
	q->b_isnt_processing= (pthread_cond_t *) malloc (sizeof (pthread_cond_t));
	pthread_cond_init (q->b_isnt_processing, NULL);
	
	return (q);
}

void queueDelete (queue *q)
{
	pthread_mutex_destroy (q->a_mutex);
	free (q->a_mutex);	
	pthread_mutex_destroy (q->b_mutex);
	free (q->b_mutex);	

	pthread_cond_destroy (q->a_is_ready);
	free (q->a_is_ready);
	pthread_cond_destroy (q->b_is_ready);
	free (q->b_is_ready);
	pthread_cond_destroy (q->a_isnt_processing);
	free (q->a_isnt_processing);
	pthread_cond_destroy (q->b_isnt_processing);
	free (q->b_isnt_processing);

	free (q);
}

