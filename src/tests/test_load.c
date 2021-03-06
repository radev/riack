
#include "test.h"

int test_load_putcount;
int test_load_delcount;

int test_load(char* testcase)
{
	if (strcmp(testcase, "init") == 0) {
		return test_load_init();
	} else if (strcmp(testcase, "cleanup") == 0) {
		return test_load_cleanup();
	} else {
		return -1;
	}
}

int process_file(char* filename, char* target_bucket)
{
	char buffer[1024*8];
	char id[20];
	int result;
	FILE *infile;
	result = 0;
	infile = fopen(filename, "r");
	if (infile) {
		result = 1;
		while (fscanf(infile, "%[-0-9]#%[^\n]\n", id, buffer) > 1) {
            if (riack_put_simple(test_client, target_bucket, id, (uint8_t*)buffer, strlen(buffer), "application/json") != RIACK_SUCCESS) {
				printf("Failed to put id %s\n", id);
				result = 0;
				break;
			}
			test_load_putcount++;
		}
		fclose(infile);
	}
	return result;
}

int test_delete_all_keys(RIACK_STRING bucket, struct RIACK_STRING_LINKED_LIST *list)
{
	struct RIACK_STRING_LINKED_LIST *current;
	int result;
	current = list;
	result = 1;
	while (current != 0) {
		if (riack_delete(test_client, bucket, current->string, 0) != RIACK_SUCCESS) {
			result = 0;
			break;
		}
		test_load_delcount ++;
		current = current->next;
	}
	return result;
}

int test_load_cleanup_bucket(char* szbucket)
{
	struct RIACK_STRING_LINKED_LIST *list;
	RIACK_STRING bucket;
	int result;
	bucket.value = szbucket;
	bucket.len = strlen(szbucket);
	
	result = 1;
	if (riack_list_keys(test_client, bucket, &list) == RIACK_SUCCESS) {
		if (test_delete_all_keys(bucket, list) == 0) {
			result = 0;
		}
		riack_free_string_linked_list(test_client, &list);
	}
	return result;
}

int test_load_cleanup()
{
	test_load_delcount = 0;
	test_load_cleanup_bucket(RIAK_TEST_BUCKET_ANSWERS);
	test_load_cleanup_bucket(RIAK_TEST_BUCKET_COMMENTS);
	test_load_cleanup_bucket(RIAK_TEST_BUCKET_POSTS);
	test_load_cleanup_bucket(RIAK_TEST_BUCKET_USERS);
	test_load_cleanup_bucket(RIAK_TEST_BUCKET);

	printf("Delete %d keys\n", test_load_delcount);
	return 0;
}

int test_load_init()
{
	test_load_putcount = 0;
	// Assumes we are in riack root folder
	if (process_file("testdata/c_friendly/answers.json.out", RIAK_TEST_BUCKET_ANSWERS) &&
		process_file("testdata/c_friendly/comments.json.out", RIAK_TEST_BUCKET_COMMENTS) &&
		process_file("testdata/c_friendly/posts.json.out", RIAK_TEST_BUCKET_POSTS) /*&&
		process_file("testdata/c_friendly/users.json.out", RIAK_TEST_BUCKET_USERS)*/) {
		// Don't include users as that is a giant file, and it is not getting used in the tests yet
		// Now list all id's and delete
		return 0;
	}
	return 1;
}
