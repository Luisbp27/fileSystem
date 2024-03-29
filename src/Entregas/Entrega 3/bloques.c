#include "bloques.h"
#include "semaforo_mutex_posix.h"

static int descriptor = 0;
static sem_t *mutex;
static unsigned int in_critic_sec = 0;

#if MMAP
static int tamSFM; // shared memory size
static void *ptrSFM; // shared memory pointer

void *do_mmap(int fd) {
    struct stat st;
    void *ptr;
    fstat(fd, &st);
    tamSFM = st.st_size;
    if ((ptr = mmap(NULL, tamSFM, PROT_WRITE, MAP_SHARED, fd, 0)) == (void *)-1) {

    }
    return ptr;
}
#endif


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

    if (descriptor > 0) {
        close(descriptor);
    }

    // Open file descriptor
    descriptor = open(path, O_RDWR | O_CREAT, RW_PERMS);

    if (descriptor == FAILURE) {
        perror("Could not open the file system");
    }

#if MMAP
    ptrSFM = do_mmap(descriptor);
#endif

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
    descriptor = close(descriptor);

    // Try to close the file system
    if (descriptor == FAILURE) {
        perror("Could not close the file system");
        return FAILURE;
    }

#if MMAP
    msync(ptrSFM, tamSFM, MS_SYNC | MS_INVALIDATE);
    munmap(ptrSFM, tamSFM);
#endif

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
#if MMAP
    unsigned int pos = nbloque * BLOCKSIZE;
    memcpy(ptrSFM + pos, buf, BLOCKSIZE);
    return BLOCKSIZE;
#else
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
#endif
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
#if MMAP
    unsigned int pos = nbloque * BLOCKSIZE;
    memcpy(buf, ptrSFM + pos, BLOCKSIZE);
    return BLOCKSIZE;
#else
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
#endif
}

/**
 * Method to manage the critical section of the file system. Concretely, it controls
 * the process inside the critical section and manages the semaphore.
*/
void mi_waitSem() {
    if (in_critic_sec == 0) {
        waitSem(mutex);
    }

    in_critic_sec++;
}

/**
 * Method to manage the critical section of the file system. Concretely, it controls
 * the process inside the critical section and manages the semaphore.
*/
void mi_signalSem() {
    in_critic_sec--;

    if (in_critic_sec == 0) {
        signalSem(mutex);
    }
}