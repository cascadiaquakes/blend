/*
* Author: Rasheed Ajala
* Contact: abioyeajala@gmail.com
*
*/

#include "blend.h"
#include "blend_module.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <sys/stat.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#ifndef BLEND_DOC_HTML_DIR_BUILD
#define BLEND_DOC_HTML_DIR_BUILD ""
#endif

#ifndef BLEND_DOC_HTML_DIR_INSTALL
#define BLEND_DOC_HTML_DIR_INSTALL ""
#endif

#ifndef BLEND_DOC_SERVER_URL
#define BLEND_DOC_SERVER_URL "https://cascadiaquakes.github.io/blend/"
#endif

typedef struct docs_page_alias {
    const char *name;
    const char *path;
} docs_page_alias;

static const docs_page_alias docs_page_aliases[] = {
    {"home", "index.html"},
    {"web", "index.html"},
    {"website", "index.html"},
    {"api", "api/index.html"},
    {"examples", "examples/index.html"},
    {"gallery", "examples/index.html"},
    {"reference", "reference/options.html"},
    {"options", "reference/options.html"},
    {"windows", "reference/windows.html"},
    {"polygons", "reference/polygons.html"},
    {"quickstart", "quickstart.html"},
    {"installation", "installation.html"},
    {"install", "installation.html"},
    {"modules", "modules/index.html"},
    {"blend", "modules/blend.html"}
};

static void docs_usage(FILE *fp)
{
    fprintf(fp, "blend docs - Show HTML documentation for a BLEND module\n\n");
    fprintf(fp, "usage: blend docs [-Q] [-S] [-V[q|e|w|t|i|c|d]] <module-name> [<-option>]\n\n");

    fprintf(fp, "REQUIRED ARGUMENTS:\n\n");
    fprintf(fp, "  <module-name>\n");
    fprintf(fp, "     One of the BLEND modules, or one of api, examples, gallery, home,\n");
    fprintf(fp, "     installation, options, polygons, quickstart, reference, website, and\n");
    fprintf(fp, "     windows. Run blend --show-modules to list the available module names.\n\n");

    fprintf(fp, "OPTIONAL ARGUMENTS:\n\n");
    fprintf(fp, "  -Q\n");
    fprintf(fp, "     Only display the documentation target and do not open it in a viewer.\n");
    fprintf(fp, "     If given, -Q must be the first argument to blend docs.\n\n");

    fprintf(fp, "  -S\n");
    fprintf(fp, "     Use the BLEND documentation website instead of local documentation.\n\n");

    fprintf(fp, "  <-option>\n");
    fprintf(fp, "     The one-letter option of the module in question, for example -R.\n");
    fprintf(fp, "     The documentation target is positioned at that option when available.\n\n");

    fprintf(fp, "  -V[level], --verbose=<level>\n");
    fprintf(fp, "     Select verbosity level [w]. Choose among q, e, w, t, i, c, and d:\n");
    fprintf(fp, "       q  Quiet; suppress all diagnostic messages.\n");
    fprintf(fp, "       e  Error messages only.\n");
    fprintf(fp, "       w  Warnings and errors [Default].\n");
    fprintf(fp, "       t  Timings, warnings, and errors.\n");
    fprintf(fp, "       i  Informational messages, timings, warnings, and errors.\n");
    fprintf(fp, "       c  Compatibility messages and all lower verbosity messages.\n");
    fprintf(fp, "       d  Debug messages and all lower verbosity messages.\n\n");

    fprintf(fp, "  -^ (or -)\n");
    fprintf(fp, "     Print short synopsis message.\n\n");

    fprintf(fp, "  -+ (or +)\n");
    fprintf(fp, "     Print longer synopsis message.\n\n");

    fprintf(fp, "  -? (or no arguments)\n");
    fprintf(fp, "     Print this usage message.\n");
}

static void docs_short_synopsis(FILE *fp)
{
    fprintf(fp, "usage: blend docs [-Q] [-S] [-V[q|e|w|t|i|c|d]] <module-name> [<-option>]\n");
}

static void docs_long_synopsis(FILE *fp)
{
    docs_short_synopsis(fp);
    fprintf(fp, "\nShow HTML documentation for a BLEND module or documentation section.\n");
}

static int docs_file_exists(const char *path)
{
    struct stat st;

    return path != NULL && path[0] != '\0' && stat(path, &st) == 0 && S_ISREG(st.st_mode);
}

