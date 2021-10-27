#include "Menu.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Input.h"

static void DateToString(Date date, char* out) {
	sprintf(out, "%04d-%02d-%02d", date.Year, date.Month, date.Day);
}

static bool StringToDate(const char* str, Date *out) {
	char* next;
	
	long year = strtol(str, &next, 10);
	if (year < 0 || (next - str) != 4 || *next != '-') {
		goto invalid;
	}

	long month = strtol(next + 1, &next, 10);
	if (month < 0 || month > 12 || (next - str) != 7 || *next != '-') {
		goto invalid;
	}

	long day = strtol(next + 1, &next, 10);
	if (day < 0 || day > 31 || (next - str) != 10 || *next != '\0') {
		goto invalid;
	}

	out->Year = (uint16_t)year;
	out->Month = (uint8_t)month;
	out->Day = (uint8_t)day;
	return true;

invalid:
	return false;
}

static void Show(TransactionRef trans) {
	char date[10 + 1];
	DateToString(trans->Date, date);

	printf("%-3d  %s  %-10s  %s  %-10d  %-10d\n", trans->Index + 1, (trans->Kind == Transaction_Income ? "입금" : "출금"), trans->Comment, date, trans->Uses, trans->Remains);
}

Status ShowAll(DatabaseRef db) {
	IterateTransactions(db, Show);

	TransactionStat stat;
	GetStats(db, &stat);

	printf("총 입출금 횟수: %lu\n", stat.TotalTransactions);
	printf("총 입금액: %lu\n", stat.TotalIncomes);
	printf("총 출금액: %lu\n", stat.TotalWithdraws);

	return Status_OK;
}

Status ShowIncomes(DatabaseRef db) {
	return SearchByKind(db, Transaction_Income, Show);
}

Status ShowWithdraws(DatabaseRef db) {
	return SearchByKind(db, Transaction_Withdraw, Show);
}

static void CreateTransaction(TransactionKind kind, TransactionRef out) {
	char date[10 + 1];

	out->Kind = kind;

	GetString("항목> ", out->Comment, MAX_COMMENT);
	
	bool isValid;
	do {
		GetString("날짜 (YYYY-MM-dd)> ", date, 10 + 1);

		isValid = StringToDate(date, &(out->Date));

		if (!isValid) {
			printf("유효한 날짜를 입력해주세요.\n");
		}
	} while (!isValid);

	out->Uses = GetInt("금액> ", true);
}

Status AddIncome(DatabaseRef db) {
	Transaction trans;
	CreateTransaction(Transaction_Income, &trans);

	return AddTransaction(db, &trans);
}

Status AddWithdraw(DatabaseRef db) {
	Transaction trans;
	CreateTransaction(Transaction_Withdraw, &trans);

	return AddTransaction(db, &trans);
}

Status Search(DatabaseRef db) {
	char comment[MAX_COMMENT];
	GetString("항목으로 검색> ", comment, MAX_COMMENT);

	return SearchByComment(db, comment, Show);
}

Status Update(DatabaseRef db) {
	Transaction trans;
	int index = GetInt("수정할 항목 번호> ", false);
	Status status = GetTransaction(db, index - 1, &trans);

	if (status != Status_OK) {
		return status;
	}

	printf("원본> %s\n", trans.Comment);
	GetString("항목 수정> ", trans.Comment, MAX_COMMENT);

	if (strlen(trans.Comment) == 0) {
		printf("항목 수정을 취소하였습니다.\n");
		return Status_OK;
	}

	status = UpdateTransaction(db, &trans);
	if (status == Status_OK) {
		printf("%d번 항목을 \"%s\"으로 수정하였습니다.\n", index, trans.Comment);
	}

	return status;
}

Status Remove(DatabaseRef db) {
	Transaction trans;
	int index = GetInt("삭제할 항목 번호> ", false);
	Status status = GetTransaction(db, index - 1, &trans);

	if (status != Status_OK) {
		return status;
	}

	status = RemoveTransaction(db, &trans);
	if (status == Status_OK) {
		printf("%d번 항목을 삭제하였습니다.\n", index);
	}

	return status;
}
