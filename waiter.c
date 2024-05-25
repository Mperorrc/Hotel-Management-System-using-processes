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



	



	// GET WAITER NUM



	



	int waiter_num;



	printf("Enter Waiter ID : ");	



	scanf("%d",&waiter_num);



	if(waiter_num<=0 || waiter_num>10){



		fprintf(stderr,"Invalid table number \n");



		return 1;



	}







	// GENERATE KEYS	



	



	struct shmid_ds buf;



	key_t table_key, hotel_key; 



	



	if((table_key = ftok("menu.txt", waiter_num))==-1){ 



		fprintf(stderr,"Error in table ftok\n");



		return 1;



	}



	



	if((hotel_key = ftok("earnings.txt", waiter_num))==-1){ 



		fprintf(stderr,"Error in hotel ftok\n");



		return 1;



	}



	



	int wt_shm_id, hotel_shm_id;



	int *wt_shm_ptr, *hotel_shm_ptr;



	int terminating = 0;



	int table_terminated = 0;



	



	// WAITER - TABLE SHM



	



	wt_shm_id = shmget(table_key, BUF_SIZE, 0644 | IPC_CREAT);



	if(wt_shm_id==-1)



	{



		fprintf(stderr,"Error in shmget in creating table shared memory\n");



		return 1;



	}



	



	wt_shm_ptr = shmat(wt_shm_id, NULL, 0);



	



	if(wt_shm_ptr==(void*)-1)



	{



		fprintf(stderr,"Error in shmPtr in attaching the table memory segment\n");



		return 1;



	}



	wt_shm_ptr[0]=-1000;



	



	// HOTEL - WAITER SHM	



	



	hotel_shm_id = shmget(hotel_key ,BUF_SIZE ,0644);



	if(hotel_shm_id==-1)



	{



		fprintf(stderr,"Error in shmget in accessing hotel shared memory\n");



		return 1;



	}



	



	hotel_shm_ptr = shmat(hotel_shm_id, NULL, 0);



	if(hotel_shm_ptr==(void*)-1)



	{



		fprintf(stderr,"Error in shmPtr in attaching the hotel memory segment\n");



		return 1;



	}



	int wrong_order_cnt=0;



	int response;



	hotel_shm_ptr[1] = 1;



	int itemcnt = 0;



	while(!table_terminated){



		



		while(wt_shm_ptr[0]!=1 && wt_shm_ptr[0]!=-1){



			continue;



		}



		response = wt_shm_ptr[0];



		hotel_shm_ptr[1]=wt_shm_ptr[0];



		wt_shm_ptr[0] = -1000;



		



		if(response == -1){



			break;



		}



		while(wt_shm_ptr[0]==-1000){



			continue;



		}



		int n=120;



		int idx = -1;



		int prices[4];



		int valid_order = 1;



		int bill_amount = 0;



		for(int i=n-1;i>0;i--){



			if(wt_shm_ptr[i]==-1000||idx>=0){



				idx++;



			}



			if(idx == 1){



				itemcnt = wt_shm_ptr[i];



			}



			else if(idx>1 && idx<=1+itemcnt){



				prices[idx-2] = wt_shm_ptr[i];



			}



			else if(idx>0){



				if(wt_shm_ptr[i]<=0||wt_shm_ptr[i]>itemcnt){



					valid_order = 0;



				}



				else{



					bill_amount += prices[wt_shm_ptr[i]-1];



				}



			}



			wt_shm_ptr[i]=0;



		}



		if(!bill_amount){



			valid_order = 0;



		}



		if(valid_order){



			printf("The total bill amount is %d INR.\n",bill_amount);



		}



		else{



			printf("Invalid. Retaking orders\n");



		}



		if(hotel_shm_ptr[0]==-1000){



			terminating =1;



		}



		int reseat_customers = 1;



		if(terminating == 1){



			reseat_customers = 0;



		}



		wt_shm_ptr[0] = -1000;



		wt_shm_ptr[1] = reseat_customers;



		wt_shm_ptr[2] = valid_order;



		wt_shm_ptr[3] = bill_amount;



		if(valid_order){



			hotel_shm_ptr[2] = bill_amount;



		}



	}



	



	printf("Table Terminated\n");	



		



	if(shmdt(wt_shm_ptr)==-1)



	{



		fprintf(stderr,"Error in shmdt in detaching from the table memory segment with shmid\n");



		return 1;



	}



	printf("Detached from table shm\n");



	



	if(shmdt(hotel_shm_ptr)==-1)



	{



		fprintf(stderr,"Error in shmdt in detaching from the Hotel memory segment with shmid\n");



		return 1;



	}



	



	printf("Detached from hotel shm\n");



	



	if(shmctl(wt_shm_id, IPC_RMID, 0)==-1)



	{



		fprintf(stderr,"Error in shmctl in removing the table shared memory segment\n");



		return 1;



	}



		



	printf("Waiter %d terminated\n", waiter_num);		



}
