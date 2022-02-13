
// standard headers

// project headers
#include "MUSI6106Config.h"

#include "ErrorDef.h"
#include "Util.h"

#include "CombFilterIf.h"
#include "CombFilter.h"

static const char*  kCMyProjectBuildDate = __DATE__;


CCombFilterIf::CCombFilterIf() : m_bIsInitialized(false) {
    // this should never hurt
    reset();
}

CCombFilterIf::~CCombFilterIf() {
    reset();
}

const int CCombFilterIf::getVersion(const Version_t eVersionIdx) {
    int iVersion = 0;

    switch (eVersionIdx) {
    case kMajor:
        iVersion = MUSI6106_VERSION_MAJOR; 
        break;
    case kMinor:
        iVersion = MUSI6106_VERSION_MINOR; 
        break;
    case kPatch:
        iVersion = MUSI6106_VERSION_PATCH; 
        break;
    case kNumVersionInts:
        iVersion = -1;
        break;
    }

    return iVersion;
}
const char* CCombFilterIf::getBuildDate() {
    return kCMyProjectBuildDate;
}

Error_t CCombFilterIf::create(CCombFilterIf*& pCCombFilter) {
    pCCombFilter = new CCombFilterIf();
    return Error_t::kNoError;
}

Error_t CCombFilterIf::destroy(CCombFilterIf*& pCCombFilter) {
    delete pCCombFilter;
    return Error_t::kNoError;
}

Error_t CCombFilterIf::init(CombFilterType_t eFilterType, float fMaxDelayLengthInS, float fSampleRateInHz, int iNumChannels) {
    // For simplicity, the underlying filter implementations are mono.
    // So, we allocate and initialize one filter instance per channel.
    if (iNumChannels < 0 || fMaxDelayLengthInS < 1 / fSampleRateInHz) {
        return Error_t::kFunctionInvalidArgsError;
    } else if (eFilterType == CombFilterType_t::kCombFIR) {
        for (int i = 0; i < iNumChannels; i++) {
            filters.emplace_back(new FIRFilter(fMaxDelayLengthInS, fSampleRateInHz));
        }
    } else if (eFilterType == CombFilterType_t::kCombIIR) {
        for (int i = 0; i < iNumChannels; i++) {
            filters.emplace_back(new IIRFilter(fMaxDelayLengthInS, fSampleRateInHz));
        }
    } else {
        return Error_t::kFunctionInvalidArgsError;
    }
    m_bIsInitialized = true;
    return Error_t::kNoError;
}

Error_t CCombFilterIf::reset() {
    // NOTE: Because we're using std::unique_ptr, clearing the vector will automatically delete the FIR/IIRFilter instances.
    filters.clear();
    return Error_t::kNoError;
}

Error_t CCombFilterIf::process(float **ppfInputBuffer, float **ppfOutputBuffer, int iNumberOfFrames) {
    if (!m_bIsInitialized) {
        return Error_t::kNotInitializedError;
    }
    // Pass each input/output channel to the corresponding filter.
    for (int i = 0; i < filters.size(); i++) {
        filters[i]->process(ppfInputBuffer[i], ppfOutputBuffer[i], iNumberOfFrames);
    }
    return Error_t::kNoError;
}

Error_t CCombFilterIf::setParam(FilterParam_t eParam, float fParamValue) {
    if (!m_bIsInitialized) {
        return Error_t::kNotInitializedError;
    }
    // Set the parameter on each of the filters in the array.
    if (eParam == FilterParam_t::kParamGain) {
        for (auto &filter : filters) {
            Error_t error = filter->setGain(fParamValue);
            if (error != Error_t::kNoError) {
                return error;
            }
        }
    } else if (eParam == FilterParam_t::kParamDelay) {
        for (auto &filter : filters) {
            Error_t error = filter->setDelay(fParamValue);
            if (error != Error_t::kNoError) {
                return error;
            }
        }
    } else {
        return Error_t::kFunctionInvalidArgsError;
    }
    return Error_t::kNoError;
}

float CCombFilterIf::getParam(FilterParam_t eParam) const {
    if (!m_bIsInitialized || filters.empty()) {
        // Per the interface, this function doesn't return Error_t, so we can't indicate the error here
        // (without e.g. introducing exceptions or returning something strange like NaN).
        return 0;
    }
    // Just get the parameter from the first filter in the array, since all the filters have the same parameters.
    if (eParam == FilterParam_t::kParamGain) {
        return filters[0]->getGain();
    } else if (eParam == FilterParam_t::kParamDelay) {
        return filters[0]->getDelay();
    }
    // Again, can't indicate the error (invalid parameter) here.
    return 0;
}
