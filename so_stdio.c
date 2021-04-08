#include "so_stdio.h"

/**
 * Function to open the file 
 * Initialize the struct
 */
SO_FILE *so_fopen(const char *pathname, const char *mode)
{
	int fd = -1;
	SO_FILE *openFile = (SO_FILE *)malloc(sizeof(SO_FILE));

	if (strncmp(mode, "r", 1) == 0 && strlen(mode) == 1) {
		fd = open(pathname, O_RDONLY);
	} else if (strncmp(mode, "r+", 2) == 0 && strlen(mode) == 2) {
		fd = open(pathname, O_RDWR);
	} else if (strncmp(mode, "w", 1) == 0 && strlen(mode) == 1) {
		fd = open(pathname, O_WRONLY | O_CREAT | O_TRUNC, 0777);
	} else if (strncmp(mode, "w+", 2) == 0 && strlen(mode) == 2) {
		fd = open(pathname, O_RDWR | O_CREAT | O_TRUNC, 0777);
	} else if (strncmp(mode, "a", 1) == 0 && strlen(mode) == 1) {
		fd = open(pathname, O_APPEND | O_WRONLY | O_CREAT, 0777);
	} else if (strncmp(mode, "a+", 2) == 0 && strlen(mode) == 2) {
		fd = open(pathname, O_RDWR | O_CREAT | O_APPEND, 0777);
	} else {
		free(openFile);
		return NULL;
	}

	if (fd < 0) {
		free(openFile);
		return NULL;
	}

	openFile->fd = fd;
	memset(openFile->buffer, 0, BUFFERSIZE);
	openFile->curr_buff_len = 0;
	openFile->index_read = 0;
	openFile->index_write = 0;
	openFile->flag = -1;
	openFile->err = 0;
	openFile->eof = 0;
	openFile->index_curr = 0;

	return openFile;
}

// return the file descriptor
int so_fileno(SO_FILE *stream)
{
	return stream->fd;
}

// close the fd and free memory
int so_fclose(SO_FILE *stream)
{
	int ret = 0;
	int flush_err = 0;

	if (stream == NULL) {
		stream->err = -1;
		return SO_EOF;
	}

	flush_err = so_fflush(stream);
	ret = close(stream->fd);

	if (ret < 0 || flush_err == SO_EOF) {
		stream->err = -1;
		free(stream);
		return SO_EOF;
	}

	stream->err = 0;
	free(stream);

	return 0;
}

/**
 * Helper function for so_fgetc
 */
ssize_t xread(SO_FILE *f, size_t count)
{
	size_t bytes_read = 0;
	ssize_t bytes_read_now = 0;

		while (bytes_read < count) {
			bytes_read_now = read(f->fd, f->buffer + bytes_read,
					count - bytes_read);

			if (bytes_read_now == 0) { /* EOF */
				f->eof = -1;
				return bytes_read_now;
			}

			if (bytes_read_now < 0) { /* I/O error */
				f->eof = -1;
				f->err = -1;
				return SO_EOF;
			}

			if (bytes_read_now < count)
				return bytes_read_now;

			bytes_read += bytes_read_now;
		}

		return bytes_read;
}

/**
 * Helper function for so_fputc
 */
ssize_t xwrite(SO_FILE *f, size_t count)
{
	size_t bytes_written = 0;

	while (bytes_written < count) {
		ssize_t bytes_written_now = write(f->fd, f->buffer + bytes_written,
				count - bytes_written);

		if (bytes_written_now <= 0) { /* I/O error */
			f->err = -1;
			return SO_EOF;
		}

		bytes_written += bytes_written_now;
	}

	return bytes_written;
}

/**
 * Reads a character from the file
 * Returns the character casted to int
 * If error -> return -1
 */
int so_fgetc(SO_FILE *stream)
{
	size_t rd_bytes_now = 0;
	int ret = 0;

	if (stream->flag == 1)
		so_fflush(stream);

	if (stream->curr_buff_len == 0 ||
			stream->curr_buff_len - 1 < stream->index_read) {
		rd_bytes_now = xread(stream, BUFFERSIZE);

		// check for error and set the error flag
		if (rd_bytes_now == -1) {
			stream->err = -1;
			return SO_EOF;
		}

		if (stream->eof == -1)
			return SO_EOF;
		// current length is the no of bytes read
		stream->curr_buff_len = rd_bytes_now;
		stream->index_read = 0;
	}
	// set the flag to read
	stream->flag = 0;
	ret = (int)stream->buffer[stream->index_read];
	// increment the read index
	stream->index_read++;

	return ret;
}

