// GrabImage.cpp : Defines the entry point for the DLL application.
//


#define _WIN32_WINNT 0x0500

#include <streams.h>
#include "qedit.h"	//ISampleGrabber interface is taken from original qedit.h (Windows SDK v6.0)

#include <windows.h>
#include <atlbase.h>

//#include "stdafx.h"
#include "GrabImage.h"

#ifdef _DEBUG
FILE *stream = NULL;
#endif

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  fdwReason,     // reason for calling function
                       LPVOID lpReserved
					 )
{
	switch (fdwReason)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
			//fprintf(stream, "Destroying the graph %d", 222);
			DestroyGraph();
			break;
	}
    return TRUE;
}

HRESULT ConnectFilters(
    IGraphBuilder *pGraph, 
    IBaseFilter *pSrc, 
    IBaseFilter *pDest);

HRESULT SaveGraphFile(IGraphBuilder *pGraph, WCHAR *wszPath);

HRESULT hr = NULL;

IGraphBuilder *pGraph = NULL;
ICaptureGraphBuilder2 *pBuilder = NULL;
IBaseFilter *pVideoCap = NULL;
IBaseFilter *pGrabberF = NULL;
ISampleGrabber *pGrabber = NULL;
IVideoWindow *pVideoWindow = NULL;
IBaseFilter *pNull = NULL;

//#ifdef _DEBUG
//FILE *stream = NULL;
//#endif

AM_MEDIA_TYPE mt;

HWND m_hWnd;
LONG m_width = 0;
LONG m_height = 0;

//double ASPECT_RATIO = 1.2;

