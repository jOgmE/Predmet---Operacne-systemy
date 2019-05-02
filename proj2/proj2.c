//proj2.c
//IOS second project
//Norbert Pocs (xpocsn00)
//VUT FIT
//2019 apr.

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>

//6 + name of the program
#define ARGS_NUM 7
#define shmSC "/proj2-shm-SC"
#define shmHC "/proj2-shm-HC"
#define shmPIER "/proj2-shm-pier"
#define shmA "/proj2-shm-a"
#define shmUBC "/proj2-shm-UBC"
#define shmBC "/proj2-shm-BC"
#define semSHMUT "/proj2-mut-sh"
#define semBAMUT "/proj2-mut-boat"
#define semPMUT "/proj2-mut-pier"
#define semHQ "/proj2-sem-hq"
#define semSQ "/proj2-sem-sq"
#define semBP "/proj2-sem-bar-pier"
#define semBB "/proj2-sem-bar-board"
#define semUB "/proj2-sem-unboard"
#define semCAP "/proj2-sem-cap"


//semaphores struct
typedef struct{
	sem_t *shMutex;
	sem_t *pierMutex;
	sem_t *boatMutex;
	sem_t *h_que;
	sem_t *s_que;
	sem_t *bar_pier;
	sem_t *bar_board;
	sem_t *unboard;
	sem_t *captain;
}semaph_t;
//arguments struct
typedef struct{
	unsigned p;
	unsigned h;
	unsigned s;
	unsigned r;
	unsigned w;
	unsigned c;	
	FILE *output;
}argus_t;

void unlink_sem(){
	shm_unlink(shmSC);
	shm_unlink(shmHC);
	shm_unlink(shmPIER);
	shm_unlink(shmA);
	shm_unlink(shmUBC);
	shm_unlink(shmBC);
	sem_unlink(semSHMUT);
	sem_unlink(semBAMUT);
	sem_unlink(semPMUT);
	sem_unlink(semHQ);
	sem_unlink(semSQ);
	sem_unlink(semBP);
	sem_unlink(semBB);
	sem_unlink(semUB);
	sem_unlink(semCAP);
}

void close_sem(semaph_t *semaphsDE){
	sem_close(semaphsDE->shMutex);
	sem_close(semaphsDE->boatMutex);
	sem_close(semaphsDE->pierMutex);
	sem_close(semaphsDE->h_que);
	sem_close(semaphsDE->s_que);
	sem_close(semaphsDE->bar_pier);
	sem_close(semaphsDE->bar_board);
	sem_close(semaphsDE->unboard);
}

void sail(unsigned time){
	usleep((rand() %time)*1000);
}

