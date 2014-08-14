#include <stdio.h>
#include <string.h>
#include <time.h>

#define SERVER_STRING "Simple Web Server"

/*
 * char *read_until(char *buf, size_t len, char *terminator)
 *
 * terminatorが入力されるか，len文字以上読み込むまでbufに入力を格納する
 */
char *read_until(char *buf, size_t len, char *terminator)
{
	int i, j, term_len = strlen(terminator);

	for (i = 0; i < len; i++) {
		int c = getchar();
		if (c == EOF)
			return NULL;
		else
			buf[i] = (char)c;

		if (i < term_len - 1)
			continue;

		for (j = 0; j < term_len; j++)
			if (buf[i + j - term_len + 1] != terminator[j])
				break;

		if (j == term_len) {
			buf[i - term_len + 1] = '\0';
			return &buf[i - term_len + 1];
		}
	}

	return buf + i;
}

/*
 * void response_header(int status, char *message, int header_count, char **headers)
 *
 * statusとmessageを出力した後に，Dateヘッダー，Serverヘッダー及び
 * headersに格納されたheader_count個のヘッダーを出力する
 */
void response_header(int status, char *message, int header_count, char **headers)
{
	int i;

	// 現在時刻を取得
	time_t t = time(NULL);
	char *time_s = asctime(localtime(&t));

	// 時刻の最後の改行を終端文字に変更
	for (i = strlen(time_s); i >= 0; i--) {
		if (time_s[i] == '\n') {
			time_s[i] = '\0';
			break;
		}
	}

	// ステータス及び固定ヘッダーを出力
	printf("HTTP/1.1 %d %s\r\n", status, message);
	printf("Date: %s\r\n", time_s);
	printf("Server: %s\r\n", SERVER_STRING);

	// その他のヘッダーを出力
	for(i = 0; i < header_count; i++)
		printf("%s\r\n", headers[i]);

	// 空行でヘッダーの末尾を出力
	printf("\r\n");
}

/*
 * size_t parse_params(char *param, size_t count, char **name, char **value)
 *
 * paramをparseしてcount個に達するまでnameに名前を，valueに値を格納した上で
 * 格納した個数を返す
 */
size_t parse_params(char *param, size_t count, char **name, char **value)
{
	size_t index = 0;
	char *ptr = param, *ampersand, *equal;

	while((equal = strchr(ptr, '=')) != NULL) {
		*equal = '\0';
		name[index] = ptr;
		value[index++] = equal + 1;

		if ((ampersand = strchr(equal + 1, '&')) == NULL)
			break;
		
		*ampersand = '\0';
		ptr = ampersand + 1;
	}

	return index;
}

/*
 * size_t parse_cookies(char *cookie, size_t count, char **name, char **value)
 *
 * cookieをparseしてcount個に達するまでnameに名前を，valueに値を格納した上で
 * 格納した個数を返す
 */
size_t parse_cookies(char *cookie, size_t count, char **name, char **value)
{
	size_t index = 0;
	char *ptr = cookie, *equal, *semicolon;

	while((equal = strchr(ptr, '=')) != NULL) {
		*equal = '\0';
		name[index] = ptr;
		value[index++] = equal + 1;

		if ((semicolon = strchr(equal + 1, ';')) == NULL)
			break;
		
		*semicolon = '\0';
		ptr = semicolon + 1;
		while(*ptr == ' ')
			ptr++;
	}

	return index;
}
