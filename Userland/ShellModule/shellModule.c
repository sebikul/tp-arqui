#include <stdint.h>
#include <types.h>
#include "syscalls.h"
#include "libc.h"
#include "string.h"
#include "commands.h"

#define MAX_ARGS 				256
#define CMD_BUFFER_SIZE 		2*MAX_ARGS

#define MAX_HISTORY_SIZE 		256

#define LEFT_STRIP(str)			while (*(++command) == ' ')

extern char bss;
extern char endOfBinary;

static int var1 = 0;
static int var2 = 0;

static char* shell_history[MAX_HISTORY_SIZE] = {0};
static int current_history = 0;
static int max_history = 0;

static int cmd_count = 12;
static char** cmd_list;

char* user_name;
char* host_name;
int USER_SIZE = 20;
int HOST_SIZE = 20;

void keyboard_uparrow_handler(uint64_t s);
void keyboard_downarrow_handler(uint64_t s);

void command_dispatcher(char* command);
static void initialize_command_list();
static void initialize_names();
static void calloc_cmd(int i, char* str);

uint64_t main(int argc, char** argv) {

	static char buffer[CMD_BUFFER_SIZE] = {0};

	memset(&bss, 0, &endOfBinary - &bss);

	ksysdebug("Initializing shell\n");

	initialize_command_list();
	initialize_names();

	sys_keyboard_catch(0x48, keyboard_uparrow_handler, 0);
	sys_keyboard_catch(0x50, keyboard_downarrow_handler, 0);

	while (TRUE) {

		printf("\n%s@%s $ ", user_name, host_name);

		if (scanf(buffer, CMD_BUFFER_SIZE) == 0) {
			continue;
		}

		ksysdebugs(buffer);

		command_dispatcher(buffer);
	}

	//Test if BSS is properly set up
	if (var1 == 0 && var2 == 0)
		return 0xDEADC0DE;

	return 0xDEADBEEF;
}

void command_dispatcher(char* command) {

	int argc = 0;
	char** argv = calloc(MAX_ARGS * sizeof(char*));

	//backup en el historial
	shell_history[max_history] = calloc(strlen(command) + 1);

	strcpy(shell_history[max_history], command);

	max_history++;

	current_history = max_history;

	//Vamos a sacarle todos los espacion al principio del comando
	if (*command == ' ') {
		LEFT_STRIP(command);
	}

	while (*command != 0) {

		//alocamos espacio para el argumento que estamos parseando
		argv[argc] = calloc(CMD_BUFFER_SIZE * sizeof(char));

		//copiamos el puntero a la cadena por comodidad, para poder modificarlo
		char* pos = argv[argc];

		bool comillas = (*command == '"');

		if (comillas)
			command++;

		while (((!comillas && *command != ' ') || (comillas && *command != '"')) && *command != 0) {

			*pos = *command;
			pos++;

			command++;
		}

		if (comillas && *command == '"') {
			command++;
			comillas = FALSE;
		}

		if (comillas) {
			fprintf(FD_STDERR, "Comando mal formateado. Contiene comillas sin cerrar!\n");
		}

		//si al argumento le siguen espacios los limpiamos
		if (*command == ' ') {
			LEFT_STRIP(command);
		}

		argc++;
	}

	int cmd = 0;

	for (; cmd < cmd_count; cmd++) {
		if (strcmp(argv[0], cmd_list[cmd]) == 0) {
			break;
		}
	}

	switch (cmd) {

	case 0: //echo
		command_echo(argc, argv);
		break;

	case 1: //help
		command_help(argc, argv, cmd_list, cmd_count);
		break;

	case 2: //time
		command_time(argc, argv);
		break;

	case 3: //color
		command_color(argc, argv);
		break;

	case 4:
		command_set_distribution(argc, argv);
		break;

	case 5: //exit
		command_exit();
		break;

	case 6: //clear
		command_clear(argc);
		break;

	case 7: //refresh
		command_refresh();
		break;

	case 8: //user
		command_user_name(argc, argv);
		break;

	case 9: //host
		command_host_name(argc, argv);
		break;

	case 10: //screensaver
		command_screensaver(argc, argv);
		break;

	case 11:
		command_rawkbd(argc, argv);
		break;

	default:

		fprintf(FD_STDERR, "Comando no encontrado.");
	}
}

void keyboard_uparrow_handler(uint64_t s) {

	ksysdebug("Flecha arriba\n");

	if (current_history == 0) {
		return;
	}

	current_history--;

	sys_clear_indexed_line(0);

	printf("%s@%s $ %s", user_name, host_name, shell_history[current_history]);

	sys_keyboard_replace_buffer(shell_history[current_history]);
}

void keyboard_downarrow_handler(uint64_t s) {

	ksysdebug("Flecha abajo\n");

	if (current_history == max_history - 1 || max_history == 0) {

		sys_clear_indexed_line(0);
		printf("%s@%s $ ", user_name, host_name);
		sys_keyboard_replace_buffer("");
		if (current_history == max_history - 1) {
			current_history = max_history;
		}
		return;
	} else if (current_history == max_history) {
		return;
	}

	current_history++;

	sys_clear_indexed_line(0);

	printf("%s@%s $ %s", user_name, host_name, shell_history[current_history]);

	sys_keyboard_replace_buffer(shell_history[current_history]);
}

static void initialize_command_list() {
	cmd_list = calloc(cmd_count * sizeof(char*));
	calloc_cmd(0, "echo");
	calloc_cmd(1, "help");
	calloc_cmd(2, "time");
	calloc_cmd(3, "color");
	calloc_cmd(4, "keyboard");
	calloc_cmd(5, "exit");
	calloc_cmd(6, "clear");
	calloc_cmd(7, "refresh");
	calloc_cmd(8, "user");
	calloc_cmd(9, "host");
	calloc_cmd(10, "screensaver");
	calloc_cmd(11, "rawkbd");
}

static void calloc_cmd(int i, char* str) {
	int len = strlen(str);
	cmd_list[i] = calloc(len * sizeof(char));
	strcpy(cmd_list[i], str);
}

static void initialize_names() {
	user_name = calloc(USER_SIZE * sizeof(char));
	host_name = calloc(HOST_SIZE * sizeof(char));
	strcpy(user_name, "user");
	strcpy(host_name, "localhost");
}
