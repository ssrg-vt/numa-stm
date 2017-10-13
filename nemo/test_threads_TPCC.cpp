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


#define  NUM_ITEMS  10000
//1000; // Correct overall # of items: 100,000
#define  NUM_WAREHOUSES  20
#define  NUM_STOCKS_PER_W  100
#define  NUM_DISTRICTS  10
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

#define GC 1
#define BC 2

typedef struct customer{
	 int C_FIRST[16];
	 int C_MIDDLE[2];
	 int C_LAST[16];
	 int C_STREET_1[20];
	 int C_STREET_2[20];
	 int C_CITY[20];
	 int C_STATE[2];
	 int C_ZIP[9];
	 int C_PHONE[16];
	 double C_SINCE;
	 int C_CREDIT;
	 double C_CREDIT_LIM;
	 double C_DISCOUNT;
	 double C_BALANCE;
	 double C_YTD_PAYMENT;
	 int C_PAYMENT_CNT;
	 int C_DELIVERY_CNT;
	 int C_DATA[50];
	 customer() : C_CREDIT_LIM(50000.0), C_DISCOUNT(10.0), C_YTD_PAYMENT(10.0), C_PAYMENT_CNT(1), C_DELIVERY_CNT(0)
	     {
		 	int r = rand_r_32(&gseed) % 100;
		 	C_CREDIT = r < 90? GC: BC;
			C_DISCOUNT = (double)((rand_r_32(&gseed)%5000) * 0.0001);
	     }
} customer_t;

typedef struct district {
	int  D_NAME[10];
	int  D_STREET_1[20];
	int  D_STREET_2[20];
	int  D_CITY[20];
	int  D_STATE[2];
	int  D_ZIP[9];

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
	 double H_DATE;
	 double H_AMOUNT;
	 int H_DATA[24];
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
	 int I_NAME[24];
	 float I_PRICE;
	 int I_DATA[50];
	 item()
	 {
		 I_PRICE = (float) (rand_r_32(&gseed)%10000);
	 }
} item_t;

typedef struct order {
	int O_C_ID;
	 double O_ENTRY_D;
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
	 double OL_DELIVERY_D;
	 int OL_QUANTITY;
	 int OL_AMOUNT;
	 int OL_DIST_INFO[24];
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
	 int S_DIST_01[24];
	 int S_DIST_02[24];
	 int S_DIST_03[24];
	 int S_DIST_04[24];
	 int S_DIST_05[24];
	 int S_DIST_06[24];
	 int S_DIST_07[24];
	 int S_DIST_08[24];
	 int S_DIST_09[24];
	 int S_DIST_10[24];
	 int S_YTD;
	 int S_ORDER_CNT;
	 int S_REMOTE_CNT;
	 int S_DATA[50];

	 stock() : S_YTD(0), S_ORDER_CNT(0), S_REMOTE_CNT(0)
	 {
		 S_QUANTITY = 10 + rand_r_32(&gseed) %(91);
		 if (rand_r_32(&gseed)%(100) < 10) {
			S_DATA[25] = 1000 + rand_r_32(&gseed)%100;//orginal
		} else {
			S_DATA[25] = rand_r_32(&gseed)%(100);
		}
	 }
} stock_t;

