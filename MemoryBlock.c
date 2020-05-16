#include "MemoryBlock.h"

CMemoryBlock::CMemoryBlock(unsigned int nSize)
{
	m_pNext = NULL;
	m_nSize = 0;
	m_pMemory = NULL;
	m_pPre = NULL;

	Realloc(nSize);
}
	
CMemoryBlock::~CMemoryBlock()
{
	Free();
}
	
char* CMemoryBlock::GetMemory()
{
	return m_pMemory;
}
	
unsigned int CMemoryBlock::GetSize()
{
	return m_nSize;
}

bool CMemoryBlock::Realloc(unsigned int nNewSize)
{
	void *pTmp = realloc(m_pMemory, nNewSize);

	if (pTmp == NULL)
	{
		return false;
	}
	else
	{
		m_pMemory = (char *)pTmp;
		m_nSize = nNewSize;

		return true;
	}
}

void CMemoryBlock::Free()
{
	free(m_pMemory);
	m_pMemory = NULL;
	m_nSize = 0;
}
