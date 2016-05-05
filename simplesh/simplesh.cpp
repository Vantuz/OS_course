#include <string>
#include <vector>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <cstring>

using namespace std;

#define CMD_BUF_SIZE 1

void do_cmds_if_can(string& cmds, int new_bytes);
vector<string> split(string& str, char sep, bool skip_consequent_dels);
void process_cmd_with_pipes(string& cmd);

vector<int> processes;

void handler(int signa) {
	// printf("%lu\n", processes.size());
	for (int i = 0; i < processes.size(); i++) {
		kill(processes[i], signa);
	}
	// processes.clear();
}

int main() {
	struct sigaction action;
	memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = &handler;
    if (sigaction(SIGINT, &action, NULL) == -1)
    	perror("sigaction");

	char buf[CMD_BUF_SIZE];
	bool can_read = true;
	string cmd_str = "";
	char prompt[] = "$ ";
	int lel = 2;
	while(lel > 0) {
		lel -= write(STDOUT_FILENO, prompt + 2 - lel, lel);
	}
	while (can_read) {
		// write(1, "<", 1);
		int bytes_readed = read(STDIN_FILENO, buf, CMD_BUF_SIZE);
		// write(1, ">", 1); 	
		can_read = bytes_readed > 0;
		if (can_read) {
			// printf("here 1\n");
			cmd_str.append(buf, bytes_readed);
			do_cmds_if_can(cmd_str, bytes_readed);
			// printf("here 228\n");
		} else {
			if (bytes_readed == 0) {
				can_read = false;
			} else {
				can_read = true;
				//something
			}
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
			cmds = splitted_str[splitted_str.size() - 1];
			char prompt[] = "$ ";
			int lel = 2;
			while(lel > 0) {
				lel -= write(STDOUT_FILENO, prompt + 2 - lel, lel);
			}
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
	// vector<int> processes;
	// processes.clear();
	// vector<int> handles;
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
		vector<char*> args_c_vec;
		// char ** args_c = new char*[args.size() + 1];
		for(size_t j = 0; j < args.size(); j++){
		    // args_c[j] = new char[args[j].size() + 1];
		    // strcpy(args_c[j], args[j].c_str());
		    args_c_vec.push_back((char*) args[j].c_str());
		}
		args_c_vec.push_back(NULL);
		// args_c[args.size()] = NULL;
		int pipe_fd[2];
		if (i != cmds.size() - 1) {
			pipe(pipe_fd);
			// handles.push_back(pipe_fd[0]);
			// handles.push_back(pipe_fd[1]);
		}
		swap(last_in_desc, pipe_fd[0]);
		if (i == cmds.size() - 1) {
			pipe_fd[1] = STDOUT_FILENO;
		}
		int mypid = fork();
		// printf("%s\n", cmds[i].c_str());
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
			// printf("before exec\n");
			if (execvp(args_c_vec[0], args_c_vec.data()) != 0) {
				char mes[] = "error!\n";
				int lel = strlen(mes);
				while(lel > 0) {
					lel -= write(STDOUT_FILENO, mes + strlen(mes) - lel, lel);
				}
				// printf("error!\n");
				_exit(0);
			}
			// printf("after\n");
		} else {
			processes.push_back(mypid);
			if(pipe_fd[0] != STDIN_FILENO) {
				close(pipe_fd[0]);
			}
			if(pipe_fd[1] != STDOUT_FILENO) {
				close(pipe_fd[1]);
			}
		}
	}
	// for (int i = 0; i < handles.size(); ++i) {
	// 	close(handles[i]);
	// }
	// printf("here\n");
	for (int i = 0; i < processes.size(); ++i) {
		int status;
		waitpid(processes[i], &status, 0);
	}
	processes.clear();
}