static int docs_directory_has_index(const char *path)
{
    char index_path[PATH_MAX];

    if (path == NULL || path[0] == '\0') {
        return 0;
    }
    if (snprintf(index_path, sizeof(index_path), "%s/index.html", path) < 0 ||
        strlen(index_path) >= sizeof(index_path)) {
        return 0;
    }
    return docs_file_exists(index_path);
}

static const char *docs_local_root(void)
{
    const char *env_root = getenv("BLEND_DOCS_DIR");

    if (docs_directory_has_index(env_root)) {
        return env_root;
    }
    if (docs_directory_has_index(BLEND_DOC_HTML_DIR_BUILD)) {
        return BLEND_DOC_HTML_DIR_BUILD;
    }
    if (docs_directory_has_index(BLEND_DOC_HTML_DIR_INSTALL)) {
        return BLEND_DOC_HTML_DIR_INSTALL;
    }
    return NULL;
}

static const char *docs_alias_path(const char *name)
{
    size_t i;

    for (i = 0; i < sizeof(docs_page_aliases) / sizeof(docs_page_aliases[0]); i++) {
        if (strcmp(name, docs_page_aliases[i].name) == 0) {
            return docs_page_aliases[i].path;
        }
    }

    return NULL;
}

static int docs_module_path(const char *module_name, char *path, size_t path_size)
{
    const blend_module *module;
    const char *alias_path;
    int n;

    if (module_name == NULL || path == NULL || path_size == 0) {
        return FAIL;
    }

    module = blend_find_module(module_name);
    if (module != NULL) {
        n = snprintf(path, path_size, "modules/%s.html", module->name);
        return n >= 0 && (size_t)n < path_size ? SUCCESS : FAIL;
    }

    alias_path = docs_alias_path(module_name);
    if (alias_path != NULL) {
        n = snprintf(path, path_size, "%s", alias_path);
        return n >= 0 && (size_t)n < path_size ? SUCCESS : FAIL;
    }

    return FAIL;
}

static int docs_parse_option_anchor(const char *option, char *anchor)
{
    if (option == NULL) {
        *anchor = '\0';
        return SUCCESS;
    }

    if (option[0] == '-' && option[1] != '\0' && option[2] == '\0') {
        *anchor = (char)tolower((unsigned char)option[1]);
        return SUCCESS;
    }
    if (option[0] != '-' && option[0] != '\0' && option[1] == '\0') {
        *anchor = (char)tolower((unsigned char)option[0]);
        return SUCCESS;
    }

    BLEND_Report(BLEND_MSG_ERROR, "docs: option lookup must be a one-letter option such as -R\n");
    return FAIL;
}

static int docs_append_anchor(char *target, size_t target_size, char anchor)
{
    size_t len;

    if (anchor == '\0') {
        return SUCCESS;
    }

    len = strlen(target);
    if (len + 2 >= target_size) {
        return FAIL;
    }
    target[len] = '#';
    target[len + 1] = anchor;
    target[len + 2] = '\0';
    return SUCCESS;
}

static int docs_build_server_target(const char *relative_path, char anchor, char *target, size_t target_size)
{
    const char *base = BLEND_DOC_SERVER_URL;
    const char *separator = "";
    int n;

    if (base[0] != '\0' && base[strlen(base) - 1] != '/') {
        separator = "/";
    }

    n = snprintf(target, target_size, "%s%s%s", base, separator, relative_path);
    if (n < 0 || (size_t)n >= target_size) {
        return FAIL;
    }

    return docs_append_anchor(target, target_size, anchor);
}

static int docs_build_local_target(const char *root, const char *relative_path, char anchor,
                                   char *target, size_t target_size)
{
    char path[PATH_MAX];
    char resolved[PATH_MAX];
    const char *local_path = path;
    int n;

    n = snprintf(path, sizeof(path), "%s/%s", root, relative_path);
    if (n < 0 || (size_t)n >= sizeof(path)) {
        return FAIL;
    }
    if (!docs_file_exists(path)) {
        return FAIL;
    }
    if (realpath(path, resolved) != NULL) {
        local_path = resolved;
    }

    n = snprintf(target, target_size, "file://%s", local_path);
    if (n < 0 || (size_t)n >= target_size) {
        return FAIL;
    }

    return docs_append_anchor(target, target_size, anchor);
}

