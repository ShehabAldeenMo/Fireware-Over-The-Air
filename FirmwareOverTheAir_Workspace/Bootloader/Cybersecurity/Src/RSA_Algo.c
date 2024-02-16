#include "RSA_Algo.h"

// Calculate the modulus (n) by multiplying two prime numbers.
uint64_t calculateModulus(uint64_t prime1, uint64_t prime2) {
    uint64_t modulus = prime1 * prime2;
    return modulus;
}

// Calculate Euler's totient function (φ(n)) for two prime numbers.
uint64_t calculateEulerTotient(uint64_t prime1, uint64_t prime2) {
    uint64_t eulerTotient = (prime1 - 1) * (prime2 - 1);
    return eulerTotient;
}

// Calculate the greatest common divisor (GCD) using Euclid's algorithm.
int gcd(int a, int b) {
    int temp;
    while (b != 0) {
        temp = a % b;
        if (temp == 0)
            return b;
        a = b;
        b = temp;
    }
    return a;  // Return the last non-zero remainder
}

// Generate the public exponent 'e' that is coprime to Euler's totient.
uint64_t generatePublicExponent(uint64_t eulerTotient) {
    uint64_t publicExponent = 2;
    while (publicExponent < eulerTotient && gcd(publicExponent, eulerTotient) != 1) {
        publicExponent++;
    }
    return publicExponent;
}

// Perform modular exponentiation to handle large power calculations efficiently.
uint64_t modExp(uint64_t base, uint64_t exponent, uint64_t modulus) {
    uint64_t result = 1;
    base = base % modulus;
    while (exponent > 0) {
        if (exponent % 2 == 1)
            result = (result * base) % modulus;
        exponent = exponent / 2;
        base = (base * base) % modulus;
    }
    return result;
}

// Convert a string to its numerical representation.
uint64_t stringToNumber(const char* message) {
    uint64_t result = 0;
    for (int i = 0; message[i] != '\0'; ++i) {
        result = result * 256 + (unsigned char)message[i];  // Assuming ASCII encoding
    }
    return result;
}

// Encrypt a message, either string or numeric, using the public key (n, e).
uint64_t encryptMessage(const void* originalMessage, uint64_t modulus, uint64_t publicExponent, int isString) {
    if (isString) {
        uint64_t numericalMessage = stringToNumber((const char*)originalMessage);
        return modExp(numericalMessage, publicExponent, modulus);
    } else {
        return modExp(*(const uint64_t*)originalMessage, publicExponent, modulus);
    }
}

// Calculate the modular multiplicative inverse using Extended Euclidean Algorithm.
uint64_t modInverse(uint64_t a, uint64_t m) {
    uint64_t m0 = m, t, q;
    uint64_t x0 = 0, x1 = 1;

    if (m == 1)
        return 0;

    while (a > 1) {
        q = floor(a / m);
        t = m;
        m = fmod(a, m);
        a = t;
        t = x0;
        x0 = x1 - q * x0;
        x1 = t;
    }

    if (x1 < 0)
        x1 += m0;

    return x1;
}

// Calculate the private exponent 'd' for decryption using Euler's totient and public exponent.
uint64_t calculatePrivateExponent(uint64_t eulerTotient, uint64_t publicExponent) {
    uint64_t privateExponent = modInverse(publicExponent, eulerTotient);
    return privateExponent;
}

// Convert a numerical value to its string representation.
char* numberToString(uint64_t number) {
    int size = log2(number) / 8 + 1;  // Assuming 8 bits per character
    char* result = (char*)malloc(size + 1);
    for (int i = size - 1; i >= 0; --i) {
        result[i] = (char)(number % 256);
        number /= 256;
    }
    result[size] = '\0';
    return result;
}

// Decrypt an encrypted message using the private key (n, d).
char* decryptMessage(uint64_t encryptedMessage, uint64_t modulus, uint64_t privateExponent, int isString) {
    uint64_t numericalMessage = modExp(encryptedMessage, privateExponent, modulus);

    if (isString) {
        return numberToString(numericalMessage);
    } else {
        char* result = (char*)malloc(sizeof(char) * 32); // Adjust size as needed
        snprintf(result, 32, "%" PRIu64, numericalMessage);
        return result;
    }
}

// Check if a number is prime.
int isPrime(uint64_t num) {
    if (num <= 1) return 0;
    for (uint64_t i = 2; i * i <= num; ++i) {
        if (num % i == 0) return 0;
    }
    return 1;
}

// Generate a random prime number within a given range.
uint64_t generateRandomPrime(uint64_t min, uint64_t max) {
    uint64_t p, q;
    do {
        p = rand() % (max - min + 1) + min;
        q = rand() % (max - min + 1) + min;
    } while (p == q || !isPrime(p) || !isPrime(q));
    return p;
}

// Free memory allocated for the string.
void freeString(char* str) {
    free(str);
}
