#include "ExeElementWrapper.h"

ExeElementWrapper::ExeElementWrapper(Executable *v_exe)
{
    if (v_exe == NULL) {
        printf("Cannot initialize with Exe == NULL!");
        throw CustomException("Cannot initialize with Exe == NULL!");
    }
    this->m_Exe = v_exe;
}

bufsize_t ExeElementWrapper::getFieldSize(size_t fieldId, size_t subField)
{
    size_t fieldsCount = getFieldsCount();
    if (fieldId >= fieldsCount) return this->getSize();

    void *ptr = this->getFieldPtr(fieldId, subField);
    if (ptr == NULL) return 0;

    void *nextPtr = NULL;
    size_t nextFID = fieldId + 1;

    if (nextFID < fieldsCount ) nextPtr = this->getFieldPtr(nextFID, subField);
    if (nextPtr != NULL) {
        int64_t dif = (offset_t) nextPtr - (offset_t) ptr;
        if (dif < 0) return 0;

        return (bufsize_t) dif;
    }

    offset_t fullSize = this->getSize();
    BYTE *bgnPtr =  (BYTE*) getPtr();
    BYTE *endPtr = bgnPtr + fullSize;
    offset_t dif =  (offset_t) endPtr - (offset_t) ptr;
    if (dif < 0) return 0;

    return (bufsize_t) dif;
}

offset_t ExeElementWrapper::getOffset()
{
    BYTE *ptr = (BYTE*)this->getPtr();
    if (ptr == NULL) return INVALID_ADDR;
    return getOffset(ptr);
}

offset_t ExeElementWrapper::getOffset(void *ptr)
{
    BYTE* content = m_Exe->getContent();
    if (!content || !ptr) return INVALID_ADDR;
    offset_t offset = (offset_t) ptr - (offset_t) content;
    return offset;
}

offset_t ExeElementWrapper::getFieldOffset(size_t fieldId, size_t subField)
{
    void* ptr = getFieldPtr(fieldId, subField);
    return getOffset(ptr);
}

WrappedValue ExeElementWrapper::getWrappedValue(size_t fieldId, size_t subFieldId)
{
    if (fieldId == -1) return WrappedValue(); //EMPTY
    void* ptr = getFieldPtr(fieldId, subFieldId);
    if (ptr == NULL) return WrappedValue(); //EMPTY

    bufsize_t fieldSize = getFieldSize(fieldId, subFieldId);
    if (fieldSize == BUFSIZE_MAX || fieldSize == 0) return WrappedValue(); //EMPTY

    offset_t offset = this->getOffset(ptr);
    if (offset == INVALID_ADDR) return WrappedValue(); //EMPTY

    WrappedValue::data_type dataType = this->containsDataType(fieldId, subFieldId);
    return WrappedValue(this->m_Exe, offset, fieldSize, dataType);
}

uint64_t ExeElementWrapper::getNumValue(size_t fId, size_t subField, bool* isOk)
{
    if (isOk) (*isOk) = false;
    if (this->m_Exe == NULL) return (-1);

    offset_t offset = this->getFieldOffset(fId, subField);
    bufsize_t size = this->getFieldSize(fId, subField);
    return this->m_Exe->getNumValue(offset, size, isOk);
}

bool ExeElementWrapper::setNumValue(size_t fId, size_t subField, uint64_t newVal)
{
    if (this->m_Exe == NULL) return false;

    offset_t offset = this->getFieldOffset(fId, subField);
    bufsize_t size = this->getFieldSize(fId, subField);

    return this->m_Exe->setNumValue(fId, subField, newVal);
}

//TODO: check it!!
bool ExeElementWrapper::setStringValue(char* dstPtr, QString newText)
{
    const char* newTextC = newText.toStdString().c_str();
    bufsize_t newTextLen = static_cast<bufsize_t>(strlen(newTextC));
    bool isOk = m_Exe->setBufferedValue((BYTE*)dstPtr, (BYTE*)newTextC, newTextLen, 1);
    return isOk;
}
