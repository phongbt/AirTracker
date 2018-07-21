#ifndef _CMD_H_
#define _CMD_H_

#define CMD_MAX_LEN 12
#define CMD_MAX_NUM 16
#define CMD_MAX_ARG_NUM 4
#define BUF_LEN 64

#define CMD_NOOP_ID  CMD_MAX_LEN +1
#define CMD_UNKNOWN_ID  CMD_MAX_LEN +2
#define CMD_QUIT_ID CMD_MAX_LEN + 3


typedef void (*DO_CMD_FUNC)(int, char*[]);

// void addCmd(int id, const char* name, DO_CMD_FUNC f);
int parseCmd(char *sCmdBuf, int& cmd_argc, char * cmd_argv[]);
// void doCmd(int id, int cmd_argc, char * cmd_argv[]);


#endif // _CMD_H_

