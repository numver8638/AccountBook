#include "Input.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>

int GetInt(const char* msg, bool allowZero) {
	char buffer[31 + 1];
	char* end;
	
	while (true) {
		int length = GetString(msg, buffer, 31 + 1);
		long value = strtol(buffer, &end, 10);

		if ((errno == ERANGE) || value > INT_MAX) {
			fputs("오류: 정수 범위를 벗어났습니다.\n", stdout);
		}
		else if (end != (buffer + length) && !isspace(*end)) {
			fputs("오류: 유효하지 않은 입력입니다.\n", stdout);
		}
		else if (value < 0 || (!allowZero && value == 0)) {
			fprintf(stdout, "%d 이상의 정수를 입력해주세요.\n", allowZero ? 0 : 1);
		}
		else {
			return (int)value;
		}
	}
}

int GetString(const char* msg, char* buffer, size_t length) {
	int ch;
	int index = 0;

	fputs(msg, stdout);

	while ((ch = fgetc(stdin)) != EOF && ch != '\n') {
		if (index < (length - 1 /* reserved for NULL */)) {
			buffer[index++] = (char)ch;
		}
	}

	buffer[index] = '\0';
	return index;
}
