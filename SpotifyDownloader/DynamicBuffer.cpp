#include <windows.h>
#include <stdio.h>
#include "DynamicBuffer.h"

DynamicBuffer::DynamicBuffer( int suggested_mem )
{
	int nBlocks = MAX_INIT_BLOCK;
	int init_mem = ( !suggested_mem || suggested_mem < 0 ) ? nBlocks * BLOCK_ALLOC : suggested_mem;

	int first_try = 1;

	mBuffer = NULL;

	do
	{
		
		if ( first_try )
		{
			mTotalSize = init_mem;
		}
		else
		{
			mTotalSize = nBlocks * BLOCK_ALLOC;
		}

		mBuffer = (uint8_t *)malloc(mTotalSize);

		if ( mBuffer )
		{
			memset(mBuffer, 0, mTotalSize);
		}
		else
		{
			nBlocks--;
		}
	}
	while ( !mBuffer ); 
 
	mReadPos = mWritePos = mBuffer;

	/***/
	//PrintState("Constructor");
}
	
char *DynamicBuffer::PrintState(char *msg)
{
	_snprintf_s(mOutputMessage, 1000, "%s: Buffer dim: %d - Free space: %d\r\n", msg, mTotalSize, CountFreeMemory());
	return mOutputMessage;
}

DynamicBuffer::~DynamicBuffer()
{
	if ( mBuffer )
	{
		free(mBuffer);
		mBuffer = NULL;
	}

	mReadPos = NULL;
	mWritePos = NULL;
}

int DynamicBuffer::ReadData ( void *dataout, int nbytes, int exclusive )
{
	if ( !dataout )
	{
		return -1;
	}

	//printf("Actual Buffer Dim: %d => read[%d] %d \r\n", mTotalSize, exclusive, nbytes);

	int ret = 0;
	int available_data = (int)(mWritePos - mReadPos);
	int to_be_read = (nbytes >= available_data)?(available_data):(nbytes);
	
	if ( exclusive && to_be_read != nbytes )
	{
		//PrintState("ReadData Exclusive");
		//flag exclusive indica che devono essere disponibili almeno nbytes di dati
		return 0;
	}

	if ( to_be_read > 0 )
	{
		memcpy(dataout, mReadPos, to_be_read);
		mReadPos = mReadPos + to_be_read;

		//riallineo la memoria
		MemoryRemap();
		ret = to_be_read;
	}

	//PrintState("ReadData NotExclusive");

	return ret;
}

int DynamicBuffer::WriteData ( void *datain, int nbytes )
{
	if ( !datain )
	{
		//PrintState("WriteData Error");
		return -1;
	}

	//printf("Actual Buffer Dim: %d => write %d \r\n", mTotalSize, nbytes);

	MemoryRemap();

	int free_mem = CountFreeMemory();
	int ret = -1;
	int new_available = 1;

	if ( free_mem < nbytes )
	{
		new_available = MemoryGrow(nbytes);
	}

	if ( new_available )
	{
		//copio
		memcpy(mWritePos, datain, nbytes);
		mWritePos += nbytes;
		ret = nbytes;
	}

	//PrintState("WriteData");

	return ret;

}

//mode 0 (write): ritorna la memoria privata garantendo almeno garantee_mem bytes
//mode 1 (read): ritorna la memoria solo se disponibili garantee_mem bytes
//in entrambi i casi, vengono incrementati i rispettivi cursori in memoria!!

void *DynamicBuffer::GetMemory( int nbytes, int mode )
{
	if ( !mBuffer )
	{
		return NULL;
	}

	MemoryRemap();
	void *ret = NULL;

	if ( mode == 0 ) 
	{
		int free_mem = CountFreeMemory();

		if ( nbytes <= free_mem )
		{
			ret = mWritePos;
			mWritePos += nbytes;
		}
		else
		{
			if ( MemoryGrow(nbytes) )
			{
				ret = mWritePos;
				mWritePos += nbytes;
			}
		}
	}
	else if ( mode == 1 )
	{
		int read_data = (int)(mWritePos - mReadPos);
		if ( read_data >= nbytes )
		{
			ret = mReadPos;
			mReadPos += nbytes;
		}
		else
		{
			ret = NULL;
		}
	}
	else 
	{

	}

	//PrintState(mode ? "WriteData-READMODE" : "WriteData-WRITEMODE");

	return ret;
}

void DynamicBuffer::MemoryRemap()
{
	if ( mBuffer )
	{
		//int read_mem = CountReadMemory();
		int to_be_moved = (int)(mWritePos - mReadPos);
		
		if ( mReadPos != mWritePos )
		{
			memmove(mBuffer, mReadPos, to_be_moved);
			mReadPos = mBuffer;
			mWritePos = mBuffer + to_be_moved;
		}
		else
		{
			//se cursore lettura == cursore scrittura, sposto
			mWritePos = mReadPos = mBuffer;
		}
	}

	//PrintState("MemoryRemap");
}

int DynamicBuffer::MemoryGrow( int min_mem )
{
	if ( mBuffer )
	{
		uint8_t *tmp_buff = NULL;
		int mem_allocated = 0;
		int block_dim = BLOCK_ALLOC;
		int write_offset = (int)(mWritePos - mBuffer);
		int read_offset = (int)(mReadPos - mBuffer);

		int new_total_size = mTotalSize + min_mem + block_dim;

		do 
		{
			tmp_buff = (uint8_t *)realloc(mBuffer, new_total_size);
			if ( !tmp_buff )
			{
				new_total_size -= 100;
				if ( new_total_size < mTotalSize + min_mem )
				{
					mem_allocated = 0;
					break;
				}
			}
			else
			{
				mem_allocated = 1;
			}
		}
		while ( !tmp_buff );

		//verifico ed aggiusto i dati
		int ret = mem_allocated;

		if ( mem_allocated )
		{
			//memoria allocata, aggiorno i puntatori;
			mBuffer = tmp_buff;
			mTotalSize = new_total_size;
			mWritePos = mBuffer + write_offset;
			mReadPos = mBuffer + read_offset;
		}	
		

		//PrintState("MemoryGrow");

		return ret;
	}

}

int DynamicBuffer::CountReadMemory()
{
	return (int)(mReadPos - mBuffer);
}

int DynamicBuffer::CountFreeMemory()
{
	return (mTotalSize - (int)(mWritePos - mReadPos));
}
