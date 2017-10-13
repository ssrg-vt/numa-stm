#include "tm/tm_thread.hpp"
#include <pthread.h>
#include <signal.h>
#include <pthread.h>

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#include "tm/rand_r_32.h"

#include <errno.h>

int total_threads;
/**
 *  Support a few lightweight barriers
 */
void
barrier(uint32_t which)
{
    static volatile uint32_t barriers[16] = {0};
    CFENCE;
    __sync_fetch_and_add(&barriers[which], 1);
    while (barriers[which] != total_threads) { }
    CFENCE;
}

int myid;
void
signal_callback_handler(int signum)
{
   printlogTM(myid);
   // Terminate program
   exit(signum);
}

bool ExperimentInProgress = true;
static void catch_SIGALRM(int sig_num)
{
    ExperimentInProgress = false;
}

unsigned long long throughputs[64];

//=======================TPC-C=====================================


#define  NUM_ITEMS  50
//1000; // Correct overall # of items: 100,000
#define  NUM_WAREHOUSES  20
#define  NUM_DISTRICTS  50
//10000; //4;
#define  NUM_CUSTOMERS_PER_D  50
//300; //30;
#define  NUM_ORDERS_PER_D  50

#define  NUM_LINES_PER_O  5
//3000; //30;
#define  MAX_CUSTOMER_NAMES  50

#define  NUM_HISTORY_PER_C 10
//1000; // 10;

#define  DEFAULT_LENGTH  6

uint32_t gseed = 459;

typedef struct customer{
	 int C_FIRST;
	 int C_MIDDLE;
	 int C_LAST;
	 int C_STREET_1;
	 int C_STREET_2;
	 int C_CITY;
	 int C_STATE;
	 int C_ZIP;
	 int C_PHONE;
	 int C_SINCE;
	 int C_CREDIT;
	 double C_CREDIT_LIM;
	 double C_DISCOUNT;
	 double C_BALANCE;
	 double C_YTD_PAYMENT;
	 int C_PAYMENT_CNT;
	 int C_DELIVERY_CNT;
	 int C_DATA;
	 customer() : C_CREDIT_LIM(50000.0), C_DISCOUNT(10.0), C_YTD_PAYMENT(10.0), C_PAYMENT_CNT(1), C_DELIVERY_CNT(0)
	     {
		 	int r = rand_r_32(&gseed) % 100;
		 	C_CREDIT = r < 90? 1: 2;
			C_DISCOUNT = (double)((rand_r_32(&gseed)%5000) * 0.0001);
	     }
} customer_t;

typedef struct district {
	int  D_NAME;
	int  D_STREET_1;
	int  D_STREET_2;
	int  D_CITY;
	int  D_STATE;
	int  D_ZIP;

	 double D_TAX;
	 double D_YTD;
	 int D_NEXT_O_ID;

	 district(): D_YTD(30000.0), D_NEXT_O_ID(3001)
	 {
		 D_TAX = ((rand_r_32(&gseed)%2000) * 0.0001);
	 }
} district_t;


typedef struct history {
	int H_C_ID;
	 int H_C_D_ID;
	 int H_C_W_ID;
	 int H_D_ID;
	 int H_W_ID;
	 int H_DATE;
	 double H_AMOUNT;
	 int H_DATA;
	 history(int c_id, int d_id): H_AMOUNT(10)
	 {
		 H_W_ID = rand_r_32(&gseed)%100;
		 H_D_ID = d_id;
		 H_C_ID = c_id;
	 }
	 history(): H_AMOUNT(10)
	 	 {
	 		 H_W_ID = rand_r_32(&gseed)%100;
	 		 H_D_ID = rand_r_32(&gseed)% NUM_DISTRICTS;
	 		 H_C_ID = rand_r_32(&gseed)% NUM_CUSTOMERS_PER_D;
	 	 }
} history_t;

typedef struct item {
	int I_IM_ID;
	 int I_NAME;
	 float I_PRICE;
	 int I_DATA;
	 item()
	 {
		 I_PRICE = (float) (rand_r_32(&gseed)%10000);
	 }
} item_t;