//typeof 1 = hack
//typeof 0 = serf
void river_crossing(semaph_t *semaphsCS, argus_t *argumentsCS, int myId,_Bool type){
	//choosing which type of person is using this function
	const char *name = type ? "HACK" : "SERF";
	char isCaptain = 0;
	//----OPENING SHARED MEMORY VARIABLES----
	//opening s counter
	int shm_id = shm_open(shmSC, O_RDWR,S_IRUSR|S_IWUSR);
	if(shm_id == -1){
		fprintf(stderr,"crossing %s s_count: %s\n",name,strerror(errno));
		close_sem(semaphsCS);
		unlink_sem();
		exit(-1);
	}
	int *sh_s_count = (int*)mmap(NULL,sizeof(int),PROT_READ|PROT_WRITE,MAP_SHARED,shm_id,0);
	close(shm_id);
	//opening h counter
	shm_id = shm_open(shmHC, O_RDWR,S_IRUSR|S_IWUSR);
	if(shm_id == -1){
		fprintf(stderr,"crossing %s h_count: %s\n",name,strerror(errno));
		close_sem(semaphsCS);
		unlink_sem();
		munmap(sh_s_count,sizeof(int));
		exit(-1);
	}
	int *sh_h_count = (int*)mmap(NULL,sizeof(int),PROT_READ|PROT_WRITE,MAP_SHARED,shm_id,0);
	close(shm_id);
	//opening pier
	shm_id = shm_open(shmPIER, O_RDWR,S_IRUSR|S_IWUSR);
	if(shm_id == -1){
		fprintf(stderr,"crossing %s sh_pier: %s\n",name,strerror(errno));
		close_sem(semaphsCS);
		unlink_sem();
		munmap(sh_h_count,sizeof(int));
		munmap(sh_s_count,sizeof(int));
		exit(-1);
	}
	unsigned *sh_pier = (unsigned*)mmap(NULL,sizeof(unsigned),PROT_READ|PROT_WRITE,MAP_SHARED,shm_id,0);
	close(shm_id);
	//opening A
	shm_id = shm_open(shmA, O_RDWR,S_IRUSR|S_IWUSR);
	if(shm_id == -1){
		fprintf(stderr,"crossing %s sh_A: %s\n",name,strerror(errno));
		close_sem(semaphsCS);
		unlink_sem();
		munmap(sh_h_count,sizeof(int));
		munmap(sh_s_count,sizeof(int));
		munmap(sh_pier,sizeof(unsigned));
		exit(-1);
	}
	unsigned *sh_A = (unsigned*)mmap(NULL,sizeof(unsigned),PROT_READ|PROT_WRITE,MAP_SHARED,shm_id,0);
	close(shm_id);
	//opening unboard counter
	shm_id = shm_open(shmUBC, O_RDWR,S_IRUSR|S_IWUSR);
	if(shm_id == -1){
		fprintf(stderr,"crossing %s sh_UBC: %s\n",name,strerror(errno));
		close_sem(semaphsCS);
		unlink_sem();
		munmap(sh_h_count,sizeof(int));
		munmap(sh_s_count,sizeof(int));
		munmap(sh_pier,sizeof(unsigned));
		munmap(sh_A,sizeof(unsigned));
		exit(-1);
	}
	unsigned *sh_UBC = (unsigned*)mmap(NULL,sizeof(unsigned),PROT_READ|PROT_WRITE,MAP_SHARED,shm_id,0);
	close(shm_id);
	//opening board counter
	shm_id = shm_open(shmBC, O_RDWR,S_IRUSR|S_IWUSR);
	if(shm_id == -1){
		fprintf(stderr,"crossing %s sh_BC: %s\n",name,strerror(errno));
		close_sem(semaphsCS);
		unlink_sem();
		munmap(sh_h_count,sizeof(int));
		munmap(sh_s_count,sizeof(int));
		munmap(sh_pier,sizeof(unsigned));
		munmap(sh_A,sizeof(unsigned));
		exit(-1);
	}
	unsigned *sh_BC = (unsigned*)mmap(NULL,sizeof(unsigned),PROT_READ|PROT_WRITE,MAP_SHARED,shm_id,0);
	close(shm_id);
	//----end of OPENING SHARED MEMORY VARIABLES----

	//----PIER START----
	while(1){
		//mutex start
		sem_wait(semaphsCS->shMutex);
		if((unsigned)(*sh_h_count + *sh_s_count) >= argumentsCS->c){
			//mutex end
			sem_post(semaphsCS->shMutex);
			int rand_sleep = argumentsCS->w == 20 ? 20 :(rand() % (argumentsCS->w-20))+20;
			//posting output
			sem_wait(semaphsCS->shMutex);
				argumentsCS->output = fopen("proj2.out","a");
				fprintf(argumentsCS->output,"%u: %s %i: leaves queue: %i: %i\n",(*sh_A)++,name,myId,*sh_h_count,*sh_s_count);
				fclose(argumentsCS->output);
			sem_post(semaphsCS->shMutex);

			//sleep before new try
			usleep(rand_sleep*1000);

			//posting output
			sem_wait(semaphsCS->shMutex);
				argumentsCS->output = fopen("proj2.out","a");
				fprintf(argumentsCS->output,"%u: %s %i: is back\n",(*sh_A)++,name,myId);
				fclose(argumentsCS->output);
			sem_post(semaphsCS->shMutex);
			
			continue;
		}
		else{
			sem_post(semaphsCS->shMutex);
			break;
		}
	}
	//----PIER END----

	//MUTEX START
	sem_wait(semaphsCS->shMutex);
	//----ARRIVING TO THE PIER----
	if(type){
		*sh_h_count = *sh_h_count +1;
	}else{
		*sh_s_count = *sh_s_count +1;
	}
	//posting output
		argumentsCS->output = fopen("proj2.out","a");
		fprintf(argumentsCS->output,"%u: %s %i: waits: %i: %i\n",(*sh_A)++,name,myId,*sh_h_count,*sh_s_count);
		fclose(argumentsCS->output);
	sem_post(semaphsCS->shMutex);

	sem_wait(semaphsCS->pierMutex);
	sem_wait(semaphsCS->shMutex);
	if((!type) && *sh_s_count >= 4){
		sem_post(semaphsCS->shMutex);
		sem_wait(semaphsCS->boatMutex);

		//sem_post(semaphsCS->s_que);
		sem_post(semaphsCS->s_que);
		sem_post(semaphsCS->s_que);
		sem_post(semaphsCS->s_que);

		sem_wait(semaphsCS->shMutex);
		*sh_s_count = *sh_s_count -4;
		sem_post(semaphsCS->shMutex);

		isCaptain = 1;
	}
	else if(type && *sh_h_count >= 4){
		sem_post(semaphsCS->shMutex);
		sem_wait(semaphsCS->boatMutex);

		//sem_post(semaphsCS->h_que);
		sem_post(semaphsCS->h_que);
		sem_post(semaphsCS->h_que);
		sem_post(semaphsCS->h_que);

		sem_wait(semaphsCS->shMutex);
		*sh_h_count = *sh_h_count -4;
		sem_post(semaphsCS->shMutex);

		isCaptain = 1;
	}
	else if(*sh_s_count == 2 && *sh_h_count >= 2){ //both >= ??
		sem_post(semaphsCS->shMutex);
		sem_wait(semaphsCS->boatMutex);

		sem_post(semaphsCS->h_que);
		sem_post(semaphsCS->s_que);
		if(type){
			//i am hacker
			sem_post(semaphsCS->s_que);
		}else{
			//i am serf
			sem_post(semaphsCS->h_que);
		}

		sem_wait(semaphsCS->shMutex);
		*sh_h_count = *sh_h_count -2;
		*sh_s_count = *sh_s_count -2;
		sem_post(semaphsCS->shMutex);

		isCaptain = 1;
	}
	else if(*sh_h_count == 2 && *sh_s_count >= 2){ //both >= ??
		sem_post(semaphsCS->shMutex);
		sem_wait(semaphsCS->boatMutex);

		sem_post(semaphsCS->h_que);
		sem_post(semaphsCS->s_que);
		if(type){
			//i am hacker
			sem_post(semaphsCS->s_que);
		}else{
			//i am serf
			sem_post(semaphsCS->h_que);
		}

		sem_wait(semaphsCS->shMutex);
		*sh_h_count = *sh_h_count -2;
		*sh_s_count = *sh_s_count -2;
		sem_post(semaphsCS->shMutex);

		isCaptain = 1;
	}
	else{
		//MUTEX END
		sem_post(semaphsCS->pierMutex);
		sem_post(semaphsCS->shMutex);	
		//----QUEUE----
		if(type){
			sem_wait(semaphsCS->h_que);
		}else{
			sem_wait(semaphsCS->s_que);
		}
		//----end of QUEUE----
	}

	//----boarding throught BARRIER START----
	sem_wait(semaphsCS->shMutex);
	*sh_BC = *sh_BC +1;
	if(*sh_BC >= 4){
		*sh_BC = 0;
		sem_post(semaphsCS->bar_board);
	}
	sem_post(semaphsCS->shMutex);

	sem_wait(semaphsCS->bar_board);
	sem_post(semaphsCS->bar_board);
	//----boarding throught BARRIER END----

	if(isCaptain){
		//releasing the pier mutex
		sem_post(semaphsCS->pierMutex);
		//posting output
		sem_wait(semaphsCS->shMutex);
			argumentsCS->output = fopen("proj2.out","a");
			fprintf(argumentsCS->output,"%u: %s %i: boards: %i: %i\n",(*sh_A)++,name,myId,*sh_h_count,*sh_s_count);
			fclose(argumentsCS->output);
		sem_post(semaphsCS->shMutex);
		//rowBoat
		sail(argumentsCS->r);
		sem_post(semaphsCS->unboard);
		sem_wait(semaphsCS->captain);
		//posting output
		sem_wait(semaphsCS->shMutex);
			argumentsCS->output = fopen("proj2.out","a");
			fprintf(argumentsCS->output,"%u: %s %i: captain exits: %i: %i\n",(*sh_A)++,name,myId,*sh_h_count,*sh_s_count);
			fclose(argumentsCS->output);
		sem_post(semaphsCS->shMutex);
		//MUTEX END
		sem_post(semaphsCS->boatMutex);
	}else{
		//wait for the end of the sailing then unboard
		sem_wait(semaphsCS->unboard);
		//posting output
		sem_wait(semaphsCS->shMutex);
			argumentsCS->output = fopen("proj2.out","a");
			fprintf(argumentsCS->output,"%u: %s %i: member exits: %i: %i\n",(*sh_A)++,name,myId,*sh_h_count,*sh_s_count);
			fclose(argumentsCS->output);
		*sh_UBC = *sh_UBC +1;
		if(*sh_UBC >= 3){
			*sh_UBC = 0;
			sem_post(semaphsCS->captain);
			sem_post(semaphsCS->shMutex);
		}
		else{
			sem_post(semaphsCS->shMutex);
			sem_post(semaphsCS->unboard);
		}
	}

	munmap(sh_h_count,sizeof(int));	
	munmap(sh_s_count,sizeof(int));	
	munmap(sh_pier,sizeof(unsigned));
	munmap(sh_A,sizeof(unsigned));
	munmap(sh_UBC,sizeof(int));
}

