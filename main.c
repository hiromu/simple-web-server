#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>

int sock_fd, accept_fd;

void parent_handler(int signal);
void child_handler(int signal);

extern void http_server(void);

int main(int argc, char **argv)
{
	// 引数の数をチェック
	if (argc != 2) {
		printf("usage: %s <port>\n", argv[0]);
		return -1;
	}

	// シグナルハンドラを設定
	signal(SIGCHLD, parent_handler);
	signal(SIGTERM, parent_handler);

	// socketを作成
	sock_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock_fd == -1) {
		perror("socket() failed");
		return -1;
	}

	// socketにbindする情報を格納する構造体sockaddr_inを初期化
	struct sockaddr_in server;
	memset(&server, 0, sizeof(server));

	// sockaddr_inにlistenするアドレスやポートを設定
	server.sin_family = AF_INET;
	server.sin_port = htons(atoi(argv[1]));
	// 0.0.0.0でlistenする設定だとすべてのアドレスからの接続を受け付ける
	inet_aton("0.0.0.0", &server.sin_addr);

	// sock_fdにsockaddr_inをbind
	if (bind(sock_fd, (struct sockaddr *)&server, sizeof(server)) == -1) {
		perror("bind() failed");
		return -1;
	}

	// 同時に接続できるqueueの数を設定
	if (listen(sock_fd, 20) == -1) {
		perror("listen() failed");
		return -1;
	}

	// 接続元の情報を格納する構造体を初期化
	struct sockaddr_in client;
	int client_len = sizeof(client);

	while (1) {
		// 接続があるまで待ち，接続が来ると受け付ける
		accept_fd = accept(sock_fd, (struct sockaddr *)&client, (socklen_t *)&client_len);
		if (accept_fd == -1) {
			perror("accept() fail");
			return -1;
		}
		
		// forkし，子プロセスで接続を処理する
		pid_t pid = fork();
		if (pid == 0) {
			// 子プロセスに対してシグナルハンドラを設定
			signal(SIGTERM, child_handler);

			// 標準入出力をstdin/stdoutに設定
			dup2(accept_fd, fileno(stdin));
			dup2(accept_fd, fileno(stdout));

			http_server();

			// 出力をflush
			fflush(stdout);

			// socketをshutdown
			if (shutdown(accept_fd, SHUT_RDWR) == -1) {
				perror("shutdown() failed");
				return -1;
			}

			// socketをclose
			if (close(accept_fd) == -1) {
				perror("close() failed");
				return -1;
			}

			return 0;
		}
	}

	// 子プロセスの処理が終わるのを待つ
	while (waitpid(-1, NULL, WNOHANG) > 0);

	if (shutdown(sock_fd, SHUT_RDWR) == -1) {
		perror("shutdown() failed");
		return -1;
	}
	if (close(sock_fd) == -1) {
		perror("close() failed");
		return -1;
	}

	return 0;
}

void parent_handler(int signal)
{
	if (signal == SIGCHLD) {
		// 子プロセスの終了を待つ
		while (waitpid(-1, NULL, WNOHANG) > 0);
	} else {
		// serverのsocketをclose
		if (close(sock_fd) == -1) {
			perror("close() failed");
			exit(-1);
		}

		exit(0);
	}
}

void child_handler(int signal)
{
	// clientのsocketをclose
	if (close(accept_fd) == -1) {
		perror("close() failed");
		exit(-1);
	}

	exit(0);
}
