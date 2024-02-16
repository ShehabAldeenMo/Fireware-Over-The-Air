
/**
 * @file RSA_Algo.h
 * @brief Header file for RSA encryption and decryption functions.
 *
 * This header file provides declarations for functions related to RSA encryption and decryption.
 * It includes functions for calculating modulus, Euler's totient function, generating public and private keys,
 * modular exponentiation, string-to-number and number-to-string conversions, encryption, decryption,
 * prime number checking, and generating random prime numbers.
 *
 * @author Reem Mahmoud
 * @date Feb 12, 2024
 */

#ifndef RSA_ALGO_H_
#define RSA_ALGO_H_

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>  // For PRIu64
#include <math.h>
#include <stdlib.h>
#include <time.h>

/**
 * @brief Calculate the product of two prime numbers to get the modulus.
 *
 * This function calculates the modulus (n) by multiplying two prime numbers.
 *
 * @param prime1 The first prime number.
 * @param prime2 The second prime number.
 * @return The modulus (n).
 */
uint64_t calculateModulus(uint64_t prime1, uint64_t prime2);

/**
 * @brief Calculate Euler's totient function for two prime numbers.
 *
 * This function calculates Euler's totient function (φ(n)) for two prime numbers.
 *
 * @param prime1 The first prime number.
 * @param prime2 The second prime number.
 * @return Euler's totient (φ(n)).
 */
uint64_t calculateEulerTotient(uint64_t prime1, uint64_t prime2);

/**
 * @brief Calculate the greatest common divisor using Euclid's algorithm.
 *
 * This function calculates the greatest common divisor (GCD) of two integers using Euclid's algorithm.
 *
 * @param a The first integer.
 * @param b The second integer.
 * @return The greatest common divisor.
 */
int gcd(int a, int b);

/**
 * @brief Generate the public exponent 'e' coprime to Euler's totient.
 *
 * This function generates the public exponent 'e' that is coprime to Euler's totient.
 *
 * @param eulerTotient Euler's totient.
 * @return The public exponent 'e'.
 */
uint64_t generatePublicExponent(uint64_t eulerTotient);

/**
 * @brief Modular exponentiation function to handle large power calculations.
 *
 * This function performs modular exponentiation to handle large power calculations efficiently.
 *
 * @param base The base.
 * @param exponent The exponent.
 * @param modulus The modulus.
 * @return The result of modular exponentiation.
 */
uint64_t modExp(uint64_t base, uint64_t exponent, uint64_t modulus);

/**
 * @brief Convert a string to a numerical value.
 *
 * This function converts a string to its numerical representation.
 *
 * @param message The input string.
 * @return The numerical representation of the string.
 */
uint64_t stringToNumber(const char* message);

/**
 * @brief Encrypt a message (string or numeric) using the public key (n, e).
 *
 * This function encrypts a message, either string or numeric, using the public key (n, e).
 *
 * @param originalMessage The original message (string or numeric).
 * @param modulus The modulus (n).
 * @param publicExponent The public exponent 'e'.
 * @param isString Whether the original message is a string.
 * @return The encrypted message.
 */
uint64_t encryptMessage(const void* originalMessage, uint64_t modulus, uint64_t publicExponent, int isString);

/**
 * @brief Calculate the modular multiplicative inverse using Extended Euclidean Algorithm.
 *
 * This function calculates the modular multiplicative inverse (a^(-1) mod m) using the Extended Euclidean Algorithm.
 *
 * @param a The base.
 * @param m The modulus.
 * @return The modular multiplicative inverse.
 */
uint64_t modInverse(uint64_t a, uint64_t m);

/**
 * @brief Calculate the private exponent 'd' for decryption.
 *
 * This function calculates the private exponent 'd' for decryption using Euler's totient and public exponent.
 *
 * @param eulerTotient Euler's totient.
 * @param publicExponent The public exponent 'e'.
 * @return The private exponent 'd'.
 */
uint64_t calculatePrivateExponent(uint64_t eulerTotient, uint64_t publicExponent);

/**
 * @brief Convert a numerical value to a string.
 *
 * This function converts a numerical value to its string representation.
 *
 * @param number The input numerical value.
 * @return The string representation of the numerical value.
 */
char* numberToString(uint64_t number);

/**
 * @brief Decrypt an encrypted message using the private key (n, d).
 *
 * This function decrypts an encrypted message using the private key (n, d).
 *
 * @param encryptedMessage The encrypted message.
 * @param modulus The modulus (n).
 * @param privateExponent The private exponent 'd'.
 * @param isString Whether the original message is a string.
 * @return The decrypted message as a dynamically allocated string.
 */
char* decryptMessage(uint64_t encryptedMessage, uint64_t modulus, uint64_t privateExponent, int isString);

/**
 * @brief Check if a number is prime.
 *
 * This function checks whether a given number is a prime number.
 *
 * @param num The number to be checked.
 * @return 1 if the number is prime, 0 otherwise.
 */
int isPrime(uint64_t num);

/**
 * @brief Generate a random prime number within a given range.
 *
 * This function generates a random prime number within the specified range [min, max].
 *
 * @param min The minimum value for the generated prime.
 * @param max The maximum value for the generated prime.
 * @return The randomly generated prime number.
 */
uint64_t generateRandomPrime(uint64_t min, uint64_t max);

/**
 * @brief Free memory allocated for the string.
 *
 * This function frees the dynamically allocated memory for a string.
 *
 * @param str The string to be freed.
 */
void freeString(char* str);

#endif /* RSA_ALGO_H_ */
