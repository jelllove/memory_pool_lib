#ifndef MEMORY_BLOCK_H_FILE_INCLUDE
#define MEMORY_BLOCK_H_FILE_INCLUDE

#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>

class CMemoryBlock
{
public:
	CMemoryBlock(unsigned int nSize);
	~CMemoryBlock();
	
	char* GetMemory();
	unsigned int GetSize();
	bool Realloc(unsigned int nNewSize);
	void Free();

public:
	CMemoryBlock *m_pNext;
	CMemoryBlock *m_pPre;

private:
	unsigned int m_nSize;
	char* m_pMemory;
};

#endif


