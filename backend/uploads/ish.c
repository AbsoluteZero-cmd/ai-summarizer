// ish.c
// Aidana Ualikhan, 20241027, Assignment 5

#include <stdio.h>
#include <stdlib.h>

#include "lexsyn.h"
#include "util.h"

#include <unistd.h> 
#include <sys/wait.h>
#include <string.h>
#include <signal.h> 
#include <time.h>
#include <bits/sigaction.h>
#include <fcntl.h>
#include <unistd.h>

/*--------------------------------------------------------------------*/
/* ish.c                                                              */
/* Original Author: Bob Dondero                                       */
/* Modified by : Park Ilwoo                                           */
/* Illustrate lexical analysis using a deterministic finite state     */
/* automaton (DFA)                                                    */
/*--------------------------------------------------------------------*/

int TIME_DELTA = 0;


// freeTokens takes DynArray_T oTokens
// returns nothing
// doesn't read from stdin, stdout, or stderr
// doesn't write to stdin, stdout, or stderr
// doesn't use or affect any global variables
void freeTokens(DynArray_T oTokens) {
  for (size_t i = 0; i < DynArray_getLength(oTokens); i++) {
    struct Token *t = DynArray_get(oTokens, i);
    freeToken((void*)t, NULL);
  }
  DynArray_free(oTokens);
}

// execCommand takes DynArray_T commandTokens
// returns nothing
// doesn't read from stdin, stdout, or stderr
// writes to stderr
// doesn't use or affect any global variables
void execCommand(DynArray_T commandTokens) {
    size_t len = DynArray_getLength(commandTokens);

    char *inputFile = NULL;
    char *outputFile = NULL;

    char **argv = malloc((len + 1) * sizeof(char *));
    if(argv == NULL) {
        errorPrint("malloc", PERROR);
        exit(EXIT_FAILURE);
    }

    size_t new_len = 0;
    for(size_t i = 0; i < len; i++) {
        struct Token *t = DynArray_get(commandTokens, i);

        if(t->eType == TOKEN_REDIN){
            if(i + 1 >= len || inputFile != NULL) {
                errorPrint("Invalid input redirection", FPRINTF);
                exit(EXIT_FAILURE);
            }

            struct Token *next = DynArray_get(commandTokens, ++i);
            if(next->eType != TOKEN_WORD) {
                errorPrint("Invalid input redirection", FPRINTF);
                exit(EXIT_FAILURE);
            }
            inputFile = next->pcValue;
        }

        else if(t->eType == TOKEN_REDOUT){
            if(i + 1 >= len || outputFile != NULL) {
                errorPrint("Invalid output redirection", FPRINTF);
                exit(EXIT_FAILURE);
            }

            struct Token *next = DynArray_get(commandTokens, ++i);
            if(next->eType != TOKEN_WORD) {
                errorPrint("Invalid output redirection", FPRINTF);
                exit(EXIT_FAILURE);
            }
            outputFile = next->pcValue;
        }

        else argv[new_len++] = t->pcValue;
    }

    argv[new_len] = NULL;


    if(inputFile != NULL) {
        int fd = open(inputFile, O_RDONLY);
        if(fd < 0) {
          errorPrint(argv[0], PERROR);
          exit(EXIT_FAILURE);
        }

        int res = dup2(fd, STDIN_FILENO);
        if(res < 0) {
          errorPrint("dup2", PERROR);
          exit(EXIT_FAILURE);
        }
        close(fd);
    }


    if(outputFile != NULL) {
        int fd = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0600);
        if(fd < 0) {
          errorPrint(argv[0], PERROR);
          exit(EXIT_FAILURE);
        }
        int res = dup2(fd, STDOUT_FILENO);
        if(res < 0) {
          errorPrint("dup2", PERROR);
          exit(EXIT_FAILURE);
        }
        close(fd);
    }

    execvp(argv[0], argv);
    errorPrint(argv[0], PERROR);

    free(argv);
    exit(EXIT_FAILURE);
}


