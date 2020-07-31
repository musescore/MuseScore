#if !defined(DARRAY_H)
#define DARRAY_H

class darray
{
protected:
	char *mData;
	int mUsed;
	int mAllocated;
	int mAllocChunk;
public:
	darray();
	~darray();
	void clear();
	char *getDataInPos(int aPosition);
	void put(int aData);
	int getSize() const { return mUsed; }
	char *getData() { return mData; } 
};

#endif

