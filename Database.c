#define _CRT_SECURE_NO_WARNINGS

#include "Database.h"

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <memory.h>
#include <string.h>

#include "LinkedList.h"

#define DATABASE_PATH ("database.dat")
#define UNUSED(val) (void)val

struct Database {
	LinkedListRef List;
};

typedef struct {
	TransactionKind		Kind;
	char				Comment[MAX_COMMENT];
	Date				Date;
	int32_t				Uses;
} TransactionItem;

/*
 * Database file format:
 *	CountOfTransactions: uint32_t
 *  Transactions: TransactionItem[CountOfTransactions]
 */
static void SerializeTransaction(void* _args, uint32_t i, void* _e) {
	UNUSED(i);

	TransactionItem* item = (TransactionItem*)_e;
	FILE* file = (FILE*)_args;

	fwrite(item, sizeof(TransactionItem), 1, file);
}

static Status Deserialize(FILE* file, LinkedListRef list) {
	uint32_t length = 0;
	size_t read = fread(&length, sizeof(uint32_t), 1, file);
	if (read  == 0) {
		return Status_IOError;
	}

	for (uint32_t i = 0; i < length; i++) {
		TransactionItem item;
		read = fread(&item, sizeof(TransactionItem), 1, file);

		if (read == 0) {
			return Status_IOError;
		}

		AddElement(list, &item);
	}

	return Status_OK;
}

Status LoadDatabase(DatabaseRef *out) {
	Status status = Status_OK;
	DatabaseRef db = malloc(sizeof(Database));

	if (db == NULL) { return Status_OutOfMemory; }

	db->List = CreateList(sizeof(TransactionItem));

	if (db->List == NULL) {
		status = Status_OutOfMemory;
		goto Finalize;
	}

	FILE* file = fopen(DATABASE_PATH, "rb");
	if (file != NULL) {
		status = Deserialize(file, db->List);
		fclose(file);

		if (status != Status_OK) {
			if (status == Status_IOError) { printf("데이터베이스가 손상되었습니다. 데이터가 초기화됩니다.\n"); }
			goto Finalize;
		}
	}

	*out = db;
	return Status_OK;

Finalize:
	RemoveList(db->List, NULL);
	free(db);
	*out = NULL;
	return status;
}

Status SaveDatabase(DatabaseRef db) {
	FILE* file = fopen(DATABASE_PATH, "wb");
	if (file == NULL) { return Status_IOError; }

	uint32_t length = GetSize(db->List);

	fwrite(&length, sizeof(uint32_t), 1, file);
	IterateList(db->List, SerializeTransaction, file);
	fclose(file);

	return Status_OK;
}

void CloseDatabase(DatabaseRef db) {
	if (db == NULL) { return; }

	SaveDatabase(db);

	RemoveList(db->List, NULL);
	free(db);
}

Status AddTransaction(DatabaseRef db, TransactionRef trans) {
	TransactionItem item;
	memset(&item, 0, sizeof(TransactionItem));

	item.Kind = trans->Kind;
	strncpy(item.Comment, trans->Comment, MAX_COMMENT);
	item.Date = trans->Date;
	item.Uses = trans->Uses;

	if (!AddElement(db->List, &item)) {
		return Status_OutOfMemory;
	}

	return SaveDatabase(db);
}

Status RemoveTransaction(DatabaseRef db, TransactionRef trans) {
	if (!RemoveElement(db->List, trans->Index, NULL)) {
		return Status_NotFound;
	}

	return SaveDatabase(db);
}

typedef struct {
	uint32_t Target;
	int32_t Remains;
	TransactionRef Result;
	int Found;
} GetTransactionArgs;

static void GetTransactionIterator(void* _args, uint32_t i, void* _e) {
	GetTransactionArgs* args = (GetTransactionArgs*)_args;
	TransactionItem* item = (TransactionItem*)_e;

	args->Remains += (item->Kind == Transaction_Income) ? item->Uses : -(item->Uses);

	if (i == args->Target) {
		args->Found++;
		
		args->Result->Index = i;
		args->Result->Kind = item->Kind;
		args->Result->Date = item->Date;
		args->Result->Uses = item->Uses;
		args->Result->Remains = args->Remains;

		memcpy(args->Result->Comment, item->Comment, MAX_COMMENT);
	}
}

Status GetTransaction(DatabaseRef db, uint32_t i, TransactionRef out) {
	GetTransactionArgs args = {
		.Target = i,
		.Remains = 0,
		.Result = out,
		.Found = 0
	};

	IterateList(db->List, GetTransactionIterator, &args);

	return (args.Found > 0) ? Status_OK : Status_NotFound;
}

Status UpdateTransaction(DatabaseRef db, TransactionRef trans) {
	TransactionItem* item = GetElementAt(db->List, trans->Index);

	if (item == NULL) {
		return Status_NotFound;
	}

	item->Kind = trans->Kind;
	memcpy(item->Comment, trans->Comment, MAX_COMMENT);
	item->Date = trans->Date;
	item->Uses = trans->Uses;

	return SaveDatabase(db);
}

typedef struct {
	int32_t Remains;
	TransactionHandler Handler;
} IterateTransactionsArgs;

