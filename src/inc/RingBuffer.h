#if !defined(__RingBuffer_hdr__)
#define __RingBuffer_hdr__

#include <cassert>
#include <algorithm>
#include <cmath>
#include <cstring>

/*! \brief implement a circular buffer of type T
*/
template <class T> 
class CRingBuffer {
public:
    explicit CRingBuffer(int lengthInSamples) : length(lengthInSamples) {
        assert(lengthInSamples > 0);

        // allocate and init
        buffer = new T[lengthInSamples];
        reset();
    }

    virtual ~CRingBuffer() {
        // free memory
        delete[] buffer;
    }

    /*! add a new value of type T to write index and increment write index
    \param tNewValue the new value
    \return void
    */
    void putPostInc(T tNewValue) {
        // NOTE: This does not perform error checking for the full buffer case.
        buffer[postInc(head)] = tNewValue;
    }

    /*! add a new value of type T to write index
    \param tNewValue the new value
    \return void
    */
    void put(T tNewValue) {
        buffer[head] = tNewValue;
    }
    
    /*! return the value at the current read index and increment the read pointer
    \return float the value from the read index
    */
    T getPostInc() {
        // NOTE: This does not perform error checking for the empty buffer case.
        return buffer[postInc(tail)];
    }

    /*! return the value at the current read index
    \return float the value from the read index
    */
    T get() const {
        return buffer[tail];
    }
    
    /*! set buffer content and indices to 0
    \return void
    */
    void reset() {
        std::memset(buffer, 0, sizeof(T) * length);
        head = 0;
        tail = 0;
    }

    /*! return the current index for writing/put
    \return int
    */
    int getWriteIdx() const {
        return head;
    }

    /*! move the write index to a new position
    \param iNewWriteIdx: new position
    \return void
    */
    void setWriteIdx(int iNewWriteIdx) {
        head = iNewWriteIdx;
    }

    /*! return the current index for reading/get
    \return int
    */
    int getReadIdx() const {
        return tail;
    }

    /*! move the read index to a new position
    \param iNewReadIdx: new position
    \return void
    */
    void setReadIdx(int iNewReadIdx) {
        tail = iNewReadIdx;
    }

    /*! returns the number of values currently buffered (note: 0 could also mean the buffer is full!)
    \return int
    */
    int getNumValuesInBuffer() const {
        return head >= tail ? (head - tail) : (head + length - tail);
    }

    /*! returns the length of the internal buffer
    \return int
    */
    int getLength() const {
        return length;
    }
private:
    CRingBuffer();
    CRingBuffer(const CRingBuffer& that);
    int postInc(int &index) {
       const int value = index++;
       if (index == length) index = 0;
       return value; 
    }

    int length;              //!< length of the internal buffer
    T* buffer;
    int head, tail;
};
#endif // __RingBuffer_hdr__
