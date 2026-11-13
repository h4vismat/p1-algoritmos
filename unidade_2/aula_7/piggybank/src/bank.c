#include "bank.h"

#include <limits.h>
#include <notcurses/notcurses.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct Bank { BankImage image; };

static int coin_index(Coin c) {
    switch (c) {
        case COIN_50: return 0;
        case COIN_100: return 1;
        case COIN_200: return 2;
        default: return -1;
    }
}

Bank *bank_create(void) { return calloc(1, sizeof(Bank)); }

void bank_destroy(Bank *bank) { free(bank); }

BankResult bank_validate(const Bank *bank, const Transaction *tx) {
    static const uint64_t denominations[] = {50, 100, 200};

    if (!bank || !tx)
        return BANK_INVALID_ARGUMENT;

    if (tx->kind != TX_DEPOSIT && tx->kind != TX_WITHDRAW)
        return BANK_INVALID_TRANSACTION;

    if (bank->image.sequence == UINT_MAX || tx->sequence != bank->image.sequence + 1)
        return BANK_SEQUENCE_ERROR;

    if (tx->counts[0] == 0 && tx->counts[1] == 0 && tx->counts[2] == 0)
        return BANK_EMPTY_TRANSACTION;

    if (tx->kind == TX_WITHDRAW) {
        for (size_t i = 0; i < 3; ++i) {
            if (tx->counts[i] > bank->image.counts[i])
                return BANK_INSUFFICIENT_COINS;
        }

        return BANK_OK;
    }

    uint64_t total;
    if (!bank_balance(bank, &total))
        return BANK_OVERFLOW;

    for (size_t i = 0; i < 3; ++i) {
        if (tx->counts[i] > (UINT64_MAX - total) / denominations[i])
            return BANK_OVERFLOW;

        total += tx->counts[i] * denominations[i];
    }

    return BANK_OK;
}

BankResult bank_apply(Bank *bank, const Transaction *tx) {
    BankResult result = bank_validate(bank, tx);
    if (result != BANK_OK) return result;

    for (size_t i = 0; i < 3; i++) {
        if (tx->kind == TX_DEPOSIT) {
            bank->image.counts[i] += tx->counts[i];
        } else {
            bank->image.counts[i] -= tx->counts[i];
        }
    }

    bank->image.sequence = tx->sequence;
    return BANK_OK;
}

bool bank_balance(const Bank *bank, uint64_t *cents) {
    if (!bank || !cents) return false;
    uint64_t total = 0;
    const uint64_t denom[] = {50, 100, 200};
    for (size_t i = 0; i < 3; i++) {
        if (bank->image.counts[i] > (UINT64_MAX - total) / denom[i]) return false;
        total += bank->image.counts[i] * denom[i];
    }
    *cents = total;
    return true;
}

void bank_export(const Bank *bank, BankImage *image) { if (bank && image) *image = bank->image; }

bool bank_import(Bank *bank, const BankImage *image) {
    if (!bank || !image) return false;
    Bank candidate = {.image = *image };
    uint64_t amount;
    if (!bank_balance(&candidate, &amount)) return false;
    bank->image = *image;
    return true;
}

uint64_t bank_sequence(const Bank *bank) { return bank ? bank->image.sequence : 0; }
uint64_t bank_coin_count(const Bank *bank, Coin coin) {
    int i = coin_index(coin);
    return (bank && i >= 0) ? bank->image.counts[i] : 0;
}

bool bank_image_balance(const BankImage *image, uint64_t *cents) {
    static const uint64_t values[3] = {50, 100, 200};

    if (!image || !cents)
        return false;

    uint64_t total = 0;

    for (size_t i = 0; i < 3; ++i) {
        if (image->counts[i] > (UINT64_MAX - total) / values[i])
            return false;

        total += image->counts[i] * values[i];
    }

    *cents = total;
    return true;
}

bool bank_image_coint_count(const BankImage *image, uint64_t *count) {
    if (!image || !count)
        return false;

    uint64_t total = 0;

    for (size_t i = 0; i < 3; ++i) {
        if (image->counts[i] > UINT64_MAX - total)
            return false;

        total += image->counts[i];
    }

    *count = total;
    return true;
}
