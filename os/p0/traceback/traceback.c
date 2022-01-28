/** @file traceback.c
 *  @brief The traceback function
 *
 *  This file contains the traceback function for the traceback library
 *
 *  @author Harry Q. Bovik (hqbovik)
 *  @bug Unimplemented
 *  TODO: Catch sigterm to indicate end of stack trace
 *  TODO: A couple of arguments cannot be printed
 */

#include <unistd.h>
#include "traceback_internal.h"
#include "helpers.h"
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <setjmp.h>


sigjmp_buf savepoint;

void sigsegv_handler(int i, siginfo_t* info, void *ucontext) {
	siglongjmp(savepoint, 1);
}

bool address_valid(char* test, char* tmp) {
	if (sigsetjmp(savepoint, 1) != 0) {
		return false;
	} else {
		*tmp = *test;
	}
	return true;
}

const functsym_t* find_function_at_address(void* addr) {
	const functsym_t* found = NULL;
	unsigned int distance = 0;

	for (int i = 0; i < FUNCTS_MAX_NUM && strlen(functions[i].name) > 0; i++) {
		if (functions[i].addr < addr && (((unsigned int) (addr - functions[i].addr)) <= MAX_FUNCTION_SIZE_BYTES) && ((found == NULL) || ((addr - functions[i].addr) < distance))) {
			found = &functions[i];
			distance = (unsigned int) (addr - functions[i].addr);
		}
	}
	return found;
}


void string_printer(int ofd, char* s) {
	char x;
	int i;
	for (i = 0; i < 25; i++) {
		if (!address_valid(s+i, &x)) {
			dprintf(ofd, "%p", s);
			return;
		}

		if (s[i] == '\0') {
			break;
		}

		if (!isprint(s[i])) {
			dprintf(ofd, "%p", s);
			return;
		}
	}

	dprintf(ofd, "\"");
	for (int j=0; j < i; j++) {
		dprintf(ofd, "%c", s[j]);
	}

	if (i == 24 && s[24] != '\0') {
		dprintf(ofd, "...");
	}
	dprintf(ofd, "\"");
	return;
}

// ebp: base pointer of function fn_info
void print_function_signature(int ofd, void* ebp, const functsym_t* fn_info) {
	int i, j;
	dprintf(ofd, "Function %s(", fn_info->name);

	for (i = 0; i < ARGS_MAX_NUM && strlen(fn_info->args[i].name) > 0; i++) {
		// if not first argument, print a comma
		if (i > 0) {
			dprintf(ofd, ", ");
		}

		const argsym_t* arg = &(fn_info->args[i]);
		void* arg_ptr = ebp + arg->offset;

		switch (arg->type)
		{
		case TYPE_CHAR:
			dprintf(ofd, "char %s=", arg->name);
			if (isprint(*((char*) arg_ptr))) {
				dprintf(ofd, "'%c'", *((char*) arg_ptr));
			} else {
				dprintf(ofd, "'\\%o'", *((char*) arg_ptr));
			}
			break;
		case TYPE_INT:
			dprintf(ofd, "int %s=%d", arg->name, *((int*) arg_ptr));
			break;
		case TYPE_FLOAT:
			dprintf(ofd, "float %s=%f", arg->name, *((float*) arg_ptr));
			break;
		case TYPE_DOUBLE:
			dprintf(ofd, "double %s=%f", arg->name, *((double*) arg_ptr));
			break;
		case TYPE_STRING:
			// TODO:
			dprintf(ofd, "char *%s=", arg->name);
			string_printer(ofd, *((char**) arg_ptr));
			break;
		case TYPE_STRING_ARRAY:
			dprintf(ofd, "char **%s={", arg->name);
			for (j=0; j < 3; j++) {
				char* string = (*((char ***)arg_ptr))[j];
				if (string == NULL) {
					break;
				} 
				if (j > 0) {
					dprintf(ofd, ", ");
				}
				string_printer(ofd, string);
			}

			if (j == 3 && (*((char ***)arg_ptr))[3] != NULL) {
				dprintf(ofd, ", ...}");
			} else {
				dprintf(ofd, "}");
			}
			break;
		case TYPE_VOIDSTAR:
			// TODO:
			dprintf(ofd, "void *%s=0v%x", arg->name, (int) *((void**) arg_ptr));
			break;
		case TYPE_UNKNOWN:
			// TODO:
			dprintf(ofd, "UNKNOWN %s", arg->name);
			break;
		}
	}

	if (i == 0) {
		dprintf(ofd, "void), in\n");
	} else {
		dprintf(ofd, "), in\n");
	}
}

void cleanup(sigset_t* old_set, struct sigaction* act) {
	sigprocmask(SIG_SETMASK, old_set, NULL);
	sigaction(SIGSEGV, act, NULL);
}

void traceback(int ofd)
{
	sigset_t segv_only;
	sigset_t old_set;
	sigemptyset(&segv_only);
	sigaddset(&segv_only, SIGSEGV);
	sigprocmask(SIG_UNBLOCK, &segv_only, &old_set);

	// Set up SIGSEGV handler
	struct sigaction act;
	struct sigaction old_act;
	act.sa_sigaction = &sigsegv_handler;
	act.sa_flags = SA_SIGINFO;

	sigaction(SIGSEGV, &act, &old_act);

	void* ebp = get_ebp();
	void* next_ebp = NULL;
	char tmp;

	while (true) {
		if (!address_valid(ebp, &tmp)) {
			dprintf(ofd, "FATAL: Invalid frame base pointer\n");
			break;
			return;
		}
		void *traceback_addr = get_function_addr_from_ebp(ebp, &next_ebp);
		const functsym_t *fn_info = find_function_at_address(traceback_addr);
		if (fn_info == NULL)
		{
			dprintf(ofd, "Function %p(...), in\n", traceback_addr);
		}
		else if (strcmp(fn_info->name, "__libc_start_main") == 0)
		{
			break;
		}
		else
		{
			print_function_signature(ofd, next_ebp, fn_info);
		}
		ebp = next_ebp;
	}
	cleanup(&old_set, &old_act);
}