typedef struct warehouse {
	int W_NAME[10];
	 int W_STREET_1[20];
	 int  W_STREET_2[20];
	 int  W_CITY[20];
	 int W_STATE[2];
	 int W_ZIP[9];
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

	unsigned long long time = get_real_time();
//	uint64_t max = 0;
//	uint64_t total = 0;
	int tx_count = 0;
	while(ExperimentInProgress) {
	    uint32_t act = rand_r_32(&seed) % 100;
		int w_id = rand_r_32(&seed) %(NUM_WAREHOUSES);
		int o_id = rand_r_32(&seed) %(NUM_ORDERS_PER_D);
		int c_id = rand_r_32(&seed) %(NUM_CUSTOMERS_PER_D);
		int d_id = rand_r_32(&seed) %(NUM_DISTRICTS);
		int l_id = rand_r_32(&seed) %(NUM_LINES_PER_O);
		int orderLineCount = rand_r_32(&seed) % NUM_LINES_PER_O;
		int crtdate = rand_r_32(&seed) % 50000;
		float h_amount = (float)(rand_r_32(&seed) %(500000) * 0.01);

	    int tx_numa = numa_zone;
		if (tx_count % 10 == 0){
			tx_numa = (numa_zone+1) % 8;
		}

	    item_t* items = itemsG[tx_numa];//[NUM_ITEMS];

	    warehouse_t* warehouses = warehousesG[tx_numa];//[NUM_WAREHOUSES];

	    stock_t** stocks = stocksG[tx_numa];//[NUM_WAREHOUSES][NUM_ITEMS];

	    district_t** districts = districtsG[tx_numa];//[NUM_WAREHOUSES][NUM_DISTRICTS];

	    customer_t*** customers = customersG[tx_numa];//[NUM_WAREHOUSES][NUM_DISTRICTS][NUM_CUSTOMERS_PER_D];

	    history_t**** histories = historiesG[tx_numa];//[NUM_WAREHOUSES][NUM_DISTRICTS][NUM_CUSTOMERS_PER_D][NUM_HISTORY_PER_C];

	    order_t** orders = ordersG[tx_numa];//[NUM_WAREHOUSES][NUM_ORDERS_PER_D];

	    order_line_t*** order_lines = order_linesG[tx_numa];//[NUM_WAREHOUSES][NUM_ORDERS_PER_D][NUM_LINES_PER_O];

		tx_count++;
		TM_BEGIN
			if(act < 4) {
		//		(READ_ONLY_TX);
		//		(TX_ORDER_STATUS);
				float dummy;
				dummy = TM_READ_Z(warehouses[w_id].W_YTD, tx_numa);
				dummy = TM_READ_Z(orders[w_id][o_id].O_CARRIER_ID, tx_numa);
				dummy = TM_READ_Z(customers[w_id][d_id][c_id].C_BALANCE, tx_numa);

				float olsum = (float)0;
				int i = 1;

				while (i < orderLineCount) {
					olsum += TM_READ_Z(order_lines[w_id][o_id][i].OL_AMOUNT, tx_numa);
					dummy = TM_READ_Z(order_lines[w_id][o_id][i].OL_QUANTITY, tx_numa);
					i += 1;
				}
			} else if (act < 8) {
		//		(READ_ONLY_TX);
		//		(TX_STOCKLEVEL);
				double dummy;

				dummy= TM_READ_Z(warehouses[w_id].W_YTD, tx_numa);

				for(int i=0; i<20; i++) {
					/*************** Transaction start ***************/
					o_id = rand_r_32(&seed) %(NUM_ORDERS_PER_D);
					dummy=TM_READ_Z(orders[w_id][o_id].O_CARRIER_ID, tx_numa);

						for(int j=0; j<10; j++) {
							l_id = rand_r_32(&seed) %(NUM_LINES_PER_O);
							dummy=TM_READ_Z(order_lines[w_id][o_id][l_id].OL_QUANTITY, tx_numa);
						}
				}

				for(int i=0; i<10; i++) {
					int s_id = rand_r_32(&seed) %(NUM_ITEMS);
					dummy=TM_READ_Z(stocks[w_id][s_id].S_QUANTITY, tx_numa);
				}
			} else if (act < 12) {
		//		(READ_WRITE_TX);
		//		(TX_DELIVERY);
				double dummy;
				dummy=TM_READ_Z(warehouses[w_id].W_YTD, tx_numa);
				for (int d_id = 0; d_id < NUM_DISTRICTS; d_id++) {
					o_id = rand_r_32(&seed) %(NUM_ORDERS_PER_D);
					dummy=TM_READ_Z(orders[w_id][o_id].O_CARRIER_ID, tx_numa);

					float olsum = (float)0;

					int i = 1;
					while (i < orderLineCount) {
						olsum += TM_READ_Z(order_lines[w_id][o_id][i].OL_AMOUNT, tx_numa);
						dummy=TM_READ_Z(order_lines[w_id][o_id][i].OL_QUANTITY, tx_numa);
						i += 1;
					}
					c_id = rand_r_32(&seed) %(NUM_CUSTOMERS_PER_D);
					TM_WRITE_Z(customers[w_id][d_id][c_id].C_BALANCE, TM_READ_Z(customers[w_id][d_id][c_id].C_BALANCE, tx_numa) + olsum, tx_numa);
					TM_WRITE_Z(customers[w_id][d_id][c_id].C_DELIVERY_CNT, TM_READ_Z(customers[w_id][d_id][c_id].C_DELIVERY_CNT, tx_numa) + 1, tx_numa);
				}

			} else if (act < 55) {
		//		(READ_WRITE_TX);
		//		(TX_PAYMENT);


				// Open Wairehouse Table

				TM_WRITE_Z(warehouses[w_id].W_YTD, TM_READ_Z(warehouses[w_id].W_YTD, tx_numa) + h_amount, tx_numa);

				// In DISTRICT table

				TM_WRITE_Z(districts[w_id][d_id].D_YTD, TM_READ_Z(districts[w_id][d_id].D_YTD, tx_numa) + h_amount, tx_numa);

				TM_WRITE_Z(customers[w_id][d_id][c_id].C_BALANCE, TM_READ_Z(customers[w_id][d_id][c_id].C_BALANCE, tx_numa) - h_amount, tx_numa);
				TM_WRITE_Z(customers[w_id][d_id][c_id].C_YTD_PAYMENT, TM_READ_Z(customers[w_id][d_id][c_id].C_YTD_PAYMENT, tx_numa) + h_amount, tx_numa);
				TM_WRITE_Z(customers[w_id][d_id][c_id].C_PAYMENT_CNT, TM_READ_Z(customers[w_id][d_id][c_id].C_PAYMENT_CNT, tx_numa) + 1, tx_numa);

				int hmyid = rand_r_32(&seed) %(NUM_HISTORY_PER_C);
				TM_WRITE_Z(histories[w_id][d_id][c_id][hmyid].H_W_ID, w_id, tx_numa);
//				TM_WRITE_Z(histories[w_id][d_id][c_id][hmyid].H_D_ID, d_id, tx_numa);
//				TM_WRITE_Z(histories[w_id][d_id][c_id][hmyid].H_C_ID, c_id, tx_numa);
//				TM_WRITE_Z(histories[w_id][d_id][c_id][hmyid].H_DATE, 10.0, tx_numa);
//				TM_WRITE_Z(histories[w_id][d_id][c_id][hmyid].H_AMOUNT, (double)h_amount, tx_numa);
//				TM_WRITE_Z(histories[w_id][d_id][c_id][hmyid].H_DATA[15], 111, tx_numa);
			} else {
		//			(READ_WRITE_TX);
		//			(TX_NEWORDER);

				double dummy;
				dummy=TM_READ_Z(warehouses[w_id].W_YTD, tx_numa);

				double D_TAX = TM_READ_Z(districts[w_id][d_id].D_TAX, tx_numa);
				int o_id = TM_READ_Z(districts[w_id][d_id].D_NEXT_O_ID, tx_numa) %(NUM_ORDERS_PER_D);
				TM_WRITE_Z(districts[w_id][d_id].D_NEXT_O_ID, o_id + 1, tx_numa);


				double C_DISCOUNT = TM_READ_Z(customers[w_id][d_id][c_id].C_DISCOUNT, tx_numa);
				int C_LAST = TM_READ_Z(customers[w_id][d_id][c_id].C_LAST[10], tx_numa);
				int C_CREDIT = TM_READ_Z(customers[w_id][d_id][c_id].C_CREDIT, tx_numa);

				//TODO we don't have create, so simulate it with one write in the array in random index
				// Create entries in ORDER and NEW-ORDER

	//			order_t  norder;// = (order_t*)TM_ALLOC(sizeof(order_t));
	//			norder.O_C_ID = c_id;
	//			norder.O_CARRIER_ID = rand_r_32(seed) %(10); //1000
	//			norder.O_ALL_LOCAL = true;
				TM_WRITE_Z(orders[w_id][o_id].O_C_ID, c_id, tx_numa);
				TM_WRITE_Z(orders[w_id][o_id].O_CARRIER_ID, rand_r_32(&seed) %(10), tx_numa);

				int items_count = 5;

				for (int i = 0; i <= items_count; i++) {
					int i_id = rand_r_32(&seed) %(NUM_ITEMS);

					float I_PRICE = TM_READ_Z(items[i_id].I_PRICE, tx_numa);
					int I_NAME = TM_READ_Z(items[i_id].I_NAME[5], tx_numa);
					int I_DATA = TM_READ_Z(items[i_id].I_DATA[10], tx_numa);



					//order_line_t orderLine;// = (order_line_t*)TM_ALLOC(sizeof(order_line_t));

					int l_id = rand_r_32(&seed) %(NUM_LINES_PER_O);

	//				orderLine.OL_QUANTITY = rand_r_32(seed) %(1000);
	//				orderLine.OL_I_ID = i_id;
	//				orderLine.OL_SUPPLY_W_ID = w_id;
	//				orderLine.OL_AMOUNT = (int)(orderLine.OL_QUANTITY  * I_PRICE);
	//				orderLine.OL_DELIVERY_D = 0;
	//				orderLine.OL_DIST_INFO = d_id;




					TM_WRITE_Z(order_lines[w_id][o_id][l_id].OL_QUANTITY, rand_r_32(&seed) %(1000), tx_numa);
//					TM_WRITE_Z(order_lines[w_id][o_id][l_id].OL_I_ID, i_id, tx_numa);
//					TM_WRITE_Z(order_lines[w_id][o_id][l_id].OL_SUPPLY_W_ID, w_id, tx_numa);
//					TM_WRITE_Z(order_lines[w_id][o_id][l_id].OL_AMOUNT, (int)(order_lines[w_id][o_id][l_id].OL_QUANTITY * I_PRICE), tx_numa);
//					TM_WRITE_Z(order_lines[w_id][o_id][l_id].OL_DELIVERY_D, 10.1, tx_numa);
//					TM_WRITE_Z(order_lines[w_id][o_id][l_id].OL_DIST_INFO[10], 1005, tx_numa);

				}


			}

		TM_END
//		uint64_t tx_t = tick() - start;
//		if (tx_t > max) max=tx_t;
//		total +=tx_t;
	}
	//endtx:
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
	time = get_real_time() - time;

	printlogTM(myid);
    //curcpu = sched_getcpu();
    //printf("cpu set to %d\n", curcpu);

	//printf("%d: Total Time = %llu\n", id, time);
	//printf("%d: Throughput = %llu\n", id, (1000000000LL * tx_count) / (time));
    throughputs[id] = (1000000000LL * tx_count) / (time);
	TM_TX_VAR
	printf("%d: commits = %d, aborts = %d, my zone %d, out of zone = %d\n", id, tx->commits, tx->aborts, tx->numa_zone, tx->internuma);

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