typedef struct order {
	int O_C_ID;
	 int O_ENTRY_D;
	 int O_CARRIER_ID;
	 int O_OL_CNT;
	 bool O_ALL_LOCAL;
	 order(): O_ALL_LOCAL(true)
	 {
		 O_C_ID = rand_r_32(&gseed)%100;
		 O_OL_CNT = 5 + rand_r_32(&gseed)%11;
	 }
} order_t;

typedef struct order_line {
	int OL_I_ID;
	 int OL_SUPPLY_W_ID;
	 int OL_DELIVERY_D;
	 int OL_QUANTITY;
	 int OL_AMOUNT;
	 int OL_DIST_INFO;
	 order_line() : OL_QUANTITY(5)
	 {
		 OL_I_ID = 1 + rand_r_32(&gseed)%100000;
		OL_SUPPLY_W_ID = rand_r_32(&gseed)%1000;
		int a = rand_r_32(&gseed)%3000;
		if (a < 2101)
			OL_AMOUNT = 0;
		else {
			OL_AMOUNT = (1 + rand_r_32(&gseed)%(999999));
		}
	 }
} order_line_t;

typedef struct stock {
	int S_QUANTITY;
	 int S_DIST_01;
	 int S_DIST_02;
	 int S_DIST_03;
	 int S_DIST_04;
	 int S_DIST_05;
	 int S_DIST_06;
	 int S_DIST_07;
	 int S_DIST_08;
	 int S_DIST_09;
	 int S_DIST_10;
	 int S_YTD;
	 int S_ORDER_CNT;
	 int S_REMOTE_CNT;
	 int S_DATA;

	 stock() : S_YTD(0), S_ORDER_CNT(0), S_REMOTE_CNT(0)
	 {
		 S_QUANTITY = 10 + rand_r_32(&gseed) %(91);
		 if (rand_r_32(&gseed)%(100) < 10) {
			S_DATA = 1000 + rand_r_32(&gseed)%100;//orginal
		} else {
			S_DATA = rand_r_32(&gseed)%(100);
		}
	 }
} stock_t;

typedef struct warehouse {
	int W_NAME;
	 int W_STREET_1;
	 int  W_STREET_2;
	 int  W_CITY;
	 int W_STATE;
	 int W_ZIP;
	 float W_TAX;
	 float W_YTD;
	 warehouse(): W_YTD(300000.0)
	 {
		W_TAX = (rand_r_32(&gseed)%(2000) * 0.0001);
	 }
} warehouse_t;




item_t* itemsG[8];//[NUM_ITEMS];

warehouse_t* warehousesG[8];//[NUM_WAREHOUSES];

stock_t** stocksG[8];//[NUM_WAREHOUSES][NUM_ITEMS];

district_t** districtsG[8];//[NUM_WAREHOUSES][NUM_DISTRICTS];

customer_t*** customersG[8];//[NUM_WAREHOUSES][NUM_DISTRICTS][NUM_CUSTOMERS_PER_D];

history_t**** historiesG[8];//[NUM_WAREHOUSES][NUM_DISTRICTS][NUM_CUSTOMERS_PER_D][NUM_HISTORY_PER_C];

order_t** ordersG[8];//[NUM_WAREHOUSES][NUM_ORDERS_PER_D];

order_line_t*** order_linesG[8];//[NUM_WAREHOUSES][NUM_ORDERS_PER_D][NUM_LINES_PER_O];









