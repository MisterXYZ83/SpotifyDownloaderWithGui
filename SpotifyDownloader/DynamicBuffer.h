
#include <stdint.h>

#define BLOCK_ALLOC			(4096*2)
#define MAX_INIT_BLOCK		4

class DynamicBuffer 
{

private:

	uint8_t *mBuffer;

	uint8_t *mReadPos;
	uint8_t *mWritePos;

	int mTotalSize;

	void MemoryRemap();
	int MemoryGrow( int min_mem );

	inline int CountReadMemory();

	char	mOutputMessage[1000+1];

public:

	DynamicBuffer(int suggested_mem = 0);
	~DynamicBuffer();

	int ReadData ( void *dataout, int nbytes, int exclusive_flag = 1 );
	int WriteData ( void *datain, int nbytes );
	
	void *GetMemory( int nbytes, int mode = 0 ); //rischiosa !!

	char *PrintState(char *msg);

	inline int CountFreeMemory();
};