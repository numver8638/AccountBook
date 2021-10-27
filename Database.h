#pragma once

#include "Status.h"

#include <stdint.h>

typedef enum TransactionKind {
	Transaction_Withdraw,
	Transaction_Income
} TransactionKind;

#define MAX_COMMENT	(31 + 1)

typedef struct Date {
	uint16_t Year;
	uint8_t  Month;
	uint8_t  Day;
} Date;

typedef struct Transaction {
	uint32_t			Index;
	TransactionKind		Kind;
	char				Comment[MAX_COMMENT];
	Date				Date;
	int32_t 			Uses;
	int32_t				Remains;
} Transaction, *TransactionRef;

typedef struct TranscationStat {
	uint64_t	TotalTransactions;
	uint64_t	TotalWithdraws;
	uint64_t	TotalIncomes;
} TransactionStat, *TransactionStatRef;

typedef void (*TransactionHandler)(TransactionRef);

typedef struct Database Database, * DatabaseRef;

Status LoadDatabase(DatabaseRef *out);
Status SaveDatabase(DatabaseRef db);

Status AddTransaction(DatabaseRef db, TransactionRef);
Status RemoveTransaction(DatabaseRef db, TransactionRef);
Status UpdateTransaction(DatabaseRef db, TransactionRef);
Status GetTransaction(DatabaseRef db, uint32_t, TransactionRef);

void IterateTransactions(DatabaseRef db, TransactionHandler handler);
Status SearchByIndex(DatabaseRef db, uint32_t, TransactionHandler handler);
Status SearchByComment(DatabaseRef db, const char*, TransactionHandler handler);
Status SearchByKind(DatabaseRef db, TransactionKind, TransactionHandler handler);

Status GetStats(DatabaseRef db, TransactionStatRef stat);

void CloseDatabase(DatabaseRef db);