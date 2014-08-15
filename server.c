#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define METHOD_LEN 8
#define URI_LEN 256
#define VERSION_LEN 16
#define HEADER_LEN 4096

extern char *read_until(char *buf, size_t len, char *terminator);
extern void response_header(int status, char *message, int header_count, char **headers);

void get(char *uri, char *param, int header_count, char **headers);
void post(char *uri, char *param, int header_count, char **headers);

void http_server(void) {
	char *ret;

	// HTTP methodを受け取る
	char method[METHOD_LEN + 1];
	ret = read_until(method, METHOD_LEN + 1, " ");

	// methodが正しく送られてこなければ400エラーを返す
	if (ret == NULL || ret == &method[METHOD_LEN + 1]) {
		response_header(400, "Bad Request", 0, NULL);
		return;
	}

	// methodがGET，POSTでなければ501エラーを返す
	if (strcmp(method, "GET") != 0 && strcmp(method, "POST") != 0) {
		response_header(501, "Not Implemented", 0, NULL);
		return;
	}

	// URIを受け取る
	char uri[URI_LEN + 1];
	ret = read_until(uri, URI_LEN + 1, " ");

	// URIが正しく送られてこなければ414エラーを返す
	if (ret == NULL || ret == &uri[URI_LEN + 1]) {
		response_header(414, "Request-URI Too Long", 0, NULL);
		return;
	}

	// HTTP versionを受け取る
	char version[VERSION_LEN + 2];
	ret = read_until(version, VERSION_LEN + 2, "\r\n");

	// HTTP versionが正しく送られてこなければ400エラーを返す
	if (ret == NULL || ret == &version[VERSION_LEN + 2]) {
		response_header(400, "Bad Request", 0, NULL);
		return;
	}

	// HTTP versionが1.1でなければ505エラーを返す
	if (strcmp(version, "HTTP/1.1") != 0) {
		response_header(505, "HTTP Version Not Supported", 0, NULL);
		return;
	}

	// URIに?が含まれていれば，パラメータを抽出
	char *param = strchr(uri, '?');
	if (param == NULL)
		param = &uri[strlen(uri)];
	else
		*(param++) = '\0';

	int status = 0, header_count = 0;
	char header[HEADER_LEN + 2], **headers = NULL;
	while (1) {
		// HTTP headerを受け取る
		ret = read_until(header, HEADER_LEN + 2, "\r\n");

		// HTTP headerが正しく送られてこなければ400エラーを返す
		if (ret == NULL || ret == &header[HEADER_LEN + 2]) {
			status = 400;
			break;
		}

		// 長さが0であればヘッダーの終端
		if (strlen(header) == 0)
			break;

		// ヘッダーに:が含まれていなければ400エラーを返す
		char *delimiter = strchr(header, ':');
		if (delimiter == NULL) {
			status = 400;
			return;
		}

		// headersをreallocする
		char **ret_s = (char **)realloc(headers, sizeof(char *) * (header_count + 1));
		if (ret_s == NULL) {
			perror("realloc() failed");
			status = 500;
			break;
		} else {
			headers = ret_s;
			header_count++;
		}

		// headersの最後の要素にheaderをコピーするための領域を確保する
		ret = (char *)malloc(sizeof(char) * (strlen(header) + 1));
		if (ret == NULL) {
			perror("realloc() failed");
			status = 500;
			break;
		} else {
			headers[header_count - 1] = ret;
		}

		// headerをコピーする
		ret = strcpy(headers[header_count - 1], header);
	}

	// エラーがなければmethodに応じで関数を呼ぶ
	if (status == 0) {
		if (strcmp(method, "GET") == 0)
			get(uri, param, header_count, headers);
		else if (strcmp(method, "POST") == 0)
			post(uri, param, header_count, headers);
	} else if (status == 400) {
		response_header(400, "Bad Request", 0, NULL);
	} else if (status == 500) {
		response_header(500, "Internal Server Error", 0, NULL);
	}

	// 確保した領域を解放
	int i = 0;
	for(i = 0; i < header_count; i++)
		free(headers[i]);
	free(headers);

	return;
}

void get(char *uri, char *param, int header_count, char **headers)
{
	int i;
	response_header(200, "OK", 0, NULL);

	printf("Request method: GET\r\n");
	printf("Request URI: %s\r\n", uri);
	printf("Request parameters: %s\r\n", param);

	printf("Request headers: %d\r\n", header_count);
	for(i = 0; i < header_count; i++)
		printf("\t%s\r\n", headers[i]);

	int counter = 0;
	FILE *fp;

	fp = fopen("counter", "r");
	fscanf(fp, "%d", &counter);
	fclose(fp);

	printf("Counter: %d\r\n", ++counter);

	fp = fopen("counter", "w");
	fprintf(fp, "%d", counter);
	fclose(fp);
}

void post(char *uri, char *param, int header_count, char **headers)
{
	int i;
	response_header(200, "OK", 0, NULL);

	printf("Request method: POST\r\n");
	printf("Request URI: %s\r\n", uri);
	printf("Request parameters: %s\r\n", param);

	printf("Request headers: %d\r\n", header_count);
	for(i = 0; i < header_count; i++)
		printf("\t%s\r\n", headers[i]);
}
