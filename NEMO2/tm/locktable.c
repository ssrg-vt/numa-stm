#include "locktable.hpp"

//lock_entry * create_lock_table(int numa_zone) {
//	//up to 100 numa zune
//	key_t key = (SHARED_TABLE_KEY + numa_zone);
//	int shmid;
//
//	size_t tbl_size = sizeof(lock_entry) * TABLE_SIZE;
//	if ((shmid = shmget(key, tbl_size, IPC_CREAT /*| IPC_EXCL*/ | 0666)) < 0) {
//		perror("shmget");
//		return NULL;
//	}
//
//	void * shmat_res;
//	if ((shmat_res = shmat(shmid, NULL, 0)) == (void *) -1) {
//		perror("shmat");
//		return NULL;
//	}
//
////	int curcpu = sched_getcpu();
////	int my_node = numa_node_of_cpu(curcpu);
//	nodemask_t mask;
//	nodemask_zero(&mask);
//	nodemask_set_compat(&mask, numa_zone /*my_node*/);
//
////	struct bitmask * bindmask = numa_bitmask_alloc(numa_num_possible_nodes());
////	numa_bitmask_clearall(bindmask);
////	numa_bitmask_setbit(bindmask, numa_zone);
////	copy_bitmask_to_nodemask(bindmask, &mask);
////	numa_bitmask_free(bindmask);
//
////	printf("mask %d  my node %d size %d\n", mask.n[0], numa_zone/*my_node*/, sizeof(mask.n) / sizeof(unsigned long));
//
//	if (mbind(shmat_res, tbl_size, MPOL_BIND, mask.n, 64/*sizeof(mask.n) / sizeof(unsigned long)*/, MPOL_MF_MOVE) == -1) {
//		perror("mbind");
//		return NULL;
//	}
//
//	memset(shmat_res, 0, tbl_size);
//
//	return  (lock_entry*) shmat_res;
//}
//
//
//
//lock_entry * get_lock_table(int numa_zone) {
//	//up to 100 numa zune
//	key_t key = (SHARED_TABLE_KEY + numa_zone);
//	int shmid;
//
//	size_t tbl_size = sizeof(lock_entry) * TABLE_SIZE;
//	if ((shmid = shmget(key, tbl_size, 0666)) < 0) {
//		perror("shmget");
//		return NULL;
//	}
//
//	void * shmat_res;
//	if ((shmat_res = shmat(shmid, NULL, 0)) == (void *) -1) {
//		perror("shmat");
//		return NULL;
//	}
//
//	return  (lock_entry*) shmat_res;
//}
//
//void * create_shared_mem(int numa_zone, int size, int shared_key) {
//	//up to 100 numa zune
//	key_t key = (shared_key + numa_zone);
//	int shmid;
//
//	size_t tbl_size = size;
//	if ((shmid = shmget(key, tbl_size, IPC_CREAT /*| IPC_EXCL*/ | 0666)) < 0) {
//		perror("shmget");
//		return NULL;
//	}
//
//	void * shmat_res;
//	if ((shmat_res = shmat(shmid, NULL, 0)) == (void *) -1) {
//		perror("shmat");
//		return NULL;
//	}
//
////	int curcpu = sched_getcpu();
////	int my_node = numa_node_of_cpu(curcpu);
//	nodemask_t mask;
//	nodemask_zero(&mask);
//	nodemask_set_compat(&mask, numa_zone /*my_node*/);
//
////	struct bitmask * bindmask = numa_bitmask_alloc(numa_num_possible_nodes());
////	numa_bitmask_clearall(bindmask);
////	numa_bitmask_setbit(bindmask, numa_zone);
////	copy_bitmask_to_nodemask(bindmask, &mask);
////	numa_bitmask_free(bindmask);
//
////	printf("mask %d  my node %d size %d\n", mask.n[0], numa_zone/*my_node*/, sizeof(mask.n) / sizeof(unsigned long));
//
//	if (mbind(shmat_res, tbl_size, MPOL_BIND, mask.n, 64/*sizeof(mask.n) / sizeof(unsigned long)*/, MPOL_MF_MOVE) == -1) {
//		perror("mbind");
//		return NULL;
//	}
//
//	memset(shmat_res, 0, tbl_size);
//
//	return  shmat_res;
//}
//
//void * get_shared_mem(int numa_zone, int size, int shared_key) {
//	//up to 100 numa zune
//	key_t key = (shared_key + numa_zone);
//	int shmid;
//
//	size_t tbl_size = size;
//	if ((shmid = shmget(key, tbl_size, 0666)) < 0) {
//		perror("shmget");
//		return NULL;
//	}
//
//	void * shmat_res;
//	if ((shmat_res = shmat(shmid, NULL, 0)) == (void *) -1) {
//		perror("shmat");
//		return NULL;
//	}
//
//	return  shmat_res;
//}
//
//void release_memory(void* table) {
//	if(shmdt(table) != 0) {
//		fprintf(stderr, "Could not close memory segment.\n");
//	}
//}
