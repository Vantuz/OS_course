#include <string>
#include <vector>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <cstring>

using namespace std;

#define CMD_BUF_SIZE 228

struct command {
	string com_str;
	int stdin_no;
	int stdout_no;
};

void do_cmds_if_can(string& cmds, int new_bytes);
vector<string> split(string& str, char sep, bool skip_consequent_dels);
void process_cmd_with_pipes(string& cmd);

int main() {
	char buf[CMD_BUF_SIZE];
	bool can_read = true;
	string cmd_str = "";
	char prompt[] = "$ ";
	int lel = 2;
	while(lel > 0) {
		lel -= write(STDOUT_FILENO, prompt + 2 - lel, lel);
	}
	while (can_read) {
		int bytes_readed = read(STDIN_FILENO, buf, CMD_BUF_SIZE);
		can_read = bytes_readed != 0;
		if (can_read) {
			// printf("here 1\n");
			cmd_str.append(buf, bytes_readed);
			do_cmds_if_can(cmd_str, bytes_readed);
			// printf("here 228\n");
		} else {
			//do smth maybe))))0))))))0
		}
	}
	return 0;
}

void do_cmds_if_can(string& cmds, int new_bytes) {
	size_t lf_pos = cmds.find('\n', cmds.size() - new_bytes);
	if (lf_pos != string::npos) {
		vector<string> splitted_str = split(cmds, '\n', false);
		// printf("here 2\n");
		for (int i = 0; i < splitted_str.size() - 1; ++i) {
			// printf("here 3\n");
			process_cmd_with_pipes(splitted_str[i]);
		}
		cmds = splitted_str[splitted_str.size() - 1];
		char prompt[] = "$ ";
		int lel = 2;
		while(lel > 0) {
			lel -= write(STDOUT_FILENO, prompt + 2 - lel, lel);
		}
	}
}

vector<string> split(string& str, char sep, bool skip_consequent_dels) {
	vector<string> res;
	size_t start = 0;
	size_t end = 0;
	while(start != string::npos) {
		end = str.find(sep, start);
		res.push_back(str.substr(start, end - start));
		end = (end == string::npos) ? end : end + 1;
		while(end != string::npos & skip_consequent_dels & str[end] == sep & end <= str.size()) {
			end++;
		}
		start = end;
	}
	return res;
}

void process_cmd_with_pipes(string& cmd) {
	// printf("%s\n", cmd.c_str());
	vector<string> cmds = split(cmd, '|', false);
	int last_in_desc = STDIN_FILENO;
	vector<int> processes;
	vector<int> handles;
	// printf("here 4\n");
	for (int i = 0; i < cmds.size(); ++i) {
		vector<string> args = split(cmds[i], ' ', true);
		if (args.size() == 1 && args[0] == "") {
			continue;
		}
		if (args[args.size() - 1] == "") {
			args.pop_back();
		}
		if (args[0] == "") {
			args.erase(args.begin(), args.begin() + 1);
		}
		char ** args_c = new char*[args.size() + 1];
		for(size_t j = 0; j < args.size(); j++){
		    args_c[j] = new char[args[j].size() + 1];
		    strcpy(args_c[j], args[j].c_str());
		}
		args_c[args.size()] = NULL;
		int pipe_fd[2];
		if (i != cmds.size() - 1) {
			pipe2(pipe_fd, O_CLOEXEC);
			handles.push_back(pipe_fd[0]);
			handles.push_back(pipe_fd[1]);
		}
		swap(last_in_desc, pipe_fd[0]);
		if (i == cmds.size() - 1) {
			pipe_fd[1] = STDOUT_FILENO;
		}
		int mypid = fork();
		// printf("%s\n", cmds[i].c_str());
		processes.push_back(mypid);
		// printf("%d\n", mypid);
		if (mypid == 0) {
			// printf("here 5\n");
			// printf("%d %d\n", pipe_fd[1], pipe_fd[0]);
			// printf("%d %d\n", STDIN_FILENO, STDOUT_FILENO);
			// printf("%d\n", args_c[1] == NULL);
			int new_1 = dup2(pipe_fd[1], STDOUT_FILENO);
			int new_2 = dup2(pipe_fd[0], STDIN_FILENO);
			if (new_1 != pipe_fd[1]) {
				close(pipe_fd[1]);
			}
			if (new_2 != pipe_fd[0]) {
				close(pipe_fd[0]);
			}
			if (execvp(args_c[0], args_c) != 0) {
				char mes[] = "error!\n";
				int lel = strlen(mes);
				while(lel > 0) {
					lel -= write(STDOUT_FILENO, mes + strlen(mes) - lel, lel);
				}
				// printf("error!\n");
				_exit(0);
			}
		}
	}
	for (int i = 0; i < handles.size(); ++i) {
		close(handles[i]);
	}
	for (int i = 0; i < processes.size(); ++i) {
		int status;
		waitpid(processes[i], &status, 0);
	}
}