void* th_run(void * args)
{

	int id = (long) args >> 4;
    int numa_zone = (long) args & 0xF;

    //printf("my id %d and zone %d\n", id, numa_zone);

	    //assume symmetric numa zones
	    //printf("numa zones count = %d\n", numa_num_configured_nodes());
		//pin the thread to the numa zone
	    bitmask* mask = numa_allocate_cpumask();

	    //get # of cpu per node
	    numa_node_to_cpus(0, mask);
	    int cpu_per_node = 0;
	    while(numa_bitmask_isbitset(mask, cpu_per_node)) {
	    	cpu_per_node++;
	    }

	    //printf("cpus per node = %d\n", cpu_per_node);

	    numa_bitmask_clearall(mask);
	    numa_bitmask_setbit(mask, numa_zone * cpu_per_node + (id % cpu_per_node));
	    if (numa_sched_setaffinity(0, mask)) {
	    	perror("numa_sched_setaffinity");
			exit(-1);
	    }
	    numa_free_cpumask(mask);

//	    int curcpu = sched_getcpu();
//	    printf("%d:%d cpu set to %d\n", id, numa_zone, curcpu);

		  thread_init(id, numa_zone);
		  //printf("Thread waiting\n");

		  barrier(0);

	unsigned int seed = id;

	if (id == 0) {
		signal(SIGALRM, catch_SIGALRM);
		alarm(1);
	}
	ts_vector* my_ts = ts_vectors[numa_zone];
	unsigned long long time = get_real_time();
	for (int i=0; i<100000; i++) {
		__sync_fetch_and_add(&my_ts->val[numa_zone], 1);
	}
	time = get_real_time() - time;

	printf("%d: Local Total Time = %llu\n", id, time);

	printlogTM(myid);
    //curcpu = sched_getcpu();
    //printf("cpu set to %d\n", curcpu);

	//printf("%d: Total Time = %llu\n", id, time);
	//printf("%d: Throughput = %llu\n", id, (1000000000LL * tx_count) / (time));
    //throughputs[id] = (1000000000LL * tx_count) / (time);
	//TM_TX_VAR
	//printf("%d: commits = %d, aborts = %d, my zone %d, out of zone = %d\n", id, tx->commits, tx->aborts, tx->numa_zone, tx->internuma);

//	long sum = 0;
//	for (int i=0; i<ACCOUT_NUM; i++) {
//		//printf("%d %d %d | ", accounts[i].id, accounts[i].ver, accounts[i].val);
//		sum += accounts[i];
//	}
//	printf("\nsum = %d\n", sum);
//	printf("\nmax tx time = %llu, avg = %llu\n", max, total/(unsigned long long)tx->commits);
	return 0;
}

