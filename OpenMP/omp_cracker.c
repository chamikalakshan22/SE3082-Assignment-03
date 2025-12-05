#include <stdio.h>
#include <string.h>
#include <openssl/sha.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#define MAX_PASSWORD_LEN 4

// Atomic flag for faster checking
_Atomic int found = 0;

void hash_to_hex(unsigned char* hash, int hash_len, char* output_str) {
    for(int i = 0; i < hash_len; i++) {
        sprintf(output_str + (i * 2), "%02x", hash[i]);
    }
    output_str[hash_len * 2] = 0;
}

void calculate_sha256(const char* input, char* output_hex) {
    unsigned char hash_bin[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)input, strlen(input), hash_bin);
    hash_to_hex(hash_bin, SHA256_DIGEST_LENGTH, output_hex);
}

void brute_force(const char* target_hash_hex, const char* charset, int charset_len, char* prefix, int max_len) {
    if (found) return;

    int current_len = strlen(prefix);

    if (current_len > 0) {
        unsigned char hash_bin[SHA256_DIGEST_LENGTH];
        char hash_hex[SHA256_DIGEST_LENGTH * 2 + 1];

        SHA256((unsigned char*)prefix, current_len, hash_bin);
        hash_to_hex(hash_bin, SHA256_DIGEST_LENGTH, hash_hex);

        if (strcmp(hash_hex, target_hash_hex) == 0) {
            #pragma omp critical
            {
                if (!found) { 
                    printf("\n[SUCCESS] Password cracked! %s\n", prefix);
                    printf("Found by Thread ID: %d\n", omp_get_thread_num());
                    // We set the flag here. Other threads will see it eventually (relaxed consistency).
                    found = 1;
                }
            }
        }
    }

    if (current_len < max_len && !found) {
        for (int i = 0; i < charset_len && !found; i++) {
            prefix[current_len] = charset[i];
            prefix[current_len + 1] = '\0';
            
            brute_force(target_hash_hex, charset, charset_len, prefix, max_len);
            
            prefix[current_len] = '\0';
        }
    }
}

int main(int argc, char *argv[]) {
    char user_password[100];
    char target_hash[SHA256_DIGEST_LENGTH * 2 + 1];

    int thread_count = 4;
    if (argc > 1) {
        thread_count = atoi(argv[1]);
    }

    // Hardcode input for automation compatibility
    // If running manually, you can change this, but for the script we use stdin
    if (scanf("%99s", user_password) != 1) {
        return 1;
    }

    if (strlen(user_password) > MAX_PASSWORD_LEN) {
        printf("Error: Password too long.\n");
        return 1;
    }

    calculate_sha256(user_password, target_hash);
    
    omp_set_num_threads(thread_count);
    
    const char* charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    int charset_len = strlen(charset);

    printf("Execution Time: ... (Calculating)\n"); // Placeholder for regex parsing
    double start_time = omp_get_wtime();

    #pragma omp parallel for schedule(guided, 2)
    for (int i = 0; i < charset_len; i++) {
        if (found) continue;

        char local_prefix[MAX_PASSWORD_LEN + 1];
        local_prefix[0] = charset[i];
        local_prefix[1] = '\0';

        brute_force(target_hash, charset, charset_len, local_prefix, MAX_PASSWORD_LEN);
    }

    double end_time = omp_get_wtime();

    if (!found) printf("Password not found.\n");
    // Print in the exact format our Python script looks for
    printf("Execution Time: %f seconds\n", end_time - start_time);

    return 0;
}