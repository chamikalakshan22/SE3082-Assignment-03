#include <stdio.h>
#include <string.h>
#include <openssl/sha.h>
#include <stdlib.h>
#include <mpi.h> 

#define MAX_PASSWORD_LEN 4

// HConvert binary hash to hex string
void hash_to_hex(unsigned char* hash, int hash_len, char* output_str) {
    for(int i = 0; i < hash_len; i++) {
        sprintf(output_str + (i * 2), "%02x", hash[i]);
    }
    output_str[hash_len * 2] = 0;
}

// Calculate SHA256
void calculate_sha256(const char* input, char* output_hex) {
    unsigned char hash_bin[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)input, strlen(input), hash_bin);
    hash_to_hex(hash_bin, SHA256_DIGEST_LENGTH, output_hex);
}

// Recursive Brute Force Function
int brute_force(const char* target_hash_hex, const char* charset, char* prefix, int max_len, int rank, char* found_password) {
    int charset_len = strlen(charset);
    int current_len = strlen(prefix);

    if (current_len > 0) {
        unsigned char hash_bin[SHA256_DIGEST_LENGTH];
        char hash_hex[SHA256_DIGEST_LENGTH * 2 + 1];

        SHA256((unsigned char*)prefix, current_len, hash_bin);
        hash_to_hex(hash_bin, SHA256_DIGEST_LENGTH, hash_hex);

        if (strcmp(hash_hex, target_hash_hex) == 0) {
            strcpy(found_password, prefix);
            return 1; // Found!
        }
    }

    if (current_len < max_len) {
        for (int i = 0; i < charset_len; i++) {
            prefix[current_len] = charset[i];
            prefix[current_len + 1] = '\0';
            
            if (brute_force(target_hash_hex, charset, prefix, max_len, rank, found_password)) {
                return 1; // Propagate found
            }
            
            prefix[current_len] = '\0'; // Backtrack
        }
    }
    return 0; // Not found
}

int main(int argc, char *argv[]) {
    int rank, size;
    char target_hash[SHA256_DIGEST_LENGTH * 2 + 1];
    char user_password[100];
    
    // 1. Initialize MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // My ID
    MPI_Comm_size(MPI_COMM_WORLD, &size); // Total Processes

    // 2. Rank 0 handles User Input (Only one process interacts with keyboard)
    if (rank == 0) {
        printf("------------------------------------------------\n");
        printf("MPI BRUTE FORCE CRACKER (Letters + Numbers)\n");
        printf("Enter password to crack (Max 4 chars): ");
        scanf("%99s", user_password);

        if (strlen(user_password) > MAX_PASSWORD_LEN) {
            printf("Error: Password too long.\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        // Calculate hash
        calculate_sha256(user_password, target_hash);
        printf("Target Hash: %s\n", target_hash);
        printf("Processes: %d\n", size);
        printf("Starting...\n");
    }

    // 3. Broadcast the Target Hash to ALL processes
    MPI_Bcast(target_hash, SHA256_DIGEST_LENGTH * 2 + 1, MPI_CHAR, 0, MPI_COMM_WORLD);

    // Charset: a-z, A-Z, 0-9
    const char* charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    int charset_len = strlen(charset);

    // 4. Start Timer
    double start_time = MPI_Wtime();

    // 5. Distribute Work (Round Robin)
    char found_password[MAX_PASSWORD_LEN + 1] = "";
    int local_found = 0;
    
    for (int i = rank; i < charset_len && !local_found; i += size) {
        char local_prefix[MAX_PASSWORD_LEN + 1];
        local_prefix[0] = charset[i];
        local_prefix[1] = '\0';

        local_found = brute_force(target_hash, charset, local_prefix, MAX_PASSWORD_LEN, rank, found_password);
    }

    // 6. Gather results from all processes
    int global_found = 0;
    MPI_Allreduce(&local_found, &global_found, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
    
    double end_time = MPI_Wtime();
    
    if (global_found) {
        // Gather the password from whoever found it
        if (local_found && rank == 0) {
            printf("\n[SUCCESS] Password cracked! %s\n", found_password);
            printf("Found by MPI Process Rank: %d\n", rank);
        } else if (local_found) {
            printf("\n[SUCCESS] Password cracked! %s\n", found_password);
            printf("Found by MPI Process Rank: %d\n", rank);
        }
    }
    
    if (rank == 0) {
        if (!global_found) {
            printf("Password not found.\n");
        }
        printf("Execution Time: %f seconds\n", end_time - start_time);
    }

    MPI_Finalize();
    return 0;
}