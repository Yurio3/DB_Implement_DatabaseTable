
#ifndef TABLE_RW_C
#define TABLE_RW_C

#include <fstream>
#include "MyDB_PageReaderWriter.h"
#include "MyDB_TableReaderWriter.h"
#include <iostream>

using namespace std;

MyDB_TableReaderWriter::MyDB_TableReaderWriter(MyDB_TablePtr forMe, MyDB_BufferManagerPtr myBuffer) {
    std::cout << "Starting MyDB_TableReaderWriter constructor..." << std::endl;

    table = forMe;
    bufferMgr = myBuffer;
    lastPage = table->lastPage();
    
    std::cout << "Table name: " << table->getName() << std::endl;
    std::cout << "Last page from table: " << lastPage << std::endl;

    // Sanity check
    if (lastPage == (size_t)-1 || lastPage > 1000000) {  // Check for uninitialized or too large values
        std::cout << "Warning: Invalid lastPage value. Setting to 0." << std::endl;
        lastPage = 0;
        table->setLastPage(0);
    }

    // Initialize pages
    for (size_t i = 0; i <= lastPage; i++) {
        std::cout << "Creating PageReaderWriter for page " << i << "..." << std::endl;
        pages.push_back(MyDB_PageReaderWriter(bufferMgr, bufferMgr->getPage(table, i)));
    }

    std::cout << "MyDB_TableReaderWriter constructor completed. Total pages: " << pages.size() << std::endl;
}

MyDB_RecordPtr MyDB_TableReaderWriter :: getEmptyRecord () {
	return make_shared<MyDB_Record>(table->getSchema());
}



void MyDB_TableReaderWriter::append(MyDB_RecordPtr recordToAppend) {
    std::cout << "Appending record..." << std::endl;

    if (lastPage == (size_t)-1 || !pages[lastPage].append(recordToAppend)) {
        std::cout << "Creating new page for append. Current last page: " << lastPage << std::endl;
        lastPage++;
        if (lastPage >= pages.size()) {
            std::cout << "Extending pages vector. New size: " << (lastPage + 1) << std::endl;
            pages.push_back(MyDB_PageReaderWriter(bufferMgr, bufferMgr->getPage(table, lastPage)));
        }
        pages[lastPage].clear();
        bool success = pages[lastPage].append(recordToAppend);
        std::cout << "Append to new page " << (success ? "successful" : "failed") << std::endl;
    }

    table->setLastPage(lastPage);
    std::cout << "Record appended. Last page is now: " << lastPage << std::endl;
}

void MyDB_TableReaderWriter :: loadFromTextFile (string fromMe) {
    ifstream inFile(fromMe);
    string line;
    MyDB_RecordPtr record = getEmptyRecord();
    
    while (getline(inFile, line)) {
        record->fromString(line);
        append(record);
    }
    
    inFile.close();
    std::cout << "After loading, last page is: " << lastPage << std::endl;
    std::cout << "Total records loaded: " << (lastPage + 1) * bufferMgr->getPageSize() << std::endl;
}

MyDB_RecordIteratorPtr MyDB_TableReaderWriter :: getIterator (MyDB_RecordPtr iterateIntoMe) {
    return make_shared<MyDB_TableRecordIterator>(*this, iterateIntoMe);
}

void MyDB_TableReaderWriter :: writeIntoTextFile (string toMe) {
    ofstream outFile(toMe);
    MyDB_RecordPtr record = getEmptyRecord();
    MyDB_RecordIteratorPtr iter = getIterator(record);
    
    while (iter->hasNext()) {
        iter->getNext();
        outFile << record << endl;
    }
    
    outFile.close();
}

MyDB_PageReaderWriter MyDB_TableReaderWriter :: operator [] (size_t i) {
    if (i >= pages.size()) {
        pages.resize(i + 1, MyDB_PageReaderWriter(bufferMgr, bufferMgr->getPage(table, i)));
    }
    return pages[i];
}

MyDB_PageReaderWriter MyDB_TableReaderWriter :: last () {
    return (*this)[lastPage];
}
MyDB_TableRecordIterator :: MyDB_TableRecordIterator(MyDB_TableReaderWriter &parent, MyDB_RecordPtr record)
    : myParent(parent), myRec(record), curPage(0) {
    pageIter = myParent[curPage].getIterator(myRec);
}

void MyDB_TableRecordIterator :: getNext() {
    if (hasNext()) {
        pageIter->getNext();
    }
}

bool MyDB_TableRecordIterator :: hasNext() {
    while (!pageIter->hasNext() && curPage < myParent.getLastPage()) {
        curPage++;
        pageIter = myParent[curPage].getIterator(myRec);
    }
    return pageIter->hasNext();
}



#endif

