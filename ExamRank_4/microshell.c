#include "shell.h"

int	ft_strlen(const char *str)
{
	int i = 0;

	while (str && str[i])
		i++;
	return (i);
}

int	count_pipes(char **arr)
{
	int i = 0;
	int n = 0;

	while (arr && arr[i])
	{
		if (!strcmp(arr[i], "|"))
			n++;
		i++;
	}
	return (n);
}

int *get_fd(int m)
{
	static int *fd;
	static int	*begin_of_fd;
	static int n;

	if (!n)
		n = m;
	if (m)
		fd = malloc(2 * n * sizeof(*fd));
	if (!fd && m)
	{
		write(2, "error: fatal", 12);
		exit(127);
	}
	if (!begin_of_fd)
		begin_of_fd = fd;
	while (n)
	{
		if (pipe(fd))
		{
			*fd = 0;
			break ;
		}
		n--;
		fd = fd + 2;
	}
	printf("---------------------------------- i = %d ---------------\n", n);
	return (begin_of_fd);
}

void	execute_part(char **arr, char **envp)
{
	
	if (arr && arr[0] && execve(arr[0], (void *)arr, (void *)envp))
		{
			write(2, "error: cannot execute ", 22);
			write(2, arr[0], ft_strlen(arr[0]));
			write(2, "\n", 1);
		}
		exit(127);
}

void replace_fd(int n, int *fd, int i)
{
	if (i < n)
		dup2(fd[2 * i + 1], STDOUT_FILENO);
	if (i)
		dup2(fd[2 * i - 2], STDIN_FILENO);
	while (2 * i + 2 < 2 * n)
	{
		if (fd[2 + 2 * i] == 0)
			break ;
		close(fd[2 + 2 * i]);
		i++;
	}
	if (fd)
		free(fd);
}

int	micro_cd(char **arr, char **envp)
{
	if (!arr[1] || arr[2])
	{
		write(2, "error: cd: bad arguments\n", 25);
		exit(1);
	}
	if (chdir(arr[1]))
	{
		write(2, "error: cd: cannot change directory to ", 38);
		write(2, arr[1], ft_strlen(arr[1]));
		write(2, "\n", 1);
		exit(1);
	}
}

void	run_bins(char **arr, char **envp)
{
	int *fd;
	int pid;
	int n;
	int i = 0;
	int k = 0;

	n = count_pipes(arr);
	fd = get_fd(n);
	if (!strcmp(*arr, "cd"))
	{
		micro_cd(arr, envp);
		return ;
	}
	while (*arr)
	{
		k = 0;
		while (arr[k] && strcmp(arr[k], "|"))
			k++;
		if (arr[k] != 0)
			arr[k++] = 0;
		pid = fork();
		if (pid < 0)
			break ;
		else if (pid == 0)
		{
			replace_fd(n, fd, i);
			execute_part(arr, envp);
		}
		if (i < n)
			close(fd[2 * i + 1]);
		if (i)
			close(fd[2 * i - 2]);
		i++;
		if (i / 3)
			get_fd(0);
		arr = &arr[k];
	}
	while (waitpid(0, 0, 0) >= 0)
		n++;
	if (fd)
		free(fd);
}

void	run_part(char ***argv, char **envp)
{
	char **arr;
	int i = 0;

	arr = *argv;
	while (arr[i] && strcmp(arr[i], ";"))
		i++;
	if (arr[i] != 0)
	{
		arr[i] = 0;
		i++;
	}
	if (*arr)
		run_bins(arr, envp);
	*argv = &arr[i];
}

int main(int argc, char const **argv, char const **envp)
{
	int	i = 0;

	if (argc < 2)
		return (1);
	argv++;
	while (*argv != 0)
		run_part((char ***)&argv, (char **)envp);
	return (0);
}