int generator(argus_t *argumentsG,semaph_t *semaphsG){
	//opening A
	int shm_id = shm_open(shmA, O_RDWR,S_IRUSR|S_IWUSR);
	if(shm_id == -1){
		fprintf(stderr,"crossing_hackers: %s\n",strerror(errno));
		close_sem(semaphsG);
		unlink_sem();
		exit(-1);
	}
	unsigned *sh_A = (unsigned*)mmap(NULL,sizeof(unsigned),PROT_READ|PROT_WRITE,MAP_SHARED,shm_id,0);
	close(shm_id);

	if(fork() == 0){ //generator process for hackers
		for(unsigned i = 0; i<argumentsG->p;i++){
			//wait to generate another process
			if(argumentsG->h != 0){
				usleep((rand() % argumentsG->h)*1000);
			}

			if(fork() == 0){ //children process
				int myId = i+1;
				sem_wait(semaphsG->shMutex);
					argumentsG->output = fopen("proj2.out","a");
					fprintf(argumentsG->output,"%u: HACK %i: starts\n",(*sh_A)++,myId); //output
					fclose(argumentsG->output);
				sem_post(semaphsG->shMutex);
				
				river_crossing(semaphsG,argumentsG,myId,1);
				
				//detaching shared vars
				close_sem(semaphsG);
				exit(0);
			}
		}
		for(unsigned i = 0; i<argumentsG->p;i++){
			wait(NULL);
		}
		exit(0);
	}else{ //remaining at the parent process
		if(fork() == 0){ //generator process for serfs
			for(unsigned i = 0; i<argumentsG->p;i++){
				//wait to generate another process
				if(argumentsG->s != 0){
					usleep((rand() % argumentsG->s)*1000);
				}

				if(fork() == 0){ //children
					int myId = i+1;
					sem_wait(semaphsG->shMutex);
						argumentsG->output = fopen("proj2.out","a");
						fprintf(argumentsG->output,"%u: SERF %i: starts\n",(*sh_A)++,myId);
						fclose(argumentsG->output);
					sem_post(semaphsG->shMutex);
					
					river_crossing(semaphsG,argumentsG,myId,0);
	
					//detaching shared vars
					close_sem(semaphsG);
					
					exit(0);
				}
			}
			for(unsigned i = 0; i<argumentsG->p;i++){
				wait(NULL);
			}
			exit(0);
		}
	}
	wait(NULL);
	munmap(sh_A,sizeof(unsigned));

	return 0;
}

