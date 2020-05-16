/*
	memory pool interface
*/
#ifndef MEMORY_POOL_H_FILE_INCLUDE
#define MEMORY_POOL_H_FILE_INCLUDE

//每个内存块的大小
#define MEM_POOL_INIT_SIZE 1024 * 1024

//初始化内存块的个数
#define MEM_POOL_INIT_BLOCK_NUM 10

class IMemoryPool
{
public:
	virtual char* malloc(unsigned int nSize) = 0;
	virtual char* realloc(char *pOldPtr, unsigned int nNewSize) = 0;
	virtual bool free(char *pPtr) = 0;

	//free all memory
	virtual void destory() = 0;

	//if failed to malloc, realloc, free, return the error msg;
	virtual char* errormsg() = 0;
};

extern "C" 
{
		IMemoryPool* CreateMemoryPool(unsigned int nSize = MEM_POOL_INIT_SIZE, unsigned int nNum = MEM_POOL_INIT_BLOCK_NUM);
}


#endif
