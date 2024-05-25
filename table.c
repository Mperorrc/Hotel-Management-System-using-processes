#include<stdio.h>



#include<stdlib.h>



#include<unistd.h>



#include<sys/types.h>



#include<string.h>



#include<sys/wait.h>



#include<stdlib.h>



#include<sys/ipc.h>



#include<sys/shm.h>



#include<stdbool.h>







#define PIPE_BUFFER_SIZE 210



#define BUF_SIZE 2000



#define MAX_LINE_SIZE 40



#define READ_END 0



#define WRITE_END 1







int main(){



	



	// TABLE CREATION



		



	int table_number;



	printf("Please Enter the Table Number : ");



	scanf("%d", &table_number);



	if(table_number<=0 || table_number>10){



		fprintf(stderr,"Invalid table number \n");



		return 1;



	}



	



	// KEY GENERATION FOR WAITER_TABLE SHM



	



	key_t key; 



	if((key = ftok("menu.txt", table_number))==-1){ 



		fprintf(stderr,"Error in ftok\n");



		return 1;



	}



	



	int shmid = -1;



	int *shmptr;



	



	int prices[9];



	



	// READING THE FILE



	



	FILE *menu_file;



	menu_file = fopen("menu.txt", "r");



	if(menu_file == NULL){



		fprintf(stderr,"Failed to open menu\n");



		return 1;



	}



	



	char file_line[MAX_LINE_SIZE + 1]; 



	char *file_contents = NULL;



	size_t file_size = 0;



	int tempidx=0;



	



	while (fgets(file_line, sizeof(file_line), menu_file) != NULL) {



	



		size_t line_length = strlen(file_line);



		char *temp = realloc(file_contents, file_size + line_length + 1); 



		



		if (temp == NULL) {



		    fprintf(stderr, "Allocation failed. Couldn't read menu\n");



		    free(file_contents);



		    fclose(menu_file);



		    return 1;



		}







		file_contents = temp;



		strcat(file_contents, file_line);



		file_size += line_length;



		



		int word_cnt=0;



		int price=0;



		for(int i=0;i<line_length;i++){



			if(word_cnt==3&&file_line[i]==' '){



				break;



			}



			if(word_cnt==3){



				price = price*10 + (file_line[i]-'0');



			}



			if(file_line[i]==' '){



				word_cnt++;



			}



		}



		prices[tempidx++]=price;



		



		if (line_length == MAX_LINE_SIZE && file_line[MAX_LINE_SIZE - 1] != '\n') {



		    fprintf(stderr, "Line too long.\n");



		    free(file_contents);



		    fclose(menu_file);



		    return 1;



		}



	}



	



	fclose(menu_file);



	



	pid_t fork_value = 1;                           



	int reseat_customers = 1;                     // in case of when admin says hotel must close, dont take in any new customers



	int valid_orders = 1;



	int no_of_items = tempidx;



	int num_customers; 



	// int reorder_cnt = 0;



	



	int write_msg[PIPE_BUFFER_SIZE]={0};



	int read_msg[5*PIPE_BUFFER_SIZE]={0};



	write_msg[0] = 1;	



	read_msg[0]=-1;



	int cid = -1;



	



	while((valid_orders == 0 || reseat_customers) && fork_value>0){



		



		// NUMBER OF CUSTOMERS TO BE SEATED



		



		if(valid_orders){



			printf("Enter Number of Customers at Table (maximum no. of customers can be 5) : ");



			scanf("%d", &num_customers);



			if(num_customers == -1){



				break;



			}



		}



		if(num_customers <=0 || num_customers>5){



			printf("Invalid number of customers entered.\n");



			continue; 



		}



		



		// CREATION OF PIPES



		



		printf("      MENU\n%s",file_contents);



		int child_to_parent[num_customers][2];



		



		for(int i=0;i<num_customers;i++){



			if( pipe(child_to_parent[i]) == -1){



				fprintf(stderr,"Pipe creation failed\n");



				return 1;



			}



		}



		



		// RESETING WRITE AND READ MSG ARRAYS



			



		for(int i=0;i<PIPE_BUFFER_SIZE;i++){



			write_msg[i]=0;



		}



		for(int i=0;i<5*PIPE_BUFFER_SIZE;i++){



			read_msg[i]=0;



		}



		write_msg[0] = 1;	



		read_msg[0]=-1;



		



		// CREATION OF CUSTOMER PROCESS



				



		for(int i=0;i<num_customers && fork_value;i++){



			



			cid = i;



			



			fork_value = fork();



			if(fork_value < 0){



				fprintf(stderr,"Fork Failed\n");



				return 1;



			}	



		}



		



		



		



		if(fork_value > 0){



			



			int idx=0;



			if(shmid==-1){



				shmid = shmget(key, BUF_SIZE, 0644);



				if(shmid==-1)



				{



					fprintf(stderr,"Error in shmget in creating table shared memory\n");



					return 1;



				}



				



				shmptr = shmat(shmid, NULL, 0);



				if(shmptr==(void*)-1)



				{



					fprintf(stderr,"Error in shmPtr in attaching the table memory segment\n");



					return 1;



				}



			}



			shmptr[0] = 1;



			



			// GET ORDERS FROM CUSTOMERS



			for(int i=0;i<num_customers;i++){



				bool getting_order = true;



				int orders[PIPE_BUFFER_SIZE]={0};



				close(child_to_parent[i][WRITE_END]);



				while(getting_order){



					read(child_to_parent[i][READ_END],orders,PIPE_BUFFER_SIZE);



					if(orders[0]==1){



						getting_order = false;



					}



				}	



				close(child_to_parent[i][READ_END]);



								



				for(int j=1;j<30 && orders[j]!=-2000;j++){



					read_msg[idx++] = orders[j];



				}



			}



			



			// SEND THE ORDER TO WAITER



			printf("Final orders : ");



			int n = BUF_SIZE/sizeof(shmptr[0]);



			for(int i=0;i<idx;i++){



				shmptr[i+1]=read_msg[i];



				printf("%d ", read_msg[i]);



			}



			printf("\n");



			for(int i=1;i<=no_of_items;i++){



				shmptr[idx+i] = prices[no_of_items-i];



			}



			shmptr[idx+no_of_items+1] = no_of_items;



			shmptr[idx+no_of_items+2] = -1000; 



			shmptr[0]=0;



			



			sleep(5);



			



			// GET INFO FROM WAITER



			int bill_val[20] = {0};



			bill_val[0] = shmptr[1];         // RESEAT_CUSTOMER VARIABLE



			bill_val[1] = shmptr[2];         // VALID_ORDER VARIABLE



			bill_val[2] = shmptr[3];	 // BILL AMOUNT



			



			valid_orders = bill_val[1];



			reseat_customers = bill_val[0];



			



			if(valid_orders == 1){



				printf("The total bill amount is %d INR.\n",bill_val[2]);



			}



			else{



				printf("Invalid Orders, retaking orders\n");



			}



			



			for(int i=0;i<num_customers;i++){



				wait(NULL);



			}



		}



		else{



			sleep(cid*15);



			printf("Enter the serial number(s) of the item(s) to order from the menu. Enter -1 when done : \n");



			



			int orders[20] = {0};



			int sz = 0;



			



			// GET ORDERS FROM USER



			



			while(sz<10){



				int order;



				scanf("%d",&order);



				if(order==-1){



					break;



				}



				orders[sz]=order;



				sz++;



			}



			orders[sz]=-2000;



			sz++;



			if(sz==10){



				printf("Max ten orders at once per customer\n");



			}



			printf("Orders given by customer %d were ", getpid());



			



			for(int i=0;i<30;i++){



				write_msg[i+1]=orders[i];



				if(orders[i]!=-2000){



					printf("%d ",orders[i]);



				}



				else{



					break;



				}



			}



			printf("\n");



			



			// SEND ORDERS TO PARENT



			



			close(child_to_parent[cid][READ_END]);



			write(child_to_parent[cid][WRITE_END],write_msg,50);



			close(child_to_parent[cid][WRITE_END]);



			



			sleep((num_customers-1-cid)*15);



			



			sleep(20);



			



			printf("customer %d terminated\n", getpid());



		}



		



	}



	if(fork_value>0&&reseat_customers==0){



		printf("The hotel is closing and shall be taking no more customers.\n");



	}



	



	if(shmid==-1){



		if(shmid==-1){



			shmid = shmget(key, BUF_SIZE, 0644);



			if(shmid==-1)



			{



				fprintf(stderr,"Error in shmget in creating table shared memory\n");



				return 1;



			}



			



			shmptr = shmat(shmid, NULL, 0);



			if(shmptr==(void*)-1)



			{



				fprintf(stderr,"Error in shmPtr in attaching the table memory segment\n");



				return 1;



			}



		}



	}



	if(fork_value>0){



		shmptr[0] = -1;



			if(shmdt(shmptr)==-1)



		{



			fprintf(stderr,"Error in shmdt in detaching from the Hotel memory segment with shmid\n");



			return 1;



		}



		



		printf("Detached from hotel shm\n");



		printf("Table %d terminated\n", table_number);



	}



	



}



		
