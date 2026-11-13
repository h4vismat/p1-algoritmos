#ifndef PIGGYBANK_BANK_H
#define PIGGYBANK_BANK_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

typedef struct Bank Bank;
typedef enum { COIN_50 = 50, COIN_100 = 100, COIN_200 = 200} Coin;
typedef enum { TX_DEPOSIT = 1, TX_WITHDRAW = 2} TxKind;
typedef struct { uint64_t sequence; TxKind kind; uint64_t counts[3]; } Transaction;
typedef struct { uint64_t sequence; uint64_t counts[3]; } BankImage;

typedef enum {
    BANK_OK = 0,
    BANK_INVALID_ARGUMENT,
    BANK_INVALID_COIN,
    BANK_INVALID_TRANSACTION,
    BANK_SEQUENCE_ERROR,
    BANK_INSUFFICIENT_COINS,
    BANK_OVERFLOW,
    BANK_EMPTY_TRANSACTION
} BankResult;

/* Creates a Bank struct. */
Bank *bank_create(void);

/* Releases a Bank from memory. */
void bank_destroy(Bank *bank);

/* Validates if a given transaction can be applied to a Bank. */
BankResult bank_validate(const Bank *bank, const Transaction *tx);

/* Applies a given transaction to a Bank. */
BankResult bank_apply(Bank *bank, const Transaction *tx);

bool bank_balance(const Bank *bank, uint64_t *cents);
void bank_export(const Bank *bank, BankImage *image);
bool bank_import(Bank *bank, const BankImage *image);
uint64_t bank_sequence(const Bank *bank);
uint64_t bank_coin_count(const Bank *bank, Coin coin);

bool bank_image_balance(const BankImage *image, uint64_t *cents);
bool bank_image_coint_count(const BankImage *image, uint64_t *count);

#endif
