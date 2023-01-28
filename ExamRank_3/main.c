#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main(int argc, char const *argv[])
{
	FILE *file = 0;
	int	count;
	float o[3];
	char	c[3];
	int	size[2];
	char *str;
	int i = 0, n = 0;

	if (argc == 2)
		file = fopen(argv[1], "r");
	if (!file)
	{
		write(2, "Error: argument\n", 16);
		return (1);
	}
	count = fscanf(file, "%d %d %c\n", &size[0], &size[1], &c[0]);
	if (size[0] == 0 || size[1] == 0 || size [0] > 300 || size [1] > 300 || c[0] == 0 || count != 3)
	{
		write(2,"Error: Operation file corrupted\n", 32);
		return (1);
	}
	str = calloc(size[0] + 1, size[1] + 1);
	while (i != (size[0] + 1) * size[1])
	{
		str[i] = c[0];
		if ((i + 1) % (size[0] + 1) == 0)
			str[i] = '\n';
		i++;
	}
	count = fscanf(file, "%c %f %f %f %c\n", &c[1], &o[0], &o[1], &o[2], &c[2]);
	while (count == 5)
	{
		i = 0;
		n = 0;
		while (str[i] != 0)
		{
			if (c[1] == 'c' && sqrt(powf(i - n * (size[0] + 1) - o[0], 2) + powf(n - o[1], 2)) <= o[2] && sqrt(powf(i - n * (size[0] + 1) - o[0], 2) + powf(n - o[1], 2)) > o[2] - 1 && str[i] != '\n')
				str[i] = c[2];
			if (c[1] == 'C' && sqrt(powf(i - n * (size[0] + 1) - o[0], 2) + powf(n - o[1], 2)) <= o[2] && str[i] != '\n')
				str[i] = c[2];
			if (str[i] == '\n')
				n++;
			i++;
		}
		count = fscanf(file, "%c %f %f %f %c\n", &c[1], &o[0], &o[1], &o[2], &c[2]);
		printf("%d\n", count);
	}
	i = 0;
	while (str[i] != 0)
	{
		write(1, &str[i], 1);
		i++;
	}
	free(str);
	fclose(file);
	return 0;
}