//************************************************************************************
int GRABIMAGE_API __stdcall StartPreview(HWND hWnd, int width, int height) {
	m_hWnd = hWnd;
	m_width = width;
	m_height = height;

//	LPCWSTR g_PathFileName = L"C:\\Documents and Settings\\roman\\My Documents\\Roxio\\VideoWave 5 Power\\Capture\\NONAME_004.AVI";

	//DestroyGraph();

#ifdef _DEBUG
	stream = fopen("grab_image.txt", "w");
#endif

	// Initialize the COM library.
    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) {
//        printf("ERROR - Could not initialize COM library");
#ifdef _DEBUG
		fclose( stream );
#endif
        return 1;
    }
    
	// Create the Filter Graph Manager object and retrieve its
    // IGraphBuilder interface.
    hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,
                           IID_IGraphBuilder, (void **)&pGraph);
    if (FAILED(hr)) { 
//        printf("ERROR - Could not create the Filter Graph Manager.");
        CoUninitialize();        
#ifdef _DEBUG
		fclose( stream );
#endif
//        return hr;
        return 1;
    }

    // Create the Capture Graph Builder.
    hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL,
							CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2, 
							(void **)&pBuilder);

    if (SUCCEEDED(hr)) {
        pBuilder->SetFiltergraph(pGraph);
    } else {
		DestroyGraph();
		return 1;
	}

	ICreateDevEnum *pDevEnum = NULL;
	IEnumMoniker *pEnum = NULL;

	// Create the System Device Enumerator.
	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL,
				CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, 
				reinterpret_cast<void**>(&pDevEnum));

	if (SUCCEEDED(hr)) {
		// Create an enumerator for the video capture category.
		hr = pDevEnum->CreateClassEnumerator(
								CLSID_VideoInputDeviceCategory,
								&pEnum, 0);


		USES_CONVERSION;
		WCHAR wachFriendlyName[120];

		IMoniker *pMoniker = NULL;
		while (pEnum->Next(1, &pMoniker, NULL) == S_OK) {
			IPropertyBag *pPropBag;
			wachFriendlyName[0] = 0;
			hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, 
												(void**)(&pPropBag));
			if (FAILED(hr)) {
				pMoniker->Release();
				continue;  // Skip this one, maybe the next one will work.
			}

			// Find the description or friendly name.
			VARIANT varName;
			//varName.vt = VT_BSTR;
			VariantInit(&varName);
			hr = pPropBag->Read(L"Description", &varName, 0);
			if (FAILED(hr)) {
				hr = pPropBag->Read(L"FriendlyName", &varName, 0);
				if(hr == NOERROR) {
					lstrcpyW(wachFriendlyName, varName.bstrVal);
					SysFreeString(varName.bstrVal);
				}
			}

			VariantClear(&varName); 

			if (SUCCEEDED(hr)) {
				hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&pVideoCap);
				if (SUCCEEDED(hr)) {
					// Add the video capture filter to the graph with its friendly name
					hr = pGraph->AddFilter(pVideoCap, wachFriendlyName);

					IAMAnalogVideoDecoder *pVDecoder;
					hr = pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
												  &MEDIATYPE_Video,
												  pVideoCap,
												  IID_IAMAnalogVideoDecoder, (void **)&pVDecoder);
					if(SUCCEEDED(hr)) {
						//int AnalogVideo_NTSC_M     = 0x00000001, 
						//int AnalogVideo_PAL_B      = 0x00000010,

						pVDecoder->put_TVFormat(AnalogVideo_PAL_B);
						pVDecoder->Release();
					}
				}
			}

			pPropBag->Release();
			pMoniker->Release();
		}
	}

	pDevEnum->Release();
	pEnum->Release();

	if (pVideoCap == NULL) {
		DestroyGraph();
		return 1;
	}

	// Create the Sample Grabber.
	hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER,
							IID_IBaseFilter, (void**)&pGrabberF);

    if (FAILED(hr)) { 
//        printf("ERROR - Could not create the Sample Grabber Filter.");
		DestroyGraph();
/*
        pVideoCap->Release();
        pBuilder->Release();
        pGraph->Release();
		pGraph = NULL;
        CoUninitialize();        
*/
//#ifdef _DEBUG
//		fclose( stream );
//#endif
//        return hr;
        return 1;
    }

	hr = pGraph->AddFilter(pGrabberF, L"Sample Grabber");
	if (FAILED(hr)) {
//        printf("ERROR - Could not add the Sample Grabber Filter.");
		DestroyGraph();
/*
        pGrabberF->Release();
        pVideoCap->Release();
        pBuilder->Release();
        pGraph->Release();
		pGraph = NULL;
        CoUninitialize();        
*/
//#ifdef _DEBUG
//		fclose( stream );
//#endif
//        return hr;
        return 1;
	}

    // Query the Sample Grabber for the ISampleGrabber interface.
	hr = pGrabberF->QueryInterface(IID_ISampleGrabber, (void**)&pGrabber);
    if (FAILED(hr)) {
//        printf("ERROR - Could not get the ISampleGrabber interface.");
		DestroyGraph();
/*
		pGrabberF->Release();
        pVideoCap->Release();
        pBuilder->Release();
        pGraph->Release();
		pGraph = NULL;
        CoUninitialize();
*/
//#ifdef _DEBUG
//		fclose( stream );
//#endif
//        return hr;
        return 1;
    }

	ZeroMemory(&mt, sizeof(AM_MEDIA_TYPE));
	mt.majortype = MEDIATYPE_Video;
	mt.subtype = MEDIASUBTYPE_RGB24;
	mt.pbFormat = NULL;
	mt.cbFormat = 0;

/*
	VIDEOINFOHEADER *pVih;

	pVih->rcSource.left = (pVih->bmiHeader.biWidth - 241) / 2;
	pVih->rcSource.top = (pVih->bmiHeader.biHeight - 286) / 2;
	pVih->rcSource.right = pVih->rcSource.left + 241;
	pVih->rcSource.bottom = pVih->rcSource.top + 286;

	pVih->rcTarget.left = (pVih->bmiHeader.biWidth - 241) / 2;
	pVih->rcTarget.top = (pVih->bmiHeader.biHeight - 286) / 2;
	pVih->rcTarget.right = pVih->rcTarget.left + 241;
	pVih->rcTarget.bottom = pVih->rcTarget.top + 286;

	pVih->bmiHeader.biWidth = 241;
	pVih->bmiHeader.biHeight = 286;

	mt.pbFormat = (BYTE *)pVih;
*/
	// Set the Media Type
	pGrabber->SetMediaType(&mt);
	pGrabber->SetBufferSamples(TRUE);

	
	hr = pBuilder->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, pVideoCap, NULL, pGrabberF);
	hr = pBuilder->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video, pVideoCap, NULL, NULL);
