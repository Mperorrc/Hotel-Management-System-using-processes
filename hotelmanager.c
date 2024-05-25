#include<stdio.h>



#include<stdlib.h>



#include<unistd.h>



#include<sys/types.h>



#include<string.h>



#include<sys/wait.h>



#include<stdlib.h>



#include<sys/ipc.h>



#include<sys/shm.h>







#define BUF_SIZE 2000







int main(){



	



	FILE* file;



	file = fopen("earnings.txt", "a");



	if (file == NULL) {



		printf("Error opening the file earnings.txt\n");



		return 1;



	}



	



	int tables;



	printf("Enter the Total Number of Tables at the Hotel:");	



	scanf("%d",&tables);



	



	struct shmid_ds buf;



	key_t waiter_keys[tables], admin_key; 



	int shmids[tables], admin_shmid;



	int * shmptrs[tables], *admin_shmptr;	



	



	int running_tables[10]={0};



	int running_table_cnt=0;



	double total_earnings = 0;



	int terminating = 0;



	double table_earnings[10]={0};	



		



	// generate key



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



	



	for(int i=0;i<tables;i++){



		if((waiter_keys[i] = ftok("earnings.txt", (i+1)))==-1){ 



			fprintf(stderr,"Error in ftok\n");



			return 1;



		}



		shmids[i] = shmget(waiter_keys[i], BUF_SIZE, 0644 | IPC_CREAT) ;



		if(shmids[i]==-1)



		{



			fprintf(stderr,"Error in shmget in creating shared memory\n");



			return 1;



		}



		shmptrs[i] = shmat(shmids[i], NULL, 0);



		if(shmptrs[i]==(void*)-1)



		{



			fprintf(stderr,"Error in pointer to shared memory while attaching the memory segment\n");



			return 1;



		}



	}



	



	while(!terminating || running_table_cnt){



		



		if(admin_shmptr[0] == 1 && !terminating ){



			terminating = 1;



		}



		



		for(int i=0;i<tables;i++){



			if(shmptrs[i][1]==1){



				running_tables[i] = 1;



			}



			else if(shmptrs[i][1]==-1){



				running_tables[i] = 0;



				shmptrs[i][1]=0;



			}



			if(shmptrs[i][2]>0){



				total_earnings += shmptrs[i][2];



				table_earnings[i] += shmptrs[i][2];



				shmptrs[i][2]=0;



			}



			if(terminating==1){



				shmptrs[i][0]=-1000;



			}



			



		}



		running_table_cnt=0;



		for(int i=0;i<tables;i++){



			if(running_tables[i]){



				running_table_cnt++;



			}



		}



	}



	



	double waiter_earnings = 0.4*total_earnings;



	double profit = total_earnings - waiter_earnings;



	



	for(int i=0;i<tables;i++){



		if(table_earnings[i]>0){



			fprintf(file, "Earning from Table %d: %.2f INR.\n",i+1,table_earnings[i]);					



		}



	}



	fprintf(file, "Total earnings of Hotel: %.2f INR.\n",total_earnings);



				



	fprintf(file, "Total earnings of Waiters: %.2f INR\n", waiter_earnings);



	



	fprintf(file, "Total Profit: %.2f INR\n", profit);



	



	fclose(file);







	for(int i=0;i<tables;i++){



		if(shmdt(shmptrs[i])==-1)



		{



			fprintf(stderr,"Error in shmdt in detaching from the hotel memory segment with shmid %d\n", shmids[i]);



			return 1;



		}



		



		if(shmctl(shmids[i], IPC_RMID, 0)==-1)



		{



			fprintf(stderr,"Error in shmctl in removing the hotel shared memory segment\n");



			return 1;



		}



	}



	



	if(shmdt(admin_shmptr)==-1)



	{



		fprintf(stderr,"Error in shmdt in detaching from the hotel memory segment with shmid \n");



		return 1;



	}



	



	if(shmctl(admin_shmid, IPC_RMID, 0)==-1)



	{



		fprintf(stderr,"Error in shmctl in removing the hotel shared memory segment\n");



		return 1;



	}



	printf("Hotel shared meory segments deleted\n");	



	



	printf("Thank you for visiting the Hotel!\n");







}
