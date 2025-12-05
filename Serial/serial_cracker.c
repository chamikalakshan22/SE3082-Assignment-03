#include <stdio.h>
#include <string.h>
#include <openssl/sha.h>
#include <stdlib.h>
#include <time.h>

// Configuration using #define to avoid Variable Length Array warnings
#define MAX_PASSWORD_LEN 4

// Global flag to stop recursion if found
int found = 0;

// Helper: Convert binary hash to hex string
void hash_to_hex(unsigned char* hash, int hash_len, char* output_str) {
    for(int i = 0; i < hash_len; i++) {
        sprintf(output_str + (i * 2), "%02x", hash[i]);
    }
    output_str[hash_len * 2] = 0;
}

// Helper: Calculate SHA256 for a given string (Used to set the target)
void calculate_sha256(const char* input, char* output_hex) {
    unsigned char hash_bin[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)input, strlen(input), hash_bin);
    hash_to_hex(hash_bin, SHA256_DIGEST_LENGTH, output_hex);
}

// Recursive Brute Force Function
void brute_force(const char* target_hash_hex, const char* charset, char* prefix, int max_len) {
    if (found) return; // Stop if already found

    int charset_len = strlen(charset);
    int current_len = strlen(prefix);

    // Check the current candidate
    if (current_len > 0) {
        unsigned char hash_bin[SHA256_DIGEST_LENGTH];
        char hash_hex[SHA256_DIGEST_LENGTH * 2 + 1];

        SHA256((unsigned char*)prefix, current_len, hash_bin);
        hash_to_hex(hash_bin, SHA256_DIGEST_LENGTH, hash_hex);

        if (strcmp(hash_hex, target_hash_hex) == 0) {
            printf("\n[SUCCESS] Password cracked! The password is: %s\n", prefix);
            found = 1;
            return;
        }
    }

    // Recursive step: add another character
    if (current_len < max_len) {
        for (int i = 0; i < charset_len; i++) {
            if (found) return; // Check again inside loop
            
            prefix[current_len] = charset[i];
            prefix[current_len + 1] = '\0';
            
            brute_force(target_hash_hex, charset, prefix, max_len);
            
            prefix[current_len] = '\0'; // Backtrack
        }
    }
}

int main() {
    char user_password[100];
    char target_hash[SHA256_DIGEST_LENGTH * 2 + 1];
    
    // 1. Get Input from User
    printf("------------------------------------------------\n");
    printf("ENTER A PASSWORD TO CRACK (Max 4 chars, case-sensitive): ");
    scanf("%99s", user_password);

    // Validate length for this assignment
    if (strlen(user_password) > MAX_PASSWORD_LEN) {
        printf("Error: Please enter %d characters or less for this demonstration.\n", MAX_PASSWORD_LEN);
        return 1;
    }

    // 2. Calculate the Hash of the user's password (The 'Target')
    calculate_sha256(user_password, target_hash);
    
    printf("\nTarget Password: %s\n", user_password);
    printf("Target Hash:     %s\n", target_hash);
    printf("------------------------------------------------\n");
    printf("Starting Brute Force Attack (Letters + Numbers)...\n");

    // Search Configuration - UPDATED to include 0-9
    const char* charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    
    char prefix_buffer[MAX_PASSWORD_LEN + 1];
    prefix_buffer[0] = '\0';

    // 3. Start Timer and Brute Force
    clock_t start = clock();

    brute_force(target_hash, charset, prefix_buffer, MAX_PASSWORD_LEN);

    clock_t end = clock();
    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;

    if (!found) {
        printf("Password not found (check your charset or length settings).\n");
    }
    
    printf("Execution Time: %f seconds\n", time_spent);

    return 0;
}