#include "bloques.h"
#include "semaforo_mutex_posix.h"

static int descriptor = 0;
static sem_t *mutex;
static unsigned int inside_sc = 0;

/**
 * Method to mount the virtual device, and since it is a file,
 * that action will consist of opening it.
 *
 * @param path Path to the file system
 *
 * @return The file descriptor of the file system, or -1 if there was an error
 */
int bmount(const char *path) {
    umask(000);
    // Open file descriptor
    descriptor = open(path, O_RDWR | O_CREAT, RW_PERMS);

    if (descriptor == FAILURE) {
        perror("Could not open the file system");
    }

    // Initialize the semaphore
    if (!mutex) {
        mutex = initSem();

        if (mutex == SEM_FAILED) {
            return FAILURE;
        }
    }

    return descriptor;
}

/**
 * Unmount the virtual device. Basically calls the close() function
 * to release the file descriptor.
 *
 * @return 0 if the file was closed successfully, or -1 otherwise.
 */
int bumount() {
    // Try to close the file system
    if (close(descriptor) == FAILURE) {
        perror("Could not close the file system");
        return FAILURE;
    }

    // Destroy the semaphore
    deleteSem();

    return SUCCESS;
}

/**
 * Writes 1 block to the virtual device, in the physical block
 * specified by n_block.
 *
 * @param buf Buffer to the contents to write. It has to be
 * the size of a block.
 *
 * @return The number of bytes written, or -1 if there was an error.
 */
int bwrite(unsigned int nbloque, const void *buf) {
    // Allocate the pointer
    if (lseek(descriptor, nbloque * BLOCKSIZE, SEEK_SET) == FAILURE) {
        perror("Error while positioning the file pointer");
        return FAILURE;
    }

    // Write the block
    size_t bytes_written = write(descriptor, buf, BLOCKSIZE);

    // If the writing has gone wrong
    if (bytes_written != BLOCKSIZE) {
        perror("Error while writing to the file system");
        return FAILURE;
    }

    return bytes_written;
}

/**
 * Reads 1 block from the virtual device, which corresponds
 * to the physical block specified by n_block.
 *
 * @param buf Buffer to the contents to read. It has to be
 * the size of a block.
 *
 * @return The number of bytes read, or -1 if there was an error.
 */
int bread(unsigned int nbloque, void *buf) {
    // Allocate the pointer
    if (lseek(descriptor, nbloque * BLOCKSIZE, SEEK_SET) == FAILURE) {
        perror("Error when positioning the file pointer");
        return FAILURE;
    }

    // Read the block
    size_t bytes = read(descriptor, buf, BLOCKSIZE);

    // If the reading has gone wrong
    if (bytes != BLOCKSIZE) {
        perror("Error when reading to the block");
        return FAILURE;
    }

    return bytes;
}

/**
 * This method allow us to call the waitSem() method, which are defined 
 * in semaforo_mutex_posix.c. Therefore, it will be possible to use the 
 * semaphore in the other files.
 * 
*/
void mi_waitSem() {
    if (!inside_sc) {
        waitSem(mutex);
    }
    inside_sc++;
}

/**
 * This method allow us to call the signalSem() method, which are defined 
 * in semaforo_mutex_posix.c. Therefore, it will be possible to use the 
 * semaphore in the other files.
 * 
*/
void mi_signalSem() {
    inside_sc--;
    if (!inside_sc) {
        signalSem(mutex);
    }
}