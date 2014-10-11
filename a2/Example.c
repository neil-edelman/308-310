#include <stdio.h>	
#include <semaphore.h> /* posix */

sem_t sem;	
int	count =	1;
int	pcount = 1;

void main()	{
	sem_init(&sem, 0, 2);

	printf("Wait semaphore %d \n", count++);	
	 	
		
	sem_wait(&sem);	
	 	
		
	printf("Wait semaphore %d \n", count++);	
	 	
		
	sem_wait(&sem);	/* wait, alloc */
	 	
	 	
	 	
	
	printf("Post semaphore %d \n", pcount++);	
	 	
		
	sem_post(&sem); /* signal, release */
	 	
	 	
		
	printf("Wait semaphore %d \n", count++);	
	 	
		
	sem_wait(&sem);	
}
