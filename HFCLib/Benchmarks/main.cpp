#include <hfclib.h>

#define DATA_LENGTH		50
#define COUNT			1000000

int main (void)
{
	fprintf(stdout,"Generating data.\n");
	fflush(stdout);

	srand(time(NULL));

	BYTE initval=rand()%240+1;

	Sleep(1000);

	char* pData=new char[DATA_LENGTH];
	char* pData2=new char[DATA_LENGTH];
	char* pDstData=new char[DATA_LENGTH];
	char* pDstData2=new char[DATA_LENGTH];

	for (DWORD i=0;i<DATA_LENGTH;i++)
	{
		pData[i]=BYTE(i);
		pData2[i]=rand();
	}
	

	fprintf(stdout,"Benchmarking memory copiers.\n");
	fflush(stdout);

	
	iMemSet(pDstData,initval,DATA_LENGTH);
	iMemSet(pDstData2,initval,DATA_LENGTH);


	// CopyMemory
	DWORD dwStart=GetTickCount();
	for (int i=0;i<COUNT;i++)
	{
		CopyMemory(pDstData,pData,DATA_LENGTH);
		CopyMemory(pDstData2,pData2,DATA_LENGTH);
	}
	DWORD dwStop=GetTickCount();
	fprintf(stdout,"\tCopyMemory:  %4d\n",dwStop-dwStart);
	fflush(stdout);

	iMemSet(pDstData,initval,DATA_LENGTH);
	iMemSet(pDstData2,initval,DATA_LENGTH);

	// strncopy
	dwStart=GetTickCount();
	for (int i=0;i<COUNT;i++)
	{
		strncpy(pDstData,pData,DATA_LENGTH);
		strncpy(pDstData2,pData2,DATA_LENGTH);
	}
	dwStop=GetTickCount();
	fprintf(stdout,"\tstrncpy:   %4d\n",dwStop-dwStart);
	fflush(stdout);

	iMemSet(pDstData,initval,DATA_LENGTH);
	iMemSet(pDstData2,initval,DATA_LENGTH);

	// MemCopy
	dwStart=GetTickCount();
	for (int i=0;i<COUNT;i++)
	{
		MemCopy(pDstData,pData,DATA_LENGTH);
		MemCopy(pDstData2,pData2,DATA_LENGTH);
	}
	dwStop=GetTickCount();
	fprintf(stdout,"\tMemCopy:    %4d\n",dwStop-dwStart);
	fflush(stdout);

	iMemSet(pDstData,initval,DATA_LENGTH);
	iMemSet(pDstData2,initval,DATA_LENGTH);

	// iMemCopy
	dwStart=GetTickCount();
	for (int i=0;i<COUNT;i++)
	{
		iMemCopy(pDstData,pData,DATA_LENGTH);
		iMemCopy(pDstData2,pData2,DATA_LENGTH);
	}
	dwStop=GetTickCount();
	fprintf(stdout,"\tiMemCopy:    %4d\n",dwStop-dwStart);
	fflush(stdout);

	iMemSet(pDstData,initval,DATA_LENGTH);
	iMemSet(pDstData2,initval,DATA_LENGTH);

	// dMemCopy
	dwStart=GetTickCount();
	for (int i=0;i<COUNT;i++)
	{
		dMemCopy(pDstData,pData,DATA_LENGTH);
		dMemCopy(pDstData2,pData2,DATA_LENGTH);
	}
	dwStop=GetTickCount();
	fprintf(stdout,"\tdMemCopy:    %4d\n",dwStop-dwStart);
	fflush(stdout);

	iMemSet(pDstData,initval,DATA_LENGTH);
	iMemSet(pDstData2,initval,DATA_LENGTH);

	// dMemCopy
	dwStart=GetTickCount();
	for (int i=0;i<COUNT;i++)
	{
		fMemCopy(pDstData,pData,DATA_LENGTH);
		fMemCopy(pDstData2,pData2,DATA_LENGTH);
	}
	dwStop=GetTickCount();
	fprintf(stdout,"\tfMemCopy:    %4d\n",dwStop-dwStart);
	fflush(stdout);

	fprintf(stdout,"Benchmarking memory setters to zero.\n");
	fflush(stdout);

	iMemSet(pDstData,initval,DATA_LENGTH);
	iMemSet(pDstData2,initval,DATA_LENGTH);

	// ZeroMemory
	dwStart=GetTickCount();
	for (int i=0;i<COUNT;i++)
	{
		ZeroMemory(pDstData,DATA_LENGTH);
		ZeroMemory(pDstData2,DATA_LENGTH);
	}
	dwStop=GetTickCount();
	fprintf(stdout,"\tZeroMemory:  %4d\n",dwStop-dwStart);
	fflush(stdout);

	iMemSet(pDstData,initval,DATA_LENGTH);
	iMemSet(pDstData2,initval,DATA_LENGTH);

	// MemSet
	dwStart=GetTickCount();
	for (int i=0;i<COUNT;i++)
	{
		MemSet(pDstData,0,DATA_LENGTH);
		MemSet(pDstData2,0,DATA_LENGTH);
	}
	dwStop=GetTickCount();
	fprintf(stdout,"\tMemSet:    %4d\n",dwStop-dwStart);
	fflush(stdout);

	iMemSet(pDstData,initval,DATA_LENGTH);
	iMemSet(pDstData2,initval,DATA_LENGTH);

	// iMemSet
	dwStart=GetTickCount();
	for (int i=0;i<COUNT;i++)
	{
		iMemSet(pDstData,0,DATA_LENGTH);
		iMemSet(pDstData2,0,DATA_LENGTH);
	}
	dwStop=GetTickCount();
	fprintf(stdout,"\tiMemSet:    %4d\n",dwStop-dwStart);
	fflush(stdout);

	iMemSet(pDstData,initval,DATA_LENGTH);
	iMemSet(pDstData2,initval,DATA_LENGTH);

	// dMemSet
	dwStart=GetTickCount();
	for (int i=0;i<COUNT;i++)
	{
		dMemSet(pDstData,0,DATA_LENGTH);
		dMemSet(pDstData2,0,DATA_LENGTH);
	}
	dwStop=GetTickCount();
	fprintf(stdout,"\tdMemSet:    %4d\n",dwStop-dwStart);
	fflush(stdout);

	iMemSet(pDstData,initval,DATA_LENGTH);
	iMemSet(pDstData2,initval,DATA_LENGTH);

	// fMemSet
	dwStart=GetTickCount();
	for (int i=0;i<COUNT;i++)
	{
		fMemSet(pDstData,0,DATA_LENGTH);
		fMemSet(pDstData2,0,DATA_LENGTH);
	}
	dwStop=GetTickCount();
	fprintf(stdout,"\tfMemSet:    %4d\n",dwStop-dwStart);
	fflush(stdout);

	fprintf(stdout,"Benchmarking memory setters to random value.\n");
	fflush(stdout);

	BYTE val=rand();

	iMemSet(pDstData,initval,DATA_LENGTH);
	iMemSet(pDstData2,initval,DATA_LENGTH);

	// MemSet
	dwStart=GetTickCount();
	for (int i=0;i<COUNT;i++)
	{
		MemSet(pDstData,val,DATA_LENGTH);
		MemSet(pDstData2,val,DATA_LENGTH);
	}
	dwStop=GetTickCount();
	fprintf(stdout,"\tMemSet:    %4d\n",dwStop-dwStart);
	fflush(stdout);

	iMemSet(pDstData,initval,DATA_LENGTH);
	iMemSet(pDstData2,initval,DATA_LENGTH);

	// iMemSet
	dwStart=GetTickCount();
	for (int i=0;i<COUNT;i++)
	{
		iMemSet(pDstData,val,DATA_LENGTH);
		iMemSet(pDstData2,val,DATA_LENGTH);
	}
	dwStop=GetTickCount();
	fprintf(stdout,"\tiMemSet:    %4d\n",dwStop-dwStart);
	fflush(stdout);

	iMemSet(pDstData,initval,DATA_LENGTH);
	iMemSet(pDstData2,initval,DATA_LENGTH);

	// dMemSet
	dwStart=GetTickCount();
	for (int i=0;i<COUNT;i++)
	{
		dMemSet(pDstData,val,DATA_LENGTH);
		dMemSet(pDstData2,val,DATA_LENGTH);
	}
	dwStop=GetTickCount();
	fprintf(stdout,"\tdMemSet:    %4d\n",dwStop-dwStart);
	fflush(stdout);

	iMemSet(pDstData,initval,DATA_LENGTH);
	iMemSet(pDstData2,initval,DATA_LENGTH);

	// fMemSet
	dwStart=GetTickCount();
	for (int i=0;i<COUNT;i++)
	{
		fMemSet(pDstData,val,DATA_LENGTH);
		fMemSet(pDstData2,val,DATA_LENGTH);
	}
	dwStop=GetTickCount();
	fprintf(stdout,"\tfMemSet:    %4d\n",dwStop-dwStart);
	fflush(stdout);

	delete[] pDstData;
	delete[] pDstData2;

	fprintf(stdout,"Benchmarking string length calculators.\n");
	fflush(stdout);


	
	iMemSet(pData,rand()%200+10,DATA_LENGTH-1);
	iMemSet(pData2,rand()%200+10,DATA_LENGTH-1);
	pData[DATA_LENGTH-1]='\0';
	pData2[DATA_LENGTH-1]='\0';

	size_t dwLength,dwLength2;

	// strlen
	dwStart=GetTickCount();
	for (int i=0;i<COUNT;i++)
	{
		dwLength=strlen(pData);
		dwLength2=strlen(pData2);
	}
	dwStop=GetTickCount();
	fprintf(stdout,"\tstrlen:    %4d\n",dwStop-dwStart);
	fflush(stdout);

	printf("%d %d\n",dwLength,dwLength2);

	iMemSet(pData,rand()%200+10,DATA_LENGTH-1);
	iMemSet(pData2,rand()%200+10,DATA_LENGTH-1);
	pData[DATA_LENGTH-1]='\0';
	pData2[DATA_LENGTH-1]='\0';

	// istrlen
	dwStart=GetTickCount();
	for (int i=0;i<COUNT;i++)
	{
		dwLength=istrlen(pData);
		dwLength=istrlen(pData2);
	}
	dwStop=GetTickCount();
	fprintf(stdout,"\tistrlen:    %4d\n",dwStop-dwStart);
	fflush(stdout);

    // fstrlen
	dwStart=GetTickCount();
	for (int i=0;i<COUNT;i++)
	{
		dwLength=fstrlen(pData);
		dwLength=fstrlen(pData2);
	}
	dwStop=GetTickCount();
	fprintf(stdout,"\tfstrlen:    %4d\n",dwStop-dwStart);
	fflush(stdout);

    	
	// dstrlen
	dwStart=GetTickCount();
	for (int i=0;i<COUNT;i++)
	{
		dstrlen(pData,dwLength);
		dstrlen(pData2,dwLength);
	}
	dwStop=GetTickCount();
	fprintf(stdout,"\tdstrlen:    %4d\n",dwStop-dwStart);
	fflush(stdout);

    	
	delete[] pData;
	delete[] pData2;
	
	return 0;
}