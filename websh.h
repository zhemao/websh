#include "map.h"

typedef struct{
	char * data;
	int index;
	int length;
} upload_buffer;

map * headers;

/* take input for a POST or PUT command from linenoise */
void handle_input(char * url, char * input);
/* callback for PUT command */
int handle_upload(void * ptr, size_t size, size_t nmemb, void * userdata);