static void IterateTransactionsIterator(void* _args, uint32_t i, void* _e) {
	IterateTransactionsArgs* args = (IterateTransactionsArgs*)_args;
	TransactionItem* item = (TransactionItem*)_e;

	args->Remains += (item->Kind == Transaction_Income) ? item->Uses : -(item->Uses);

	Transaction trans = {
			.Index = i,
			.Kind = item->Kind,
			.Date = item->Date,
			.Uses = item->Uses,
			.Remains = args->Remains
	};

	memcpy(trans.Comment, item->Comment, MAX_COMMENT);

	args->Handler(&trans);
}

void IterateTransactions(DatabaseRef db, TransactionHandler handler) {
	IterateTransactionsArgs args = {
		.Remains = 0,
		.Handler = handler
	};

	IterateList(db->List, IterateTransactionsIterator, &args);
}

typedef struct {
	uint32_t Target;
	int32_t Remains;
	TransactionHandler Handler;
	int Found;
} SearchByIndexArgs;

static void SearchByIndexIterator(void* _args, uint32_t i, void* _e) {
	SearchByIndexArgs* args = (SearchByIndexArgs*)_args;
	TransactionItem* item = (TransactionItem*)_e;

	args->Remains += (item->Kind == Transaction_Income) ? item->Uses : -(item->Uses);

	if (i == args->Target) {
		args->Found++;
		Transaction trans = {
			.Index = i,
			.Kind = item->Kind,
			.Date = item->Date,
			.Uses = item->Uses,
			.Remains = args->Remains
		};

		memcpy(trans.Comment, item->Comment, MAX_COMMENT);

		args->Handler(&trans);
	}
}

Status SearchByIndex(DatabaseRef db, uint32_t index, TransactionHandler handler) {
	SearchByIndexArgs args = {
		.Target = index,
		.Remains = 0,
		.Handler = handler,
		.Found = 0
	};

	IterateList(db->List, SearchByIndexIterator, &args);
	
	return (args.Found > 0) ? Status_OK : Status_NotFound;
}

typedef struct {
	const char* Comment;
	int32_t Remains;
	TransactionHandler Handler;
	int Found;
} SearchByCommentArgs;

static void SearchByCommentIterator(void* _args, uint32_t i, void* _e) {
	UNUSED(i);

	SearchByCommentArgs* args = (SearchByCommentArgs*)_args;
	TransactionItem* item = (TransactionItem*)_e;

	args->Remains += (item->Kind == Transaction_Income) ? item->Uses : -(item->Uses);

	if (strcmp(item->Comment, args->Comment) == 0) {
		args->Found++;
		Transaction trans = {
			.Index = i,
			.Kind = item->Kind,
			.Date = item->Date,
			.Uses = item->Uses,
			.Remains = args->Remains
		};

		memcpy(trans.Comment, item->Comment, MAX_COMMENT);

		args->Handler(&trans);
	}
}

Status SearchByComment(DatabaseRef db, const char* comment, TransactionHandler handler) {
	SearchByCommentArgs args = {
		.Comment = comment,
		.Remains = 0,
		.Handler = handler,
		.Found = 0
	};

	IterateList(db->List, SearchByCommentIterator, &args);

	return (args.Found > 0) ? Status_OK : Status_NotFound;
}

typedef struct {
	TransactionKind Kind;
	int32_t Remains;
	TransactionHandler Handler;
	int Found;
} SearchByKindArgs;

static void SearchByKindIterator(void* _args, uint32_t i, void* _e) {
	UNUSED(i);
	
	SearchByKindArgs* args = (SearchByKindArgs*)_args;
	TransactionItem* item = (TransactionItem*)_e;

	args->Remains += (item->Kind == Transaction_Income) ? item->Uses : -(item->Uses);

	if (args->Kind == item->Kind) {
		args->Found++;
		Transaction trans = {
			.Index = i,
			.Kind = item->Kind,
			.Date = item->Date,
			.Uses = item->Uses,
			.Remains = args->Remains
		};

		memcpy(trans.Comment, item->Comment, MAX_COMMENT);

		args->Handler(&trans);
	}
}

Status SearchByKind(DatabaseRef db, TransactionKind kind, TransactionHandler handler) {
	SearchByKindArgs args = {
		.Kind = kind,
		.Remains = 0,
		.Handler = handler,
		.Found = 0
	};

	IterateList(db->List, SearchByKindIterator, &args);

	return (args.Found > 0) ? Status_OK : Status_NotFound;
}

static void GetStatIterator(void* _args, uint32_t i, void* _e) {
	UNUSED(i);

	TransactionItem* item = (TransactionItem*)_e;
	TransactionStatRef stat = (TransactionStatRef)_args;

	stat->TotalTransactions++;
	if (item->Kind == Transaction_Withdraw) {
		stat->TotalWithdraws += item->Uses;
	}
	else /* item->Kind == Transaction_Income */ {
		stat->TotalIncomes += item->Uses;
	}
}

Status GetStats(DatabaseRef db, TransactionStatRef stat) {
	stat->TotalIncomes = stat->TotalWithdraws = stat->TotalTransactions = 0;

	IterateList(db->List, GetStatIterator, stat);
	return Status_OK;
}
