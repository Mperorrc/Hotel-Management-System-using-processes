#include<stdio.h>



#include<stdlib.h>



#include<unistd.h>



#include<sys/types.h>



#include<string.h>



#include<sys/wait.h>



#include<stdlib.h>



#include<sys/ipc.h>



#include<sys/shm.h>







#define BUF_SIZE 2







int main(){







	char close_hotel = 'N';



	key_t admin_key;



	int admin_shmid;



	int *admin_shmptr;



	printf("The hotel is running\n");



	



	// Create token 



	if((admin_key = ftok("hotelmanager.c",'A'))==-1){



		fprintf(stderr,"Error in ftok\n");



		return 1;



	}



	// create hotel-admin shm



	



	admin_shmid = shmget(admin_key, BUF_SIZE, 0644 | IPC_CREAT);



	if(admin_shmid==-1)



	{



		fprintf(stderr,"Error in shmget in creating hotel - admin shared memory\n");



		return 1;



	}



	admin_shmptr = shmat(admin_shmid, NULL, 0);



	if(admin_shmptr==(void*)-1)



	{



		fprintf(stderr,"Error in pointer to shared memory while attaching the hotel - admin memory segment\n");



		return 1;



	}



	



	while(close_hotel=='N'||close_hotel=='n'){



		printf("Do you want to close the hotel? Enter Y for Yes and N for No.\n");



		scanf("%s",&close_hotel);



		if(!(close_hotel=='Y'||close_hotel=='y'||close_hotel=='N'||close_hotel=='n')){



			printf("Invalid input\n");



			close_hotel = 'n';



		}



	}



	



	admin_shmptr[0] = 1;



	



	if(shmdt(admin_shmptr)==-1)



	{



		fprintf(stderr,"Error in shmdt in detaching from the hotel memory segment with shmid\n");



		return 1;



	}



	



	printf("The hotel will be closed soon.\n");



}