/*	
	pGrabber->GetConnectedMediaType(&mt);

	VIDEOINFOHEADER *pVih;
	pVih = (VIDEOINFOHEADER*)mt.pbFormat;

	pVih->rcSource.left = (pVih->bmiHeader.biWidth - 241) / 2;
	pVih->rcSource.top = (pVih->bmiHeader.biHeight - 286) / 2;
	pVih->rcSource.right = pVih->rcSource.left + 241;
	pVih->rcSource.bottom = pVih->rcSource.top + 286;

	pVih->rcTarget.left = (pVih->bmiHeader.biWidth - 241) / 2;
	pVih->rcTarget.top = (pVih->bmiHeader.biHeight - 286) / 2;
	pVih->rcTarget.right = pVih->rcTarget.left + 241;
	pVih->rcTarget.bottom = pVih->rcTarget.top + 286;
*/
	

	hr = pGraph->QueryInterface(IID_IVideoWindow, (void **)&pVideoWindow);
	if(hr == NOERROR) {
		pVideoWindow->put_Owner((OAHWND)m_hWnd);
		pVideoWindow->put_WindowStyle(WS_CHILD | WS_CLIPSIBLINGS);

		IBasicVideo *pBasicVideo;
		hr = pGraph->QueryInterface(IID_IBasicVideo, (void **)&pBasicVideo);
		if(hr == NOERROR) {
			LONG lWidth = 0;
			LONG lHeight = 0;
			hr = pBasicVideo->GetVideoSize(&lWidth, &lHeight);
			//fprintf(stream, "Video size width: %d\n", lWidth);
			//fprintf(stream, "Video size height: %d\n", lHeight);

			float r = (float)lWidth / lHeight;

			//pBasicVideo->put_SourceWidth(lWidth / ASPECT_RATIO);
			if (!m_width || !m_height) {
				m_width = lWidth;
				m_height = lHeight;
			} else {
				if (m_width > lWidth || (lWidth - m_width * r) < 0)
					m_width = lWidth;
				else
					m_width *= r;

				if (m_height > lHeight || (lHeight - m_height * r) < 0)
					m_height = lHeight;
				else
					m_height *= r;
			}

			pBasicVideo->put_SourceWidth(m_width);
			pBasicVideo->put_SourceHeight(m_height);
			pBasicVideo->put_SourceLeft((lWidth - m_width) / 2);
			pBasicVideo->put_SourceTop((lHeight - m_height) / 2);


			//pBasicVideo->put_DestinationWidth(m_width / 2);
			//pBasicVideo->put_DestinationHeight(m_height / 2);
			//pBasicVideo->put_DestinationLeft(0);
			//pBasicVideo->put_DestinationTop(0);

			pBasicVideo->Release();

		}

		RECT rc;
		GetClientRect(m_hWnd, &rc);

		pVideoWindow->SetWindowPosition(0, 0, rc.right, rc.bottom);
		pVideoWindow->put_Visible(OATRUE);
	}

	// Create the Null Renderer Filter.
	hr = CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER,
							IID_IBaseFilter, reinterpret_cast<void**>(&pNull));
	if (FAILED(hr)) {
//        printf("ERROR - Could not create the Null Renderer Filter.");
		FreeMediaType(mt);
		DestroyGraph();
/*
		pVideoWindow->Release();
        pGrabber->Release();
        pGrabberF->Release();
        pVideoCap->Release();
        pBuilder->Release();
        pGraph->Release();
		pGraph = NULL;
        CoUninitialize();
*/
//#ifdef _DEBUG
//		fclose( stream );
//#endif
//        return hr;
        return 1;
    }
	
	hr = pGraph->AddFilter(pNull, L"NullRenderer");
	if (FAILED(hr)) {
//        printf("ERROR - Could not add the Null Renderer Filter.");
		FreeMediaType(mt);
		DestroyGraph();
/*
		pNull->Release();
        pVideoWindow->Release();
        pGrabber->Release();
        pGrabberF->Release();
        pVideoCap->Release();
        pBuilder->Release();
        pGraph->Release();
		pGraph = NULL;
        CoUninitialize();
*/
//#ifdef _DEBUG
//		fclose( stream );
//endif
        return 1;
    }

	hr = ConnectFilters(pGraph, pGrabberF, pNull);
//#ifdef _DEBUG
//	fprintf(stream, "Return code: is %d\n", hr);
//#endif
	
	// Run the graph.
	IMediaControl *pMC = NULL;
    hr = pGraph->QueryInterface(IID_IMediaControl, (void **)&pMC);
    if(SUCCEEDED(hr)) {
        hr = pMC->Run();
        pMC->Release();
    } else {
		DestroyGraph();
		return 1;
	}

