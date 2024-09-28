
#ifndef PAGE_RW_C
#define PAGE_RW_C

#include "MyDB_PageReaderWriter.h"
#include <memory>

MyDB_PageReaderWriter::MyDB_PageReaderWriter(MyDB_BufferManagerPtr bufferMgr, MyDB_PageHandle pageHandle)
    : bufferMgr(bufferMgr), myPage(pageHandle) {}

MyDB_PageHandle MyDB_PageReaderWriter::getPage() {
    return myPage;
}


void MyDB_PageReaderWriter::clear() {
    char *bytes = (char *)myPage->getBytes();
    size_t pageSize = bufferMgr->getPageSize();
    memset(bytes, 0, pageSize);
    *((int *)bytes) = 0;  // Set number of records to 0
    myPage->wroteBytes();
}


MyDB_PageType MyDB_PageReaderWriter::getType() {
    char *bytes = (char *)myPage->getBytes();
    return *((MyDB_PageType *)(bytes + sizeof(int)));
}


void MyDB_PageReaderWriter::setType(MyDB_PageType toMe) {
    char *bytes = (char *)myPage->getBytes();
    *((MyDB_PageType *)(bytes + sizeof(int))) = toMe;
    myPage->wroteBytes();
}

bool MyDB_PageReaderWriter::append(MyDB_RecordPtr appendMe) {
    char *bytes = (char *)myPage->getBytes();
    int numRecs = *((int *)bytes);
    size_t pageSize = bufferMgr->getPageSize();
    size_t recordSize = appendMe->getBinarySize();
    
    size_t headerSize = sizeof(int) + sizeof(MyDB_PageType);
    size_t usedSpace = headerSize + numRecs * recordSize;
    
    if (usedSpace + recordSize > pageSize) {
        std::cout << "Page full. Records on this page: " << numRecs << std::endl;
        return false;  // Not enough space
    }
    
    appendMe->toBinary(bytes + usedSpace);
    *((int *)bytes) = numRecs + 1;
    myPage->wroteBytes();
    
    std::cout << "Record appended. Records on this page: " << (numRecs + 1) 
              << ", Used space: " << (usedSpace + recordSize) << "/" << pageSize << std::endl;
    return true;
}

MyDB_RecordIteratorPtr MyDB_PageReaderWriter::getIterator(MyDB_RecordPtr iterateIntoMe) {
    return make_shared<MyDB_PageRecordIterator>(myPage, iterateIntoMe);
}




MyDB_PageRecordIterator :: MyDB_PageRecordIterator(MyDB_PageHandle pageHandle, MyDB_RecordPtr recordPtr) : 
    myPage(pageHandle), myRec(recordPtr) {
    bytes = (char *) myPage->getBytes();
    numRecs = *((int *) bytes);
    currentPos = 0;
}

void MyDB_PageRecordIterator :: getNext() {
    if (hasNext()) {
        int offset = sizeof(int) + currentPos * myRec->getBinarySize();
        myRec->fromBinary(&bytes[offset]);
        currentPos++;
    }
}

bool MyDB_PageRecordIterator :: hasNext() {
    return currentPos < numRecs;
}


#endif