static int docs_shell_append(char *cmd, size_t cmd_size, const char *text)
{
    size_t len = strlen(cmd);
    size_t i;

    if (len + 1 >= cmd_size) {
        return FAIL;
    }
    cmd[len++] = '\'';
    cmd[len] = '\0';

    for (i = 0; text[i] != '\0'; i++) {
        const char *piece = text[i] == '\'' ? "'\\''" : NULL;

        if (piece != NULL) {
            size_t piece_len = strlen(piece);

            if (len + piece_len >= cmd_size) {
                return FAIL;
            }
            memcpy(cmd + len, piece, piece_len + 1);
            len += piece_len;
        }
        else {
            if (len + 1 >= cmd_size) {
                return FAIL;
            }
            cmd[len++] = text[i];
            cmd[len] = '\0';
        }
    }

    if (len + 1 >= cmd_size) {
        return FAIL;
    }
    cmd[len++] = '\'';
    cmd[len] = '\0';
    return SUCCESS;
}

static int docs_open_target(const char *target)
{
    char command[PATH_MAX + 128];
    int status;

#if defined(__APPLE__)
    strcpy(command, "open ");
#elif defined(_WIN32)
    strcpy(command, "start ");
#else
    strcpy(command, "xdg-open ");
#endif

    if (docs_shell_append(command, sizeof(command), target) != SUCCESS) {
        BLEND_Report(BLEND_MSG_ERROR, "docs: documentation target is too long\n");
        return FAIL;
    }

    status = system(command);
    if (status != 0) {
        BLEND_Report(BLEND_MSG_ERROR, "docs: could not open %s\n", target);
        return FAIL;
    }
    return SUCCESS;
}

int blend_docs_module(int argc, char **argv)
{
    const char *module_name = NULL;
    const char *option = NULL;
    const char *root = NULL;
    char relative_path[PATH_MAX];
    char target[PATH_MAX * 2];
    char anchor = '\0';
    int query_only = 0;
    int use_server = 0;
    int i;

    if (argc == 0) {
        docs_usage(stdout);
        return SUCCESS;
    }

    if (strcmp(argv[0], "-?") == 0 || strcmp(argv[0], "--help") == 0) {
        docs_usage(stdout);
        return SUCCESS;
    }
    if (strcmp(argv[0], "-^") == 0 || strcmp(argv[0], "-") == 0) {
        docs_short_synopsis(stdout);
        return SUCCESS;
    }
    if (strcmp(argv[0], "-+") == 0 || strcmp(argv[0], "+") == 0) {
        docs_long_synopsis(stdout);
        return SUCCESS;
    }

    for (i = 0; i < argc; i++) {
        if (strcmp(argv[i], "-Q") == 0) {
            if (i != 0) {
                BLEND_Report(BLEND_MSG_ERROR, "docs: -Q must be the first argument\n");
                return FAIL;
            }
            query_only = 1;
        }
        else if (strcmp(argv[i], "-S") == 0) {
            use_server = 1;
        }
        else if (argv[i][0] == '-' && argv[i][1] != '\0' && module_name == NULL) {
            docs_usage(stderr);
            return FAIL;
        }
        else if (module_name == NULL) {
            module_name = argv[i];
        }
        else if (option == NULL) {
            option = argv[i];
        }
        else {
            docs_usage(stderr);
            return FAIL;
        }
    }

    if (module_name == NULL) {
        docs_usage(stdout);
        return SUCCESS;
    }

    if (docs_module_path(module_name, relative_path, sizeof(relative_path)) != SUCCESS) {
        BLEND_Report(BLEND_MSG_ERROR, "docs: unknown documentation target %s\n", module_name);
        BLEND_Report(BLEND_MSG_ERROR, "docs: run blend --show-modules to list available modules\n");
        return FAIL;
    }
    if (docs_parse_option_anchor(option, &anchor) != SUCCESS) {
        return FAIL;
    }

    if (!use_server) {
        root = docs_local_root();
    }
    if (root != NULL &&
        docs_build_local_target(root, relative_path, anchor, target, sizeof(target)) == SUCCESS) {
        BLEND_Report(BLEND_MSG_INFORMATION, "docs: using local documentation\n");
    }
    else if (docs_build_server_target(relative_path, anchor, target, sizeof(target)) == SUCCESS) {
        BLEND_Report(BLEND_MSG_INFORMATION, "docs: using BLEND documentation website\n");
    }
    else {
        BLEND_Report(BLEND_MSG_ERROR, "docs: could not resolve documentation target\n");
        return FAIL;
    }

    if (query_only) {
        printf("%s\n", target);
        return SUCCESS;
    }

    return docs_open_target(target);
}