#ifdef _DEBUG
	// Before we finish, save the filter graph to a file.
	//SaveGraphFile(pGraph, L"C:\\MyGraph.GRF");
#endif

	return 0;
}

//************************************************************************************
long GRABIMAGE_API __stdcall MakeOneShot(void) {
	if (pGraph == NULL)
		return 0;

	// Set one-shot mode and buffering.
	pGrabber->SetOneShot(TRUE);

    // stop the graph
	IMediaControl *pMC = NULL;
    hr = pGraph->QueryInterface(IID_IMediaControl, (void **)&pMC);
    if(SUCCEEDED(hr))
    {
        hr = pMC->Stop();
        pMC->Release();
    }

	// Grab the Sample
	// Find the required buffer size.
	long cbBuffer = 0;
	hr = pGrabber->GetCurrentBuffer(&cbBuffer, NULL);
	//fprintf(stream, "Buffer length is %d\n", cbBuffer);
	if (cbBuffer <= 0) {
//        printf("ERROR - Could not get connected media type.");
		DestroyGraph();
/*
		FreeMediaType(mt);
        pNull->Release();
        pVideoWindow->Release();
        pGrabber->Release();
        pGrabberF->Release();
        pVideoCap->Release();
        pBuilder->Release();
        pGraph->Release();
		pGraph = NULL;
        CoUninitialize();
*/
//#ifdef _DEBUG
//		fclose( stream );
//#endif
		return 0;
	}

	hr = pGrabber->GetConnectedMediaType(&mt);
	if (FAILED(hr)) {
//        printf("ERROR - Could not get connected media type.");
		DestroyGraph();
/*
		FreeMediaType(mt);
        pNull->Release();
        pVideoWindow->Release();
        pGrabber->Release();
        pGrabberF->Release();
        pVideoCap->Release();
        pBuilder->Release();
        pGraph->Release();
		pGraph = NULL;
        CoUninitialize();
*/
//#ifdef _DEBUG
//		fclose( stream );
//#endif
		return 0;
	}

	// Examine the format block.
	VIDEOINFOHEADER *pVih;
	if ((mt.formattype == FORMAT_VideoInfo) && 
		(mt.cbFormat >= sizeof(VIDEOINFOHEADER)) &&
		(mt.pbFormat != NULL) ) {
		pVih = (VIDEOINFOHEADER*)mt.pbFormat;
	} else {
		// Wrong format. Free the format block and return an error.
//        printf("ERROR - Wrong format.");
		FreeMediaType(mt);
		DestroyGraph();
/*
		pNull->Release();
        pVideoWindow->Release();
        pGrabber->Release();
        pGrabberF->Release();
        pVideoCap->Release();
        pBuilder->Release();
        pGraph->Release();
		pGraph = NULL;
        CoUninitialize();
*/
//#ifdef _DEBUG
//		fclose( stream );
//#endif
		return 0;
	}
	/*
			pBasicVideo->put_SourceWidth(241 * 2);
			pBasicVideo->put_SourceHeight(286 * 2);
			pBasicVideo->put_SourceLeft((lWidth - 241 * 2) / 2);
			pBasicVideo->put_SourceTop((lHeight - 286 * 2) / 2);
	*/
	//pVih->bmiHeader.biWidth = pVih->bmiHeader.biWidth * 2;
	//pVih->bmiHeader.biHeight = pVih->bmiHeader.biHeight * 2;
	//fprintf(stream, "Video size width: %d\n", pVih->bmiHeader.biWidth);
	//fprintf(stream, "Video size height: %d\n", pVih->bmiHeader.biHeight);

	pVih->bmiHeader.biWidth = m_width;
	pVih->bmiHeader.biHeight = m_height;

	//int newStride = ((pVih->bmiHeader.biWidth + 3) & ~3) * pVih->bmiHeader.biBitCount / 8;
	int newStride = (pVih->bmiHeader.biWidth * (pVih->bmiHeader.biBitCount / 8) + 3) & ~3; 


	//pVih->bmiHeader.biWidth = pVih->bmiHeader.biHeight / ASPECT_RATIO;
	//int newStride = ((pVih->bmiHeader.biWidth + 3) & ~3) * pVih->bmiHeader.biBitCount / 8;
	pVih->bmiHeader.biSizeImage = newStride * pVih->bmiHeader.biHeight;
	
	DWORD bfSize = (DWORD) (sizeof(BITMAPFILEHEADER) + 
							pVih->bmiHeader.biSize + 
							pVih->bmiHeader.biClrUsed * sizeof(RGBQUAD) +
							pVih->bmiHeader.biSizeImage); 
/*
	DWORD bfSize = (DWORD) (sizeof(BITMAPFILEHEADER) + 
							pVih->bmiHeader.biSize + 
							pVih->bmiHeader.biClrUsed * sizeof(RGBQUAD) +
							cbBuffer); 
*/
	return bfSize;
}

