typedef struct{
	char * data;
	int index;
	int length;
} upload_buffer;

void handle_input(char * url, char * input);
int handle_upload(void * ptr, size_t size, size_t nmemb, void * userdata);
