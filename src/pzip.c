#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pzip.h"

char* chars;

typedef struct {
	int t_index;
	int range;
	int uniqueCharCount;
	int frequencies[26];
	struct zipped_char *local_zipped_chars;
} threadObj;

void* handleZip (void * arg);

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

	chars = input_chars;


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
		threadData[i].uniqueCharCount = 0;
		//THIS STILL NEEDS TO BE FREED
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
			int index = curr_thread.local_zipped_chars[j].character - 'a';
			int value = curr_thread.frequencies[index];
			char_frequency[index] += value;
			zipped_chars[j + nextIndex] = curr_thread.local_zipped_chars[j];
		}

		for(int j = 0; j < curr_thread.uniqueCharCount; j++){
		}

		nextIndex += curr_thread.uniqueCharCount;

		free(threadData[i].local_zipped_chars);
	}

	free(threads);
	free(threadData);
}

void* handleZip (void* arg){
	threadObj* data = (threadObj*)arg;
	int* freq_order = malloc(data->range * sizeof(int));
	char* freq_order_char = malloc(data->range * sizeof(char));
	

	for(int i = 0; i < 26; i++){
		data->frequencies[i] = 0;
	}

	int start = data->range * data->t_index;
	int end = start + data->range;

	for(int i = start; i < end; i++){
		char currentChar = chars[i];
		if(i == start || currentChar != freq_order_char[data->uniqueCharCount - 1]){
			data->uniqueCharCount += 1;
			freq_order_char[data->uniqueCharCount - 1] = currentChar;
			freq_order[data->uniqueCharCount - 1] = 0;
		}
		freq_order[data->uniqueCharCount - 1]++;
		data->frequencies[currentChar - 'a']++;		
	}

	// Freed at end of main function 
	data->local_zipped_chars = malloc(data->uniqueCharCount * sizeof(struct zipped_char));
	if(data->local_zipped_chars == NULL){
		perror("Failed to create local_zipped_char array");
		exit(1);
	}
	for(int i = 0; i <  data->uniqueCharCount; i++){
		data->local_zipped_chars[i].character = freq_order_char[i];
		data->local_zipped_chars[i].occurence = freq_order[i];
	}
	free(freq_order);
	free(freq_order_char);
	return NULL;
}
