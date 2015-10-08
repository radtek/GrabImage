// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the GRABIMAGE_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// GRABIMAGE_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef GRABIMAGE_EXPORTS
#define GRABIMAGE_API __declspec(dllexport)
#else
#define GRABIMAGE_API __declspec(dllimport)
#endif
/*
template <class T> void SafeRelease(T **ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}
*/
#define SAFE_RELEASE(x) { if (x) x->Release(); x = NULL; }

/*
// This class is exported from the GrabImage.dll
class GRABIMAGE_API CGrabImage {
public:
	CGrabImage(void);
	// TODO: add your methods here.
};

extern GRABIMAGE_API int nGrabImage;

GRABIMAGE_API int fnGrabImage(void);
*/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	int GRABIMAGE_API __stdcall StartPreview(HWND hWnd, int width, int height);
	int GRABIMAGE_API __stdcall TakeSnap(char* fileName);
//	void GRABIMAGE_API __stdcall GrabImage(void);
	long GRABIMAGE_API __stdcall MakeOneShot();
	void GRABIMAGE_API __stdcall GetBitmap(LPBYTE lpBits);
	void GRABIMAGE_API __stdcall DestroyGraph(void);

#ifdef __cplusplus
}
#endif