//************************************************************************************
//void GRABIMAGE_API __stdcall GetBitmap(LPBYTE lpBitsPB) {
int GRABIMAGE_API __stdcall TakeSnap(char* fileName) {


	//GlobalFree((HGLOBAL)lpBits);
	//delete[] pBuffOrg;

//	DestroyGraph();
//return;

	// Set one-shot mode and buffering.
//	pGrabber->SetOneShot(TRUE);

	if (pGraph == NULL)
		return 1;

    // stop the graph
	IMediaControl *pMC = NULL;
    hr = pGraph->QueryInterface(IID_IMediaControl, (void **)&pMC);
    if(SUCCEEDED(hr))
    {
        hr = pMC->Stop();
        pMC->Release();
    } else {
		DestroyGraph();
		return 1;
	}

	long cbBuffer = 0;
	hr = pGrabber->GetCurrentBuffer(&cbBuffer, NULL);
#ifdef _DEBUG
	fprintf(stream, "Buffer length is %d\n", cbBuffer);
#endif

	//char *pBuffer = new char[cbBuffer];
	BYTE *pBuffer = NULL;

    pBuffer = (BYTE *)CoTaskMemAlloc(cbBuffer);
	if (!pBuffer) {
		// Out of memory.
//        printf("ERROR - Out of memory.");
		//GlobalFree((HGLOBAL)lpBits);
		//delete[] pBuffOrg;

		DestroyGraph();
/*
		FreeMediaType(mt);
        pNull->Release();
        pVideoWindow->Release();
        pGrabber->Release();
        pGrabberF->Release();
        pVideoCap->Release();
        pBuilder->Release();
        pGraph->Release();
		pGraph = NULL;
        CoUninitialize();
*/
//#ifdef _DEBUG
//		fclose( stream );
//#endif
		return 1;
	}

	BYTE *pBuffOrg = pBuffer;

	hr = pGrabber->GetCurrentBuffer(&cbBuffer, (long*)pBuffer);
	if (FAILED(hr)) {
		DestroyGraph();
//#ifdef _DEBUG
//		fclose( stream );
//#endif
		return 1;
	}

	hr = pGrabber->GetConnectedMediaType(&mt);
	if (FAILED(hr)) {
		CoTaskMemFree(pBuffer);
		DestroyGraph();
/*
		FreeMediaType(mt);
        pNull->Release();
        pVideoWindow->Release();
        pGrabber->Release();
        pGrabberF->Release();
        pVideoCap->Release();
        pBuilder->Release();
        pGraph->Release();
		pGraph = NULL;
        CoUninitialize();
*/
//#ifdef _DEBUG
//		fclose( stream );
//#endif
		return 1;
	}


	// Examine the format block.
	VIDEOINFOHEADER *pVih;
	if ((mt.formattype == FORMAT_VideoInfo) && 
		(mt.cbFormat >= sizeof(VIDEOINFOHEADER)) &&
		(mt.pbFormat != NULL) ) {
		pVih = (VIDEOINFOHEADER*)mt.pbFormat;
	} else {
		// Wrong format. Free the format block and return an error.
//        printf("ERROR - Wrong format.");
		FreeMediaType(mt);
		CoTaskMemFree(pBuffer);
		DestroyGraph();
/*
		FreeMediaType(mt);
        pNull->Release();
        pVideoWindow->Release();
        pGrabber->Release();
        pGrabberF->Release();
        pVideoCap->Release();
        pBuilder->Release();
        pGraph->Release();
		pGraph = NULL;
        CoUninitialize();
*/
//#ifdef _DEBUG
//		fclose( stream );
//#endif
		return 1;
	}

    //hr = WriteBitmap(fileName, &pVih->bmiHeader, 
    //        mt.cbFormat - SIZE_PREHEADER, pBuffer, cbBuffer);

    BITMAPFILEHEADER hdr = { }; // bitmap file-header 
    hdr.bfType = 0x4d42;        // 0x42 = "B" 0x4d = "M" 

	// Compute the size of the entire file. 

	// (Round the image width up to a DWORD boundary.)
	//int origStride = ((pVih->bmiHeader.biWidth + 3) & ~3) * pVih->bmiHeader.biBitCount / 8;
	int origStride = (pVih->bmiHeader.biWidth * (pVih->bmiHeader.biBitCount / 8) + 3) & ~3; 

#ifdef _DEBUG
	fprintf(stream, "Video size width: %d\n", pVih->bmiHeader.biWidth);
	fprintf(stream, "Video size height: %d\n", pVih->bmiHeader.biHeight);
	fprintf(stream, "Video size BMIHEADER size: %d\n", pVih->bmiHeader.biSize);
	fprintf(stream, "Video size size image: %d\n", pVih->bmiHeader.biSizeImage);
#endif
//pVih->rcSource.

			//pBasicVideo->put_SourceWidth(241 * 2);
			//pBasicVideo->put_SourceHeight(286 * 2);
			//pBasicVideo->put_SourceLeft((lWidth - 241 * 2) / 2);
			//pBasicVideo->put_SourceTop((lHeight - 286 * 2) / 2);

	//pVih->bmiHeader.biWidth = pVih->bmiHeader.biHeight / ASPECT_RATIO;
/*
	pVih->rcSource.left = (pVih->bmiHeader.biWidth - 241) / 2;
	pVih->rcSource.top = (pVih->bmiHeader.biHeight - 286) / 2;
	pVih->rcSource.right = pVih->rcSource.left + 241;
	pVih->rcSource.bottom = pVih->rcSource.top + 286;

	pVih->rcTarget.left = (pVih->bmiHeader.biWidth - 241) / 2;
	pVih->rcTarget.top = (pVih->bmiHeader.biHeight - 286) / 2;
	pVih->rcTarget.right = pVih->rcTarget.left + 241;
	pVih->rcTarget.bottom = pVih->rcTarget.top + 286;
*/
	long clipRectLeftPosition = (pVih->bmiHeader.biHeight - m_height) / 2;
//	fprintf(stream, "Top: %d\n", clipRectLeftPosition);
	clipRectLeftPosition *= origStride;
//	fprintf(stream, "Top: %d\n", clipRectLeftPosition);
	clipRectLeftPosition += ((pVih->bmiHeader.biWidth - m_width) / 2) * 3;
//	fprintf(stream, "Top: %d\n", clipRectLeftPosition);

	pBuffer += clipRectLeftPosition;

	pVih->bmiHeader.biWidth = m_width;
	pVih->bmiHeader.biHeight = m_height;

	//int newStride = ((pVih->bmiHeader.biWidth + 3) & ~3) * pVih->bmiHeader.biBitCount / 8;
	int newStride = (pVih->bmiHeader.biWidth * (pVih->bmiHeader.biBitCount / 8) + 3) & ~3; 

	pVih->bmiHeader.biSizeImage = newStride * pVih->bmiHeader.biHeight;

#ifdef _DEBUG
	fprintf(stream, "Video source left: %d\n", pVih->rcSource.left);
	fprintf(stream, "Video source top: %d\n", pVih->rcSource.top);
	fprintf(stream, "Video source right: %d\n", pVih->rcSource.right);
	fprintf(stream, "Video source bottom: %d\n", pVih->rcSource.bottom);
#endif

#ifdef _DEBUG
	fprintf(stream, "Video target left: %d\n", pVih->rcTarget.left);
	fprintf(stream, "Video target top: %d\n", pVih->rcTarget.top);
	fprintf(stream, "Video target right: %d\n", pVih->rcTarget.right);
	fprintf(stream, "Video target bottom: %d\n", pVih->rcTarget.bottom);
#endif

#ifdef _DEBUG
	fprintf(stream, "Origin stride: %d\n", origStride);
	fprintf(stream, "New stride: %d\n", newStride);
#endif

	hdr.bfSize = (DWORD) (sizeof(BITMAPFILEHEADER) + 
							pVih->bmiHeader.biSize + 
							pVih->bmiHeader.biClrUsed * sizeof(RGBQUAD) +
							pVih->bmiHeader.biSizeImage); 
/*
	hdr.bfSize = (DWORD) (sizeof(BITMAPFILEHEADER) + 
							pVih->bmiHeader.biSize + 
							pVih->bmiHeader.biClrUsed * sizeof(RGBQUAD) +
							cbBuffer); 
*/
	hdr.bfReserved1 = 0; 
    hdr.bfReserved2 = 0; 

    // Compute the offset to the array of color indices. 
    hdr.bfOffBits = (DWORD) sizeof(BITMAPFILEHEADER) + 
							pVih->bmiHeader.biSize + 
							pVih->bmiHeader.biClrUsed * sizeof(RGBQUAD); 

	LPBYTE lpBits = (LPBYTE) GlobalAlloc(GMEM_FIXED, hdr.bfSize);

	LPBYTE lpBuff = lpBits;
	memcpy (lpBuff, &hdr, sizeof(BITMAPFILEHEADER));
	lpBuff += sizeof(BITMAPFILEHEADER);
	memcpy (lpBuff, &pVih->bmiHeader, sizeof(BITMAPINFOHEADER));
	lpBuff += sizeof(BITMAPINFOHEADER);
//	memcpy (lpBuff, pBuffer, cbBuffer);

	for (int i = 0; i < pVih->bmiHeader.biHeight; i++) {
		memcpy (lpBuff, pBuffer, newStride);
		lpBuff += newStride;
		pBuffer += origStride;
	}

//	memcpy (lpBitsPB, lpBits, hdr.bfSize);

//#ifdef _DEBUG
	HANDLE hf;					// file handle 
    DWORD dwTmp; 
	// Create the .BMP file. 
	hf = CreateFile(fileName, 
					GENERIC_READ | GENERIC_WRITE, 
					(DWORD) 0, 
                    NULL, 
					CREATE_ALWAYS, 
					FILE_ATTRIBUTE_NORMAL, 
					(HANDLE) NULL);

    // Copy the array of color indices into the .BMP file. 
    WriteFile(hf, (LPSTR) lpBits, (int) hdr.bfSize, (LPDWORD) &dwTmp, NULL);

	// Close the .BMP file. 
    CloseHandle(hf); 

//#endif
	FreeMediaType(mt);
	GlobalFree((HGLOBAL)lpBits);
	CoTaskMemFree(pBuffOrg);
	//delete[] pBuffOrg;

	// Run the graph.
	IMediaControl *pMC2 = NULL;
    hr = pGraph->QueryInterface(IID_IMediaControl, (void **)&pMC2);
    if(SUCCEEDED(hr)) {
        hr = pMC2->Run();
        pMC2->Release();
    } else {
		DestroyGraph();
		return 1;
	}

	return 0;
	//DestroyGraph();
}


