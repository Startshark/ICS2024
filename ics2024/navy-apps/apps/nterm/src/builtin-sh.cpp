#include <nterm.h>
#include <stdarg.h>
#include <unistd.h>
#include <SDL.h>
#include <string.h>

char handle_key(SDL_Event *ev);

static void sh_printf(const char *format, ...) {
  static char buf[256] = {};
  va_list ap;
  va_start(ap, format);
  int len = vsnprintf(buf, 256, format, ap);
  va_end(ap);
  term->write(buf, len);
}

static void sh_banner() {
  sh_printf("Built-in Shell in NTerm (NJU Terminal)\n\n");
}

static void sh_prompt() {
  sh_printf("sh> ");
}

static void sh_handle_cmd(const char *cmd) {
  char *tmp = strdup(cmd);
  tmp = strtok(tmp, "\n");
  if (strcmp(tmp, "exit") == 0) {
    exit(0);
  } else if (strcmp(tmp, "help") == 0) {
    sh_printf("Built-in Shell in NTerm (NJU Terminal)\n");
    sh_printf("Commands:\n");
    sh_printf("  exit - exit the shell\n");
    sh_printf("  help - show this message\n");
    sh_printf("  echo - obviously.\n");
  } 
  else if(strcmp(tmp, "echo")){
    char *arg = strtok(NULL, " ");
    if (arg) {
      arg += strspn(arg, " ");
      term->write(arg, strlen(arg));
    }
    else {
      sh_printf("\n");
    }
  }
  else {
    sh_printf("Unknown command: %s\n", cmd);
  }
  setenv("PATH", "/bin", 0);
  execvp(tmp, NULL);
}

void builtin_sh_run() {
  sh_banner();
  sh_prompt();

  while (1) {
    SDL_Event ev;
    if (SDL_PollEvent(&ev)) {
      if (ev.type == SDL_KEYUP || ev.type == SDL_KEYDOWN) {
        const char *res = term->keypress(handle_key(&ev));
        if (res) {
          sh_handle_cmd(res);
          sh_prompt();
        }
      }
    }
    refresh_terminal();
  }
}