int main(int argc, char *argv[]){
	//reading arguments
	argus_t arguments;
	//opening and clearing the output file
	arguments.output = fopen("proj2.out","w+");
	fclose(arguments.output);

	if(arguments.output == NULL){
		fprintf(stderr,"main: Opening the file proj2.out; %s\n",strerror(errno));
		exit(-1);
	}
	
	if(argc != 7 || sscanf(argv[1],"%u ",&arguments.p) == EOF || sscanf(argv[2],"%u ",&arguments.h) == EOF || sscanf(argv[3],"%u ",&arguments.s) == EOF || sscanf(argv[4],"%u ",&arguments.r) == EOF || sscanf(argv[5],"%u ",&arguments.w) == EOF || sscanf(argv[6],"%u",&arguments.c) == EOF){
		fprintf(stderr,"main: Wrong argument format given\n");
		exit(1);
	}
	//remake!!
	if((arguments.p >= 2) && (arguments.p % 2 == 0) && (arguments.h <= 2000) && (arguments.s <= 2000) && (arguments.r <= 2000) && (arguments.w >= 20) && (arguments.w <= 2000) && (arguments.c >= 5)){} //P H S R W C
	//bad arguments
	else{
		fprintf(stderr, "main: Wrong arguments\n");
		exit(1);
	}
	
	//shared variables
	//opening h_counter
	int shm_id = shm_open(shmHC, O_CREAT|O_EXCL|O_RDWR,S_IRUSR|S_IWUSR);
	if(shm_id == -1){
		unlink_sem();
		exit(-1);
	}
	ftruncate(shm_id,sizeof(int)*2);
	int *sh_h_count = (int*)mmap(NULL,sizeof(int),PROT_READ|PROT_WRITE,MAP_SHARED,shm_id,0);
	close(shm_id);

	//opening s_counter
	shm_id = shm_open(shmSC, O_CREAT|O_EXCL|O_RDWR,S_IRUSR|S_IWUSR);
	if(shm_id == -1){
		fprintf(stderr,"main: %s\n",strerror(errno));
		munmap(sh_h_count,sizeof(int));
		unlink_sem();
		exit(-1);
	}
	ftruncate(shm_id,sizeof(int)*2);
	int *sh_s_count = (int*)mmap(NULL,sizeof(int),PROT_READ|PROT_WRITE,MAP_SHARED,shm_id,0);
	close(shm_id);
	//opening pier
	shm_id = shm_open(shmPIER, O_CREAT|O_EXCL|O_RDWR,S_IRUSR|S_IWUSR);
	if(shm_id == -1){
		fprintf(stderr,"main: %s\n",strerror(errno));
		unlink_sem();
		munmap(sh_h_count,sizeof(int));
		munmap(sh_s_count,sizeof(int));
		exit(-1);
	}
	ftruncate(shm_id,sizeof(int)*2);
	unsigned *sh_pier = (unsigned*)mmap(NULL,sizeof(unsigned),PROT_READ|PROT_WRITE,MAP_SHARED,shm_id,0);
	close(shm_id);
	//opening A
	shm_id = shm_open(shmA, O_CREAT|O_EXCL|O_RDWR,S_IRUSR|S_IWUSR);
	if(shm_id == -1){
		fprintf(stderr,"main: %s\n",strerror(errno));
		unlink_sem();
		munmap(sh_h_count,sizeof(int));
		munmap(sh_s_count,sizeof(int));
		munmap(sh_pier,sizeof(unsigned));
		exit(-1);
	}
	ftruncate(shm_id,sizeof(int)*2);
	unsigned *sh_A = (unsigned*)mmap(NULL,sizeof(unsigned),PROT_READ|PROT_WRITE,MAP_SHARED,shm_id,0);
	close(shm_id);
	//opening unboarding counter
	shm_id = shm_open(shmUBC, O_CREAT|O_EXCL|O_RDWR,S_IRUSR|S_IWUSR);
	if(shm_id == -1){
		fprintf(stderr,"main: %s\n",strerror(errno));
		unlink_sem();
		munmap(sh_h_count,sizeof(int));
		munmap(sh_s_count,sizeof(int));
		munmap(sh_pier,sizeof(unsigned));
		munmap(sh_A,sizeof(unsigned));
		exit(-1);
	}
	ftruncate(shm_id,sizeof(int)*2);
	int *sh_UBC = (int*)mmap(NULL,sizeof(int),PROT_READ|PROT_WRITE,MAP_SHARED,shm_id,0);
	close(shm_id);
	//opening boarding counter
	shm_id = shm_open(shmBC, O_CREAT|O_EXCL|O_RDWR,S_IRUSR|S_IWUSR);
	if(shm_id == -1){
		fprintf(stderr,"main: %s\n",strerror(errno));
		unlink_sem();
		munmap(sh_h_count,sizeof(int));
		munmap(sh_s_count,sizeof(int));
		munmap(sh_pier,sizeof(unsigned));
		munmap(sh_A,sizeof(unsigned));
		munmap(sh_UBC,sizeof(unsigned));
		exit(-1);
	}
	ftruncate(shm_id,sizeof(int)*2);
	int *sh_BC = (int*)mmap(NULL,sizeof(int),PROT_READ|PROT_WRITE,MAP_SHARED,shm_id,0);
	close(shm_id);

	*sh_h_count = 0;
	*sh_s_count = 0;
	*sh_pier = 0;
	*sh_A = 1;
	*sh_UBC = 0;
	*sh_BC = 0;
	munmap(sh_h_count,sizeof(int));
	munmap(sh_s_count,sizeof(int));
	munmap(sh_pier,sizeof(unsigned));
	munmap(sh_A,sizeof(unsigned));
	munmap(sh_UBC,sizeof(int));

	semaph_t semaphs;

	//opening mutexes
	if((semaphs.pierMutex = sem_open(semPMUT, O_CREAT|O_EXCL, 0666, 1)) == SEM_FAILED){
		fprintf(stderr,"main: %s\n",strerror(errno));
		unlink_sem();
		exit(-1);
	}
	if((semaphs.shMutex = sem_open(semSHMUT, O_CREAT|O_EXCL, 0666, 1)) == SEM_FAILED){
		fprintf(stderr,"main: %s\n",strerror(errno));
		unlink_sem();
		exit(-1);
	}
	if((semaphs.boatMutex = sem_open(semBAMUT, O_CREAT|O_EXCL, 0666, 1)) == SEM_FAILED){
		fprintf(stderr,"main: %s\n",strerror(errno));
		unlink_sem();
		exit(-1);
	}
	//opening semaphores
	if((semaphs.h_que = sem_open(semHQ, O_CREAT|O_EXCL, 0666, 0)) == SEM_FAILED){
		fprintf(stderr,"main: %s\n",strerror(errno));
		unlink_sem();
		exit(-1);
	}
	if((semaphs.s_que = sem_open(semSQ, O_CREAT|O_EXCL, 0666, 0)) == SEM_FAILED){
		fprintf(stderr,"main: %s\n",strerror(errno));
		unlink_sem();
		exit(-1);
	}
	if((semaphs.bar_pier = sem_open(semBP, O_CREAT|O_EXCL, 0666, 0)) == SEM_FAILED){
		fprintf(stderr,"main: %s\n",strerror(errno));
		unlink_sem();
		exit(-1);
	}
	if((semaphs.bar_board = sem_open(semBB, O_CREAT|O_EXCL, 0666, 0)) == SEM_FAILED){
		fprintf(stderr,"main: %s\n",strerror(errno));
		unlink_sem();
		exit(-1);
	}
	if((semaphs.unboard = sem_open(semUB, O_CREAT|O_EXCL, 0666, 0)) == SEM_FAILED){
		fprintf(stderr,"main: %s\n",strerror(errno));
		unlink_sem();
		exit(-1);
	}
	if((semaphs.captain = sem_open(semCAP, O_CREAT|O_EXCL, 0666, 0)) == SEM_FAILED){
		fprintf(stderr,"main: %s\n",strerror(errno));
		unlink_sem();
		exit(-1);
	}

	generator(&arguments,&semaphs);
	
	wait(NULL);
	close_sem(&semaphs);
	unlink_sem();
	munmap(sh_h_count,sizeof(int));
	munmap(sh_s_count,sizeof(int));
	munmap(sh_pier,sizeof(int));
	munmap(sh_A,sizeof(unsigned));
	munmap(sh_UBC,sizeof(int));
	munmap(sh_BC,sizeof(int));

	return 0;
}
