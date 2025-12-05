#include <stdio.h>
#include <string.h>
#include <openssl/sha.h>
#include <stdlib.h>
#include <time.h>

#define MAX_PASSWORD_LEN 4

int found = 0;

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

void brute_force(const char* target_hash_hex, const char* charset, char* prefix, int max_len) {
    if (found) return;

    int charset_len = strlen(charset);
    int current_len = strlen(prefix);

    if (current_len > 0) {
        unsigned char hash_bin[SHA256_DIGEST_LENGTH];
        char hash_hex[SHA256_DIGEST_LENGTH * 2 + 1];

        SHA256((unsigned char*)prefix, current_len, hash_bin);
        hash_to_hex(hash_bin, SHA256_DIGEST_LENGTH, hash_hex);

        if (strcmp(hash_hex, target_hash_hex) == 0) {
            printf("\n[SUCCESS] Password cracked! %s\n", prefix);
            found = 1;
        }
    }

    if (current_len < max_len) {
        for (int i = 0; i < charset_len; i++) {
            if (found) return;
            
            prefix[current_len] = charset[i];
            prefix[current_len + 1] = '\0';
            
            brute_force(target_hash_hex, charset, prefix, max_len);
            
            prefix[current_len] = '\0';
        }
    }
}

int main(int argc, char *argv[]) {
    char user_password[100];
    char target_hash[SHA256_DIGEST_LENGTH * 2 + 1];

    // Read input from stdin for automation
    if (scanf("%99s", user_password) != 1) {
        return 1;
    }

    if (strlen(user_password) > MAX_PASSWORD_LEN) {
        printf("Error: Password too long.\n");
        return 1;
    }

    calculate_sha256(user_password, target_hash);
    
    // Exact same charset as Parallel versions
    const char* charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    int charset_len = strlen(charset);

    printf("Execution Time: ... (Calculating)\n");
    clock_t start = clock();

    // Serial Loop (No OpenMP directives)
    for (int i = 0; i < charset_len; i++) {
        if (found) break;

        char local_prefix[MAX_PASSWORD_LEN + 1];
        local_prefix[0] = charset[i];
        local_prefix[1] = '\0';

        brute_force(target_hash, charset, local_prefix, MAX_PASSWORD_LEN);
    }

    clock_t end = clock();
    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;

    if (!found) printf("Password not found.\n");
    printf("Execution Time: %f seconds\n", time_spent);

    return 0;
}