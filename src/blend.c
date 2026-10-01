/*
* Author: Rasheed Ajala
* Contact: abioyeajala@gmail.com
*
*/

#include "blend.h"
#include "blend_module.h"
#include "blend_version.h"

#include <stdio.h>
#include <string.h>

static void blend_usage(void)
{
    printf("\n");
    printf("BLEND - Function localization using arbitrary support, Version %s\n", BLEND_VERSION);
    printf("\t(c) 2021-2026 The CRESCENT cyberinfrastructure team.\n\n");
    printf("\tBLEND is distributed under the GNU LGPL License (https://www.gnu.org/licenses/lgpl.html).\n");
    printf("\tDependencies: C standard library, libm.\n\n");
    printf("usage: blend [options]\n");
    printf("       blend <module name> [<module-options>]\n\n");
    printf("options:\n");
    printf("  -V[level], --verbose=<level>\n");
    printf("     Select verbosity level [w]. Choose among q, e, w, t, i, c, and d:\n");
    printf("       q  Quiet; suppress all diagnostic messages.\n");
    printf("       e  Error messages only.\n");
    printf("       w  Warnings and errors [Default].\n");
    printf("       t  Timings, warnings, and errors.\n");
    printf("       i  Informational messages, timings, warnings, and errors.\n");
    printf("       c  Compatibility messages and all lower verbosity messages.\n");
    printf("       d  Debug messages and all lower verbosity messages.\n");
    printf("  --help              List descriptions of available BLEND modules.\n");
    printf("  --show-citation     Show references for citing BLEND.\n");
    printf("  --show-modules      Show all module names.\n");
    printf("  --show-windows      Show all available window function names.\n");
    printf("  --version           Print BLEND version number.\n\n");
    printf("if <module-options> is '=' we call exit (0) if module exist and non-zero otherwise.\n");
}

static int blend_handle_verbosity_option(const char *arg, int *is_option)
{
    const char *value = NULL;
    blend_verbosity level;

    *is_option = 0;
    if (strcmp(arg, "-V") == 0 || strcmp(arg, "--verbose") == 0) {
        value = "i";
    }
    else if (strncmp(arg, "-V", 2) == 0 && arg[2] != '\0') {
        value = arg + 2;
    }
    else if (strncmp(arg, "--verbose=", 10) == 0) {
        value = arg + 10;
    }

    if (value == NULL) {
        return SUCCESS;
    }

    *is_option = 1;
    if (blend_verbosity_from_name(value, &level) != SUCCESS) {
        BLEND_Report(BLEND_MSG_ERROR, "invalid verbosity level: %s\n", value);
        return FAIL;
    }

    return blend_set_verbosity(level);
}

static int blend_strip_verbosity_options(int argc, char **argv)
{
    int read, write;

    write = 0;
    for (read = 0; read < argc; read++) {
        int is_option = 0;

        if (blend_handle_verbosity_option(argv[read], &is_option) != SUCCESS) {
            return -1;
        }
        if (!is_option) {
            argv[write] = argv[read];
            write++;
        }
    }

    return write;
}

static void blend_help(void)
{
    printf("BLEND modules:\n");
    blend_print_module_list();
}

static void blend_citation(void)
{
    printf("Ajala, R., Persaud, P., 2021. Effect of merging multiscale models on seismic wavefield predictions near the southern San Andreas fault. Journal of Geophysical Research: Solid Earth 126, 1-23.\n");
    printf("Ajala, R., Persaud, P., 2022. Ground-motion evaluation of hybrid seismic velocity models. The Seismic Record 2, 186-196.\n");
    printf("Ajala, R., Kolawole, F., Share, P.E., Sahakian, V., Delph, J.R., Hooft, E., He, B., 2025. Toward an accessible framework for synthesizing solid earth models across multiple scales. Seismological Society of America Annual Meeting, Baltimore, Maryland, USA.\n");
}

int main(int argc, char **argv)
{
    const blend_module *module;
    int argi = 1;
    int module_argc;

    while (argi < argc) {
        int is_option = 0;

        if (blend_handle_verbosity_option(argv[argi], &is_option) != SUCCESS) {
            return FAIL;
        }
        if (!is_option) {
            break;
        }
        argi++;
    }

    if (argi >= argc || strcmp(argv[argi], "-?") == 0) {
        blend_usage();
        return SUCCESS;
    }

    if (strcmp(argv[argi], "--help") == 0) {
        blend_help();
        return SUCCESS;
    }

    if (strcmp(argv[argi], "--show-citation") == 0) {
        blend_citation();
        return SUCCESS;
    }

    if (strcmp(argv[argi], "--show-modules") == 0) {
        blend_print_module_names();
        return SUCCESS;
    }

    if (strcmp(argv[argi], "--show-windows") == 0) {
        blend_print_window_function_names(stdout);
        return SUCCESS;
    }

    if (strcmp(argv[argi], "--version") == 0) {
        printf("%s\n", BLEND_VERSION);
        return SUCCESS;
    }

    module = blend_find_module(argv[argi]);
    if (module == NULL) {
        BLEND_Report(BLEND_MSG_ERROR, "unknown module %s\n\n", argv[argi]);
        if (blend_get_verbosity() >= BLEND_MSG_ERROR) {
            blend_usage();
        }
        return FAIL;
    }

    if (argc == argi + 2 && strcmp(argv[argi + 1], "=") == 0) {
        return SUCCESS;
    }

    module_argc = blend_strip_verbosity_options(argc - argi - 1, argv + argi + 1);
    if (module_argc < 0) {
        return FAIL;
    }

    return module->run(module_argc, argv + argi + 1);
}
