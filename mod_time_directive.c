#include "conf.h"
#include <time.h>

#define  DEFAULT_FORMAT "%Y-%m-%d_%H:%M:%S"
static char *time_format = DEFAULT_FORMAT;

MODRET set_iftime_format(cmd_rec *cmd) {
  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_GLOBAL);

  time_format = pstrdup(cmd->server->pool, cmd->argv[1]);
  pr_log_debug(DEBUG3,
               "set IfTimeFormat '%s'", time_format);

  return PR_HANDLED(cmd);
}

MODRET start_iftime(cmd_rec *cmd) {
  unsigned int ctx_count = 1;
  char buf[PR_TUNABLE_BUFFER_SIZE] = {'\0'}, *config_line = NULL;
  unsigned char overtime = TRUE;
  time_t now, limit;
  struct tm tm_limit;

  CHECK_ARGS(cmd, 1);

  if(*(cmd->argv[1]) == '+') {
      overtime = TRUE;
      (cmd->argv[1])++;
  }
  else if (*(cmd->argv[1]) == '-') {
      overtime = FALSE;
      (cmd->argv[1])++;
  }

  /* 設定時間読み取り */
  if(strptime(cmd->argv[1], time_format, &tm_limit) == NULL) {
      CONF_ERROR(cmd, "invalid <IfTime> context. maybe time format is invalid");
  }

  /* 設定時間と現在の時刻を比較 */
  time(&now);
  limit = mktime(&tm_limit);

  /* ... todo */
  if (overtime){
      if(difftime(now, limit) > 0) {
          pr_log_debug(DEBUG3, "time is ok");
          return PR_HANDLED(cmd);
      }
  } else {
      if(difftime(now, limit) < 0) {
          pr_log_debug(DEBUG3, "time is ok");
          return PR_HANDLED(cmd);
      }
  }

  while (ctx_count && (config_line = pr_parser_read_line(buf,
      sizeof(buf))) != NULL) {

    if (strncasecmp(config_line, "<IfTime", 7) == 0)
      ctx_count++;

    if (strcasecmp(config_line, "</IfTime>") == 0)
      ctx_count--;
  }

  if (ctx_count)
    CONF_ERROR(cmd, "unclosed <IfTime> context");

  return PR_HANDLED(cmd);
}

MODRET end_iftime(cmd_rec *cmd) {
  return PR_HANDLED(cmd);
}

static conftable time_directive_conftab[] = {
  { "<IfTime>",      start_iftime, NULL },
  { "</IfTime>",     end_iftime,   NULL },
  { "IfTimeFormat",  set_iftime_format,   NULL },
  { NULL },
};

module time_directive_module = {
  NULL, NULL,

  /* Module API version */
  0x20,

  /* Module name */
  "time_directive",

  /* Module configuration  table */
  time_directive_conftab,

  /* Module command handler table */
  NULL,

  /* Module authentication handler table */
  NULL,

  /* Module initialization function */
  NULL,

  /* Session initialization function */
  NULL,
};