// shellHelper takes const char* inLine
// returns nothing
// doesn't read from stdin, stdout, or stderr
// writes to stderr
// doesn't use or affect any global variables
static void
shellHelper(const char *inLine) {
  DynArray_T oTokens;

  enum LexResult lexcheck;
  enum SyntaxResult syncheck;
  enum BuiltinType btype;

  oTokens = DynArray_new(0);
  if (oTokens == NULL) {
    errorPrint("Cannot allocate memory", FPRINTF);
    exit(EXIT_FAILURE);
  }


  lexcheck = lexLine(inLine, oTokens);
  switch (lexcheck) {
    case LEX_SUCCESS:
      if (DynArray_getLength(oTokens) == 0)
        return;

      /* dump lex result when DEBUG is set */
      dumpLex(oTokens);

      syncheck = syntaxCheck(oTokens);
      if (syncheck == SYN_SUCCESS) {
        btype = checkBuiltin(DynArray_get(oTokens, 0));
        /* TODO */

        if(btype == NORMAL){
          size_t len = DynArray_getLength(oTokens);
          if(len == 0) {
            freeTokens(oTokens);
            return;
          }

          int cnt_pipe = countPipe(oTokens);
          if(cnt_pipe == 0){
            int pid = fork();

            if(pid < 0){
              errorPrint("fork", PERROR);
              freeTokens(oTokens);
              return;
            }
            else if(pid == 0){
              execCommand(oTokens);
              exit(0);
            }
            else{
              int res = wait(NULL);
              if(res < 0) errorPrint("wait", PERROR);
              freeTokens(oTokens);
              return;
            }
          }

          DynArray_T commands[cnt_pipe + 1];
          for(int i = 0; i < cnt_pipe + 1; i++) {
            commands[i] = DynArray_new(0);
            if(commands[i] == NULL) {
              errorPrint("Cannot allocate memory", FPRINTF);
              freeTokens(oTokens);
              return;
            }
          }
          
          int pipes[cnt_pipe][2];
          for(int i = 0; i < cnt_pipe; i++) {
            int res = pipe(pipes[i]);
            if(res < 0) {
              errorPrint("pipe", PERROR);
              freeTokens(oTokens);
              return;
            }
          }

          int cur_com = 0;
          for (int i = 0; i < len; i++) {
            struct Token *t = DynArray_get(oTokens, i);
            if(t->eType == TOKEN_PIPE){
              cur_com++;
            }
            else{
              DynArray_add(commands[cur_com], t);
            }
          }


          for(int i = 0; i < cnt_pipe + 1; i++){
            int pid = fork();
            
            if(pid == 0){
              if(i > 0){
                int res = dup2(pipes[i - 1][0], STDIN_FILENO);
                if(res < 0){
                  errorPrint("dup2", PERROR);
                  freeTokens(oTokens);
                  exit(EXIT_FAILURE);
                }
              }
              if(i < cnt_pipe){
                int res = dup2(pipes[i][1], STDOUT_FILENO);
                if(res < 0){
                  errorPrint("dup2", PERROR);
                  freeTokens(oTokens);
                  exit(EXIT_FAILURE);
                }
              }
              
              for(int j = 0; j < cnt_pipe; j++){
                close(pipes[j][0]);
                close(pipes[j][1]);
              }
              
              execCommand(commands[i]);
              exit(EXIT_SUCCESS);
            }
            else if(pid < 0){
              errorPrint("fork", PERROR);
              freeTokens(oTokens);
              return;
            }


          }

          for(int j = 0; j < cnt_pipe; j++){
            close(pipes[j][0]);
            close(pipes[j][1]);
          }


          for(int i = 0; i < cnt_pipe + 1; i++){
            int res = wait(NULL);
            if(res < 0) errorPrint("wait", PERROR);
          }

          for(int i = 0; i < cnt_pipe + 1; i++){
            freeTokens(commands[i]);
          }

          DynArray_free(oTokens);

        }
        else if(btype == B_CD){
          if(DynArray_getLength(oTokens) > 2){
            errorPrint("cd takes one parameter", FPRINTF);
            freeTokens(oTokens);
            return;
          }
          
          char *dir;
          if(DynArray_getLength(oTokens) == 1){
            dir = getenv("HOME");
            if(dir == NULL){
              // errorPrint("HOME not set", FPRINTF);
              freeTokens(oTokens);
              return;
            }
          }
          else{
            struct Token *t = DynArray_get(oTokens, 1);
            dir = t->pcValue;
          }

          int res = chdir(dir);
          if(res != 0){
            // errorPrint("No such file or directory", FPRINTF);
            errorPrint(dir, PERROR);
            freeTokens(oTokens);
            return;
          }
        }
        else if(btype == B_EXIT){
          if(DynArray_getLength(oTokens) > 1){
            errorPrint("exit does not take any parameters", FPRINTF);
            freeTokens(oTokens);
            return;
          }
          exit(EXIT_SUCCESS);
        }
        else if(btype == B_SETENV){
          if(DynArray_getLength(oTokens) > 3 || DynArray_getLength(oTokens) < 2){
            errorPrint("setenv takes one or two parameters", FPRINTF);
            freeTokens(oTokens);
            return;
          }

          struct Token *t = DynArray_get(oTokens, 1);
          char *var = t->pcValue;
          char *val = "";

          if(DynArray_getLength(oTokens) == 3){
            t = DynArray_get(oTokens, 2);
            val = t->pcValue;
          }

          int res = setenv(var, val, 1);
          if(res != 0){
            errorPrint("setenv", PERROR);
            freeTokens(oTokens);            
            return;
          }
        }
        else if(btype == B_USETENV){
          if(DynArray_getLength(oTokens) != 2){
            errorPrint("unsetenv takes one parameter", FPRINTF);
            freeTokens(oTokens);
            return;
          }

          struct Token *t = DynArray_get(oTokens, 1);
          char *var = t->pcValue;

          int res = unsetenv(var);
          if(res != 0){
            errorPrint("unsetenv", PERROR);
            freeTokens(oTokens);
            return;
          }
        }
        else{
          freeTokens(oTokens);
          return;
        }

      }

      /* syntax error cases */
      else if (syncheck == SYN_FAIL_NOCMD)
        errorPrint("Missing command name", FPRINTF);
      else if (syncheck == SYN_FAIL_MULTREDOUT)
        errorPrint("Multiple redirection of standard out", FPRINTF);
      else if (syncheck == SYN_FAIL_NODESTOUT)
        errorPrint("Standard output redirection without file name", FPRINTF);
      else if (syncheck == SYN_FAIL_MULTREDIN)
        errorPrint("Multiple redirection of standard input", FPRINTF);
      else if (syncheck == SYN_FAIL_NODESTIN)
        errorPrint("Standard input redirection without file name", FPRINTF);
      else if (syncheck == SYN_FAIL_INVALIDBG)
        errorPrint("Invalid use of background", FPRINTF);
      break;

    case LEX_QERROR:
      errorPrint("Unmatched quote", FPRINTF);
      break;

    case LEX_NOMEM:
      errorPrint("Cannot allocate memory", FPRINTF);
      break;

    case LEX_LONG:
      errorPrint("Command is too large", FPRINTF);
      break;

    default:
      errorPrint("lexLine needs to be fixed", FPRINTF);
      exit(EXIT_FAILURE);
  }
}