//************************************************************************************
void GRABIMAGE_API __stdcall DestroyGraph(void) {
	if (pGraph != NULL) {
//		hr = pGraph->Abort();
/*
		// Set one-shot mode and buffering.
		pGrabber->SetOneShot(TRUE);
*/
		// stop the graph
		IMediaControl *pMC = NULL;
		hr = pGraph->QueryInterface(IID_IMediaControl, (void **)&pMC);
		if(SUCCEEDED(hr)) {
			hr = pMC->Stop();
			pMC->Release();
		}

		SAFE_RELEASE(pNull);
		//pNull->Release();
		SAFE_RELEASE(pVideoWindow);
		//pVideoWindow->Release();
		SAFE_RELEASE(pGrabber);
		//pGrabber->Release();
		SAFE_RELEASE(pGrabberF);
		//pGrabberF->Release();
		SAFE_RELEASE(pVideoCap);
		//pVideoCap->Release();
		SAFE_RELEASE(pBuilder);
		//pBuilder->Release();
		SAFE_RELEASE(pGraph);
		//pGraph->Release();
		//pGraph = NULL;
//		CoUninitialize();
#ifdef _DEBUG
		if (stream) {
			fclose( stream );
			stream = NULL;
		}
#endif
		CoUninitialize();

	}
}

/*
// This is the constructor of a class that has been exported.
// see GrabImage.h for the class definition
CGrabImage::CGrabImage()
{ 
	return; 
}
*/
