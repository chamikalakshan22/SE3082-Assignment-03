#include <stdio.h>
#include <string.h>
#include <openssl/sha.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h> // Required for OpenMP

#define MAX_PASSWORD_LEN 4

// Shared flag: 1 if ANY thread finds the password
// 'volatile' tells the compiler that this value can change at any time
volatile int found = 0;

// Helper: Convert binary hash to hex string
void hash_to_hex(unsigned char* hash, int hash_len, char* output_str) {
    for(int i = 0; i < hash_len; i++) {
        sprintf(output_str + (i * 2), "%02x", hash[i]);
    }
    output_str[hash_len * 2] = 0;
}

// Helper: Calculate SHA256 for setting the target
void calculate_sha256(const char* input, char* output_hex) {
    unsigned char hash_bin[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)input, strlen(input), hash_bin);
    hash_to_hex(hash_bin, SHA256_DIGEST_LENGTH, output_hex);
}

// Recursive Brute Force Function
void brute_force(const char* target_hash_hex, const char* charset, char* prefix, int max_len) {
    // Optimization: Stop immediately if another thread has already found it
    if (found) return; 

    int charset_len = strlen(charset);
    int current_len = strlen(prefix);

    if (current_len > 0) {
        unsigned char hash_bin[SHA256_DIGEST_LENGTH];
        char hash_hex[SHA256_DIGEST_LENGTH * 2 + 1];

        SHA256((unsigned char*)prefix, current_len, hash_bin);
        hash_to_hex(hash_bin, SHA256_DIGEST_LENGTH, hash_hex);

        if (strcmp(hash_hex, target_hash_hex) == 0) {
            // CRITICAL SECTION: Only one thread can execute this block at a time
            // This prevents messy printing if two threads find it (rare)
            #pragma omp critical
            {
                if (!found) { // Double check inside critical section
                    printf("\n[SUCCESS] Password cracked! %s\n", prefix);
                    printf("Found by Thread ID: %d\n", omp_get_thread_num());
                    found = 1;
                }
            }
        }
    }

    if (current_len < max_len && !found) {
        for (int i = 0; i < charset_len; i++) {
            if (found) return; // Check again to exit recursion early
            
            prefix[current_len] = charset[i];
            prefix[current_len + 1] = '\0';
            
            brute_force(target_hash_hex, charset, prefix, max_len);
            
            prefix[current_len] = '\0'; // Backtrack
        }
    }
}

int main(int argc, char *argv[]) {
    char user_password[100];
    char target_hash[SHA256_DIGEST_LENGTH * 2 + 1];

    // 1. Get Thread Count from command line (Default to 4)
    int thread_count = 4;
    if (argc > 1) {
        thread_count = atoi(argv[1]);
    }

    // 2. Get User Input
    printf("------------------------------------------------\n");
    printf("OPENMP BRUTE FORCE CRACKER (Letters + Numbers)\n");
    printf("Enter password to crack (Max 4 chars): ");
    scanf("%99s", user_password);

    if (strlen(user_password) > MAX_PASSWORD_LEN) {
        printf("Error: Password too long for this demo.\n");
        return 1;
    }

    calculate_sha256(user_password, target_hash);
    
    // 3. Setup OpenMP
    omp_set_num_threads(thread_count);
    
    // Charset: a-z, A-Z, 0-9
    const char* charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    int charset_len = strlen(charset);

    printf("Target Hash: %s\n", target_hash);
    printf("Threads: %d\n", thread_count);
    printf("Charset Size: %d characters\n", charset_len);
    printf("Starting...\n");

    // 4. Start Timer (OpenMP has its own high-precision timer)
    double start_time = omp_get_wtime();

    // --- PARALLEL REGION ---
    #pragma omp parallel for schedule(dynamic, 4)
    for (int i = 0; i < charset_len; i++) {
        // Skip work if password is already found
        if (found) continue;

        // PRIVATE buffer for each thread.
        char local_prefix[MAX_PASSWORD_LEN + 1];
        
        local_prefix[0] = charset[i];
        local_prefix[1] = '\0';

        // Start recursion for this starting letter
        brute_force(target_hash, charset, local_prefix, MAX_PASSWORD_LEN);
    }

    double end_time = omp_get_wtime();

    if (!found) printf("Password not found.\n");
    printf("Execution Time: %f seconds\n", end_time - start_time);

    return 0;
}