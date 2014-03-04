#ifndef UTIL_H
#define UTIL_H

int strrnicmp(const char *src, const char *dst, int limit)
{
	// derived from VString function
	// first determine our maximum running length
	int i = strlen(src) - 1;
	int j = strlen(dst) - 1;
	int ret = 0;

	limit--;

	if (i < limit)
	{
		if (i < j)
		{
			return -1;
		}
		else if (j < i)
		{
			return 1;
		}
		limit = i;
	}
	if (j < limit)
	{
		if (i < j)
		{
			return -1;
		}
		else if (j < i)
		{
			return 1;
		}
		limit = j;
	}

	src = src + i;
	dst = dst + j;

	while (limit >= 0)
	{
		i = tolower(*src);
		j = tolower(*dst);

		ret = i - j;
		if (ret)
		{
			if (ret < 0)
			{
				return -1;
			}
			else
			{
				return 1;
			}
		}
		src--;
		dst--;
		limit--;
	}

	return 0;
}

#endif // UTIL_H