// handle_sigint takes int sig
// returns nothing
// doesn't read from stdin, stdout, or stderr
// writes to stdout
// doesn't use or affect any global variables
void handle_sigint(int sig) {
  printf("\n");
  fprintf(stdout, "%% ");
  fflush(stdout);
}


// handle_sigquit takes int sig
// returns nothing
// doesn't read from stdin, stdout, or stderr
// writes to stdout
// uses and affects global variable TIME_DELTA
void handle_sigquit(int sig) {
  if(TIME_DELTA == 1){
    exit(EXIT_SUCCESS);
  }
  TIME_DELTA = 1;
  printf("\nType Ctrl-\\ again within 5 seconds to exit.\n");
  alarm(5);

  fprintf(stdout, "%% ");
  fflush(stdout);
}


// handle_sigalrm takes int sig
// returns nothing
// doesn't read from stdin, stdout, or stderr
// doesn't write to stdin, stdout, or stderr
// uses and affects global variable TIME_DELTA
void handle_sigalrm(int sig) {
  TIME_DELTA = 0;
}


// main takes int argc and array of strings argv
// returns int
// reads from stdin
// writes to stdout and stderr
// uses and affects no global variables
int main(int argc, char *argv[]) {
  /* TODO */

  sigset_t sset;
  sigaddset(&sset, SIGINT);
  sigaddset(&sset, SIGQUIT);
  sigaddset(&sset, SIGALRM);

  sigprocmask(SIG_UNBLOCK, &sset, NULL);

  if(sigprocmask(SIG_UNBLOCK, NULL, NULL) != 0){
    errorPrint("sigprocmask", PERROR);
    exit(EXIT_FAILURE);
  }

  errorPrint(argv[0], SETUP);

  signal(SIGINT, handle_sigint);
  signal(SIGQUIT, handle_sigquit);
  signal(SIGALRM, handle_sigalrm);

  char *home_env = getenv("HOME");
  if(home_env == NULL){
    // errorPrint("HOME not set", FPRINTF);
    exit(EXIT_FAILURE);
  }

  char *home = malloc(MAX_LINE_SIZE + 2);
  if(home == NULL){
    errorPrint("Cannot allocate memory", FPRINTF);
    exit(EXIT_FAILURE);
  }

  strcpy(home, home_env);

  if(home != NULL){
    char line[MAX_LINE_SIZE + 2];
    char file_name[MAX_LINE_SIZE + 2] = "/.ishrc";
    strcat(home, file_name);

    FILE *ptr = fopen(home, "r");

    if(ptr != NULL){
      while(fgets(line, sizeof(line), ptr) != NULL){
        printf("%% %s", line);
        fflush(stdout);
        shellHelper(line);
      }
      fclose(ptr);
    }

    free(home);
  }
  
  char acLine[MAX_LINE_SIZE + 2];
  while (1) {
    fprintf(stdout, "%% ");
    fflush(stdout);
    if (fgets(acLine, MAX_LINE_SIZE, stdin) == NULL) {
      printf("\n");
      exit(EXIT_SUCCESS);
    }
    shellHelper(acLine);
  }
}

