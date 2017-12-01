include Makefile.defines.in

#CCFLAGS += -DLOCK_TABLE_SHARED 
#CCFLAGS += -DLOCK_TABLE_SHARED_WRITE
#CCFLAGS += -DWITH_BATCHING
#CCFLAGS += -DSRV_COUNT=8
#CCFLAGS += -DPROCESS_COUNT=56
CCFLAGS += -std=c++11
#CCFLAGS += -DDEBUG
#CCFLAGS += -DDEBUG_W_TIME

OBJFILES = $(OBJ_DIR)/connection.o $(OBJ_DIR)/WriteSet.o $(OBJ_DIR)/tm.o $(OBJ_DIR)/locktable.o $(OBJ_DIR)/test.o   

OBJFILES2 = $(OBJ_DIR)/connection.o $(OBJ_DIR)/WriteSet.o $(OBJ_DIR)/tm.o $(OBJ_DIR)/locktable.o $(OBJ_DIR)/server.o

OBJFILES3 = $(OBJ_DIR)/WriteSet.o $(OBJ_DIR)/tm_t.o $(OBJ_DIR)/test_t.o

OBJFILES4 = $(OBJ_DIR)/test_t.o
#$(OBJ_DIR)/test_queue_seq.o $(OBJ_DIR)/test_queue_busy_wait.o \
           $(OBJ_DIR)/test_queue_pthread_wait.o $(OBJ_DIR)/test_dynamic_array.o $(OBJ_DIR)/test_infrastructure.o $(OBJ_DIR)/test_maysam_shm.o

.PHONY: clean

all:  $(OBJ_DIR)/test_threads 

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/test_threads: $(OBJFILES3)
	$(CPP) $(CCFLAGS) -o $@ $(OBJFILES3) $(LDFLAGS)
	cp $(OBJ_DIR)/test_threads .


$(OBJ_DIR)/test_t.o: $(OBJ_DIR) $(SRC_DIR)/test_threads.cpp 
	$(CPP) $(CCFLAGS) $(CPPFLAGS) $(SRC_DIR)/test_threads.cpp -c -o $@


$(OBJ_DIR)/WriteSet.o: $(OBJ_DIR) $(SRC_DIR)/tm/WriteSet.c $(SRC_DIR)/tm/WriteSet.hpp
	$(CPP) $(CCFLAGS) $(CPPFLAGS) $(SRC_DIR)/tm/WriteSet.c -c -o $@

$(OBJ_DIR)/tm_t.o: $(OBJ_DIR) $(SRC_DIR)/tm/tm_thread.c $(SRC_DIR)/tm/tm_thread.hpp
	$(CPP) $(CCFLAGS) $(CPPFLAGS) $(SRC_DIR)/tm/tm_thread.c -c -o $@

#$(OBJ_DIR)/numa_alloc.o: $(OBJ_DIR) $(SRC_DIR)/tm/numa_fine_allocator.c $(SRC_DIR)/tm/numa_fine_allocator.hpp
#	$(CPP) $(CCFLAGS) $(CPPFLAGS) $(SRC_DIR)/tm/numa_fine_allocator.c -c -o $@

# $(OBJ_DIR)/locktable.o: $(OBJ_DIR) $(SRC_DIR)/tm/locktable.cpp $(SRC_DIR)/tm/locktable.hpp
#	$(CPP) $(CCFLAGS) $(CPPFLAGS) $(SRC_DIR)/tm/locktable.cpp -c -o $@
	
#$(OBJ_DIR)/test_queue_seq.o: $(OBJ_DIR) $(SRC_DIR)/mpass/queue.h $(SRC_DIR)/test/test_queue_seq.cc
#	$(CPP) $(CCFLAGS) $(CPPFLAGS) $(SRC_DIR)/test/test_queue_seq.cc -c -o $@

#$(OBJ_DIR)/test_queue_busy_wait.o: $(OBJ_DIR) $(SRC_DIR)/mpass/queue.h $(SRC_DIR)/test/test_queue_busy_wait.cc
#	$(CPP) $(CCFLAGS) $(CPPFLAGS) $(SRC_DIR)/test/test_queue_busy_wait.cc -c -o $@

#$(OBJ_DIR)/test_queue_pthread_wait.o: $(OBJ_DIR) $(SRC_DIR)/mpass/queue.h $(SRC_DIR)/test/test_queue_pthread_wait.cc
#	$(CPP) $(CCFLAGS) $(CPPFLAGS) $(SRC_DIR)/test/test_queue_pthread_wait.cc -c -o $@

#$(OBJ_DIR)/test_dynamic_array.o: $(OBJ_DIR) $(SRC_DIR)/common/dynamic_array.h $(SRC_DIR)/test/test_dynamic_array.cc
#	$(CPP) $(CCFLAGS) $(CPPFLAGS) $(SRC_DIR)/test/test_dynamic_array.cc -c -o $@

#$(OBJ_DIR)/test_infrastructure.o: $(OBJ_DIR) $(SRC_DIR)/test/test_infrastructure.cc
#	$(CPP) $(CCFLAGS) $(CPPFLAGS) $(SRC_DIR)/test/test_infrastructure.cc -c -o $@

#$(OBJ_DIR)/test_maysam_shm.o: $(OBJ_DIR) $(SRC_DIR)/test/test_maysam_shm.cc
#	$(CPP) $(CCFLAGS) $(CPPFLAGS) $(SRC_DIR)/test/test_maysam_shm.cc -c -o $@

################
# common tasks #
################

clean:
	rm -rf $(TARGET_DIR)
	rm -f server test test_threads


