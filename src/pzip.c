#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pzip.h"

// Define a struct for a node in the linked list
struct Node {
    int data;
    struct Node* next;
};

// Define a struct that includes a linked list
struct LinkedList {
    struct Node* head;
};

typedef struct {
	int t_index;
	int range;
	int input_chars_size;
	int uniqueCharCount;
	char *input_chars;
	int frequencies[26];
	struct zipped_char *local_zipped_chars;
	struct LinkedList *unique_chars;
} threadObj;

void* handleZip (void * arg);
void addNode(struct LinkedList* list, char data);
void printLinkedList(const struct LinkedList* list);

/**
 * pzip() - zip an array of characters in parallel
 *
 * Inputs:
 * @n_threads:		   The number of threads to use in pzip
 * @input_chars:		   The input characters (a-z) to be zipped
 * @input_chars_size:	   The number of characaters in the input file
 *
 * Outputs:
 * @zipped_chars:       The array of zipped_char structs
 * @zipped_chars_count:   The total count of inserted elements into the zippedChars array.
 * @char_frequency[26]: Total number of occurences
 *
 * NOTE: All outputs are already allocated. DO NOT MALLOC or REASSIGN THEM !!!
 *
 */
void pzip(int n_threads, char *input_chars, int input_chars_size,
	  struct zipped_char *zipped_chars, int *zipped_chars_count,
	  int *char_frequency)
{
	int range = input_chars_size/n_threads;
	pthread_t* threads = malloc(n_threads * sizeof(pthread_t));
	threadObj* threadData = malloc(n_threads * sizeof(threadObj));


	if(threads == NULL){
		perror("Failed to create threads");
		exit(1);
	}

	if(threadData == NULL){
		perror("failed to create thread data");
	}

	for(int i = 0; i < n_threads; i++){
		threadData[i].t_index = i;
		threadData[i].range = range;
		threadData[i].input_chars_size = input_chars_size;
		threadData[i].input_chars = input_chars;
		threadData[i].uniqueCharCount = 0;
		//THIS STILL NEEDS TO BE FREED
		threadData[i].unique_chars = malloc(sizeof(struct LinkedList));
		threadData[i].unique_chars->head = NULL;
		if (pthread_create(&threads[i], NULL, handleZip, (void*)&threadData[i]) != 0) {
            perror("Failed to create thread");
            exit(1);
		}
	}

	for (int i = 0; i < n_threads; i++) {
		if (pthread_join(threads[i], NULL) != 0) {
			perror("Failed to join thread");
			exit(1);
		}
    }

	int nextIndex = 0;

	for (int i = 0; i < n_threads; i++){
		threadObj curr_thread = threadData[i];
		// printf("%d\n", curr_thread.uniqueCharCount);
		*zipped_chars_count = *zipped_chars_count + curr_thread.uniqueCharCount;
		for(int j = 0; j < curr_thread.uniqueCharCount; j++){
			int index = curr_thread.unique_chars->head->data - 'a';
			int value = curr_thread.frequencies[index];
			char_frequency[index] += value;
			struct Node* temp = curr_thread.unique_chars->head;
			curr_thread.unique_chars->head = curr_thread.unique_chars->head->next;
			free(temp);
		}

		for(int j = 0; j < curr_thread.uniqueCharCount; j++){
			zipped_chars[j + nextIndex] = curr_thread.local_zipped_chars[j];
		}

		nextIndex += curr_thread.uniqueCharCount;
	}

	// for(int j = 0; j < n_threads; j++){
	// 	for(int i = 0; i < 26; i++){
	// 		printf("%d ", threadData[j].frequencies[i]);
	// 	}
	// 	printf("\n");
	// }

	free(threads);
	free(threadData);
}

void* handleZip (void* arg){
	threadObj* data = (threadObj*)arg;
	int freq_order[data->range];
	

	for(int i = 0; i < 26; i++){
		data->frequencies[i] = 0;
	}

	int start = data->range * data->t_index;
	int end = start + data->range;

	for(int i = start; i < end; i++){
		char currentChar = data->input_chars[i];
		if(i == start || currentChar != data->unique_chars->head->data){
			data->uniqueCharCount += 1;
			addNode(data->unique_chars, currentChar);

		}
		freq_order[data->uniqueCharCount - 1]++;
		data->frequencies[currentChar - 'a']++;		
	}

	//THIS STILL NEEDS TO BE FREED
	data->local_zipped_chars = malloc(data->uniqueCharCount * sizeof(struct zipped_char));
	struct Node* temp = data->unique_chars->head;
	for(int i = data->uniqueCharCount - 1; i >= 0; i--){
		data->local_zipped_chars[i].character = temp->data;
		data->local_zipped_chars[i].occurence = freq_order[i];
		temp = temp->next;
	}



	return NULL;
}

void addNode(struct LinkedList* list, char data) {
    struct Node* newNode = malloc(sizeof(struct Node));
    if (newNode == NULL) {
        perror("Failed to allocate memory for a new node");
        exit(1);
    }

    newNode->data = data;
    newNode->next = list->head;
    list->head = newNode;
}

void printLinkedList(const struct LinkedList* list) {
    struct Node* current = list->head;
    while (current != NULL) {
        printf("%d -> ", current->data);
        current = current->next;
    }
    printf("NULL\n");
}
