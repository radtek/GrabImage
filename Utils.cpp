#include <streams.h>
#include <windows.h>
#include <stdio.h>
#include <crtdbg.h>

#ifdef _DEBUG
extern FILE *stream;
#endif

HRESULT GetUnconnectedPin(
    IBaseFilter *pFilter,   // Pointer to the filter.
    PIN_DIRECTION PinDir,   // Direction of the pin to find.
    IPin **ppPin);           // Receives a pointer to the pin.

//****************************************************************************************
//The following function connects an output pin from one filter
//to the first available input pin on another filter:

HRESULT ConnectFilters(
    IGraphBuilder *pGraph, // Filter Graph Manager.
    IPin *pOut,            // Output pin on the upstream filter.
    IBaseFilter *pDest)    // Downstream filter.
{
    if ((pGraph == NULL) || (pOut == NULL) || (pDest == NULL)) {
        return E_POINTER;
    }

#ifdef _DEBUG
        PIN_DIRECTION PinDir;
        pOut->QueryDirection(&PinDir);
        _ASSERTE(PinDir == PINDIR_OUTPUT);
#endif

    // Find an input pin on the downstream filter.
    IPin *pIn = 0;
    HRESULT hr = GetUnconnectedPin(pDest, PINDIR_INPUT, &pIn);
    if (FAILED(hr)) {
        return hr;
    }
    // Try to connect them.
    hr = pGraph->Connect(pOut, pIn);
    pIn->Release();
/*
#ifdef _DEBUG
	fprintf(stream, "Util code1: is %x\n", hr);

	fprintf(stream, "GetCurrentBuffer ret code is %p\n", pOut);
	fprintf(stream, "GetCurrentBuffer ret code is %p\n", pIn);
	fprintf(stream, "GetCurrentBuffer ret code is %x\n", VFW_S_PARTIAL_RENDER);
	fprintf(stream, "GetCurrentBuffer ret code is %x\n", E_ABORT);
	fprintf(stream, "GetCurrentBuffer ret code is %x\n", E_POINTER);
	fprintf(stream, "GetCurrentBuffer ret code is %x\n", VFW_E_NOT_CONNECTED);
	fprintf(stream, "GetCurrentBuffer ret code is %x\n", VFW_E_NOT_IN_GRAPH);

#endif
*/
    return hr;
}

//****************************************************************************************
//Here is an overloaded version of the same function. 
//The second parameter is a pointer to a filter, rather than a pin. The function connects the first filter to the second filter: 

HRESULT ConnectFilters(
    IGraphBuilder *pGraph, 
    IBaseFilter *pSrc, 
    IBaseFilter *pDest)
{
    if ((pGraph == NULL) || (pSrc == NULL) || (pDest == NULL)) {
        return E_POINTER;
    }

    // Find an output pin on the first filter.
    IPin *pOut = 0;
    HRESULT hr = GetUnconnectedPin(pSrc, PINDIR_OUTPUT, &pOut);
    if (FAILED(hr)) {
        return hr;
    }
    hr = ConnectFilters(pGraph, pOut, pDest);
    pOut->Release();

    return hr;
}

//****************************************************************************************
//The following function searches on a filter for an unconnected pin,
//either input or output. It returns the first matching pin.
//Finding unconnected pins is useful when you are connecting filters.

HRESULT GetUnconnectedPin(
    IBaseFilter *pFilter,   // Pointer to the filter.
    PIN_DIRECTION PinDir,   // Direction of the pin to find.
    IPin **ppPin)           // Receives a pointer to the pin.
{
    *ppPin = 0;
    IEnumPins *pEnum = 0;
    IPin *pPin = 0;
    HRESULT hr = pFilter->EnumPins(&pEnum);
    if (FAILED(hr)) {
        return hr;
    }

    while (pEnum->Next(1, &pPin, NULL) == S_OK) {
        PIN_DIRECTION ThisPinDir;
        pPin->QueryDirection(&ThisPinDir);
        if (ThisPinDir == PinDir) {
/*
			PIN_INFO pInfo;
			hr = pPin->QueryPinInfo(&pInfo);
//			fwprintf(stream, L"Filter name is %S\n", reinterpret_cast<const wchar_t*>(pInfo.achName));
//			fwprintf(stream, L"Filter name is %s\n", pInfo.achName);

			if (ThisPinDir == PINDIR_OUTPUT && 
				wcscmp(pInfo.achName, L"Still") == 0) {
//				fwprintf(stream, L"Filter name2 is %s\n", pInfo.achName);
				continue;			
			}
*/
            IPin *pTmp = 0;
            hr = pPin->ConnectedTo(&pTmp);
			if (SUCCEEDED(hr)) {  // Already connected, not the pin we want.
                pTmp->Release();
			} else {  // Unconnected, this is the pin we want.
                pEnum->Release();
                *ppPin = pPin;
                return S_OK;
            }
        }
        pPin->Release();
    }
    pEnum->Release();

    // Did not find a matching pin.
    return E_FAIL;
}

//****************************************************************************************
// Pass it a file name in wszPath, and it will save the filter graph
// to that file.
HRESULT SaveGraphFile(IGraphBuilder *pGraph, WCHAR *wszPath) {
    const WCHAR wszStreamName[] = L"ActiveMovieGraph";
    HRESULT hr;

    IStorage *pStorage = NULL;
    // First, create a document file that will hold the GRF file
    hr = StgCreateDocfile(
         wszPath,
         STGM_CREATE | STGM_TRANSACTED | STGM_READWRITE |
             STGM_SHARE_EXCLUSIVE,
         0, &pStorage);
    if(FAILED(hr)) {
        return hr;
    }
    // Next, create a stream to store.
    IStream *pStream;
    hr = pStorage->CreateStream(
         wszStreamName,
         STGM_WRITE | STGM_CREATE | STGM_SHARE_EXCLUSIVE,
         0, 0, &pStream);
    if (FAILED(hr)) {
        pStorage->Release();
        return hr;
    }
    // The IpersistStream::Save method converts a stream
    // into a persistent object.
    IPersistStream *pPersist = NULL;
    pGraph->QueryInterface(IID_IPersistStream,
                            reinterpret_cast<void**>(&pPersist));
    hr = pPersist->Save(pStream, TRUE);
    pStream->Release();
    pPersist->Release();
    if (SUCCEEDED(hr)) {
        hr = pStorage->Commit(STGC_DEFAULT);
    }
    pStorage->Release();

    return hr;
}
