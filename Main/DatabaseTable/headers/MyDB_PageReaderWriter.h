
#ifndef PAGE_RW_H
#define PAGE_RW_H

#include "MyDB_PageType.h"
#include "MyDB_TableReaderWriter.h"
#include "MyDB_BufferManager.h"
#include <memory>


class MyDB_PageReaderWriter {

public:

	// ANY OTHER METHODS YOU WANT HERE


    MyDB_PageReaderWriter(MyDB_BufferManagerPtr bufferMgr, MyDB_PageHandle pageHandle);
   
	// empties out the contents of this page, so that it has no records in it
	// the type of the page is set to MyDB_PageType :: RegularPage
	void clear ();	

	// return an itrator over this page... each time returnVal->next () is
	// called, the resulting record will be placed into the record pointed to
	// by iterateIntoMe
	MyDB_RecordIteratorPtr getIterator (MyDB_RecordPtr iterateIntoMe);

	// appends a record to this page... return false is the append fails because
	// there is not enough space on the page; otherwise, return true
	bool append (MyDB_RecordPtr appendMe);

	// gets the type of this page... this is just a value from an ennumeration
	// that is stored within the page
	MyDB_PageType getType ();

	// sets the type of the page
	void setType (MyDB_PageType toMe);

	

	
private:

	MyDB_PageHandle getPage();
    MyDB_PageHandle myPage;
    MyDB_BufferManagerPtr bufferMgr;
};

class MyDB_PageRecordIterator : public MyDB_RecordIterator {
public:
    void getNext() override;
    bool hasNext() override;
    MyDB_PageRecordIterator(MyDB_PageHandle pageHandle, MyDB_RecordPtr recordPtr);
    
private:
    MyDB_PageHandle myPage;
    MyDB_RecordPtr myRec;
    int currentPos;
    int numRecs;
    char* bytes;
};


#endif
