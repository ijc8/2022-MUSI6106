#if !defined(__RingBuffer_hdr__)
#define __RingBuffer_hdr__

#include <cassert>
#include <algorithm>
#include <cmath>
#include <cstring>

// Helper function because C++'s modulo operator (%) can return negative results.
inline int mod(int a, int b) {
    int r = a % b;
    return r < 0 ? r + b : r;
}

inline float mod(float a, float b) {
    float r = fmod(a, b);
    return r < 0 ? r + b : r;
}

/*! \brief implement a circular buffer of type T
*/
template <class T> 
class RingBuffer {
public:
    explicit RingBuffer(int lengthInSamples) : length(lengthInSamples) {
        assert(lengthInSamples > 0);

        // allocate and init
        buffer = new T[lengthInSamples];
        reset();
    }

    virtual ~RingBuffer() {
        // free memory
        delete[] buffer;
    }

    /*! add a new value of type T to write index and increment write index
    \param tNewValue the new value
    \return void
    */
    void putPostInc(T tNewValue) {
        // NOTE: This does not perform error checking for the full buffer case.
        int index = postInc(head);
        buffer[index] = tNewValue;
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

    /*! return the value at the current read index (with optional offset)
    \param iOffset (optional) offset from the read index
    \return T the value from the offset index
    */
    T get(int iOffset = 0) const {
        return buffer[mod(tail + iOffset, length)];
    }

    /*! return the value at the current read index (with optional offset). performs linear interpolation.
    \param fOffset (optional) offset from the read index
    \return T the value from the offset index
    */
    T get(float fOffset) const {
        // Compute fractional index between [0, length).
        float fracIndex = mod(tail + fOffset, (float)length);
        // Get samples before and after fractional index.
        int index = fracIndex;
        float prevSample = buffer[index];
        float nextSample = buffer[(index + 1) % length];
        // Compute weighted sum of samples before and after fractional index.
        float frac = fracIndex - index;
        return prevSample * (1 - frac) + nextSample * frac;
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
        head = mod(iNewWriteIdx, length);
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
        tail = mod(iNewReadIdx, length);
    }

    /*! returns the number of values currently buffered (note: 0 could also mean the buffer is full!)
    \return int
    */
    int getNumValuesInBuffer() const {
        // If `tail > head`, we assume it's because `head` has wrapped around.
        return tail > head ? (head + length - tail) : (head - tail);
    }

    /*! returns the length of the internal buffer
    \return int
    */
    int getLength() const {
        return length;
    }
private:
    RingBuffer();
    RingBuffer(const RingBuffer& that);

    // Perform post-increment with wrapping.
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
