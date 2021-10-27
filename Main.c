#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>

#include "Status.h"
#include "Database.h"
#include "Input.h"
#include "Menu.h"

#define lengthof(arr)	(sizeof((arr))/sizeof((arr)[0]))

static MenuHandler Handlers[] = {
	&ShowAll,
	&ShowIncomes,
	&ShowWithdraws,
	&AddIncome,
	&AddWithdraw,
	&Search,
	&Update,
	&Remove
};

static const char* HelpMsg =
"1. 전체 사용 목록 보기\n"
"2. 입금 목록 보기\n"
"3. 출금 목록 보기\n"
"4. 입금 정보 추가\n"
"5. 출금 정보 추가\n"
"6. 입출금 정보 검색\n"
"7. 입출금 항목 수정\n"
"8. 입출금 항목 삭제\n"
"0. 종료\n";

static const char* ErrorMsgs[] = {
	"정상 처리됨.",							// Status_OK
	"입출력 오류가 발생하였습니다.",		// Status_IOError
	"메모리가 부족합니다.",					// Status_OutOfMemory
	"조건에 맞는 정보를 찾을 수 없습니다.",	// Status_NotFound
};

bool HandleMenu(DatabaseRef);

int main(int argc, char** argv) {
	DatabaseRef db = NULL;

	printf("만든이: 1701169 신진환\n");

	Status status = LoadDatabase(&db);
	if (status != Status_OK && status != Status_IOError) { printf("%s\n", ErrorMsgs[status]); goto exit; }

	while (HandleMenu(db)) { /* do nothing*/ }

exit:
	CloseDatabase(db);
	return 0;
}

bool HandleMenu(DatabaseRef db) {
	printf(HelpMsg);
	int menu = GetInt("번호> ", true);

	if (menu > lengthof(Handlers)) {
		printf("범위를 벗어났습니다. 0 이상 %ld 이하로 입력해주세요.\n", lengthof(Handlers));
	}
	else if (menu > 0) {
		Status status = Handlers[menu - 1](db);
		if (status != Status_OK) printf("%s\n", ErrorMsgs[status]);
	}

	return menu != 0;
}
