#include "IMemoryPool.h"
#include <stdio.h>
#include <unistd.h>

int main()
{
	IMemoryPool *pM = CreateMemoryPool(1, 2);

	if (pM != NULL)
	{
		char *p = pM->malloc(1000);
		for (int i = 0; i < 999; i++)
		{
				p[i] = 'a';
		}
		p[999] = '\0';
		printf(p);

		pM->free(p);
	}

	pM->destory();

	return 0;
}


