/*
 * hello_libpmemobj.c -- an example for libpmemobj library
 */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <libpmemobj.h>

// Name of our layout in the pool
#define LAYOUT "KVS_layout"
#define NUM_LAYOUT "number"

// Maximum length of our buffer
#define MAX_BUF_LEN 30
#define MAX_STORE 100

#define PATH "/mnt/pmem0/data"
#define NUM_PATH "/mnt/pmem0/num"


// Root structure
struct KVstruct {
	int id;
    int keylen;
	char key[MAX_BUF_LEN];
    int value;
};

/****************************
 * This function writes the "Hello..." string to persistent-memory.
 *****************************/
void write_KVS (char *key, int value)
{
    PMEMobjpool *num_pop;
    int id;

    //char *num_path = NUM_PATH;
    
    num_pop = pmemobj_create(NUM_PATH, NUM_LAYOUT, PMEMOBJ_MIN_POOL, 0666);
		
	if (num_pop == NULL) 
	{
        num_pop = pmemobj_open(NUM_PATH, NUM_LAYOUT);
        if(num_pop == NULL){
            perror(NUM_PATH);
            exit(1);
        }

        PMEMoid num_root = pmemobj_root(num_pop, sizeof (int));
        int *num_rootp = pmemobj_direct(num_root);
        *num_rootp = *num_rootp + 1;
	    pmemobj_persist(num_pop, num_rootp, sizeof (int));

        id = *num_rootp - 1;
        pmemobj_close(num_pop);
	}
    else{
        PMEMoid num_root = pmemobj_root(num_pop, sizeof (int));
        int *num_rootp = pmemobj_direct(num_root);
        *num_rootp = 1;
	    pmemobj_persist(num_pop, num_rootp, sizeof (int));

        id = *num_rootp - 1;
        pmemobj_close(num_pop);
    }
	
    PMEMobjpool *pop;
    char *path = PATH;

	pop = pmemobj_create(path, LAYOUT, PMEMOBJ_MIN_POOL, 0666);
		
	if (pop == NULL) 
	{
        pop = pmemobj_open(path, LAYOUT);
        if(pop == NULL){
            perror(path);
            exit(1);
        }
	}

	PMEMoid root = pmemobj_root(pop, sizeof (struct KVstruct) * MAX_STORE);

	struct KVstruct *rootp = pmemobj_direct(root);
    rootp += id;

	rootp->keylen = strlen(key);
	pmemobj_persist(pop, &rootp->keylen, sizeof (rootp->keylen));

    rootp->value = value;
    pmemobj_persist(pop, &rootp->value, sizeof (rootp->value));

    rootp->id = id;
    pmemobj_persist(pop, &rootp->id, sizeof (rootp->id));
	
	pmemobj_memcpy_persist(pop, rootp->key, key, rootp->keylen);

	printf("Write [key: %s, value: %d]\n", rootp->key, rootp->value);

	pmemobj_close(pop);	
		
	return;
}

/****************************
 * This function reads the "Hello..." string from persistent-memory.
 *****************************/
void read_KVS(char *key, int read_all)
{
    PMEMobjpool *num_pop;
    int number;
    
    num_pop = pmemobj_open(NUM_PATH, NUM_LAYOUT);
		
	if (num_pop == NULL) 
	{
        perror(NUM_PATH);
        exit(1);
    }

    PMEMoid num_root = pmemobj_root(num_pop, sizeof (int));
    int *num_rootp = pmemobj_direct(num_root);
    number = *num_rootp;
    pmemobj_close(num_pop);

	PMEMobjpool *pop;
    char *path = PATH;
	
	pop = pmemobj_open(path, LAYOUT);

	if (pop == NULL) {
		perror(path);
		exit(1);
	} 
	
	PMEMoid root = pmemobj_root(pop, sizeof (struct KVstruct) * MAX_STORE);
	struct KVstruct *rootp = pmemobj_direct(root);

    int flag = 0;
    int i;
    if(read_all == 0){
        for(i = 0; i < number; i++){
            //printf("loop: %d\n", i);
            if(strncmp((rootp + i)->key, key, (rootp + i)->keylen) == 0){
                printf("Read [key: %s, value: %d]\n", (rootp + i)->key, (rootp + i)->value);
                printf("id: %d\n", (rootp + i)->id);
                flag = 1;
                break;
            }
        }
        if(flag == 0)
            printf("%s is not registered.\n", key);
    }
    else{
        for(i = 0; i < number; i++){
            printf("Read [key: %s, value: %d]\n", (rootp + i)->key, (rootp + i)->value);
            printf("id: %d\n", (rootp + i)->id);
        }
    }
    
	pmemobj_close(pop);

	return;
}

void delete_KVS(char *key){
    PMEMobjpool *num_pop;
    int number;
    
    num_pop = pmemobj_open(NUM_PATH, NUM_LAYOUT);
		
	if (num_pop == NULL) 
	{
        perror(NUM_PATH);
        exit(1);
    }

    PMEMoid num_root = pmemobj_root(num_pop, sizeof (int));
    int *num_rootp = pmemobj_direct(num_root);
    number = *num_rootp;


	PMEMobjpool *pop;
    char *path = PATH;
	
	pop = pmemobj_open(path, LAYOUT);

	if (pop == NULL) {
		perror(path);
		exit(1);
	} 
	
	PMEMoid root = pmemobj_root(pop, sizeof (struct KVstruct) * MAX_STORE);
	struct KVstruct *rootp = pmemobj_direct(root);

    int flag = 0;
    int i;
    for(i = 0; i < number; i++){
        if(strncmp((rootp + i)->key, key, (rootp + i)->keylen) == 0){
            printf("Read [key: %s, value: %d]\n", (rootp + i)->key, (rootp + i)->value);
            printf("id: %d\n", (rootp + i)->id);

            if(i != number - 1){
                pmemobj_memcpy_persist(pop, rootp + i, rootp + number-1, sizeof(struct KVstruct));
                (rootp + i)->id = i;
                pmemobj_persist(pop, &(rootp+i)->id, sizeof(rootp->id));

                printf("overwrite %s\n", key);
            }
            *num_rootp = *num_rootp - 1;
            pmemobj_persist(num_pop, num_rootp, sizeof (int));
            printf("decrement number: %d\n", *num_rootp);

            flag = 1;
            break;
        }
    }

    if(flag == 0)
        printf("%s is not registered.\n", key);

    pmemobj_close(num_pop);
	pmemobj_close(pop);

	return;
}

/****************************
 * This main function gather from the command line and call the appropriate
 * function to perform read and write persistently to memory.
 *****************************/
int main(int argc, char *argv[])
{
    char *key = argv[2];
	
	if (strcmp (argv[1], "-w") == 0 && argc == 4) {
        int value = atoi(argv[3]);
		write_KVS(key, value);
	} else if (strcmp (argv[1], "-r") == 0 && argc == 3) {
		read_KVS(key, 0);
	} else if (strcmp (argv[1], "-ra") == 0 && argc == 2) {
		read_KVS("", 1);
	} else if (strcmp (argv[1], "-d") == 0 && argc == 3){
        delete_KVS(key);
    } else { 
		fprintf(stderr, "Usage: %s <-w/-r> <filename>\n", argv[0]);
		exit(1);
	}

}