#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

void error(char *s)
{
	while (*s)
		write(2, s++, 1);
}

int cd(char **argv)
{
	if ((argv[1] && argv[2]) || !argv[1])
		return error("error: cd: bad arguments\n"), 1;
	if (chdir(argv[1]) == -1)
		return error("error: cd: cannot change directory to "), error(argv[1]), error("\n"), 1;
	return 0;
}

int	exec(char *argv[], int has_pipe, char *env[])
{
	int	fd[2], status, pid;

	if (!has_pipe && !strcmp(*argv, "cd"))
		return (cd(argv));
	if ((has_pipe && pipe(fd) == -1) || ((pid = fork()) == -1))
		error("error fatal\n"), exit(EXIT_FAILURE);
	if (!pid)
	{
		if (has_pipe && (close(fd[0]) == -1 || dup2(fd[1], STDOUT_FILENO) == -1
			|| close(fd[1]) == -1))
				close(fd[1]), write(2, "error fatal\n", 13), exit(EXIT_FAILURE);
		execve(*argv, argv, env);
		error("error: cannot execute "), error(*argv), error("\n"), exit(EXIT_FAILURE);
	}
	waitpid(pid, &status, 0);
	if (has_pipe && (close(fd[1]) == -1 || dup2(fd[0], STDIN_FILENO) == -1
		|| close(fd[0]) == -1))
			close(fd[0]), write(2, "error fatal\n", 13), exit(EXIT_FAILURE);
	return (WIFEXITED(status) && WEXITSTATUS(status));
}

void	split(char **argv[], char *sub_argv[], int i)
{
	while (sub_argv[++i])
		sub_argv[i] = 0;
	while (*++*argv && **argv[0] != '|' && **argv[0] != ';')
		*sub_argv++ = **argv;
}

int main(int argc, char *argv[], char *env[])
{
	int exit_code = 0;
	char *sub_argv[100] = {NULL};

	while (argc > 1 && *argv && ++*argv)
	{
		split(&argv, sub_argv, -1);
		if (*sub_argv)
			exit_code = exec(sub_argv, (*argv && *argv[0] == '|'), env);
	}
	return (exit_code);
}