int main(int argc, char* argv[])
{
	signal(SIGINT, signal_callback_handler);

	tm_sys_init();

	if (argc < 2) {
		printf("Usage test threads_per_zone#\n");
		exit(0);
	}

//	  for (int i=0; i<8;i++) {
//		  create_lock_table(i);
//	  }

    int th_per_zone = atoi(argv[1]);
	total_threads = th_per_zone? th_per_zone * 8 : 1;


	for (int j=0; j < 8; j++) {
		itemsG[j] = (item_t*) numa_alloc_onnode(sizeof(item_t) * NUM_ITEMS, j);
		warehousesG[j] = (warehouse_t*) numa_alloc_onnode(sizeof(warehouse_t) * NUM_WAREHOUSES, j);
		stocksG[j] =  (stock_t**) numa_alloc_onnode(sizeof(stock_t*) * NUM_WAREHOUSES, j);
		districtsG[j] = (district_t**) numa_alloc_onnode(sizeof(district_t*) * NUM_WAREHOUSES, j);
		customersG[j] = (customer_t***) numa_alloc_onnode(sizeof(customer_t**) * NUM_WAREHOUSES, j);
		historiesG[j] = (history_t****) numa_alloc_onnode(sizeof(history_t***) * NUM_WAREHOUSES, j);
		ordersG[j] = (order_t**) numa_alloc_onnode(sizeof(order_t*) * NUM_WAREHOUSES, j);
		order_linesG[j] = (order_line_t***) numa_alloc_onnode(sizeof(order_line_t**) * NUM_WAREHOUSES, j);
		for (int i=0; i<NUM_WAREHOUSES; i++) {
			stocksG[j][i] =  (stock_t*) numa_alloc_onnode(sizeof(stock_t) * NUM_ITEMS, j);
			districtsG[j][i] = (district_t*) numa_alloc_onnode(sizeof(district_t) * NUM_DISTRICTS, j);
			customersG[j][i] = (customer_t**) numa_alloc_onnode(sizeof(customer_t*) * NUM_DISTRICTS, j);
			historiesG[j][i] = (history_t***) numa_alloc_onnode(sizeof(history_t**) * NUM_DISTRICTS, j);
			ordersG[j][i] = (order_t*) numa_alloc_onnode(sizeof(order_t) * NUM_ORDERS_PER_D, j);
			order_linesG[j][i] = (order_line_t**) numa_alloc_onnode(sizeof(order_line_t*) * NUM_ORDERS_PER_D, j);
			for (int k =0; k < NUM_DISTRICTS; k++) {
				customersG[j][i][k] = (customer_t*) numa_alloc_onnode(sizeof(customer_t) * NUM_CUSTOMERS_PER_D, j);
				historiesG[j][i][k] = (history_t**) numa_alloc_onnode(sizeof(history_t*) * NUM_CUSTOMERS_PER_D, j);
				for (int o=0; o<NUM_CUSTOMERS_PER_D; o++) {
					historiesG[j][i][k][o] = (history_t*) numa_alloc_onnode(sizeof(history_t) * NUM_HISTORY_PER_C, j);
				}
			}
			for (int k =0; k < NUM_ORDERS_PER_D; k++) {
				order_linesG[j][i][k] = (order_line_t*) numa_alloc_onnode(sizeof(order_line_t) * NUM_LINES_PER_O, j);
			}
		}
	}

//	for (int j=0; j < 8; j++) {
//		accountsAll[j] = (long*) numa_alloc_onnode(sizeof(long) * ACCOUT_NUM, j);//malloc(sizeof(long) * ACCOUT_NUM);// create_shared_mem(j, sizeof(long) * ACCOUT_NUM, SHARED_MEM_KEY5);//createSharedMem(j);
//	}
//
//	unsigned long long initSum = 0;
//	for (int j=0; j<8; j++)
//		for (int i=0; i<ACCOUT_NUM; i++) {
//			accountsAll[j][i] = 1000;
//			initSum += 1000;
//		}
//	printf("init sum = %llu\n", initSum);

	pthread_attr_t thread_attr;
	pthread_attr_init(&thread_attr);

	pthread_t client_th[64];
	int ids = 1;
	for (int j=0; j < 8; j++) {
		for (int i = 0; i<th_per_zone; i++) {
			if (j==0 && i==0) continue;
			long encodedInfo = (ids << 4) | j;
			pthread_create(&client_th[ids-1], &thread_attr, th_run, (void*)encodedInfo);
			ids++;
		}
	}

	th_run(0);
	//printf("jj= %d, x= %d\n", jj, x);

//  pthread_t thread1, thread2, thread3, thread4;
//  pthread_attr_t thread_attr;
//  pthread_attr_init(&thread_attr);

//	pthread_create(&thread1, &thread_attr, tx_fn, (void*)45678);
//	pthread_create(&thread2, &thread_attr, tx_fn, (void*)4968147);
//	pthread_create(&thread3, &thread_attr, tx_fn, (void*)49147);
//	pthread_create(&thread4, &thread_attr, tx_fn, (void*)4967);

//	barrier =0;
//
//	pthread_join(thread1, NULL);
//	pthread_join(thread2, NULL);
//	pthread_join(thread3, NULL);
//	pthread_join(thread4, NULL);

	for (int i=0; i<ids-1; i++) {
		pthread_join(client_th[i], NULL);
	}

	unsigned long long totalThroughput = 0;
	for (int i=0; i<total_threads; i++) {
		totalThroughput += throughputs[i];
	}

	printf("\nThroughput = %llu\n", totalThroughput);

//	unsigned long long sum = 0;
//	for (int j=0; j<8; j++)
//		for (int i=0; i<ACCOUT_NUM; i++) {
//			//printf("%d %d %d | ", accounts[i].id, accounts[i].ver, accounts[i].val);
//			sum += accountsAll[j][i];
//		}
//	printf("\nsum = %llu, matched = %d\n", sum, sum == initSum);

	return 0;
}
