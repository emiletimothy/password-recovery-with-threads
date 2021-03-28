CLEAN_COMMAND = rm -f bin/* passwords.txt
ifdef DEBUG
  CC = clang-with-asan
  CFLAGS = -Iinclude -Wall -Wextra
  ifeq ($(wildcard .debug),)
    $(shell $(CLEAN_COMMAND))
    $(shell touch .debug)
  endif
else
  CC = clang
  CFLAGS = -Iinclude -Wall -Wextra -O3 -g
  ifneq ($(wildcard .debug),)
    $(shell $(CLEAN_COMMAND) .debug)
  endif
endif

TESTS_SQUEUE = squeue_single_fill squeue_push_pop
TESTS_MQUEUE = mqueue_push_pop mqueue_test_empty
TESTS_THREADPOOL = prime_printer multiple_pools recursive_add_work periodic_work repeat_drain sleepers

test: test_queue test_threadpool

test_squeue: $(addprefix bin/,$(TESTS_SQUEUE))
	tests/test_squeue.sh

test_mqueue: $(addprefix bin/,$(TESTS_MQUEUE))
	tests/test_mqueue.sh

test_queue: test_squeue test_mqueue

test_threadpool: $(addprefix bin/,$(TESTS_THREADPOOL))
	tests/test_pool.sh

passwords.txt: bin/password_cracker hashes.txt
	nohup $< < hashes.txt | tee $@

bin/%.o: src/%.c
	$(CC) $(CFLAGS) -c $^ -o $@

bin/squeue_%: tests/squeue_%.c bin/queue.o
	$(CC) $(CFLAGS) -lpthread $^ -o $@

bin/mqueue_%: tests/mqueue_%.c bin/queue.o
	$(CC) $(CFLAGS) -lpthread $^ -o $@

bin/%: tests/%.c bin/queue.o bin/thread_pool.o
	$(CC) $(CFLAGS) -lpthread $^ -o $@

bin/password_cracker: src/password_cracker.c bin/queue.o bin/thread_pool.o
	$(CC) $(CFLAGS) -lcrypt -lpthread $^ -o $@

clean:
	$(CLEAN_COMMAND)