/**
 * Writes a character in the file
 * Returns the character written
 * If error -> return -1
 */
int so_fputc(int c, SO_FILE *stream)
{
	size_t bytes_written = 0;
	int ret = 0;
	unsigned char c_write;

	if (stream->flag == 0)
		so_fflush(stream);

	// if the buffer is full, reset
	if (stream->curr_buff_len == BUFFERSIZE) {
		bytes_written == xwrite(stream, BUFFERSIZE);

		// check for error and set the error flag
		if (bytes_written == -1) {
			stream->err = -1;
			return SO_EOF;
		}

		memset(stream->buffer, 0, BUFFERSIZE);
		stream->curr_buff_len = 0;
		stream->index_write = 0;
	}
	// set the flag to write
	stream->flag = 1;
	// store the character in the buffer
	c_write = (unsigned char) c;
	stream->buffer[stream->index_write] = c_write;

	ret = stream->buffer[stream->index_write];

	// increment the write index and the length of the buffer
	stream->curr_buff_len++;
	stream->index_write++;

	return ret;
}

/**
 * Reads nmemb elements of size 'size' each
 * Store the data in ptr and check for error and eof
 * Returns the number of elements read
 */
size_t so_fread(void *ptr, size_t size, size_t nmemb, SO_FILE *stream)
{
	int i, j;
	unsigned char get;
	int offset = 0;

	for (i = 0; i < nmemb; i++) {
		for (j = 0; j < size; j++) {
			get = (unsigned char)so_fgetc(stream);

			// check if error and set the error flag
			if (stream->err == -1) {
				stream->err = -1;
				return 0;
			}
			// check if eof and set the flag
			if  (stream->eof == -1)
				return i;
			// store in ptr at the correct offset
			memcpy(ptr + offset, &get, sizeof(unsigned char));
			offset += sizeof(unsigned char);
		}
	}

	// store the current index in buffer
	stream->index_curr = i;

	return i;
}

/**
 * Writes nmemb elements of size 'size' each
 * Get the data from ptr and check for error and eof
 * Returns the number of elements written
 */
size_t so_fwrite(const void *ptr, size_t size, size_t nmemb, SO_FILE *stream)
{
	int i, j;
	unsigned char c;
	// cast to further access data
	unsigned char *aux = (unsigned char *)ptr;

	for (i = 0; i < nmemb; i++) {
		for (j = 0; j < size; j++) {
			// get data from address
			c = *aux;
			so_fputc(c, stream);

			// check if error and set the error flag
			if (stream->err == -1) {
				stream->err = -1;
				return 0;
			}
			aux++;
		}
	}

	// store the current index in buffer
	stream->index_curr = i;

	return i;
}

/**
 * The data from the buffer is written in the file
 * If the last operation stored in the flag field is write
 * If error -> return -1
 */
int so_fflush(SO_FILE *stream)
{
	ssize_t bytes_write = 0;

	// if the last op is write, write the data in buffer in file
	if (stream->flag == 1) {
		bytes_write = xwrite(stream, stream->curr_buff_len);

		// check for error and set the flag
		if (bytes_write < 0) {
			stream->err = -1;
			return SO_EOF;
		}
	}

	// reset the buffer and index
	memset(stream->buffer, 0, BUFFERSIZE);
	stream->index_read = 0;
	stream->index_write = 0;
	stream->curr_buff_len = 0;

	return 0;
}

/**
 * Moves the index
 * If error -> return -1
 */
int so_fseek(SO_FILE *stream, long offset, int whence)
{
	off_t loc;

	so_fflush(stream);

	loc = lseek(stream->fd, offset, whence);

	if (loc == (off_t) -1) {
		stream->err = -1;
		return SO_EOF;
	}

	// update the curent index
	stream->index_curr = (int) loc;

	return 0;
}

// Returns the current position in file
long so_ftell(SO_FILE *stream)
{
	if (stream == NULL)
		return SO_EOF;

	return stream->index_curr;
}

// Returns the eof field
int so_feof(SO_FILE *stream)
{
	return stream->eof;
}

// Returns the error field
int so_ferror(SO_FILE *stream)
{
	return stream->err;
}

SO_FILE *so_popen(const char *command, const char *type)
{
	return NULL;
}

int so_pclose(SO_FILE *stream)
{
	return 0;
}
